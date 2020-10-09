/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_BINK_PLAYER_H
#define GRIM_BINK_PLAYER_H

#include "common/list.h"

#include "engines/grim/movie/movie.h"

#ifdef USE_BINK

namespace Grim {

class BinkPlayer : public MoviePlayer {
	struct Subtitle {
		Subtitle(unsigned int startFrame, unsigned int endFrame, const char *textId) :
			_startFrame(startFrame), _endFrame(endFrame), _textId(textId), active(false) { };
		unsigned int _startFrame;
		unsigned int _endFrame;
		Common::String _textId;
		bool active;
	};

public:
	BinkPlayer(bool demo);
private:
	Common::List<Subtitle> _subtitles;
	Common::List<Subtitle>::iterator _subtitleIndex;
	bool loadFile(const Common::String &filename) override;
	bool _demo;
	bool bikCheck(Common::SeekableReadStream *stream, uint32 pos);
	virtual void deinit() override;
	virtual void handleFrame() override;
};

} // end of namespace Grim

#endif // USE_BINK

#endif
