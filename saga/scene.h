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

// Scene management module private header file

#ifndef SAGA_SCENE_H
#define SAGA_SCENE_H

#include "saga/text.h"

namespace Saga {

enum R_SCENE_MODES {
	R_SCENE_MODE_INVALID,
	R_SCENE_MODE_NORMAL,
	R_SCENE_MODE_ISO
};

struct SCENE_ZINFO {
	int beginSlope;
	int endSlope;
};

struct SCENE_BGINFO {
	int bg_x;
	int bg_y;
	int bg_w;
	int bg_h;
	int bg_p;
	byte *bg_buf;
	size_t bg_buflen;
};

struct R_SCENE_INFO {
	SCENE_ZINFO z_info;
	SCENE_BGINFO bg_info;
	R_TEXTLIST *text_list;
};

typedef int (R_SCENE_PROC) (int, R_SCENE_INFO *);

#define PALETTE_FADE_DURATION 1000

enum SCENE_LOAD_FLAGS {
	BY_RESOURCE = 0,
	BY_SCENE,
	BY_DESC
};

enum SCENE_PROC_PARAMS {
	SCENE_BEGIN = 0,
	SCENE_END
};

// Resource type numbers
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
	int16 flags;
	int16 resListRN;
	int16 endSlope;
	int16 beginSlope;
	uint16 scriptNum;
	uint16 sceneScriptNum;
	uint16 startScriptNum;
	int16 musicRN;
	R_SCENE_RESLIST *resList;
	size_t resListCnt;
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

enum SCENE_FADE_TYPES {
	SCENE_NOFADE = 0,
	SCENE_FADE = 1,
	SCENE_FADE_NO_INTERFACE = 2
};

struct R_SCENE_QUEUE {
	uint32 scene_n;
	R_SCENE_DESC *scene_desc;
	int load_flag;
	R_SCENE_PROC *scene_proc;
	int scene_skiptarget;
	int fadeType;
};

class Scene {
 public:
	Scene(SagaEngine *vm);
	~Scene();
	int reg();

	int startScene();
	int nextScene();
	int skipScene();
	int endScene();
	int queueScene(R_SCENE_QUEUE *scene_queue);
	int draw(R_SURFACE *);
	int getMode();
	int getBGMaskInfo(int *w, int *h, byte **buf, size_t *buf_len);
	int isBGMaskPresent(void);
	int getBGInfo(SCENE_BGINFO *bginfo);
	int getZInfo(SCENE_ZINFO *zinfo);
	int getBGPal(PALENTRY **pal);
	int getInfo(R_SCENE_INFO *si);

	int clearSceneQueue(void);
	int changeScene(int scene_num);

	bool initialized() { return _initialized; }

	void sceneInfoCmd(int argc, char *argv[]);
	void sceneChangeCmd(int argc, char *argv[]);

 private:
	int loadScene(int scene, int load_flag, R_SCENE_PROC scene_proc, R_SCENE_DESC *, 
				  int fadeIn);
	int loadSceneDescriptor(uint32 res_number);
	int loadSceneResourceList(uint32 res_number);
	int processSceneResources();

 private:
	SagaEngine *_vm;
	bool _initialized;

	R_RSCFILE_CONTEXT *_sceneContext;
	int *_sceneLUT;
	int _sceneCount;
	int _sceneMax;
	YS_DL_LIST *_sceneQueue;
	int _firstScene;
	bool _sceneLoaded;
	int _sceneMode;
	int _sceneNumber;
	int _sceneResNum;
	bool _inGame;
	bool _loadDesc;
	R_SCENE_DESC _desc;
	int _resListEntries;
	R_SCENE_RESLIST *_resList;
	int _animEntries;
	YS_DL_LIST *_animList;
	R_SCENE_PROC *_sceneProc;
	R_TEXTLIST *_textList;
	SCENE_IMAGE _bg;
	SCENE_IMAGE _bgMask;
};

} // End of namespace Saga

#endif
