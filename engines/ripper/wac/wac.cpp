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

#include "ripper/wac/wac.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/wac/broken_mug.h"
#include "ripper/wac/database.h"

namespace Ripper {

static const int kWacWidth = 640;
static const int kWacHeight = 400;
static const int kWacControlY = 349;
static const int kWacControlX[] = { 172, 252, 326, 390 };
static const uint16 kWacControlActions[] = { 0x1900, 0x2000, 0x3100, 0x3b00 };
static const uint kWacDefaultCursor = 14;
static const uint kWacControlCursor = 16;
static const int kWacIdleWindowX[] = { 64, 460 };
static const int kWacIdleWindowY[] = { 21, 19 };
static const uint32 kDosTickMillis = 55;
static const uint32 kWacIdleWindowInterval = 3 * kDosTickMillis;
static const uint16 kDosF10Command = 0x4400;
static const uint kWacDatabaseSkinFrameCount = 16;
static const uint kWacFrontEndHelpResource = 404;
static const uint kWacNotebookTitleResource = 0x49;
static const uint kWacNotebookMaximumBytes = 0x27c0;
static const char *const kWacNotebookFileName = "ripper.txt";
static const char *const kWacNotebookUpdateAudio = "wacnote.wav";

WacManager::WacManager(RipperEngine *engine) : _engine(engine),
		_idleWindowLastMillis(0), _hoveredControl(-1), _pressedControl(-1),
		_notebookUpdatePending(false), _initialized(false), _demoVariant(false) {
	_idleWindowFrame[0] = 0;
	_idleWindowFrame[1] = 0;
}

bool WacManager::initialize(ResourceManager &resources, bool demoVariant) {
	_demoVariant = demoVariant;
	if (!resources.loadInterfacePcx("wac.pcx", _background) ||
		_background.width != kWacWidth || _background.height != kWacHeight ||
		_background.palette.size() < 256 * 3 ||
		!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
		!resources.loadGameText(_gameText))
		return false;

	_controls.clear();
	_controlBitmaps.clear();
	for (uint i = 0; i < ARRAYSIZE(kWacControlActions); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("wac%u.bbm", i), sequence) || sequence.frames.empty())
			return false;
		_controlBitmaps.push_back(Common::move(sequence.frames[0]));
		const BitmapAssetFrame &bitmap = _controlBitmaps.back();
		const Common::Rect bounds(kWacControlX[i], kWacControlY,
			kWacControlX[i] + bitmap.width, kWacControlY + bitmap.height);
		_controls.add(bounds, kWacControlActions[i]);
		debugC(2, kDebugWac,
			"Ripper: WAC front-end control=%u action=0x%x rect=%d,%d,%d,%d",
			i, _controls[i].action, bounds.left, bounds.top,
			bounds.width(), bounds.height());
	}

	for (uint i = 0; !_demoVariant && i < ARRAYSIZE(_idleWindowAnimations); ++i) {
		if (!resources.loadInterfaceBitmapSet(
			Common::String::format("wacwn%u", i + 1), _idleWindowAnimations[i]))
			return false;
		debugC(2, kDebugWac,
			"Ripper: WAC idle window slot=%u position=%d,%d frames=%u intervalTicks=3",
			i, kWacIdleWindowX[i], kWacIdleWindowY[i], _idleWindowAnimations[i].size());
	}

	_databaseSkin.clear();
	for (uint i = 0; !_demoVariant && i < kWacDatabaseSkinFrameCount; ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
			Common::String::format("wacmnu%u", i), sequence) || sequence.frames.empty())
			return false;
		_databaseSkin.push_back(Common::move(sequence.frames[0]));
	}

	_databaseScrollArrows.resize(_demoVariant ? 0 : 4);
	for (uint i = 0; i < _databaseScrollArrows.size(); ++i) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("mnarrow%u.bbm", i), sequence) ||
				sequence.frames.empty())
			return false;
		_databaseScrollArrows[i] = Common::move(sequence.frames[0]);
	}

	_initialized = true;
	debugC(1, kDebugWac,
		"Ripper: initialized %s WAC front end background=wac.pcx controls=%u databaseSkinFrames=%u layout=screen-row-major",
		_demoVariant ? "demo" : "retail", _controls.size(), _databaseSkin.size());
	return true;
}

