/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zgl.h"

// glVertex

void tglVertex4f(float x, float y, float z, float w) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_Vertex;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;
	p[4].f = w;

	c->gl_add_op(p);
}

void tglVertex2f(float x, float y)  {
	tglVertex4f(x, y, 0, 1);
}

void tglVertex3f(float x, float y, float z)  {
	tglVertex4f(x, y, z, 1);
}

void tglVertex3fv(const float *v)  {
	tglVertex4f(v[0], v[1], v[2], 1);
}

// glNormal

void tglNormal3f(float x, float y, float z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_Normal;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	c->gl_add_op(p);
}

void tglNormal3fv(const float *v)  {
	tglNormal3f(v[0], v[1], v[2]);
}

// glColor

void tglColor4f(float r, float g, float b, float a) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[9];

	p[0].op = TinyGL::OP_Color;
	p[1].f = r;
	p[2].f = g;
	p[3].f = b;
	p[4].f = a;
	c->gl_add_op(p);
}

void tglColor4fv(const float *v) {
	tglColor4f(v[0], v[1], v[2], v[3]);
}

void tglColor3f(float x, float y, float z) {
	tglColor4f(x, y, z, 1);
}

void tglColor3fv(const float *v)  {
	tglColor4f(v[0], v[1], v[2], 1);
}

void tglColor3ub(unsigned char r, unsigned char g, unsigned char b) {
	tglColor4f(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void tglColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	tglColor4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

// TexCoord

void tglTexCoord4f(float s, float t, float r, float q) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_TexCoord;
	p[1].f = s;
	p[2].f = t;
	p[3].f = r;
	p[4].f = q;

	c->gl_add_op(p);
}

void tglTexCoord2f(float s, float t) {
	tglTexCoord4f(s, t, 0, 1);
}

void tglTexCoord2fv(const float *v) {
	tglTexCoord4f(v[0], v[1], 0, 1);
}

void tglEdgeFlag(int flag) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_EdgeFlag;
	p[1].i = flag;

	c->gl_add_op(p);
}

// misc

void tglShadeModel(int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	assert(mode == TGL_FLAT || mode == TGL_SMOOTH);

	p[0].op = TinyGL::OP_ShadeModel;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglCullFace(int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	assert(mode == TGL_BACK || mode == TGL_FRONT || mode == TGL_FRONT_AND_BACK);

	p[0].op = TinyGL::OP_CullFace;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglFrontFace(int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	assert(mode == TGL_CCW || mode == TGL_CW);

	mode = (mode != TGL_CCW);

	p[0].op = TinyGL::OP_FrontFace;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglColorMask(TGLboolean r, TGLboolean g, TGLboolean b, TGLboolean a) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_ColorMask;
	p[1].i = r;
	p[2].i = g;
	p[3].i = b;
	p[4].i = a;

	c->gl_add_op(p);
}

void tglDepthMask(int enableWrite) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_DepthMask;
	p[1].i = enableWrite;

	c->gl_add_op(p);
}

void tglStencilMask(TGLuint mask) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_StencilMask;
	p[1].i = mask;

	c->gl_add_op(p);
}

void tglBlendFunc(TGLenum sfactor, TGLenum dfactor) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_BlendFunc;
	p[1].i = sfactor;
	p[2].i = dfactor;

	c->gl_add_op(p);
}

void tglAlphaFunc(TGLenum func, float ref) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_AlphaFunc;
	p[1].i = func;
	p[2].f = ref;

	c->gl_add_op(p);
}

void tglDepthFunc(TGLenum func) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_DepthFunc;
	p[1].i = func;

	c->gl_add_op(p);
}

void tglStencilFunc(TGLenum func, TGLint ref, TGLuint mask) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];
	p[0].op = TinyGL::OP_StencilFunc;
	p[1].i = func;
	p[2].i = ref;
	p[3].i = mask;

	c->gl_add_op(p);
}

void tglStencilOp(TGLenum sfail, TGLenum dpfail, TGLenum dppass) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];
	p[0].op = TinyGL::OP_StencilOp;
	p[1].i = sfail;
	p[2].i = dpfail;
	p[3].i = dppass;

	c->gl_add_op(p);
}

