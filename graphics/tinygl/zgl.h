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

#ifndef _tgl_zgl_h_
#define _tgl_zgl_h_

#include "common/util.h"
#include "common/textconsole.h"
#include "common/array.h"
#include "common/list.h"
#include "common/scummsys.h"

#include "graphics/tinygl/gl.h"
#include "graphics/tinygl/zbuffer.h"
#include "graphics/tinygl/zmath.h"
#include "graphics/tinygl/zblit.h"
#include "graphics/tinygl/zdirtyrect.h"
#include "graphics/tinygl/texelbuffer.h"

namespace TinyGL {

enum {

#define ADD_OP(a,b,c) OP_ ## a ,

#include "graphics/tinygl/opinfo.h"

	DUMMY
};

// initially # of allocated GLVertexes (will grow when necessary)
#define POLYGON_MAX_VERTEX 16

// Max # of specular light pow buffers
#define MAX_SPECULAR_BUFFERS 8
// # of entries in specular buffer
#define SPECULAR_BUFFER_SIZE 1024
// specular buffer granularity
#define SPECULAR_BUFFER_RESOLUTION 1024

#define MAX_MODELVIEW_STACK_DEPTH   35
#define MAX_PROJECTION_STACK_DEPTH  8
#define MAX_TEXTURE_STACK_DEPTH     8
#define MAX_NAME_STACK_DEPTH        64
#define MAX_TEXTURE_LEVELS          11
#define T_MAX_LIGHTS                32

#define VERTEX_HASH_SIZE 1031

#define MAX_DISPLAY_LISTS 1024
#define OP_BUFFER_MAX_SIZE 512

#define TGL_OFFSET_FILL    0x1
#define TGL_OFFSET_LINE    0x2
#define TGL_OFFSET_POINT   0x4

struct GLSpecBuf {
	int shininess_i;
	int last_used;
	float buf[SPECULAR_BUFFER_SIZE + 1];
	struct GLSpecBuf *next;
};

struct GLLight {
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	bool has_specular;
	Vector4 position;
	Vector3 spot_direction;
	float spot_exponent;
	float spot_cutoff;
	float attenuation[3];
	// precomputed values
	float cos_spot_cutoff;
	Vector3 norm_spot_direction;
	Vector3 norm_position;
	// we use a linked list to know which are the enabled lights
	int enabled;
	struct GLLight *next, *prev;
};

struct GLMaterial {
	Vector4 emission;
	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	bool has_specular;
	float shininess;

	// computed values
	int shininess_i;
	int do_specular;
};


struct GLViewport {
	int xmin, ymin, xsize, ysize;
	Vector3 scale;
	Vector3 trans;
	int updated;
};

union GLParam {
	int op;
	float f;
	int i;
	unsigned int ui;
	void *p;
};

struct GLParamBuffer {
	GLParam ops[OP_BUFFER_MAX_SIZE];
	struct GLParamBuffer *next;
};

struct GLList {
	GLParamBuffer *first_op_buffer;
	// TODO: extensions for an hash table or a better allocating scheme
};

struct GLVertex {
	int edge_flag;
	Vector3 normal;
	Vector4 coord;
	Vector4 tex_coord;
	Vector4 color;

	// computed values
	Vector4 ec;                // eye coordinates
	Vector4 pc;                // coordinates in the normalized volume
	int clip_code;        // clip code
	ZBufferPoint zp;      // integer coordinates for the rasterization

	bool operator==(const GLVertex &other) const {
		return	edge_flag == other.edge_flag &&
				normal == other.normal &&
				coord == other.coord && 
				tex_coord == other.tex_coord && 
				color == other.color &&
				ec == other.ec &&
				pc == other.pc && 
				clip_code == other.clip_code &&
				zp == other.zp;
	}