bool WacManager::captureDisplay() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != kWacWidth ||
		screen->h != kWacHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	_savedPixels.resize(kWacWidth * kWacHeight);
	for (int y = 0; y < kWacHeight; ++y)
		memcpy(_savedPixels.data() + y * kWacWidth, screen->getBasePtr(0, y), kWacWidth);
	g_system->unlockScreen();
	_savedPalette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(_savedPalette.data(), 0, 256);
	return true;
}

void WacManager::restoreDisplay() {
	if (_savedPixels.size() != kWacWidth * kWacHeight || _savedPalette.size() != 256 * 3)
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != kWacWidth ||
		screen->h != kWacHeight) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kWacHeight; ++y)
		memcpy(screen->getBasePtr(0, y), _savedPixels.data() + y * kWacWidth, kWacWidth);
	g_system->unlockScreen();
	g_system->getPaletteManager()->setPalette(_savedPalette.data(), 0, 256);
	presentScreen();
	_savedPixels.clear();
	_savedPalette.clear();
}

void WacManager::drawBitmap(const BitmapAssetFrame &bitmap, int x, int y) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	IndexedBitmapRenderer::drawBitmap((byte *)screen->getPixels(),
		screen->pitch, bitmap, x, y, Common::Rect(0, 0, screen->w, screen->h));
	g_system->unlockScreen();
}

void WacManager::drawFrontEnd() const {
	g_system->copyRectToScreen(_background.pixels.data(), _background.width,
		0, 0, _background.width, _background.height);
	g_system->getPaletteManager()->setPalette(_background.palette.data(), 0, 256);
	for (uint i = 0; i < _controls.size(); ++i)
		drawBitmap(_controlBitmaps[i], _controls[i].bounds.left, _controls[i].bounds.top);
	presentScreen();
}

void WacManager::serviceIdleWindowAnimations() {
	const uint32 now = g_system->getMillis(true);
	if (now - _idleWindowLastMillis < kWacIdleWindowInterval)
		return;

	for (uint slot = 0; slot < ARRAYSIZE(_idleWindowAnimations); ++slot) {
		if (_idleWindowAnimations[slot].empty())
			continue;
		drawBitmap(_idleWindowAnimations[slot][_idleWindowFrame[slot]],
			kWacIdleWindowX[slot], kWacIdleWindowY[slot]);
		debugC(11, kDebugWac,
			"Ripper: WAC idle window slot=%u frame=%u position=%d,%d",
			slot, _idleWindowFrame[slot], kWacIdleWindowX[slot], kWacIdleWindowY[slot]);
		_idleWindowFrame[slot] = (_idleWindowFrame[slot] + 1) %
			_idleWindowAnimations[slot].size();
	}
	_engine->getCursor()->refresh();
	_idleWindowLastMillis = now;
}

void WacManager::serviceIdleEffects() {
	serviceIdleWindowAnimations();
}

int WacManager::findControl(const Common::Point &point) const {
	return _controls.findFirst(point);
}

uint16 WacManager::serviceFrontEndControls(const MouseState &mouse,
		uint fallbackCursor, bool updateCursor) {
	// RunWacFrontEndLoop at 0x21865 leaves these four UiControlState records in
	// the shared list. ServiceWacSceneInputAction at 0x21eef therefore services
	// them from every WAC subscene, not only from the front page.
	const int hoveredControl = findControl(mouse.position);
	if (hoveredControl != _hoveredControl) {
		_hoveredControl = hoveredControl;
		debugC(2, kDebugWac,
			"Ripper: WAC persistent-control hover control=%d action=0x%x point=%d,%d",
			_hoveredControl, _hoveredControl < 0 ? 0 : _controls[_hoveredControl].action,
			mouse.position.x, mouse.position.y);
	}
	if (updateCursor) {
		_engine->getCursor()->update(
			_hoveredControl < 0 ? fallbackCursor : kWacControlCursor);
	}

	if ((mouse.pressed & kMouseButtonLeft) != 0)
		_pressedControl = _hoveredControl;
	if ((mouse.released & kMouseButtonLeft) == 0 || _pressedControl < 0)
		return kNoAction;

	const int pressedControl = _pressedControl;
	_pressedControl = -1;
	if (pressedControl != _hoveredControl)
		return kNoAction;

	debugC(1, kDebugWac,
		"Ripper: WAC persistent-control selected control=%d action=0x%x",
		pressedControl, _controls[pressedControl].action);
	return _controls[pressedControl].action;
}

