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
class ActionMap;
class IsoMap;
class ObjectMap;
class Gfx;
class SData;
class Script;
class Actor;
class Font;
class Sprite;
class Scene;

using Common::MemoryReadStream;

#define R_PBOUNDS(n,max) (((n)>=(0))&&((n)<(max)))
#define R_MAXPATH 512

struct R_RSCFILE_CONTEXT;
struct R_SEMAPHORE;

enum R_ERRORCODE {
	R_MEM = -2,
	R_FAILURE = -1,
	R_SUCCESS = 0
};

enum SAGAGameId {
	GID_ITE,
	GID_ITECD,
	GID_IHNM
};

int TRANSITION_Dissolve(byte *dst_img, int dst_w, int dst_h,
	int dst_p, const byte *src_img, int src_p, int flags, double percent);

int SYSINPUT_ProcessInput(void);
R_POINT SYSINPUT_GetMousePos();

class SagaEngine : public Engine {
	void errorString(const char *buf_input, char *buf_output);

protected:
	void go();

public:
	SagaEngine(GameDetector * detector, OSystem * syst);
	virtual ~SagaEngine();

	void shutdown();

	int _soundEnabled;
	int _musicEnabled;

	SndRes *_sndRes;
	Sound *_sound;
	Music *_music;
	Anim *_anim;
	Render *_render;
	ActionMap *_actionMap;
	IsoMap *_isoMap;
	ObjectMap *_objectMap;
	Gfx *_gfx;
	SData *_sdata;
	Script *_script;
	Actor *_actor;
	Font *_font;
	Sprite *_sprite;
	Scene *_scene;
	
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
	R_TEXTLIST *textCreateList();
	void textDestroyList(R_TEXTLIST *textlist);
	void textClearList(R_TEXTLIST *textlist);
	int textDrawList(R_TEXTLIST *textlist, R_SURFACE *ds);
	R_TEXTLIST_ENTRY *textAddEntry(R_TEXTLIST *textlist, R_TEXTLIST_ENTRY *entry);
	int textDeleteEntry(R_TEXTLIST *textlist, R_TEXTLIST_ENTRY *entry);
	int textSetDisplay(R_TEXTLIST_ENTRY *entry, int val);
	int textDraw(int font_id, R_SURFACE *ds, const char *string, int text_x, int text_y, int color,
				  int effect_color, int flags);
	int textProcessList(R_TEXTLIST *textlist, long ms);

};

// FIXME: Global var. We use it until everything will be turned into objects
extern SagaEngine *_vm;

} // End of namespace Saga

#endif
