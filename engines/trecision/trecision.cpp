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

#include "common/scummsys.h"
#include "common/error.h"
#include "common/system.h"
#include "common/events.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/translation.h"
#include "gui/saveload.h"

#include "trecision/trecision.h"
#include "trecision/anim.h"
#include "trecision/scheduler.h"
#include "trecision/actor.h"
#include "trecision/3d.h"
#include "trecision/defines.h"
#include "trecision/console.h"
#include "trecision/dialog.h"
#include "trecision/graphics.h"
#include "trecision/video.h"
#include "trecision/logic.h"
#include "trecision/text.h"
#include "trecision/sound.h"

namespace Common {
class File;
}

namespace Trecision {

TrecisionEngine *g_vm;

TrecisionEngine::TrecisionEngine(OSystem *syst, const ADGameDescription *desc) : Engine(syst), _gameDescription(desc) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "AUTORUN");
	SearchMan.addSubDirectoryMatching(gameDataDir, "DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "FMV");

	g_vm = nullptr;

	_curRoom = 0;
	_oldRoom = 0;

	_curInventory = 0;

	for (int i = 0; i < 10; ++i)
		_curScriptFrame[i] = 0;

	_iconBase = 0;
	_inventoryRefreshStartIcon = 0;
	_lastCurInventory = 0;
	_flagInventoryLocked = false;
	_inventorySpeed[0] = 20;
	_inventorySpeed[1] = 10;
	_inventorySpeed[2] = 5;
	_inventorySpeed[3] = 3;
	_inventorySpeed[4] = 2;
	_inventorySpeed[5] = _inventorySpeed[6] = _inventorySpeed[7] = 0;
	_inventorySpeedIndex = 0;
	_inventoryScrollTime = 0;

	_fastWalk = false;

	// Use With
	_useWith[0] = _useWith[1] = 0;
	_useWithInv[0] = _useWithInv[1] = false;

	// Messages
	for (int i = 0; i < MAXOBJNAME; ++i)
		_objName[i] = nullptr;

	for (int i = 0; i < MAXSENTENCE; ++i)
		_sentence[i] = nullptr;

	for (int i = 0; i < MAXSYSTEXT; ++i)
		_sysText[i] = nullptr;

	_curMessage = nullptr;

	// Original values
	_idleMsg = {MC_IDLE, 0, MP_DEFAULT, 0, 0, 0, 0, 0};
	_curObj = 1;
	_inventoryStatus = INV_OFF;
	_lightIcon = 0xFF;
	_inventoryRefreshStartLine = INVENTORY_HIDE;
	_lastLightIcon = 0xFF;
	_inventoryCounter = INVENTORY_HIDE;

	_animMgr = nullptr;
	_dialogMgr = nullptr;
	_graphicsMgr = nullptr;
	_logicMgr = nullptr;
	_soundMgr = nullptr;
	_renderer = nullptr;
	_pathFind = nullptr;
	_textMgr = nullptr;
	_animTypeMgr = nullptr;
	
	_actorRect = nullptr;
	_nextRefresh = 0;

	_curKey = Common::KEYCODE_INVALID;
	_curAscii = 0;
	_mousePos = Common::Point(0, 0);
	_mouseLeftBtn = _mouseRightBtn = false;
	_keybInput = false;

	_gamePaused = false;

	_closeUpObj = 0;
	_textPtr = nullptr;
	_lastInv = 0;
	_lastObj = 0;

	_curStack = 0;

	_flagscriptactive = false;

	_font = nullptr;
	_arrows = nullptr;
	_textureArea = nullptr;
	_icons = nullptr;
	_actor = nullptr;

	_flagDialogActive = false;
	_flagDialogMenuActive = false;
	_flagSkipTalk = false;
	_flagPaintCharacter = false;
	_flagShowCharacter = true;
	_flagSomeoneSpeaks = false;
	_flagCharacterSpeak = false;
	_flagUseWithStarted = false;
	_flagCharacterExists = true;
	_flagNoPaintScreen = false;
	_flagWaitRegen = false;

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		_objPointers[i] = nullptr;
		_maskPointers[i] = nullptr;
	}

	_blinkLastDTextChar = MASKCOL;
	_curTime = 0;
	_characterSpeakTime = 0;

	_actorPos = 0;
	_forcedActorPos = 0;

	for (int i = 0; i < MAXDTEXTLINES; ++i) {
		for (int j = 0; j < MAXDTEXTCHARS; ++j)
			_drawTextLines[i][j] = '\0';
	}
}

TrecisionEngine::~TrecisionEngine() {
	if (_animMgr)
		_animMgr->stopAllSmkAnims();

	_dataFile.close();
	_soundMgr->stopSoundSystem();

	delete _animMgr;
	delete _dialogMgr;
	delete _graphicsMgr;
	delete _logicMgr;
	delete _soundMgr;
	delete _renderer;
	delete _pathFind;
	delete _textMgr;
	delete _scheduler;
	delete _animTypeMgr;
	
	delete[] _font;
	delete[] _arrows;
	delete[] _textureArea;
	delete[] _icons;
	delete _actor;
	delete[] TextArea;

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		delete[] _objPointers[i];
		delete[] _maskPointers[i];
	}

}

