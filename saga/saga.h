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

#ifndef SAGA_H
#define SAGA_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/util.h"
#include "common/stream.h"
#include "common/rect.h"

#include <limits.h>
#include <stddef.h>

#include "saga/text.h"
#include "saga/gfx.h"

namespace Saga {

class SndRes;
class Sound;
class Music;
class Anim;
class Render;
class IsoMap;
class Gfx;
class Script;
class Actor;
class Font;
class Sprite;
class Scene;
class Interface;
class Console;
class Events;
class PalAnim;


#define PBOUNDS(n,max) (((n)>=(0))&&((n)<(max)))
#define MAXPATH 512

struct RSCFILE_CONTEXT;
struct SPRITELIST;

enum ERRORCODE {
	MEM = -2,
	FAILURE = -1,
	SUCCESS = 0
};

enum SAGAGameType {
	GType_ITE,
	GType_IHNM
};

enum scriptTimings {
	kScriptTimeTicksPerSecond = (728L/10L),
	kRepeatSpeed = 40   // 25 frames/sec
};

enum Directions {
	kDirUp = 0,
	kDirUpRight = 1,
	kDirRight = 2,
	kDirDownRight = 3,
	kDirDown = 4,
	kDirDownLeft = 5,
	kDirLeft = 6,
	kDirUpLeft = 7
};

enum HitZoneFlags {
	kHitZoneEnabled = (1 << 0),   // Zone is enabled
	kHitZoneExit = (1 << 1),      // Causes char to exit

	//	The following flag causes the zone to act differently.
	//	When the actor hits the zone, it will immediately begin walking
	//	in the specified direction, and the actual specified effect of
	//	the zone will be delayed until the actor leaves the zone.
	kHitZoneAutoWalk = (1 << 2),

	//	zone activates only when character stops walking
	kHitZoneTerminus = (1 << 3)
};

struct CLICKAREA {
	int n_points;
	Point *points;
};


#define IS_BIG_ENDIAN ((_vm->_features & GF_BIG_ENDIAN_DATA) != 0)


enum GAME_IDS {
	// Dreamers Guild
	GID_ITE_DEMO_G = 0,
	GID_ITE_DISK_G,
	GID_ITE_CD_G,
	GID_ITE_MAC_G,  // TODO

	// Wyrmkeep
	GID_ITE_CD,       // data for Win rerelease is same as in old DOS
	GID_ITE_MACCD,
	GID_ITE_LINCD,
	GID_ITE_WINDEMO1, // older Wyrmkeep windows demo
	GID_ITE_MACDEMO1, // older Wyrmkeep mac demo
	GID_ITE_LINDEMO,
	GID_ITE_WINDEMO2,
	GID_ITE_MACDEMO2,

	// German
	GID_ITE_DISK_DE,
	GID_ITE_AMIGACD_DE, // TODO
	GID_ITE_OLDMAC_DE,  // TODO
	GID_ITE_AMIGA_FL_DE,
	GID_ITE_CD_DE,      // don't have it
	GID_ITE_AMIGA_AGA_DEMO,
	GID_ITE_AMIGA_ECS_DEMO,

