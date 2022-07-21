// SPDX-License-Identifier: LGPL-2.1-only
#include <libcgroup.h>

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	struct cgroup_mount_point info;
	void *handle;
	int error;

	error = cgroup_init();
	if (error) {
		printf("cgroup_init failed with %s\n", cgroup_strerror(error));
		exit(1);
	}

	error = cgroup_get_controller_begin(&handle, &info);
	while (error != ECGEOF) {
		printf("Controller %s is mounted at %s\n",
		       info.name, info.path);
		error = cgroup_get_controller_next(&handle, &info);
		if (error && error != ECGEOF) {
			printf("cgroup_get_controller_next failed with %s",
			       cgroup_strerror(error));
			exit(1);
		}
	}

	error = cgroup_get_controller_end(&handle);
/*
	enum cg_sysd_unit_mode mode = CG_SYSD_UNIT_MODE_FAIL;
/*
	error = cgroup_create_scope_and_slice("testing-delegated.scope","testing-delegated.slice", 1, mode);
	fprintf(stdout, "Create Scope And Slice: %d\n", error);
	error = cgroup_is_delegated("/sys/fs/cgroup/unified/testing.slice/testing-delegated.slice/testing-delegated.scope");
	fprintf(stdout, "Check Delegated: %d\n", error);
	sleep(100); // Allow time to check manually
*/
	return 0;
}
