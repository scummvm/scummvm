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

// The following constants come from Mesa

#ifndef GRAPHICS_TGL_H
#define GRAPHICS_TGL_H

#define TGL_VERSION_1_1 1

enum {
	// Boolean values
	TGL_FALSE                       = 0,
	TGL_TRUE                        = 1,

	// Data types
	TGL_BYTE                        = 0x1400,
	TGL_UNSIGNED_BYTE               = 0x1401,
	TGL_SHORT                       = 0x1402,
	TGL_UNSIGNED_SHORT              = 0x1403,
	TGL_INT                         = 0x1404,
	TGL_UNSIGNED_INT                = 0x1405,
	TGL_FLOAT                       = 0x1406,
	TGL_DOUBLE                      = 0x140A,
	TGL_2_BYTES                     = 0x1407,
	TGL_3_BYTES                     = 0x1408,
	TGL_4_BYTES                     = 0x1409,

	// Primitives
	TGL_LINES                       = 0x0001,
	TGL_POINTS                      = 0x0000,
	TGL_LINE_STRIP                  = 0x0003,
	TGL_LINE_LOOP                   = 0x0002,
	TGL_TRIANGLES                   = 0x0004,
	TGL_TRIANGLE_STRIP              = 0x0005,
	TGL_TRIANGLE_FAN                = 0x0006,
	TGL_QUADS                       = 0x0007,
	TGL_QUAD_STRIP                  = 0x0008,
	TGL_POLYGON                     = 0x0009,
	TGL_EDGE_FLAG                   = 0x0B43,

	// Vertex Arrays
	TGL_VERTEX_ARRAY                = 0x8074,
	TGL_NORMAL_ARRAY                = 0x8075,
	TGL_COLOR_ARRAY                 = 0x8076,
	TGL_INDEX_ARRAY                 = 0x8077,
	TGL_TEXTURE_COORD_ARRAY         = 0x8078,
	TGL_EDGE_FLAG_ARRAY             = 0x8079,
	TGL_VERTEX_ARRAY_SIZE           = 0x807A,
	TGL_VERTEX_ARRAY_TYPE           = 0x807B,
	TGL_VERTEX_ARRAY_STRIDE         = 0x807C,
	TGL_VERTEX_ARRAY_COUNT          = 0x807D,
	TGL_NORMAL_ARRAY_TYPE           = 0x807E,
	TGL_NORMAL_ARRAY_STRIDE         = 0x807F,
	TGL_NORMAL_ARRAY_COUNT          = 0x8080,
	TGL_COLOR_ARRAY_SIZE            = 0x8081,
	TGL_COLOR_ARRAY_TYPE            = 0x8082,
	TGL_COLOR_ARRAY_STRIDE          = 0x8083,
	TGL_COLOR_ARRAY_COUNT           = 0x8084,
	TGL_INDEX_ARRAY_TYPE            = 0x8085,
	TGL_INDEX_ARRAY_STRIDE          = 0x8086,
	TGL_INDEX_ARRAY_COUNT           = 0x8087,
	TGL_TEXTURE_COORD_ARRAY_SIZE    = 0x8088,
	TGL_TEXTURE_COORD_ARRAY_TYPE    = 0x8089,
	TGL_TEXTURE_COORD_ARRAY_STRIDE  = 0x808A,
	TGL_TEXTURE_COORD_ARRAY_COUNT   = 0x808B,
	TGL_EDGE_FLAG_ARRAY_STRIDE      = 0x808C,
	TGL_EDGE_FLAG_ARRAY_COUNT       = 0x808D,
	TGL_VERTEX_ARRAY_POINTER        = 0x808E,
	TGL_NORMAL_ARRAY_POINTER        = 0x808F,
	TGL_COLOR_ARRAY_POINTER         = 0x8090,
	TGL_INDEX_ARRAY_POINTER         = 0x8091,
	TGL_TEXTURE_COORD_ARRAY_POINTER = 0x8092,
	TGL_EDGE_FLAG_ARRAY_POINTER     = 0x8093,
	TGL_V2F                         = 0x2A20,
	TGL_V3F                         = 0x2A21,
	TGL_C4UB_V2F                    = 0x2A22,
	TGL_C4UB_V3F                    = 0x2A23,
	TGL_C3F_V3F                     = 0x2A24,
	TGL_N3F_V3F                     = 0x2A25,
	TGL_C4F_N3F_V3F                 = 0x2A26,
	TGL_T2F_V3F                     = 0x2A27,
	TGL_T4F_V4F                     = 0x2A28,
	TGL_T2F_C4UB_V3F                = 0x2A29,
	TGL_T2F_C3F_V3F                 = 0x2A2A,
	TGL_T2F_N3F_V3F                 = 0x2A2B,
	TGL_T2F_C4F_N3F_V3F             = 0x2A2C,
	TGL_T4F_C4F_N3F_V4F             = 0x2A2D,

	// Matrix Mode
	TGL_MATRIX_MODE                 = 0x0BA0,
	TGL_MODELVIEW                   = 0x1700,
	TGL_PROJECTION                  = 0x1701,
	TGL_TEXTURE                     = 0x1702,

	// Points
	TGL_POINT_SMOOTH                = 0x0B10,
	TGL_POINT_SIZE                  = 0x0B11,
	TGL_POINT_SIZE_GRANULARITY      = 0x0B13,
	TGL_POINT_SIZE_RANGE            = 0x0B12,

