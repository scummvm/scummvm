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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/menu.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

struct MenuControlBounds {
	int16 top;
	int16 left;
	int16 height;
	int16 width;
};

// RunStartupFrontEndLoop at 0x10778 reads this layout from the table at 0x1001b.
static const MenuControlBounds kMenuControlBounds[] = {
	{ 64, 248, 48, 366 },
	{ 122, 248, 48, 366 },
	{ 180, 286, 48, 328 },
	{ 238, 352, 48, 262 },
	{ 296, 512, 48, 102 }
};

static const uint kDefaultMenuCursor = 14;
static const uint kActiveMenuCursor = 16;

static bool playMenuAudio(RipperEngine *engine, const Common::String &memberName,
		Audio::Mixer::SoundType soundType, Audio::SoundHandle &handle, bool loop) {
	Common::SeekableReadStream *stream =
		engine->getResources()->sound().createReadStreamForMember(memberName);
	if (!stream)
		return false;

	Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!wavStream) {
		warning("Ripper: could not decode startup menu audio '%s'", memberName.c_str());
		return false;
	}

	Audio::Mixer *mixer = g_system->getMixer();
	mixer->stopHandle(handle);
	Audio::AudioStream *audioStream = wavStream;
	if (loop)
		audioStream = Audio::makeLoopingAudioStream(wavStream, 0);
	mixer->playStream(soundType, &handle, audioStream);
	debugC(2, kDebugAudio, "Ripper: started startup menu audio '%s' loop=%d",
		memberName.c_str(), loop);
	return true;
}

static int findMenuControl(const Common::Point &position, int yOffset) {
	for (uint i = 0; i < ARRAYSIZE(kMenuControlBounds); ++i) {
		const MenuControlBounds &bounds = kMenuControlBounds[i];
		if (position.x >= bounds.left && position.x < bounds.left + bounds.width &&
			position.y >= bounds.top + yOffset &&
			position.y < bounds.top + yOffset + bounds.height)
			return i;
	}
	return -1;
}

MainMenu::MainMenu(RipperEngine *engine) : _engine(engine) {
}

MainMenuAction MainMenu::run() {
	Common::SeekableReadStream *stream =
		_engine->getResources()->interface().createReadStreamForMember("rip_open.smk");
	if (!stream) {
		warning("Ripper: could not open startup menu presentation 'RIP_OPEN.SMK'");
		return kMainMenuQuit;
	}

	Video::SmackerDecoder decoder;
	if (!decoder.loadStream(stream)) {
		warning("Ripper: invalid startup menu presentation 'RIP_OPEN.SMK'");
		return kMainMenuQuit;
	}

	debugC(1, kDebugGeneral,
		"Ripper: entering startup menu frames=%u size=%ux%u actions=%u",
		decoder.getFrameCount(), decoder.getWidth(), decoder.getHeight(),
		ARRAYSIZE(kMenuControlBounds));
	const int menuY = (g_system->getHeight() - decoder.getHeight()) / 2;
	g_system->fillScreen(0);
	presentScreen();
	_engine->getInput()->drainKeys();
	_engine->getInput()->publishMouseState();
	_engine->getCursor()->update(kDefaultMenuCursor);

	Audio::SoundHandle titleAudioHandle;
	Audio::SoundHandle hoverAudioHandle;
	Audio::SoundHandle selectionAudioHandle;
	auto finishMenu = [&](MainMenuAction action) {
		g_system->getMixer()->stopHandle(titleAudioHandle);
		_engine->getCursor()->setVisible(false);
		// RIPPER.LE clears without presenting, so the next controlled AVI captures
		// a blank backing instead of the final RIP_OPEN.SMK frame.
		g_system->fillScreen(0);
		debugC(2, kDebugVideo,
			"Ripper: cleared startup menu logical page action=%d", action);
		return action;
	};
	playMenuAudio(_engine, "title0.wav", Audio::Mixer::kMusicSoundType,
		titleAudioHandle, true);

	int hoveredControl = -1;
	decoder.start();
	while (!_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			debugC(1, kDebugGeneral, "Ripper: quit requested from startup menu");
			_engine->quitGame();
			break;
		}

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b) {
				debugC(1, kDebugInput, "Ripper: Escape selected startup menu quit");
				return finishMenu(kMainMenuQuit);
			}
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		const int currentControl = findMenuControl(mouse.position, menuY);
		if (currentControl != hoveredControl) {
			hoveredControl = currentControl;
			debugC(2, kDebugInput, "Ripper: startup menu hover action=%d position=%d,%d",
				hoveredControl + 1, mouse.position.x, mouse.position.y);
			if (hoveredControl >= 0)
				playMenuAudio(_engine, "title1.wav", Audio::Mixer::kSFXSoundType,
					hoverAudioHandle, false);
		}
		_engine->getCursor()->update(hoveredControl >= 0 ? kActiveMenuCursor : kDefaultMenuCursor);

		if (hoveredControl >= 0 && (mouse.pressed & kMouseButtonLeft) != 0) {
			const MainMenuAction action = (MainMenuAction)(hoveredControl + 1);
			debugC(1, kDebugInput, "Ripper: startup menu selected action=%d", action);
			playMenuAudio(_engine, "title2.wav", Audio::Mixer::kSFXSoundType,
				selectionAudioHandle, false);
			return finishMenu(action);
		}

		if (decoder.endOfVideo()) {
			if (!decoder.rewind()) {
				warning("Ripper: could not rewind startup menu presentation");
				break;
			}
			debugC(2, kDebugVideo, "Ripper: rewound startup menu presentation");
		}

		if (decoder.needsUpdate()) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (frame) {
				if (decoder.hasDirtyPalette()) {
					byte palette[256 * 3];
					memcpy(palette, decoder.getPalette(), sizeof(palette));
					_engine->applySharedPalettePatch(palette, 256);
					_engine->getSettings()->applyVideoPalette(palette, 256);
					g_system->getPaletteManager()->setPalette(palette, 0, 256);
				}
				g_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, menuY,
					frame->w, frame->h);
				presentScreen();
				debugC(11, kDebugVideo, "Ripper: startup menu frame=%d", decoder.getCurFrame());
			}
		}

		g_system->delayMillis(MIN<uint32>(decoder.getTimeToNextFrame(), 10));
	}

	return finishMenu(kMainMenuQuit);
}

} // End of namespace Ripper
