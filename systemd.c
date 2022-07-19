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

#define CG_SYSTEMD_USER_SLICE_NAME "user.slice"

enum SYSD_UNIT_MODE {SYSD_UNIT_MODE_FAIL, SYSD_UNIT_MODE_REPLACE, SYSD_UNIT_MODE_ISOLATE,
					SYSD_UNIT_MODE_IGN_DEPS, SYSD_UNIT_MODE_IGN_REQS};

char SYSD_UNIT_MODE_NAMES[5][20] = {"fail", "replace", "isolate","ignore-dependencies", "ignore-requirements"};

void permanent_sleep(){
	while (1){
		sleep(10000);
	}
}

//@todo Improve error messages for create_scope_and_slice & create_scope_user_slice; look at cg_delegate.py

int cgroup_create_scope_and_slice(char *scope_name, char *slice_name, int delegated, enum SYSD_UNIT_MODE mode){

	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *m = NULL, *reply = NULL;
	char *object = NULL;
	sd_bus *bus = NULL;
	int ret = 0;
	
	int child_pid = fork();

	if (child_pid < 0){
		//No child process created
		fprintf(stderr, "fork() failed: %d:%s\n", child_pid, strerror(-child_pid));
	}
	else if (child_pid == 0){
	//This is the child process, sleep forever
		permanent_sleep();
	}
	else{
		//This is the parent process, move child process to the desired slice/scope
		
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

		ret = sd_bus_message_append(m, "ss", scope_name, SYSD_UNIT_MODE_NAMES[mode]);
		if (ret<0){
			fprintf(stderr, "1st sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret));
			goto out;
		}

		ret = sd_bus_message_open_container(m, 'a', "(sv)");
		if (ret<0)
			{fprintf(stderr, "sd_bus_message_open_container failed: %d:%s\n", ret, strerror(-ret));
			goto out;
		}

		ret = sd_bus_message_append(m, "(sv)", "PIDs", "au", 1, (uint32_t) child_pid);
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
		
		// Parent thread will return PID of child thread
		ret = child_pid;
	}

	//@todo Does object need to be freed?
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

int cgroup_create_scope_user_slice(char *scope_name, int delegated, enum SYSD_UNIT_MODE mode){

	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *m = NULL, *reply = NULL;
	char *object = NULL;
	sd_bus *bus = NULL;
	int ret = 0;
	
	int child_pid = fork();

	if(child_pid < 0){ //No child process created
		fprintf(stderr, "fork() failed: %d:%s\n", child_pid, strerror(-child_pid));
	}
	else if (child_pid == 0){ //This is the child process, sleep forever
		permanent_sleep();
	}
	else{ //This is the parent process, move child process to the desired slice/scope

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

		ret = sd_bus_message_append(m, "ss", scope_name, SYSD_UNIT_MODE_NAMES[mode]);
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

		ret = sd_bus_message_append(m, "(sv)", "Slice", "s", CG_SYSTEMD_USER_SLICE_NAME);
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
		
		// Parent thread will return PID of child thread
		ret = child_pid;
	}
	
	//@todo Does object need to be freed?
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

int cgroup_is_delegated(char *scope_name){

	// Use system call to check if the scope has been delegated--name must match scope/slice defined at start of program
	char result[FILENAME_MAX];
	int ret = 0;

	ret = getxattr(scope_name, "trusted.delegate", result, FILENAME_MAX);
	if (ret<0){
		fprintf(stderr, "getxattr failed: %d:%s\n", ret, strerror(-ret));
		fflush(stderr);
	}
	else if (strncmp(result, "1", 1)==0){
		fprintf(stdout, "Successfully Delegated Scope!\n");
		ret=1;
	}

	return ret;
}

int main (int argc, char* argv[]){
	
	enum SYSD_UNIT_MODE mode = SYSD_UNIT_MODE_FAIL;
	char result[FILENAME_MAX];
	strcpy(result, SYSD_UNIT_MODE_NAMES[mode]);
	fprintf(stdout, "%s\n", result);
	
	int ret = 0;
	ret = cgroup_create_scope_and_slice("testing-delegated.scope","testing-delegated.slice", 1, mode);
	fprintf(stdout, "Create Scope And Slice: %d\n", ret);
	ret = cgroup_is_delegated("/sys/fs/cgroup/unified/testing.slice/testing-delegated.slice/testing-delegated.scope");
	fprintf(stdout, "Check Delegated: %d\n", ret);
	sleep(100); // Allow time to check manually

	ret = cgroup_create_scope_user_slice("user-delegated.scope", 1, mode);
	fprintf(stdout, "Create Scope User Slice: %d\n", ret);
	ret = cgroup_is_delegated("/sys/fs/cgroup/unified/user.slice/user-delegated.scope");
	fprintf(stdout, "Check Delegated: %d\n", ret);
	fprintf(stdout, "%d\n", getpid());

//	fprintf(stdout, "Sucess!\n");
	fflush(stdout);
	fflush(stderr);
	sleep(100); // Allow time to check manually

}
