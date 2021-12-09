/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "graphics/tinygl/zgl.h"
#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zdirtyrect.h"

namespace TinyGL {

GLContext *gl_ctx;

void GLContext::initSharedState() {
	GLSharedState *s = &shared_state;
	s->lists = (GLList **)gl_zalloc(sizeof(GLList *) * MAX_DISPLAY_LISTS);
	s->texture_hash_table = (GLTexture **)gl_zalloc(sizeof(GLTexture *) * TEXTURE_HASH_TABLE_SIZE);

	alloc_texture(0);
}

void GLContext::endSharedState() {
	GLSharedState *s = &shared_state;

	uint h = 0;
	free_texture(h);
	for (int i = 0; i < MAX_DISPLAY_LISTS; i++) {
		// TODO
	}
	gl_free(s->lists);

	gl_free(s->texture_hash_table);
}

void createContext(int screenW, int screenH, Graphics::PixelFormat pixelFormat, int textureSize, bool dirtyRectsEnable) {
	assert(gl_ctx == nullptr);
	GLContext *c = new GLContext();
	gl_ctx = c;
	c->init(screenW, screenH, pixelFormat, textureSize, dirtyRectsEnable);
}

void GLContext::init(int screenW, int screenH, Graphics::PixelFormat pixelFormat, int textureSize, bool dirtyRectsEnable) {
	GLViewport *v;

	_enableDirtyRectangles = dirtyRectsEnable;

	fb = new TinyGL::FrameBuffer(screenW, screenH, pixelFormat);
	renderRect = Common::Rect(0, 0, screenW, screenH);

	if ((textureSize & (textureSize - 1)))
		error("glInit: texture size not power of two: %d", textureSize);
	if (textureSize <= 1 || textureSize > 4096)
		error("glInit: texture size not allowed: %d", textureSize);
	_textureSize = textureSize;
	fb->setTextureSizeAndMask(textureSize, (textureSize - 1) << ZB_POINT_ST_FRAC_BITS);

	// allocate GLVertex array
	vertex_max = POLYGON_MAX_VERTEX;
	vertex = (GLVertex *)gl_malloc(POLYGON_MAX_VERTEX * sizeof(GLVertex));

	// viewport
	v = &viewport;
	v->xmin = 0;
	v->ymin = 0;
	v->xsize = screenW;
	v->ysize = screenH;
	v->updated = 1;

	// shared state
	initSharedState();

	// lists
	exec_flag = 1;
	compile_flag = 0;
	print_flag = 0;

	in_begin = 0;

	// lights
	for (int i = 0; i < T_MAX_LIGHTS; i++) {
		GLLight *l = &lights[i];
		l->ambient = Vector4(0, 0, 0, 1);
		if (i == 0) {
			l->diffuse = Vector4(1, 1, 1, 1);
			l->specular = Vector4(1, 1, 1, 1);
			l->has_specular = true;
		} else {
			l->diffuse = Vector4(0, 0, 0, 1);
			l->specular = Vector4(0, 0, 0, 1);
			l->has_specular = false;
		}
		l->position = Vector4(0, 0, 1, 0);
		l->spot_direction = Vector3(0, 0, -1);
		l->spot_exponent = 0;
		l->spot_cutoff = 180;
		l->attenuation[0] = 1;
		l->attenuation[1] = 0;
		l->attenuation[2] = 0;
		l->cos_spot_cutoff = -1.0f;
		l->norm_spot_direction = Vector3(0, 0, -1);
		l->norm_position = Vector3(0, 0, 1);
		l->enabled = 0;
		l->next = NULL;
		l->prev = NULL;
	}
	first_light = NULL;
	ambient_light_model = Vector4(0.2f, 0.2f, 0.2f, 1);
	local_light_model = 0;
	lighting_enabled = 0;
	light_model_two_side = 0;

	// default materials */
	for (int i = 0; i < 2; i++) {
		GLMaterial *m = &materials[i];
		m->emission = Vector4(0, 0, 0, 1);
		m->ambient = Vector4(0.2f, 0.2f, 0.2f, 1);
		m->diffuse = Vector4(0.8f, 0.8f, 0.8f, 1);
		m->specular = Vector4(0, 0, 0, 1);
		m->has_specular = false;
		m->shininess = 0;
	}
	current_color_material_mode = TGL_FRONT_AND_BACK;
	current_color_material_type = TGL_AMBIENT_AND_DIFFUSE;
	color_material_enabled = 0;

	// textures
	glInitTextures();

	// default state
	current_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	current_normal = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

	current_edge_flag = 1;

	current_tex_coord = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	polygon_mode_front = TGL_FILL;
	polygon_mode_back = TGL_FILL;

	current_front_face = 0; // 0 = GL_CCW  1 = GL_CW
	current_cull_face = TGL_BACK;
	current_shade_model = TGL_SMOOTH;
	cull_face_enabled = 0;

	// clear
	clear_color = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	clear_depth = 1.0f;

	// selection
	render_mode = TGL_RENDER;
	select_buffer = NULL;
	name_stack_size = 0;

	// blending
	blending_enabled = false;

	// alpha test
	alpha_test_enabled = false;

	// depth test
	depth_test = false;

	// matrix
	matrix_mode = 0;

	matrix_stack_depth_max[0] = MAX_MODELVIEW_STACK_DEPTH;
	matrix_stack_depth_max[1] = MAX_PROJECTION_STACK_DEPTH;
	matrix_stack_depth_max[2] = MAX_TEXTURE_STACK_DEPTH;

	for (int i = 0; i < 3; i++) {
		matrix_stack[i] = (Matrix4 *)gl_zalloc(matrix_stack_depth_max[i] * sizeof(Matrix4));
		matrix_stack_ptr[i] = matrix_stack[i];
	}

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglMatrixMode(TGL_TEXTURE);
	tglLoadIdentity();
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);

	tglAlphaFunc(TGL_ALWAYS, 0.f);

	tglDepthFunc(TGL_LESS);

	matrix_model_projection_updated = 1;

	// opengl 1.1 arrays
	client_states = 0;

	// opengl 1.1 polygon offset
	offset_states = 0;
	offset_factor = 0.0f;
	offset_units = 0.0f;

	// shadow mode
	shadow_mode = 0;

	// clear the resize callback function pointer
	gl_resize_viewport = NULL;

	// specular buffer
	specbuf_first = NULL;
	specbuf_used_counter = 0;
	specbuf_num_buffers = 0;

	color_mask = (1 << 24) | (1 << 16) | (1 << 8) | (1 << 0);

	const int kDrawCallMemory = 5 * 1024 * 1024;

	_currentAllocatorIndex = 0;
	_drawCallAllocator[0].initialize(kDrawCallMemory);
	_drawCallAllocator[1].initialize(kDrawCallMemory);
	_enableDirtyRectangles = true;
	_debugRectsEnabled = false;

	TinyGL::Internal::tglBlitResetScissorRect();
}

void destroyContext() {
	GLContext *c = gl_get_context();
	assert(c);
	c->deinit();
	delete c;
	gl_ctx = nullptr;
}

void GLContext::deinit() {
	disposeDrawCallLists();
	disposeResources();

	specbuf_cleanup();
	for (int i = 0; i < 3; i++)
		gl_free(matrix_stack[i]);
	endSharedState();
	gl_free(vertex);
}

} // end of namespace TinyGL
