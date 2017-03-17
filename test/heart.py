# Reference: https://www.zhihu.com/question/20187195/answer/34873279

import math
import sys

def f(x, y, z):
	a = x * x + 9.0 / 4.0 * y * y + z * z - 1
	return a * a * a - x * x * z * z * z - 9.0 / 80.0 * y * y * z * z * z

def h(x, z):
	y = 1.0
	while y >= 0.0:
		if f(x, y, z) <= 0.0:
			return y
		y -= 0.001
	return 0.0

z = 1.5
while z > -1.5:
	x = -1.5
	while x < 1.5:
		if f(x, 0.0, z) <= 0.0:
			y0 = h(x, z)
			ny = 0.01
			nx = h(x + ny, z) - y0
			nz = h(x, z + ny) - y0
			nd = 1.0 / math.sqrt(nx * nx + ny * ny + nz * nz)
			d = (nx + ny - nz) * nd * 0.5 + 0.5
			sys.stdout.write(".:-=+*#%@"[int(d * 5.0)])
		else:
			sys.stdout.write(" ")
		x += 0.025
	print ""
	z -= 0.05
