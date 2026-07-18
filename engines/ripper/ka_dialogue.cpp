/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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
 */

#include "ripper/ka_dialogue.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "image/pcx.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/script.h"

namespace Ripper {

namespace {

static const char *const kDeckMedia = "ka_deck.avi";
static const char *const kLoopMedia = "ka_loop.smk";
static const char *const kCardMedia = "ka_card.avi";
static const char *const kBookMedia = "ka_book.avi";
static const char *const kCdMedia = "ka_cd.avi";
static const char *const kCdGateScene = "SB2_1_D";
static const char *const kCodeBackground = "ka_puz.pcx";
static const char *const kCodeKeyAudio = "ka_key.wav";
static const char *const kCodeSuccessAudio = "li1_1_ve.wav";
static const char *const kCodeFailureAudio = "li1_1_vf.wav";
static const char *const kFirstGreetingAudio = "li1_1_v1.wav";
static const char *const kRepeatGreetingAudio[2] = {
	"li1_1_z1.wav", "li1_1_z3.wav"
};
static const char *const kLoopCueAudio[3] = {
	"library2.wav", "library1.wav", "library0.wav"
};
static const uint kLoopCueFrames[3] = { 1, 28, 45 };
static const uint kLoopCueVolumes[3] = { 25, 20, 10 };

static const uint kDefaultCursor = 14;
static const uint kStartCursor = 11;
static const uint kCardCursor = 6;
static const uint kChoiceCursor = 16;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kEnterCommand = 0x0d;
static const uint16 kUpCommand = 0x4800;
static const uint16 kDownCommand = 0x5000;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kStartCommand = 0x672;
static const uint16 kCardCommand = 0x673;
static const uint16 kChoiceCommandBase = 0x7000;
static const uint16 kFailureCommand = 0x7ffd;
static const uint kHelpSelectionTable = 0x1a6;
static const uint kLoopStartFrame = 1;
static const byte kChooserBackgroundColor = 0;
static const byte kChooserSelectionColor = 248;
static const byte kChooserBorderColor = 248;
static const byte kChooserTextColor = 4;
static const byte kChooserHighlightColor = 254;

static const uint kCardSeenFlag = 0xcc;
static const uint kFirstChoiceFlag = 0x14a;
static const uint kSecondChoiceFlag = 0x14b;
static const uint kBookChoiceFlag = 0x14c;
static const uint kCdChoiceFlag = 0x14d;
static const uint kBookSolvedFlag = 0xe1;
static const uint kBookFollowupFlag = 0x53;
static const uint kCdFollowupFlag = 0x54;
static const uint kActOneCompleteFlag = 2;
static const uint kFirstChoiceText = 0xaa;

static const int kStartLeft = 294;
static const int kStartTop = 103;
static const int kStartRight = 418;
static const int kStartBottom = 147;
static const int kCardLeft = 413;
static const int kCardTop = 213;
static const int kCardRight = 467;
static const int kCardBottom = 303;
static const int kChoiceLeft = 300;
static const int kChoiceTop = 72;
static const int kChoiceRight = 626;
static const int kChoiceBottom = 224;
static const int kChoiceInset = 8;
static const int kChoiceRowHeight = 34;
static const int kCodeBackgroundY = 50;
static const int kCodeTextY = 310;
static const int kCodeCellX[7] = { 64, 82, 101, 119, 138, 156, 175 };
static const char kExpectedBookCode[] = "HC2021R";

} // End of anonymous namespace

KaDialogueScene::KaDialogueScene(RipperEngine *engine) : _engine(engine),
		_random("ripper-ka-dialogue"), _sceneArgument(0), _selectedChoice(0),
		_hoveredControl(-1), _conversationStarted(false), _voicePending(false),
		_acceptInput(false) {
}

bool KaDialogueScene::initialize() {
	if (!_engine->getResources()->loadInterfaceBitmapFont("small.fnt", _font) ||
			!_engine->getResources()->loadGameText(_gameText)) {
		warning("Ripper: could not load Ka dialogue font or game text");
		return false;
	}
	if (_gameText.size() <= kFirstChoiceText + 3) {
		warning("Ripper: Ka dialogue text table has only %u entries", _gameText.size());
		return false;
	}
	debugC(1, kDebugDialogue,
		"Ripper: initialized Ka dialogue textChoices=4 fontGlyphs=%u",
		_font.glyphs.size());
	return true;
}

bool KaDialogueScene::loadCodeBackground() {
	if (!_codeBackground.pixels.empty())
		return true;
	Common::File file;
	Image::PCXDecoder decoder;
	if (!file.open(Common::Path(kCodeBackground)) || !decoder.loadStream(file)) {
		warning("Ripper: could not decode Ka book-code background '%s'", kCodeBackground);
		return false;
	}
	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || surface->format.bytesPerPixel != 1 || surface->w <= 0 || surface->h <= 0)
		return false;
	_codeBackground.width = surface->w;
	_codeBackground.height = surface->h;
	_codeBackground.pixels.resize((uint32)surface->w * surface->h);
	for (int y = 0; y < surface->h; ++y)
		memcpy(_codeBackground.pixels.data() + y * surface->w,
			surface->getBasePtr(0, y), surface->w);
	const Graphics::Palette &palette = decoder.getPalette();
	_codeBackground.palette.resize(palette.size() * 3);
	if (!_codeBackground.palette.empty())
		memcpy(_codeBackground.palette.data(), palette.data(), _codeBackground.palette.size());
	debugC(2, kDebugDialogue,
		"Ripper: loaded Ka book-code background media='%s' size=%ux%u colors=%u",
		kCodeBackground, _codeBackground.width, _codeBackground.height, palette.size());
	return true;
}

