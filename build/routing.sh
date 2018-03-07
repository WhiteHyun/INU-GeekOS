#!/bin/sh -x

# Computer 1
#
# Connected to VLAN0 and VLAN1
#
# OPTS=-std-vga
OPTS="-curses"
echo "Starting computer 1"
screen /usr/bin/qemu $OPTS \
    -net nic,model=ne2k_isa,vlan=1,macaddr=52:54:00:12:10:1 \
    -net socket,vlan=1,listen=127.0.0.1:8080 \
    -net nic,model=ne2k_isa,vlan=0,macaddr=52:54:00:12:20:1 \
    -net socket,vlan=0,listen=127.0.0.1:8010 \
    -no-fd-bootchk -s -S -p 10141 -m 10  -no-acpi -hdb diskd.img diskc.img &

# Computer 2
#
# Connected to VLAN1
#
echo "Starting computer 2"
screen /usr/bin/qemu $OPTS \
    -net nic,model=ne2k_isa,vlan=1,macaddr=52:54:00:12:10:2  \
    -net nic,model=ne2k_isa,vlan=1,macaddr=52:54:00:12:10:3  \
    -net socket,vlan=1,connect=127.0.0.1:8080 \
    -no-fd-bootchk -m 10  -no-acpi -hdb diskd.img diskc.img &

# Computer 3
#
# Connected to VLAN0
#
echo "Starting computer 3"
screen /usr/bin/qemu $OPTS \
    -net nic,model=ne2k_isa,vlan=0,macaddr=52:54:00:12:20:2  \
    -net nic,model=ne2k_isa,vlan=0,macaddr=52:54:00:12:20:3 \
    -net socket,vlan=0,connect=127.0.0.1:8010 \
    -no-fd-bootchk -m 10  -no-acpi -hdb diskd.img diskc.img &
