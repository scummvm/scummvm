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
/*
 Description:   
 
    Scene management module private header file

 Notes: 
*/

#ifndef SAGA_SCENE_H
#define SAGA_SCENE_H

namespace Saga {

enum SCENE_LOAD_FLAGS {

	BY_RESOURCE = 0,
	BY_SCENE,
	BY_DESC
};

enum SCENE_PROC_PARAMS {

	SCENE_BEGIN = 0,
	SCENE_END
};

/* Resource type numbers */
enum SAGA_RESOURCE_TYPES {

	SAGA_BG_IMAGE = 2,
	SAGA_BG_MASK = 3,
	SAGA_OBJECT_NAME_LIST = 5,
	SAGA_OBJECT_MAP = 6,
	SAGA_ACTION_MAP = 7,
	SAGA_ISO_TILESET = 8,
	SAGA_ISO_METAMAP = 9,
	SAGA_ISO_METATILESET = 10,
	SAGA_ANIM_1 = 14,
	SAGA_ANIM_2,
	SAGA_ANIM_3,
	SAGA_ANIM_4,
	SAGA_ANIM_5,
	SAGA_ANIM_6,
	SAGA_ANIM_7,
	SAGA_PAL_ANIM = 23
};

#define SAGA_RESLIST_ENTRY_LEN 4

struct R_SCENE_RESLIST {

	uint32 res_number;
	int res_type;

	byte *res_data;
	size_t res_data_len;

};

#define SAGA_SCENE_DESC_LEN 16

struct R_SCENE_DESC {

	int unknown0;
	int res_list_rn;
	int end_slope;
	int begin_slope;
	int script_num;
	int scene_scriptnum;
	int start_scriptnum;
	int music_rn;

	R_SCENE_RESLIST *res_list;
	size_t res_list_ct;

};

struct SCENE_IMAGE {

	int loaded;

	int w;
	int h;
	int p;

	byte *buf;
	size_t buf_len;

	byte *res_buf;
	size_t res_len;

	PALENTRY pal[256];

};

struct SCENE_ANIMINFO {

	int anim_res_number;
	int anim_handle;

	SCENE_ANIMINFO *next;

};

struct R_SCENE_QUEUE {

	uint32 scene_n;
	R_SCENE_DESC *scene_desc;
	int load_flag;

	R_SCENE_PROC *scene_proc;
	int scene_skiptarget;

};

struct R_SCENE_MODULE {

	int init;

	R_RSCFILE_CONTEXT *scene_ctxt;

	int *scene_lut;
	int scene_count;
	int scene_max;

	YS_DL_LIST *scene_queue;

	int first_scene;

	int scene_loaded;
	int scene_mode;
	int scene_number;
	int scene_rn;
	int in_game;

	int load_desc;
	R_SCENE_DESC desc;

	int reslist_loaded;
	int reslist_entries;
	R_SCENE_RESLIST *reslist;

	int anim_entries;
	YS_DL_LIST *anim_list;

	R_SCENE_PROC *scene_proc;

	R_TEXTLIST *text_list;

	SCENE_IMAGE bg;
	SCENE_IMAGE bg_mask;

};

int SCENE_Queue(R_SCENE_QUEUE * scene_queue);
int SCENE_ClearQueue(void);

int
SCENE_Load(int scene,
    int load_flag, R_SCENE_PROC scene_proc, R_SCENE_DESC *);

int LoadSceneDescriptor(uint32 res_number);

int LoadSceneResourceList(uint32 res_number);

int ProcessSceneResources(void);

void CF_scenechange(int argc, char *argv[]);

void CF_sceneinfo(int argc, char *argv[]);

/*
 * r_sceneproc.c                                                
\*--------------------------------------------------------------------------*/

int IHNM_StartProc(void);

int InitialSceneProc(int param, R_SCENE_INFO * scene_info);
int DefaultSceneProc(int param, R_SCENE_INFO * scene_info);

/*
 * r_ite_introproc.c                                             
\*--------------------------------------------------------------------------*/

int ITE_StartProc(void);
int ITE_IntroAnimProc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroCave1Proc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroCave2Proc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroCave3Proc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroCave4Proc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroValleyProc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroTreeHouseProc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroFairePathProc(int param, R_SCENE_INFO * scene_info);
int ITE_IntroFaireTentProc(int param, R_SCENE_INFO * scene_info);

/*
 * r_ihnm_introproc.c                                             
\*--------------------------------------------------------------------------*/
int IHNM_StartProc(void);
int IHNM_IntroMovieProc1(int param, R_SCENE_INFO * scene_info);
int IHNM_IntroMovieProc2(int param, R_SCENE_INFO * scene_info);
int IHNM_IntroMovieProc3(int param, R_SCENE_INFO * scene_info);
int IHNM_HateProc(int param, R_SCENE_INFO * scene_info);

} // End of namespace Saga

#endif				/* SAGA_SCENE_H_ */