uint KaDialogueScene::measureText(const Common::String &text) const {
	uint width = 0;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			width += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
				character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		width += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
	return width;
}

void KaDialogueScene::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	int drawX = x;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			drawX += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
				character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		for (uint glyphY = 0; glyphY < glyph.height; ++glyphY) {
			for (uint glyphX = 0; glyphX < glyph.width; ++glyphX) {
				const byte pixel = _font.pixels[glyph.pixelOffset + glyphY * glyph.width + glyphX];
				const int targetX = drawX + glyph.xOffset + glyphX;
				const int targetY = y + glyph.yOffset + glyphY;
				if (pixel != _font.transparentColor && targetX >= 0 && targetX < 640 &&
						targetY >= 0 && targetY < 400)
					screen[targetY * pitch + targetX] = color;
			}
		}
		drawX += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
}

void KaDialogueScene::drawChoiceOverlay() {
	if (_choices.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	const int bottom = MIN<int>(kChoiceTop + kChoiceInset * 2 +
		_choices.size() * kChoiceRowHeight, kChoiceBottom);
	for (int y = kChoiceTop; y < bottom; ++y) {
		for (int x = kChoiceLeft; x < kChoiceRight; ++x) {
			const bool border = x == kChoiceLeft || x == kChoiceRight - 1 ||
				y == kChoiceTop || y == bottom - 1;
			pixels[y * screen->pitch + x] = border ? kChooserBorderColor :
				kChooserBackgroundColor;
		}
	}
	for (uint i = 0; i < _choices.size(); ++i) {
		const Choice &choice = _choices[i];
		const int textY = kChoiceTop + kChoiceInset + i * kChoiceRowHeight;
		const bool highlighted = (int)i + 2 == _hoveredControl || i == _selectedChoice;
		if (highlighted) {
			const int rowTop = textY - 2;
			const int rowBottom = MIN<int>(rowTop + kChoiceRowHeight, bottom - 1);
			for (int y = rowTop; y < rowBottom; ++y)
				memset(pixels + y * screen->pitch + kChoiceLeft + 1,
					kChooserSelectionColor, kChoiceRight - kChoiceLeft - 2);
		}
		drawText(pixels, screen->pitch, kChoiceLeft + kChoiceInset, textY,
			_gameText[choice.textResource], highlighted ? kChooserHighlightColor :
			kChooserTextColor);
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

void KaDialogueScene::drawCodePrompt(const Common::String &typedCode) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint y = 0; y < _codeBackground.height && y + kCodeBackgroundY < 400; ++y) {
		const uint width = MIN<uint>(_codeBackground.width, 640);
		memcpy(screen->getBasePtr(0, y + kCodeBackgroundY),
			_codeBackground.pixels.data() + y * _codeBackground.width, width);
	}
	for (uint i = 0; i < typedCode.size() && i < ARRAYSIZE(kCodeCellX); ++i) {
		const Common::String character(typedCode.c_str() + i, 1);
		const int x = kCodeCellX[i] + (15 - (int)measureText(character)) / 2;
		drawText(pixels, screen->pitch, x, kCodeTextY, character,
			kChooserHighlightColor);
	}
	g_system->unlockScreen();
	if (_codeBackground.palette.size() >= 256 * 3)
		g_system->getPaletteManager()->setPalette(_codeBackground.palette.data(), 0, 256);
	g_system->updateScreen();
}

void KaDialogueScene::rebuildChoices() {
	// RunKaDialogueScene at 0x2aef5 builds this list from GAMETEXT.TF entries
	// 0xaa..0xad and the original 0x14a..0x14d progress bits.
	_choices.clear();
	Milestones *milestones = _engine->getMilestones();
	static const Choice choices[4] = {
		{ kFirstChoiceFlag, kFirstChoiceText, "li1_1_va.wav" },
		{ kSecondChoiceFlag, kFirstChoiceText + 1, "li1_1_vb.wav" },
		{ kBookChoiceFlag, kFirstChoiceText + 2, "li1_1_vd.wav" },
		{ kCdChoiceFlag, kFirstChoiceText + 3, "li1_1_vc.wav" }
	};
	if (!milestones->isSet(choices[0].flag))
		_choices.push_back(choices[0]);
	if (!milestones->isSet(choices[1].flag))
		_choices.push_back(choices[1]);
	if (milestones->isSet(kCardSeenFlag) && !milestones->isSet(kBookSolvedFlag))
		_choices.push_back(choices[2]);
	if (milestones->isSet(kActOneCompleteFlag) && !milestones->isSet(kCdChoiceFlag) &&
			_engine->getScripts()->hasPlayedScene(kCdGateScene))
		_choices.push_back(choices[3]);
	if (_selectedChoice >= _choices.size())
		_selectedChoice = _choices.empty() ? 0 : _choices.size() - 1;
	debugC(2, kDebugDialogue,
		"Ripper: rebuilt Ka dialogue choices count=%u conversationStarted=%d flags=cc:%d e1:%d 54:%d",
		_choices.size(), _conversationStarted, milestones->isSet(kCardSeenFlag),
		milestones->isSet(kBookSolvedFlag), milestones->isSet(kCdFollowupFlag));
}

bool KaDialogueScene::startVoice(const char *path, const char *source) {
	_engine->getMedia()->stopSoundEffect(_voiceHandle);
	if (!_engine->getMedia()->playSoundEffect(path, _voiceHandle)) {
		warning("Ripper: could not start Ka dialogue voice '%s'", path);
		return false;
	}
	_voicePending = true;
	_choices.clear();
	_hoveredControl = -1;
	debugC(2, kDebugDialogue,
		"Ripper: started Ka dialogue voice media='%s' source=%s", path, source);
	return true;
}

bool KaDialogueScene::runBookCodePrompt() {
	if (!loadCodeBackground())
		return false;
	Common::String typedCode;
	_engine->getCursor()->setVisible(false);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	drawCodePrompt(typedCode);
	debugC(1, kDebugDialogue,
		"Ripper: entered Ka book-code prompt expectedLength=7 helpTable=0x%x",
		kHelpSelectionTable);
	bool matched = false;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				_engine->getModalDialog()->run(kHelpSelectionTable);
				drawCodePrompt(typedCode);
				continue;
			}
			if (command == 8 && !typedCode.empty()) {
				_engine->getMedia()->playSoundEffect(kCodeKeyAudio, _keyHandle);
				typedCode.deleteLastChar();
				drawCodePrompt(typedCode);
				continue;
			}
			if (command >= 'a' && command <= 'z')
				command -= 'a' - 'A';
			if (typedCode.size() >= 7 || !((command >= '0' && command <= '9') ||
					(command >= 'A' && command <= 'Z')))
				continue;
			_engine->getMedia()->playSoundEffect(kCodeKeyAudio, _keyHandle);
			typedCode += (char)command;
			drawCodePrompt(typedCode);
			if (typedCode == kExpectedBookCode) {
				matched = true;
				active = false;
				break;
			}
		}
		g_system->delayMillis(10);
	}
	_engine->getMedia()->stopSoundEffect(_keyHandle);
	const char *resultAudio = matched ? kCodeSuccessAudio : kCodeFailureAudio;
	if (!_engine->shouldQuit() && !_engine->getMedia()->play(resultAudio, true))
		warning("Ripper: could not play Ka book-code result audio '%s'", resultAudio);
	debugC(1, kDebugDialogue,
		"Ripper: left Ka book-code prompt matched=%d typedLength=%u",
		matched, typedCode.size());
	return matched;
}

