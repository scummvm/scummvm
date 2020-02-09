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

#ifndef CHEWY_CHEWY_H
#define CHEWY_CHEWY_H


#include "common/scummsys.h"
#include "common/file.h"
#include "common/util.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"

#include "engines/engine.h"

namespace Chewy {

struct ChewyGameDescription;
class Console;
class Cursor;
class Events;
class Graphics;
class Scene;
class Sound;
class Text;

class ChewyEngine : public Engine {
public:
	ChewyEngine(OSystem *syst, const ChewyGameDescription *gameDesc);
	~ChewyEngine() override;

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	const ChewyGameDescription *_gameDescription;
	Common::RandomSource _rnd;

	void setPlayVideo(uint num) { _videoNum = num; }

	Graphics *_graphics;
	Cursor *_cursor;
	Scene *_scene;
	Sound *_sound;
	Text *_text;

protected:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

	void initialize();
	void shutdown();

	Events *_events;

	uint _curCursor;
	uint _elapsedFrames;
	int _videoNum;
};

} // End of namespace Chewy

#endif
