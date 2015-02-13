echo 30 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio30/direction
echo 1 > /sys/class/gpio/gpio30/value