bool KaDialogueScene::serviceVoiceCompletion() {
	Milestones *milestones = _engine->getMilestones();
	if (milestones->isSet(kBookChoiceFlag) && !milestones->isSet(kBookSolvedFlag)) {
		const bool solved = runBookCodePrompt();
		if (solved) {
			if (!_engine->getMedia()->play(kBookMedia, true))
				return false;
			milestones->set(kBookSolvedFlag, true, "ka-dialogue-book-code");
			milestones->set(kBookFollowupFlag, true, "ka-dialogue-book-code");
		} else {
			milestones->set(kBookChoiceFlag, false, "ka-dialogue-book-code");
		}
	}
	if (milestones->isSet(kCdChoiceFlag) && !milestones->isSet(kCdFollowupFlag)) {
		if (!_engine->getMedia()->play(kCdMedia, true))
			return false;
		milestones->set(kCdFollowupFlag, true, "ka-dialogue-cd");
	}
	rebuildChoices();
	return true;
}

void KaDialogueScene::serviceLoopAudio(uint frame) {
	// The original Ka loop queues descriptors 2, 1, and 0 at frames 1, 28,
	// and 45, using the packed 25%, 20%, and 10% volumes initialized at entry.
	for (uint i = 0; i < ARRAYSIZE(kLoopCueFrames); ++i) {
		if (frame != kLoopCueFrames[i])
			continue;
		_engine->getMedia()->playSoundEffect(kLoopCueAudio[i], _loopCueHandles[i],
			kLoopCueVolumes[i]);
		debugC(3, kDebugDialogue,
			"Ripper: queued Ka loop cue media='%s' frame=%u volume=%u",
			kLoopCueAudio[i], frame, kLoopCueVolumes[i]);
	}
}

