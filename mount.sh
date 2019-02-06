
help() { echo "Help: $0 comands: \n -u it's for unmount \n -m it's for mount";}

if [ $1 ];
then
while getopts ":mu" opt
		do
			case ${opt} in
				m)
					mount -t ext4 -o dax /dev/sda4 /home/mrmind/Desktop/BOPT/ramdisk/
					;;
				u)
					umount /home/mrmind/Desktop/BOPT/ramdisk/
					;;
				 h | *)
					help
					;;
			esac

		done
else
	help
fi
