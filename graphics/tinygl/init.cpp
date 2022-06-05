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

void createContext(int screenW, int screenH, Graphics::PixelFormat pixelFormat, int textureSize, bool enableStencilBuffer, bool dirtyRectsEnable) {
	assert(gl_ctx == nullptr);
	gl_ctx = new GLContext();
	gl_ctx->init(screenW, screenH, pixelFormat, textureSize, enableStencilBuffer, dirtyRectsEnable);
}

void GLContext::init(int screenW, int screenH, Graphics::PixelFormat pixelFormat, int textureSize, bool enableStencilBuffer, bool dirtyRectsEnable) {
	GLViewport *v;

	_enableDirtyRectangles = dirtyRectsEnable;

	fb = new TinyGL::FrameBuffer(screenW, screenH, pixelFormat, enableStencilBuffer);
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
		l->next = nullptr;
		l->prev = nullptr;
	}
	first_light = nullptr;
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
	texture_2d_enabled = false;
	current_texture = alloc_texture(0);
	maxTextureName = 0;
	texture_mag_filter = TGL_LINEAR;
	texture_min_filter = TGL_NEAREST_MIPMAP_LINEAR;
#if defined(SCUMM_LITTLE_ENDIAN)
	colorAssociationList.push_back({Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), TGL_RGBA, TGL_UNSIGNED_BYTE});
	colorAssociationList.push_back({Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0),  TGL_RGB,  TGL_UNSIGNED_BYTE});
#else
	colorAssociationList.push_back({Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0), TGL_RGBA, TGL_UNSIGNED_BYTE});
	colorAssociationList.push_back({Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0),  TGL_RGB,  TGL_UNSIGNED_BYTE});
#endif
	colorAssociationList.push_back({Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),  TGL_RGB,  TGL_UNSIGNED_SHORT_5_6_5});
	colorAssociationList.push_back({Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0),  TGL_RGBA, TGL_UNSIGNED_SHORT_5_5_5_1});
	colorAssociationList.push_back({Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0),  TGL_RGBA, TGL_UNSIGNED_SHORT_4_4_4_4});

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

	// fog
	fog_enabled = false;
	fog_mode = TGL_EXP;
	fog_color = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	fog_density = 1.0f;
	fog_start = 0.0f;
	fog_end = 0.0f;

	// clear
	clear_color = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	clear_depth = 1.0f;
	clear_stencil = 0;

	// selection
	render_mode = TGL_RENDER;
	select_buffer = nullptr;
	name_stack_size = 0;

	// blending
	blending_enabled = false;
	source_blending_factor = TGL_ONE;
	destination_blending_factor = TGL_ZERO;

	// alpha test
	alpha_test_enabled = false;
	alpha_test_func = TGL_ALWAYS;
	alpha_test_ref_val = 0;

	// depth test
	depth_test_enabled = false;
	depth_func = TGL_LESS;
	depth_write_mask = true;

	// stencil
	stencil_test_enabled = false;
	stencil_test_func = TGL_ALWAYS;
	stencil_ref_val = 0;
	stencil_mask = 0xff;
	stencil_write_mask = 0xff;
	stencil_sfail = TGL_KEEP;
	stencil_dpfail = TGL_KEEP;
	stencil_dppass = TGL_KEEP;

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

	matrix_model_projection_updated = 1;

	// opengl 1.1 arrays
	client_states = 0;

	// opengl 1.1 polygon offset
	offset_states = 0;
	offset_factor = 0.0f;
	offset_units = 0.0f;

	// clear the resize callback function pointer
	gl_resize_viewport = nullptr;

	// specular buffer
	specbuf_first = nullptr;
	specbuf_used_counter = 0;
	specbuf_num_buffers = 0;

	// color mask
	color_mask_red = color_mask_green = color_mask_blue = color_mask_alpha = true;

	const int kDrawCallMemory = 5 * 1024 * 1024;

	_currentAllocatorIndex = 0;
	_drawCallAllocator[0].initialize(kDrawCallMemory);
	_drawCallAllocator[1].initialize(kDrawCallMemory);
	_debugRectsEnabled = false;
	_profilingEnabled = false;

	TinyGL::Internal::tglBlitResetScissorRect();
}

GLContext *gl_get_context() {
	return gl_ctx;
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
