from PIL import Image

bulb = Image.open('inventor-live-logo-light-bulb.png')
text1 = Image.open('inventor-live-logo-text-inventor.png')
text2 = Image.open('inventor-live-logo-text-live.png')

def printimage(im1, mask1, im2 = None, mask2 = None):
  for y in range(im1.size[1]):
    s = '    '
    for x in range(im1.size[0]):
      p1 = im1.getpixel((x,y,))
      p1 = ((p1[0] * mask1[0] + 127) / 255, (p1[1] * mask1[1] + 127) / 255, (p1[2] * mask1[2] + 127) / 255, p1[3])
      if im2:
        p2 = im2.getpixel((x,y,))
        p2 = ((p2[0] * mask2[0] + 127) / 255, (p2[1] * mask2[1] + 127) / 255, (p2[2] * mask2[2] + 127) / 255, p2[3])
        p1 = (p1[0] + p2[0], p1[1] + p2[1], p1[2] + p2[2], p1[3] + p2[3])
      s += '{%3d,%3d,%3d,%3d},' % (p1[0], p1[1], p1[2], p1[3])
    print s

printimage(bulb, (255, 255, 255))
print
printimage(text1, (101, 246, 176), text2, (243, 44, 140))
print
