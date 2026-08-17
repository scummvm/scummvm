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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/puzzles/shock_lever.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kLeverCount = 3;
static const uint kLeverFrameCount = 17;
static const uint kAudioCueCount = 5;
static const int kSceneOriginY = 50;
static const int kSceneHeight = 300;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kExitCursor = 7;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kSubmitCommand = 0x677;
static const uint16 kCheatCommand = 0x7ffe;
static const uint kHelpSelectionTable = 0x1af;
static const uint kDosTickMillis = 55;
static const uint kLeverStepTicks = 3;
static const byte kPuzzleHelpColor = 254;

static const char *const kLibraryName = "ef_monk.pl";
static const char *const kScreenMediaName = "screenn";
static const char *const kBackgroundName = "screen";
static const char *const kSubmitName = "down";
static const char kCheatKeyword[] = "sparky";

// g_shockLeverPuzzleLeverControlOrigins at 0x3ac07 stores the three
// screen-space X/Y pairs copied by RunShockLeverPuzzleScene at 0x3affb.
static const Common::Point kLeverPositions[kLeverCount] = {
	Common::Point(258, 96 + kSceneOriginY),
	Common::Point(305, 96 + kSceneOriginY),
	Common::Point(342, 100 + kSceneOriginY)
};

// g_shockLeverPuzzleTargetFrames at 0x3ac1f.
static const uint kTargetFrames[kLeverCount] = {7, 14, 9};

static const char *const kOutcomeMediaNames[4] = {
	"m_scream", "m_normal", "m_1", "m_2"
};

static const Common::Point kSubmitPosition(222, 252 + kSceneOriginY);
// ServiceShockPuzzleScreenPlayback at 0x3ad26 posts SCREENN's dirty region at
// logical x=0x114, y=3. The shared scene presentation adds the physical
// 50-pixel toolbar origin to y.
static const Common::Point kScreenMediaPosition(276, 3);

static bool isExitPoint(const Common::Point &point) {
	return point.y >= kSceneOriginY &&
		point.y < kSceneOriginY + kSceneHeight &&
		(point.x < 175 || point.x >= 460);
}

} // End of anonymous namespace

ShockLeverPuzzle::ShockLeverPuzzle(RipperEngine *engine) :
		Puzzle(engine), _draggedLever(-1), _hoveredLever(-1),
		_hoveredControl(kHoverNone), _keywordIndex(0),
		_lastLeverStepMillis(0), _puzzleHelpFontLoadAttempted(false),
		_puzzleHelpEnabled(false), _solved(false),
		_completionRecorded(false) {
	for (uint lever = 0; lever < kLeverCount; ++lever)
		_leverFrame[lever] = 3;
}

bool ShockLeverPuzzle::loadBitmap(AssetLibrary &library,
		const Common::String &name, BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		library.createReadStreamForMember(name));
	if (!stream || !decodeBitmapAsset(*stream, frame)) {
		warning("Ripper: could not decode shock lever bitmap '%s'", name.c_str());
		return false;
	}
	return true;
}

bool ShockLeverPuzzle::loadLever(uint lever) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_library.createReadStreamForMember(
			Common::String::format("lever%un", lever + 1)));
	AssetLibrary leverLibrary;
	if (!stream || !leverLibrary.open(*stream,
			Common::Path(Common::String::format("lever%un", lever + 1)))) {
		warning("Ripper: could not open shock lever bitmap library lever=%u",
			lever + 1);
		return false;
	}

	_leverFrames[lever].clear();
	for (uint frameIndex = 0; frameIndex < kLeverFrameCount; ++frameIndex) {
		BitmapAssetFrame frame;
		const Common::String member =
			Common::String::format("%u", frameIndex + 1);
		if (!loadBitmap(leverLibrary, member, frame))
			return false;
		_leverFrames[lever].push_back(Common::move(frame));
	}
	return true;
}