Common::Error TrecisionEngine::run() {
	g_vm = this;

	syncSoundSettings();

	_graphicsMgr = new GraphicsManager(this);
	if (!_graphicsMgr->initScreen())
		return Common::kUnsupportedColorMode;
	_animMgr = new AnimManager(this);
	_dialogMgr = new DialogManager(this);
	_logicMgr = new LogicManager(this);
	_soundMgr = new SoundManager(this);
	_pathFind = new PathFinding3D(this);
	_renderer = new Renderer3D(this);
	_textMgr = new TextManager(this);
	_scheduler = new Scheduler(this);
	_animTypeMgr = new AnimTypeManager(this);
	
	setDebugger(new Console(this));

	initMain();
	_graphicsMgr->initCursor();

	// Check if a saved game is to be loaded from the launcher
	if (ConfMan.hasKey("save_slot"))
		loadGameState(ConfMan.getInt("save_slot"));

	while (!shouldQuit()) {
		eventLoop();
		if (!_flagNoPaintScreen)
			processTime();

		processMouse();
		_scheduler->process();

		if (_curMessage->_class == MC_SYSTEM && _curMessage->_event == ME_QUIT)
			break;

		_animTypeMgr->handler(kAnimTypeBackground);

		processCurrentMessage();

		if (_flagscriptactive)
			evalScript();
	}

	if (isDemo())
		_graphicsMgr->showDemoPic();

	return Common::kNoError;
}

bool TrecisionEngine::isDemo() const {
	return _gameDescription->flags & ADGF_DEMO;
}

void TrecisionEngine::eventLoop() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_mouseLeftBtn = true;
			break;

		case Common::EVENT_LBUTTONUP:
			_mouseLeftBtn = false;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mouseRightBtn = true;
			break;

		case Common::EVENT_RBUTTONUP:
			_mouseRightBtn = false;
			break;

		case Common::EVENT_KEYUP:
			_curKey = event.kbd.keycode;
			_curAscii = event.kbd.ascii;
			switch (event.kbd.keycode) {
			case Common::KEYCODE_p:
				if (!_gamePaused && !_keybInput) {
					_curKey = Common::KEYCODE_INVALID;
					_gamePaused = true;
					waitKey();
				}
				_gamePaused = false;
				break;

			case Common::KEYCODE_CAPSLOCK:
				_fastWalk ^= true;
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
	g_system->delayMillis(10);
	g_system->updateScreen();
}

bool TrecisionEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsSubtitleOptions) ||
//		   (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime) ||
		   (f == kSupportsChangingOptionsDuringRuntime);
}

Common::Error TrecisionEngine::loadGameStream(Common::SeekableReadStream *stream) {
	const byte version = stream->readByte();
	Common::Serializer ser(stream, nullptr);
	ser.setVersion(version);
	syncGameStream(ser);
	return Common::kNoError;
}

Common::Error TrecisionEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	const byte version = SAVE_VERSION_SCUMMVM;
	Common::Serializer ser(nullptr, stream);
	ser.setVersion(version);
	stream->writeByte(version);
	syncGameStream(ser);
	return Common::kNoError;
}

