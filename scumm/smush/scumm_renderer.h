/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "mixer.h"
#include "rect.h"
#include "blitter.h"

#ifndef min
#define min(x, y) ((x) > (y) ? (y) : (x))
#endif

class scumm_mixer;

class ScummRenderer : public BaseRenderer {
private:
	Scumm * _scumm;
	scumm_mixer * _smixer;
	volatile bool _wait;
public:
	ScummRenderer(Scumm * scumm);
	virtual ~ScummRenderer();
	virtual bool wait(int ms);
	bool update();
protected:
	virtual bool startDecode(const char * fname, int version, int nbframes);
	virtual bool setPalette(const Palette & pal);
	virtual void save(int frame = -1);
	virtual Mixer * getMixer();
	virtual bool prematureClose();
};

#endif