bool ShockLeverPuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(kLibraryName));
	if (!stream || !_library.open(*stream, Common::Path(kLibraryName))) {
		warning("Ripper: could not open shock lever library '%s'",
			kLibraryName);
		return false;
	}
	if (!loadBitmap(_library, kBackgroundName, _background) ||
			!loadBitmap(_library, kSubmitName, _submit))
		return false;
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		if (!loadLever(lever))
			return false;
	}
	if (!_library.hasMember(kScreenMediaName)) {
		warning("Ripper: shock lever screen media '%s' is missing",
			kScreenMediaName);
		return false;
	}
	for (uint cue = 0; cue < kAudioCueCount; ++cue) {
		const Common::String name = Common::String::format("shock%u", cue);
		if (!_library.hasMember(name)) {
			warning("Ripper: shock lever audio '%s' is missing", name.c_str());
			return false;
		}
	}
	for (uint outcome = 0; outcome < ARRAYSIZE(kOutcomeMediaNames); ++outcome) {
		if (!_library.hasMember(kOutcomeMediaNames[outcome])) {
			warning("Ripper: shock lever outcome media '%s' is missing",
				kOutcomeMediaNames[outcome]);
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded shock lever assets library='%s' "
		"background=%ux%u submit=%ux%u levers=%u framesPerLever=%u "
		"audio=%u outcomes=%u",
		kLibraryName, _background.width, _background.height,
		_submit.width, _submit.height, kLeverCount, kLeverFrameCount,
		kAudioCueCount, ARRAYSIZE(kOutcomeMediaNames));
	return true;
}

void ShockLeverPuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y, bool transparent) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= kRipperScreenHeight)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= kRipperScreenWidth)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (!transparent || pixel != frame.transparentColor)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

bool ShockLeverPuzzle::drawBackground() {
	if (_background.pixels.size() !=
			(uint32)_background.width * _background.height ||
			_background.palette.size() != kRipperPaletteByteCount)
		return false;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w != kRipperScreenWidth ||
			screen->h != kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	drawFrame((byte *)screen->getPixels(), screen->pitch, _background,
		0, kSceneOriginY, false);
	g_system->unlockScreen();

	Common::Array<byte> palette = _background.palette;
	_engine->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
	drawOverlays();
	return true;
}

void ShockLeverPuzzle::restoreLeverBackings(byte *screen, uint pitch) const {
	// AnimateShockLeverToFrame at 0x3ad98 opens a scoped display backing before
	// copying each transparent lever frame, then posts that complete dirty
	// region. Restore the corresponding SCREEN pixels before compositing the
	// replacement frame so transparent pixels cannot retain the prior pose.
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		if (_leverFrames[lever].empty())
			continue;
		const BitmapAssetFrame &frame = _leverFrames[lever].front();
		const int sourceX = kLeverPositions[lever].x;
		const int sourceY = kLeverPositions[lever].y - kSceneOriginY;
		if (sourceX < 0 || sourceY < 0 ||
				sourceX + frame.width > _background.width ||
				sourceY + frame.height > _background.height)
			continue;
		for (uint y = 0; y < frame.height; ++y) {
			memcpy(screen + (kLeverPositions[lever].y + y) * pitch +
					kLeverPositions[lever].x,
				_background.pixels.data() +
					(sourceY + y) * _background.width + sourceX,
				frame.width);
		}
	}
}

Common::Point ShockLeverPuzzle::puzzleHelpLabelPosition(uint lever) const {
	if (lever >= kLeverCount || _leverFrames[lever].empty())
		return Common::Point();

	const BitmapAssetFrame &frame = _leverFrames[lever].front();
	const uint frameHeight = MAX<uint>(frame.height / kLeverFrameCount, 1);
	const int targetY = kLeverPositions[lever].y - 5 +
		kTargetFrames[lever] * frameHeight + frameHeight / 2;
	const Common::String label = Common::String::format("%u", lever + 1);
	const int labelWidth = BitmapFontRenderer::measureText(
		_puzzleHelpFont, label);
	// The first two controls have room on their left; the third has room on
	// its right. Keeping the labels outside the lever art makes the exact
	// target position visible while the player drags through it.
	const int labelX = lever + 1 == kLeverCount ?
		kLeverPositions[lever].x + frame.width + 3 :
		kLeverPositions[lever].x - labelWidth - 3;
	return Common::Point(labelX,
		targetY - _puzzleHelpFont.lineHeight / 2);
}