bool TrecisionEngine::syncGameStream(Common::Serializer &ser) {
	uint16 unused = 0;

	if (ser.isLoading()) {
		ser.skip(40, SAVE_VERSION_ORIGINAL, SAVE_VERSION_ORIGINAL);	// description
		ser.skip(ICONDX * ICONDY * sizeof(uint16), SAVE_VERSION_ORIGINAL, SAVE_VERSION_ORIGINAL); // thumbnail
	}

	ser.syncAsUint16LE(_curRoom);
	ser.syncAsByte(unused);	// _inventorySize
	ser.syncAsByte(unused); // _cyberInventorySize
	ser.syncAsByte(_iconBase);
	ser.syncAsSint16LE(_flagSkipTalk);
	ser.syncAsSint16LE(unused); // _flagSkipEnable
	ser.syncAsSint16LE(unused);	// _flagMouseEnabled
	ser.syncAsSint16LE(unused);	// _flagScreenRefreshed
	ser.syncAsSint16LE(_flagPaintCharacter);
	ser.syncAsSint16LE(_flagSomeoneSpeaks);
	ser.syncAsSint16LE(_flagCharacterSpeak);
	ser.syncAsSint16LE(_flagInventoryLocked);
	ser.syncAsSint16LE(_flagUseWithStarted);
	ser.syncAsSint16LE(unused);	// FlagMousePolling
	ser.syncAsSint16LE(unused); // FlagDialogSolitaire
	ser.syncAsSint16LE(_flagCharacterExists);

	syncInventory(ser);
	_actor->syncGameStream(ser);
	_pathFind->syncGameStream(ser);

	for (int a = 0; a < MAXROOMS; a++) {
		ser.syncBytes((byte *)_room[a]._baseName, 4);
		for (int i = 0; i < MAXACTIONINROOM; i++)
			ser.syncAsUint16LE(_room[a]._actions[i]);
		ser.syncAsByte(_room[a]._flag);
		ser.syncAsUint16LE(_room[a]._bkgAnim);
	}

	for (int a = 0; a < MAXOBJ; a++) {
		ser.syncAsUint16LE(_obj[a]._lim.left);
		ser.syncAsUint16LE(_obj[a]._lim.top);
		ser.syncAsUint16LE(_obj[a]._lim.right);
		ser.syncAsUint16LE(_obj[a]._lim.bottom);
		ser.syncAsUint16LE(_obj[a]._name);
		ser.syncAsUint16LE(_obj[a]._examine);
		ser.syncAsUint16LE(_obj[a]._action);
		ser.syncAsUint16LE(_obj[a]._anim);
		ser.syncAsByte(_obj[a]._mode);
		ser.syncAsByte(_obj[a]._flag);
		ser.syncAsByte(_obj[a]._goRoom);
		ser.syncAsByte(_obj[a]._nbox);
		ser.syncAsByte(_obj[a]._ninv);
		ser.syncAsSByte(_obj[a]._position);
	}

	for (int a = 0; a < MAXINVENTORY; a++) {
		ser.syncAsUint16LE(_inventoryObj[a]._name);
		ser.syncAsUint16LE(_inventoryObj[a]._examine);
		ser.syncAsUint16LE(_inventoryObj[a]._action);
		ser.syncAsUint16LE(_inventoryObj[a]._anim);
		ser.syncAsByte(_inventoryObj[a]._flag);
	}

	_animMgr->syncGameStream(ser);
	_soundMgr->syncGameStream(ser);
	_dialogMgr->syncGameStream(ser);
	_logicMgr->syncGameStream(ser);

	return true;
}

void TrecisionEngine::initMain() {
	for (int c = 0; c < MAXOBJ; c++)
		_obj[c]._position = -1;

	_logicMgr->initScript();
	openDataFiles();
	openSys();

	loadAll();

	initMessageSystem();
	_logicMgr->initInventory();

	_curRoom = kRoomIntro;

	processTime();

	_scheduler->doEvent(MC_SYSTEM, ME_START, MP_DEFAULT, 0, 0, 0, 0);
}

void TrecisionEngine::openDataFiles() {
	if (!_dataFile.open("nldata.cd0"))
		error("openDataFiles() - Error opening nldata.cd0");

	if (!isDemo()) {
		if (!Common::File::exists("nlanim.cd1") ||
			!Common::File::exists("nlanim.cd2") ||
			!Common::File::exists("nlanim.cd3"))
			error("openDataFiles() - nlanim.cd1 or nlanim.cd2 or nlanim.cd3 is missing");
	}
	
	_font = readData("nlfont.fnt");
	int size;
	_arrows = readData16("frecc.bm", size);
	_graphicsMgr->updatePixelFormat(_arrows, size);

	Common::SeekableReadStream *ff = _dataFile.createReadStreamForMember("icone.bm");
	size = ceil(ff->size() / 2.0);
	int iconSize = ICONDX * ICONDY;
	int arraySize = size + iconSize * (INVICONNUM + 1);
	_icons = new uint16[arraySize];
	for (int i = 0; i < arraySize; ++i)
		_icons[i] = 0;
	for (int i = 0; i < size; ++i)
		_icons[iconSize + i] = ff->readUint16LE();
	delete ff;
	_graphicsMgr->updatePixelFormat(&_icons[iconSize], size);

	_textureArea = readData("textur.bm");
}

void TrecisionEngine::initMessageSystem() {
	_gameQueue.initQueue();
	_animQueue.initQueue();
	_characterQueue.initQueue();
	for (uint8 i = 0; i < MAXMESSAGE; i++) {
		_gameQueue._event[i] = &_gameMsg[i];
		_characterQueue._event[i] = &_characterMsg[i];
		_animQueue._event[i] = &_animMsg[i];
	}
}

