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

#ifndef _tgl_zgl_h_
#define _tgl_zgl_h_

#include "common/util.h"
#include "common/textconsole.h"
#include "common/array.h"
#include "common/list.h"
#include "common/scummsys.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"
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
	uint ui;
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
	float fog_factor;

	// computed values
	Vector4 ec;           // eye coordinates
	Vector4 pc;           // coordinates in the normalized volume
	int clip_code;        // clip code
	ZBufferPoint zp;      // integer coordinates for the rasterization

	bool operator==(const GLVertex &other) const {
		return
			edge_flag == other.edge_flag &&
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
	TexelBuffer *pixmap;
	int xsize, ysize;
};

// textures

#define TEXTURE_HASH_TABLE_SIZE 256

struct GLTexture {
	GLImage images[MAX_TEXTURE_LEVELS];
	uint handle;
	int versionNumber;
	struct GLTexture *next, *prev;
	bool disposed;
};

struct tglColorAssociation {
	Graphics::PixelFormat pf;
	TGLuint format;
	TGLuint type;
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
		return ((byte *)_memoryBuffer) + returnPos;
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

	// blending
	bool blending_enabled;
	int source_blending_factor;
	int destination_blending_factor;

	// alpha blending
	bool alpha_test_enabled;
	int alpha_test_func;
	int alpha_test_ref_val;

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
	GLTexture *current_texture, *default_texture;
	uint maxTextureName;
	bool texture_2d_enabled;
	int texture_mag_filter;
	int texture_min_filter;
	uint texture_wrap_s;
	uint texture_wrap_t;
	Common::Array<struct tglColorAssociation> colorAssociationList;

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
	uint *select_buffer;
	int select_size;
	uint *select_ptr, *select_hit;
	int select_overflow;
	int select_hits;

	// names
	uint name_stack[MAX_NAME_STACK_DEPTH];
	int name_stack_size;

	// clear
	float clear_depth;
	Vector4 clear_color;
	int clear_stencil;

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
	TGLvoid *vertex_array;
	int vertex_array_size;
	int vertex_array_stride;
	int vertex_array_type;
	TGLvoid *normal_array;
	int normal_array_stride;
	int normal_array_type;
	TGLvoid *color_array;
	int color_array_size;
	int color_array_stride;
	int color_array_type;
	TGLvoid *texcoord_array;
	int texcoord_array_size;
	int texcoord_array_stride;
	int texcoord_array_type;
	int client_states;

	// opengl 1.1 polygon offset
	float offset_factor;
	float offset_units;
	int offset_states;

	// specular buffer. could probably be shared between contexts,
	// but that wouldn't be 100% thread safe
	GLSpecBuf *specbuf_first;
	int specbuf_used_counter;
	int specbuf_num_buffers;

	// opaque structure for user's use
	void *opaque;
	// resize viewport function
	int (*gl_resize_viewport)(int *xsize, int *ysize);

	// depth test
	bool depth_test_enabled;
	int depth_func;
	bool depth_write_mask;

	// stencil
	bool stencil_test_enabled;
	int stencil_test_func;
	int stencil_ref_val;
	uint stencil_mask;
	uint stencil_write_mask;
	int stencil_sfail;
	int stencil_dpfail;
	int stencil_dppass;

	bool color_mask_red;
	bool color_mask_green;
	bool color_mask_blue;
	bool color_mask_alpha;

	bool fog_enabled;
	int fog_mode;
	Vector4 fog_color;
	float fog_density;
	float fog_start;
	float fog_end;

	Common::Rect _scissorRect;

	bool _enableDirtyRectangles;

	// blit test
	Common::List<BlitImage *> _blitImages;

	// Draw call queue
	Common::List<DrawCall *> _drawCallsQueue;
	Common::List<DrawCall *> _previousFrameDrawCallsQueue;
	int _currentAllocatorIndex;
	LinearAllocator _drawCallAllocator[2];
	bool _debugRectsEnabled;
	bool _profilingEnabled;