void ShockLeverPuzzle::restorePuzzleHelpBackings(byte *screen,
		uint pitch) const {
	if (_puzzleHelpFont.glyphs.empty())
		return;

	for (uint lever = 0; lever < kLeverCount; ++lever) {
		const Common::String label = Common::String::format("%u", lever + 1);
		const Common::Point position = puzzleHelpLabelPosition(lever);
		const int labelWidth = BitmapFontRenderer::measureText(
			_puzzleHelpFont, label);
		const int left = MAX<int>(position.x - 2, 0);
		const int top = MAX<int>(position.y - 2, kSceneOriginY);
		const int right = MIN<int>(position.x + labelWidth + 2,
			_background.width);
		const int bottom = MIN<int>(position.y +
			_puzzleHelpFont.lineHeight + 2,
			kSceneOriginY + _background.height);
		if (left >= right || top >= bottom)
			continue;
		for (int y = top; y < bottom; ++y) {
			memcpy(screen + y * pitch + left,
				_background.pixels.data() +
					(y - kSceneOriginY) * _background.width + left,
				right - left);
		}
	}
}

void ShockLeverPuzzle::drawPuzzleHelpOverlay(byte *screen, uint pitch) const {
	if (!_puzzleHelpEnabled || _puzzleHelpFont.glyphs.empty())
		return;

	const Common::Rect clip(0, kSceneOriginY, kRipperScreenWidth,
		kSceneOriginY + kSceneHeight);
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		const Common::String label = Common::String::format("%u", lever + 1);
		const Common::Point position = puzzleHelpLabelPosition(lever);
		BitmapFontRenderer::drawTextClipped(screen, pitch,
			_puzzleHelpFont, position.x, position.y, label,
			kPuzzleHelpColor, clip);
	}
}

void ShockLeverPuzzle::drawOverlays() {
	if (_puzzleHelpEnabled && !_puzzleHelpFontLoadAttempted) {
		_puzzleHelpFontLoadAttempted = true;
		if (!_engine->getResources()->loadInterfaceBitmapFont(
				"small.fnt", _puzzleHelpFont)) {
			warning("Ripper: could not load shock lever puzzle-help font");
		} else {
			debugC(2, kDebugPuzzles,
				"Ripper: loaded shock lever puzzle-help font='small.fnt' glyphs=%u color=%u",
				_puzzleHelpFont.glyphs.size(), kPuzzleHelpColor);
		}
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	restoreLeverBackings(pixels, screen->pitch);
	restorePuzzleHelpBackings(pixels, screen->pitch);
	drawFrame(pixels, screen->pitch, _submit,
		kSubmitPosition.x, kSubmitPosition.y, true);
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		if (_leverFrame[lever] >= _leverFrames[lever].size())
			continue;
		drawFrame(pixels, screen->pitch,
			_leverFrames[lever][_leverFrame[lever]],
			kLeverPositions[lever].x, kLeverPositions[lever].y, true);
	}
	drawPuzzleHelpOverlay(pixels, screen->pitch);
	g_system->unlockScreen();
	_engine->getCursor()->setVisible(true);
	presentScreen();
}

int ShockLeverPuzzle::findLever(const Common::Point &point) const {
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		if (_leverFrames[lever].empty())
			continue;
		const BitmapAssetFrame &frame = _leverFrames[lever].front();
		if (Common::Rect(kLeverPositions[lever].x,
				kLeverPositions[lever].y,
				kLeverPositions[lever].x + frame.width,
				kLeverPositions[lever].y + frame.height).contains(point))
			return lever;
	}
	return -1;
}

ShockLeverPuzzle::HoverControl ShockLeverPuzzle::findControl(
		const Common::Point &point, int &lever) const {
	lever = findLever(point);
	if (lever >= 0)
		return kHoverLever;
	if (Common::Rect(kSubmitPosition.x, kSubmitPosition.y,
			kSubmitPosition.x + _submit.width,
			kSubmitPosition.y + _submit.height).contains(point))
		return kHoverSubmit;
	if (isExitPoint(point))
		return kHoverExit;
	return kHoverNone;
}

