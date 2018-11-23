#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
struct Timer {
	LARGE_INTEGER frequency;

	Timer() {
		QueryPerformanceFrequency(&frequency);
	}

	double GetTime() {
		LARGE_INTEGER pcount = {};
		QueryPerformanceCounter(&pcount);
		return double(pcount.QuadPart) / double(frequency.QuadPart) * 1000.0;
	}
};
#elif defined(__linux__)
#include <unistd.h>
#include <stdlib.h>
struct Timer
{
	Timer() {}
	double GetTime(){
		clockid_t clockid;
		struct timespec t_spec;
		int start = clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec);
		u64 res = t_spec.tv_nsec + 1000000000 * t_spec.tv_sec;
		return (double)res / 1000000.0;
	}
};

#endif

typedef int s32;
typedef long long int s64;
typedef unsigned int u32;
typedef unsigned char u8;
typedef double r64;
typedef float r32;

#include <stdio.h>

u32 its = 256;
r64 scale = 2.0f;
r64 xoff = 0.0;
r64 yoff = 0.0;

struct Complex {
	r64 re;
	r64 im;

	Complex() : re(0.0f), im(0.0f) {}
	Complex(r64 real, r64 imag) : re(real), im(imag){}

	inline Complex operator*(Complex r) {
		// (a + bi) * (c + di)
		// ac + adi + cbi - bd
		// ac - bd + i(ad + cb)
		Complex result;
		result.re = this->re * r.re - this->im * r.im;
		result.im = this->re * r.im + this->im * r.re;
		return result;
	}

	inline Complex operator+(Complex r) {
		// (a + bi) + (c + di)
		// (a + c) + (b + d)i
		Complex result;
		result.re = this->re + r.re;
		result.im = this->im + r.im;
		return result;
	}
};

void modify_image(u8* image, u32 width, u32 height, r32 scale, r64 xoff, r64 yoff) {
	for (u32 y = 0; y < height; ++y) {
		for (u32 x = 0; x < width; ++x) {

			r64 half_scale = scale / 2.0;

			u32 index = (y * width * 4) + x * 4;
			r64 x_coord = ((r64)x / width)  * scale - half_scale;
			r64 y_coord = ((r64)y / height) * scale - half_scale;

			x_coord += xoff;
			y_coord += yoff;

			u8 r = 0, g = 0, b = 0;

			Complex z;
			Complex coord(x_coord, y_coord);

			for (u32 it = 0; it < its; ++it) {
				z = z * z + coord;
				r64 v = z.re * z.re + z.im * z.im;
				if (sqrt(v) > 2.0) {
					r = g = b = it;
					break;
				}
			}
			*(u32*)(image + index) = 0xff000000 | (r << 16) | (g << 8) | b;
		}
	}
}

int main() {
	Timer t;
	
	s32 width = 800;
	s32 height = 800;
	u8* image = (u8*)calloc(1, sizeof(u32) * width * height);

	double start = t.GetTime();
	modify_image(image, width, height, scale, xoff, yoff);
	double end = t.GetTime();
	printf("Elapsed time %f ms\n", end - start);

	stbi_write_png("out.png", width, height, 4, image, width * 4);
	
	return 0;
}