	// Lines
	TGL_LINE_SMOOTH                 = 0x0B20,
	TGL_LINE_STIPPLE                = 0x0B24,
	TGL_LINE_STIPPLE_PATTERN        = 0x0B25,
	TGL_LINE_STIPPLE_REPEAT         = 0x0B26,
	TGL_LINE_WIDTH                  = 0x0B21,
	TGL_LINE_WIDTH_GRANULARITY      = 0x0B23,
	TGL_LINE_WIDTH_RANGE            = 0x0B22,

	// Polygons
	TGL_POINT                       = 0x1B00,
	TGL_LINE                        = 0x1B01,
	TGL_FILL                        = 0x1B02,
	TGL_CCW                         = 0x0901,
	TGL_CW                          = 0x0900,
	TGL_FRONT                       = 0x0404,
	TGL_BACK                        = 0x0405,
	TGL_CULL_FACE                   = 0x0B44,
	TGL_CULL_FACE_MODE              = 0x0B45,
	TGL_POLYGON_SMOOTH              = 0x0B41,
	TGL_POLYGON_STIPPLE             = 0x0B42,
	TGL_FRONT_FACE                  = 0x0B46,
	TGL_POLYGON_MODE                = 0x0B40,
	TGL_POLYGON_OFFSET_FACTOR       = 0x3038,
	TGL_POLYGON_OFFSET_UNITS        = 0x2A00,
	TGL_POLYGON_OFFSET_POINT        = 0x2A01,
	TGL_POLYGON_OFFSET_LINE         = 0x2A02,
	TGL_POLYGON_OFFSET_FILL         = 0x8037,
	TGL_SHADOW_MASK_MODE            = 0x0C40,
	TGL_SHADOW_MODE                 = 0x0C41,

	// Display Lists
	TGL_COMPILE                     = 0x1300,
	TGL_COMPILE_AND_EXECUTE         = 0x1301,
	TGL_LIST_BASE                   = 0x0B32,
	TGL_LIST_INDEX                  = 0x0B33,
	TGL_LIST_MODE                   = 0x0B30,

	// Depth buffer
	TGL_NEVER                       = 0x0200,
	TGL_LESS                        = 0x0201,
	TGL_GEQUAL                      = 0x0206,
	TGL_LEQUAL                      = 0x0203,
	TGL_GREATER                     = 0x0204,
	TGL_NOTEQUAL                    = 0x0205,
	TGL_EQUAL                       = 0x0202,
	TGL_ALWAYS                      = 0x0207,
	TGL_DEPTH_TEST                  = 0x0B71,
	TGL_DEPTH_BITS                  = 0x0D56,
	TGL_DEPTH_CLEAR_VALUE           = 0x0B73,
	TGL_DEPTH_FUNC                  = 0x0B74,
	TGL_DEPTH_RANGE                 = 0x0B70,
	TGL_DEPTH_WRITEMASK             = 0x0B72,
	TGL_DEPTH_COMPONENT             = 0x1902,

	// Lighting
	TGL_LIGHTING                    = 0x0B50,
	TGL_LIGHT0                      = 0x4000,
	TGL_LIGHT1                      = 0x4001,
	TGL_LIGHT2                      = 0x4002,
	TGL_LIGHT3                      = 0x4003,
	TGL_LIGHT4                      = 0x4004,
	TGL_LIGHT5                      = 0x4005,
	TGL_LIGHT6                      = 0x4006,
	TGL_LIGHT7                      = 0x4007,
	TGL_SPOT_EXPONENT               = 0x1205,
	TGL_SPOT_CUTOFF                 = 0x1206,
	TGL_CONSTANT_ATTENUATION        = 0x1207,
	TGL_LINEAR_ATTENUATION          = 0x1208,
	TGL_QUADRATIC_ATTENUATION       = 0x1209,
	TGL_AMBIENT                     = 0x1200,
	TGL_DIFFUSE                     = 0x1201,
	TGL_SPECULAR                    = 0x1202,
	TGL_SHININESS                   = 0x1601,
	TGL_EMISSION                    = 0x1600,
	TGL_POSITION                    = 0x1203,
	TGL_SPOT_DIRECTION              = 0x1204,
	TGL_AMBIENT_AND_DIFFUSE         = 0x1602,
	TGL_COLOR_INDEXES               = 0x1603,
	TGL_LIGHT_MODEL_TWO_SIDE        = 0x0B52,
	TGL_LIGHT_MODEL_LOCAL_VIEWER    = 0x0B51,
	TGL_LIGHT_MODEL_AMBIENT         = 0x0B53,
	TGL_FRONT_AND_BACK              = 0x0408,
	TGL_SHADE_MODEL                 = 0x0B54,
	TGL_FLAT                        = 0x1D00,
	TGL_SMOOTH                      = 0x1D01,
	TGL_COLOR_MATERIAL              = 0x0B57,
	TGL_COLOR_MATERIAL_FACE         = 0x0B55,
	TGL_COLOR_MATERIAL_PARAMETER    = 0x0B56,
	TGL_NORMALIZE                   = 0x0BA1,

	// User clipping planes
	TGL_CLIP_PLANE0                 = 0x3000,
	TGL_CLIP_PLANE1                 = 0x3001,
	TGL_CLIP_PLANE2                 = 0x3002,
	TGL_CLIP_PLANE3                 = 0x3003,
	TGL_CLIP_PLANE4                 = 0x3004,
	TGL_CLIP_PLANE5                 = 0x3005,

	// Accumulation buffer
	TGL_ACCUM_RED_BITS              = 0x0D58,
	TGL_ACCUM_GREEN_BITS            = 0x0D59,
	TGL_ACCUM_BLUE_BITS             = 0x0D5A,
	TGL_ACCUM_ALPHA_BITS            = 0x0D5B,
	TGL_ACCUM_CLEAR_VALUE           = 0x0B80,
	TGL_ACCUM                       = 0x0100,
	TGL_ADD                         = 0x0104,
	TGL_LOAD                        = 0x0101,
	TGL_MULT                        = 0x0103,
	TGL_RETURN                      = 0x0102,