void ShockLeverPuzzle::updateCursor(const Common::Point &point) {
	int lever = -1;
	const HoverControl control = findControl(point, lever);
	if (control != _hoveredControl || lever != _hoveredLever) {
		debugC(3, kDebugPuzzles,
			"Ripper: shock lever hover control=%d lever=%d "
			"previousControl=%d previousLever=%d point=%d,%d",
			control, lever, _hoveredControl, _hoveredLever,
			point.x, point.y);
		_hoveredControl = control;
		_hoveredLever = lever;
	}

	uint cursor = kDefaultCursor;
	if (control == kHoverLever || control == kHoverSubmit)
		cursor = kSelectionCursor;
	else if (control == kHoverExit)
		cursor = kExitCursor;
	_engine->getCursor()->update(cursor);
}

void ShockLeverPuzzle::updateDraggedLever(const Common::Point &point) {
	if (_draggedLever < 0 || _draggedLever >= (int)kLeverCount)
		return;

	const BitmapAssetFrame &frame = _leverFrames[_draggedLever].front();
	const uint frameHeight = MAX<uint>(frame.height / kLeverFrameCount, 1);
	const int relativeY =
		point.y - (kLeverPositions[_draggedLever].y - 5);
	const uint targetFrame = CLIP<int>(
		relativeY / (int)frameHeight, 0, kLeverFrameCount - 1);
	const uint32 now = g_system->getMillis();
	if (_leverFrame[_draggedLever] == targetFrame) {
		g_system->getMixer()->stopHandle(_audioHandles[4]);
		return;
	}
	if ((int32)(now - _lastLeverStepMillis) <
				(int32)(kLeverStepTicks * kDosTickMillis))
		return;

	const uint previousFrame = _leverFrame[_draggedLever];
	if (!g_system->getMixer()->isSoundHandleActive(_audioHandles[4]))
		playCue(4);
	if (_leverFrame[_draggedLever] < targetFrame)
		++_leverFrame[_draggedLever];
	else
		--_leverFrame[_draggedLever];
	_lastLeverStepMillis = now;
	if (_leverFrame[_draggedLever] == targetFrame)
		g_system->getMixer()->stopHandle(_audioHandles[4]);
	debugC(3, kDebugPuzzles,
		"Ripper: shock lever moved lever=%d frame=%u->%u target=%u "
		"pointerY=%d frameHeight=%u",
		_draggedLever + 1, previousFrame, _leverFrame[_draggedLever],
		targetFrame, point.y, frameHeight);
}

bool ShockLeverPuzzle::playCue(uint cue, bool loop, uint volumePercent) {
	if (cue >= kAudioCueCount)
		return false;
	const Common::String name = Common::String::format("shock%u", cue);
	Common::SeekableReadStream *stream =
		_library.createReadStreamForMember(name);
	if (!stream)
		return false;
	Audio::SeekableAudioStream *wavStream =
		Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!wavStream) {
		warning("Ripper: could not decode shock lever audio '%s'",
			name.c_str());
		return false;
	}

	Audio::Mixer *mixer = g_system->getMixer();
	mixer->stopHandle(_audioHandles[cue]);
	Audio::AudioStream *playbackStream = wavStream;
	if (loop)
		playbackStream = Audio::makeLoopingAudioStream(wavStream, 0);
	const byte volume = (byte)(MIN<uint>(volumePercent, 100) *
		Audio::Mixer::kMaxChannelVolume / 100);
	mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioHandles[cue],
		playbackStream, -1, volume);
	debugC(3, kDebugPuzzles,
		"Ripper: shock lever played cue=%u path='%s' loop=%d volume=%u",
		cue, name.c_str(), loop, volumePercent);
	return true;
}

void ShockLeverPuzzle::stopAudio() {
	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		g_system->getMixer()->stopHandle(_audioHandles[cue]);
}

bool ShockLeverPuzzle::waitForCue(uint cue) {
	if (cue >= kAudioCueCount)
		return false;
	while (!_engine->shouldQuit() &&
			g_system->getMixer()->isSoundHandleActive(_audioHandles[cue])) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

uint ShockLeverPuzzle::countCorrect(bool logState) const {
	uint correctCount = 0;
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		const int difference =
			(int)_leverFrame[lever] - (int)kTargetFrames[lever];
		if (ABS(difference) <= 1)
			++correctCount;
	}
	if (logState) {
		debugC(1, kDebugPuzzles,
			"Ripper: shock lever validation frames=[%u,%u,%u] "
			"targets=[%u,%u,%u] tolerance=1 correct=%u/%u",
			_leverFrame[0], _leverFrame[1], _leverFrame[2],
			kTargetFrames[0], kTargetFrames[1], kTargetFrames[2],
			correctCount, kLeverCount);
	}
	return correctCount;
}

