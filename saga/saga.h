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
class IsoMap;
class Gfx;
class SData;
class Script;
class Actor;
class Font;
class Sprite;
class Scene;
class Interface;
class Console;
class Events;
class PalAnim;

using Common::MemoryReadStream;

#define PBOUNDS(n,max) (((n)>=(0))&&((n)<(max)))
#define MAXPATH 512

struct RSCFILE_CONTEXT;
struct SEMAPHORE;
struct SPRITELIST;

enum ERRORCODE {
	MEM = -2,
	FAILURE = -1,
	SUCCESS = 0
};

enum SAGAGameId {
	GID_ITE,
	GID_ITECD,
	GID_IHNM
};

struct CLICKAREA {
	int n_points;
	Point *points;
};

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
	IsoMap *_isoMap;
	Gfx *_gfx;
	SData *_sdata;
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
};

// FIXME: Global var. We use it until everything will be turned into objects
extern SagaEngine *_vm;

} // End of namespace Saga

#endif
