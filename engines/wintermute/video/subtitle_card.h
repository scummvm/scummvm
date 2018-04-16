/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on Wintermute Engine
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SUBTITLECARD_H
#define WINTERMUTE_SUBTITLECARD_H

#include "common/str.h"

namespace Wintermute {

class BaseGame;

class SubtitleCard {
public:
	SubtitleCard(BaseGame *inGame, const Common::String &text, const uint &startFrame, const uint &endFrame);
	uint32 getEndFrame() const;
	uint32 getStartFrame() const;
	Common::String getText() const;
private:
	BaseGame *_gameRef;
	uint32 _endFrame;
	uint32 _startFrame;
	Common::String _text;
};

} // End of namespace Wintermute

#endif
