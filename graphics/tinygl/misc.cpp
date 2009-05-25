
#include "graphics/tinygl/zgl.h"

namespace TinyGL {

void glopViewport(GLContext *c, GLParam *p) {
	int xsize, ysize, xmin, ymin, xsize_req, ysize_req;

	xmin = p[1].i;
	ymin = p[2].i;
	xsize = p[3].i;
	ysize = p[4].i;

	// we may need to resize the zbuffer

	if (c->viewport.xmin != xmin || c->viewport.ymin != ymin ||
			c->viewport.xsize != xsize || c->viewport.ysize != ysize) {

		xsize_req = xmin + xsize;
		ysize_req = ymin + ysize;

		if (c->gl_resize_viewport && c->gl_resize_viewport(c, &xsize_req, &ysize_req) != 0) {
			error("glViewport: error while resizing display");
		}

		xsize = xsize_req - xmin;
		ysize = ysize_req - ymin;
		if (xsize <= 0 || ysize <= 0) {
			error("glViewport: size too small");
		}

		c->viewport.xmin = xmin;
		c->viewport.ymin = ymin;
		c->viewport.xsize = xsize;
		c->viewport.ysize = ysize;

		c->viewport.updated = 1;
	}
}

void glopEnableDisable(GLContext *c, GLParam *p) {
	int code = p[1].i;
	int v = p[2].i;

	switch (code) {
	case TGL_CULL_FACE:
		c->cull_face_enabled = v;
		break;
	case TGL_LIGHTING:
		c->lighting_enabled = v;
		break;
	case TGL_COLOR_MATERIAL:
		c->color_material_enabled = v;
		break;
	case TGL_TEXTURE_2D:
		c->texture_2d_enabled=v;
		break;
	case TGL_NORMALIZE:
		c->normalize_enabled=v;
		break;
	case TGL_DEPTH_TEST:
		c->depth_test = v;
		break;
	case TGL_POLYGON_OFFSET_FILL:
		if (v)
			c->offset_states |= TGL_OFFSET_FILL;
		else
			c->offset_states &= ~TGL_OFFSET_FILL;
		break; 
	case TGL_POLYGON_OFFSET_POINT:
		if (v)
			c->offset_states |= TGL_OFFSET_POINT;
		else
			c->offset_states &= ~TGL_OFFSET_POINT;
		break; 
	case TGL_POLYGON_OFFSET_LINE:
		if (v)
			c->offset_states |= TGL_OFFSET_LINE;
		else
			c->offset_states &= ~TGL_OFFSET_LINE;
		break; 
	case TGL_SHADOW_MASK_MODE:
		if (v)
			c->shadow_mode |= 1;
		else
			c->shadow_mode &= ~1;
		break; 
	case TGL_SHADOW_MODE:
		if (v)
			c->shadow_mode |= 2;
		else
			c->shadow_mode &= ~2;
		break; 
	default:
		if (code >= TGL_LIGHT0 && code < TGL_LIGHT0 + T_MAX_LIGHTS) {
			gl_enable_disable_light(c, code - TGL_LIGHT0, v);
		} else {
			//warning("glEnableDisable: 0x%X not supported.", code);
		}
		break;
	}
}

void glopShadeModel(GLContext *c, GLParam *p) {
	int code = p[1].i;
	c->current_shade_model = code;
}

void glopCullFace(GLContext *c, GLParam *p) {
	int code = p[1].i;
	c->current_cull_face = code;
}

void glopFrontFace(GLContext *c, GLParam *p) {
	int code = p[1].i;
	c->current_front_face = code;
}

void glopPolygonMode(GLContext *c, GLParam *p) {
	int face = p[1].i;
	int mode = p[2].i;
  
	switch(face) {
	case TGL_BACK:
		c->polygon_mode_back = mode;
		break;
	case TGL_FRONT:
		c->polygon_mode_front = mode;
		break;
	case TGL_FRONT_AND_BACK:
		c->polygon_mode_front = mode;
		c->polygon_mode_back = mode;
		break;
	default:
		assert(0);
	}
}

void glopHint(GLContext *, GLParam *) {
	// do nothing
}

void  glopPolygonOffset(GLContext *c, GLParam *p) {
	c->offset_factor = p[1].f;
	c->offset_units = p[2].f;
}

} // end of namespace TinyGL
