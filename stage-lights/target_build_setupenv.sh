# create minimal ubuntu 12.04 install
mkdir precise-chroot
sudo debootstrap --variant=minbase precise ./precise-chroot http://archive.ubuntu.com/ubuntu/

# bind stage-lights dir to chroot
sudo mkdir precise-chroot/home/stage-lights
sudo mount --bind ./ ./precise-chroot/home/stage-lights

echo deb http://archive.ubuntu.com/ubuntu precise universe >> ./precise-chroot/etc/apt/sources.list

# download build dependencies to chroot
sudo chroot precise-chroot <<<CUT
apt-get install make
# add universe repo for gcc-arm-linux-gnueabihf
apt-get update
apt-get install gcc-arm-linux-gnueabihf
CUT