bool ShockLeverPuzzle::playOutcome(uint correctCount) {
	if (correctCount >= ARRAYSIZE(kOutcomeMediaNames))
		return false;
	const char *name = kOutcomeMediaNames[correctCount];
	debugC(1, kDebugPuzzles,
		"Ripper: shock lever presenting outcome correct=%u media='%s'",
		correctCount, name);
	_engine->getCursor()->setVisible(false);
	const bool result = _engine->getMedia()->playSceneStream(
		_library.createReadStreamForMember(name), name, 0, 0, true);
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(true);
	return result;
}

uint16 ShockLeverPuzzle::serviceKeyboard() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand || command == kHelpCommand)
			return command;
		if (command > 0xff)
			continue;

		char character = (char)command;
		if (character >= 'A' && character <= 'Z')
			character += 'a' - 'A';
		if (character == kCheatKeyword[_keywordIndex])
			++_keywordIndex;
		else
			_keywordIndex = 0;
		if (_keywordIndex + 1 == ARRAYSIZE(kCheatKeyword)) {
			debugC(1, kDebugPuzzles,
				"Ripper: shock lever completion keyword matched keyword='%s'",
				kCheatKeyword);
			_keywordIndex = 0;
			return kCheatCommand;
		}
	}
	return 0;
}

uint16 ShockLeverPuzzle::service(uint frame) {
	const uint16 keyboardCommand = serviceKeyboard();
	if (keyboardCommand != 0)
		return keyboardCommand;

	const MouseState mouse = _engine->getInput()->publishMouseState();
	const bool puzzleHelpEnabled = _engine->isPuzzleHelpEnabled();
	if (puzzleHelpEnabled != _puzzleHelpEnabled) {
		_puzzleHelpEnabled = puzzleHelpEnabled;
		debugC(2, kDebugPuzzles,
			"Ripper: shock lever puzzle-help overlay enabled=%d targets=[1:%u,2:%u,3:%u] tolerance=1 command=PUZZLE_HELP",
			_puzzleHelpEnabled, kTargetFrames[0], kTargetFrames[1],
			kTargetFrames[2]);
	}
	updateCursor(mouse.position);
	if ((mouse.pressed & kMouseButtonLeft) != 0) {
		int lever = -1;
		const HoverControl control = findControl(mouse.position, lever);
		if (control == kHoverExit) {
			debugC(1, kDebugPuzzles,
				"Ripper: shock lever puzzle exited by edge control "
				"point=%d,%d",
				mouse.position.x, mouse.position.y);
			return kEscapeCommand;
		}
		if (control == kHoverSubmit)
			return kSubmitCommand;
		if (control == kHoverLever) {
			_draggedLever = lever;
			_lastLeverStepMillis = 0;
			debugC(2, kDebugPuzzles,
				"Ripper: shock lever drag started lever=%d frame=%u "
				"point=%d,%d",
				lever + 1, _leverFrame[lever],
				mouse.position.x, mouse.position.y);
		}
	}
	if ((mouse.released & kMouseButtonLeft) != 0 && _draggedLever >= 0) {
		debugC(2, kDebugPuzzles,
			"Ripper: shock lever drag completed lever=%d frame=%u",
			_draggedLever + 1, _leverFrame[_draggedLever]);
		g_system->getMixer()->stopHandle(_audioHandles[4]);
		_draggedLever = -1;
	}
	if ((mouse.buttons & kMouseButtonLeft) != 0)
		updateDraggedLever(mouse.position);
	else if (_draggedLever >= 0) {
		g_system->getMixer()->stopHandle(_audioHandles[4]);
		_draggedLever = -1;
	}

	drawOverlays();
	debugC(11, kDebugPuzzles,
		"Ripper: serviced shock lever screen frame=%u "
		"levers=[%u,%u,%u] dragged=%d",
		frame, _leverFrame[0], _leverFrame[1], _leverFrame[2],
		_draggedLever);
	return 0;
}

