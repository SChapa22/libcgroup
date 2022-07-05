#include <errno.h>
#include <fcntl.h>
#include <systemd/sd-bus.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//static const char * CGROUP = "
static const char *SCOPE = "delegated.scope"


// struct and functions for proof this is doing something
typedef struct CounterData {
	uint32_t count;
} CounterData;

int methodIncrement(sd_bus_message *message, void *userData,
	sd_bus_error *error){
	int err = 0;
	uint32_t parameter = 0;
	err = sd_bus_message_read(message, "u", &parameter);
	struct CounterData *counterData = (CounterData *)(userData);
	counterData->count += parameter;
	return sd_bus_reply_method_return(message, "u", counterData->count);
}

int propertyCounterGet(sd_bus *bus, const char *path, const char *interface,
	const char *property, sd_bus_message *reply,
	void *userData, sd_bus_error * err){
	struct CounterData *counterData = (CounterData *)(userData);
	return sd_bus_message_append(reply, "u", counterData->count);
}

int propertyCounterSet(sd_bus *bus, const char *path, const char *interface,
	const char *property, sd_bus_message *message,
	void *userData, sd_bus_error * err){
	int ret = 0;
	uint32_t parameter = 0;
	ret = sd_bus_message_read(message, "u", &parameter);
	struct CounterData *counterData = (CounterData *)(userData);
	counterData->count = parameter;
}

static const sd_bus_vtable counter_vtable[] = {
	SD_BUS_VTABLE_START(0),
	SD_BUS_WRITABLE_PROPERTY("Count", "u", propertyCounterGet, propertyCounterSet, 0, 0),
	SD_BUS_METHOD("IncrementBy", "u", "u", methodIncrement, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END
};

int main(int argc, char* argv[])
{
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *m = NULL, *reply = NULL;
	const char *object = NULL;
	sd_bus *bus = NULL;
	sd_bus_slot *slot = NULL;
	struct CounterData *counterData;
//	counterData->count = 0;
	int ret = 0;
	
	ret = sd_bus_default_system(&bus);
	if (ret < 0) {
		fprintf(stderr, "sd_bus_default_system failed: %d\n", ret);
		goto out;
	}
	
	
	ret = sd_bus_add_object_vtable(bus, &slot,
		"/test/delegator/StandAlone",
		"test.delegator.StandAlone", counter_vtable, &counterData);
	if (ret < 0) {
		fprintf(stderr, "sd_bus_add_object_vtable failed: %d\n", ret);
		goto out;
	}
	ret = sd_bus_request_name(bus, "test.delegator.StandAlone", 0);
	if (ret < 0) {
		//fprintf(stderr, "sd_bus_request_name failed: %d\n", ret);
		fprintf(stderr, "%s\n", strerror(-ret));
		goto out;
	}
	
	ret = sd_bus_message_new_method_call(bus, &m,
		"test.delegator.StandAlone",
		"",
		"test.delegator.StandAlone",
		"StartTransientUnit");
	if (ret < 0) {
		fprintf(stderr, "StartTransientUnit() failed: %d\n", ret);
		goto out;	
	}
	
	ret = sd_bus_message_append(m, "ss", SCOPE, "fail");
	if (ret < 0) {
		fprintf(stderr, "append1 failed: %d\n", ret);
		goto out;
	}
	
	ret = sd_bus_message_append(m, "(sv)", "Delegate", "b", 1);
	if (ret < 0) {
		fprintf(stderr, "append2 failed: %d\n", ret);
		goto out;
	}
	
	ret = sd_bus_call(bus, m, 0, &error, &reply);
	if (ret < 0) {
		fprintf(stderr, "sd_bus_call failed: %d\n", ret);
		fprintf(stderr, "error = %s\n", error.message);
		goto out;
	}
	
	ret = sd_bus_message_read(reply, "o", &object);
	if (ret < 0) {
		fprintf(stderr, "sd_bus_message_read failed: %d\n", ret);
		goto out;
	}
	
	fprintf(stdout, "%s\n", object);
	
out:
#if 0
	if (error != SD_BUS_ERROR_NULL)
		sd_bus_error_free(&error);
#endif
	if (m)
		sd_bus_message_unref(m);
	if (bus)
		sd_bus_unref(bus);
	fflush(stderr);
	return ret;
}