Common::String WacManager::notebookFileName() const {
	return ConfMan.getActiveDomainName() + "-" + kWacNotebookFileName;
}

bool WacManager::loadNotebookText(Common::String &body) const {
	body.clear();
	Common::ScopedPtr<Common::InSaveFile> savedFile(
		_engine->getSaveFileManager()->openForLoading(notebookFileName()));
	Common::File resourceFile;
	Common::SeekableReadStream *stream = savedFile.get();
	const char *source = "save-directory";
	if (!stream && resourceFile.open(kWacNotebookFileName)) {
		stream = &resourceFile;
		source = "game-data";
	}
	if (!stream) {
		debugC(2, kDebugWac,
			"Ripper: WAC notebook source='%s' is absent; using empty text",
			notebookFileName().c_str());
		return true;
	}

	const uint32 byteCount = MIN<uint32>(stream->size(), kWacNotebookMaximumBytes);
	Common::Array<char> buffer;
	buffer.resize(byteCount);
	const uint32 bytesRead = byteCount == 0 ? 0 : stream->read(buffer.data(), byteCount);
	if (stream->err()) {
		warning("Ripper: could not read WAC notebook from %s", source);
		return false;
	}
	if (bytesRead != 0)
		body = Common::String(buffer.data(), buffer.data() + bytesRead);
	debugC(2, kDebugWac,
		"Ripper: loaded WAC notebook source=%s bytes=%u", source, body.size());
	return true;
}

bool WacManager::saveNotebookText(const Common::String &body) const {
	Common::ScopedPtr<Common::OutSaveFile> file(
		_engine->getSaveFileManager()->openForSaving(notebookFileName(), false));
	if (!file) {
		warning("Ripper: could not open WAC notebook '%s' for writing",
			notebookFileName().c_str());
		return false;
	}
	if (!body.empty())
		file->write(body.c_str(), body.size());
	file->finalize();
	if (file->err()) {
		warning("Ripper: could not save WAC notebook '%s'",
			notebookFileName().c_str());
		return false;
	}
	return true;
}

bool WacManager::appendNotebookResourceString(uint resourceId) {
	if (resourceId == 0 || resourceId > _gameText.size()) {
		warning("Ripper: WAC notebook resource string %u is unavailable", resourceId);
		return false;
	}

	Common::String body;
	if (!loadNotebookText(body))
		return false;
	body += "\n\n";
	body += resourceString(resourceId);
	body += '\n';
	if (!saveNotebookText(body))
		return false;
	_notebookUpdatePending = true;
	debugC(1, kDebugWac,
		"Ripper: appended resource string=%u to WAC notebook bytes=%u updatePending=1",
		resourceId, body.size());
	return true;
}

bool WacManager::resetNotebook() {
	if (!saveNotebookText(Common::String()))
		return false;
	_notebookUpdatePending = false;
	debugC(1, kDebugWac,
		"Ripper: reset WAC notebook for new game file='%s'",
		notebookFileName().c_str());
	return true;
}

bool WacManager::runNotebook() {
	Common::String body;
	if (!loadNotebookText(body))
		return false;

	const Common::String &title = resourceString(kWacNotebookTitleResource);
	debugC(1, kDebugWac,
		"Ripper: opening WAC notebook action=0x3100 source='%s' bytes=%u",
		notebookFileName().c_str(), body.size());
	_engine->getCursor()->setVisible(true);
	return _engine->getModalDialog()->runText(
		title, body, kWacNotebookFileName);
}