	// Alpha testing
	TGL_ALPHA_TEST                  = 0x0BC0,
	TGL_ALPHA_TEST_REF              = 0x0BC2,
	TGL_ALPHA_TEST_FUNC             = 0x0BC1,

	// Blending
	TGL_BLEND                       = 0x0BE2,
	TGL_BLEND_SRC                   = 0x0BE1,
	TGL_BLEND_DST                   = 0x0BE0,
	TGL_ZERO                        = 0,
	TGL_ONE                         = 1,
	TGL_SRC_COLOR                   = 0x0300,
	TGL_ONE_MINUS_SRC_COLOR         = 0x0301,
	TGL_DST_COLOR                   = 0x0306,
	TGL_ONE_MINUS_DST_COLOR         = 0x0307,
	TGL_SRC_ALPHA                   = 0x0302,
	TGL_ONE_MINUS_SRC_ALPHA         = 0x0303,
	TGL_DST_ALPHA                   = 0x0304,
	TGL_ONE_MINUS_DST_ALPHA         = 0x0305,
	TGL_SRC_ALPHA_SATURATE          = 0x0308,
	TGL_CONSTANT_COLOR              = 0x8001,
	TGL_ONE_MINUS_CONSTANT_COLOR    = 0x8002,
	TGL_CONSTANT_ALPHA              = 0x8003,
	TGL_ONE_MINUS_CONSTANT_ALPHA    = 0x8004,

	// Render Mode
	TGL_FEEDBACK                    = 0x1C01,
	TGL_RENDER                      = 0x1C00,
	TGL_SELECT                      = 0x1C02,

	// Feedback
	TGL_2D                          = 0x0600,
	TGL_3D                          = 0x0601,
	TGL_3D_COLOR                    = 0x0602,
	TGL_3D_COLOR_TEXTURE            = 0x0603,
	TGL_4D_COLOR_TEXTURE            = 0x0604,
	TGL_POINT_TOKEN                 = 0x0701,
	TGL_LINE_TOKEN                  = 0x0702,
	TGL_LINE_RESET_TOKEN            = 0x0707,
	TGL_POLYGON_TOKEN               = 0x0703,
	TGL_BITMAP_TOKEN                = 0x0704,
	TGL_DRAW_PIXEL_TOKEN            = 0x0705,
	TGL_COPY_PIXEL_TOKEN            = 0x0706,
	TGL_PASS_THROUGH_TOKEN          = 0x0700,

	// Fog
	TGL_FOG                         = 0x0B60,
	TGL_FOG_MODE                    = 0x0B65,
	TGL_FOG_DENSITY                 = 0x0B62,
	TGL_FOG_COLOR                   = 0x0B66,
	TGL_FOG_INDEX                   = 0x0B61,
	TGL_FOG_START                   = 0x0B63,
	TGL_FOG_END                     = 0x0B64,
	TGL_LINEAR                      = 0x2601,
	TGL_EXP                         = 0x0800,
	TGL_EXP2                        = 0x0801,

	// Logic Ops
	TGL_LOGIC_OP                    = 0x0BF1,
	TGL_LOGIC_OP_MODE               = 0x0BF0,
	TGL_CLEAR                       = 0x1500,
	TGL_SET                         = 0x150F,
	TGL_COPY                        = 0x1503,
	TGL_COPY_INVERTED               = 0x150C,
	TGL_NOOP                        = 0x1505,
	TGL_INVERT                      = 0x150A,
	TGL_AND                         = 0x1501,
	TGL_NAND                        = 0x150E,
	TGL_OR                          = 0x1507,
	TGL_NOR                         = 0x1508,
	TGL_XOR                         = 0x1506,
	TGL_EQUIV                       = 0x1509,
	TGL_AND_REVERSE                 = 0x1502,
	TGL_AND_INVERTED                = 0x1504,
	TGL_OR_REVERSE                  = 0x150B,
	TGL_OR_INVERTED                 = 0x150D,

	// Stencil
	TGL_STENCIL_TEST                = 0x0B90,
	TGL_STENCIL_WRITEMASK           = 0x0B98,
	TGL_STENCIL_BITS                = 0x0D57,
	TGL_STENCIL_FUNC                = 0x0B92,
	TGL_STENCIL_VALUE_MASK          = 0x0B93,
	TGL_STENCIL_REF                 = 0x0B97,
	TGL_STENCIL_FAIL                = 0x0B94,
	TGL_STENCIL_PASS_DEPTH_PASS     = 0x0B96,
	TGL_STENCIL_PASS_DEPTH_FAIL     = 0x0B95,
	TGL_STENCIL_CLEAR_VALUE         = 0x0B91,
	TGL_STENCIL_INDEX               = 0x1901,
	TGL_KEEP                        = 0x1E00,
	TGL_REPLACE                     = 0x1E01,
	TGL_INCR                        = 0x1E02,
	TGL_DECR                        = 0x1E03,

