import dbus
import os
import time
import xattr

from dbus.types import UInt32

def create_delegated_scope(scopeName, sliceName):
	try:
		bus = dbus.SystemBus()
		sysD1 = bus.get_object("org.freedesktop.systemd1", "/org/freedesktop/systemd1")
		manager = dbus.Interface(sysD1, "org.freedesktop.systemd1.Manager")
	except Exception as e:
		print(type(e), e)
	try:
		manager.StartTransientUnit(scopeName, "fail",
			[("PIDs", [UInt32(os.getpid())]),
			("Delegate", True),
			("Slice", sliceName)],
			[])
	except Exception as e:
		print(type(e), e)

def check_delegated(pathName):
	ret =int.from_bytes(xattr.getxattr(pathName, "trusted.delegate"), "big")
	if (ret == ord('1')):
		print("Successfully Delegated scope")
	return ret

if __name__ == "__main__":
	print("Running")
	create_delegated_scope("testing-delegated.scope", "testing-delegated.slice")
	check_delegated("/sys/fs/cgroup/unified/testing.slice/testing-delegated.slice/testing-delegated.scope")
	print("Success!")
	time.sleep(100)
