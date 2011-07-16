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

#ifndef COMPOSER_H
#define COMPOSER_H

#include "common/config-file.h"
#include "common/random.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/rect.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "graphics/surface.h"

#include "audio/mixer.h"

#include "composer/resource.h"

namespace Audio {
	class QueuingAudioStream;
}

namespace Composer {

struct ComposerGameDescription;

enum GameType {
	GType_ComposerV1,
	GType_ComposerV2
};

class Archive;
class ComposerEngine;

struct Sprite {
	uint16 id;
	uint16 animId;
	uint16 zorder;
	Common::Point pos;
	Graphics::Surface surface;
};

struct AnimationEntry {
	uint32 dword0;
	uint16 op;
	uint16 word6;
	uint16 counter;
	uint16 word10;
};

struct Animation {
	Animation(Common::SeekableReadStream *stream, uint16 id, Common::Point basePos, uint32 eventParam);
	~Animation();

	void seekToCurrPos();

	uint16 _id;
	Common::Point _basePos;
	uint32 _eventParam;
	
	uint32 _state;

	Common::Array<AnimationEntry> _entries;

	uint32 _offset;
	Common::SeekableReadStream *_stream;
};

struct PipeResourceEntry {
	uint32 size;
	uint32 offset;
};

struct PipeResource {
	Common::Array<PipeResourceEntry> entries;
};

class Pipe {
public:
	Pipe(Common::SeekableReadStream *stream);
	void nextFrame();

	Animation *_anim;

        bool hasResource(uint32 tag, uint16 id) const;
	Common::SeekableReadStream *getResource(uint32 tag, uint16 id, bool buffering);

protected:
	Common::SeekableReadStream *_stream;

	typedef Common::HashMap<uint16, PipeResource> ResourceMap;
	typedef Common::HashMap<uint32, ResourceMap> TypeMap;
	TypeMap _types;

	uint32 _offset;
};

class Button {
public:
	Button(ComposerEngine *vm, uint16 id);
};

struct Library {
	uint _id;
	Archive *_archive;
};

struct QueuedScript {
	uint32 _baseTime;
	uint32 _duration;
	uint32 _count;
	uint16 _scriptId;
};

struct PendingPageChange {
	PendingPageChange() { }
	PendingPageChange(uint16 id, bool remove) : _pageId(id), _remove(remove) { }

	uint16 _pageId;
	bool _remove;
};

class ComposerEngine : public Engine {
protected:
	Common::Error run();

public:
	ComposerEngine(OSystem *syst, const ComposerGameDescription *gameDesc);
	virtual ~ComposerEngine();

	virtual bool hasFeature(EngineFeature f) const;

	int getGameType() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	const ComposerGameDescription *_gameDescription;

private:
	Common::RandomSource *_rnd;

	Audio::SoundHandle _soundHandle;
	Audio::QueuingAudioStream *_audioStream;

	Graphics::Surface _surface;
	Common::List<Sprite> _sprites;

	uint _directoriesToStrip;
	Common::ConfigFile _bookIni;
	Common::List<Library> _libraries;
	Common::Array<PendingPageChange> _pendingPageChanges;

	Common::Array<uint16> _stack;
	Common::Array<uint16> _vars;

	Common::Array<QueuedScript> _queuedScripts;
	Common::List<Animation *> _anims;
	Common::List<Pipe *> _pipes;

	Common::String getStringFromConfig(const Common::String &section, const Common::String &key);
	Common::String getFilename(const Common::String &section, uint id);
	void loadLibrary(uint id);
	void unloadLibrary(uint id);

	bool hasResource(uint32 tag, uint16 id);
	Common::SeekableReadStream *getResource(uint32 tag, uint16 id);

	void runEvent(uint16 id, int16 param1, int16 param2, int16 param3);
	int16 runScript(uint16 id, int16 param1, int16 param2, int16 param3);

	int16 getArg(uint16 arg, uint16 type);
	void setArg(uint16 arg, uint16 type, uint16 val);
	void runScript(uint16 id);
	int16 scriptFuncCall(uint16 id, int16 param1, int16 param2, int16 param3);

	void playAnimation(uint16 animId, int16 param1, int16 param2, int16 param3);
	void stopAnimation(Animation *anim, bool localOnly = false, bool pipesOnly = false);
	void playWaveForAnim(uint16 id, bool bufferingOnly);
	void processAnimFrame();

	void addSprite(uint16 id, uint16 animId, uint16 zorder, const Common::Point &pos);
	void removeSprite(uint16 id, uint16 animId);

	void loadCTBL(uint id, uint fadePercent);
	void decompressBitmap(uint16 type, Common::SeekableReadStream *stream, byte *buffer, uint32 size, uint width, uint height);
	bool initSprite(Sprite &sprite);
	void drawSprite(const Sprite &sprite);
};

} // End of namespace Composer

#endif
