# create minimal ubuntu 12.04 install
if [ ! -d precise-chroot ]; then
  echo "Running debootstrap to make initial cross-build env"
  mkdir precise-chroot
  sudo debootstrap --variant=minbase precise ./precise-chroot http://archive.ubuntu.com/ubuntu/
fi

# bind stage-lights dir to chroot
echo "Binding build folder to home dir in cross-build env"
sudo umount ./
sudo mkdir -p precise-chroot/home/stage-lights
sudo mount --bind ./ ./precise-chroot/home/stage-lights

# add universe repo for gcc-arm-linux-gnueabihf
if ! grep 'deb http://archive.ubuntu.com/ubuntu precise universe' precise-chroot/etc/apt/sources.list; then
  echo "Updating apt sources in cross-build env"
  sudo sh -c "echo deb http://archive.ubuntu.com/ubuntu precise universe >> ./precise-chroot/etc/apt/sources.list"
fi

# download build dependencies to chroot
echo "Installing build dependency packages in cross-build env"
sudo chroot precise-chroot << CUT
apt-get update
apt-get install -y make
apt-get install -y gcc-arm-linux-gnueabihf
CUT
