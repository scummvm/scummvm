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

#include "trecision/trecision.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/sysdef.h"
#include "trecision/console.h"
#include "trecision/graphics.h"
#include "trecision/video.h"
#include "trecision/logic.h"
#include "trecision/sound.h"

#include "engines/util.h"
#include "common/scummsys.h"
#include "common/error.h"
#include "common/system.h"
#include "common/events.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "graphics/cursorman.h"

#include "common/savefile.h"
#include "common/str.h"
#include "common/translation.h"
#include "gui/saveload.h"

namespace Common {
class File;
}

namespace Trecision {

TrecisionEngine *g_vm;

TrecisionEngine::TrecisionEngine(OSystem *syst) : Engine(syst) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "AUTORUN");
	SearchMan.addSubDirectoryMatching(gameDataDir, "DATA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "FMV");

	g_vm = nullptr;

	_curRoom = 0;
	_oldRoom = 0;

	_curInventory = 0;
	_curSortTableNum = 0;

	for (int i = 0; i < 10; ++i)
		_curScriptFrame[i] = 0;

	// Inventory
	for (int i = 0; i < MAXICON; ++i) {
		_inventory[i] = 0;
		_cyberInventory[i] = 0;
	}

	_inventorySize = 0;
	_cyberInventorySize = 0;
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
	_fastWalkLocked = false;

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

	_screenBuffer = nullptr;
	_animMgr = nullptr;
	_graphicsMgr = nullptr;
	_logicMgr = nullptr;
	_soundMgr = nullptr;

	for (int i = 0; i < 50; ++i) {
		_limits[i].left = _limits[i].top = _limits[i].right = _limits[i].bottom = 0;
	}

	_limitsNum = 0;
	_actorLimit = 0;
	_nextRefresh = 0;

	_curKey = Common::KEYCODE_INVALID;
	_curAscii = 0;
	_mouseX = _mouseY = 0;
	_mouseLeftBtn = _mouseRightBtn = false;
	_keybInput = false;

	_gamePaused = false;

	_closeUpObj = 0;
	_textPtr = nullptr;
	_lastInv = 0;
	_lastObj = 0;

	_curStack = 0;
	_textStackTop = -1;
	
	_flagscriptactive = false;
	_flagScreenRefreshed = false;

	_font = nullptr;
	_arrows = nullptr;
	_textureArea = nullptr;
	_icons = nullptr;
	_zBuffer = nullptr;
	_actor = nullptr;

	_flagDialogActive = false;
	_flagDialogMenuActive = false;
	_flagSkipTalk = false;
	_flagSkipEnable = false;
	_flagPaintCharacter = false;
	_flagShowCharacter = true;
	_flagSomeoneSpeaks = false;
	_flagCharacterSpeak = false;
	_flagUseWithStarted = false;
	_flagCharacterExists = true;
	_flagNoPaintScreen = false;
	_flagWaitRegen = false;

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		ObjPointers[i] = nullptr;
		MaskPointers[i] = nullptr;
	}

	BlinkLastDTextChar = MASKCOL;
}

TrecisionEngine::~TrecisionEngine() {
	if (_animMgr)
		_animMgr->stopAllSmkAnims();

	_dataFile.close();
	_speechFile.close();
	_soundMgr->stopSoundSystem();

	delete _extraRoomObject;
	delete _animMgr;
	delete _graphicsMgr;
	delete _logicMgr;
	delete _soundMgr;
	
	delete[] _font;
	delete[] _arrows;
	delete[] _textureArea;
	delete[] _icons;
	delete[] _zBuffer;
	delete _actor;
	delete[] TextArea;
	delete[] _screenBuffer;

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		delete[] ObjPointers[i];
		delete[] MaskPointers[i];
	}
}

Common::Error TrecisionEngine::run() {
	g_vm = this;

	syncSoundSettings();
	
	_graphicsMgr = new GraphicsManager(this);
	if (!_graphicsMgr->initScreen())
		return Common::kUnsupportedColorMode;
	_animMgr = new AnimManager(this);
	_logicMgr = new LogicManager(this);
	_soundMgr = new SoundManager(this);
	setDebugger(new Console(this));
	
	initMain();
	initCursor();

	// Check if a saved game is to be loaded from the launcher
	if (ConfMan.hasKey("save_slot"))
		loadGameState(ConfMan.getInt("save_slot"));

	while (!shouldQuit()) {
		eventLoop();
		if (!_flagNoPaintScreen)
			ProcessTime();

		ProcessMouse();
		Scheduler();

		if (_curMessage->_class == MC_SYSTEM && _curMessage->_event == ME_QUIT)
			break;

		AtFrameHandler(kAnimTypeBackground);

		ProcessTheMessage();

		if (_flagscriptactive)
			evalScript();
	}

	return Common::kNoError;
}