	// Buffers, Pixel Drawing/Reading
	TGL_NONE                        = 0,
	TGL_LEFT                        = 0x0406,
	TGL_RIGHT                       = 0x0407,
	//TGL_FRONT                     = 0x0404,
	//TGL_BACK                      = 0x0405,
	//TGL_FRONT_AND_BACK            = 0x0408,
	TGL_FRONT_LEFT                  = 0x0400,
	TGL_FRONT_RIGHT                 = 0x0401,
	TGL_BACK_LEFT                   = 0x0402,
	TGL_BACK_RIGHT                  = 0x0403,
	TGL_AUX0                        = 0x0409,
	TGL_AUX1                        = 0x040A,
	TGL_AUX2                        = 0x040B,
	TGL_AUX3                        = 0x040C,
	TGL_COLOR_INDEX                 = 0x1900,
	TGL_RED                         = 0x1903,
	TGL_GREEN                       = 0x1904,
	TGL_BLUE                        = 0x1905,
	TGL_ALPHA                       = 0x1906,
	TGL_LUMINANCE                   = 0x1909,
	TGL_LUMINANCE_ALPHA             = 0x190A,
	TGL_ALPHA_BITS                  = 0x0D55,
	TGL_RED_BITS                    = 0x0D52,
	TGL_GREEN_BITS                  = 0x0D53,
	TGL_BLUE_BITS                   = 0x0D54,
	TGL_INDEX_BITS                  = 0x0D51,
	TGL_SUBPIXEL_BITS               = 0x0D50,
	TGL_AUX_BUFFERS                 = 0x0C00,
	TGL_READ_BUFFER                 = 0x0C02,
	TGL_DRAW_BUFFER                 = 0x0C01,
	TGL_DOUBLEBUFFER                = 0x0C32,
	TGL_STEREO                      = 0x0C33,
	TGL_BITMAP                      = 0x1A00,
	TGL_COLOR                       = 0x1800,
	TGL_DEPTH                       = 0x1801,
	TGL_STENCIL                     = 0x1802,
	TGL_DITHER                      = 0x0BD0,
	TGL_RGB                         = 0x1907,
	TGL_RGBA                        = 0x1908,

	// Implementation limits
	TGL_MAX_LIST_NESTING            = 0x0B31,
	TGL_MAX_ATTRIB_STACK_DEPTH      = 0x0D35,
	TGL_MAX_MODELVIEW_STACK_DEPTH   = 0x0D36,
	TGL_MAX_NAME_STACK_DEPTH        = 0x0D37,
	TGL_MAX_PROJECTION_STACK_DEPTH  = 0x0D38,
	TGL_MAX_TEXTURE_STACK_DEPTH     = 0x0D39,
	TGL_MAX_EVAL_ORDER              = 0x0D30,
	TGL_MAX_LIGHTS                  = 0x0D31,
	TGL_MAX_CLIP_PLANES             = 0x0D32,
	TGL_MAX_TEXTURE_SIZE            = 0x0D33,
	TGL_MAX_PIXEL_MAP_TABLE         = 0x0D34,
	TGL_MAX_VIEWPORT_DIMS           = 0x0D3A,
	TGL_MAX_CLIENT_ATTRIB_STACK_DEPTH = 0x0D3B,

	// Gets
	TGL_ATTRIB_STACK_DEPTH          = 0x0BB0,
	TGL_COLOR_CLEAR_VALUE           = 0x0C22,
	TGL_COLOR_WRITEMASK             = 0x0C23,
	TGL_CURRENT_INDEX               = 0x0B01,
	TGL_CURRENT_COLOR               = 0x0B00,
	TGL_CURRENT_NORMAL              = 0x0B02,
	TGL_CURRENT_RASTER_COLOR        = 0x0B04,
	TGL_CURRENT_RASTER_DISTANCE     = 0x0B09,
	TGL_CURRENT_RASTER_INDEX        = 0x0B05,
	TGL_CURRENT_RASTER_POSITION     = 0x0B07,
	TGL_CURRENT_RASTER_TEXTURE_COORDS = 0x0B06,
	TGL_CURRENT_RASTER_POSITION_VALID = 0x0B08,
	TGL_CURRENT_TEXTURE_COORDS      = 0x0B03,
	TGL_INDEX_CLEAR_VALUE           = 0x0C20,
	TGL_INDEX_MODE                  = 0x0C30,
	TGL_INDEX_WRITEMASK             = 0x0C21,
	TGL_MODELVIEW_MATRIX            = 0x0BA6,
	TGL_MODELVIEW_STACK_DEPTH       = 0x0BA3,
	TGL_NAME_STACK_DEPTH            = 0x0D70,
	TGL_PROJECTION_MATRIX           = 0x0BA7,
	TGL_PROJECTION_STACK_DEPTH      = 0x0BA4,
	TGL_RENDER_MODE                 = 0x0C40,
	TGL_RGBA_MODE                   = 0x0C31,
	TGL_TEXTURE_MATRIX              = 0x0BA8,
	TGL_TEXTURE_STACK_DEPTH         = 0x0BA5,
	TGL_VIEWPORT                    = 0x0BA2,