void TrecisionEngine::loadAll() {
	Common::File dataNl;
	if (!dataNl.open("DATA.NL"))
		error("loadAll : Couldn't open DATA.NL");

	for (int i = 0; i < MAXROOMS; ++i) {
		dataNl.read(&_room[i]._baseName, ARRAYSIZE(_room[i]._baseName));
		_room[i]._flag = dataNl.readByte();
		dataNl.readByte(); // Padding
		_room[i]._bkgAnim = dataNl.readUint16LE();
		for (int j = 0; j < MAXOBJINROOM; ++j)
			_room[i]._object[j] = dataNl.readUint16LE();
		for (int j = 0; j < MAXSOUNDSINROOM; ++j)
			_room[i]._sounds[j] = dataNl.readUint16LE();
		for (int j = 0; j < MAXACTIONINROOM; ++j)
			_room[i]._actions[j] = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXOBJ; ++i) {
		uint16 w = dataNl.readUint16LE();
		uint16 h = dataNl.readUint16LE();
		_obj[i]._rect.left = dataNl.readUint16LE();
		_obj[i]._rect.top = dataNl.readUint16LE();
		_obj[i]._rect.setWidth(w);
		_obj[i]._rect.setHeight(h);

		_obj[i]._lim.left = dataNl.readUint16LE();
		_obj[i]._lim.top = dataNl.readUint16LE();
		_obj[i]._lim.right = dataNl.readUint16LE();
		_obj[i]._lim.bottom = dataNl.readUint16LE();

		_obj[i]._position = dataNl.readSByte();
		dataNl.readByte(); // Padding
		_obj[i]._name = dataNl.readUint16LE();
		_obj[i]._examine = dataNl.readUint16LE();
		_obj[i]._action = dataNl.readUint16LE();
		_obj[i]._goRoom = dataNl.readByte();
		_obj[i]._nbox = dataNl.readByte();
		_obj[i]._ninv = dataNl.readByte();
		_obj[i]._mode = dataNl.readByte();
		_obj[i]._flag = dataNl.readByte();
		dataNl.readByte(); // Padding
		_obj[i]._anim = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXINVENTORY; ++i) {
		_inventoryObj[i]._name = dataNl.readUint16LE();
		_inventoryObj[i]._examine = dataNl.readUint16LE();
		_inventoryObj[i]._action = dataNl.readUint16LE();
		_inventoryObj[i]._flag = dataNl.readByte();
		dataNl.readByte(); // Padding
		_inventoryObj[i]._anim = dataNl.readUint16LE();
	}

	_soundMgr->loadSamples(&dataNl);

	for (int i = 0; i < MAXSCRIPTFRAME; ++i) {
		_scriptFrame[i]._class = dataNl.readByte();
		_scriptFrame[i]._event = dataNl.readByte();
		_scriptFrame[i]._u8Param = dataNl.readByte();
		dataNl.readByte(); // Padding
		_scriptFrame[i]._u16Param1 = dataNl.readUint16LE();
		_scriptFrame[i]._u16Param2 = dataNl.readUint16LE();
		_scriptFrame[i]._u32Param = dataNl.readUint16LE();
		_scriptFrame[i]._noWait = !(dataNl.readSint16LE() == 0);
	}

	for (int i = 0; i < MAXSCRIPT; ++i) {
		_script[i]._firstFrame = dataNl.readUint16LE();
		_script[i]._flag = dataNl.readByte();
		dataNl.readByte(); // Padding
	}

	_animMgr->loadAnimTab(&dataNl);
	_dialogMgr->loadData(&dataNl);

	for (int i = 0; i < MAXACTION; ++i)
		_actionLen[i] = dataNl.readByte();

	int numFileRef = dataNl.readSint32LE();
	dataNl.skip(numFileRef * (12 + 4));	// fileRef name + offset

	dataNl.read(TextArea, MAXTEXTAREA);

	_textPtr = TextArea;

	for (int a = 0; a < MAXOBJNAME; a++)
		_objName[a] = getNextSentence();

	for (int a = 0; a < MAXSENTENCE; a++)
		_sentence[a] = getNextSentence();

	for (int a = 0; a < MAXSYSTEXT; a++)
		_sysText[a] = getNextSentence();

	dataNl.close();
}

void TrecisionEngine::checkSystem() {
	_animMgr->refreshAllAnimations();
	eventLoop();
}

byte *TrecisionEngine::readData(const Common::String &fileName) {
	Common::SeekableReadStream *stream = _dataFile.createReadStreamForMember(fileName);
	if (stream == nullptr)
		error("readData(): File %s not found", fileName.c_str());

	byte *buf = new byte[stream->size()];
	stream->read(buf, stream->size());
	delete stream;

	return buf;
}

uint16 *TrecisionEngine::readData16(const Common::String &fileName, int &size) {
	Common::SeekableReadStream *stream = _dataFile.createReadStreamForMember(fileName);
	if (stream == nullptr)
		error("readData16(): File %s not found", fileName.c_str());

	size = ceil(stream->size() / 2.0);
	uint16 *buf = new uint16[size];
	for (int i = 0; i < size; ++i)
		buf[i] = stream->readUint16LE();
	delete stream;

	return buf;
}

void TrecisionEngine::read3D(const Common::String &filename) {

	Common::SeekableReadStream *ff = _dataFile.createReadStreamForMember(filename);
	if (ff == nullptr)
		error("read3D: Can't open 3D file %s", filename.c_str());

	_actor->read3D(ff);
	_pathFind->read3D(ff);

	delete ff;

	_cx = 320;
	_cy = 240;

	_pathFind->initSortPan();

	_renderer->init3DRoom(_graphicsMgr->getScreenBufferPtr());
	_renderer->setClipping(0, TOP, MAXX, AREA + TOP);
}