	void gl_vertex_transform(GLVertex *v);
	void gl_calc_fog_factor(GLVertex *v);

public:
	// The glob* functions exposed to public, however they are only for internal use.
	// Calling them from outside of TinyGL is forbidden
	#define ADD_OP(a, b, d) void glop ## a (GLParam *p);
	#include "graphics/tinygl/opinfo.h"

	void gl_add_op(GLParam *p);
	void gl_compile_op(GLParam *p);

	void gl_eval_viewport();
	void gl_transform_to_viewport(GLVertex *v);
	void gl_draw_triangle(GLVertex *p0, GLVertex *p1, GLVertex *p2);
	void gl_draw_line(GLVertex *p0, GLVertex *p1);
	void gl_draw_point(GLVertex *p0);

	static void gl_draw_triangle_point(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
	static void gl_draw_triangle_line(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
	static void gl_draw_triangle_fill(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
	static void gl_draw_triangle_select(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
	void gl_draw_triangle_clip(GLVertex *p0, GLVertex *p1, GLVertex *p2, int clip_bit);

	void gl_add_select(uint zmin, uint zmax);
	void gl_add_select1(int z1, int z2, int z3);
	void gl_enable_disable_light(int light, int v);
	void gl_shade_vertex(GLVertex *v);

	void gl_GetIntegerv(TGLenum pname, TGLint *data);
	void gl_GetFloatv(TGLenum pname, TGLfloat *data);

	GLTexture *alloc_texture(uint h);
	GLTexture *find_texture(uint h);
	void free_texture(GLTexture *t);
	void gl_GenTextures(TGLsizei n, TGLuint *textures);
	void gl_DeleteTextures(TGLsizei n, const TGLuint *textures);

	void gl_resizeImage(Graphics::PixelBuffer &dest, int xsize_dest, int ysize_dest,
	                    const Graphics::PixelBuffer &src, int xsize_src, int ysize_src);
	void gl_resizeImageNoInterpolate(Graphics::PixelBuffer &dest, int xsize_dest, int ysize_dest, const Graphics::PixelBuffer &src, int xsize_src, int ysize_src);

	void issueDrawCall(DrawCall *drawCall);
	void disposeResources();
	void disposeDrawCallLists();

	void presentBufferDirtyRects(Common::List<Common::Rect> &dirtyAreas);
	void presentBufferSimple(Common::List<Common::Rect> &dirtyAreas);

	void debugDrawRectangle(Common::Rect rect, int r, int g, int b);

	GLSpecBuf *specbuf_get_buffer(const int shininess_i, const float shininess);
	void specbuf_cleanup();

	TGLint gl_RenderMode(TGLenum mode);
	void gl_SelectBuffer(TGLsizei size, TGLuint *buffer);

	GLList *alloc_list(int list);
	GLList *find_list(uint list);
	void delete_list(int list);
	void gl_NewList(TGLuint list, TGLenum mode);
	void gl_EndList();
	TGLboolean gl_IsList(TGLuint list);
	TGLuint gl_GenLists(TGLsizei range);

	void initSharedState();
	void endSharedState();

	void init(int screenW, int screenH, Graphics::PixelFormat pixelFormat, int textureSize,
	          bool enableStencilBuffer, bool dirtyRectsEnable, uint32 drawCallMemorySize);
	void deinit();

	void gl_print_matrix(const float *m);
	void gl_debug(int mode) {
		print_flag = mode;
	}
};

extern GLContext *gl_ctx;
GLContext *gl_get_context();

#define VERTEX_ARRAY    0x0001
#define COLOR_ARRAY     0x0002
#define NORMAL_ARRAY    0x0004
#define TEXCOORD_ARRAY  0x0008

// this clip epsilon is needed to avoid some rounding errors after
// several clipping stages

#define CLIP_EPSILON (1E-5)

static inline int gl_clipcode(float x, float y, float z, float w1) {
	float w;

	w = (float)(w1 * (1.0 + CLIP_EPSILON));
	return (x < -w) | ((x > w) << 1) | ((y < -w) << 2) | ((y > w) << 3) | ((z < -w) << 4) | ((z > w) << 5);
}

static inline float clampf(float a, float min, float max) {
	if (a < min)
		return min;
	if (a > max)
		return max;
	else
		return a;
}

} // end of namespace TinyGL

#endif