	// Evaluators
	TGL_AUTO_NORMAL                 = 0x0D80,
	TGL_MAP1_COLOR_4                = 0x0D90,
	TGL_MAP1_GRID_DOMAIN            = 0x0DD0,
	TGL_MAP1_GRID_SEGMENTS          = 0x0DD1,
	TGL_MAP1_INDEX                  = 0x0D91,
	TGL_MAP1_NORMAL                 = 0x0D92,
	TGL_MAP1_TEXTURE_COORD_1        = 0x0D93,
	TGL_MAP1_TEXTURE_COORD_2        = 0x0D94,
	TGL_MAP1_TEXTURE_COORD_3        = 0x0D95,
	TGL_MAP1_TEXTURE_COORD_4        = 0x0D96,
	TGL_MAP1_VERTEX_3               = 0x0D97,
	TGL_MAP1_VERTEX_4               = 0x0D98,
	TGL_MAP2_COLOR_4                = 0x0DB0,
	TGL_MAP2_GRID_DOMAIN            = 0x0DD2,
	TGL_MAP2_GRID_SEGMENTS          = 0x0DD3,
	TGL_MAP2_INDEX                  = 0x0DB1,
	TGL_MAP2_NORMAL                 = 0x0DB2,
	TGL_MAP2_TEXTURE_COORD_1        = 0x0DB3,
	TGL_MAP2_TEXTURE_COORD_2        = 0x0DB4,
	TGL_MAP2_TEXTURE_COORD_3        = 0x0DB5,
	TGL_MAP2_TEXTURE_COORD_4        = 0x0DB6,
	TGL_MAP2_VERTEX_3               = 0x0DB7,
	TGL_MAP2_VERTEX_4               = 0x0DB8,
	TGL_COEFF                       = 0x0A00,
	TGL_DOMAIN                      = 0x0A02,
	TGL_ORDER                       = 0x0A01,

	// Hints
	TGL_FOG_HINT                    = 0x0C54,
	TGL_LINE_SMOOTH_HINT            = 0x0C52,
	TGL_PERSPECTIVE_CORRECTION_HINT = 0x0C50,
	TGL_POINT_SMOOTH_HINT           = 0x0C51,
	TGL_POLYGON_SMOOTH_HINT         = 0x0C53,
	TGL_DONT_CARE                   = 0x1100,
	TGL_FASTEST                     = 0x1101,
	TGL_NICEST                      = 0x1102,

	// Scissor box
	TGL_SCISSOR_TEST                = 0x0C11,
	TGL_SCISSOR_BOX                 = 0x0C10,

	// Pixel Mode / Transfer
	TGL_MAP_COLOR                   = 0x0D10,
	TGL_MAP_STENCIL                 = 0x0D11,
	TGL_INDEX_SHIFT                 = 0x0D12,
	TGL_INDEX_OFFSET                = 0x0D13,
	TGL_RED_SCALE                   = 0x0D14,
	TGL_RED_BIAS                    = 0x0D15,
	TGL_GREEN_SCALE                 = 0x0D18,
	TGL_GREEN_BIAS                  = 0x0D19,
	TGL_BLUE_SCALE                  = 0x0D1A,
	TGL_BLUE_BIAS                   = 0x0D1B,
	TGL_ALPHA_SCALE                 = 0x0D1C,
	TGL_ALPHA_BIAS                  = 0x0D1D,
	TGL_DEPTH_SCALE                 = 0x0D1E,
	TGL_DEPTH_BIAS                  = 0x0D1F,
	TGL_PIXEL_MAP_S_TO_S_SIZE       = 0x0CB1,
	TGL_PIXEL_MAP_I_TO_I_SIZE       = 0x0CB0,
	TGL_PIXEL_MAP_I_TO_R_SIZE       = 0x0CB2,
	TGL_PIXEL_MAP_I_TO_G_SIZE       = 0x0CB3,
	TGL_PIXEL_MAP_I_TO_B_SIZE       = 0x0CB4,
	TGL_PIXEL_MAP_I_TO_A_SIZE       = 0x0CB5,
	TGL_PIXEL_MAP_R_TO_R_SIZE       = 0x0CB6,
	TGL_PIXEL_MAP_G_TO_G_SIZE       = 0x0CB7,
	TGL_PIXEL_MAP_B_TO_B_SIZE       = 0x0CB8,
	TGL_PIXEL_MAP_A_TO_A_SIZE       = 0x0CB9,
	TGL_PIXEL_MAP_S_TO_S            = 0x0C71,
	TGL_PIXEL_MAP_I_TO_I            = 0x0C70,
	TGL_PIXEL_MAP_I_TO_R            = 0x0C72,
	TGL_PIXEL_MAP_I_TO_G            = 0x0C73,
	TGL_PIXEL_MAP_I_TO_B            = 0x0C74,
	TGL_PIXEL_MAP_I_TO_A            = 0x0C75,
	TGL_PIXEL_MAP_R_TO_R            = 0x0C76,
	TGL_PIXEL_MAP_G_TO_G            = 0x0C77,
	TGL_PIXEL_MAP_B_TO_B            = 0x0C78,
	TGL_PIXEL_MAP_A_TO_A            = 0x0C79,
	TGL_PACK_ALIGNMENT              = 0x0D05,
	TGL_PACK_LSB_FIRST              = 0x0D01,
	TGL_PACK_ROW_LENGTH             = 0x0D02,
	TGL_PACK_SKIP_PIXELS            = 0x0D04,
	TGL_PACK_SKIP_ROWS              = 0x0D03,
	TGL_PACK_SWAP_BYTES             = 0x0D00,
	TGL_UNPACK_ALIGNMENT            = 0x0CF5,
	TGL_UNPACK_LSB_FIRST            = 0x0CF1,
	TGL_UNPACK_ROW_LENGTH           = 0x0CF2,
	TGL_UNPACK_SKIP_PIXELS          = 0x0CF4,
	TGL_UNPACK_SKIP_ROWS            = 0x0CF3,
	TGL_UNPACK_SWAP_BYTES           = 0x0CF0,
	TGL_ZOOM_X                      = 0x0D16,
	TGL_ZOOM_Y                      = 0x0D17,

