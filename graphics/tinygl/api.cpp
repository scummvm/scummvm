/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 * This file is based on, or a modified version of code from TinyGL (C) 1997-2022 Fabrice Bellard,
 * which is licensed under the MIT license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zgl.h"

// glVertex

void tglVertex4f(TGLfloat x, TGLfloat y, TGLfloat z, TGLfloat w) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_Vertex;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;
	p[4].f = w;

	c->gl_add_op(p);
}

void tglVertex2f(TGLfloat x, TGLfloat y)  {
	tglVertex4f(x, y, 0, 1);
}

void tglVertex3f(TGLfloat x, TGLfloat y, TGLfloat z)  {
	tglVertex4f(x, y, z, 1);
}

void tglVertex3fv(const TGLfloat *v)  {
	tglVertex4f(v[0], v[1], v[2], 1);
}

// glNormal

void tglNormal3f(TGLfloat x, TGLfloat y, TGLfloat z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_Normal;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	c->gl_add_op(p);
}

void tglNormal3fv(const TGLfloat *v)  {
	tglNormal3f(v[0], v[1], v[2]);
}

// glColor

void tglColor4f(TGLfloat r, TGLfloat g, TGLfloat b, TGLfloat a) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[9];

	p[0].op = TinyGL::OP_Color;
	p[1].f = r;
	p[2].f = g;
	p[3].f = b;
	p[4].f = a;
	c->gl_add_op(p);
}

void tglColor4fv(const TGLfloat *v) {
	tglColor4f(v[0], v[1], v[2], v[3]);
}

void tglColor3f(TGLfloat x, TGLfloat y, TGLfloat z) {
	tglColor4f(x, y, z, 1);
}

void tglColor3fv(const TGLfloat *v)  {
	tglColor4f(v[0], v[1], v[2], 1);
}