void KaDialogueScene::updateCursor(const Common::Point &point) {
	int hovered = -1;
	uint cursor = kDefaultCursor;
	const bool startEnabled = !_voicePending &&
		(!_conversationStarted || _choices.empty());
	const bool cardEnabled = !_voicePending &&
		!_engine->getMilestones()->isSet(kCardSeenFlag);
	if (!_choices.empty() && point.x >= kChoiceLeft && point.x < kChoiceRight &&
			point.y >= kChoiceTop && point.y < kChoiceBottom) {
		const int row = (point.y - kChoiceTop - kChoiceInset) / kChoiceRowHeight;
		if (row >= 0 && row < (int)_choices.size()) {
			hovered = row + 2;
			cursor = kChoiceCursor;
			_selectedChoice = row;
		}
	} else if (startEnabled && point.x >= kStartLeft && point.x < kStartRight &&
			point.y >= kStartTop && point.y < kStartBottom) {
		hovered = 0;
		cursor = kStartCursor;
	} else if (cardEnabled && point.x >= kCardLeft && point.x < kCardRight &&
			point.y >= kCardTop && point.y < kCardBottom) {
		hovered = 1;
		cursor = kCardCursor;
	}
	if (hovered != _hoveredControl) {
		debugC(2, kDebugDialogue,
			"Ripper: Ka dialogue hover control=%d previous=%d point=%d,%d cursor=%u",
			hovered, _hoveredControl, point.x, point.y, cursor);
		_hoveredControl = hovered;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

uint16 KaDialogueScene::serviceInput() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand) {
			if (_voicePending) {
				_engine->getMedia()->stopSoundEffect(_voiceHandle);
				_voicePending = false;
				rebuildChoices();
				debugC(2, kDebugDialogue,
					"Ripper: Escape stopped active Ka dialogue voice");
				continue;
			}
			return kEscapeCommand;
		}
		if (_choices.empty())
			continue;
		if (command == kUpCommand) {
			_selectedChoice = _selectedChoice == 0 ? _choices.size() - 1 : _selectedChoice - 1;
		} else if (command == kDownCommand) {
			_selectedChoice = (_selectedChoice + 1) % _choices.size();
		} else if (command == kEnterCommand) {
			return kChoiceCommandBase + _selectedChoice;
		}
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	updateCursor(mouse.position);
	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return 0;
	if (_hoveredControl == 0)
		return kStartCommand;
	if (_hoveredControl == 1)
		return kCardCommand;
	if (_hoveredControl >= 2)
		return kChoiceCommandBase + _hoveredControl - 2;
	return 0;
}