	GID_IHNM_DEMO,
	GID_IHNM_CD
};

enum GAME_FILETYPES {
	GAME_RESOURCEFILE = 0x01,
	GAME_SCRIPTFILE = 0x02,
	GAME_SOUNDFILE = 0x04,
	GAME_VOICEFILE = 0x08,
	GAME_DEMOFILE = 0x10,
	GAME_MUSICFILE = 0x20,
	GAME_MUSICFILE_GM = 0x40,
	GAME_MUSICFILE_FM = 0x80
};

enum GAME_SOUNDINFO_TYPES {
	GAME_SOUND_PCM = 0,
	GAME_SOUND_VOC,
	GAME_SOUND_WAV,
	GAME_SOUND_VOX
};

enum GAME_FONT_IDS {
	GAME_FONT_SMALL = 0,
	GAME_FONT_MEDIUM,
	GAME_FONT_LARGE,
	GAME_FONT_SMALL2,
	GAME_FONT_MEDIUM2,
	GAME_FONT_LARGE2,
	GAME_FONT_LARGE3
};

enum GAME_FEATURES {
	GF_VOX_VOICES = 1,
	GF_BIG_ENDIAN_DATA = 2,
	GF_MAC_RESOURCES = 4,
	GF_LANG_DE = 8
};

struct GAME_DISPLAYINFO {
	int logical_w;
	int logical_h;
	int scene_h;
};

struct GAME_SOUNDINFO {
	int res_type;
	long freq;
	int sample_size;
	int stereo;
};

struct GAME_FONTDESC {
	uint16 font_id;
	uint32 font_rn;
};

struct GAME_SCENEDESC {
	uint32 scene_lut_rn;
	uint32 first_scene;
};

struct GAME_RESOURCEDESC {
	uint32 scene_lut_rn;
	uint32 script_lut_rn;
	uint32 command_panel_rn;
	uint32 dialogue_panel_rn;
};

inline int ticksToMSec(int tick) {
	return tick * 1000 / kScriptTimeTicksPerSecond;
}

inline int clamp(int minValue, int value, int maxValue) {
	if (value <= minValue) {
		return minValue;
	} else {
		if (value >= maxValue) {
			return maxValue;
		} else {
			return value;
		}
	}
}

inline int integerCompare(int i1, int i2) {
	return ((i1) > (i2) ? 1 : ((i1) < (i2) ? -1 : 0));
}

class SagaEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);

protected:
	int go();
	int init(GameDetector &detector);

public:
	SagaEngine(GameDetector * detector, OSystem * syst);
	virtual ~SagaEngine();

	void shutdown();

	int _soundEnabled;
	int _musicEnabled;

	int _gameId;
	int _gameType;
	uint32 _features;

	SndRes *_sndRes;
	Sound *_sound;
	Music *_music;
	Anim *_anim;
	Render *_render;
	IsoMap *_isoMap;
	Gfx *_gfx;
	Script *_script;
	Actor *_actor;
	Font *_font;
	Sprite *_sprite;
	Scene *_scene;
	Interface *_interface;
	Console *_console;
	Events *_events;
	PalAnim *_palanim;

	SPRITELIST *_mainSprites;

	/** Random number generator */
	Common::RandomSource _rnd;

private:
	int decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len);
	int flipImage(byte *img_buf, int columns, int scanlines);
	int unbankBGImage(byte *dest_buf, const byte *src_buf, int columns, int scanlines);
	uint32 _previousTicks;

public:
	int decodeBGImage(const byte *image_data, size_t image_size,
						byte **output_buf, size_t *output_buf_len, int *w, int *h);
	const byte *getImagePal(const byte *image_data, size_t image_size);

public:
	TEXTLIST *textCreateList();
	void textDestroyList(TEXTLIST *textlist);
	void textClearList(TEXTLIST *textlist);
	int textDrawList(TEXTLIST *textlist, SURFACE *ds);
	TEXTLIST_ENTRY *textAddEntry(TEXTLIST *textlist, TEXTLIST_ENTRY *entry);
	int textDeleteEntry(TEXTLIST *textlist, TEXTLIST_ENTRY *entry);
	int textSetDisplay(TEXTLIST_ENTRY *entry, int val);
	int textDraw(int font_id, SURFACE *ds, const char *string, int text_x, int text_y, int color,
				  int effect_color, int flags);
	int textProcessList(TEXTLIST *textlist, long ms);

	int transitionDissolve(byte *dst_img, int dst_w, int dst_h, int dst_p, 
			const byte *src_img, int src_w, int src_h, int src_p, int flags, int x, int y, 
			double percent);

	int processInput(void);
	Point getMousePos();

 private:
	Point _mousePos;

public:
	int initGame(void);
	RSCFILE_CONTEXT *getFileContext(uint16 type, int param);
	int getFontInfo(GAME_FONTDESC **, int *);
	const GAME_RESOURCEDESC getResourceInfo(void);
	const GAME_SOUNDINFO getSoundInfo(void);
	int getDisplayInfo(GAME_DISPLAYINFO *disp_info);
	int getSceneInfo(GAME_SCENEDESC *);
	int getDisplayWidth();
	int getDisplayHeight();
	int getStatusYOffset();
	int getPathYOffset();
private:
	int loadLanguage(void);
	int loadGame(int game_n_p);
};

// FIXME: Global var. We use it until everything will be turned into objects
extern SagaEngine *_vm;

} // End of namespace Saga

#endif
