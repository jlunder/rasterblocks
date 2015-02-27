for X in 70 71 72 73 74 75 76 77; do
  echo $X > /sys/class/gpio/export
  echo out > /sys/class/gpio/gpio$X/direction
  echo 1 > /sys/class/gpio/gpio$X/value
done
