/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on Wintermute Engine
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/video/subtitle_card.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

SubtitleCard::SubtitleCard(BaseGame *inGame,
						   const Common::String &text,
						   const uint &startFrame,
						   const uint &endFrame) : _gameRef(inGame),
	_startFrame(startFrame),
	_endFrame(endFrame) {
	_text = text;
	_gameRef->expandStringByStringTable(_text);
}

uint32 SubtitleCard::getStartFrame() const {
	return _startFrame;
}

uint32 SubtitleCard::getEndFrame() const {
	return _endFrame;
}

Common::String SubtitleCard::getText() const {
	return _text;
}

} // End of namespace Wintermute
