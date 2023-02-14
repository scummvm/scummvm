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

#ifndef CHAMBER_H
#define CHAMBER_H

#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace Audio {
class SoundHandle;
class PCSpeaker;
}

struct ADGameDescription;

namespace Chamber {

class ChamberEngine : public Engine {
private:
	// We need random numbers
	Common::RandomSource *_rnd;

public:
	ChamberEngine(OSystem *syst, const ADGameDescription *desc);
	~ChamberEngine();

	Common::Language getLanguage() const;

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return true; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncGameStream(Common::Serializer &s);

	byte readKeyboardChar();

	void initSound();
	void deinitSound();

public:
	bool _shouldQuit;

	byte *_pxiData;

	Audio::PCSpeaker *_speakerStream;
	Audio::SoundHandle *_speakerHandle;

private:
	const ADGameDescription *_gameDescription;
};

void init(void);

extern ChamberEngine *g_vm;

} // End of namespace Chamber

#endif