void TrecisionEngine::loadSaveSlots(Common::StringArray &saveNames) {
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (uint i = 0; i < _inventory.size(); ++i) {
		Common::String saveFileName = getSaveStateName(i + 1);
		Common::InSaveFile *saveFile = saveFileMan->openForLoading(saveFileName);
		ExtendedSavegameHeader header;

		if (!saveFile) {
			saveNames.push_back(_sysText[kMessageEmptySpot]);
			_inventory[i] = iEMPTYSLOT;
			continue;
		}

		const byte version = saveFile->readByte();

		if (saveFile && version == SAVE_VERSION_ORIGINAL) {
			// Original saved game, convert
			char buf[40];
			saveFile->read(buf, 40);
			buf[39] = '\0';
			saveNames.push_back(buf);

			uint16 *thumbnailBuf = _icons + (READICON + 1 + i) * ICONDX * ICONDY;
			saveFile->read((void *)thumbnailBuf, ICONDX * ICONDY * sizeof(uint16));
			_graphicsMgr->updatePixelFormat(thumbnailBuf, ICONDX * ICONDY);

			_inventory[i] = LASTICON + i;
		} else if (saveFile && version == SAVE_VERSION_SCUMMVM) {
			const bool headerRead = MetaEngine::readSavegameHeader(saveFile, &header, false);
			if (headerRead) {
				saveNames.push_back(header.description);

				Graphics::Surface *thumbnail = convertScummVMThumbnail(header.thumbnail);
				uint16 *thumbnailBuf = _icons + (READICON + 1 + i) * ICONDX * ICONDY;
				memcpy(thumbnailBuf, thumbnail->getPixels(), ICONDX * ICONDY * 2);
				thumbnail->free();
				delete thumbnail;

				_inventory[i] = LASTICON + i;
			} else {
				saveNames.push_back(_sysText[kMessageEmptySpot]);
				_inventory[i] = iEMPTYSLOT;
			}
		} else {
			saveNames.push_back(_sysText[kMessageEmptySpot]);
			_inventory[i] = iEMPTYSLOT;
		}

		delete saveFile;
	}

	refreshInventory(0, 0);
}

bool TrecisionEngine::dataSave() {
	const Common::Array<byte> savedInventory = _inventory;
	const uint8 savedIconBase = _iconBase;
	Common::StringArray saveNames;
	saveNames.reserve(MAXSAVEFILE);
	uint16 posx, LenText;
	bool ret = true;

	_actor->actorStop();
	_pathFind->nextStep();

	if (!ConfMan.getBool("originalsaveload")) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		int saveSlot = dialog->runModalWithCurrentTarget();
		Common::String saveName = dialog->getResultString();
		bool skipSave = saveSlot == -1;
		delete dialog;

		// Remove the mouse click event from the save/load dialog
		eventLoop();
		_mouseLeftBtn = _mouseRightBtn = false;

		if (!skipSave)
			saveGameState(saveSlot, saveName);

		return skipSave;
	}

	_graphicsMgr->clearScreenBufferTop();

	SDText SText;
	SText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageSavePosition]
	);
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_graphicsMgr->clearScreenBufferInventoryFull();
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_gameQueue.initQueue();
	_animQueue.initQueue();
	_characterQueue.initQueue();

	freeKey();

	// Reset the inventory and turn it into save slots
	_inventory.clear();
	_iconBase = 0;

