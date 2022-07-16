#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <systemd/sd-bus.h>

#define CG_SYSTEMD_USER_SLICE_NAME "user.slice"

#define CG_SYSTEMD_UNIT_MODE_FAIL "fail"
#define CG_SYSTEMD_UNIT_MODE_REPLACE "replace"
#define CG_SYSTEMD_UNIT_MODE_ISOLATE "isolate"
#define CG_SYSTEMD_UNIT_MODE_IGN_DEPENDENCIES "ignore-dependencies"
#define CG_SYSTEMD_UNIT_MODE_IGN_REQUIREMENTS "ignore-requirements"

int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated, char* mode){

	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *m = NULL, *reply = NULL;
	char *object = NULL, *cgroup = NULL;
	sd_bus *bus = NULL;
	int ret = 0;

	ret = sd_bus_open_system(&bus);
	if (ret<0){
		fprintf(stderr, "sd_bus_open_system failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_new_method_call(bus, &m, "org.freedesktop.systemd1",
		"/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
		 "StartTransientUnit");
	if (ret<0){
		fprintf(stderr, "sd_bus_message_new_method_call failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_append(m, "ss", scope_name, mode);
	if (ret<0){
		fprintf(stderr, "1st sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_open_container(m, 'a', "(sv)");
	if (ret<0)
		{fprintf(stderr, "sd_bus_message_open_container failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_append(m, "(sv)", "PIDs", "au", 1, (uint32_t) getpid());
	if (ret<0){
		fprintf(stderr, "2nd sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	if(delegated == 1){
		ret = sd_bus_message_append(m, "(sv)", "Delegate", "b", 1);
		if (ret<0){
			fprintf(stderr, "3rd sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
			goto out;
		}
	}

	ret = sd_bus_message_append(m, "(sv)", "Slice", "s", slice_name);
	if (ret<0){
		fprintf(stderr, "4th sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_close_container(m);
	if (ret<0){
		fprintf(stderr, "sd_bus_message_close failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_append(m, "a(sa(sv))", 0);
	if (ret<0){
		fprintf(stderr, "5th sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_call(bus, m, 0, &error, &reply);
	if (ret<0){
		fprintf(stderr, "sd_bus_call failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_read(reply, "o", &object);
	if (ret<0){
		fprintf(stderr, "sd_bus_message_reply failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	out:
	if(m != NULL){
		sd_bus_message_unref(m);
	}
	if(reply != NULL){
		sd_bus_message_unref(reply);
	}
	if(bus != NULL){
		sd_bus_unref(bus);
	}

	return ret;
}

int cgroup_create_scope_user_slice(char *scope_name, int delegated, char *mode){

	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *m = NULL, *reply = NULL;
	char *object = NULL, *cgroup = NULL;
	sd_bus *bus = NULL;
	int ret = 0;

	ret = sd_bus_open_system(&bus);
	if (ret<0){
		fprintf(stderr, "sd_bus_open_system failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_new_method_call(bus, &m, "org.freedesktop.systemd1",
		"/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
		 "StartTransientUnit");
	if (ret<0){
		fprintf(stderr, "sd_bus_message_new_method_call failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_append(m, "ss", scope_name, mode);
	if (ret<0){
		fprintf(stderr, "1st sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_open_container(m, 'a', "(sv)");
	if (ret<0)
		{fprintf(stderr, "sd_bus_message_open_container failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_append(m, "(sv)", "PIDs", "au", 1, (uint32_t) getpid());
	if (ret<0){
		fprintf(stderr, "2nd sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	if(delegated == 1){
		ret = sd_bus_message_append(m, "(sv)", "Delegate", "b", 1);
		if (ret<0){
			fprintf(stderr, "3rd sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
			goto out;
		}
	}

	ret = sd_bus_message_append(m, "(sv)", "Slice", "s", "user.slice");
	if (ret<0){
		fprintf(stderr, "4th sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_close_container(m);
	if (ret<0){
		fprintf(stderr, "sd_bus_message_close failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_append(m, "a(sa(sv))", 0);
	if (ret<0){
		fprintf(stderr, "5th sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_call(bus, m, 0, &error, &reply);
	if (ret<0){
		fprintf(stderr, "sd_bus_call failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	ret = sd_bus_message_read(reply, "o", &object);
	if (ret<0){
		fprintf(stderr, "sd_bus_message_reply failed: %d:%s\n", ret, strerror(-ret));
		goto out;
	}

	out:
	if(m != NULL){
		sd_bus_message_unref(m);
	}
	if(reply != NULL){
		sd_bus_message_unref(reply);
	}
	if(bus != NULL){
		sd_bus_unref(bus);
	}

	return ret;
}

int cgroup_is_delegated(char *path){

	// Use system call to check if the scope has been delegated--name must match scope/slice defined at start of program
	char result[FILENAME_MAX];
	int ret = 0;

	ret = getxattr(path, "trusted.delegate", result, FILENAME_MAX);
	if (ret<0){
		fprintf(stderr, "getxattr failed: %d:%s\n", ret, strerror(-ret));
		fflush(stderr);
	}
	else if (strcmp(result, "1")==0){
		fprintf(stdout, "Successfully Delegated Scope!\n");
		ret=1;
	}

	return ret;
}

int main (int argc, char* argv[]){
	int ret = 0;
	ret = cgroup_create_scope_and_slice("testing-delegated.scope","testing-delegated.slice", 1, CG_SYSTEMD_UNIT_MODE_FAIL);
	fprintf(stdout, "Create Scope And Slice: %d\n", ret);
	ret = cgroup_is_delegated("/sys/fs/cgroup/unified/testing.slice/testing-delegated.slice/testing-delegated.scope");
	fprintf(stdout, "Check Delegated: %d\n", ret);
	sleep(100); // Allow time to check manually

	ret = cgroup_create_scope_user_slice("user-delegated.scope", 1, CG_SYSTEMD_UNIT_MODE_FAIL);
	fprintf(stdout, "Create Scope User Slice: %d\n", ret);
	ret = cgroup_is_delegated("/sys/fs/cgroup/unified/user.slice/user-delegated.scope");
	fprintf(stdout, "Check Delegated: %d\n", ret);
	fprintf(stdout, "%d\n", getpid());

//	fprintf(stdout, "Sucess!\n");
	fflush(stdout);
	fflush(stderr);
	sleep(100); // Allow time to check manually

}
