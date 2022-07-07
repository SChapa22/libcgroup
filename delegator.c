#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <systemd/sd-bus.h>

//static const char * CGROUP =
static const char *SCOPE = "delegated.scope";

int main (int argc, char* argv[]){
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *m = NULL, *reply = NULL;
//	const char *object = NULL;
	sd_bus *bus = NULL;
//	sd_bus_slot *slot = NULL;
	int ret;

	ret = sd_bus_open_user(&bus);
	if (ret<0){fprintf(stderr, "sd_bus_open_system failed: %d:%s\n", ret, strerror(-ret)); goto out;}

//	ret = sd_bus_request_name(bus, "org.freedesktop.systemd1", 0);
//	if (ret<0){fprintf(stderr, "sd_bus_request_name failed: %d:%s\n", ret, strerror(-ret)); goto out;}

//	ret = sd_bus_release_name(bus, "org.freedesktop.systemd1");
//	if (ret<0){fprintf(stderr, "sd_bus_release_name failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = sd_bus_call_method(bus, "org.freedesktop.systemd1", 
		"/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
		"StartTransientUnit", &error, &reply, "ssa(sv)a(sa(sv))",
		"libcgroup-delegated.scope", "fail", 2,
		"PIDs", "au", 1, (uint32_t)getpid(),
//		"Delegate", "b", 1,
//		"Slice", "s", "libcgroup-delegated.slice", 0);
		"Scope", "s", "libcgroup-delegated.scope", 0);
	if (ret<0){fprintf(stderr, "sd_bus_call_method failed: %d:%s\n", ret, strerror(-ret)); goto out;}

//	ret = sd_bus_message_append(m, "(sv)", "Delegate", "b", 1);
//	if (ret<0){fprintf(stderr, "sd_bus_message_append failed: %d:%s\n", ret, strerror(-ret)); goto out;}

	ret = 0;
	fprintf(stdout, "PID %d running in delegated cgroup\n", getpid());
	fflush(stdout);
	sleep(100000);

//	fprintf(stdout, "%s\n", object);
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
