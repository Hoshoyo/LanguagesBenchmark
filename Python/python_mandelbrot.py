import png
import numpy
import math
import time

class Complex:
	def __init__(self, re, im):
		self.re = re
		self.im = im
	
	def add(self, r):
		result = Complex(0.0, 0.0)
		result.re = self.re + r.re
		result.im = self.im + r.im
		return result
	
	def mult(self, r):
		result = Complex(0.0, 0.0)
		result.re = self.re * r.re - self.im * r.im
		result.im = self.re * r.im + self.im * r.re
		return result


scale = 2.0
xoff = 0.0
yoff = 0.0
its = 256
		
width = 800
height = 800
pngImage = numpy.uint16(numpy.zeros((width, height, 3)))

start_time = time.time()
for y in range(height):
	for x in range(width):

		half_scale = scale / 2.0

		x_coord = (x / width)  * scale - half_scale
		y_coord = (y / height) * scale - half_scale

		x_coord += xoff
		y_coord += yoff

		r = 0
		g = 0
		b = 0

		z = Complex(0.0, 0.0);
		coord = Complex(x_coord, y_coord)
		
		for it in range(its):
			z = z.mult(z).add(coord);
			v = z.re * z.re + z.im * z.im;
			if (math.sqrt(v) > 2.0):
				r = it
				g = it
				b = it
				break

		pngImage[y, x] = (r, g, b)
		
elapsed_time = time.time() - start_time
print('Elapsed time = {0}'.format(elapsed_time))

rowCount, columnCount, planeCount = pngImage.shape
with open("python_mandelbrot_image.png", "wb") as outFile:
        pngWriter = png.Writer(width, height, greyscale=False, bitdepth=8)
        pngWriter.write(outFile, numpy.reshape(pngImage, (-1, columnCount*planeCount)))
