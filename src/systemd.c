// SPDX-License-Identifier: LGPL-2.1-only
/**
 * Libcgroup systemd interfaces
 *
 * Copyright (c) 2022 Oracle and/or its affiliates.
 * Author: Silvia Chapa <silvia.chapa@oracle.com>
 */

#ifdef SYSTEMD

#include <dirent.h>
#include <errno.h>
#include <libcgroup.h>
#include <libcgroup-internal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <linux/magic.h>

#include <sys/xattr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>

#define CG_SYSTEMD_USER_SLICE_NAME "user.slice"
#define CG_SCOPE_TREE_ROOT "/sys/fs/cgroup/"
#define CG_SCOPE_TREE_ROOT_UNI "/sys/fs/cgroup/unified/"
#define CG_SCOPE_TREE_ROOT_SYSD "/sys/fs/cgroup/systemd/"

/*
 * See https://www.freedesktop.org/software/systemd/man/systemctl.html ctrl-f "--job-mode"
 * for a description of the unit mode functions.
 */
char SYSD_UNIT_MODE_NAMES[5][20] = {"fail", "replace", "isolate",
	"ignore-dependencies", "ignore-requirements"};

static void permanent_sleep(void)
{
	while (1)
		sleep(10000);
}

/*
 * Originally added to allow checking of scope/slice names for invalid inputs in scope/slice_name.
 * @TODO Determine if this should be checked and/or corrected here, allowing user to give names
 *       without the ".scope"/".slice" endings.
 * @TODO Determine if this should be filtered against here to give a more specific error message.
 * @TODO Determine if checks should be made agains invalid ".service" ending.
 */
int match_ends(char *a, char *b)
{
	int a_len = strlen(a), b_len = strlen(b);

	while ((a_len >= 0) && (b_len >= 0)) {
		if (a[a_len] != b[b_len])
			return 0;
		a_len = a_len - 1;
		b_len = b_len - 1;
	}
	return 1;
}

/*
 * Because of the asynchronous nature of the bus, this function could return before the requested
 * scope has been fully instantiated. Always check that it exists prior to any further calls which
 * use/modify the scope, as demonstrated in SC_test.c
 * See https://wiki.archlinux.org/title/cgroups for description of cgroup functions & hierarchy
 *     as done from command line
 * See https://systemd.io/CGROUP_DELEGATION/ for descriptions of scope vs. slice vs. service
 * See https://gitlab.com/franks_reich/systemd-by-example/-/tree/master/ for introduction to
 *     SystemD and sd_bus calls
 * @TODO SC_test.c is a temporary test/demo standalone. Once permanent testing files have been
 *       written, modify here. It is assumed here that the scope exists once it's corresponding
 *       "folder" has been added to the cgroup hierarchy file tree.
 * @TODO The create_cgroup function controls more than what is touched by this function; expand
 *       this function to provide similar controls over the generated cgroup.
 */
int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated,
	enum cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
{

	sd_bus_message *m = NULL, *reply = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus *bus = NULL;
	int ret = 0;

	int child_pid = fork();