ShockLeverPuzzle::Result ShockLeverPuzzle::run(uint completionFlag) {
	_puzzleHelpEnabled = _engine->isPuzzleHelpEnabled();
	if (!_incomingDisplay.capture() || !loadAssets() || !drawBackground()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	for (uint lever = 0; lever < kLeverCount; ++lever)
		_leverFrame[lever] = 3;
	_draggedLever = -1;
	_hoveredLever = -1;
	_hoveredControl = kHoverNone;
	_keywordIndex = 0;
	_solved = false;
	_completionRecorded = false;
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	playCue(2, true);
	drawOverlays();
	debugC(1, kDebugPuzzles,
		"Ripper: entered shock lever puzzle completionFlag=%u "
		"initialFrames=[%u,%u,%u] targets=[%u,%u,%u] tolerance=1 "
		"helpTable=0x%x puzzleHelp=%d",
		completionFlag, _leverFrame[0], _leverFrame[1], _leverFrame[2],
		kTargetFrames[0], kTargetFrames[1], kTargetFrames[2],
		kHelpSelectionTable, _puzzleHelpEnabled);

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		uint16 command = 0;
		Common::SeekableReadStream *screenStream =
			_library.createReadStreamForMember(kScreenMediaName);
		if (!_engine->getMedia()->playPuzzleSequenceStream(screenStream,
				kScreenMediaName, kScreenMediaPosition.x,
				kScreenMediaPosition.y, 1, this, &command)) {
			result = kLoadFailed;
			break;
		}

		switch (command) {
		case kEscapeCommand:
			active = false;
			break;
		case kHelpCommand:
			debugC(1, kDebugPuzzles,
				"Ripper: shock lever puzzle opening modal help table=0x%x",
				kHelpSelectionTable);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable))
				warning("Ripper: shock lever puzzle modal help failed");
			if (!drawBackground()) {
				result = kLoadFailed;
				active = false;
			}
			break;
		case kSubmitCommand: {
			playCue(3);
			const uint correctCount = countCorrect(true);
			playCue(1);
			playCue(0);
			g_system->getMixer()->setChannelVolume(_audioHandles[2],
				Audio::Mixer::kMaxChannelVolume * 40 / 100);
			if (!waitForCue(1) || !playOutcome(correctCount) ||
					!drawBackground()) {
				result = kLoadFailed;
				active = false;
				break;
			}
			g_system->getMixer()->setChannelVolume(_audioHandles[2],
				Audio::Mixer::kMaxChannelVolume);
			if (correctCount == kLeverCount) {
				_solved = true;
				result = kSolved;
				debugC(1, kDebugPuzzles,
					"Ripper: shock lever puzzle solved; "
					"milestone deferred until exit milestone=%u",
					completionFlag);
			}
			break;
		}
		case kCheatCommand:
			_solved = true;
			result = kSolved;
			// RunShockLeverPuzzleScene at 0x3b90b sets the supplied flag
			// before presenting M_2 on the hidden-keyword path.
			if (!_engine->getMilestones()->set(
					completionFlag, true, "shock-lever-keyword")) {
				result = kLoadFailed;
			} else {
				_completionRecorded = true;
			}
			if (result != kLoadFailed && !playOutcome(kLeverCount))
				result = kLoadFailed;
			active = false;
			break;
		default:
			if (!_engine->shouldQuit()) {
				warning("Ripper: shock lever screen loop returned "
					"unexpected command=0x%04x", command);
				result = kLoadFailed;
			}
			active = false;
			break;
		}
	}

	stopAudio();
	if (_solved && !_completionRecorded && result != kLoadFailed) {
		if (!_engine->getMilestones()->set(
				completionFlag, true, "shock-lever-puzzle")) {
			result = kLoadFailed;
		} else {
			_completionRecorded = true;
		}
	}
	_engine->getCursor()->update(0);
	_incomingDisplay.restore();
	_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugPuzzles,
		"Ripper: left shock lever puzzle result=%d solved=%d recorded=%d "
		"milestone=%u frames=[%u,%u,%u]",
		result, _solved, _completionRecorded, completionFlag,
		_leverFrame[0], _leverFrame[1], _leverFrame[2]);
	return result;
}

} // End of namespace Ripper