void tglColor3ub(TGLubyte r, TGLubyte g, TGLubyte b) {
	tglColor4f(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void tglColor4ub(TGLubyte r, TGLubyte g, TGLubyte b, TGLubyte a) {
	tglColor4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

// TexCoord

void tglTexCoord4f(TGLfloat s, TGLfloat t, TGLfloat r, TGLfloat q) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_TexCoord;
	p[1].f = s;
	p[2].f = t;
	p[3].f = r;
	p[4].f = q;

	c->gl_add_op(p);
}

void tglTexCoord3f(TGLfloat s, TGLfloat t, TGLfloat q) {
	tglTexCoord4f(s, t, q, 1);
}

void tglTexCoord2f(TGLfloat s, TGLfloat t) {
	tglTexCoord4f(s, t, 0, 1);
}

void tglTexCoord1f(TGLfloat s) {
	tglTexCoord4f(s, 0, 0, 1);
}

void tglTexCoord4fv(const TGLfloat *v) {
	tglTexCoord4f(v[0], v[1], v[2], v[3]);
}

void tglTexCoord3fv(const TGLfloat *v) {
	tglTexCoord4f(v[0], v[1], v[2], 1);
}

void tglTexCoord2fv(const TGLfloat *v) {
	tglTexCoord4f(v[0], v[1], 0, 1);
}

void tglTexCoord1fv(const TGLfloat *v) {
	tglTexCoord4f(v[0], 0, 0, 1);
}

// misc

void tglEdgeFlag(TGLboolean flag) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_EdgeFlag;
	p[1].i = flag == TGL_TRUE ? 1 : 0;;

	c->gl_add_op(p);
}

void tglShadeModel(TGLenum mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	assert(mode == TGL_FLAT || mode == TGL_SMOOTH);

	p[0].op = TinyGL::OP_ShadeModel;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglCullFace(TGLenum mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	assert(mode == TGL_BACK || mode == TGL_FRONT || mode == TGL_FRONT_AND_BACK);

	p[0].op = TinyGL::OP_CullFace;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglFrontFace(TGLenum mode) {
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
	p[1].i = r == TGL_TRUE ? 1 : 0;
	p[2].i = g == TGL_TRUE ? 1 : 0;
	p[3].i = b == TGL_TRUE ? 1 : 0;
	p[4].i = a == TGL_TRUE ? 1 : 0;;

	c->gl_add_op(p);
}

void tglDepthMask(TGLboolean flag) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_DepthMask;
	p[1].i = flag == TGL_TRUE ? 1 : 0;

	c->gl_add_op(p);
}

void tglStencilMask(TGLuint mask) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];
	p[0].op = TinyGL::OP_StencilMask;
	p[1].ui = mask;

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

void tglAlphaFunc(TGLenum func, TGLclampf ref) {
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
	p[3].ui = mask;

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

void tglPolygonMode(TGLenum face, TGLenum mode) {
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

void tglEnable(TGLenum cap) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_EnableDisable;
	p[1].i = cap;
	p[2].i = 1;

	c->gl_add_op(p);
}

void tglDisable(TGLenum cap) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_EnableDisable;
	p[1].i = cap;
	p[2].i = 0;

	c->gl_add_op(p);
}

// glBegin, glEnd

void tglBegin(TGLenum mode) {
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

void tglMatrixMode(TGLenum mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_MatrixMode;
	p[1].i = mode;

	c->gl_add_op(p);
}

void tglLoadMatrixf(const TGLfloat *m) {
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

void tglMultMatrixf(const TGLfloat *m) {
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

void tglRotatef(TGLfloat angle, TGLfloat x, TGLfloat y, TGLfloat z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_Rotate;
	p[1].f = angle;
	p[2].f = x;
	p[3].f = y;
	p[4].f = z;

	c->gl_add_op(p);
}

void tglTranslatef(TGLfloat x, TGLfloat y, TGLfloat z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_Translate;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	c->gl_add_op(p);
}

void tglScalef(TGLfloat x, TGLfloat y, TGLfloat z) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_Scale;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	c->gl_add_op(p);
}

void tglViewport(TGLint x, TGLint y, TGLsizei width, TGLsizei height) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_Viewport;
	p[1].i = x;
	p[2].i = y;
	p[3].i = width;
	p[4].i = height;

	c->gl_add_op(p);
}

void tglFrustum(TGLdouble left, TGLdouble right, TGLdouble bottom, TGLdouble top, TGLdouble nearv, TGLdouble farv) {
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

void tglOrtho(TGLdouble left, TGLdouble right, TGLdouble bottom, TGLdouble top, TGLdouble zNear, TGLdouble zFar) {
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

void tglMaterialfv(TGLenum mode, TGLenum type, const TGLfloat *v) {
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

void tglMaterialf(TGLenum mode, TGLenum type, TGLfloat v) {
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

void tglColorMaterial(TGLenum mode, TGLenum type) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_ColorMaterial;
	p[1].i = mode;
	p[2].i = type;

	c->gl_add_op(p);
}

void tglLightfv(TGLenum light, TGLenum type, const TGLfloat *v) {
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

void tglLightf(TGLenum light, TGLenum type, TGLfloat v) {
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

void tglLightModeli(TGLenum pname, TGLint param) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[6];

	p[0].op = TinyGL::OP_LightModel;
	p[1].i = pname;
	p[2].f = (float)param;
	for (int i = 0; i < 3; i++)
		p[3 + i].f = 0;

	c->gl_add_op(p);
}

void tglLightModelfv(TGLenum pname, const TGLfloat *param) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[6];

	p[0].op = TinyGL::OP_LightModel;
	p[1].i = pname;
	for (int i = 0; i < 4; i++)
		p[2 + i].f = param[i];

	c->gl_add_op(p);
}

// clear

void tglClear(TGLbitfield mask) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_Clear;
	p[1].i = mask;

	c->gl_add_op(p);
}

void tglClearColor(TGLfloat r, TGLfloat g, TGLfloat b, TGLfloat a) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_ClearColor;
	p[1].f = r;
	p[2].f = g;
	p[3].f = b;
	p[4].f = a;

	c->gl_add_op(p);
}

void tglClearDepth(TGLdouble depth) {
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

void tglPolygonOffset(TGLfloat factor, TGLfloat units) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_PolygonOffset;
	p[1].f = factor;
	p[2].f = units;

	c->gl_add_op(p);
}

void tglFlush() {
	// nothing to do
}

void tglHint(TGLenum target, TGLenum mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_Hint;
	p[1].i = target;
	p[2].i = mode;

	c->gl_add_op(p);
}

// textures

void tglTexImage2D(TGLenum target, TGLint level, TGLint internalformat, TGLsizei width,
	           TGLsizei height, TGLint border, TGLenum format, TGLenum type, const void *pixels) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[10];

	p[0].op = TinyGL::OP_TexImage2D;
	p[1].i = target;
	p[2].i = level;
	p[3].i = internalformat;
	p[4].i = width;
	p[5].i = height;
	p[6].i = border;
	p[7].i = format;
	p[8].i = type;
	p[9].p = const_cast<void *>(pixels);

	c->gl_add_op(p);
}

void tglBindTexture(TGLenum target, TGLuint texture) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[3];

	p[0].op = TinyGL::OP_BindTexture;
	p[1].i = target;
	p[2].ui = texture;

	c->gl_add_op(p);
}

void tglTexEnvi(TGLenum target, TGLenum pname, TGLint param) {
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

void tglTexParameteri(TGLenum target, TGLenum pname, TGLint param) {
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

void tglGenTextures(TGLsizei n, TGLuint *textures) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	c->gl_GenTextures(n, textures);
}

void tglDeleteTextures(TGLsizei n, const TGLuint *textures) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	c->gl_DeleteTextures(n, textures);
}

void tglPixelStorei(TGLenum pname, TGLint param) {
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

void tglPushName(TGLuint name) {
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

void tglLoadName(TGLuint name) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_LoadName;
	p[1].i = name;

	c->gl_add_op(p);
}

TGLint tglRenderMode(TGLenum mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	return c->gl_RenderMode(mode);
}

void tglSelectBuffer(TGLsizei size, TGLuint *buffer) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	c->gl_SelectBuffer(size, buffer);
}

// lists

void tglCallList(TGLuint list) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_CallList;
	p[1].ui = list;

	c->gl_add_op(p);
}

