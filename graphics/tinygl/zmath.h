#ifndef GRAPHICS_TINYGL_ZMATH_H
#define GRAPHICS_TINYGL_ZMATH_H

namespace TinyGL {

// Matrix & Vertex

struct M4 {
	float m[4][4];
};

struct M3 {
	float m[3][3];
};

struct M34 {
	float m[3][4];
};


#define X v[0]
#define Y v[1]
#define Z v[2]
#define W v[3]

struct V3 {
	float v[3];
};

struct V4 {
	float v[4];
};

void gl_M4_Id(M4 *a);
int gl_M4_IsId(const M4 *a);
void gl_M4_Move(M4 *a, const M4 *b);
void gl_MoveV3(V3 *a, const V3 *b);
void gl_MulM4V3(V3 *a, const M4 *b, const V3 *c);
void gl_MulM3V3(V3 *a, const M4 *b, const V3 *c);

void gl_M4_MulV4(V4 *a, const M4 *b, const V4 *c);
void gl_M4_InvOrtho(M4 *a, const M4 &b);
void gl_M4_Inv(M4 *a, const M4 *b);
void gl_M4_Mul(M4 *c, const M4 *a, const M4 *b);
void gl_M4_MulLeft(M4 *c, const M4 *a);
void gl_M4_Transpose(M4 *a, const M4 *b);
void gl_M4_Rotate(M4 *c, float t, int u);
int gl_V3_Norm(V3 *a);

V3 gl_V3_New(float x, float y, float z);
V4 gl_V4_New(float x, float y, float z, float w);

int gl_Matrix_Inv(float *r, float *m, int n);

} // end of namespace TinyGL

#endif