	// Texture mapping
	TGL_TEXTURE_ENV                 = 0x2300,
	TGL_TEXTURE_ENV_MODE            = 0x2200,
	TGL_TEXTURE_1D                  = 0x0DE0,
	TGL_TEXTURE_2D                  = 0x0DE1,
	TGL_TEXTURE_WRAP_S              = 0x2802,
	TGL_TEXTURE_WRAP_T              = 0x2803,
	TGL_TEXTURE_MAG_FILTER          = 0x2800,
	TGL_TEXTURE_MIN_FILTER          = 0x2801,
	TGL_TEXTURE_ENV_COLOR           = 0x2201,
	TGL_TEXTURE_GEN_S               = 0x0C60,
	TGL_TEXTURE_GEN_T               = 0x0C61,
	TGL_TEXTURE_GEN_MODE            = 0x2500,
	TGL_TEXTURE_BORDER_COLOR        = 0x1004,
	TGL_TEXTURE_WIDTH               = 0x1000,
	TGL_TEXTURE_HEIGHT              = 0x1001,
	TGL_TEXTURE_BORDER              = 0x1005,
	TGL_TEXTURE_COMPONENTS          = 0x1003,
	TGL_NEAREST_MIPMAP_NEAREST      = 0x2700,
	TGL_NEAREST_MIPMAP_LINEAR       = 0x2702,
	TGL_LINEAR_MIPMAP_NEAREST       = 0x2701,
	TGL_LINEAR_MIPMAP_LINEAR        = 0x2703,
	TGL_OBJECT_LINEAR               = 0x2401,
	TGL_OBJECT_PLANE                = 0x2501,
	TGL_EYE_LINEAR                  = 0x2400,
	TGL_EYE_PLANE                   = 0x2502,
	TGL_SPHERE_MAP                  = 0x2402,
	TGL_DECAL                       = 0x2101,
	TGL_MODULATE                    = 0x2100,
	TGL_NEAREST                     = 0x2600,
	TGL_REPEAT                      = 0x2901,
	TGL_CLAMP                       = 0x2900,
	TGL_CLAMP_TO_EDGE		= 0x812F,
	TGL_MIRRORED_REPEAT		= 0x8370,
	TGL_S                           = 0x2000,
	TGL_T                           = 0x2001,
	TGL_R                           = 0x2002,
	TGL_Q                           = 0x2003,
	TGL_TEXTURE_GEN_R               = 0x0C62,
	TGL_TEXTURE_GEN_Q               = 0x0C63,

	TGL_PROXY_TEXTURE_1D            = 0x8063,
	TGL_PROXY_TEXTURE_2D            = 0x8064,
	TGL_TEXTURE_PRIORITY            = 0x8066,
	TGL_TEXTURE_RESIDENT            = 0x8067,
	TGL_TEXTURE_1D_BINDING          = 0x8068,
	TGL_TEXTURE_2D_BINDING          = 0x8069,

	// Internal texture formats
	TGL_ALPHA4                      = 0x803B,
	TGL_ALPHA8                      = 0x803C,
	TGL_ALPHA12                     = 0x803D,
	TGL_ALPHA16                     = 0x803E,
	TGL_LUMINANCE4                  = 0x803F,
	TGL_LUMINANCE8                  = 0x8040,
	TGL_LUMINANCE12                 = 0x8041,
	TGL_LUMINANCE16                 = 0x8042,
	TGL_LUMINANCE4_ALPHA4           = 0x8043,
	TGL_LUMINANCE6_ALPHA2           = 0x8044,
	TGL_LUMINANCE8_ALPHA8           = 0x8045,
	TGL_LUMINANCE12_ALPHA4          = 0x8046,
	TGL_LUMINANCE12_ALPHA12         = 0x8047,
	TGL_LUMINANCE16_ALPHA16         = 0x8048,
	TGL_INTENSITY                   = 0x8049,
	TGL_INTENSITY4                  = 0x804A,
	TGL_INTENSITY8                  = 0x804B,
	TGL_INTENSITY12                 = 0x804C,
	TGL_INTENSITY16                 = 0x804D,
	TGL_R3_G3_B2                    = 0x2A10,
	TGL_RGB4                        = 0x804F,
	TGL_RGB5                        = 0x8050,
	TGL_RGB8                        = 0x8051,
	TGL_RGB10                       = 0x8052,
	TGL_RGB12                       = 0x8053,
	TGL_RGB16                       = 0x8054,
	TGL_RGBA2                       = 0x8055,
	TGL_RGBA4                       = 0x8056,
	TGL_RGB5_A1                     = 0x8057,
	TGL_RGBA8                       = 0x8058,
	TGL_RGB10_A2                    = 0x8059,
	TGL_RGBA12                      = 0x805A,
	TGL_RGBA16                      = 0x805B,
	TGL_UNSIGNED_SHORT_5_6_5        = 0x8363,
	TGL_UNSIGNED_SHORT_5_6_5_REV    = 0x8364,
	TGL_UNSIGNED_INT_8_8_8_8        = 0x8035,
	TGL_UNSIGNED_INT_8_8_8_8_REV    = 0x8367,
	TGL_UNSIGNED_SHORT_5_5_5_1      = 0x8034,
	TGL_UNSIGNED_SHORT_1_5_5_5_REV  = 0x8366,

	// Utility
	TGL_VENDOR                      = 0x1F00,
	TGL_RENDERER                    = 0x1F01,
	TGL_VERSION                     = 0x1F02,
	TGL_EXTENSIONS                  = 0x1F03,

	// Errors
	TGL_INVALID_VALUE               = 0x0501,
	TGL_INVALID_ENUM                = 0x0500,
	TGL_INVALID_OPERATION           = 0x0502,
	TGL_STACK_OVERFLOW              = 0x0503,
	TGL_STACK_UNDERFLOW             = 0x0504,
	TGL_OUT_OF_MEMORY               = 0x0505,

	// 1.0 Extensions

