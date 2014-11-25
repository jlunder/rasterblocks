# create minimal ubuntu 12.04 install
#mkdir precise-chroot
#sudo debootstrap --variant=minbase precise ./precise-chroot http://archive.ubuntu.com/ubuntu/

# bind stage-lights dir to chroot
sudo mkdir precise-chroot/home/stage-lights
sudo mount --bind ./ ./precise-chroot/home/stage-lights

# download build dependencies to chroot
sudo chroot precise-chroot <<CUT

apt-get install make
# add universe repo for gcc-arm-linux-gnueabihf
echo deb http://archive.ubuntu.com/ubuntu precise universe >> /etc/apt/sources.list
apt-get update
apt-get install gcc-arm-linux-gnueabihf


exit
CUT