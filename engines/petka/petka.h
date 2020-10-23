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

#ifndef PETKA_PETKA_H
#define PETKA_PETKA_H

#include "common/random.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/savefile.h"

#include "engines/engine.h"
#include "engines/savestate.h"

#include "gui/debugger.h"

#include "graphics/surface.h"

/*
 *  This is the namespace of the Petka engine.
 *
 *  Status of this engine: In Development
 *
 *  Games using this engine:
 *  - Red Comrades Demo
 *  - Red Comrades Save the Galaxy - Fully playable
 *  - Red Comrades 2: For the Great Justice - Fully playable
 */

struct ADGameDescription;

namespace Common {
class SeekableReadStream;
}

namespace Video {
class VideoDecoder;
}

namespace Petka {

class Console;
class BigDialogue;
class FileMgr;
class SoundMgr;
class QManager;
class QSystem;
class VideoSystem;

enum {
	kPetkaDebugGeneral = 1 << 0,
	kPetkaDebugResources = 1 << 1,
	kPetkaDebugMessagingSystem = 1 << 2,
	kPetkaDebugDialogs = 1 << 3
};

class PetkaEngine : public Engine {
public:
	PetkaEngine(OSystem *syst, const ADGameDescription *desc);
	~PetkaEngine() override;

	bool isDemo() const;
	bool isPetka2() const;

	void loadPart(byte part);
	void loadPartAtNextFrame(byte part);

	byte getPart();
	int getSaveSlot();

	void loadChapter(byte chapter);

	virtual Common::Error run() override;

	bool hasFeature(EngineFeature f) const override;

	Common::SeekableReadStream *openFile(const Common::String &name, bool addCurrentPath);

	void playVideo(Common::SeekableReadStream *stream);
	QSystem *getQSystem() const;
	BigDialogue *getBigDialogue() const;
	SoundMgr *soundMgr() const;
	QManager *resMgr() const;

	VideoSystem *videoSystem() const;

	Common::RandomSource &getRnd();
	const Common::String &getSpeechPath();

	virtual Common::Error loadGameState(int slot) override;
	bool canLoadGameStateCurrently() override;

	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) override;
	bool canSaveGameStateCurrently() override;

	const ADGameDescription *const _desc;
	Common::ScopedPtr<Common::MemoryReadStream> _thumbnail;

protected:
	void pauseEngineIntern(bool pause) override;

private:
	void loadStores();

private:
	Common::ScopedPtr<Console> _console;
	Common::ScopedPtr<FileMgr> _fileMgr;
	Common::ScopedPtr<QManager> _resMgr;
	Common::ScopedPtr<SoundMgr> _soundMgr;
	Common::ScopedPtr<QSystem> _qsystem;
	Common::ScopedPtr<VideoSystem> _vsys;
	Common::ScopedPtr<BigDialogue> _dialogMan;
	Common::ScopedPtr<Video::VideoDecoder> _videoDec;

	Common::RandomSource _rnd;

	Common::String _currentPath;
	Common::String _speechPath;

	Common::String _chapterStoreName;

	uint8 _part;
	uint8 _nextPart;
	uint8 _chapter;
	bool _shouldChangePart;
	int _saveSlot;
};

class Console : public GUI::Debugger {
public:
	Console(PetkaEngine *vm) {}
	virtual ~Console() {}
};

extern PetkaEngine *g_vm;

WARN_UNUSED_RESULT bool readSaveHeader(Common::InSaveFile &in, SaveStateDescriptor &desc, bool skipThumbnail = true);
Common::String generateSaveName(int slot, const char *gameId);

} // End of namespace Petka

#endif
