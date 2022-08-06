// SPDX-License-Identifier: LGPL-2.1-only
#include <libcgroup.h>
#include <systemd/sd-login.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define CGROUP_ROOT_FOLDER "/sys/fs/cgroup/unified"

int main(void)
{
	static const char delegated_path[] = "/sys/fs/cgroup/unified/testing.slice/"
					"testing-delegated.slice/testing-delegated.scope";
	static const char delegated_path2[] = "/sys/fs/cgroup/unified/testing.slice/"
					"testing-delegated.slice/testing-delegated2.scope";
	pid_t *sleeper = malloc(sizeof(pid_t)), *sleeper2 = malloc(sizeof(pid_t));
	char *found_path;
	int error;

	*sleeper = -1;
	*sleeper2 = -1;

	error = cgroup_init();
	if (error) {
		printf("cgroup_init failed with %s\n", cgroup_strerror(error));
		exit(1);
	}

/*
 * Correct Calls
 */
	printf("\n\nThe following attempt to create delegated scopes.\n\n");

	printf("Attempting create_scope_and_slice, _FAIL\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		 1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Fail: %d\n", error);
/*
 * The following is an example for wanting to use the cgroup_is_delegated function, where the rest
 * of this file uses the wrapper cgroup_is_delegated_pid function.
 *
	error = sd_pid_get_cgroup(*sleeper, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper, &found_path);
	}
	error = access(delegated_path, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path, F_OK);
	strncat(built_path, CGROUP_ROOT_FOLDER, FILENAME_MAX);
	strncat(built_path, found_path, FILENAME_MAX);
	error = cgroup_is_delegated(built_path);
 */

	error = cgroup_is_delegated_pid(sleeper);
	if (error != 1)
		printf("Scope is delegated--Fail: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	printf("Attempting create_scope_and_slice, _REPLACE\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_REPLACE, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Replace: %d\n", error);
	error = cgroup_is_delegated_pid(sleeper);
	if (error != 1)
		printf("Scope is delegated--Replace: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	printf("Attempting create_scope_and_slice, _IGN_DEPS\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_IGN_DEPS, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Ignore Dependencies: %d\n", error);
	error = cgroup_is_delegated_pid(sleeper);
	if (error != 1)
		printf("Scope is delegated--Ignore Dependencies: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	printf("Attempting create_scope_and_slice, _IGN_REQS\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_IGN_REQS, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Ignore Requirements: %d\n", error);
	error = cgroup_is_delegated_pid(sleeper);
	if (error != 1)
		printf("Scope is delegated--Ignore Requirements: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);


	printf("\n\nThe following attempt to create scopes without delegation.\n\n");

	printf("Attempting create_scope_and_slice, _FAIL\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		 0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Fail: %d\n", error);
	error = cgroup_is_delegated_pid(sleeper);
	if (error != 0)
		printf("Scope is delegated--Fail: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	printf("Attempting create_scope_and_slice, _REPLACE\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_REPLACE, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Replace: %d\n", error);
	error = cgroup_is_delegated_pid(sleeper);
	if (error != 0)
		printf("Scope is delegated--Replace: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	printf("Attempting create_scope_and_slice, _IGN_DEPS\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_IGN_DEPS, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Ignore Dependencies: %d\n", error);
	error = cgroup_is_delegated_pid(sleeper);
	if (error != 0)
		printf("Scope is delegated--Ignore Dependencies: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	printf("Attempting create_scope_and_slice, _IGN_REQS\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_IGN_REQS, sleeper);
	if (error != 1)
		printf("Create Scope and Slice--Ignore Requirements: %d\n", error);
	error = cgroup_is_delegated_pid(sleeper);
	if (error != 0)
		printf("Scope is delegated--Ignore Requirements: %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);


/*
 * Potentially Incorrect Calls
 */

	printf("\n\nThe following attempt double-calls with delegation.\n\n");

	//Double-call, same scope and same slice
	printf("Attempting double-call: same scope and slice names--2nd call Fails.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create scope_and_slice failed--1st Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper, &found_path);
	}
	error = access(delegated_path, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path, F_OK);
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper2);
	if (error != ECGSYSDSLICESCOPEEXISTS)
		printf("ERROR: Does not recognize incorrect scope name. Returns %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	if (*sleeper2 != -1) {
		kill(*sleeper2, SIGSEGV);
		*sleeper2 = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	//Double-call, different scope and same slice
	printf("Attempting double-call: different scope and same slice names.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create scope_and_slice failed--1st Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper, &found_path);
	}
	error = access(delegated_path, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path, F_OK);
	error = cgroup_create_scope_and_slice("testing-delegated2.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper2);
	if (error != 1)
		printf("Create scope_and_slice failed--2nd Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper2, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper2, &found_path);
	}
	error = access(delegated_path2, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path2, F_OK);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	if (*sleeper2 != -1) {
		kill(*sleeper2, SIGSEGV);
		*sleeper2 = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);
	error = access(delegated_path2, F_OK);
	while (error == 0)
		error = access(delegated_path2, F_OK);

	//Double-call, same scope and different slice
	printf("Attempting double-call: same scope and different slice names--2nd call Fails.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create scope_and_slice failed--1st Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper, &found_path);
	}
	error = access(delegated_path, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path, F_OK);
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "delegated-testing.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper2);
	if (error != ECGSYSDSLICESCOPEEXISTS)
		printf("ERROR: Does not recognize incorrect scope name. Returns %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	if (*sleeper2 != -1) {
		kill(*sleeper2, SIGSEGV);
		*sleeper2 = -1;
	}
	error = access(delegated_path, F_OK);
	while (error != -1)
		error = access(delegated_path, F_OK);

	printf("\n\nThe following attempt double-calls without delegation.\n\n");

	//Double-call, same scope and same slice
	printf("Attempting double-call: same scope and slice names--2nd call Fails.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create scope_and_slice failed--1st Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper, &found_path);
	}
	error = access(delegated_path, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path, F_OK);
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper2);
	if (error != ECGSYSDSLICESCOPEEXISTS)
		printf("ERROR: Does not recognize incorrect scope name. Returns %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	if (*sleeper2 != -1) {
		kill(*sleeper2, SIGSEGV);
		*sleeper2 = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);

	//Double-call, different scope and same slice
	printf("Attempting double-call: different scope and same slice names.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create scope_and_slice failed--1st Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper, &found_path);
	}
	error = access(delegated_path, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path, F_OK);
	error = cgroup_create_scope_and_slice("testing-delegated2.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper2);
	if (error != 1)
		printf("Create scope_and_slice failed--2nd Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper2, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper2, &found_path);
	}
	error = access(delegated_path2, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path2, F_OK);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	if (*sleeper2 != -1) {
		kill(*sleeper2, SIGSEGV);
		*sleeper2 = -1;
	}
	error = access(delegated_path, F_OK);
	while (error == 0)
		error = access(delegated_path, F_OK);
	error = access(delegated_path2, F_OK);
	while (error == 0)
		error = access(delegated_path2, F_OK);

	//Double-call, same scope and different slice
	printf("Attempting double-call: same scope and different slice names--2nd call Fails.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 1)
		printf("Create scope_and_slice failed--1st Double-call: %d\n", error);
	error = sd_pid_get_cgroup(*sleeper, &found_path);
	while (error < 0) {
		if (error != ESRCH)
			printf("get_cgroup: %d\n", error);
		error = sd_pid_get_cgroup(*sleeper, &found_path);
	}
	error = access(delegated_path, F_OK);
	if (error < 0)
		printf("get_cgroup loop insufficient: %d\n", error);
	while (error < 0)
		error = access(delegated_path, F_OK);
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "delegated-testing.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper2);
	if (error != ECGSYSDSLICESCOPEEXISTS)
		printf("ERROR: Does not recognize incorrect scope name. Returns %d\n", error);
	if (*sleeper != -1) {
		kill(*sleeper, SIGSEGV);
		*sleeper = -1;
	}
	if (*sleeper2 != -1) {
		kill(*sleeper2, SIGSEGV);
		*sleeper2 = -1;
	}
	error = access(delegated_path, F_OK);
	while (error != -1)
		error = access(delegated_path, F_OK);


	printf("\n\nThe following attempt calls with incorrect names, delegated.\n\n");

	//Call for a service rather than a scope
	printf("Attempting to call for a service rather than a scope--Error 30.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.service",
		"testing-delegated.slice", 1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 30)
		printf("Create scope_and_slice failed--Service rather than scope: %d\n", error);

	//Call for a service rather than a slice
	printf("Attempting to call for a service rather than a scope--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope",
		"testing-delegated.service", 1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Service rather than slice: %d\n", error);

	//Call with incorrect slice name
	printf("Attempting to call for a slice named \".nope\"--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope",
		"testing-delegated.nope", 1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Incorrect slice ending: %d\n", error);

	//Call with incorrect slice name
	printf("Attempting to call for a slice ommitting the \".slice\" ending--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Ommitting slice ending: %d\n", error);

	//Call with incorrect slice name
	printf("Attempting to call for a slice named \".scope\"--Error 22.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.scope",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Scope rather than slice: %d\n", error);

/*
 * This code will cause an error 5 that can only be cleared by rebooting the machine.
 * It appears to come from systemd's operation, as the error will be found by any
 * subsequent call to sd_bus_call, rather than ocurring here.
 *
 *
	//Call with incorrect scope name
	printf("Attempting to call for a scope named \".slice\"--Error 30\n");
	error = cgroup_create_scope_and_slice("testing-delegated.slice", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 30)
		printf("Create scope_and_slice failed--Slice rather than scope: %d\n", error);
 */

	//Call with incorrect scope name
	printf("Attempting to call for a scope named \".nope\"--Error 22.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.nope", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Incorrect scope ending: %d\n", error);

	//Call with incorrect scope name
	printf("Attempting to call for a scope ommitting the \".scope\" ending--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated", "testing-delegated.slice",
		1, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Ommitting scope ending: %d\n", error);


	printf("\n\nThe following attempt calls with incorrect names, without delegation.\n\n");


	//Call for a service rather than a scope
	printf("Attempting to call for a service rather than a scope--Error 30.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.service",
		"testing-delegated.slice", 0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 30)
		printf("Create scope_and_slice failed--Service rather than scope: %d\n", error);

	//Call for a service rather than a slice
	printf("Attempting to call for a service rather than a scope--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope",
		"testing-delegated.service", 0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Service rather than slice: %d\n", error);

	//Call with incorrect slice name
	printf("Attempting to call for a slice named \".nope\"--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope",
		"testing-delegated.nope", 0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Incorrect slice ending: %d\n", error);

	//Call with incorrect slice name
	printf("Attempting to call for a slice ommitting the \".slice\" ending--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Ommitting slice ending: %d\n", error);

	//Call with incorrect slice name
	printf("Attempting to call for a slice named \".scope\"--Error 22.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.scope", "testing-delegated.scope",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Scope rather than slice: %d\n", error);

/*
 * This code will cause an error 5 that can only be cleared by rebooting the machine.
 * It appears to come from systemd's operation, as the error will be found by any
 * subsequent call to sd_bus_call, rather than ocurring here.
 *
 *
	//Call with incorrect scope name
	printf("Attempting to call for a scope named \".slice\"--Error 30\n");
	error = cgroup_create_scope_and_slice("testing-delegated.slice", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 30)
		printf("Create scope_and_slice failed--Slice rather than scope: %d\n", error);
 */

	//Call with incorrect scope name
	printf("Attempting to call for a scope named \".nope\"--Error 22.\n");
	error = cgroup_create_scope_and_slice("testing-delegated.nope", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Incorrect scope ending: %d\n", error);

	//Call with incorrect scope name
	printf("Attempting to call for a scope ommitting the \".scope\" ending--Error 22\n");
	error = cgroup_create_scope_and_slice("testing-delegated", "testing-delegated.slice",
		0, SYSD_UNIT_MODE_FAIL, sleeper);
	if (error != 22)
		printf("Create scope_and_slice failed--Ommitting scope ending: %d\n", error);

	free(sleeper);

	return 0;
}
