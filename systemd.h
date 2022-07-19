#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <systemd/sd-bus.h>
#include <libcgroup.h>
#include <libcgroup-internal.h>

#include "tools/tools-common.h"

/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef _LIBCGROUP_SYSTEMD_H
#define _LIBCGROUP_SYSTEMD_H

#ifndef _LIBCGROUP_H_INSIDE
#error "Only <libcgroup.h> should be included directly."
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a systemd scope in the specified slice
 *
 * @param scope_name Name of the scope, must end in .scope
 * @param slice_name Name of the slice, must end in .slice
 * @param delegated Instruct systemd that this cgroup is delegated and should not be managed
 * 	  by systemd
 * @param mode Unit mode for systemd, must be from SYSD_UNIT_MODE Enum
 */
int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated, enum SYSD_UNIT_MODE mode);

/**
 * Create a systemd scope in the user slice
 *
 * @param scope_name Name of the scope, must end in .scope
 * @param delegated Instruct systemd that this cgroup is delegated and should not be managed
 * 	  by systemd
 * @param mode Unit mode for systemd, must be from SYSD_UNIT_MODE Enum
 */
int cgroup_create_scope_user_slice(char *scope_name, int delegated, enum SYSD_UNIT_MODE mode);

/**
 * Check if a systemd scope has been delegated
 *
 * @param scope_name Name of scope to be checked, must end in .scope
 */
 int cgroup_is_delegated(char *scope_name);

#endif /* SYSTEMD */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _LIBCGROUP_SYSTEMD_H */