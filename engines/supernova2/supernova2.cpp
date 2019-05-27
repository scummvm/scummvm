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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "graphics/thumbnail.h"
#include "gui/saveload.h"

#include "supernova2/supernova2.h"
#include "supernova2/state.h"


namespace Supernova2 {

Supernova2Engine::Supernova2Engine(OSystem *syst)
	: Engine(syst)
	, _console(nullptr)
	, _gm(nullptr)
	, _allowLoadGame(true)
	, _allowSaveGame(true)
	, _sleepAutoSave(nullptr)
	, _sleepAuoSaveVersion(-1)
	, _delay(33)
	, _textSpeed(1) {
	if (ConfMan.hasKey("textspeed"))
		_textSpeed = ConfMan.getInt("textspeed");

	DebugMan.addDebugChannel(1 , "general", "Supernova 2 general debug channel");
}

Supernova2Engine::~Supernova2Engine() {
	DebugMan.clearAllDebugChannels();

	delete _console;
	delete _gm;
	delete _sleepAutoSave;
}

Common::Error Supernova2Engine::run() {
	init();

	while (!shouldQuit()) {
		uint32 start = _system->getMillis();
		_console->onFrame();
		_system->updateScreen();
		int end = _delay - (_system->getMillis() - start);
		if (end > 0)
			_system->delayMillis(end);
	}

	return Common::kNoError;
}

void Supernova2Engine::init() {
	Graphics::ModeList modes;
	modes.push_back(Graphics::Mode(320, 200));
	modes.push_back(Graphics::Mode(640, 480));
	initGraphicsModes(modes);
	initGraphics(320, 200);

	Common::Error status = loadGameStrings();
	if (status.getCode() != Common::kNoError)
		error("Failed reading game strings");

	_gm = new GameManager(this);
	_console = new Console(this, _gm);
	setTotalPlayTime(0);
}

Common::Error Supernova2Engine::loadGameStrings() {
	Common::String cur_lang = ConfMan.get("language");
	Common::String string_id("TEXT");

	// Note: we don't print any warning or errors here if we cannot find the file
	// or the format is not as expected. We will get those warning when reading the
	// strings anyway (actually the engine will even refuse to start).
	Common::File f;
	if (!f.open(SUPERNOVA2_DAT)) {
		GUIErrorMessageFormat(_("Unable to locate the '%s' engine data file."), SUPERNOVA2_DAT);
		return Common::kReadingFailed;
	}

	// Validate the data file header
	char id[5], lang[5];
	id[4] = lang[4] = '\0';
	f.read(id, 3);
	if (strncmp(id, "MS2", 3) != 0) {
		GUIErrorMessageFormat(_("The '%s' engine data file is corrupt."), SUPERNOVA2_DAT);
		return Common::kReadingFailed;
	}

	int version = f.readByte();
	if (version != SUPERNOVA2_DAT_VERSION) {
		GUIErrorMessageFormat(
			_("Incorrect version of the '%s' engine data file found. Expected %d but got %d."),
			SUPERNOVA2_DAT, SUPERNOVA2_DAT_VERSION, version);
		return Common::kReadingFailed;
	}

	while (!f.eos()) {
		f.read(id, 4);
		f.read(lang, 4);
		uint32 size = f.readUint32LE();
		if (f.eos())
			break;
		if (string_id == id && cur_lang == lang) {
			while (size > 0) {
				Common::String s;
				char ch;
				while ((ch = (char)f.readByte()) != '\0')
					s += ch;
				_gameStrings.push_back(s);
				size -= s.size() + 1;
			}
			return Common::kNoError;
		} else
			f.skip(size);
	}

	Common::Language l = Common::parseLanguage(cur_lang);
	GUIErrorMessageFormat(_("Unable to locate the text for %s language in '%s' engine data file."), Common::getLanguageDescription(l), SUPERNOVA2_DAT);
	return Common::kReadingFailed;
}

const Common::String &Supernova2Engine::getGameString(int idx) const {
	if (idx < 0 || idx >= (int)_gameStrings.size())
		return _nullString;
	return _gameStrings[idx];
}

void Supernova2Engine::setGameString(int idx, const Common::String &string) {
	if (idx < 0)
		return;
	while ((int)_gameStrings.size() <= idx)
		_gameStrings.push_back(Common::String());
	_gameStrings[idx] = string;
}

bool Supernova2Engine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsRTL:
		return true;
	case kSupportsLoadingDuringRuntime:
		return true;
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	}
}

}