uint16 KaDialogueScene::service(uint frame) {
	serviceLoopAudio(frame);
	if (_voicePending && !_engine->getMedia()->isSoundEffectActive(_voiceHandle)) {
		_voicePending = false;
		debugC(2, kDebugDialogue,
			"Ripper: Ka dialogue voice completed frame=%u", frame);
		if (!serviceVoiceCompletion())
			return kFailureCommand;
		updateCursor(_engine->getInput()->peekMouseState().position);
		drawChoiceOverlay();
		return MediaSequenceCallback::kContinueRefreshPalette;
	}
	if (!_acceptInput)
		return 0;
	uint16 command = serviceInput();
	if (command == kStartCommand) {
		const char *voice = !_conversationStarted ? kFirstGreetingAudio :
			kRepeatGreetingAudio[_random.getRandomNumber(1)];
		_conversationStarted = true;
		if (!startVoice(voice, "start-control"))
			return kFailureCommand;
		command = 0;
	} else if (command == kCardCommand) {
		_choices.clear();
		if (!_engine->getMedia()->play(kCardMedia, true) ||
				!_engine->getMilestones()->set(kCardSeenFlag, true, "ka-dialogue-card"))
			return kFailureCommand;
		if (_conversationStarted)
			rebuildChoices();
		updateCursor(_engine->getInput()->peekMouseState().position);
		drawChoiceOverlay();
		return MediaSequenceCallback::kContinueRefreshPalette;
	} else if (command >= kChoiceCommandBase &&
			command < kChoiceCommandBase + _choices.size()) {
		const uint choiceIndex = command - kChoiceCommandBase;
		const Choice choice = _choices[choiceIndex];
		if (!_engine->getMilestones()->set(choice.flag, true, "ka-dialogue-choice") ||
				!startVoice(choice.audioPath, "choice"))
			return kFailureCommand;
		debugC(2, kDebugDialogue,
			"Ripper: selected Ka dialogue choice index=%u flag=0x%x textResource=0x%x audio='%s'",
			choiceIndex, choice.flag, choice.textResource, choice.audioPath);
		command = 0;
	}
	drawChoiceOverlay();
	if (command != 0) {
		debugC(2, kDebugDialogue,
			"Ripper: Ka dialogue input command=0x%04x frame=%u choices=%u voicePending=%d",
			command, frame, _choices.size(), _voicePending);
	}
	return command;
}

void KaDialogueScene::stopAllAudio() {
	_engine->getMedia()->stopSoundEffect(_ambientHandle);
	_engine->getMedia()->stopSoundEffect(_deckCueHandle);
	for (uint i = 0; i < ARRAYSIZE(_loopCueHandles); ++i)
		_engine->getMedia()->stopSoundEffect(_loopCueHandles[i]);
	_engine->getMedia()->stopSoundEffect(_voiceHandle);
	_engine->getMedia()->stopSoundEffect(_keyHandle);
}

KaDialogueScene::Result KaDialogueScene::run(uint sceneArgument) {
	_sceneArgument = sceneArgument;
	if (!initialize())
		return kLoadFailed;
	debugC(1, kDebugDialogue,
		"Ripper: entered Ka dialogue scene argument=%u deck='%s' loop='%s' toolbarMask=0x0000",
		_sceneArgument, kDeckMedia, kLoopMedia);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(false);
	_engine->getMedia()->playSoundEffect("deck10.wav", _deckCueHandle);
	if (!_engine->getMedia()->play(kDeckMedia, false)) {
		stopAllAudio();
		return kLoadFailed;
	}
	// RunKaDialogueScene at 0x2aef5 issues display command 0x14 after
	// KA_DECK.AVI. ClearGenericVideoLogicalPage at 0x45ed8 zeroes the page so
	// KA_LOOP.SMK's 640x300 palette cannot recolor stale pixels in the bands.
	g_system->fillScreen(0);
	g_system->updateScreen();
	debugC(2, kDebugDialogue,
		"Ripper: cleared Ka deck display before loop media='%s' source=display-command-0x14",
		kLoopMedia);
	_engine->getMedia()->playSoundEffect("library3.wav", _ambientHandle, 100, true);
	_acceptInput = true;

	uint16 command = 0;
	const bool played = _engine->getMedia()->playPuzzleSequence(
		kLoopMedia, kLoopStartFrame, this, &command);
	const Result result = (!played && !_engine->shouldQuit()) || command == kFailureCommand ?
		kLoadFailed : kExited;

	stopAllAudio();
	_engine->getCursor()->setVisible(false);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kExited ? 1 : 2, kDebugDialogue,
		"Ripper: left Ka dialogue scene result=%d conversationStarted=%d choices=%u quit=%d",
		result, _conversationStarted, _choices.size(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