insave:

	int8 CurPos = -1;
	int8 OldPos = -1;
	bool skipSave = false;

	loadSaveSlots(saveNames);

	for (;;) {
		checkSystem();
		getKey();

		int16 mx = _mousePos.x;
		int16 my = _mousePos.y;

		if (my >= FIRSTLINE &&
			my < FIRSTLINE + ICONDY &&
			mx >= ICONMARGSX &&
			mx < MAXX - ICONMARGDX) {
			OldPos = CurPos;
			CurPos = ((mx - ICONMARGSX) / ICONDX);

			if (OldPos != CurPos) {
				_graphicsMgr->clearScreenBufferInventoryDescriptions();

				posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				LenText = textLength(saveNames[CurPos]);

				posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
				SText.set(
					Common::Rect(posx, FIRSTLINE + ICONDY + 10, LenText + posx, CARHEI + (FIRSTLINE + ICONDY + 10)),
					Common::Rect(0, 0, LenText, CARHEI),
					MOUSECOL,
					MASKCOL,
					saveNames[CurPos].c_str()
				);
				SText.DText();

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn)
				break;
		} else {
			if (OldPos != -1) {
				_graphicsMgr->clearScreenBufferInventoryDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if (_mouseLeftBtn || _mouseRightBtn) {
				skipSave = true;
				break;
			}
		}
	}

	if (!skipSave) {
		if (_inventory[CurPos] == iEMPTYSLOT) {
			saveNames[CurPos].clear();

			_graphicsMgr->clearScreenBufferInventoryDescriptions();
			_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (;;) {
			_keybInput = true;
			checkSystem();
			char ch = getKey();
			freeKey();

			_keybInput = false;

			if (ch == 0x1B) {
				ch = 0;
				_graphicsMgr->clearScreenBufferInventoryDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);

				goto insave;
			}

			if (ch == 8)	// Backspace
				saveNames[CurPos].deleteLastChar();
			else if (ch == 13)	// Enter
				break;
			else if (saveNames[CurPos].size() < 39 && Common::isPrint(ch))
				saveNames[CurPos] += ch;

			_graphicsMgr->clearScreenBufferInventoryDescriptions();

			saveNames[CurPos] += '_';	// add blinking cursor

			posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
			LenText = textLength(saveNames[CurPos]);

			posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
			SText.set(
				Common::Rect(posx, FIRSTLINE + ICONDY + 10, LenText + posx, CARHEI + (FIRSTLINE + ICONDY + 10)),
				Common::Rect(0, 0, LenText, CARHEI),
				MOUSECOL,
				MASKCOL,
				saveNames[CurPos].c_str()
			);

			if ((readTime() / 8) & 1)
				_blinkLastDTextChar = 0x0000;

			SText.DText();
			_blinkLastDTextChar = MASKCOL;

			saveNames[CurPos].deleteLastChar();	// remove blinking cursor

			_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		_graphicsMgr->clearScreenBufferInventoryFull();

		ret = false;

		// Restore the inventory
		_inventory = savedInventory;
		_curInventory = 0;
		_iconBase = savedIconBase;

		saveGameState(CurPos + 1, saveNames[CurPos]);
	}

	_graphicsMgr->clearScreenBufferInventoryFull();
	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	_graphicsMgr->clearScreenBufferTopDescription();
	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	// Restore the inventory
	_inventory = savedInventory;
	_curInventory = 0;
	_iconBase = savedIconBase;

	return ret;
}

bool TrecisionEngine::dataLoad() {
	const Common::Array<byte> savedInventory = _inventory;
	const uint8 savedIconBase = _iconBase;	
	Common::StringArray saveNames;
	saveNames.reserve(MAXSAVEFILE);
	bool retval = true;

	if (!ConfMan.getBool("originalsaveload")) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
		int saveSlot = dialog->runModalWithCurrentTarget();
		bool skipLoad = saveSlot == -1;
		delete dialog;

		// Remove the mouse click event from the save/load dialog
		eventLoop();
		_mouseLeftBtn = _mouseRightBtn = false;

		performLoad(saveSlot - 1, skipLoad);

		return !skipLoad;
	}

	_graphicsMgr->clearScreenBufferTop();

	_graphicsMgr->showCursor();

	SDText SText;
	SText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageLoadPosition]
	);
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_graphicsMgr->clearScreenBufferInventoryFull();
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_gameQueue.initQueue();
	_animQueue.initQueue();
	_characterQueue.initQueue();

	freeKey();

	// Reset the inventory and turn it into save slots
	_inventory.clear();
	_iconBase = 0;

	loadSaveSlots(saveNames);

	bool skipLoad = false;
	int8 CurPos = -1;
	int8 OldPos = -1;

	for (;;) {
		checkSystem();
		getKey();

		if (_mousePos.y >= FIRSTLINE &&
			_mousePos.y < (FIRSTLINE + ICONDY) &&
			_mousePos.x >= ICONMARGSX &&
			(_mousePos.x < (MAXX - ICONMARGDX))) {
			OldPos = CurPos;
			CurPos = (_mousePos.x - ICONMARGSX) / ICONDX;

			if (OldPos != CurPos) {
				_graphicsMgr->clearScreenBufferInventoryDescriptions();

				uint16 posX = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				uint16 lenText = textLength(saveNames[CurPos]);
				if (posX - (lenText / 2) < 2)
					posX = 2;
				else
					posX = posX - (lenText / 2);
				if (posX + lenText > MAXX - 2)
					posX = MAXX - 2 - lenText;

				SText.set(
					Common::Rect(posX, FIRSTLINE + ICONDY + 10, lenText + posX, CARHEI + (FIRSTLINE + ICONDY + 10)),
					Common::Rect(0, 0, lenText, CARHEI),
					MOUSECOL,
					MASKCOL,
					saveNames[CurPos].c_str()
				);
				SText.DText();

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn && (_inventory[CurPos] != iEMPTYSLOT))
				break;
		} else {
			if (OldPos != -1) {
				_graphicsMgr->clearScreenBufferInventoryDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if (_mouseLeftBtn || _mouseRightBtn) {
				retval = false;
				skipLoad = true;
				break;
			}
		}
	}

	performLoad(CurPos, skipLoad);

	if (skipLoad) {
		// Restore the inventory
		_inventory = savedInventory;
		_curInventory = 0;
		_iconBase = savedIconBase;
	}

	return retval;
}