	bool operator!=(const GLVertex &other) const {
		return !(*this == other);
	}
};

struct GLImage {
	Graphics::TexelBuffer *pixmap;
	int xsize, ysize;
};

// textures

#define TEXTURE_HASH_TABLE_SIZE 256

struct GLTexture {
	GLImage images[MAX_TEXTURE_LEVELS];
	unsigned int handle;
	int versionNumber;
	struct GLTexture *next, *prev;
	bool disposed;
};


// shared state

struct GLSharedState {
	GLList **lists;
	GLTexture **texture_hash_table;
};

/**
 * A linear allocator implementation.
 * The allocator can be initialized to a specific buffer size only once.
 * The allocation scheme is pretty simple: pointers are returned relative to a current memory position,
 * the allocator starts with an offset of 0 and increases its offset by the allocated amount every time.
 * Memory is released through the method free(), care has to be taken to call the destructors of the deallocated objects either manually (for complex struct arrays) or
 * by overriding the delete operator (with an empty implementation).
 */
class LinearAllocator {
public:
	LinearAllocator() {
		_memoryBuffer = nullptr;
		_memorySize = 0;
		_memoryPosition = 0;
	}

	void initialize(size_t newSize) {
		assert(_memoryBuffer == nullptr);
		void *newBuffer = gl_malloc(newSize);
		if (newBuffer == nullptr) {
			error("Couldn't allocate memory for linear allocator.");
		}
		_memoryBuffer = newBuffer;
		_memorySize = newSize;
	}

	~LinearAllocator() {
		if (_memoryBuffer != nullptr) {
			gl_free(_memoryBuffer);
		}
	}

	void *allocate(size_t size) {
		if (_memoryPosition + size >= _memorySize) {
			error("Allocator out of memory: couldn't allocate more memory from linear allocator.");
		}
		size_t returnPos = _memoryPosition;
		_memoryPosition += size;
		return ((char *)_memoryBuffer) + returnPos;
	}

	void reset() {
		_memoryPosition = 0;
	}
private:
	void *_memoryBuffer;
	size_t _memorySize;
	size_t _memoryPosition;
};

struct GLContext;

typedef void (*gl_draw_triangle_func)(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);

// display context

struct GLContext {
	// Z buffer
	FrameBuffer *fb;
	Common::Rect renderRect;

	// Internal texture size
	int _textureSize;

	// lights
	GLLight lights[T_MAX_LIGHTS];
	GLLight *first_light;
	Vector4 ambient_light_model;
	int local_light_model;
	int lighting_enabled;
	int light_model_two_side;

	// materials
	GLMaterial materials[2];
	int color_material_enabled;
	int current_color_material_mode;
	int current_color_material_type;

	// textures
	GLTexture *current_texture;
	int texture_2d_enabled;
	int texture_mag_filter;
	int texture_min_filter;
	unsigned int texture_wrap_s;
	unsigned int texture_wrap_t;

	// shared state
	GLSharedState shared_state;

	// current list
	GLParamBuffer *current_op_buffer;
	int current_op_buffer_index;
	int exec_flag, compile_flag, print_flag;

	// matrix
	int matrix_mode;
	Matrix4 *matrix_stack[3];
	Matrix4 *matrix_stack_ptr[3];
	int matrix_stack_depth_max[3];

	Matrix4 matrix_model_view_inv;
	Matrix4 matrix_model_projection;
	int matrix_model_projection_updated;
	int matrix_model_projection_no_w_transform;
	int apply_texture_matrix;

	// viewport
	GLViewport viewport;

	// current state
	int polygon_mode_back;
	int polygon_mode_front;

	int current_front_face;
	int current_shade_model;
	int current_cull_face;
	int cull_face_enabled;
	int normalize_enabled;
	gl_draw_triangle_func draw_triangle_front, draw_triangle_back;

	// selection
	int render_mode;
	unsigned int *select_buffer;
	int select_size;
	unsigned int *select_ptr, *select_hit;
	int select_overflow;
	int select_hits;

	// names
	unsigned int name_stack[MAX_NAME_STACK_DEPTH];
	int name_stack_size;

	// clear
	float clear_depth;
	Vector4 clear_color;

	// current vertex state
	Vector4 current_color;
	Vector4 current_normal;
	Vector4 current_tex_coord;
	int current_edge_flag;

