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

#ifndef ULTIMA8_GRAPHICS_SKFPlayer_H
#define ULTIMA8_GRAPHICS_SKFPlayer_H

#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

struct SKFEvent;
class RawArchive;
class RenderSurface;
class RenderedText;
class IDataSource;
namespace Pentagram {
struct Palette;
}

class SKFPlayer {
public:
	SKFPlayer(RawArchive *movie, int width, int height, bool introMusicHack = false);
	~SKFPlayer();

	void run();
	void paint(RenderSurface *surf, int lerp);

	void start();
	void stop();
	bool isPlaying() const {
		return playing;
	}

private:

	void parseEventList(IDataSource *eventlist);

	int width, height;
	RawArchive *skf;
	std::vector<SKFEvent *> events;
	unsigned int curframe, curobject;
	unsigned int curaction;
	unsigned int curevent;
	bool playing;
	unsigned int lastupdate;
	unsigned int timer;
	unsigned int framerate;
	uint8 fadecolour, fadelevel;
	RenderSurface *buffer;
	RenderedText *subs;
	int subtitley;
	bool introMusicHack;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