	// TGL_EXT_blend_minmax and TGL_EXT_blend_color
	TGL_CONSTANT_COLOR_EXT          = 0x8001,
	TGL_ONE_MINUS_CONSTANT_COLOR_EXT = 0x8002,
	TGL_CONSTANT_ALPHA_EXT          = 0x8003,
	TGL_ONE_MINUS_CONSTANT_ALPHA_EXT = 0x8004,
	TGL_BLEND_EQUATION_EXT          = 0x8009,
	TGL_MIN_EXT                     = 0x8007,
	TGL_MAX_EXT                     = 0x8008,
	TGL_FUNC_ADD_EXT                = 0x8006,
	TGL_FUNC_SUBTRACT_EXT           = 0x800A,
	TGL_FUNC_REVERSE_SUBTRACT_EXT   = 0x800B,
	TGL_BLEND_COLOR_EXT             = 0x8005,

	// TGL_EXT_polygon_offset
	TGL_POLYGON_OFFSET_EXT           = 0x8037,
	TGL_POLYGON_OFFSET_FACTOR_EXT    = 0x8038,
	TGL_POLYGON_OFFSET_BIAS_EXT      = 0x8039,

	// TGL_EXT_vertex_array
	TGL_VERTEX_ARRAY_EXT            = 0x8074,
	TGL_NORMAL_ARRAY_EXT            = 0x8075,
	TGL_COLOR_ARRAY_EXT             = 0x8076,
	TGL_INDEX_ARRAY_EXT             = 0x8077,
	TGL_TEXTURE_COORD_ARRAY_EXT     = 0x8078,
	TGL_EDGE_FLAG_ARRAY_EXT         = 0x8079,
	TGL_VERTEX_ARRAY_SIZE_EXT       = 0x807A,
	TGL_VERTEX_ARRAY_TYPE_EXT       = 0x807B,
	TGL_VERTEX_ARRAY_STRIDE_EXT     = 0x807C,
	TGL_VERTEX_ARRAY_COUNT_EXT      = 0x807D,
	TGL_NORMAL_ARRAY_TYPE_EXT       = 0x807E,
	TGL_NORMAL_ARRAY_STRIDE_EXT     = 0x807F,
	TGL_NORMAL_ARRAY_COUNT_EXT      = 0x8080,
	TGL_COLOR_ARRAY_SIZE_EXT        = 0x8081,
	TGL_COLOR_ARRAY_TYPE_EXT        = 0x8082,
	TGL_COLOR_ARRAY_STRIDE_EXT      = 0x8083,
	TGL_COLOR_ARRAY_COUNT_EXT       = 0x8084,
	TGL_INDEX_ARRAY_TYPE_EXT        = 0x8085,
	TGL_INDEX_ARRAY_STRIDE_EXT      = 0x8086,
	TGL_INDEX_ARRAY_COUNT_EXT       = 0x8087,
	TGL_TEXTURE_COORD_ARRAY_SIZE_EXT = 0x8088,
	TGL_TEXTURE_COORD_ARRAY_TYPE_EXT = 0x8089,
	TGL_TEXTURE_COORD_ARRAY_STRIDE_EXT = 0x808A,
	TGL_TEXTURE_COORD_ARRAY_COUNT_EXT = 0x808B,
	TGL_EDGE_FLAG_ARRAY_STRIDE_EXT  = 0x808C,
	TGL_EDGE_FLAG_ARRAY_COUNT_EXT   = 0x808D,
	TGL_VERTEX_ARRAY_POINTER_EXT    = 0x808E,
	TGL_NORMAL_ARRAY_POINTER_EXT    = 0x808F,
	TGL_COLOR_ARRAY_POINTER_EXT     = 0x8090,
	TGL_INDEX_ARRAY_POINTER_EXT     = 0x8091,
	TGL_TEXTURE_COORD_ARRAY_POINTER_EXT = 0x8092,
	TGL_EDGE_FLAG_ARRAY_POINTER_EXT = 0x8093,

	// Color-types from 1.2, from SDL_opengl.h
	TGL_BGR                         = 0x80E0,
	TGL_BGRA                        = 0x80E1
};

enum {
	TGL_CURRENT_BIT         = 0x00000001,
	TGL_POINT_BIT           = 0x00000002,
	TGL_LINE_BIT            = 0x00000004,
	TGL_POLYGON_BIT         = 0x00000008,
	TGL_POLYGON_STIPPLE_BIT = 0x00000010,
	TGL_PIXEL_MODE_BIT      = 0x00000020,
	TGL_LIGHTING_BIT        = 0x00000040,
	TGL_FOG_BIT             = 0x00000080,
	TGL_DEPTH_BUFFER_BIT    = 0x00000100,
	TGL_ACCUM_BUFFER_BIT    = 0x00000200,
	TGL_STENCIL_BUFFER_BIT  = 0x00000400,
	TGL_VIEWPORT_BIT        = 0x00000800,
	TGL_TRANSFORM_BIT       = 0x00001000,
	TGL_ENABLE_BIT          = 0x00002000,
	TGL_COLOR_BUFFER_BIT    = 0x00004000,
	TGL_HINT_BIT            = 0x00008000,
	TGL_EVAL_BIT            = 0x00010000,
	TGL_LIST_BIT            = 0x00020000,
	TGL_TEXTURE_BIT         = 0x00040000,
	TGL_SCISSOR_BIT         = 0x00080000,
	TGL_ALL_ATTRIB_BITS     = 0x000fffff
};

// some types

