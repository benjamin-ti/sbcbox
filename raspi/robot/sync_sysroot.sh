#! /bin/sh

PI_IP_ADDR=192.168.0.81

help()
{
    echo -h: help
    echo -s IP: board ip-address
    exit
}

while getopts hs: flag
do
    case "${flag}" in
        h) help;;
        s) PI_IP_ADDR=$OPTARG;;
    esac
done

echo Raspi-IP: $PI_IP_ADDR

if [ -z "$PI_IP_ADDR" ]
then
	PI_IP_ADDR=$(awk 'BEGIN{FS="="}/^ *PI_IP_ADDR/{gsub(" ", "", $2); gsub("\r", "", $2); gsub("\n", "", $2); print($2);}' $1)
fi
printf "PI_IP_ADDR: '%s'\n" "$PI_IP_ADDR"

if [ ! -f "/home/$USER/.ssh/id_rsa" ]
then
    mkdir -p "/home/$USER/.ssh"
    ssh pi@"$PI_IP_ADDR" "exit"
    ssh-keygen
fi

ssh-keygen -f "/home/$USER/.ssh/known_hosts" -R "$PI_IP_ADDR"
ssh-copy-id pi@"$PI_IP_ADDR"

mkdir -p ~/opt
sudo mkdir -p ~/opt/rpi_debian11_lx5_15_sysroot
cd ~/opt/rpi_debian11_lx5_15_sysroot

sudo rsync -avz pi@$PI_IP_ADDR:/lib .
sudo rsync -avz pi@$PI_IP_ADDR:/usr/include usr
sudo rsync -avz pi@$PI_IP_ADDR:/usr/lib usr
sudo rsync -avz pi@$PI_IP_ADDR:/usr/local/include usr/local
sudo rsync -avz pi@$PI_IP_ADDR:/usr/local/lib usr/local
