File Structure:

/usr/share/dbus-1/system-services/test.delegator.StandAlone.d-bus.service
/etc/dbus-1/system.d/test.delegator.StandAlone.conf
/etc/systemd/system/test.delegator.service

From what I understand, systemD will figure out these files are there and the system will be set up by as soon as they are.
For proof, run busctl to check that "test.delegator.StandAlone" exists in the list of names. It has an "activatable" connection, but no PID or Process/User names (which would indicate it was active).
LibCGroups does NOT automatically set up the service when a file is added (this being "test.delegator.service"), so run "sudo systemctl daemon-reload". This file seems to do nothing, whether or not it's there.
Delegate.c (and it's corresponding .o) are in "/home/ubuntu", as specified in both .service files. If it is moved, the files must change to account for the new location. Always use complete paths, rather than relative paths (which don't work, at all) in these files.

Compile (from folder containing delegate.c):
gcc delegate.c -o delegate.o -lsystemd

Main References (if y'all want more, just ask; these are the ones that summarize/are most useful):
https://gitlab.com/franks_reich/systemd-by-example/-/tree/master/
https://techbase.kde.org/Development/Tutorials/PolicyKit/Helper_HowTo#The_DBus_policy_file