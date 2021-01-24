#ifndef  __COLORH
#define __COLORH

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define DEFAULT_RGB_R_SHIFT_32  16
#define DEFAULT_RGB_G_SHIFT_32  8
#define DEFAULT_RGB_B_SHIFT_32  0
#define DEFAULT_RGB_A_SHIFT_32  24

#pragma region Color_Functions


int getr32(int c);
int getg32(int c);
int getb32(int c);
int geta32(int c);
int makeacol32(int r, int g, int b, int a);

#pragma endregion

#endif