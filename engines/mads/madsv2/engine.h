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

#ifndef MADSV2_ENGINE_H
#define MADSV2_ENGINE_H

#include "audio/mixer.h"
#include "common/events.h"
#include "common/serializer.h"
#include "common/stack.h"
#include "common/random.h"
#include "graphics/screen.h"
#include "mads/mads.h"
#include "mads/core/sound.h"
#include "mads/madsv2/core/speech.h"

namespace MADS {
namespace MADSV2 {

class MADSV2Engine : public MADSEngine {
private:
	void syncGame(Common::Serializer &s);

protected:
	Graphics::Screen *_screen = nullptr;
	Common::Stack<Common::KeyState> _keyEvents;
	uint32 _nextFrameTime = 0;
	Common::Point _mousePos;
	int _mouseButtons = 0;
	Audio::SoundHandle _speechHandle;

	void pollEvents();

public:
	MADS::SoundManager *_soundManager = nullptr;
	bool _musicFlag = true;
	bool _soundFlag = true;
	bool &_speechFlag = speech_on;

public:
	MADSV2Engine(OSystem *syst, const MADSGameDescription *gameDesc);
	~MADSV2Engine() override;
	void readConfigFile();

	Graphics::Screen *getScreen() const {
		return _screen;
	}

	bool hasPendingKey();
	int getKey();
	void flushKeys();

	int getMouseState(int &x, int &y);

	/**
	 * Get the elapsed time in milliseconds
	 */
	uint32 getMillis();

	/* Callback routines in game-specific MAIN module */
	int main_cheating_key(int mykey) const {
		return mykey;
	}
	int main_normal_key(int mykey) const {
		return mykey;
	}
	int  main_copy_verify() {
		return 0;
	}

	bool canLoadGameStateCurrently(Common::U32String *msg) override;
	bool canSaveGameStateCurrently(Common::U32String *msg) override {
		return canLoadGameStateCurrently(msg);
	}
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	SaveStateList listSaves() const;

	virtual void global_init_code() = 0;
	virtual void section_music(int section_num) = 0;
	virtual void global_section_constructor() = 0;
	virtual void global_daemon_code() = 0;
	virtual void global_pre_parser_code() = 0;
	virtual void global_parser_code() = 0;
	virtual void global_error_code() = 0;
	virtual void global_room_init() = 0;
	virtual void global_sound_driver() = 0;
	virtual void global_verb_filter() {}

	void playSpeech(Audio::AudioStream *stream);
	void stopSpeech();
};

extern MADSV2Engine *g_engine;

} // namespace MADSV2
} // namespace MADS

#endif