void TrecisionEngine::performLoad(int slot, bool skipLoad) {
	if (!skipLoad) {
		_graphicsMgr->clearScreenBufferInventoryFull();

		loadGameState(slot + 1);

		_flagNoPaintScreen = true;
		_curStack = 0;
		_flagscriptactive = false;

		_oldRoom = _curRoom;
		_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _curRoom, 0, 0, 0);
	}

	_actor->actorStop();
	_pathFind->nextStep();
	checkSystem();

	_graphicsMgr->clearScreenBufferInventoryFull();
	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	_graphicsMgr->clearScreenBufferTopDescription();
	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	if (_flagscriptactive) {
		_graphicsMgr->hideCursor();
	}
}

void TrecisionEngine::startCharacterAction(uint16 Act, uint16 NewRoom, uint8 NewPos, uint16 sent) {
	_characterQueue.initQueue();

	_flagInventoryLocked = false;
	if (Act > hLAST) {
		_animMgr->startSmkAnim(Act);
		_animTypeMgr->init(Act, _curObj);
		_graphicsMgr->hideCursor();
		_flagShowCharacter = false;
		_scheduler->doEvent(MC_CHARACTER, ME_CHARACTERCONTINUEACTION, MP_DEFAULT, Act, NewRoom, NewPos, _curObj);
	} else {
		if ((Act == aWALKIN) || (Act == aWALKOUT))
			_curObj = 0;
		_graphicsMgr->hideCursor();
		_actor->actorDoAction(Act);
		_pathFind->nextStep();
	}

	if (sent)
		_textMgr->characterSayInAction(sent);
	else
		_textMgr->clearLastText();
}

bool TrecisionEngine::canPlayerInteract() {
	return (!_flagSomeoneSpeaks &&
			!_flagscriptactive &&
			!_flagDialogActive &&
			!_flagDialogMenuActive &&
			(_actor->_curAction < hWALKIN) &&
			!_flagUseWithStarted &&
			_flagShowCharacter &&
			!_animMgr->_playingAnims[kSmackerAction]);
}

void TrecisionEngine::addDirtyRect(Common::Rect rect) {
	rect.translate(0, TOP);
	_dirtyRects.push_back(rect);
}

void TrecisionEngine::setObjectVisible(uint16 objectId, bool visible) {
	if (visible)
		_obj[objectId]._mode |= OBJMODE_OBJSTATUS;
	else
		_obj[objectId]._mode &= ~OBJMODE_OBJSTATUS;

	if (_obj[objectId]._mode & (OBJMODE_MASK | OBJMODE_FULL)) {
		SSortTable entry;
		entry._objectId = objectId;
		entry._remove = !isObjectVisible(objectId);
		_sortTable.push_back(entry);
	}
}

bool TrecisionEngine::isObjectVisible(uint16 objectId) const {
	return _obj[objectId]._mode & OBJMODE_OBJSTATUS;
}

void TrecisionEngine::setObjectAnim(uint16 objectId, uint16 animId) {
	_obj[objectId]._anim = animId;
}

void TrecisionEngine::reEvent() {
	_scheduler->doEvent(_curMessage->_class, _curMessage->_event, _curMessage->_priority, _curMessage->_u16Param1, _curMessage->_u16Param2, _curMessage->_u8Param, _curMessage->_u32Param);
}

void TrecisionEngine::ReadLoc() {
	if (_curRoom == kRoom11 && !(_room[kRoom11]._flag & kObjFlagDone))
		_flagShowCharacter = true;

	_soundMgr->fadeOut();

	_graphicsMgr->clearScreenBufferTop();

	Common::String filename = Common::String::format("%s.cr", _room[_curRoom]._baseName);
	Common::SeekableReadStream *picFile = _dataFile.createReadStreamForCompressedMember(filename);

	SObject bgInfo;
	bgInfo.readRect(picFile);

	_graphicsMgr->loadBackground(picFile, bgInfo._rect.width(), bgInfo._rect.height());
	_sortTable.clear();
	readObj(picFile);

	_soundMgr->stopAll();

	if (_room[_curRoom]._sounds[0] != 0)
		_soundMgr->loadRoomSounds();

	Common::String fname = Common::String::format("%s.3d", _room[_curRoom]._baseName);
	read3D(fname);

	_graphicsMgr->resetScreenBuffer();

	if (_room[_curRoom]._bkgAnim) {
		_animMgr->startSmkAnim(_room[_curRoom]._bkgAnim);
	} else
		_animMgr->smkStop(kSmackerBackground);

	_animTypeMgr->init(_room[_curRoom]._bkgAnim, 0);
}

