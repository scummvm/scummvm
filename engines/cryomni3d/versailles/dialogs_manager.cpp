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

#include "audio/decoders/wave.h"
#include "common/file.h"
#include "common/system.h"

#include "cryomni3d/video/hnm_decoder.h"

#include "cryomni3d/versailles/dialogs_manager.h"
#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

Versailles_DialogsManager::Versailles_DialogsManager(CryOmni3DEngine_Versailles *engine,
        bool padAudioFileName) :
	_engine(engine), _padAudioFileName(padAudioFileName) {
}

bool Versailles_DialogsManager::play(const Common::String &sequence) {
	// Prepare with specific Versailles stuff
	if (!_engine->preprocessDialog(sequence)) {
		return false;
	}

	_engine->musicSetQuiet(true);

	_engine->setCursor(181);
	// No need to adjust hide cursor counter, there isn't any in ScummVM
	bool cursorWasVisible = _engine->showMouse(true);

	bool slowStop = false;
	bool didSth = DialogsManager::play(sequence, slowStop);

	_engine->showMouse(cursorWasVisible);

	if (didSth && slowStop) {
		if (_engine->showSubtitles()) {
			bool skip = false;
			uint end = g_system->getMillis() + 2000;
			while (!_engine->shouldAbort() && g_system->getMillis() < end && !skip) {
				g_system->updateScreen();
				g_system->delayMillis(10);
				if (_engine->pollEvents() &&
				        (_engine->checkKeysPressed(1, Common::KEYCODE_SPACE) ||
				         _engine->getCurrentMouseButton() == 1)) {
					skip = true;
				}
			}
		}
	}
	_engine->postprocessDialog(sequence);

	_engine->musicSetQuiet(false);

	_lastImage.free();

	_engine->waitMouseRelease();
	return didSth;
}

void Versailles_DialogsManager::executeShow(const Common::String &show) {
	Common::HashMap<Common::String, ShowCallback>::iterator showIt = _shows.find(show);

	if (showIt == _shows.end()) {
		error("Missing show %s", show.c_str());
	}

	_lastImage.free();

	ShowCallback cb = showIt->_value;
	(_engine->*cb)();
}

