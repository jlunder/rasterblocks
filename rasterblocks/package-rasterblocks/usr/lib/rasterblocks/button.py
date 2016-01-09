import time

config = """
{
	"logLevel": "warning", // info, warning, error
	"audioInput": "alsa", // alsa, openal, file
	"audioInputParam": "plughw:1",
	"lightOutput": "pixelpusher", // opengl, pixelpusher, spidev
	"lightOutputParam": "192.168.0.4",

	"lowCutoff": 200,
	"hiCutoff": 500,
	"agcMax": 1,
	"agcMin": 0.001,
	"agcStrength": 0.75,
	
	"mode": %d,

	"brightness": 1
}
"""

try:
  f = open("/sys/class/gpio/export", "wt")
  f.write("72\n")
  f.close()
except:
  pass
try:
  f = open("/sys/class/gpio/gpio72/direction", "wt")
  f.write("in\n")
  f.close()
except:
  pass
last_pressed = False
mode = 0
open('/var/lib/rasterblocks/config.json', 'wt').write(config % (mode,))
while True:
  time.sleep(0.1)
  pressed_str = open("/sys/class/gpio/gpio72/value", "rt").read().strip()
  pressed = pressed_str != '1'
  if pressed and not last_pressed:
    mode = (mode + 1) % 13
    open('/var/lib/rasterblocks/config.json', 'wt').write(config % (mode,))
  last_pressed = pressed