void TrecisionEngine::eventLoop() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
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

		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_CAPSLOCK) {
				if (!_fastWalkLocked)
					_fastWalk ^= true;
				_fastWalkLocked = true;
			}
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
				_fastWalkLocked = false;
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
	return (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
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
	bool unused = true;

	if (ser.isLoading()) {
		ser.skip(40, SAVE_VERSION_ORIGINAL, SAVE_VERSION_ORIGINAL);	// description
		ser.skip(ICONDX * ICONDY * sizeof(uint16), SAVE_VERSION_ORIGINAL, SAVE_VERSION_ORIGINAL); // thumbnail
	}

	ser.syncAsUint16LE(_curRoom);
	ser.syncAsByte(_inventorySize);
	ser.syncAsByte(_cyberInventorySize);
	ser.syncAsByte(_iconBase);
	ser.syncAsSint16LE(_flagSkipTalk);
	ser.syncAsSint16LE(_flagSkipEnable);
	ser.syncAsSint16LE(unused);	// _flagMouseEnabled
	ser.syncAsSint16LE(_flagScreenRefreshed);
	ser.syncAsSint16LE(_flagPaintCharacter);
	ser.syncAsSint16LE(_flagSomeoneSpeaks);
	ser.syncAsSint16LE(_flagCharacterSpeak);
	ser.syncAsSint16LE(_flagInventoryLocked);
	ser.syncAsSint16LE(_flagUseWithStarted);
	ser.syncAsSint16LE(unused);	// FlagMousePolling
	ser.syncAsSint16LE(unused); // FlagDialogSolitaire
	ser.syncAsSint16LE(_flagCharacterExists);
	ser.syncBytes(_inventory, MAXICON);
	ser.syncBytes(_cyberInventory, MAXICON);
	ser.syncAsFloatLE(_actor->_px);
	ser.syncAsFloatLE(_actor->_py);
	ser.syncAsFloatLE(_actor->_pz);
	ser.syncAsFloatLE(_actor->_dx);
	ser.syncAsFloatLE(_actor->_dz);
	ser.syncAsFloatLE(_actor->_theta);
	ser.syncAsSint32LE(_curPanel);
	ser.syncAsSint32LE(_oldPanel);

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

	for (int a = 0; a < MAXCHOICE; a++) {
		DialogChoice *cur = &_choice[a];
		ser.syncAsUint16LE(cur->_flag);
		ser.syncAsUint16LE(cur->_sentenceIndex);
		ser.syncAsUint16LE(cur->_firstSubTitle);
		ser.syncAsUint16LE(cur->_subTitleNumb);
		for (int i = 0; i < MAXDISPCHOICES; i++)
			ser.syncAsUint16LE(cur->_on[i]);
		for (int i = 0; i < MAXDISPCHOICES; i++)
			ser.syncAsUint16LE(cur->_off[i]);
		ser.syncAsUint16LE(cur->_startFrame);
		ser.syncAsUint16LE(cur->_nextDialog);
	}

	for (int a = 0; a < MAXDIALOG; a++) {
		Dialog *cur = &_dialog[a];
		ser.syncAsUint16LE(cur->_flag);
		ser.syncAsUint16LE(cur->_interlocutor);
		ser.syncBytes((byte *)cur->_startAnim, 14);
		ser.syncAsUint16LE(cur->_startLen);
		ser.syncAsUint16LE(cur->_firstChoice);
		ser.syncAsUint16LE(cur->_choiceNumb);
		for (int i = 0; i < MAXNEWSMKPAL; i++)
			ser.syncAsUint16LE(cur->_newPal[i]);
	}

	_logicMgr->syncGameStream(ser);

	return true;
}

void TrecisionEngine::initMain() {
	for (int c = 0; c < MAXOBJ; c++)
		_obj[c]._position = -1;

	initNames();
	_logicMgr->initScript();
	openDataFiles();
	openSys();

	LoadAll();

	initMessageSystem();
	_logicMgr->initInventory();

	_curRoom = kRoomIntro;

	ProcessTime();

	doEvent(MC_SYSTEM, ME_START, MP_DEFAULT, 0, 0, 0, 0);
}