void Versailles_DialogsManager::playDialog(const Common::String &video, const Common::String &sound,
        const Common::String &text, const SubtitlesSettings &settings) {
	// Don't look for HNS file here
	Common::String videoFName(_engine->prepareFileName(video, "hnm"));
	Common::String soundFName(sound);

	if (_padAudioFileName) {
		while (soundFName.size() < 8) {
			soundFName += '_';
		}
	}
	soundFName = _engine->prepareFileName(soundFName, "wav");

	Video::HNMDecoder *videoDecoder = new Video::HNMDecoder(true);

	if (!videoDecoder->loadFile(videoFName)) {
		warning("Failed to open movie file %s/%s", video.c_str(), videoFName.c_str());
		delete videoDecoder;
		return;
	}

	Common::File *audioFile = new Common::File();
	if (!audioFile->open(soundFName)) {
		warning("Failed to open sound file %s/%s", sound.c_str(), soundFName.c_str());
		delete videoDecoder;
		delete audioFile;
		return;
	}

	Audio::SeekableAudioStream *audioDecoder = Audio::makeWAVStream(audioFile, DisposeAfterUse::YES);
	// We lost ownership of the audioFile just set it to nullptr and don't use it
	audioFile = nullptr;

	if (!audioDecoder) {
		delete videoDecoder;
		return;
	}

	_engine->showMouse(false);

	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();

	// Preload first frame to draw subtitles from it
	const Graphics::Surface *firstFrame = videoDecoder->decodeNextFrame();
	assert(firstFrame != nullptr);

	if (videoDecoder->hasDirtyPalette()) {
		const byte *palette = videoDecoder->getPalette();
		_engine->setupPalette(palette, 0, 256);
	}

	FontManager &fontManager = _engine->_fontManager;
	_lastImage.create(firstFrame->w, firstFrame->h, firstFrame->format);
	_lastImage.blitFrom(*firstFrame);

	fontManager.setCurrentFont(7);
	fontManager.setTransparentBackground(true);
	fontManager.setForeColor(241);
	fontManager.setLineHeight(22);
	fontManager.setSpaceWidth(2);
	fontManager.setCharSpacing(1);

	if (_engine->showSubtitles()) {
		Common::Rect block = settings.textRect;

		uint lines = fontManager.getLinesCount(text, block.width() - 8);
		if (lines == 0) {
			lines = 5;
		}
		uint blockHeight = fontManager.lineHeight() * lines + 6;
		block.setHeight(blockHeight);

		if (block.bottom >= 480) {
			block.bottom = 470;
			warning("Dialog text is really too long");
		}

		// Make only the block area translucent inplace
		Graphics::Surface blockSurface = _lastImage.getSubArea(block);
		_engine->makeTranslucent(blockSurface, blockSurface);

		fontManager.setSurface(&_lastImage);
		block.grow(-4);
		fontManager.setupBlock(block);
		fontManager.displayBlockText(text);
	}

	g_system->copyRectToScreen(_lastImage.getPixels(), _lastImage.pitch, 0, 0, width, height);
	g_system->updateScreen();

	const Common::Rect &drawRect = settings.drawRect;

	if (audioDecoder->getLength() == 0) {
		// Empty wave file
		delete audioDecoder;

		uint duration = 100 * text.size();
		if (duration < 1000) {
			duration = 1000;
		}

		bool skipWait = false;
		uint end = g_system->getMillis() + duration;
		while (!_engine->shouldAbort() && g_system->getMillis() < end && !skipWait) {
			g_system->updateScreen();
			g_system->delayMillis(10);
			if (_engine->pollEvents() && _engine->checkKeysPressed(1, Common::KEYCODE_SPACE)) {
				skipWait = true;
			}
		}
	} else {
		// Let start the show!
		videoDecoder->start();

		Audio::SoundHandle audioHandle;
		_engine->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &audioHandle, audioDecoder);
		// We lost ownership of the audioDecoder just set it to nullptr and don't use it
		audioDecoder = nullptr;

		bool skipVideo = false;
		while (!_engine->shouldAbort() && _engine->_mixer->isSoundHandleActive(audioHandle) && !skipVideo) {
			if (_engine->pollEvents() && _engine->checkKeysPressed(1, Common::KEYCODE_SPACE)) {
				skipVideo = true;
			}

			if (videoDecoder->needsUpdate()) {
				const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

				if (frame) {
					if (videoDecoder->hasDirtyPalette()) {
						const byte *palette = videoDecoder->getPalette();
						_engine->setupPalette(palette, 0, 256);
					}

					// Only refresh the moving part of the animation
					const Graphics::Surface subFrame = frame->getSubArea(drawRect);
					g_system->copyRectToScreen(subFrame.getPixels(), subFrame.pitch, drawRect.left, drawRect.top,
					                           subFrame.w, subFrame.h);
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
		_engine->_mixer->stopHandle(audioHandle);
	}

	// It's intentional that _lastImage is set with the first video image

	delete videoDecoder;
	_engine->showMouse(true);
}

void Versailles_DialogsManager::displayMessage(const Common::String &text) {
	_engine->displayMessageBoxWarp(text);
}

uint Versailles_DialogsManager::askPlayerQuestions(const Common::String &video,
        const Common::StringArray &questions) {
	if (_lastImage.empty()) {
		loadFrame(video);
	}

	if (questions.size() == 0 || questions.size() > 5) {
		return uint(-1);
	}

	FontManager &fontManager = _engine->_fontManager;
	fontManager.setCurrentFont(7);
	fontManager.setTransparentBackground(true);
	fontManager.setLineHeight(18);
	fontManager.setSpaceWidth(2);
	fontManager.setSurface(&_lastImage);

	int16 tops[5];
	int16 bottoms[5];
	int16 currentHeight = 0;
	uint questionId = 0;
	for (Common::StringArray::const_iterator it = questions.begin(); it != questions.end();
	        it++, questionId++) {
		tops[questionId] = currentHeight;
		uint lines = fontManager.getLinesCount(*it, 598);
		if (lines == 0) {
			lines = 1;
		}
		currentHeight += 18 * lines;
		bottoms[questionId] = currentHeight;
	}

	int offsetY = 480 - (bottoms[questions.size() - 1] - tops[0]);
	if (offsetY > 402) {
		offsetY = 402;
	} else if (offsetY < 2) {
		offsetY = 2;
	}

	for (questionId = 0; questionId < questions.size(); questionId++) {
		tops[questionId] += offsetY;
		bottoms[questionId] += offsetY;
	}

	_engine->setCursor(181);
	Graphics::Surface alphaSurface = _lastImage.getSubArea(Common::Rect(0, offsetY - 2, 640, 480));
	_engine->makeTranslucent(alphaSurface, alphaSurface);

	bool finished = false;
	bool update = true;
	uint selectedQuestion = uint(-1);
	while (!finished) {
		if (update) {
			update = false;
			questionId = 0;
			for (Common::StringArray::const_iterator it = questions.begin(); it != questions.end();
			        it++, questionId++) {
				fontManager.setForeColor(selectedQuestion == questionId ? 241 : 245);
				fontManager.setupBlock(Common::Rect(10, tops[questionId], 608, bottoms[questionId]));
				fontManager.displayBlockText(*it);
			}
			g_system->copyRectToScreen(_lastImage.getPixels(), _lastImage.pitch, 0, 0, _lastImage.w,
			                           _lastImage.h);
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (_engine->pollEvents()) {
			_engine->clearKeys();
			if (_engine->shouldAbort()) {
				finished = true;
				selectedQuestion = uint(-1);
				break;
			}
			Common::Point mousePos = _engine->getMousePos();
			if (_engine->getDragStatus() == kDragStatus_Finished && selectedQuestion != uint(-1)) {
				finished = true;
			} else if (mousePos.x >= 608 || mousePos.y < offsetY) {
				if (selectedQuestion != uint(-1)) {
					selectedQuestion = uint(-1);
					update = true;
				}
			} else {
				for (questionId = 0; questionId < questions.size(); questionId++) {
					if (mousePos.y > tops[questionId] && mousePos.y < bottoms[questionId]) {
						break;
					}
				}
				if (questionId < questions.size()) {
					if (selectedQuestion != questionId) {
						selectedQuestion = questionId;
						update = true;
					}
				} else {
					selectedQuestion = uint(-1);
					update = true;
				}
			}
		}
	}

	return selectedQuestion;
}

void Versailles_DialogsManager::loadFrame(const Common::String &video) {
	Common::String videoFName(_engine->prepareFileName(video, "hnm"));

	Video::HNMDecoder *videoDecoder = new Video::HNMDecoder();

	if (!videoDecoder->loadFile(videoFName)) {
		warning("Failed to open movie file %s/%s", video.c_str(), videoFName.c_str());
		delete videoDecoder;
		return;
	}

	// Preload first frame to draw questions on it
	const Graphics::Surface *firstFrame = videoDecoder->decodeNextFrame();
	_lastImage.create(firstFrame->w, firstFrame->h, firstFrame->format);
	_lastImage.blitFrom(*firstFrame);

	if (videoDecoder->hasDirtyPalette()) {
		const byte *palette = videoDecoder->getPalette();
		_engine->setupPalette(palette, 0, 256);
	}
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
