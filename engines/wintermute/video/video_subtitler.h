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

#ifndef WINTERMUTE_VIDSUBTITLER_H
#define WINTERMUTE_VIDSUBTITLER_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/video/subtitle_card.h"

namespace Wintermute {

class VideoSubtitler : public BaseClass {
public:
	VideoSubtitler(BaseGame *inGame);
	~VideoSubtitler(void) override;
	bool loadSubtitles(const Common::String &filename, const Common::String &subtitleFile);
	void display();
	void update(uint32 frame);
private:
	Common::Array<SubtitleCard> _subtitles;
	int32 _lastSample;
	bool _showSubtitle;
	uint32 _currentSubtitle;
};

} // End of namespace Wintermute

#endif
