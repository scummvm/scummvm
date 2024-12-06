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

#include "bagel/bagel.h"
#include "bagel/detection.h"
#include "bagel/music.h"

#include "backends/keymapper/keymapper.h"

#include "bagel/baglib/bagel.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/cursor.h"
#include "bagel/baglib/dossier_object.h"
#include "bagel/baglib/event_sdev.h"
#include "bagel/baglib/expression.h"
#include "bagel/baglib/inv.h"
#include "bagel/baglib/log_msg.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/moo.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/baglib/paint_table.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/parse_object.h"
#include "bagel/baglib/pda.h"
#include "bagel/baglib/sound_object.h"
#include "bagel/dialogs/start_dialog.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/var.h"
#include "bagel/baglib/wield.h"
#include "bagel/baglib/zoom_pda.h"

#include "bagel/boflib/cache.h"
#include "bagel/boflib/gfx/cursor.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/gui/window.h"

namespace Bagel {

#define SAVEGAME_VERSION 1

BagelEngine *g_engine;

BagelEngine::BagelEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Bagel") {
	g_engine = this;

	// baglib/ class statics initializations
	CBagCharacterObject::initialize();
	CBagCursor::initialize();
	CBagDossierObject::initialize();
	CBagEventSDev::initialize();
	CBagExpression::initialize();
	CBagInv::initialize();
	CBagLog::initialize();
	CBagMenu::initialize();
	CBagMenuDlg::initialize();
	CBagMoo::initialize();
	CBagPanWindow::initialize();
	CBagParseObject::initialize();
	CBagPDA::initialize();
	CBagSoundObject::initialize();
	CBagStorageDev::initialize();
	CBagStorageDevWnd::initialize();
	CBagVarManager::initialize();
	CBagWield::initialize();
	SBZoomPda::initialize();

	// boflib/ class statics initializations
	CCache::initialize();
	CBofCursor::initialize();
	CBofError::initialize();
	CBofPalette::initialize();
	CBofSound::initialize();
	CBofSprite::initialize();
	CBofWindow::initialize();

	_saveData.clear();
}

BagelEngine::~BagelEngine() {
	CBofSound::shutdown();
	CBofSprite::shutdown();
	CBagCursor::shutdown();
	CBagExpression::shutdown();
	CBagStorageDev::shutdown();

	delete _midi;
	delete _screen;
}

uint32 BagelEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String BagelEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Platform BagelEngine::getPlatform() const {
	return _gameDescription->platform;
}

bool BagelEngine::isDemo() const {
	return (_gameDescription->flags & ADGF_DEMO) != 0;
}

bool BagelEngine::canSaveLoadFromWindow(bool save) const {
	CBofWindow *win = CBofWindow::getActiveWindow();

	// Don't allow saves when capture/focus is active
	if (CBofApp::getApp()->getCaptureControl() != nullptr ||
			CBofApp::getApp()->getFocusControl() != nullptr ||
			win == nullptr)
		return false;

	// These two dialogs need to allow save/load for the ScummVM
	// dialogs to work from them when original save/load is disabled
	if ((dynamic_cast<CBagStartDialog *>(win) != nullptr && !save) ||
		dynamic_cast<CBagOptWindow *>(win) != nullptr)
		return true;

	// Otherwise, allow save/load if it's not a dialog, and it's
	// not a special view that shows the system cursor, like the
	// Nav Window minigame or Drink Mixer
	return dynamic_cast<CBofDialog *>(win) == nullptr &&
		!CBagCursor::isSystemCursorVisible();
}

bool BagelEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return canSaveLoadFromWindow(false);
}

bool BagelEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return canSaveLoadFromWindow(true);
}

Common::Error BagelEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_masterWin->fillSaveBuffer(&_saveData);

	return Engine::saveGameState(slot, desc, isAutosave);
}

Common::Error BagelEngine::saveGameState(int slot, const Common::String &desc,
		bool isAutosave, StBagelSave &saveData) {
	_saveData = saveData;
	return Engine::saveGameState(slot, desc, isAutosave);
}

Common::Error BagelEngine::loadGameState(int slot) {
	Common::Error result = Engine::loadGameState(slot);

	if (result.getCode() == Common::kNoError) {
		// Make sure we close any GUI windows before loading from GMM
		CBofWindow *win = CBofWindow::getActiveWindow();
		if (win)
			win->close();
		_masterWin->doRestore(&_saveData);
	}

	return result;
}

Common::Error BagelEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	stream->writeByte(SAVEGAME_VERSION);

	Common::Serializer s(nullptr, stream);
	_saveData.synchronize(s);
	return Common::kNoError;
}

Common::Error BagelEngine::loadGameStream(Common::SeekableReadStream *stream) {
	const byte version = stream->readByte();
	if (version > SAVEGAME_VERSION)
		error("Tried to load unsupported savegame version");

	Common::Serializer s(stream, nullptr);
	_saveData.synchronize(s);
	return Common::kNoError;
}

SaveStateList BagelEngine::listSaves() const {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

bool BagelEngine::savesExist() const {
	return !listSaves().empty();
}

void BagelEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause) {
		_midi->pause();
	} else {
		_midi->resume();
	}
}

void BagelEngine::errorDialog(const char *msg) const {
	GUIErrorMessage(msg);
}

void BagelEngine::enableKeymapper(bool enabled) {
	getEventManager()->getKeymapper()->setEnabled(enabled);
}

} // End of namespace Bagel