void tglPolygonMode(int face, int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	assert(face == TGL_BACK || face == TGL_FRONT || face == TGL_FRONT_AND_BACK);
	assert(mode == TGL_POINT || mode == TGL_LINE || mode == TGL_FILL);

	p[0].op = TinyGL::OP_PolygonMode;
	p[1].i = face;
	p[2].i = mode;

	c->gl_add_op(p);
}

// glEnable, glDisable

void tglEnable(int cap) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_EnableDisable;
	p[1].i = cap;
	p[2].i = 1;

	c->gl_add_op(p);
}

void tglDisable(int cap) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_EnableDisable;
	p[1].i = cap;
	p[2].i = 0;

	c->gl_add_op(p);
}

// glBegin, glEnd

void tglBegin(int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_Begin;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglEnd() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[1];

	p[0].op = TinyGL::OP_End;

	c->gl_add_op(p);
}

// matrix

void tglMatrixMode(int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_MatrixMode;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglLoadMatrixf(const float *m) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[17];

	p[0].op = TinyGL::OP_LoadMatrix;
	for (int i = 0; i < 16; i++)
		p[i + 1].f = m[i];

	c->gl_add_op(p);
}

void tglLoadIdentity() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[1];

	p[0].op = TinyGL::OP_LoadIdentity;

	c->gl_add_op(p);
}

void tglMultMatrixf(const float *m) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[17];

	p[0].op = TinyGL::OP_MultMatrix;
	for (int i = 0; i < 16; i++)
		p[i + 1].f = m[i];

	c->gl_add_op(p);
}

void tglPushMatrix() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[1];

	p[0].op = TinyGL::OP_PushMatrix;

	c->gl_add_op(p);
}

void tglPopMatrix() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[1];

	p[0].op = TinyGL::OP_PopMatrix;

	c->gl_add_op(p);
}

void tglRotatef(float angle, float x, float y, float z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_Rotate;
	p[1].f = angle;
	p[2].f = x;
	p[3].f = y;
	p[4].f = z;

	c->gl_add_op(p);
}

void tglTranslatef(float x, float y, float z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_Translate;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	c->gl_add_op(p);
}

void tglScalef(float x, float y, float z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_Scale;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	c->gl_add_op(p);
}

void tglViewport(int x, int y, int width, int height) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_Viewport;
	p[1].i = x;
	p[2].i = y;
	p[3].i = width;
	p[4].i = height;

	c->gl_add_op(p);
}

void tglFrustum(double left, double right, double bottom, double top, double nearv, double farv) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[7];

	p[0].op = TinyGL::OP_Frustum;
	p[1].f = (float)left;
	p[2].f = (float)right;
	p[3].f = (float)bottom;
	p[4].f = (float)top;
	p[5].f = (float)nearv;
	p[6].f = (float)farv;

	c->gl_add_op(p);
}

void tglOrtho(double left, double right, double bottom, double top, double zNear, double zFar) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[7];

	p[0].op = TinyGL::OP_Ortho;
	p[1].f = (float)left;
	p[2].f = (float)right;
	p[3].f = (float)bottom;
	p[4].f = (float)top;
	p[5].f = (float)zNear;
	p[6].f = (float)zFar;

	c->gl_add_op(p);
}

// lightening

void tglMaterialfv(int mode, int type, const float *v) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[7];
	int n;

	assert(mode == TGL_FRONT  || mode == TGL_BACK || mode == TGL_FRONT_AND_BACK);

	p[0].op = TinyGL::OP_Material;
	p[1].i = mode;
	p[2].i = type;
	n = 4;
	if (type == TGL_SHININESS)
		n = 1;
	for (int i = 0; i < 4; i++)
		p[3 + i].f = v[i];
	for (int i = n; i < 4; i++)
		p[3 + i].f = 0;

	c->gl_add_op(p);
}

void tglMaterialf(int mode, int type, float v) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[7];

	p[0].op = TinyGL::OP_Material;
	p[1].i = mode;
	p[2].i = type;
	p[3].f = v;
	for (int i = 0; i < 3; i++)
		p[4 + i].f = 0;

	c->gl_add_op(p);
}

void tglColorMaterial(int mode, int type) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_ColorMaterial;
	p[1].i = mode;
	p[2].i = type;

	c->gl_add_op(p);
}

