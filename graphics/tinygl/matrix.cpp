#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

#include "graphics/tinygl/zgl.h"

namespace TinyGL {

void gl_print_matrix(const float *m) {
	for (int i = 0; i < 4; i++) {
		fprintf(stderr, "%f %f %f %f\n", m[i], m[4 + i], m[8 + i], m[12 + i]);
	}
}

static inline void gl_matrix_update(GLContext *c) {
	c->matrix_model_projection_updated = (c->matrix_mode <= 1);
}

void glopMatrixMode(GLContext *c, GLParam *p) {
	int mode = p[1].i;
	switch (mode) {
	case TGL_MODELVIEW:
		c->matrix_mode = 0;
		break;
	case TGL_PROJECTION:
		c->matrix_mode = 1;
		break;
	case TGL_TEXTURE:
		c->matrix_mode = 2;
		break;
	default:
		assert(0);
	}
}

void glopLoadMatrix(GLContext *c, GLParam *p) {
	Matrix4 *m;
	GLParam *q;

	m = c->matrix_stack_ptr[c->matrix_mode];
	q = p + 1;

	for (int i = 0; i < 4; i++) {
		m->_m[0][i] = q[0].f;
		m->_m[1][i] = q[1].f;
		m->_m[2][i] = q[2].f;
		m->_m[3][i] = q[3].f;
		q += 4;
	}

	gl_matrix_update(c);
}

void glopLoadIdentity(GLContext *c, GLParam *) {
	c->matrix_stack_ptr[c->matrix_mode]->identity();
	gl_matrix_update(c);
}

void glopMultMatrix(GLContext *c, GLParam *p) {
	Matrix4 m;
	GLParam *q;
	q = p + 1;

	for (int i = 0; i < 4; i++) {
		m._m[0][i] = q[0].f;
		m._m[1][i] = q[1].f;
		m._m[2][i] = q[2].f;
		m._m[3][i] = q[3].f;
		q += 4;
	}

	*c->matrix_stack_ptr[c->matrix_mode] *= m;

	gl_matrix_update(c);
}


void glopPushMatrix(GLContext *c, GLParam *) {
	int n = c->matrix_mode;
	Matrix4 *m;

	assert((c->matrix_stack_ptr[n] - c->matrix_stack[n] + 1) < c->matrix_stack_depth_max[n]);

	m = ++c->matrix_stack_ptr[n];

	m[0] = m[-1];

	gl_matrix_update(c);
}

void glopPopMatrix(GLContext *c, GLParam *) {
	int n = c->matrix_mode;

	assert(c->matrix_stack_ptr[n] > c->matrix_stack[n]);
	c->matrix_stack_ptr[n]--;
	gl_matrix_update(c);
}

void glopRotate(GLContext *c, GLParam *p) {
	Matrix4 m;
	float u[3];
	float angle;
	int dir_code;

	angle = (float)(p[1].f * LOCAL_PI / 180.0);
	u[0] = p[2].f;
	u[1] = p[3].f;
	u[2] = p[4].f;

	// simple case detection
	dir_code = ((u[0] != 0) << 2) | ((u[1] != 0) << 1) | (u[2] != 0);

	switch (dir_code) {
	case 0:
		m.identity();
		break;
	case 4:
		if (u[0] < 0) angle = -angle;
		m.rotation(angle, 0);
		break;
	case 2:
		if (u[1] < 0) angle = -angle;
		m.rotation(angle, 1);
		break;
	case 1:
		if (u[2] < 0) angle = -angle;
		m.rotation(angle, 2);
		break;
	default: {
		float cost, sint;

		// normalize vector
		float len = u[0] * u[0] + u[1] * u[1] + u[2] * u[2];
		if (len == 0.0f)
			return;
		len = 1.0f / sqrt(len);
		u[0] *= len;
		u[1] *= len;
		u[2] *= len;

		// store cos and sin values
		cost = cos(angle);
		sint = sin(angle);

		// fill in the values
		m._m[3][0] = 0.0f;
		m._m[3][2] = 0.0f;
		m._m[0][3] = 0.0f;
		m._m[1][3] = 0.0f;
		m._m[2][3] = 0.0f;
		m._m[3][3] = 1.0f;

		// do the math
		m._m[0][0] = u[0] * u[0] + cost * (1 - u[0] * u[0]);
		m._m[1][0] = u[0] * u[1] * (1 -cost) - u[2] * sint;
		m._m[2][0] = u[2] * u[0] * (1 -cost) + u[1] * sint;
		m._m[0][1] = u[0] * u[1] * (1 -cost) + u[2] * sint;
		m._m[1][1] = u[1] * u[1] + cost * (1 - u[1] * u[1]);
		m._m[2][1] = u[1] * u[2] * (1 - cost) - u[0] * sint;
		m._m[0][2] = u[2] * u[0] * (1 - cost) - u[1] * sint;
		m._m[1][2] = u[1] * u[2] * (1 - cost) + u[0] * sint;
		m._m[2][2] = u[2] * u[2] + cost * (1 - u[2] * u[2]);
	}
	}

	*c->matrix_stack_ptr[c->matrix_mode] *= m;

	gl_matrix_update(c);
}

void glopScale(GLContext *c, GLParam *p) {
	c->matrix_stack_ptr[c->matrix_mode]->scale(p[1].f, p[2].f, p[3].f);
	gl_matrix_update(c);
}

void glopTranslate(GLContext *c, GLParam *p) {
	c->matrix_stack_ptr[c->matrix_mode]->translate(p[1].f, p[2].f, p[3].f);
	gl_matrix_update(c);
}

void glopFrustum(GLContext *c, GLParam *p) {
	float left = p[1].f;
	float right = p[2].f;
	float bottom = p[3].f;
	float top = p[4].f;
	float nearp = p[5].f;
	float farp = p[6].f;
	Matrix4 m = Matrix4::frustrum(left, right, bottom, top, nearp, farp);

	*c->matrix_stack_ptr[c->matrix_mode] *= m;

	gl_matrix_update(c);
}

void glopOrtho(GLContext *context, GLParam *p) {
	float *r;
	TinyGL::Matrix4 m;
	float left = p[1].f;
	float right = p[2].f;
	float bottom = p[3].f;
	float top = p[4].f;
	float zNear = p[5].f;
	float zFar = p[6].f;

	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = -2.0f / (zFar - zNear);

	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	float tz = -(zFar + zNear) / (zFar - zNear);

	r = &m._m[0][0];
	r[0] = a; r[1] = 0; r[2] = 0; r[3] = 0;
	r[4] = 0; r[5] = b; r[6] = 0; r[7] = 0;
	r[8] = 0; r[9] = 0; r[10] = c; r[11] = 0;
	r[12] = tx; r[13] = ty; r[14] = tz; r[15] = 0;

	*context->matrix_stack_ptr[context->matrix_mode] *= m;
	gl_matrix_update(context);
}

} // end of namespace TinyGL

