# SPDX-License-Identifier: LGPL-2.1-only
#
# Libcgroup Python Bindings
#
# Copyright (c) 2021-2022 Oracle and/or its affiliates.
# Author: Silvia Chapa <silvia.chapa@oracle.com>
#

# cython: language_level = 3str

""" Python bindings for the libcgroup library's systemd support functions/definitions
"""

__author__ =  'Silvia Chapa <silvia.chapa@oracle.com>'
__date__ = "8 August 2022"


# Equivalent to the defined variables in systemd.c
CG_SYSTEMD_USER_SLICE_NAME = "user.slice"

cpdef enum cgroup_sysd_unit_mode:
	SYSD_UNIT_MODE_FAIL = 0
	SYSD_UNIT_MODE_REPLACE = 1
	SYSD_UNIT_MODE_ISOLATE = 2
	SYSD_UNIT_MODE_IGN_DEPS = 3
	SYSD_UNIT_MODE_IGN_REQS = 4

#CG_SYSTEMD_UNIT_MODE_FAIL = "fail"
#CG_SYSTEMD_UNIT_MODE_REPLACE = "replace"
#CG_SYSTEMD_UNIT_MODE_ISOLATE = "isolate"
#CG_SYSTEMD_UNIT_MODE_IGN_DEPENDENCIES = "ignore-dependencies"
#CG_SYSTEMD_UNIT_MODE_IGN_REQUIREMENTS = "ignore-requirements"

cdef extern from "systemd.h":
	int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated,
					enum cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
	cgroup_create_scope_user_slice(char *scope_name, int delegated,
					enum cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
	int cgroup_is_delegated(char *path)
	int cgroup_is_delegate_pid(pid_t *target)