void TrecisionEngine::TendIn() {
	_textStatus = TEXT_OFF;

	if (_curRoom == kRoomIntro) {
		_dialogMgr->playDialog(dFLOG);
		return;
	}

	_flagPaintCharacter = true;
	_soundMgr->waitEndFading();
	_renderer->paintScreen(true);

	_graphicsMgr->copyToScreen(0, 0, MAXX, MAXY);
}

void TrecisionEngine::readObject(Common::SeekableReadStream *stream, uint16 objIndex, uint16 objectId) {
	SObject *obj = &_obj[objectId];

	if (obj->_mode & OBJMODE_FULL) {
		obj->readRect(stream);

		uint32 size = obj->_rect.width() * obj->_rect.height();
		delete[] _objPointers[objIndex];
		_objPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			_objPointers[objIndex][i] = stream->readUint16LE();

		_graphicsMgr->updatePixelFormat(_objPointers[objIndex], size);
	}

	if (obj->_mode & OBJMODE_MASK) {
		obj->readRect(stream);

		uint32 size = stream->readUint32LE();
		delete[] _objPointers[objIndex];
		_objPointers[objIndex] = new uint16[size];
		for (uint32 i = 0; i < size; ++i)
			_objPointers[objIndex][i] = stream->readUint16LE();

		_graphicsMgr->updatePixelFormat(_objPointers[objIndex], size);

		size = stream->readUint32LE();
		delete[] _maskPointers[objIndex];
		_maskPointers[objIndex] = new uint8[size];
		for (uint32 i = 0; i < size; ++i)
			_maskPointers[objIndex][i] = (uint8)stream->readByte();
	}

	if (obj->_mode & (OBJMODE_MASK | OBJMODE_FULL)) {
		SSortTable entry;
		entry._objectId = objectId;
		entry._remove = !isObjectVisible(objectId);
		_sortTable.push_back(entry);
	}
}

void TrecisionEngine::readObj(Common::SeekableReadStream *stream) {
	if (!_room[_curRoom]._object[0])
		return;

	for (uint16 objIndex = 0; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = _room[_curRoom]._object[objIndex];
		if (!objectId)
			break;

		if (_curRoom == kRoom41D && objIndex == PATCHOBJ_ROOM41D)
			break;

		if (_curRoom == kRoom2C && objIndex == PATCHOBJ_ROOM2C)
			break;

		readObject(stream, objIndex, objectId);
	}
}

void TrecisionEngine::readExtraObj2C() {
	if (!_room[_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = _dataFile.createReadStreamForMember("2c2.bm");

	for (uint16 objIndex = PATCHOBJ_ROOM2C; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = _room[_curRoom]._object[objIndex];
		if (!objectId)
			break;

		readObject(ff, objIndex, objectId);
	}

	delete ff;
}

void TrecisionEngine::readExtraObj41D() {
	if (!_room[_curRoom]._object[32])
		return;

	Common::SeekableReadStream *ff = _dataFile.createReadStreamForMember("41d2.bm");
	for (uint16 objIndex = PATCHOBJ_ROOM41D; objIndex < MAXOBJINROOM; objIndex++) {
		const uint16 objectId = _room[_curRoom]._object[objIndex];
		if (!objectId)
			break;

		readObject(ff, objIndex, objectId);
	}
	delete ff;
}

void TrecisionEngine::RedrawRoom() {
	const uint16 curDialog = _dialogMgr->_curDialog;
	const uint16 curChoice = _dialogMgr->_curChoice;
	const uint16 bgAnim = _room[_curRoom]._bkgAnim;
	const ElevatorAction elevatorActions[6] = {
		{dASCENSORE12, 3, a129PARLACOMPUTERESCENDE, kRoom13},
		{dASCENSORE12, 4, a129PARLACOMPUTERESCENDE, kRoom16},
		{dASCENSORE13, 17, a139CHIUDONOPORTESU, kRoom12},
		{dASCENSORE13, 18, a1316CHIUDONOPORTEGIU, kRoom16},
		{dASCENSORE16, 32, a1616SALECONASCENSORE, kRoom12},
		{dASCENSORE16, 33, a1616SALECONASCENSORE, kRoom13},
	};

	_flagShowCharacter = _dialogMgr->showCharacterAfterDialog();

	for (int i = 0; i < 6; ++i) {
		if (curDialog == elevatorActions[i].dialog && curChoice == elevatorActions[i].choice) {
			startCharacterAction(elevatorActions[i].action, elevatorActions[i].newRoom, 20, 0);
			break;
		}
	}

	_sortTable.clear();
	_graphicsMgr->resetScreenBuffer();

	if (bgAnim)
		_animMgr->startSmkAnim(bgAnim);

	if (_curRoom == kRoom4P && curDialog == dF4PI)
		_animMgr->smkGoto(kSmackerBackground, 21);

	_textStatus = TEXT_OFF;
	_flagPaintCharacter = true;
	_renderer->paintScreen(true);
	_graphicsMgr->copyToScreen(0, 0, 640, 480);
}

} // End of namespace Trecision