uint16 WacManager::dispatchSubsceneAction(uint16 action, uint helpResourceId,
		bool databaseActive) {
	switch (action) {
	case kNoAction:
		return kNoAction;
	case kExitAction:
		debugC(1, kDebugWac,
			"Ripper: WAC persistent power action=0x1900 requested front-end exit");
		return kExitAction;
	case kDatabaseAction:
		if (!databaseActive)
			return kDatabaseAction;
		debugC(2, kDebugWac,
			"Ripper: ignored nested WAC database action=0x2000 while chooser is active");
		return kNoAction;
	case kTextViewerAction:
		if (!runNotebook())
			warning("Ripper: WAC notebook action failed");
		_engine->getInput()->discardMouseTransitions();
		_pressedControl = -1;
		return kNoAction;
	case kHelpAction:
		debugC(1, kDebugWac,
			"Ripper: opening contextual WAC help resource=%u", helpResourceId);
		_engine->getCursor()->setVisible(true);
		if (!_engine->getModalDialog()->run(helpResourceId, false,
				ModalDialogManager::kWacPresentation))
			warning("Ripper: WAC contextual help resource=%u failed", helpResourceId);
		_engine->getInput()->discardMouseTransitions();
		_pressedControl = -1;
		return kNoAction;
	default:
		warning("Ripper: unsupported WAC front-end action 0x%x", action);
		return kNoAction;
	}
}

bool WacManager::dispatchAction(uint16 action) {
	const uint16 result = dispatchSubsceneAction(action, kWacFrontEndHelpResource, false);
	if (result == kExitAction)
		return false;
	if (result == kDatabaseAction) {
		if (_demoVariant) {
			// The demo RunWacFrontEndLoop at 0x1c085 routes 0x2000 directly to
			// RunWacMugSelectionScene at 0x1c58e; it has no retail database list.
			BrokenMugPuzzle puzzle(_engine);
			const BrokenMugPuzzle::Result puzzleResult = puzzle.run();
			drawFrontEnd();
			_engine->getInput()->discardMouseTransitions();
			debugC(1, kDebugWac,
				"Ripper: demo WAC mug selection completed result=%d",
				puzzleResult);
			return puzzleResult != BrokenMugPuzzle::kExitWac;
		}
		WacDatabaseSession database(this);
		const uint16 databaseResult = database.run();
		drawFrontEnd();
		_engine->getInput()->discardMouseTransitions();
		return databaseResult != kExitAction;
	}
	return true;
}

const Common::String &WacManager::resourceString(uint resourceId) const {
	static const Common::String empty;
	if (resourceId == 0 || resourceId > _gameText.size())
		return empty;
	return _gameText[resourceId - 1];
}

uint WacManager::measureText(const Common::String &text) const {
	return BitmapFontRenderer::measureText(_font, text);
}

void WacManager::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	BitmapFontRenderer::drawText(screen, pitch, _font, x, y, text, color);
}

void WacManager::run() {
	if (!_initialized || !captureDisplay()) {
		warning("Ripper: WAC front end could not capture the active presentation");
		return;
	}

	// RunWacFrontEndLoop at 0x21865 owns a modal chooser registry and restores
	// the borrowed scene presentation after its 0x1900 or Escape exit action.
	debugC(1, kDebugWac, "Ripper: entered WAC front-end loop");
	_engine->getInput()->discardMouseTransitions();
	_hoveredControl = -1;
	_pressedControl = -1;
	drawFrontEnd();
	_idleWindowFrame[0] = 0;
	_idleWindowFrame[1] = 0;
	_idleWindowLastMillis = g_system->getMillis(true);
	_engine->getCursor()->update(kWacDefaultCursor);
	if (_notebookUpdatePending) {
		Audio::SoundHandle updateHandle;
		const bool played = _engine->getMedia()->playSoundEffect(
			kWacNotebookUpdateAudio, updateHandle);
		debugC(played ? 1 : 2, kDebugWac,
			"Ripper: consumed WAC notebook update notification audio='%s' played=%d",
			kWacNotebookUpdateAudio, played);
		_notebookUpdatePending = false;
	}

	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b || command == kExitAction || command == kDosF10Command) {
				debugC(1, kDebugWac,
					"Ripper: WAC front end keyboard exit command=0x%x", command);
				active = false;
			} else if (command == kHelpAction || command == kTextViewerAction ||
					command == kDatabaseAction) {
				active = dispatchAction(command);
			}
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		serviceIdleWindowAnimations();
		const uint16 controlAction = serviceFrontEndControls(mouse, kWacDefaultCursor);
		if (controlAction != kNoAction)
			active = dispatchAction(controlAction);
		presentScreen();
		g_system->delayMillis(10);
	}

	_engine->getCursor()->setVisible(false);
	restoreDisplay();
	debugC(1, kDebugWac, "Ripper: left WAC front-end loop");
}

} // End of namespace Ripper
