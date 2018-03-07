# Computer 1
#
# Connected to VLAN0 and VLAN1
#

PORT=8849
QEMU=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu

echo "Starting computer 1"
$QEMU \
    -net nic,model=ne2k_isa,vlan=0,macaddr=52:54:00:12:20:1 \
    -net socket,vlan=0,listen=0.0.0.0:$PORT \
    -m 10 -std-vga -no-acpi -hdb diskd.img diskc.img &

# Computer 2
#
# Connected to VLAN1
#

sleep 5

# Computer 3
#
# Connected to VLAN0
#
echo "Starting computer 3"
$QEMU \
    -net nic,model=ne2k_isa,vlan=0,macaddr=52:54:00:12:20:2  \
    -net socket,vlan=0,connect=127.0.0.1:$PORT \
    -m 10 -std-vga -no-acpi -hdb diskd.img diskc.img &
