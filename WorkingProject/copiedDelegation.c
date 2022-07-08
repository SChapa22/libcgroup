#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <systemd/sd-bus.h>

#define SYSTEMD_CGROUP_CONTROLLER "_systemd"

//static const char * CGROUP =
static const char *SCOPE = "delegated.scope";

int main (int argc, char* argv[]){
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *m = NULL, *reply = NULL;
	sd_bus *bus = NULL;
	const char *object = NULL;
	char *cgroup = NULL;
	int ret;

//	ret = sd_bus_open_user(&bus); // Don't use this, it won't delegate
	ret = sd_bus_open_system(&bus); // Requires use of sudo when running .o
	if (ret<0){fprintf(stderr, "sd_bus_open_system failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_new_method_call(bus, &m, "org.freedesktop.systemd1", 
		"/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
		 "StartTransientUnit");
	if (ret<0){fprintf(stderr, "sd_bus_message_new_method_call failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_append(m, "ss", SCOPE, "fail");
	if (ret<0){fprintf(stderr, "1st sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_open_container(m, 'a', "(sv)");
	if (ret<0){fprintf(stderr, "sd_bus_message_open_container failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_append(m, "(sv)", "PIDs", "au", 1, (uint32_t) getpid());
	if (ret<0){fprintf(stderr, "2nd sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_append(m, "(sv)", "Delegate", "b", 1);
	if (ret<0){fprintf(stderr, "3rd sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_append(m, "(sv)", "CollectMode", "s", "inactive-or-failed");
	if (ret<0){fprintf(stderr, "4th sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_close_container(m);
	if (ret<0){fprintf(stderr, "sd_bus_message_close failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_append(m, "a(sa(sv))", 0);
	if (ret<0){fprintf(stderr, "5th sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_call(bus, m, 0, &error, &reply);
	if (ret<0){fprintf(stderr, "sd_bus_call failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_message_read(reply, "o", &object);
	if (ret<0){fprintf(stderr, "sd_bus_message_reply failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	fprintf(stdout, "%s  --  %d\n", object, getpid());
/*
	ret = cg_pid_get_path(SYSTEMD_CGROUP_CONTROLLER, 0, &cgroup);
	if (ret<0){fprintf(stderr, "cg_pid_get_path failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = cg_get_xattr_bool(SYSTEMD_CGROUP_CONTROLLER, cgroup, "trusted.delegate");
	if (ret==0){fprintf(stderr, "Group is delegated, but to someone else.\n"); goto out;}
	if (ret<0){fprintf(stderr, "cg_get_xattr_bool failed: %d:%s\n", ret, strerror(-ret)); goto out;}
*/
	fprintf(stdout, "Sucess!\n");
	fflush(stdout);
	sleep (100000);

out:
#if 0
	if(error != SD_BUS_ERROR_NULL)
		sd_bus_error_free(&error);
#endif
	if(m)
		sd_bus_message_unref(m);
	if(bus)
		sd_bus_unref(bus);
	fflush(stderr);
	return ret;
}