void tglNewList(TGLuint list, TGLenum mode) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	c->gl_NewList(list, mode);
}

void tglEndList() {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	c->gl_EndList();
}

TGLboolean tglIsList(TGLuint list) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	TinyGL::GLList *l = c->find_list(list);
	return (l != nullptr);
}

TGLuint tglGenLists(TGLsizei range) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	return c->gl_GenLists(range);
}

// arrays

void tglArrayElement(TGLint i) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_ArrayElement;
	p[1].i = i;

	c->gl_add_op(p);
}

void tglDrawArrays(TGLenum mode, TGLint first, TGLsizei count) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_DrawArrays;
	p[1].i = mode;
	p[2].i = first;
	p[3].i = count;

	c->gl_add_op(p);
}

void tglDrawElements(TGLenum mode, TGLsizei count, TGLenum type, const TGLvoid *indices) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_DrawElements;
	p[1].i = mode;
	p[2].i = count;
	p[3].i = type;
	p[4].p = const_cast<void *>(indices);

	c->gl_add_op(p);
}

void tglEnableClientState(TGLenum array) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_EnableClientState;

	switch (array) {
	case TGL_VERTEX_ARRAY:
		p[1].i = VERTEX_ARRAY;
		break;
	case TGL_NORMAL_ARRAY:
		p[1].i = NORMAL_ARRAY;
		break;
	case TGL_COLOR_ARRAY:
		p[1].i = COLOR_ARRAY;
		break;
	case TGL_TEXTURE_COORD_ARRAY:
		p[1].i = TEXCOORD_ARRAY;
		break;
	default:
		assert(0);
		break;
	}

	c->gl_add_op(p);
}

void tglDisableClientState(TGLenum array) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[2];

	p[0].op = TinyGL::OP_DisableClientState;

	switch (array) {
	case TGL_VERTEX_ARRAY:
		p[1].i = ~VERTEX_ARRAY;
		break;
	case TGL_NORMAL_ARRAY:
		p[1].i = ~NORMAL_ARRAY;
		break;
	case TGL_COLOR_ARRAY:
		p[1].i = ~COLOR_ARRAY;
		break;
	case TGL_TEXTURE_COORD_ARRAY:
		p[1].i = ~TEXCOORD_ARRAY;
		break;
	default:
		assert(0);
		break;
	}

	c->gl_add_op(p);
}

void tglVertexPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_VertexPointer;
	p[1].i = size;
	p[2].i = type;
	p[3].i = stride;
	p[4].p = const_cast<void *>(pointer);

	c->gl_add_op(p);
}

void tglColorPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_ColorPointer;
	p[1].i = size;
	p[2].i = type;
	p[3].i = stride;
	p[4].p = const_cast<void *>(pointer);

	c->gl_add_op(p);
}

void tglNormalPointer(TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[4];

	p[0].op = TinyGL::OP_NormalPointer;
	p[1].i = type;
	p[2].i = stride;
	p[3].p = const_cast<void *>(pointer);

	c->gl_add_op(p);
}

void tglTexCoordPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();
	TinyGL::GLParam p[5];

	p[0].op = TinyGL::OP_TexCoordPointer;
	p[1].i = size;
	p[2].i = type;
	p[3].i = stride;
	p[4].p = const_cast<void *>(pointer);

	c->gl_add_op(p);
}

// gets

void tglGetIntegerv(TGLenum pname, TGLint *data) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	c->gl_GetIntegerv(pname, data);
}

void tglGetFloatv(TGLenum pname, TGLfloat *data) {
	TinyGL::GLContext *c = TinyGL::gl_get_context();

	c->gl_GetFloatv(pname, data);
}