void tglLightfv(int light, int type, const float *v) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[7];

	p[0].op = TinyGL::OP_Light;
	p[1].i = light;
	p[2].i = type;
	for (int i = 0; i < 4; i++) {
		if (type != TGL_SPOT_DIRECTION)
			p[3 + i].f = v[i];
		else
			p[3 + i].f = 0.0f;
	}

	c->gl_add_op(p);
}

void tglLightf(int light, int type, float v) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[7];

	p[0].op = TinyGL::OP_Light;
	p[1].i = light;
	p[2].i = type;
	p[3].f = v;
	for (int i = 0; i < 3; i++)
		p[4 + i].f = 0;

	c->gl_add_op(p);
}

void tglLightModeli(int pname, int param) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[6];

	p[0].op = TinyGL::OP_LightModel;
	p[1].i = pname;
	p[2].f = (float)param;
	for (int i = 0; i < 3; i++)
		p[3 + i].f = 0;

	c->gl_add_op(p);
}

void tglLightModelfv(int pname, const float *param) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[6];

	p[0].op = TinyGL::OP_LightModel;
	p[1].i = pname;
	for (int i = 0; i < 4; i++)
		p[2 + i].f = param[i];

	c->gl_add_op(p);
}

// clear

void tglClear(int mask) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_Clear;
	p[1].i = mask;

	c->gl_add_op(p);
}

void tglClearColor(float r, float g, float b, float a) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_ClearColor;
	p[1].f = r;
	p[2].f = g;
	p[3].f = b;
	p[4].f = a;

	c->gl_add_op(p);
}

void tglClearDepth(double depth) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_ClearDepth;
	p[1].f = (float)depth;

	c->gl_add_op(p);
}

void tglClearStencil(TGLint s) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_ClearStencil;
	p[1].i = s;

	c->gl_add_op(p);
}

// textures

void tglTexImage2D(int target, int level, int components, int width, int height, int border, int format, int type, void *pixels) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[10];

	p[0].op = TinyGL::OP_TexImage2D;
	p[1].i = target;
	p[2].i = level;
	p[3].i = components;
	p[4].i = width;
	p[5].i = height;
	p[6].i = border;
	p[7].i = format;
	p[8].i = type;
	p[9].p = pixels;

	c->gl_add_op(p);
}

void tglBindTexture(int target, int texture) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_BindTexture;
	p[1].i = target;
	p[2].i = texture;

	c->gl_add_op(p);
}

void tglTexEnvi(int target, int pname, int param) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[8];

	p[0].op = TinyGL::OP_TexEnv;
	p[1].i = target;
	p[2].i = pname;
	p[3].i = param;
	p[4].f = 0;
	p[5].f = 0;
	p[6].f = 0;
	p[7].f = 0;

	c->gl_add_op(p);
}

void tglTexParameteri(int target, int pname, int param) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[8];

	p[0].op = TinyGL::OP_TexParameter;
	p[1].i = target;
	p[2].i = pname;
	p[3].i = param;
	p[4].f = 0;
	p[5].f = 0;
	p[6].f = 0;
	p[7].f = 0;

	c->gl_add_op(p);
}

void tglPixelStorei(int pname, int param) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_PixelStore;
	p[1].i = pname;
	p[2].i = param;

	c->gl_add_op(p);
}

// selection

void tglInitNames() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[1];

	p[0].op = TinyGL::OP_InitNames;

	c->gl_add_op(p);
}

void tglPushName(uint name) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_PushName;
	p[1].i = name;

	c->gl_add_op(p);
}

void tglPopName() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[1];

	p[0].op = TinyGL::OP_PopName;

	c->gl_add_op(p);
}

void tglLoadName(uint name) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_LoadName;
	p[1].i = name;

	c->gl_add_op(p);
}

void tglPolygonOffset(TGLfloat factor, TGLfloat units) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_PolygonOffset;
	p[1].f = factor;
	p[2].f = units;

	c->gl_add_op(p);
}

// Special Functions

void tglCallList(uint list) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_CallList;
	p[1].i = list;

	c->gl_add_op(p);
}

void tglFlush() {
	// nothing to do
}

void tglHint(int target, int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_Hint;
	p[1].i = target;
	p[2].i = mode;

	c->gl_add_op(p);
}

// Non standard functions

void tglDebug(int mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	c->print_flag = mode;
}