typedef int             TGLenum;
typedef void            TGLvoid;
typedef unsigned char   TGLboolean;
typedef signed char     TGLbyte;    // 1-byte signed
typedef short           TGLshort;   // 2-byte signed
typedef int             TGLint;     // 4-byte signed
typedef unsigned char   TGLubyte;   // 1-byte unsigned
typedef unsigned short  TGLushort;  // 2-byte unsigned
typedef unsigned int    TGLuint;    // 4-byte unsigned
typedef float           TGLfloat;   // single precision float
typedef double          TGLdouble;  // double precision float
typedef int             TGLsizei;

// functions

void tglEnable(int code);
void tglDisable(int code);

void tglShadeModel(int mode);
void tglCullFace(int mode);
void tglPolygonMode(int face, int mode);

void tglBegin(int type);
void tglEnd();

#define PROTO_GL1(name)				\
void tgl ## name ## 1f(float);		\
void tgl ## name ## 1d(double);		\
void tgl ## name ## 1fv(const float *);	\
void tgl ## name ## 1dv(const double *);

#define PROTO_GL2(name)					\
void tgl ## name ## 2f(float, float);	\
void tgl ## name ## 2d(double, double);	\
void tgl ## name ## 2fv(const float *);		\
void tgl ## name ## 2dv(const double *);

#define PROTO_GL3(name)							\
void tgl ## name ## 3f(float, float, float);	\
void tgl ## name ## 3d(double, double, double);	\
void tgl ## name ## 3fv(const float *);				\
void tgl ## name ## 3dv(const double *);

#define PROTO_GL4(name)									\
void tgl ## name ## 4f(float, float, float, float);		\
void tgl ## name ## 4d(double, double, double, double);	\
void tgl ## name ## 4fv(const float *);						\
void tgl ## name ## 4dv(const double *);

PROTO_GL2(Vertex)
PROTO_GL3(Vertex)
PROTO_GL4(Vertex)

PROTO_GL3(Color)
PROTO_GL4(Color)
void tglColor3ub(unsigned char r, unsigned char g, unsigned char b);
void tglColor4ub(unsigned char r, unsigned char g, unsigned char b,
				 unsigned char a);

PROTO_GL3(Normal)

PROTO_GL1(TexCoord)
PROTO_GL2(TexCoord)
PROTO_GL3(TexCoord)
PROTO_GL4(TexCoord)

void tglEdgeFlag(int flag);

// matrix
void tglMatrixMode(int mode);
void tglLoadMatrixf(const float *m);
void tglLoadIdentity();
void tglMultMatrixf(const float *m);
void tglPushMatrix();
void tglPopMatrix();
void tglRotatef(float angle, float x, float y, float z);
void tglTranslatef(float x, float y, float z);
void tglScalef(float x, float y, float z);

void tglViewport(int x, int y, int width, int height);
void tglFrustum(double left, double right, double bottom, double top,
				double nearv, double farv);
void tglOrtho(double left, double right, double bottom, double top, double zNear, double zFar);

// lists
unsigned int tglGenLists(int range);
int tglIsList(unsigned int list);
void tglNewList(unsigned int list, int mode);
void tglEndList();
void tglCallList(unsigned int list);

// clear
void tglClear(int mask);
void tglClearColor(float r, float g, float b, float a);
void tglClearDepth(double depth);

// selection
int tglRenderMode(int mode);
void tglSelectBuffer(int size, unsigned int *buf);

void tglInitNames();
void tglPushName(unsigned int name);
void tglPopName();
void tglLoadName(unsigned int name);

// textures
void tglGenTextures(int n, unsigned int *textures);
void tglDeleteTextures(int n, const unsigned int *textures);
void tglBindTexture(int target, int texture);
void tglTexImage2D(int target, int level, int components,
				   int width, int height, int border,
				   int format, int type, void *pixels);
void tglTexEnvi(int target, int pname, int param);
void tglTexParameteri(int target, int pname, int param);
void tglPixelStorei(int pname, int param);

// lighting

void tglMaterialfv(int mode, int type, const float *v);
void tglMaterialf(int mode, int type, float v);
void tglColorMaterial(int mode, int type);

void tglLightfv(int light, int type, const float *v);
void tglLightf(int light, int type, const float v);
void tglLightModeli(int pname, int param);
void tglLightModelfv(int pname, const float *param);

// misc

void tglFlush();
void tglHint(int target, int mode);
void tglGetIntegerv(int pname, int *params);
void tglGetFloatv(int pname, float *v);
void tglFrontFace(int mode);
void tglColorMask(TGLboolean r, TGLboolean g, TGLboolean b, TGLboolean a);
void tglDepthMask(int enableWrite);
void tglBlendFunc(TGLenum sfactor, TGLenum dfactor);
void tglAlphaFunc(TGLenum func, float ref);
void tglDepthFunc(TGLenum func);

void tglSetShadowMaskBuf(unsigned char *buf);
void tglSetShadowColor(unsigned char r, unsigned char g, unsigned char b);

// opengl 1.2 arrays
void tglEnableClientState(TGLenum array);
void tglDisableClientState(TGLenum array);
void tglArrayElement(TGLint i);
void tglDrawArrays(TGLenum mode, TGLint first, TGLsizei count);
void tglVertexPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer);
void tglColorPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer);
void tglNormalPointer(TGLenum type, TGLsizei stride, const TGLvoid *pointer);
void tglTexCoordPointer(TGLint size, TGLenum type, TGLsizei stride, const TGLvoid *pointer);

// opengl 1.2 polygon offset
void tglPolygonOffset(TGLfloat factor, TGLfloat units);

void tglEnableDirtyRects(bool enable);

void tglDebug(int mode);

namespace TinyGL {

void tglPresentBuffer();

} // end of namespace TinyGL

#endif
