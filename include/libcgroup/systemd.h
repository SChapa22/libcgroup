/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef _LIBCGROUP_SYSTEMD_H
#define _LIBCGROUP_SYSTEMD_H

#ifndef _LIBCGROUP_H_INSIDE
#error "Only <libcgroup.h> should be included directly."
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum cgroup_sysd_unit_mode {
	SYSD_UNIT_MODE_FAIL = 0,
	SYSD_UNIT_MODE_REPLACE = 1,
	SYSD_UNIT_MODE_ISOLATE = 2,
	SYSD_UNIT_MODE_IGN_DEPS = 3,
	SYSD_UNIT_MODE_IGN_REQS = 4,
};

/**
 * Create a systemd scope in the specified slice
 *
 * @param scope_name Name of the scope, must end in .scope
 * @param slice_name Name of the slice, must end in .slice
 * @param delegated Instruct systemd that this cgroup is delegated and should not be managed
 * 	  by systemd
 * @param mode Unit mode for systemd, must be from SYSD_UNIT_MODE Enum
 * @param sleeper_pid Pointer wherein to return the PID of the infinite sleep child process
 *    that is placed into the newly-created scope
 */
int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated,
	enum cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid);

/**
 * Create a systemd scope in the user slice
 *
 * @param scope_name Name of the scope, must end in .scope
 * @param delegated Instruct systemd that this cgroup is delegated and should not be managed
 *	  by systemd
 * @param mode Unit mode for systemd, must be from SYSD_UNIT_MODE Enum
 * @param sleeper_pid Pointer wherein to return the PID of the infinite sleep child process
 *    that is placed into the newly-created scope
 */
int cgroup_create_scope_user_slice(char *scope_name, int delegated,
	enum cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid);

/**
 * Check if a systemd scope has been delegated
 *
 * @param scope_name Name of scope to be checked, must end in .scope
 */
int cgroup_is_delegated(char *scope_name);

/**
 * Check if the systemd scope a process is in has been delegated, based on process PID
 *
 * @param target Pointer to the process PID
 */
int cgroup_is_delegated_pid(pid_t *target);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBCGROUP_SYSTEMD_H */