// Code take from openGL wiki and adapted: http://www.opengl.org/wiki/GluProject_and_gluUnProject_code
void tgluUnProject(double winx, double winy, double winz, const double modelMatrix[16], const double projMatrix[16], const int viewport[4], double *objx, double *objy, double *objz) {
	//Transformation matrices
	
	TinyGL::Vector4 in, out;
	TinyGL::Matrix4 A;
	TinyGL::Matrix4 m;
	TinyGL::Matrix4 model, projection;
	for(int i = 0; i < 16; i++)
	{
		((float *)model._m)[i] = modelMatrix[i];
		((float *)projection._m)[i] = projMatrix[i];
	}
	//Calculation for inverting a matrix, compute projection x modelview
	//and store in A[16]
	A = model * projection;
	//Now compute the inverse of matrix A
	m = A.inverse();
	//Transformation of normalized coordinates between -1 and 1
	in.X = (winx - (float)viewport[0]) / (float)viewport[2] * 2.0 - 1.0;
	in.Y = (winy - (float)viewport[1]) / (float)viewport[3] * 2.0 - 1.0;
	in.Z = 2.0 * winz - 1.0;
	in.W = 1.0;
	//Objects coordinates
	m.transform(in,out);
	if (out.W == 0.0f)
		return;
	out.W = 1.0 / out.W;
	*objx = out.X * out.W;
	*objy = out.Y * out.W;
	*objz = out.Z * out.W;
}
