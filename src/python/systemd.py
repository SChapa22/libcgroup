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

CG_SYSTEMD_UNIT_MODE_FAIL = "fail"
CG_SYSTEMD_UNIT_MODE_REPLACE = "replace"
CG_SYSTEMD_UNIT_MODE_ISOLATE = "isolate"
CG_SYSTEMD_UNIT_MODE_IGN_DEPENDENCIES = "ignore-dependencies"
CG_SYSTEMD_UNIT_MODE_IGN_REQUIREMENTS = "ignore-requirements"

cdef extern from "systemd.h":
	int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated, char* mode)
	cgroup_create_scope_user_slice(char *scope_name, int delegated, char *mode)
	int cgroup_is_delegated(char *path)
