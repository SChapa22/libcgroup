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

char SYSD_UNIT_MODE_NAMES[5][20] = {"fail", "replace", "isolate",
	"ignore-dependencies", "ignore-requirements"};

static void permanent_sleep(void)
{
	while (1)
		sleep(10000);
}

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

int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated,
	enum cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
{

	sd_bus_message *m = NULL, *reply = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus *bus = NULL;
	int ret = 0;

	int child_pid = fork();

	if (child_pid < 0) {
		cgroup_err("fork() failed: %d:%s\n", child_pid, strerror(-child_pid));
	} else if (child_pid == 0) {
		permanent_sleep();
	} else {
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
	struct statfs fs;
	int error = 0;

	memset(built_path, 0, FILENAME_MAX + 1);
	error = sd_pid_get_cgroup(*target, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			cgroup_err("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*target, &found_path);
	}
	error = statfs(CG_SCOPE_TREE_ROOT, &fs);
	if (error < 0){
		cgroup_err("statfs on %s failed: %d-%s\n", CG_SCOPE_TREE_ROOT, errno, strerror(errno));
	}
	if ((fs.f_type == (typeof(fs.f_type)) CGROUP2_SUPER_MAGIC)) {
		strncat(built_path, CG_SCOPE_TREE_ROOT, FILENAME_MAX);
	} else if ((fs.f_type == (typeof(fs.f_type)) TMPFS_MAGIC)) {
		error = statfs(CG_SCOPE_TREE_ROOT_UNI, &fs);
		if (error < 0) {
			cgroup_err("statfs on %s failed: %d-%s\n", CG_SCOPE_TREE_ROOT_UNI, errno, strerror(errno));
		}
		if ((fs.f_type == (typeof(fs.f_type)) CGROUP2_SUPER_MAGIC)) {
			strncat(built_path, CG_SCOPE_TREE_ROOT_UNI, FILENAME_MAX);
		} else {
			error = statfs(CG_SCOPE_TREE_ROOT_SYSD, &fs);
			if (error < 0) {
				cgroup_err("statfs on %s failed: %d-%s\n", CG_SCOPE_TREE_ROOT_UNI, errno, strerror(errno));
			}
			if (((fs.f_type == (typeof(fs.f_type)) CGROUP2_SUPER_MAGIC)) || ((fs.f_type == (typeof(fs.f_type)) CGROUP_SUPER_MAGIC))) {
				strncat(built_path, CG_SCOPE_TREE_ROOT_SYSD, FILENAME_MAX);
			}
			else {
				return ECGROUPUNKMOUNT;
			}
		}
	} else if ((fs.f_type == (typeof(fs.f_type)) SYSFS_MAGIC)) {
		return ECGROUPNOTMOUNTED;
	} else {
		return ECGROUPUNKMOUNT;
	}
	strncat(built_path, found_path, FILENAME_MAX);
	error = cgroup_is_delegated(built_path);
	free(found_path);
	memset(built_path, 0, FILENAME_MAX + 1);

	return error;
}

#endif /* SYSTEMD */