	/*
	 * Fork to ensure a process is always present in the scope, lest SystemD's clean-
	 * up removes the scope before the user can put something in it. The child process
	 * sleeps infinitely, and is returned to the user, while the parent calls sd_bus.
	 */
	if (child_pid < 0) {
		cgroup_err("fork() failed: %d:%s\n", child_pid, strerror(-child_pid));
	} else if (child_pid == 0) {
		permanent_sleep();
	} else {
		/*
		 * System bus requires sudo, but can place slice/scope at any level of the
		 * hierarchy. User bus can only create slice/scope inside the user slice. The
		 * change would only swap out sd_bus_open_system().
		 */
		ret = sd_bus_open_system(&bus);
		if (ret < 0) {
			cgroup_err("sd_bus_open_system failed: %d:%s\n", ret, strerror(-ret));
			goto out;
		}

		ret = sd_bus_message_new_method_call(bus, &m, "org.freedesktop.systemd1",
			"/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
			 "StartTransientUnit");
		if (ret < 0) {
			cgroup_err("sd_bus_message_new_method_call failed: %d:%s\n",
				ret, strerror(-ret));
			goto out;
		}

		// Identify the scope and where it should be built
		ret = sd_bus_message_append(m, "ss", scope_name, SYSD_UNIT_MODE_NAMES[mode]);
		if (ret < 0) {
			cgroup_err("1st sd_bus_message_append failed: %d:%s\n",
				ret, strerror(-ret));
			goto out;
		}

		ret = sd_bus_message_open_container(m, 'a', "(sv)");
		if (ret < 0) {
			cgroup_err("sd_bus_message_open_container failed: %d:%s\n",
				ret, strerror(-ret));
			goto out;
		}

		/*
		 * Ensure the child process is moved into the scope when it is created to prevent
		 * having scope removed by systemd.
		 */
		ret = sd_bus_message_append(m, "(sv)", "PIDs", "au", 1, (uint32_t) child_pid);
		if (ret < 0) {
			cgroup_err("2nd sd_bus_message_append failed: %d:%s\n",
				ret, strerror(-ret));
			goto out;
		}

		if (delegated == 1) {
			ret = sd_bus_message_append(m, "(sv)", "Delegate", "b", 1);
			if (ret < 0) {
				cgroup_err("3rd sd_bus_message_append failed: %d:%s\n",
					ret, strerror(-ret));
				goto out;
			}
		}

		ret = sd_bus_message_append(m, "(sv)", "Slice", "s", slice_name);
		if (ret < 0) {
			cgroup_err("4th sd_bus_message_append failed: %d:%s\n",
				ret, strerror(-ret));
			goto out;
		}

		ret = sd_bus_message_close_container(m);
		if (ret < 0) {
			cgroup_err("sd_bus_message_close failed: %d:%s\n",
				ret, strerror(-ret));
			goto out;
		}

		ret = sd_bus_message_append(m, "a(sa(sv))", 0);
		if (ret < 0) {
			cgroup_err("5th sd_bus_message_append failed: %d:%s\n",
				ret, strerror(-ret));
			goto out;
		}

		ret = sd_bus_call(bus, m, 0, &error, &reply);
		/*
		 * Error message that indicates the scope name is invalid.
		 */
		if (ret == -17)
			ret = ECGSYSDSLICESCOPEEXISTS;
		if (ret < 0) {
			cgroup_err("sd_bus_call failed: %d:%s\n", ret, strerror(-ret));
			goto out;
		}

		*sleeper_pid = child_pid;
	}

out:
	if (m != NULL)
		sd_bus_message_unref(m);
	if (reply != NULL)
		sd_bus_message_unref(reply);
	if (bus != NULL)
		sd_bus_unref(bus);
	if (ret < 0) {
		kill(child_pid, SIGSEGV);
		ret = ret * -1;
	}

	return ret;
}

