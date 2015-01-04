#!/usr/bin/python

import sys

in_files = [open(x, 'rt') for x in sys.argv[1:]]
out_file = open('src/icons.h', 'wt');

def parse_icon(f):
  vals = [0] * 8 
  ln = 0
  for l in f.readlines():
    v = 0
    for i in range(8):
      if l[i] != ' ' and l[i] != '.':
        v |= (1 << i)
    vals[ln] = v
    ln += 1
    if ln == 8:
      break
  return vals

for f in in_files:
  icon = parse_icon(f)
  out_file.write(
    ('    {0x%02X, 0x%02X, 0x%02X, 0x%02X, ' +
      '0x%02X, 0x%02X, 0x%02X, 0x%02X},\n') %
    tuple(icon))

out_file.close()
