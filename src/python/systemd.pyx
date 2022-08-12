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


from posix.types cimport pid_t

#cdef extern from "unistd.h":
#	ctypedef pid_t

# Equivalent to the defined variables in systemd.c
CG_SYSTEMD_USER_SLICE_NAME = "user.slice"

cdef extern from "systemd.h":

	cpdef enum cgroup_sysd_unit_mode:
		SYSD_UNIT_MODE_FAIL = 0
		SYSD_UNIT_MODE_REPLACE = 1
		SYSD_UNIT_MODE_ISOLATE = 2
		SYSD_UNIT_MODE_IGN_DEPS = 3
		SYSD_UNIT_MODE_IGN_REQS = 4

	cdef int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated, cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
	cdef cgroup_create_scope_user_slice(char *scope_name, int delegated, cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
	int cgroup_is_delegated(char *path)
	cdef int cgroup_is_delegated_pid(pid_t *target)
