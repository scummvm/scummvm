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

namespace TinyGL {

#define MAX_INTEGER             2147483647
#define FLOAT_TO_INTEGER(f)     ((TGLint)(f * MAX_INTEGER))
#define VALUE_TO_BOOLEAN(f)     ((f) ? TGL_TRUE : TGL_FALSE)

void GLContext::gl_get_pname(TGLenum pname, union uglValue *data, eDataType &dataType) {
	int mnr = 0;

	switch (pname) {
	case TGL_ACCUM_ALPHA_BITS:
		// fall through
	case TGL_ACCUM_BLUE_BITS:
		// fall through
	case TGL_ACCUM_CLEAR_VALUE:
		// fall through
	case TGL_ACCUM_GREEN_BITS:
		// fall through
	case TGL_ACCUM_RED_BITS:
		data->_int = 0;
		dataType = kIntType;
		break;
	case TGL_ALIASED_LINE_WIDTH_RANGE:
		error("gl_get_pname: TGL_ALIASED_LINE_WIDTH_RANGE option not implemented");
		break;
	case TGL_ALIASED_POINT_SIZE_RANGE:
		error("gl_get_pname: TGL_ALIASED_POINT_SIZE_RANGE option not implemented");
		break;
	case TGL_ALPHA_BIAS:
		// fall through
	case TGL_RED_BIAS:
		// fall through
	case TGL_GREEN_BIAS:
		// fall through
	case TGL_BLUE_BIAS:
		// fall through
	case TGL_DEPTH_BIAS:
		data->_float = 0.0f;
		dataType = kFloatType;
		break;
	case TGL_ALPHA_SCALE:
		// fall through
	case TGL_RED_SCALE:
		// fall through
	case TGL_GREEN_SCALE:
		// fall through
	case TGL_DEPTH_SCALE:
		// fall through
	case TGL_BLUE_SCALE:
		data->_float = 1.0f;
		dataType = kFloatType;
		break;
	case TGL_ALPHA_BITS:
		data->_int = fb->getPixelFormat().aBits();
		dataType = kIntType;
		break;
	case TGL_RED_BITS:
		data->_int = fb->getPixelFormat().rBits();
		dataType = kIntType;
		break;
	case TGL_GREEN_BITS:
		data->_int = fb->getPixelFormat().gBits();
		dataType = kIntType;
		break;
	case TGL_BLUE_BITS:
		data->_int = fb->getPixelFormat().bBits();
		dataType = kIntType;
		break;
	case TGL_ALPHA_TEST:
		data->_int = (TGLint)alpha_test_enabled;
		dataType = kIntType;
		break;
	case TGL_ALPHA_TEST_FUNC:
		data->_int = alpha_test_func;
		dataType = kIntType;
		break;
	case TGL_ALPHA_TEST_REF:
		data->_float = alpha_test_ref_val / 255.0f;
		dataType = kFloatType;
		break;
	case TGL_ATTRIB_STACK_DEPTH:
		error("gl_get_pname: TGL_ALIASED_POINT_SIZE_RANGE option not implemented");
		break;
	case TGL_AUTO_NORMAL:
		data->_int = 0;
		dataType = kIntType;
		break;
	case TGL_AUX_BUFFERS:
		error("gl_get_pname: TGL_AUX_BUFFERS option not implemented");
		break;
	case TGL_BLEND:
		data->_int = (TGLint)blending_enabled;
		dataType = kIntType;
		break;
	case TGL_BLEND_DST:
		data->_int = destination_blending_factor;
		dataType = kIntType;
		break;
	case TGL_BLEND_SRC:
		data->_int = source_blending_factor;
		dataType = kIntType;
		break;
	case TGL_CLIENT_ATTRIB_STACK_DEPTH:
		error("gl_get_pname: TGL_CLIENT_ATTRIB_STACK_DEPTH option not implemented");
		break;
	case TGL_CLIP_PLANE0:
		// fall through
	case TGL_CLIP_PLANE1:
		// fall through
	case TGL_CLIP_PLANE2:
		// fall through
	case TGL_CLIP_PLANE3:
		// fall through
	case TGL_CLIP_PLANE4:
		// fall through
	case TGL_CLIP_PLANE5:
		error("gl_get_pname: TGL_CLIP_PLANEx option not implemented");
		break;
	case TGL_COLOR_ARRAY:
		data->_int = (TGLint)(color_array != nullptr);
		dataType = kIntType;
		break;
	case TGL_COLOR_ARRAY_SIZE:
		data->_int = color_array_size;
		dataType = kIntType;
		break;
	case TGL_COLOR_ARRAY_STRIDE:
		data->_int = color_array_stride;
		dataType = kIntType;
		break;
	case TGL_COLOR_ARRAY_TYPE:
		data->_int = color_array_type;
		dataType = kIntType;
		break;
	case TGL_COLOR_CLEAR_VALUE:
		data->_float4[0] = clear_color._v[0];
		data->_float4[1] = clear_color._v[1];
		data->_float4[2] = clear_color._v[2];
		data->_float4[3] = clear_color._v[3];
		dataType = kFloat4Type;
		break;
	case TGL_COLOR_LOGIC_OP:
		error("gl_get_pname: TGL_COLOR_LOGIC_OP option not implemented");
		break;
	case TGL_COLOR_MATERIAL:
		data->_int = (TGLint)color_material_enabled;
		dataType = kIntType;
		break;
	case TGL_COLOR_MATERIAL_FACE:
		data->_int = current_color_material_mode;
		dataType = kIntType;
		break;
	case TGL_COLOR_MATERIAL_PARAMETER:
		data->_int = current_color_material_type;
		dataType = kIntType;
		break;
	case TGL_COLOR_WRITEMASK:
		data->_int4[0] = (TGLint)TGL_TRUE;
		data->_int4[1] = (TGLint)TGL_TRUE;
		data->_int4[2] = (TGLint)TGL_TRUE;
		data->_int4[3] = (TGLint)TGL_TRUE;
		dataType = kInt4Type;
		break;
	case TGL_CULL_FACE:
		data->_int = (TGLint)cull_face_enabled;
		dataType = kIntType;
		break;
	case TGL_CULL_FACE_MODE:
		data->_int = (TGLint)current_cull_face;
		dataType = kIntType;
		break;
	case TGL_CURRENT_COLOR:
		data->_float4[0] = current_color._v[0];
		data->_float4[1] = current_color._v[1];
		data->_float4[2] = current_color._v[2];
		data->_float4[3] = current_color._v[3];
		dataType = kFloat4Type;
		break;
	case TGL_CURRENT_INDEX:
		error("gl_get_pname: TGL_CURRENT_INDEX not supported");
		break;
	case TGL_CURRENT_NORMAL:
		data->_float4[0] = current_normal._v[0];
		data->_float4[1] = current_normal._v[1];
		data->_float4[2] = current_normal._v[2];
		data->_float4[3] = current_normal._v[3];
		dataType = kFloat4Type;
		break;
	case TGL_CURRENT_RASTER_INDEX:
		error("gl_get_pname: TGL_CURRENT_RASTER_INDEX not supported");
		break;
	case TGL_CURRENT_RASTER_COLOR:
		// fall through
	case TGL_CURRENT_RASTER_DISTANCE:
		// fall through
	case TGL_CURRENT_RASTER_POSITION:
		// fall through
	case TGL_CURRENT_RASTER_POSITION_VALID:
		// fall through
	case TGL_CURRENT_RASTER_TEXTURE_COORDS:
		error("gl_get_pname: TGL_CURRENT_RASTER_x options not implemented");
		break;
	case TGL_CURRENT_TEXTURE_COORDS:
		data->_float4[0] = current_tex_coord._v[0];
		data->_float4[1] = current_tex_coord._v[1];
		data->_float4[2] = current_tex_coord._v[2];
		data->_float4[3] = current_tex_coord._v[3];
		dataType = kFloat4Type;
		break;
	case TGL_DEPTH_BITS:
		data->_int = 16;
		dataType = kIntType;
		break;
	case TGL_DEPTH_CLEAR_VALUE:
		data->_float = clear_depth;
		dataType = kFloatType;
		break;
	case TGL_DEPTH_FUNC:
		data->_int = depth_func;
		dataType = kIntType;
		break;
	case TGL_DEPTH_RANGE:
		data->_float2[0] = 1.0f;
		data->_float2[1] = -1.0f;
		dataType = kFloat2Type;
		break;
	case TGL_DEPTH_TEST:
		data->_int = (TGLint)depth_test_enabled;
		dataType = kIntType;
		break;
	case TGL_DEPTH_WRITEMASK:
		data->_int = (TGLint)depth_write_mask;
		dataType = kIntType;
		break;
	case TGL_DITHER:
		data->_int = (TGLint)TGL_FALSE;
		dataType = kIntType;
		break;
	case TGL_DOUBLEBUFFER:
		data->_int = (TGLint)TGL_FALSE;
		dataType = kIntType;
		break;
	case TGL_DRAW_BUFFER:
		error("gl_get_pname: TGL_DRAW_BUFFER option not implemented");
		break;
	case TGL_EDGE_FLAG:
		data->_int = current_edge_flag;
		dataType = kIntType;
		break;
	case TGL_EDGE_FLAG_ARRAY:
		error("gl_get_pname: TGL_EDGE_FLAG_ARRAY option not implemented");
		break;
	case TGL_EDGE_FLAG_ARRAY_STRIDE:
		error("gl_get_pname: TGL_EDGE_FLAG_ARRAY_STRIDE option not implemented");
		break;
	case TGL_FEEDBACK_BUFFER_SIZE:
		// fall through
	case TGL_FEEDBACK_BUFFER_TYPE:
		error("gl_get_pname: TGL_FEEDBACK_BUFFER_x option not implemented");
		break;
	case TGL_FOG:
		data->_int = (TGLint)fog_enabled;
		dataType = kIntType;
		break;
	case TGL_FOG_COLOR:
		data->_float4[0] = fog_color._v[0];
		data->_float4[1] = fog_color._v[1];
		data->_float4[2] = fog_color._v[2];
		data->_float4[3] = fog_color._v[3];
		dataType = kFloat4Type;
		break;
	case TGL_FOG_DENSITY:
		data->_float = fog_density;
		dataType = kFloatType;
		break;
	case TGL_FOG_END:
		data->_float = fog_end;
		dataType = kFloatType;
		break;
	case TGL_FOG_HINT:
		data->_int = TGL_DONT_CARE;
		dataType = kIntType;
		break;
	case TGL_FOG_INDEX:
		error("gl_get_pname: TGL_FOG_INDEX not supported");
		break;
	case TGL_FOG_MODE:
		data->_int = fog_mode;
		dataType = kIntType;
		break;
	case TGL_FOG_START:
		data->_float = fog_start;
		dataType = kFloatType;
		break;
	case TGL_FRONT_FACE:
		data->_int = (current_front_face == 0) ? TGL_CCW : TGL_CW;
		dataType = kIntType;
		break;
	case TGL_INDEX_ARRAY:
		// fall through
	case TGL_INDEX_ARRAY_STRIDE:
		// fall through
	case TGL_INDEX_ARRAY_TYPE:
		// fall through
	case TGL_INDEX_BITS:
		// fall through
	case TGL_INDEX_CLEAR_VALUE:
		// fall through
	case TGL_INDEX_LOGIC_OP:
		// fall through
	case TGL_INDEX_MODE:
		// fall through
	case TGL_INDEX_OFFSET:
		// fall through
	case TGL_INDEX_SHIFT:
		// fall through
	case TGL_INDEX_WRITEMASK:
		error("gl_get_pname: TGL_INDEX_x not supported");
		break;
	case TGL_LIGHT0:
		data->_int = (TGLint)lights[0].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT1:
		data->_int = (TGLint)lights[1].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT2:
		data->_int = (TGLint)lights[2].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT3:
		data->_int = (TGLint)lights[3].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT4:
		data->_int = (TGLint)lights[4].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT5:
		data->_int = (TGLint)lights[5].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT6:
		data->_int = (TGLint)lights[6].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT7:
		data->_int = (TGLint)lights[7].enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHTING:
		data->_int = (TGLint)lighting_enabled;
		dataType = kIntType;
		break;
	case TGL_LIGHT_MODEL_AMBIENT:
		data->_float4[0] = ambient_light_model._v[0];
		data->_float4[1] = ambient_light_model._v[1];
		data->_float4[2] = ambient_light_model._v[2];
		data->_float4[3] = ambient_light_model._v[3];
		dataType = kFloat4Type;
		break;
	case TGL_LIGHT_MODEL_COLOR_CONTROL:
		error("gl_get_pname: TGL_LIGHT_MODEL_COLOR_CONTROL option not implemented");
		break;
	case TGL_LIGHT_MODEL_LOCAL_VIEWER:
		data->_int = local_light_model;
		dataType = kIntType;
		break;
	case TGL_LIGHT_MODEL_TWO_SIDE:
		data->_int = light_model_two_side;
		dataType = kIntType;
		break;
	case TGL_LINE_SMOOTH:
		data->_int = (TGLint)TGL_FALSE;
		dataType = kIntType;
		break;
	case TGL_LINE_SMOOTH_HINT:
		data->_int = TGL_DONT_CARE;
		dataType = kIntType;
		break;
	case TGL_LINE_STIPPLE:
		// fall through
	case TGL_LINE_STIPPLE_PATTERN:
		// fall through
	case TGL_LINE_STIPPLE_REPEAT:
		// fall through
		error("gl_get_pname: TGL_LINE_STIPPLE_x option not implemented");
		break;
	case TGL_LINE_WIDTH:
		// fall through
	case TGL_LINE_WIDTH_GRANULARITY:
		// fall through
	case TGL_LINE_WIDTH_RANGE:
		error("gl_get_pname: TGL_LINE_x option not implemented");
		break;
	case TGL_LIST_BASE:
		// fall through
	case TGL_LIST_INDEX:
		// fall through
	case TGL_LIST_MODE:
		error("gl_get_pname: TGL_LIST_x option not implemented");
		break;
	case TGL_LOGIC_OP_MODE:
		error("gl_get_pname: TGL_LOGIC_OP_MODE option not implemented");
		break;
	case TGL_MAP1_COLOR_4:
		// fall through
	case TGL_MAP1_GRID_DOMAIN:
		// fall through
	case TGL_MAP1_GRID_SEGMENTS:
		// fall through
	case TGL_MAP1_INDEX:
		// fall through
	case TGL_MAP1_NORMAL:
		// fall through
	case TGL_MAP1_TEXTURE_COORD_1:
		// fall through
	case TGL_MAP1_TEXTURE_COORD_2:
		// fall through
	case TGL_MAP1_TEXTURE_COORD_3:
		// fall through
	case TGL_MAP1_TEXTURE_COORD_4:
		// fall through
	case TGL_MAP1_VERTEX_3:
		// fall through
	case TGL_MAP1_VERTEX_4:
		// fall through
	case TGL_MAP2_COLOR_4:
		// fall through
	case TGL_MAP2_GRID_DOMAIN:
		// fall through
	case TGL_MAP2_GRID_SEGMENTS:
		// fall through
	case TGL_MAP2_INDEX:
		// fall through
	case TGL_MAP2_NORMAL:
		// fall through
	case TGL_MAP2_TEXTURE_COORD_1:
		// fall through
	case TGL_MAP2_TEXTURE_COORD_2:
		// fall through
	case TGL_MAP2_TEXTURE_COORD_3:
		// fall through
	case TGL_MAP2_TEXTURE_COORD_4:
		// fall through
	case TGL_MAP2_VERTEX_3:
		// fall through
	case TGL_MAP2_VERTEX_4:
		error("gl_get_pname: TGL_MAPx option not implemented");
		break;
	case TGL_MAP_COLOR:
		// fall through
	case TGL_MAP_STENCIL:
		error("gl_get_pname: TGL_MAP_x not supported");
		break;
	case TGL_MATRIX_MODE:
		data->_int = matrix_mode;
		dataType = kIntType;
		break;
	case TGL_MAX_ATTRIB_STACK_DEPTH:
		error("gl_get_pname: TGL_MAX_ATTRIB_STACK_DEPTH option not implemented");
		break;
	case TGL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
		error("gl_get_pname: TGL_MAX_CLIENT_ATTRIB_STACK_DEPTH option not implemented");
		break;
	case TGL_MAX_CLIP_PLANES:
		error("gl_get_pname: TGL_MAX_CLIP_PLANES option not implemented");
		break;
	case TGL_MAX_ELEMENTS_INDICES:
		// fall through
	case TGL_MAX_ELEMENTS_VERTICES:
		error("gl_get_pname: TGL_MAX_ELEMENTS_x option not implemented");
		break;
	case TGL_MAX_EVAL_ORDER:
		error("gl_get_pname: TGL_MAX_EVAL_ORDER option not implemented");
		break;
	case TGL_MAX_LIGHTS:
		data->_int = T_MAX_LIGHTS;
		dataType = kIntType;
		break;
	case TGL_MAX_LIST_NESTING:
		error("gl_get_pname: TGL_MAX_LIST_NESTING option not implemented");
		break;
	case TGL_MAX_MODELVIEW_STACK_DEPTH:
		data->_int = MAX_MODELVIEW_STACK_DEPTH;
		dataType = kIntType;
		break;
	case TGL_MAX_NAME_STACK_DEPTH:
		data->_int = MAX_NAME_STACK_DEPTH;
		dataType = kIntType;
		break;
	case TGL_MAX_PIXEL_MAP_TABLE:
		error("gl_get_pname: TGL_MAX_PIXEL_MAP_TABLE option not implemented");
		break;
	case TGL_MAX_PROJECTION_STACK_DEPTH:
		data->_int = MAX_PROJECTION_STACK_DEPTH;
		dataType = kIntType;
		break;
	case TGL_MAX_TEXTURE_SIZE:
		data->_int = _textureSize;
		dataType = kIntType;
		break;
	case TGL_MAX_TEXTURE_STACK_DEPTH:
		data->_int = MAX_TEXTURE_STACK_DEPTH;
		dataType = kIntType;
		break;
	case TGL_MAX_VIEWPORT_DIMS:
		error("gl_get_pname: TGL_MAX_VIEWPORT_DIMS option not implemented");
		break;
	case TGL_TEXTURE_MATRIX:
		mnr++;
		// fall through
	case TGL_PROJECTION_MATRIX:
		mnr++;
		// fall through
	case TGL_MODELVIEW_MATRIX: {
		float *p = &matrix_stack_ptr[mnr]->_m[0][0];
		for (int i = 0; i < 4; i++) {
			data->_float16[i * 4 + 0] = p[0];
			data->_float16[i * 4 + 1] = p[4];
			data->_float16[i * 4 + 2] = p[8];
			data->_float16[i * 4 + 3] = p[12];
			p++;
		}
	}
		dataType = kFloat16Type;
		break;
	case TGL_MODELVIEW_STACK_DEPTH:
		error("gl_get_pname: TGL_MODELVIEW_STACK_DEPTH option not implemented");
		break;
	case TGL_NAME_STACK_DEPTH:
		error("gl_get_pname: TGL_NAME_STACK_DEPTH option not implemented");
		break;
	case TGL_NORMALIZE:
		data->_int = 0;
		dataType = kIntType;
		break;
	case TGL_NORMAL_ARRAY:
		data->_int = (TGLint)(normal_array != nullptr);
		dataType = kIntType;
		break;
	case TGL_NORMAL_ARRAY_STRIDE:
		data->_int = normal_array_stride;
		dataType = kIntType;
		break;
	case TGL_NORMAL_ARRAY_TYPE:
		data->_int = normal_array_type;
		dataType = kIntType;
		break;
	case TGL_PACK_ALIGNMENT:
		// fall through
	case TGL_PACK_LSB_FIRST:
		// fall through
	case TGL_PACK_ROW_LENGTH:
		// fall through
	case TGL_PACK_SKIP_PIXELS:
		// fall through
	case TGL_PACK_SKIP_ROWS:
		// fall through
	case TGL_PACK_SWAP_BYTES:
		error("gl_get_pname: TGL_PACK_x option not implemented");
		break;
	case TGL_PERSPECTIVE_CORRECTION_HINT:
		data->_int = TGL_DONT_CARE;
		break;
	case TGL_PIXEL_MAP_A_TO_A_SIZE:
		// fall through
	case TGL_PIXEL_MAP_B_TO_B_SIZE:
		// fall through
	case TGL_PIXEL_MAP_G_TO_G_SIZE:
		// fall through
	case TGL_PIXEL_MAP_I_TO_A_SIZE:
		// fall through
	case TGL_PIXEL_MAP_I_TO_B_SIZE:
		// fall through
	case TGL_PIXEL_MAP_I_TO_G_SIZE:
		// fall through
	case TGL_PIXEL_MAP_I_TO_I_SIZE:
		// fall through
	case TGL_PIXEL_MAP_I_TO_R_SIZE:
		// fall through
	case TGL_PIXEL_MAP_R_TO_R_SIZE:
		// fall through
	case TGL_PIXEL_MAP_S_TO_S_SIZE:
		error("gl_get_pname: TGL_PIXEL_MAP_x option not implemented");
		break;
	case TGL_POINT_SIZE:
		// fall through
	case TGL_POINT_SIZE_GRANULARITY:
		// fall through
	case TGL_POINT_SIZE_RANGE:
		// fall through
	case TGL_POINT_SMOOTH:
		// fall through
	case TGL_POINT_SMOOTH_HINT:
		error("gl_get_pname: TGL_POINT_x option not implemented");
		break;
	case TGL_POLYGON_MODE:
		data->_float2[0] = polygon_mode_front;
		data->_float2[1] = polygon_mode_back;
		dataType = kFloat2Type;
		break;
	case TGL_POLYGON_OFFSET_FACTOR:
		data->_float = offset_factor;
		dataType = kFloatType;
		break;
	case TGL_POLYGON_OFFSET_FILL:
		data->_int = (offset_states & TGL_OFFSET_FILL) != 0 ? 1 : 0;
		dataType = kIntType;
		break;
	case TGL_POLYGON_OFFSET_LINE:
		data->_int = (offset_states & TGL_OFFSET_LINE) != 0 ? 1 : 0;
		dataType = kIntType;
		break;
	case TGL_POLYGON_OFFSET_POINT:
		data->_int = (offset_states & TGL_OFFSET_POINT) != 0 ? 1 : 0;
		dataType = kIntType;
		break;
	case TGL_POLYGON_OFFSET_UNITS:
		data->_float = offset_units;
		dataType = kFloatType;
		break;
	case TGL_POLYGON_SMOOTH:
		// fall through
	case TGL_POLYGON_SMOOTH_HINT:
		error("gl_get_pname: TGL_POLYGON_SMOOTHx option not implemented");
		break;
	case TGL_POLYGON_STIPPLE:
		error("gl_get_pname: TGL_POLYGON_STIPPLE option not implemented");
		break;
	case TGL_PROJECTION_STACK_DEPTH:
		error("gl_get_pname: TGL_PROJECTION_STACK_DEPTH option not implemented");
		break;
	case TGL_READ_BUFFER:
		error("gl_get_pname: TGL_READ_BUFFER option not implemented");
		break;
	case TGL_RENDER_MODE:
		error("gl_get_pname: TGL_RENDER_MODE option not implemented");
		break;
	case TGL_RGBA_MODE:
		data->_int = 1;
		dataType = kIntType;
		break;
	case TGL_SCISSOR_BOX:
		// fall through
	case TGL_SCISSOR_TEST:
		error("gl_get_pname: TGL_SCISSOR_x option not implemented");
		break;
	case TGL_SELECTION_BUFFER_SIZE:
		error("gl_get_pname: TGL_SELECTION_BUFFER_SIZE option not implemented");
		break;
	case TGL_SHADE_MODEL:
		data->_int = current_shade_model;
		dataType = kIntType;
		break;
	case TGL_STENCIL_BITS:
		data->_int = (TGLint)stencil_buffer_supported;
		dataType = kIntType;
		break;
	case TGL_STENCIL_CLEAR_VALUE:
		data->_float = clear_stencil;
		dataType = kFloatType;
		break;
	case TGL_STENCIL_FAIL:
		data->_int = (TGLint)stencil_sfail;
		dataType = kIntType;
		break;
	case TGL_STENCIL_FUNC:
		data->_int = (TGLint)stencil_test_func;
		dataType = kIntType;
		break;
	case TGL_STENCIL_PASS_DEPTH_FAIL:
		data->_int = (TGLint)TGL_FALSE;
		dataType = kIntType;
		break;
	case TGL_STENCIL_PASS_DEPTH_PASS:
		data->_int = (TGLint)TGL_FALSE;
		dataType = kIntType;
		break;
	case TGL_STENCIL_REF:
		data->_int = (TGLint)stencil_ref_val;
		dataType = kIntType;
		break;
	case TGL_STENCIL_TEST:
		data->_int = (TGLint)stencil_test_enabled;
		dataType = kIntType;
		break;
	case TGL_STENCIL_VALUE_MASK:
		error("gl_get_pname: TGL_SCISSOR_BOX option not implemented");
		break;
	case TGL_STENCIL_WRITEMASK:
		data->_int = (TGLint)stencil_write_mask;
		dataType = kUintType;
		break;
	case TGL_STEREO:
		data->_int = (TGLint)TGL_FALSE;
		dataType = kIntType;
		break;
	case TGL_SUBPIXEL_BITS:
		error("gl_get_pname: TGL_SUBPIXEL_BITS option not implemented");
		break;
	case TGL_TEXTURE_1D:
		error("gl_get_pname: TGL_TEXTURE_1D option not implemented");
		break;
	case TGL_TEXTURE_2D:
		data->_int = (TGLint)texture_2d_enabled;
		dataType = kIntType;
		break;
	case TGL_TEXTURE_BINDING_1D:
		// fall through
	case TGL_TEXTURE_BINDING_2D:
		// fall through
	case TGL_TEXTURE_BINDING_3D:
		error("gl_get_pname: TGL_TEXTURE_BINDING_x option not implemented");
		break;
	case TGL_TEXTURE_COORD_ARRAY:
		data->_int = (texcoord_array != nullptr) ? 1 : 0;
		dataType = kIntType;
		break;
	case TGL_TEXTURE_COORD_ARRAY_SIZE:
		data->_int = texcoord_array_size;
		dataType = kIntType;
		break;
	case TGL_TEXTURE_COORD_ARRAY_STRIDE:
		data->_int = texcoord_array_stride;
		dataType = kIntType;
		break;
	case TGL_TEXTURE_COORD_ARRAY_TYPE:
		data->_int = texcoord_array_type;
		dataType = kIntType;
		break;
	case TGL_TEXTURE_GEN_Q:
		// fall through
	case TGL_TEXTURE_GEN_R:
		// fall through
	case TGL_TEXTURE_GEN_S:
		// fall through
	case TGL_TEXTURE_GEN_T:
		error("gl_get_pname: TGL_TEXTURE_GEN_x option not implemented");
		break;
	case TGL_TEXTURE_STACK_DEPTH:
		error("gl_get_pname: TGL_TEXTURE_STACK_DEPTH option not implemented");
		break;
	case TGL_UNPACK_ALIGNMENT:
		// fall through
	case TGL_UNPACK_LSB_FIRST:
		// fall through
	case TGL_UNPACK_ROW_LENGTH:
		// fall through
	case TGL_UNPACK_SKIP_IMAGES:
		// fall through
	case TGL_UNPACK_SKIP_PIXELS:
		// fall through
	case TGL_UNPACK_SKIP_ROWS:
		// fall through
	case TGL_UNPACK_SWAP_BYTES:
		error("gl_get_pname: TGL_UNPACK_x option not implemented");
		break;
	case TGL_VERTEX_ARRAY:
		data->_int = (vertex_array != nullptr) ? 1 : 0;
		dataType = kIntType;
		break;
	case TGL_VERTEX_ARRAY_SIZE:
		data->_int = vertex_array_size;
		dataType = kIntType;
		break;
	case TGL_VERTEX_ARRAY_STRIDE:
		data->_int = vertex_array_stride;
		dataType = kIntType;
		break;
	case TGL_VERTEX_ARRAY_TYPE:
		data->_int = vertex_array_type;
		dataType = kIntType;
		break;
	case TGL_VIEWPORT:
		data->_int4[0] = viewport.xmin;
		data->_int4[1] = viewport.ymin;
		data->_int4[2] = viewport.xsize;
		data->_int4[3] = viewport.ysize;
		dataType = kInt4Type;
		break;
	case TGL_ZOOM_X:
		// fall through
	case TGL_ZOOM_Y:
		error("gl_get_pname: TGL_ZOOM_x option not implemented");
		break;
	default:
		error("gl_get_pname: unknown option");
		break;
	}
}

void GLContext::gl_GetIntegerv(TGLenum pname, TGLint *data) {
	eDataType dataType;
	union uglValue tmpData;

	gl_get_pname(pname, &tmpData, dataType);

	switch (dataType) {
	case kIntType:
		data[0] = tmpData._int;
		break;
	case kInt4Type:
		for (int i = 0; i < 4; i++)
			data[i] = tmpData._int4[i];
		break;
	case kUintType:
		data[0] = tmpData._int;
		break;
	case kFloatType:
		data[0] = FLOAT_TO_INTEGER(tmpData._float);
		break;
	case kFloat2Type:
		for (int i = 0; i < 2; i++)
			data[i] = FLOAT_TO_INTEGER(tmpData._float2[i]);
		break;
	case kFloat4Type:
		for (int i = 0; i < 4; i++)
			data[i] = FLOAT_TO_INTEGER(tmpData._float4[i]);
		break;
	case kFloat16Type:
		for (int i = 0; i < 16; i++)
			data[i] = FLOAT_TO_INTEGER(tmpData._float16[i]);
		break;
	default:
		assert("gl_GetIntegerv: unknown data type");
		break;
	}
}

void GLContext::gl_GetFloatv(TGLenum pname, TGLfloat *data) {
	eDataType dataType;
	union uglValue tmpData;

	gl_get_pname(pname, &tmpData, dataType);

	switch (dataType) {
	case kIntType:
		data[0] = tmpData._int;
		break;
	case kInt4Type:
		for (int i = 0; i < 4; i++)
			data[i] = tmpData._int4[i];
		break;
	case kUintType:
		data[0] = (TGLuint)tmpData._int;
		break;
	case kFloatType:
		data[0] = tmpData._float;
		break;
	case kFloat2Type:
		for (int i = 0; i < 2; i++)
			data[i] = tmpData._float2[i];
		break;
	case kFloat4Type:
		for (int i = 0; i < 4; i++)
			data[i] = tmpData._float4[i];
		break;
	case kFloat16Type:
		for (int i = 0; i < 16; i++)
			data[i] = tmpData._float16[i];
		break;
	default:
		assert("gl_GetFloatv: unknown data type");
		break;
	}
}

void GLContext::gl_GetDoublev(TGLenum pname, TGLdouble *data) {
	eDataType dataType;
	union uglValue tmpData;
	
	gl_get_pname(pname, &tmpData, dataType);
	
	switch (dataType) {
	case kIntType:
		data[0] = tmpData._int;
		break;
	case kInt4Type:
		for (int i = 0; i < 4; i++)
			data[i] = tmpData._int4[i];
		break;
	case kUintType:
		data[0] = (TGLuint)tmpData._int;
		break;
	case kFloatType:
		data[0] = tmpData._float;
		break;
	case kFloat2Type:
		for (int i = 0; i < 2; i++)
			data[i] = tmpData._float2[i];
		break;
	case kFloat4Type:
		for (int i = 0; i < 4; i++)
			data[i] = tmpData._float4[i];
		break;
	case kFloat16Type:
		for (int i = 0; i < 16; i++)
			data[i] = tmpData._float16[i];
		break;
	default:
		assert("gl_GetDoublev: unknown data type");
		break;
	}
}

void GLContext::gl_GetBooleanv(TGLenum pname, TGLboolean *data) {
	eDataType dataType;
	union uglValue tmpData;

	gl_get_pname(pname, &tmpData, dataType);

	switch (dataType) {
	case kIntType:
		data[0] = VALUE_TO_BOOLEAN(tmpData._int);
		break;
	case kInt4Type:
		for (int i = 0; i < 4; i++)
			data[i] = VALUE_TO_BOOLEAN(tmpData._int4[i]);
		break;
	case kUintType:
		data[0] = VALUE_TO_BOOLEAN(tmpData._int);
		break;
	case kFloatType:
		data[0] = VALUE_TO_BOOLEAN(tmpData._float);
		break;
	case kFloat2Type:
		for (int i = 0; i < 2; i++)
			data[i] = VALUE_TO_BOOLEAN(tmpData._float2[i]);
		break;
	case kFloat4Type:
		for (int i = 0; i < 4; i++)
			data[i] = VALUE_TO_BOOLEAN(tmpData._float4[i]);
		break;
	case kFloat16Type:
		for (int i = 0; i < 16; i++)
			data[i] = VALUE_TO_BOOLEAN(tmpData._float16[i]);
		break;
	default:
		assert("gl_GetBooleanv: unknown data type");
		break;
	}
}

} // end of namespace TinyGL