void TrecisionEngine::openDataFiles() {
	if (!_dataFile.open("nldata.cd0"))
		error("openDataFiles() - Error opening nldata.cd0");

	if (!Common::File::exists("nlanim.cd1") ||
		!Common::File::exists("nlanim.cd2") ||
		!Common::File::exists("nlanim.cd3")
		)
		error("openDataFiles() - nlanim.cd1 or nlanim.cd2 or nlanim.cd3 is missing");

	if (!_speechFile.open("nlspeech.cd0"))
		warning("openDataFiles() - nlspeech.cd0 is missing - skipping");

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

void TrecisionEngine::LoadAll() {
	Common::File dataNl;
	if (!dataNl.open("DATA.NL"))
		error("LoadAll : Couldn't open DATA.NL");

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
		_obj[i]._dx = dataNl.readUint16LE();
		_obj[i]._dy = dataNl.readUint16LE();
		_obj[i]._px = dataNl.readUint16LE();
		_obj[i]._py = dataNl.readUint16LE();

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

	for (int i = 0; i < MAXDIALOG; ++i) {
		_dialog[i]._flag = dataNl.readUint16LE();
		_dialog[i]._interlocutor = dataNl.readUint16LE();

		dataNl.read(&_dialog[i]._startAnim, ARRAYSIZE(_dialog[i]._startAnim));

		_dialog[i]._startLen = dataNl.readUint16LE();
		_dialog[i]._firstChoice = dataNl.readUint16LE();
		_dialog[i]._choiceNumb = dataNl.readUint16LE();

		for (int j = 0; j < MAXNEWSMKPAL; ++j)
			_dialog[i]._newPal[j] = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXCHOICE; ++i) {
		_choice[i]._flag = dataNl.readUint16LE();
		_choice[i]._sentenceIndex = dataNl.readUint16LE();
		_choice[i]._firstSubTitle = dataNl.readUint16LE();
		_choice[i]._subTitleNumb = dataNl.readUint16LE();

		for (int j = 0; j < MAXDISPCHOICES; ++j)
			_choice[i]._on[j] = dataNl.readUint16LE();

		for (int j = 0; j < MAXDISPCHOICES; ++j)
			_choice[i]._off[j] = dataNl.readUint16LE();

		_choice[i]._startFrame = dataNl.readUint16LE();
		_choice[i]._nextDialog = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXSUBTITLES; ++i) {
		_subTitles[i]._sentence = dataNl.readUint16LE();
		_subTitles[i]._x = dataNl.readUint16LE();
		_subTitles[i]._y = dataNl.readUint16LE();
		_subTitles[i]._color = dataNl.readUint16LE();
		_subTitles[i]._startFrame = dataNl.readUint16LE();
		_subTitles[i]._length = dataNl.readUint16LE();
	}

	for (int i = 0; i < MAXACTION; ++i)
		_actionLen[i] = dataNl.readByte();

	int numFileRef = dataNl.readSint32LE();
	dataNl.skip(numFileRef * (12 + 4));	// fileRef name + offset

	dataNl.read(TextArea, MAXTEXTAREA);

	_textPtr = (char *)TextArea;

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

void TrecisionEngine::initCursor() {
	const int cw = 21, ch = 21;
	const int cx = 10, cy = 10;
	uint16 cursor[cw * ch];
	memset(cursor, 0, ARRAYSIZE(cursor) * 2);

	const uint16 cursorColor = _graphicsMgr->palTo16bit(255, 255, 255);

	for (int i = 0; i < cw; i++) {
		if (i >= 8 && i <= 12 && i != 10)
			continue;
		cursor[cx * cw + i] = cursorColor;	// horizontal
		cursor[cx + cw * i] = cursorColor;	// vertical
	}

	Graphics::PixelFormat format = g_system->getScreenFormat();
	CursorMan.pushCursor(cursor, cw, ch, cx, cy, 0, false, &format);
}

void TrecisionEngine::showCursor() {
	CursorMan.showMouse(true);
}

void TrecisionEngine::hideCursor() {
	CursorMan.showMouse(false);
}

bool TrecisionEngine::isCursorVisible() {
	return CursorMan.isVisible();
}

byte *TrecisionEngine::readData(Common::String fileName) {
	Common::SeekableReadStream *stream = _dataFile.createReadStreamForMember(fileName);
	if (stream == nullptr)
		error("readData(): File %s not found", fileName.c_str());

	byte *buf = new byte[stream->size()];
	stream->read(buf, stream->size());
	delete stream;
	
	return buf;
}

uint16 *TrecisionEngine::readData16(Common::String fileName, int &size) {
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

void TrecisionEngine::loadSaveSlots(Common::StringArray &saveNames) {
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (int i = 0; i < _inventorySize; i++) {
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
		}
		else if (saveFile && version == SAVE_VERSION_SCUMMVM) {
			const bool headerRead = MetaEngine::readSavegameHeader(saveFile, &header, false);
			if (headerRead) {
				saveNames.push_back(header.description);

				Graphics::Surface *thumbnail = convertScummVMThumbnail(header.thumbnail);
				uint16 *thumbnailBuf = _icons + (READICON + 1 + i) * ICONDX * ICONDY;
				memcpy(thumbnailBuf, thumbnail->getPixels(), ICONDX * ICONDY * 2);
				thumbnail->free();
				delete thumbnail;

				_inventory[i] = LASTICON + i;
			}
			else {
				saveNames.push_back(_sysText[kMessageEmptySpot]);
				_inventory[i] = iEMPTYSLOT;
			}
		}
		else {
			saveNames.push_back(_sysText[kMessageEmptySpot]);
			_inventory[i] = iEMPTYSLOT;
		}

		delete saveFile;
	}

	refreshInventory(0, 0);
}

bool TrecisionEngine::DataSave() {
	uint8 OldInv[MAXICON], OldIconBase, OldInvLen;
	char ch;
	Common::StringArray saveNames;
	saveNames.reserve(MAXSAVEFILE);
	uint16 posx, LenText;
	bool ret = true;

	actorStop();
	nextStep();

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

		return !skipSave;
	}

	for (int a = 0; a < TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	SDText SText;
	SText.set(0, TOP - 20, MAXX, CARHEI, 0, 0, MAXX, CARHEI, MOUSECOL, MASKCOL, _sysText[kMessageSavePosition]);
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	for (int a = TOP + AREA; a < AREA + 2 * TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_gameQueue.initQueue();
	_animQueue.initQueue();
	_characterQueue.initQueue();

	FreeKey();

	// Reset the inventory and turn it into save slots
	memcpy(OldInv, _inventory, MAXICON);
	memset(_inventory, 0, MAXICON);
	OldIconBase = _iconBase;
	_iconBase = 0;
	OldInvLen = _inventorySize;
	_inventorySize = MAXSAVEFILE;

insave:

	int8 CurPos = -1;
	int8 OldPos = -1;
	bool skipSave = false;
	ch = 0;

	loadSaveSlots(saveNames);

	for (;;) {
		checkSystem();
		GetKey();

		int16 mx = _mouseX;
		int16 my = _mouseY;

		if (my >= FIRSTLINE &&
			my < FIRSTLINE + ICONDY &&
			mx >= ICONMARGSX &&
			mx < MAXX - ICONMARGDX) {
			OldPos = CurPos;
			CurPos = ((mx - ICONMARGSX) / ICONDX);

			if (OldPos != CurPos) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

				posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				LenText = TextLength(saveNames[CurPos].c_str(), 0);

				posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
				SText.set(posx, FIRSTLINE + ICONDY + 10, LenText, CARHEI, 0, 0, LenText, CARHEI, MOUSECOL, MASKCOL, saveNames[CurPos].c_str());
				SText.DText();

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn)
				break;
		}
		else {
			if (OldPos != -1) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

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

			for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
				memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

			_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (;;) {
			_keybInput = true;
			checkSystem();
			ch = GetKey();
			FreeKey();

			_keybInput = false;

			if (ch == 0x1B) {
				ch = 0;
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);

				goto insave;
			}

			if (ch == 8)	// Backspace
				saveNames[CurPos].deleteLastChar();
			else if (ch == 13)	// Enter
				break;
			else if (saveNames[CurPos].size() < 39 && Common::isPrint(ch))
				saveNames[CurPos] += ch;

			for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
				memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

			saveNames[CurPos] += '_';	// add blinking cursor

			posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
			LenText = TextLength(saveNames[CurPos].c_str(), 0);

			posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
			SText.set(posx, FIRSTLINE + ICONDY + 10, LenText, CARHEI, 0, 0, LenText, CARHEI, MOUSECOL, MASKCOL, saveNames[CurPos].c_str());

			if ((ReadTime() / 8) & 1)
				BlinkLastDTextChar = 0x0000;

			SText.DText();
			BlinkLastDTextChar = MASKCOL;

			saveNames[CurPos].deleteLastChar();	// remove blinking cursor

			_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (int a = FIRSTLINE; a < MAXY; a++)
			memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

		ret = false;

		// Restore the inventory
		memcpy(_inventory, OldInv, MAXICON);
		_curInventory = 0;
		_iconBase = OldIconBase;
		_inventorySize = OldInvLen;

		saveGameState(CurPos + 1, saveNames[CurPos]);
	}

	for (int a = FIRSTLINE; a < MAXY; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	for (int a = TOP - 20; a < TOP - 20 + CARHEI; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	// Restore the inventory
	memcpy(_inventory, OldInv, MAXICON);
	_curInventory = 0;
	_iconBase = OldIconBase;
	_inventorySize = OldInvLen;

	return ret;
}

bool TrecisionEngine::DataLoad() {
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

	for (int a = 0; a < TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	showCursor();

	SDText SText;
	SText.set(0, TOP - 20, MAXX, CARHEI, 0, 0, MAXX, CARHEI, MOUSECOL, MASKCOL, _sysText[kMessageLoadPosition]);
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	for (int a = TOP + AREA; a < AREA + 2 * TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_gameQueue.initQueue();
	_animQueue.initQueue();
	_characterQueue.initQueue();

	FreeKey();

	uint8 OldInv[MAXICON];
	// Reset the inventory and turn it into save slots
	memcpy(OldInv, _inventory, MAXICON);
	memset(_inventory, 0, MAXICON);
	uint8 OldIconBase = _iconBase;
	_iconBase = 0;
	uint8 OldInvLen = _inventorySize;
	_inventorySize = MAXSAVEFILE;

	loadSaveSlots(saveNames);

	bool skipLoad = false;
	int8 CurPos = -1;
	int8 OldPos = -1;

	for (;;) {
		checkSystem();
		GetKey();

		if (_mouseY >= FIRSTLINE &&
			_mouseY < (FIRSTLINE + ICONDY) &&
			_mouseX >= ICONMARGSX &&
			(_mouseX < (MAXX - ICONMARGDX))) {
			OldPos = CurPos;
			CurPos = (_mouseX - ICONMARGSX) / ICONDX;

			if (OldPos != CurPos) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

				uint16 posX = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				uint16 lenText = TextLength(saveNames[CurPos].c_str(), 0);
				if (posX - (lenText / 2) < 2)
					posX = 2;
				else
					posX = posX - (lenText / 2);
				if ((posX + lenText) > MAXX - 2)
					posX = MAXX - 2 - lenText;

				SText.set(posX, FIRSTLINE + ICONDY + 10, lenText, CARHEI, 0, 0, lenText, CARHEI, MOUSECOL, MASKCOL, saveNames[CurPos].c_str());
				SText.DText();

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn && (_inventory[CurPos] != iEMPTYSLOT))
				break;
		}
		else {
			if (OldPos != -1) {
				for (int a = FIRSTLINE + ICONDY + 10; a < FIRSTLINE + ICONDY + 10 + CARHEI; a++)
					memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

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
		memcpy(_inventory, OldInv, MAXICON);
		_curInventory = 0;
		_iconBase = OldIconBase;
		_inventorySize = OldInvLen;
	}

	return retval;
}

void TrecisionEngine::performLoad(int slot, bool skipLoad) {
	if (!skipLoad) {
		for (int a = FIRSTLINE; a < MAXY; a++)
			memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

		loadGameState(slot + 1);

		_flagNoPaintScreen = true;
		_curStack = 0;
		_flagscriptactive = false;

		_oldRoom = _curRoom;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _curRoom, 0, 0, 0);
	}

	actorStop();
	nextStep();
	checkSystem();

	for (int a = FIRSTLINE; a < MAXY; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	for (int a = TOP - 20; a < TOP - 20 + CARHEI; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	if (_flagscriptactive) {
		hideCursor();
	}
}

SActor::SActor(TrecisionEngine *vm) : _vm(vm) {
	_vertex = nullptr;
	_face = nullptr;
	_light = nullptr;
	_camera = nullptr;
	_texture = nullptr;

	_vertexNum = 0;
	_faceNum = 0;
	_lightNum = 0;
	_matNum = 0;

	_px = _py = _pz = 0.0;
	_dx = _dz = 0.0;
	_theta = 0.0;

	for (int i = 0; i < 6; ++i)
		_lim[i] = 0;

	_curFrame = 0;
	_curAction = 0;

	for (int i = 0; i < 256; ++i) {
		for (int j = 0; j < 91; ++j)
			_textureMat[i][j] = 0;
	}

	for (int i = 0; i < MAXFACE; ++i) {
		for (int j = 0; j < 3; ++j) {
			_textureCoord[i][j][0] = 0;
			_textureCoord[i][j][1] = 0;
		}
	}

	_characterArea = nullptr;
}

SActor::~SActor() {
	delete[] _characterArea;
	delete[] _face;
//	delete _light;
//	delete _camera;
//	delete _texture;
}

} // End of namespace Trecision