int cgroup_create_scope_user_slice(char *scope_name, int delegated,
	enum cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
{
	return cgroup_create_scope_and_slice(scope_name, CG_SYSTEMD_USER_SLICE_NAME,
					delegated, mode, sleeper_pid);
}

int cgroup_is_delegated(char *scope_name)
{

	char result[FILENAME_MAX];
	int ret = 0;

	/*
	 * Depending on the version of SystemD used, either the trusted.delegate or user.delegate
	 * property might be set to indicate that the scope is not controlled by SystemD. Prior to
	 * version 251, neither of these is set; this code cannot decide delegation status for scopes
	 * on v250 or below, and will thus assume they are not delegated to prevent errors. Further,
	 * getxattr will fail if the provided path does not exist, so always check that the cgroup has
	 * been added to the hierarchy before calling is_delegated().
	 * See https://lwn.net/Articles/896043/ for description of SystemD v251
	 * See https://man7.org/linux/man-pages/man2/getxattr.2.html for description of getxattr
	 * @TODO Add support for older versions of SystemD.
	 * @TODO getxattr is an external library function. Should libcgroup provide a get function
	 *       for xattributes which, like these two, affect cgroup operation and/or hierarchy?
	 */
	ret = getxattr(scope_name, "trusted.delegate", result, FILENAME_MAX);
	if ((ret == 1) && (strncmp(result, "1", 1) == 0)) {
		ret = 1;
	} else if (errno == ENODATA) {
		int retOrig = errno;

		ret = getxattr(scope_name, "user.delegate", result, FILENAME_MAX);
		if ((ret == 1) && (strncmp(result, "1", 1) == 0)) {
			ret = 1;
		} else if (errno == ENODATA) {
			ret = 0;
		} else {
			cgroup_err("getxattr failed for trusted.delegate: %d:%s\n",
				retOrig, strerror(-retOrig));
			cgroup_err("getxattr failed for user.delegate: %d:%s\n",
				errno, strerror(errno));
			fflush(stderr);
		}
	}

	return ret;
}

int cgroup_is_delegated_pid(pid_t *target)
{
	char *found_path, built_path[FILENAME_MAX + 1];
	int error = 0, mount_err = 0;
	struct statfs fs;

	/*
	 * Errors result in testing unless memset is used both before and after use of built_path.
	 * @TODO Cause of errors unknown, investigate & resolve.
	 */
	memset(built_path, 0, FILENAME_MAX + 1);
	error = sd_pid_get_cgroup(*target, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			// ESRCH indicates the scope has not been created yet, expected from async bus
			cgroup_err("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*target, &found_path);
	}
	/*
	 * Determine root path for cgroup hierarchy, accumulated in built_path, by checking the two
	 * roots used in legacy, unified, and hybrid mode of libcgroup, as well as the root if SystemD
	 * controls the system. Hardcoded values for these roots are #define'd on lines 32-34.
	 * See https://linux.die.net/man/2/statfs for a description of statfs
	 * @TODO Determine which versions of libcgroup & systemd are supported.
	 * @TODO Find a way to support a custom tree root, if possible. Check if libcgroup tracks root.
	 */
	error = statfs(CG_SCOPE_TREE_ROOT, &fs);
	if (error < 0){
		cgroup_err("statfs on %s failed: %d-%s\n", CG_SCOPE_TREE_ROOT, errno, strerror(errno));
	}
	if ((fs.f_type == (typeof(fs.f_type)) CGROUP2_SUPER_MAGIC)) {
		strncat(built_path, CG_SCOPE_TREE_ROOT, FILENAME_MAX);
	} else if ((fs.f_type == (typeof(fs.f_type)) TMPFS_MAGIC)) {
		error = statfs(CG_SCOPE_TREE_ROOT_UNI, &fs);
		if (error < 0) {
			cgroup_err("statfs on %s failed: %d-%s\n", CG_SCOPE_TREE_ROOT_UNI, errno,
						strerror(errno));
		}
		if ((fs.f_type == (typeof(fs.f_type)) CGROUP2_SUPER_MAGIC)) {
			strncat(built_path, CG_SCOPE_TREE_ROOT_UNI, FILENAME_MAX);
		} else {
			error = statfs(CG_SCOPE_TREE_ROOT_SYSD, &fs);
			if (error < 0) {
				cgroup_err("statfs on %s failed: %d-%s\n", CG_SCOPE_TREE_ROOT_UNI, errno,
							strerror(errno));
			}
			if (((fs.f_type == (typeof(fs.f_type)) CGROUP2_SUPER_MAGIC)) ||
				((fs.f_type == (typeof(fs.f_type)) CGROUP_SUPER_MAGIC))) {
				strncat(built_path, CG_SCOPE_TREE_ROOT_SYSD, FILENAME_MAX);
			}
			else {
				mount_err = 2;
				goto out_isd;
			}
		}
	} else if ((fs.f_type == (typeof(fs.f_type)) SYSFS_MAGIC)) {
		mount_err = 1;
		goto out_isd;
	} else {
		mount_err = 2;
		goto out_isd;
	}
	strncat(built_path, found_path, FILENAME_MAX);

	error = cgroup_is_delegated(built_path);

out_isd:

	free(found_path);
	memset(built_path, 0, FILENAME_MAX + 1);
	if (mount_err == 1) {
		return ECGROUPNOTMOUNTED;
	}
	else if (mount_err == 2) {
		return ECGROUPUNKMOUNT;
	}

	return error;
}

#endif /* SYSTEMD */
