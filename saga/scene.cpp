/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Scene management module

#include "reinherit.h"

#include "yslib.h"

#include "game_mod.h"
#include "animation_mod.h"
#include "console_mod.h"
#include "cvar_mod.h"
#include "events_mod.h"
#include "actionmap_mod.h"
#include "gfx_mod.h"
#include "image_mod.h"
#include "isomap_mod.h"
#include "script_mod.h"
#include "objectmap_mod.h"
#include "palanim_mod.h"
#include "render_mod.h"
#include "rscfile_mod.h"
#include "text_mod.h"

#include "scene_mod.h"
#include "scene.h"

namespace Saga {

static R_SCENE_MODULE SceneModule;

int SCENE_Register() {
	CVAR_Register_I(&SceneModule.scene_number, "scene", NULL, R_CVAR_READONLY, 0, 0);
	CVAR_RegisterFunc(CF_scenechange, "scene_change", "<Scene number>", R_CVAR_NONE, 1, 1);
	CVAR_RegisterFunc(CF_sceneinfo, "scene_info", NULL, R_CVAR_NONE, 0, 0);

	return R_SUCCESS;
}

int SCENE_Init() {
	R_GAME_SCENEDESC gs_desc;
	byte *scene_lut_p;
	size_t scene_lut_len;
	const byte *read_p;
	int result;
	int i;

	// Load game-specific scene data
	GAME_GetSceneInfo(&gs_desc);

	// Load scene module resource context
	result = GAME_GetFileContext(&SceneModule.scene_ctxt, R_GAME_RESOURCEFILE, 0);
	if (result != R_SUCCESS) {
		R_printf(R_STDERR, "Couldn't load scene resource context.\n");
		return R_FAILURE;
	}

	// Initialize scene queue
	SceneModule.scene_queue = ys_dll_create();
	if (SceneModule.scene_queue == NULL) {
		return R_FAILURE;
	}

	// Load scene lookup table
	R_printf(R_STDOUT, "SCENE_Init(): Loading scene LUT from resource %u.\n", gs_desc.scene_lut_rn);
	result = RSC_LoadResource(SceneModule.scene_ctxt, gs_desc.scene_lut_rn, &scene_lut_p, &scene_lut_len);
	if (result != R_SUCCESS) {
		R_printf(R_STDERR, "Error: couldn't load scene LUT.\n");
		return R_FAILURE;
	}

	SceneModule.scene_count = scene_lut_len / 2;
	SceneModule.scene_max = SceneModule.scene_count - 1;
	SceneModule.scene_lut = (int *)malloc(SceneModule.scene_max * sizeof *SceneModule.scene_lut);
	if (SceneModule.scene_lut == NULL) {
		R_printf(R_STDERR, "SCENE_Init(): Memory allocation failed.\n");
		return R_MEM;
	}

	read_p = scene_lut_p;

	for (i = 0; i < SceneModule.scene_max; i++) {
		SceneModule.scene_lut[i] = ys_read_u16_le(read_p, &read_p);
	}

	free(scene_lut_p);

	if (gs_desc.first_scene != 0) {
		SceneModule.first_scene = gs_desc.first_scene;
	}

	R_printf(R_STDOUT,
	    "SCENE_Init(): First scene set to %d.\n", SceneModule.first_scene);

	R_printf(R_STDOUT, "SCENE_Init(): LUT has %d entries.\n", SceneModule.scene_max);

	// Create scene module text list
	SceneModule.text_list = TEXT_CreateList();

	if (SceneModule.text_list == NULL) {
		R_printf(R_STDERR, "Error: Couldn't create scene text list.\n");
		return R_FAILURE;
	}

	SceneModule.init = 1;

	return R_SUCCESS;
}

int SCENE_Shutdown() {
	if (SceneModule.init) {
		SCENE_End();
		free(SceneModule.scene_lut);
	}

	return R_SUCCESS;
}

int SCENE_Queue(R_SCENE_QUEUE *scene_queue) {
	assert(SceneModule.init);
	assert(scene_queue != NULL);

	ys_dll_add_tail(SceneModule.scene_queue, scene_queue, sizeof *scene_queue);

	return R_SUCCESS;
}

int SCENE_ClearQueue() {
	assert(SceneModule.init);

	ys_dll_delete_all(SceneModule.scene_queue);

	return R_SUCCESS;
}

int SCENE_Start() {
	YS_DL_NODE *node;
	R_SCENE_QUEUE *scene_qdat;

	assert(SceneModule.init);

	if (SceneModule.scene_loaded) {
		R_printf(R_STDERR, "Error: Can't start game...scene already loaded!\n");
		return R_FAILURE;
	}

	if (SceneModule.in_game) {
		R_printf(R_STDERR, "Error: Can't start game...game already started!\n");
		return R_FAILURE;
	}

	switch (GAME_GetGameType()) {
	case R_GAMETYPE_ITE:
		ITE_StartProc();
		break;
	case R_GAMETYPE_IHNM:
		IHNM_StartProc();
		break;
	default:
		R_printf(R_STDERR, "Error: Can't start game... gametype not supported.\n");
		break;
	}

	// Load the head node in scene queue
	node = ys_dll_head(SceneModule.scene_queue);
	if (node == NULL) {
		return R_SUCCESS;
	}

	scene_qdat = (R_SCENE_QUEUE *)ys_dll_get_data(node);
	assert(scene_qdat != NULL);

	SCENE_Load(scene_qdat->scene_n, scene_qdat->load_flag, scene_qdat->scene_proc, scene_qdat->scene_desc);

	return R_SUCCESS;
}

int SCENE_Next() {
	YS_DL_NODE *node;
	R_SCENE_QUEUE *scene_qdat;

	assert(SceneModule.init);

	if (!SceneModule.scene_loaded) {
		R_printf(R_STDERR, "Error: Can't advance scene...no scene loaded!\n");
		return R_FAILURE;
	}

	if (SceneModule.in_game) {
		R_printf(R_STDERR, "Error: Can't advance scene...game already started!\n");
		return R_FAILURE;
	}

	SCENE_End();

	// Delete the current head node in scene queue
	node = ys_dll_head(SceneModule.scene_queue);
	if (node == NULL) {
		return R_SUCCESS;
	}

	ys_dll_delete(node);

	// Load the head node in scene queue
	node = ys_dll_head(SceneModule.scene_queue);
	if (node == NULL) {
		return R_SUCCESS;
	}

	scene_qdat = (R_SCENE_QUEUE *)ys_dll_get_data(node);
	assert(scene_qdat != NULL);

	SCENE_Load(scene_qdat->scene_n, scene_qdat->load_flag, scene_qdat->scene_proc, scene_qdat->scene_desc);

	return R_SUCCESS;
}

int SCENE_Skip() {
	YS_DL_NODE *node;
	YS_DL_NODE *prev_node;
	YS_DL_NODE *skip_node = NULL;

	R_SCENE_QUEUE *scene_qdat = NULL;
	R_SCENE_QUEUE *skip_qdat = NULL;

	assert(SceneModule.init);

	if (!SceneModule.scene_loaded) {
		R_printf(R_STDERR, "Error: Can't skip scene...no scene loaded.\n");
		return R_FAILURE;
	}

	if (SceneModule.in_game) {
		R_printf(R_STDERR, "Error: Can't skip scene...game already started.\n");
		return R_FAILURE;
	}

	// Walk down scene queue and try to find a skip target
	node = ys_dll_head(SceneModule.scene_queue);
	if (node == NULL) {
		R_printf(R_STDERR, "Error: Can't skip scene...no scenes in queue.\n");
		return R_FAILURE;
	}

	for (node = ys_dll_next(node); node != NULL; node = ys_dll_next(node)) {
		scene_qdat = (R_SCENE_QUEUE *)ys_dll_get_data(node);
		assert(scene_qdat != NULL);

		if (scene_qdat->scene_skiptarget) {
			skip_node = node;
			skip_qdat = scene_qdat;
			break;
		}
	}

	// If skip target found, remove preceding scenes and load
	if (skip_node != NULL) {
		for (node = ys_dll_prev(skip_node); node != NULL; node = prev_node) {
			prev_node = ys_dll_prev(node);
			ys_dll_delete(node);
		}
		SCENE_End();
		SCENE_Load(skip_qdat->scene_n, skip_qdat->load_flag, skip_qdat->scene_proc, skip_qdat->scene_desc);
	}
	// Search for a scene to skip to

	return R_SUCCESS;
}

int SCENE_Change(int scene_num) {
	assert(SceneModule.init);

	if (!SceneModule.scene_loaded) {
		R_printf(R_STDERR, "Error: Can't change scene. No scene currently loaded. Game in invalid state.\n");
		return R_FAILURE;
	}

	if ((scene_num < 0) || (scene_num > SceneModule.scene_max)) {
		R_printf(R_STDERR, "Error: Can't change scene. Invalid scene number.\n");
		return R_FAILURE;
	}

	if (SceneModule.scene_lut[scene_num] == 0) {
		R_printf(R_STDERR, "Error: Can't change scene; invalid scene descriptor resource number (0)\n");
		return R_FAILURE;
	}

	SCENE_End();
	SCENE_Load(scene_num, BY_SCENE, DefaultSceneProc, NULL);

	return R_SUCCESS;
}

int SCENE_GetMode() {
	assert(SceneModule.init);

	return SceneModule.scene_mode;
}

int SCENE_GetZInfo(SCENE_ZINFO *zinfo) {
	assert(SceneModule.init);

	zinfo->begin_slope = SceneModule.desc.begin_slope;
	zinfo->end_slope = SceneModule.desc.end_slope;

	return R_SUCCESS;
}

int SCENE_GetBGInfo(SCENE_BGINFO *bginfo) {
	R_GAME_DISPLAYINFO di;
	int x, y;

	assert(SceneModule.init);

	bginfo->bg_buf = SceneModule.bg.buf;
	bginfo->bg_buflen = SceneModule.bg.buf_len;
	bginfo->bg_w = SceneModule.bg.w;
	bginfo->bg_h = SceneModule.bg.h;
	bginfo->bg_p = SceneModule.bg.p;

	GAME_GetDisplayInfo(&di);
	x = 0;
	y = 0;

	if (SceneModule.bg.w < di.logical_w) {
		x = (di.logical_w - SceneModule.bg.w) / 2;
	}

	if (SceneModule.bg.h < di.scene_h) {
		y = (di.scene_h - SceneModule.bg.h) / 2;
	}

	bginfo->bg_x = x;
	bginfo->bg_y = y;

	return R_SUCCESS;
}

int SCENE_GetBGPal(PALENTRY **pal) {
	assert(SceneModule.init);
	*pal = SceneModule.bg.pal;

	return R_SUCCESS;
}

int SCENE_GetBGMaskInfo(int *w, int *h, byte **buf, size_t *buf_len) {
	assert(SceneModule.init);

	if (!SceneModule.bg_mask.loaded) {
		return R_FAILURE;
	}

	*w = SceneModule.bg_mask.w;
	*h = SceneModule.bg_mask.h;
	*buf = SceneModule.bg_mask.buf;
	*buf_len = SceneModule.bg_mask.buf_len;

	return R_SUCCESS;
}

int SCENE_IsBGMaskPresent() {
	assert(SceneModule.init);

	return SceneModule.bg_mask.loaded;
}

int SCENE_GetInfo(R_SCENE_INFO *si) {
	assert(SceneModule.init);
	assert(si != NULL);

	si->text_list = SceneModule.text_list;

	return R_SUCCESS;
}

int SCENE_Load(int scene_num, int load_flag, R_SCENE_PROC scene_proc, R_SCENE_DESC *scene_desc_param) {
	R_SCENE_INFO scene_info;
	uint32 res_number = 0;
	int result;
	int i;

	assert(SceneModule.init);

	if (SceneModule.scene_loaded == 1) {
		R_printf(R_STDERR, "Error, a scene is already loaded.\n");
		return R_FAILURE;
	}

	SceneModule.anim_list = ys_dll_create();
	SceneModule.scene_mode = 0;
	SceneModule.load_desc = 1;

	switch (load_flag) {
	case BY_RESOURCE:
		res_number = scene_num;
		break;
	case BY_SCENE:
		assert((scene_num > 0) && (scene_num < SceneModule.scene_max));
		res_number = SceneModule.scene_lut[scene_num];
		SceneModule.scene_number = scene_num;
		break;
	case BY_DESC:
		assert(scene_desc_param != NULL);
		assert(scene_desc_param->res_list != NULL);
		SceneModule.load_desc = 0;
		SceneModule.desc = *scene_desc_param;
		SceneModule.reslist = scene_desc_param->res_list;
		SceneModule.reslist_entries = scene_desc_param->res_list_ct;
		break;
	default:
		R_printf(R_STDERR, "Error: Invalid scene load flag.\n");
		return R_FAILURE;
		break;
	}

	// Load scene descriptor and resource list resources
	if (SceneModule.load_desc) {

		SceneModule.scene_rn = res_number;
		assert(SceneModule.scene_rn != 0);
		R_printf(R_STDOUT, "Loading scene resource %u:\n", res_number);

		if (LoadSceneDescriptor(res_number) != R_SUCCESS) {
			R_printf(R_STDERR, "Error reading scene descriptor.\n");
			return R_FAILURE;
		}

		if (LoadSceneResourceList(SceneModule.desc.res_list_rn) != R_SUCCESS) {
			R_printf(R_STDERR, "Error reading scene resource list.\n");
			return R_FAILURE;
		}
	} else {
		R_printf(R_STDOUT, "Loading memory scene resource.\n");
	}

	// Load resources from scene resource list
	for (i = 0; i < SceneModule.reslist_entries; i++) {
		result = RSC_LoadResource(SceneModule.scene_ctxt, SceneModule.reslist[i].res_number,
								&SceneModule.reslist[i].res_data, &SceneModule.reslist[i].res_data_len);
		if (result != R_SUCCESS) {
			R_printf(R_STDERR, "Error: Allocation failure loading scene resource list.\n");
			return R_FAILURE;
		}
	}

	// Process resources from scene resource list
	if (ProcessSceneResources() != R_SUCCESS) {
		R_printf(R_STDERR, "Error loading scene resources.\n");
		return R_FAILURE;
	}

	// Load scene script data
	if (SceneModule.desc.script_num > 0) {
		if (SCRIPT_Load(SceneModule.desc.script_num) != R_SUCCESS) {
			R_printf(R_STDERR, "Error loading scene script.\n");
			return R_FAILURE;
		}
	}

	SceneModule.scene_loaded = 1;

	if (scene_proc == NULL) {
		SceneModule.scene_proc = DefaultSceneProc;
	} else {
		SceneModule.scene_proc = scene_proc;
	}

	SCENE_GetInfo(&scene_info);

	SceneModule.scene_proc(SCENE_BEGIN, &scene_info);

	return R_SUCCESS;
}

int LoadSceneDescriptor(uint32 res_number) {
	byte *scene_desc_data;
	size_t scene_desc_len;
	const byte *read_p;
	int result;

	result = RSC_LoadResource(SceneModule.scene_ctxt, res_number, &scene_desc_data, &scene_desc_len);
	if (result != R_SUCCESS) {
		R_printf(R_STDERR, "Error: couldn't load scene descriptor.\n");
		return R_FAILURE;
	}

	if (scene_desc_len != SAGA_SCENE_DESC_LEN) {
		R_printf(R_STDERR, "Error: scene descriptor length invalid.\n");
		return R_FAILURE;
	}

	read_p = scene_desc_data;

	SceneModule.desc.unknown0 = ys_read_u16_le(read_p, &read_p);
	SceneModule.desc.res_list_rn = ys_read_u16_le(read_p, &read_p);
	SceneModule.desc.end_slope = ys_read_u16_le(read_p, &read_p);
	SceneModule.desc.begin_slope = ys_read_u16_le(read_p, &read_p);
	SceneModule.desc.script_num = ys_read_u16_le(read_p, &read_p);
	SceneModule.desc.scene_scriptnum = ys_read_u16_le(read_p, &read_p);
	SceneModule.desc.start_scriptnum = ys_read_u16_le(read_p, &read_p);
	SceneModule.desc.music_rn = ys_read_s16_le(read_p, &read_p);

	RSC_FreeResource(scene_desc_data);

	return R_SUCCESS;
}

int LoadSceneResourceList(uint32 reslist_rn) {
	byte *resource_list;
	size_t resource_list_len;
	const byte *read_p;
	int result;
	int i;

	// Load the scene resource table
	result = RSC_LoadResource(SceneModule.scene_ctxt, reslist_rn, &resource_list, &resource_list_len);
	if (result != R_SUCCESS) {
		R_printf(R_STDERR, "Error: couldn't load scene resource list.\n");
		return R_FAILURE;
	}

	read_p = resource_list;

	// Allocate memory for scene resource list 
	SceneModule.reslist_entries = resource_list_len / SAGA_RESLIST_ENTRY_LEN;
	R_printf(R_STDOUT, "Scene resource list contains %d entries.\n", SceneModule.reslist_entries);
	SceneModule.reslist = (R_SCENE_RESLIST *)calloc(SceneModule.reslist_entries, sizeof *SceneModule.reslist);

	if (SceneModule.reslist == NULL) {
		R_printf(R_STDERR, "Error: Memory allocation failed.\n");
		return R_MEM;
	}

	// Load scene resource list from raw scene 
	// resource table
	R_printf(R_STDOUT, "Loading scene resource list...\n");

	for (i = 0; i < SceneModule.reslist_entries; i++) {
		SceneModule.reslist[i].res_number = ys_read_u16_le(read_p, &read_p);
		SceneModule.reslist[i].res_type = ys_read_u16_le(read_p, &read_p);
	}

	RSC_FreeResource(resource_list);

	return R_SUCCESS;
}

int ProcessSceneResources() {
	const byte *res_data;
	size_t res_data_len;
	const byte *pal_p;
	int i;

	// Process the scene resource list
	for (i = 0; i < SceneModule.reslist_entries; i++) {
		res_data = SceneModule.reslist[i].res_data;
		res_data_len = SceneModule.reslist[i].res_data_len;
		switch (SceneModule.reslist[i].res_type) {
		case SAGA_BG_IMAGE: // Scene background resource
			if (SceneModule.bg.loaded) {
				R_printf(R_STDERR, "Error: Multiple background resources encountered.\n");
				return R_FAILURE;
			}

			R_printf(R_STDOUT, "Loading background resource.\n");
			SceneModule.bg.res_buf = SceneModule.reslist[i].res_data;
			SceneModule.bg.res_len = SceneModule.reslist[i].res_data_len;
			SceneModule.bg.loaded = 1;

			if (IMG_DecodeBGImage(SceneModule.bg.res_buf,
				SceneModule.bg.res_len,
				&SceneModule.bg.buf,
				&SceneModule.bg.buf_len,
				&SceneModule.bg.w,
				&SceneModule.bg.h) != R_SUCCESS) {
				R_printf(R_STDERR, "Error loading background resource: %u\n", SceneModule.reslist[i].res_number);
				return R_FAILURE;
			}

			pal_p = IMG_GetImagePal(SceneModule.bg.res_buf, SceneModule.bg.res_len);
			memcpy(SceneModule.bg.pal, pal_p, sizeof SceneModule.bg.pal);
			SceneModule.scene_mode = R_SCENE_MODE_NORMAL;
			break;
		case SAGA_BG_MASK: // Scene background mask resource
			if (SceneModule.bg_mask.loaded) {
				R_printf(R_STDERR, "Error: Duplicate background mask resource encountered.\n");
			}
			R_printf(R_STDOUT, "Loading BACKGROUND MASK resource.\n");
			SceneModule.bg_mask.res_buf = SceneModule.reslist[i].res_data;
			SceneModule.bg_mask.res_len = SceneModule.reslist[i].res_data_len;
			SceneModule.bg_mask.loaded = 1;
			IMG_DecodeBGImage(SceneModule.bg_mask.res_buf, SceneModule.bg_mask.res_len, &SceneModule.bg_mask.buf,
							&SceneModule.bg_mask.buf_len, &SceneModule.bg_mask.w, &SceneModule.bg_mask.h);
			break;
		case SAGA_OBJECT_NAME_LIST:
			R_printf(R_STDOUT, "Loading object name list resource...\n");
			OBJECTMAP_LoadNames(SceneModule.reslist[i].res_data, SceneModule.reslist[i].res_data_len);
			break;
		case SAGA_OBJECT_MAP:
			R_printf(R_STDOUT, "Loading object map resource...\n");
			if (OBJECTMAP_Load(res_data,
				res_data_len) != R_SUCCESS) {
				R_printf(R_STDERR, "Error loading object map resource.\n");
				return R_FAILURE;
			}
			break;
		case SAGA_ACTION_MAP:
			R_printf(R_STDOUT, "Loading exit map resource...\n");
			if (ACTIONMAP_Load(res_data, res_data_len) != R_SUCCESS) {
				R_printf(R_STDERR, "Error loading exit map resource.\n");
				return R_FAILURE;
			}
			break;
		case SAGA_ISO_TILESET:
			if (SceneModule.scene_mode == R_SCENE_MODE_NORMAL) {
				R_printf(R_STDERR, "Isometric tileset incompatible with normal scene mode.\n");
				return R_FAILURE;
			}

			R_printf(R_STDOUT, "Loading isometric tileset resource.\n");

			if (ISOMAP_LoadTileset(res_data, res_data_len) != R_SUCCESS) {
				R_printf(R_STDERR, "Error loading isometric tileset resource.\n");
				return R_FAILURE;
			}

			SceneModule.scene_mode = R_SCENE_MODE_ISO;
			break;
		case SAGA_ISO_METAMAP:
			if (SceneModule.scene_mode == R_SCENE_MODE_NORMAL) {
				R_printf(R_STDERR, "Isometric metamap incompatible with normal scene mode.\n");
				return R_FAILURE;
			}

			R_printf(R_STDOUT, "Loading isometric metamap resource.\n");

			if (ISOMAP_LoadMetamap(res_data, res_data_len) != R_SUCCESS) {
				R_printf(R_STDERR, "Error loading isometric metamap resource.\n");
				return R_FAILURE;
			}

			SceneModule.scene_mode = R_SCENE_MODE_ISO;
			break;
		case SAGA_ISO_METATILESET:
			if (SceneModule.scene_mode == R_SCENE_MODE_NORMAL) {
				R_printf(R_STDERR, "Isometric metatileset incompatible with normal scene mode.\n");
				return R_FAILURE;
			}

			R_printf(R_STDOUT, "Loading isometric metatileset resource.\n");

			if (ISOMAP_LoadMetaTileset(res_data, res_data_len) != R_SUCCESS) {
				R_printf(R_STDERR, "Error loading isometric tileset resource.\n");
				return R_FAILURE;
			}

			SceneModule.scene_mode = R_SCENE_MODE_ISO;
			break;
		case SAGA_ANIM_1:
		case SAGA_ANIM_2:
		case SAGA_ANIM_3:
		case SAGA_ANIM_4:
		case SAGA_ANIM_5:
		case SAGA_ANIM_6:
		case SAGA_ANIM_7:
			{
				SCENE_ANIMINFO *new_animinfo;
				uint16 new_anim_id;

				R_printf(R_STDOUT, "Loading animation resource...\n");

				new_animinfo = (SCENE_ANIMINFO *)malloc(sizeof *new_animinfo);
				if (new_animinfo == NULL) {
					R_printf(R_STDERR, "Memory allocation error.\n");
					return R_MEM;
				}

				if (ANIM_Load(SceneModule.reslist[i].res_data,
					SceneModule.reslist[i].res_data_len,
					&new_anim_id) == R_SUCCESS) {
				} else {
					R_printf(R_STDERR, "Error loading animation resource\n");
					return R_FAILURE;
				}

				new_animinfo->anim_handle = new_anim_id;
				new_animinfo->anim_res_number =  SceneModule.reslist[i].res_number;
				ys_dll_add_tail(SceneModule.anim_list, new_animinfo, sizeof *new_animinfo);
				SceneModule.anim_entries++;
			}
			break;
		case SAGA_PAL_ANIM:
			R_printf(R_STDOUT, "Loading palette animation resource.\n");
			PALANIM_Load(SceneModule.reslist[i].res_data, SceneModule.reslist[i].res_data_len);
			break;
		default:
			R_printf(R_STDERR, "Encountered unknown resource type: %d\n", SceneModule.reslist[i].res_type);
			break;
		}
	}

	return R_SUCCESS;
}

int SCENE_Draw(R_SURFACE *dst_s) {
	R_GAME_DISPLAYINFO disp_info;
	R_BUFFER_INFO buf_info;
	R_POINT bg_pt;

	assert(SceneModule.init);

	RENDER_GetBufferInfo(&buf_info);
	GAME_GetDisplayInfo(&disp_info);

	bg_pt.x = 0;
	bg_pt.y = 0;

	switch (SceneModule.scene_mode) {

	case R_SCENE_MODE_NORMAL:
		GFX_BufToSurface(dst_s, buf_info.r_bg_buf, disp_info.logical_w,
						MAX(disp_info.scene_h, SceneModule.bg.h), NULL, &bg_pt);
		break;
	case R_SCENE_MODE_ISO:
		ISOMAP_Draw(dst_s);
		break;
	default:
		// Unknown scene mode
		return R_FAILURE;
		break;
	};

	return R_SUCCESS;
}

int SCENE_End() {
	R_SCENE_INFO scene_info;

	assert(SceneModule.init);

	if (SceneModule.scene_loaded != 1) {
		R_printf(R_STDERR, "SCENE_End(): No scene to end.\n");
		return -1;
	}

	R_printf(R_STDOUT, "SCENE_End(): Ending scene...\n");

	SCENE_GetInfo(&scene_info);

	SceneModule.scene_proc(SCENE_END, &scene_info);

	if (SceneModule.desc.script_num > 0) {
		SCRIPT_Free();
	}

	// Free scene background
	if (SceneModule.bg.loaded) {
		free(SceneModule.bg.buf);
		SceneModule.bg.loaded = 0;
	}

	// Free scene background mask
	if (SceneModule.bg_mask.loaded) {
		free(SceneModule.bg_mask.buf);
		SceneModule.bg_mask.loaded = 0;
	}

	// Free scene resource list
	if (SceneModule.load_desc) {

		free(SceneModule.reslist);
	}

	// Free animation info list
	ANIM_Reset();

	PALANIM_Free();
	OBJECTMAP_Free();
	ACTIONMAP_Free();

	ys_dll_destroy(SceneModule.anim_list);

	SceneModule.anim_entries = 0;

	EVENT_ClearList();
	TEXT_ClearList(SceneModule.text_list);

	SceneModule.scene_loaded = 0;

	return R_SUCCESS;
}

void CF_scenechange(int argc, char *argv[]) {
	int scene_num = 0;

	if ((argc == 0) || (argc > 1)) {
		return;
	}

	scene_num = atoi(argv[0]);

	if ((scene_num < 1) || (scene_num > SceneModule.scene_max)) {
		CON_Print("Invalid scene number.");
		return;
	}

	SCENE_ClearQueue();

	if (SCENE_Change(scene_num) == R_SUCCESS) {
		CON_Print("Scene changed.");
	} else {
		CON_Print("Couldn't change scene!");
	}
}

void CF_sceneinfo(int argc, char *argv[]) {
	const char *fmt = "%-20s %d";

	YS_IGNORE_PARAM(argc);
	YS_IGNORE_PARAM(argv);

	CON_Print(fmt, "Scene number:", SceneModule.scene_number);
	CON_Print(fmt, "Descriptor R#:", SceneModule.scene_rn);
	CON_Print("-------------------------");
	CON_Print(fmt, "Unknown:", SceneModule.desc.unknown0);
	CON_Print(fmt, "Resource list R#:", SceneModule.desc.res_list_rn);
	CON_Print(fmt, "End slope:", SceneModule.desc.end_slope);
	CON_Print(fmt, "Begin slope:", SceneModule.desc.begin_slope);
	CON_Print(fmt, "Script resource:", SceneModule.desc.script_num);
	CON_Print(fmt, "Scene script:", SceneModule.desc.scene_scriptnum);
	CON_Print(fmt, "Start script:", SceneModule.desc.start_scriptnum);
	CON_Print(fmt, "Music R#", SceneModule.desc.music_rn);
}

} // End of namespace Saga
