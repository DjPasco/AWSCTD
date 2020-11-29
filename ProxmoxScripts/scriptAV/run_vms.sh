# script arguments:
#$1 = VM ID
#$2 = template VM ID
#$3 = vir-fname

vmid=$1
logFile=/${PWD}/$vmid.log
templvmid=$2

function LogImpl() {
    dt=$(date +"%d-%m-%y %T")
    echo $dt $1 >> $logFile
    echo $dt $1
}


vmdiskdest=/var/lib/vz/images/$vmid

vmdisk1=$vmdiskdest/vm-$vmid-disk-1.qcow2
vmdisk2=$vmdiskdest/vm-$vmid-disk-2.raw

# Report to log
LogImpl "Script start"
LogImpl "qm stop "$vmid
# In case off VM is running, stop it
qm stop $vmid

# Clean up destination VM disks
LogImpl "rm -f "$vmdiskdest
rm -f $vmdiskdest/*

LogImpl "Copying disk images."

# Restore destination VM disks from template VM
cp -f /var/lib/vz/images/$templvmid/vm-$templvmid-disk-1.qcow2 $vmdisk1
cp -f /var/lib/vz/images/$templvmid/vm-$templvmid-disk-2.raw   $vmdisk2

LogImpl "Copy finished."

# How to list partitions in the disk, just to check.
#kpartx -l /var/lib/vz/images/$vmid/vm-$vmid-disk-2.raw

# Create mapping as loopXpY, where X = loop device (0, 1, 2, ...), Y = partition nr (1, 2, 3, ...). First dev, first part =loop0p1
# Partition will be mapped to: /dev/mapper/loopXpY
LogImpl "kpartx -a -v "$vmdisk2
kpartx -a -v $vmdisk2

# Extract mapped device
loDev=$(losetup -a | grep $vmdisk2 | cut -c 6-10)
part=p1
LogImpl "loDev = "$loDev
LogImpl "part = "$part

# Destination folder for mount
mntDest=/mnt/win-disk-$vmid
LogImpl "mntDest = "$mntDest
# Just in case off - unmount
umount -f $mntDest

# Clean up
rm -rf $mntDest
mkdir $mntDest

# Mount mapped partition
# Always map first partition, NTFS is OK
LogImpl "Mounting "$loDev$part" to "$mntDest
mount.ntfs-3g /dev/mapper/$loDev$part $mntDest
LogImpl "Mounted."

# Copy content to mapped partition
LogImpl "Copying "$3" to "$mntDest
cp /var/lib/vz/samples/$3 $mntDest/virsrc.7z

#flush file system buffers
LogImpl "sync"
sync

# Umount/Clean up
LogImpl "Cleaning mount information and loop device"
umount -f $mntDest
rm -rf $mntDest

#remove mappings
kpartx -d $vmdisk2 

LogImpl "Startting machine: "$vmid
qm start $vmid

LogImpl "sleep 1m"
sleep 1m

LogImpl "qm stop "$vmid
qm stop $vmid

# Report to log
LogImpl "Script end."