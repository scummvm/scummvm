#include "color.h"

int getr32(int c) {
	return ((c >> DEFAULT_RGB_R_SHIFT_32) & 0xFF);
}


int getg32(int c) {
	return ((c >> DEFAULT_RGB_G_SHIFT_32) & 0xFF);
}


int getb32(int c) {
	return ((c >> DEFAULT_RGB_B_SHIFT_32) & 0xFF);
}


int geta32(int c) {
	return ((c >> DEFAULT_RGB_A_SHIFT_32) & 0xFF);
}


int makeacol32(int r, int g, int b, int a) {
	return ((r << DEFAULT_RGB_R_SHIFT_32) |
	        (g << DEFAULT_RGB_G_SHIFT_32) |
	        (b << DEFAULT_RGB_B_SHIFT_32) |
	        (a << DEFAULT_RGB_A_SHIFT_32));
}