	// glBegin / glEnd
	int in_begin;
	int begin_type;
	int vertex_n, vertex_cnt;
	int vertex_max;
	GLVertex *vertex;

	// opengl 1.1 arrays
	float *vertex_array;
	int vertex_array_size;
	int vertex_array_stride;
	float *normal_array;
	int normal_array_stride;
	float *color_array;
	int color_array_size;
	int color_array_stride;
	float *texcoord_array;
	int texcoord_array_size;
	int texcoord_array_stride;
	int client_states;

	// opengl 1.1 polygon offset
	float offset_factor;
	float offset_units;
	int offset_states;

	int shadow_mode;

	// specular buffer. could probably be shared between contexts,
	// but that wouldn't be 100% thread safe
	GLSpecBuf *specbuf_first;
	int specbuf_used_counter;
	int specbuf_num_buffers;

	// opaque structure for user's use
	void *opaque;
	// resize viewport function
	int (*gl_resize_viewport)(GLContext *c, int *xsize, int *ysize);

	// depth test
	int depth_test;
	int color_mask;

	Common::Rect _scissorRect;

	bool _enableDirtyRectangles;

	// blit test
	Common::List<Graphics::BlitImage *> _blitImages;

	// Draw call queue
	Common::List<Graphics::DrawCall *> _drawCallsQueue;
	Common::List<Graphics::DrawCall *> _previousFrameDrawCallsQueue;
	int _currentAllocatorIndex;
	LinearAllocator _drawCallAllocator[2];
};

extern GLContext *gl_ctx;

void gl_add_op(GLParam *p);

// clip.c
void gl_transform_to_viewport(GLContext *c, GLVertex *v);
void gl_draw_triangle(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_line(GLContext *c, GLVertex *p0, GLVertex *p1);
void gl_draw_point(GLContext *c, GLVertex *p0);

void gl_draw_triangle_point(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_line(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_fill(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_select(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);

// matrix.c
void gl_print_matrix(const float *m);

// light.c
void gl_add_select(GLContext *c, unsigned int zmin, unsigned int zmax);
void gl_enable_disable_light(GLContext *c, int light, int v);
void gl_shade_vertex(GLContext *c, GLVertex *v);

void glInitTextures(GLContext *c);
void glEndTextures(GLContext *c);
GLTexture *alloc_texture(GLContext *c, int h);
void free_texture(GLContext *c, int h);
void free_texture(GLContext *c, GLTexture *t);

// image_util.c
void gl_resizeImage(Graphics::PixelBuffer &dest, int xsize_dest, int ysize_dest,
		    const Graphics::PixelBuffer &src, int xsize_src, int ysize_src);
void gl_resizeImageNoInterpolate(Graphics::PixelBuffer &dest, int xsize_dest, int ysize_dest,
				 const Graphics::PixelBuffer &src, int xsize_src, int ysize_src);

void tglIssueDrawCall(Graphics::DrawCall *drawCall);

// zdirtyrect.cpp
void tglDisposeResources(GLContext *c);
void tglDisposeDrawCallLists(TinyGL::GLContext *c);

GLContext *gl_get_context();

// specular buffer "api"
GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, const float shininess);
void specbuf_cleanup(GLContext *c); // free all memory used

void glInit(void *zbuffer, int textureSize);
void glClose();

#ifdef DEBUG
#define dprintf fprintf
#else
#define dprintf
#endif

// glopXXX functions

#define ADD_OP(a,b,c) void glop ## a (GLContext *, GLParam *);
#include "opinfo.h"

// this clip epsilon is needed to avoid some rounding errors after
// several clipping stages

#define CLIP_EPSILON (1E-5)

static inline int gl_clipcode(float x, float y, float z, float w1) {
	float w;

	w = (float)(w1 * (1.0 + CLIP_EPSILON));
	return (x < -w) | ((x > w) << 1) | ((y < -w) << 2) | ((y > w) << 3) | ((z < -w) << 4) | ((z > w) << 5);
}

} // end of namespace TinyGL

#endif
