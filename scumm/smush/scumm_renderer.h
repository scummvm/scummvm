/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SCUMM_RENDERER_H
#define SCUMM_RENDERER_H

#include "config.h"

#ifdef DEBUG
# ifndef NO_DEBUG_SCUMM_RENDERER
#  define DEBUG_SCUMM_RENDERER
# endif
#else
# ifdef DEBUG_SCUMM_RENDERER
#  error DEBUG_SCUMM_RENDERER defined without DEBUG
# endif
#endif

#include "brenderer.h"

class ScummMixer;
class Scumm;
class Mixer;

class ScummRenderer : public BaseRenderer {
private:
	Scumm *_scumm;
	ScummMixer *_smixer;
	uint32 _insaneSpeed;
	volatile int _pending_updates;
public:
	ScummRenderer(Scumm *scumm, uint32 speed);
	virtual ~ScummRenderer();
	virtual bool wait(int32 ms);
	bool update();
protected:
	virtual bool initFrame(const Point &size);
	virtual byte *lockFrame(int32 frame);
	virtual void clean();
	virtual bool startDecode(const char *fname, int32 version, int32 nbframes);
	virtual bool setPalette(const Palette & pal);
	virtual void save();
	virtual Mixer *getMixer();
	virtual bool prematureClose();
};

#endif
