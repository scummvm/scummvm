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

namespace Saga {

class SndRes;
class Sound;
class Music;
class Anim;
class Render;
class ActionMap;

using Common::MemoryReadStream;

#define R_PBOUNDS(n,max) (((n)>=(0))&&((n)<(max)))
#define R_MAXPATH 512

struct R_RSCFILE_CONTEXT;
struct R_SEMAPHORE;

typedef Common::Point R_POINT;
typedef Common::Rect R_RECT;

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

	SndRes *_sndRes;
	Sound *_sound;
	Music *_music;
	Anim *_anim;
	Render *_render;
	ActionMap *_actionMap;

private:
	int decodeBGImageRLE(const byte *inbuf, size_t inbuf_len, byte *outbuf, size_t outbuf_len);
	int flipImage(byte *img_buf, int columns, int scanlines);
	int unbankBGImage(byte *dest_buf, const byte *src_buf, int columns, int scanlines);

public:
	int decodeBGImage(const byte *image_data, size_t image_size,
						byte **output_buf, size_t *output_buf_len, int *w, int *h);
	const byte *getImagePal(const byte *image_data, size_t image_size);
};

// FIXME: Global var. We use it until everything will be turned into objects
extern SagaEngine *_vm;

} // End of namespace Saga

#endif
