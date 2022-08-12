# SPDX-License-Identifier: LGPL-2.1-only
#
# Libcgroup Python Bindings
#
# Copyright (c) 2021-2022 Oracle and/or its affiliates.
# Author: Tom Hromatka <tom.hromatka@oracle.com>
#

# cython: language_level = 3str

from posix.types cimport pid_t

cdef extern from "libcgroup.h":
    cdef struct cgroup:
        pass

    cdef struct cgroup_controller:
        pass

    cdef enum cg_version_t:
        CGROUP_UNK
        CGROUP_V1
        CGROUP_V2
        CGROUP_DISK

    cdef struct cgroup_library_version:
        unsigned int major
        unsigned int minor
        unsigned int release

    int cgroup_init()
    const cgroup_library_version * cgroup_version()

    cgroup *cgroup_new_cgroup(const char *name)
    int cgroup_create_cgroup(cgroup *cg, int ignore_ownership)
    int cgroup_convert_cgroup(cgroup *out_cg, cg_version_t out_version,
                              cgroup  *in_cg, cg_version_t in_version)
    void cgroup_free(cgroup **cg)

    cgroup_controller *cgroup_add_controller(cgroup *cg, const char *name)
    cgroup_controller *cgroup_get_controller(cgroup *cg, const char *name)

    int cgroup_add_value_string(cgroup_controller *cgc, const char *name,
                                const char *value)
    int cgroup_get_value_string(cgroup_controller *cgc, const char *name,
                                char **value)
    char *cgroup_get_value_name(cgroup_controller *cgc, int index)
    int cgroup_get_value_name_count(cgroup_controller *cgc)

    int cgroup_cgxget(cgroup ** cg, cg_version_t version,
                      bint ignore_unmappable)

    int cgroup_cgxset(const cgroup * const cg, cg_version_t version,
                      bint ignore_unmappable)

    int cgroup_list_mount_points(const cg_version_t cgrp_version,
                                 char ***mount_paths)

# Equivalent to the defined variables in systemd.c
cdef char *CG_SYSTEMD_USER_SLICE_NAME = "user.slice"
cdef char *CG_SCOPE_TREE_ROOT = "/sys/fs/cgroup/"
cdef char *CG_SCOPE_TREE_ROOT_UNI = "/sys/fs/cgroup/unified/"
cdef char *CG_SCOPE_TREE_ROOT_SYSD = "/sys/fs/cgroup/systemd/"

cdef extern from "systemd.h":

        cdef enum cgroup_sysd_unit_mode:
                SYSD_UNIT_MODE_FAIL = 0
                SYSD_UNIT_MODE_REPLACE = 1
                SYSD_UNIT_MODE_ISOLATE = 2
                SYSD_UNIT_MODE_IGN_DEPS = 3
                SYSD_UNIT_MODE_IGN_REQS = 4

        cdef int cgroup_create_scope_and_slice(char *scope_name,
               char *slice_name, int delegated, cgroup_sysd_unit_mode mode,
               pid_t * const sleeper_pid)
        cdef cgroup_create_scope_user_slice(char *scope_name, int delegated,
               cgroup_sysd_unit_mode mode, pid_t * const sleeper_pid)
        int cgroup_is_delegated(char *path)
        cdef int cgroup_is_delegated_pid(pid_t *target)



# vim: set et ts=4 sw=4:
