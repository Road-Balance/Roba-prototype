import pygame

pygame.init()
p1 = pygame.joystick.Joystick(0)
p1.init()
clock = pygame.time.Clock()

#logitech f350 in X mode
# -- location |axis|  neg  | pos|
# 0 -> left   X    |  left | right
# 1 -> left   Y    |  up   | down
# 2 => triggers -  | right | left
# 3-> right   Y    |  up   | down
# 4->  right X     |  left | right
# (mode)  hat    left,down | up,right
# 0 a | 1 b | x 2 | y 3 | lb4 | rb 5 | ls 8, rs 9| back 6 start 7|

while 1:
  for event in pygame.event.get():
      print (p1.get_hat(0))
      clock.tick(40)
pygame.quit ()