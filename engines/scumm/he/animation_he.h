/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include "common/file.h"
#include "graphics/dxa_player.h"

namespace Scumm {

class ScummEngine_v90he;

class MoviePlayer : public Graphics::DXAPlayer {
	ScummEngine_v90he *_vm;

	uint32 _flags;
	uint32 _wizResNum;
	
public:
	MoviePlayer(ScummEngine_v90he *vm);

	int getImageNum();
	int load(const char *filename, int flags, int image = 0);

	void handleNextFrame();

protected:
	virtual void setPalette(byte *pal);
};

} // End of namespace Simon

#endif
