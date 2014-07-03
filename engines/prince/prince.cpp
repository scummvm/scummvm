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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/fs.h"
#include "common/keyboard.h"
#include "common/substream.h"
#include "common/str.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "audio/audiostream.h"

#include "prince/prince.h"
#include "prince/font.h"
#include "prince/graphics.h"
#include "prince/script.h"
#include "prince/debugger.h"
#include "prince/object.h"
#include "prince/mob.h"
#include "prince/sound.h"
#include "prince/variatxt.h"
#include "prince/flags.h"
#include "prince/font.h"
#include "prince/mhwanh.h"
#include "prince/cursor.h"
#include "prince/archive.h"
#include "prince/hero.h"
#include "prince/resource.h"
#include "prince/animation.h"
#include "prince/option_text.h"

namespace Prince {

void PrinceEngine::debugEngine(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	debug("Prince::Engine frame %08ld %s", _frameNr, buf);
}

PrinceEngine::PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc) : 
	Engine(syst), _gameDescription(gameDesc), _graph(nullptr), _script(nullptr), _interpreter(nullptr), _flags(nullptr),
	_locationNr(0), _debugger(nullptr), _midiPlayer(nullptr), _room(nullptr), testAnimNr(0), testAnimFrame(0),
	_frameNr(0), _cursor1(nullptr), _cursor2(nullptr), _cursor3(nullptr), _font(nullptr),
	_suitcaseBmp(nullptr), _roomBmp(nullptr), _cursorNr(0), _picWindowX(0), _picWindowY(0), _randomSource("prince"),
	_invLineX(134), _invLineY(176), _invLine(5), _invLines(3), _invLineW(70), _invLineH(76), _maxInvW(72), _maxInvH(76),
	_invLineSkipX(2), _invLineSkipY(3), _showInventoryFlag(false), _inventoryBackgroundRemember(false),
	_mst_shadow(0), _mst_shadow2(0), _candleCounter(0), _invX1(53), _invY1(18), _invWidth(536), _invHeight(438),
	_invCurInside(false), _optionsFlag(false), _optionEnabled(0), _invExamY(120), _invMaxCount(2), _invCounter(0),
	_optionsMob(-1), _currentPointerNumber(1), _selectedMob(-1), _selectedItem(0), _selectedMode(0),
	_optionsWidth(210), _optionsHeight(170), _invOptionsWidth(210), _invOptionsHeight(130), _optionsStep(20),
	_invOptionsStep(20), _optionsNumber(7), _invOptionsNumber(5), _optionsColor1(236), _optionsColor2(252),
	_dialogWidth(600), _dialogHeight(0), _dialogLineSpace(10), _dialogColor1(220), _dialogColor2(223),
	_dialogFlag(false), _dialogLines(0), _dialogText(nullptr) {

	// Debug/console setup
	DebugMan.addDebugChannel(DebugChannel::kScript, "script", "Prince Script debug channel");
	DebugMan.addDebugChannel(DebugChannel::kEngine, "engine", "Prince Engine debug channel");

	DebugMan.enableDebugChannel("script");

	memset(_voiceStream, 0, sizeof(_voiceStream));

	gDebugLevel = 10;
}

PrinceEngine::~PrinceEngine() {
	DebugMan.clearAllDebugChannels();

	delete _rnd;
	delete _debugger;
	delete _cursor1;
	delete _cursor3;
	delete _midiPlayer;
	delete _script;
	delete _flags;
	delete _interpreter;
	delete _font;
	delete _roomBmp;
	delete _suitcaseBmp;
	delete _variaTxt;
	delete[] _talkTxt;
	delete[] _invTxt;
	delete[] _dialogDat;
	delete _graph;
	delete _room;

	if (_cursor2 != nullptr) {
		_cursor2->free();
		delete _cursor2;
	}

	for (uint i = 0; i < _objList.size(); i++) {
		delete _objList[i];
	}
	_objList.clear();

	for (uint32 i = 0; i < _pscrList.size(); i++) {
		delete _pscrList[i];
	}
	_pscrList.clear();

	for (uint i = 0; i < _maskList.size(); i++) {
		free(_maskList[i]._data);
	}
	_maskList.clear();

	freeDrawNodes();

	clearBackAnimList();

	freeAllNormAnims();

	for (uint i = 0; i < _allInvList.size(); i++) {
		_allInvList[i]._surface->free();
		delete _allInvList[i]._surface;
	}
	_allInvList.clear();

	_optionsPic->free();
	delete _optionsPic;

	_optionsPicInInventory->free();
	delete _optionsPicInInventory;

	for (uint i = 0; i < _mainHero->_moveSet.size(); i++) {
		delete _mainHero->_moveSet[i];
	}

	for (uint i = 0; i < _secondHero->_moveSet.size(); i++) {
		delete _secondHero->_moveSet[i];
	}

	delete _mainHero;
	delete _secondHero;
}

GUI::Debugger *PrinceEngine::getDebugger() {
	return _debugger;
}

void PrinceEngine::init() {

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	
	debugEngine("Adding all path: %s", gameDataDir.getPath().c_str());

	PtcArchive *all = new PtcArchive();
	if (!all->open("all/databank.ptc")) 
		error("Can't open all/databank.ptc");

	PtcArchive *voices = new PtcArchive();
	if (!voices->open("data/voices/databank.ptc"))
		error("Can't open data/voices/databank.ptc");

	PtcArchive *sound = new PtcArchive();
	if (!sound->open("sound/databank.ptc"))
		error("Can't open sound/databank.ptc");

	SearchMan.addSubDirectoryMatching(gameDataDir, "all");

	SearchMan.add("all", all);
	SearchMan.add("voices", voices);
	SearchMan.add("sound", sound);

	_graph = new GraphicsMan(this);

	_rnd = new Common::RandomSource("prince");
	_debugger = new Debugger(this);

	_midiPlayer = new MusicPlayer(this);

	if (getLanguage() == Common::DE_DEU) {
		_font = new Font();
		Resource::loadResource(_font, "font3.raw", true);
	} else {
		_font = new Font();
		Resource::loadResource(_font, "font1.raw", true);
	}

	_suitcaseBmp = new MhwanhDecoder();
	Resource::loadResource(_suitcaseBmp, "walizka", true);

	_script = new Script(this);
	Resource::loadResource(_script, "skrypt.dat", true);

	_flags = new InterpreterFlags();
	_interpreter = new Interpreter(this, _script, _flags);

	_variaTxt = new VariaTxt();
	Resource::loadResource(_variaTxt, "variatxt.dat", true);
	
	_cursor1 = new Cursor();
	Resource::loadResource(_cursor1, "mouse1.cur", true);

	_cursor3 = new Cursor();
	Resource::loadResource(_cursor3, "mouse2.cur", true);

	Common::SeekableReadStream *talkTxtStream = SearchMan.createReadStreamForMember("talktxt.dat");
	if (!talkTxtStream) {
		error("Can't load talkTxtStream");
		return;
	}
	_talkTxtSize = talkTxtStream->size();
	_talkTxt = new byte[_talkTxtSize];
	talkTxtStream->read(_talkTxt, _talkTxtSize);

	delete talkTxtStream;

	Common::SeekableReadStream *invTxtStream = SearchMan.createReadStreamForMember("invtxt.dat");
	if (!invTxtStream) {
		error("Can't load invTxtStream");
		return;
	}
	_invTxtSize = invTxtStream->size();
	_invTxt = new byte[_invTxtSize];
	invTxtStream->read(_invTxt, _invTxtSize);

	delete invTxtStream;

	loadAllInv();

	Common::SeekableReadStream *dialogDatStream = SearchMan.createReadStreamForMember("dialog.dat");
	if (!dialogDatStream) {
		error("Can't load dialogDatStream");
		return;
	}
	_dialogDatSize = dialogDatStream->size();
	_dialogDat = new byte[_dialogDatSize];
	dialogDatStream->read(_dialogDat, _dialogDatSize);

	delete dialogDatStream;

	_optionsPic = new Graphics::Surface();
	_optionsPic->create(_optionsWidth, _optionsHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect picRect(0, 0, _optionsWidth, _optionsHeight);
	_optionsPic->fillRect(picRect, _graph->kShadowColor);

	_optionsPicInInventory = new Graphics::Surface();
	_optionsPicInInventory->create(_invOptionsWidth, _invOptionsHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect invPicRect(0, 0, _invOptionsWidth, _invOptionsHeight);
	_optionsPicInInventory->fillRect(invPicRect, _graph->kShadowColor);

	_roomBmp = new Image::BitmapDecoder();

	_room = new Room();

	_mainHero = new Hero(this, _graph);
	_secondHero = new Hero(this, _graph);

	_mainHero->loadAnimSet(1);
	_secondHero->loadAnimSet(3);

	Anim tempAnim;
	tempAnim._animData = nullptr;
	tempAnim._shadowData = nullptr;
	for (int i = 0; i < kMaxNormAnims; i++) {
		_normAnimList.push_back(tempAnim);
	}
}

void PrinceEngine::showLogo() {
	MhwanhDecoder logo;
	if (Resource::loadResource(&logo, "logo.raw", true)) {
		_graph->setPalette(logo.getPalette());
		_graph->draw(_graph->_frontScreen, logo.getSurface());
		_graph->update(_graph->_frontScreen);
		_system->delayMillis(700);
	}
}

Common::Error PrinceEngine::run() {

	init();

	showLogo();

	mainLoop();

	return Common::kNoError;
}

bool AnimListItem::loadFromStream(Common::SeekableReadStream &stream) {
	int32 pos = stream.pos();

	uint16 type = stream.readUint16LE();
	if (type == 0xFFFF) {
		return false;
	}
	_type = type;
	_fileNumber = stream.readUint16LE();
	_startPhase = stream.readUint16LE();
	_endPhase = stream.readUint16LE();
	_loopPhase = stream.readUint16LE();
	_x = stream.readSint16LE();
	_y = stream.readSint16LE();
	_loopType = stream.readUint16LE();
	_nextAnim = stream.readUint16LE();
	_flags = stream.readUint16LE();

	//debug("AnimListItem type %d, fileNumber %d, x %d, y %d, flags %d", _type, _fileNumber, _x, _y, _flags);
	//debug("startPhase %d, endPhase %d, loopPhase %d", _startPhase, _endPhase, _loopPhase);

	// 32 byte aligment
	stream.seek(pos + 32);

	return true;
}

bool PrinceEngine::loadLocation(uint16 locationNr) {
	_flicPlayer.close();

	memset(_textSlots, 0, sizeof(_textSlots));
	for(uint32 sampleId = 0; sampleId < MAX_SAMPLES; sampleId++) {
		stopSample(sampleId);
	}

	debugEngine("PrinceEngine::loadLocation %d", locationNr);
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.remove(Common::String::format("%02d", _locationNr));

	_locationNr = locationNr;
	_debugger->_locationNr = locationNr;

	_flags->setFlagValue(Flags::CURRROOM, _locationNr);
	_interpreter->stopBg();

	changeCursor(0);

	const Common::String locationNrStr = Common::String::format("%02d", _locationNr);
	debugEngine("loadLocation %s", locationNrStr.c_str());

	PtcArchive *locationArchive = new PtcArchive();
	if (!locationArchive->open(locationNrStr + "/databank.ptc"))
		error("Can't open location %s", locationNrStr.c_str());

	SearchMan.add(locationNrStr, locationArchive);

	const char *musName = MusicPlayer::_musTable[MusicPlayer::_musRoomTable[locationNr]];
	_midiPlayer->loadMidi(musName);

	// load location background, replace old one
	Resource::loadResource(_roomBmp, "room", true);
	if (_roomBmp->getSurface()) {
		_sceneWidth = _roomBmp->getSurface()->w;
		_graph->setPalette(_roomBmp->getPalette());
	}

	loadZoom(_mainHero->_zoomBitmap, _mainHero->kZoomBitmapLen, "zoom");
	loadShadow(_mainHero->_shadowBitmap, _mainHero->kShadowBitmapSize, "shadow", "shadow2");

	for (uint32 i = 0; i < _pscrList.size(); i++) {
		delete _pscrList[i];
	}
	_pscrList.clear();
	Resource::loadResource(_pscrList, "pscr.lst", false);

	_mobList.clear();
	if (getLanguage() == Common::DE_DEU) {
		const Common::String mobLstName = Common::String::format("mob%02d.lst", _locationNr);
		debug("name: %s", mobLstName.c_str());
		Resource::loadResource(_mobList, mobLstName.c_str(), false);
	} else {
		Resource::loadResource(_mobList, "mob.lst", false);
	}

	_animList.clear();
	Resource::loadResource(_animList, "anim.lst", false);

	for (uint32 i = 0; i < _objList.size(); i++) {
		delete _objList[i];
	}
	_objList.clear();
	Resource::loadResource(_objList, "obj.lst", false);

	_room->loadRoom(_script->getRoomOffset(_locationNr));

	for (uint i = 0; i < _maskList.size(); i++) {
		free(_maskList[i]._data);
	}
	_maskList.clear();
	_script->loadAllMasks(_maskList, _room->_nak);

	_picWindowX = 0;

	_mainHero->_lightX = _script->getLightX(_locationNr);
	_mainHero->_lightY = _script->getLightY(_locationNr);
	_mainHero->setShadowScale(_script->getShadowScale(_locationNr));

	for (uint i = 0; i < _mobList.size(); i++) {
		_mobList[i]._visible = _script->getMobVisible(i);
	}

	freeDrawNodes();

	clearBackAnimList();
	_script->installBackAnims(_backAnimList, _room->_backAnim);

	_graph->makeShadowTable(70, _graph->_shadowTable70);
	_graph->makeShadowTable(50, _graph->_shadowTable50);

	return true;
}

void PrinceEngine::changeCursor(uint16 curId) {
	_debugger->_cursorNr = curId;

	const Graphics::Surface *curSurface = nullptr;

	uint16 hotspotX = 0;
	uint16 hotspotY = 0;

	switch(curId) {
	case 0:
		CursorMan.showMouse(false);
		_optionsFlag = 0;
		_selectedMob = -1;
		return;
	case 1:
		curSurface = _cursor1->getSurface();
		break;
	case 2:
		curSurface = _cursor2;
		break;
	case 3:
		curSurface = _cursor3->getSurface();
		hotspotX = curSurface->w >> 1;
		hotspotY = curSurface->h >> 1;
		break;
	}

	CursorMan.replaceCursorPalette(_roomBmp->getPalette(), 0, 255);
	CursorMan.replaceCursor(
		curSurface->getBasePtr(0, 0), 
		curSurface->w, curSurface->h, 
		hotspotX, hotspotY, 
		255, false,
		&curSurface->format
	);
	CursorMan.showMouse(true);
}

void PrinceEngine::makeInvCursor(int itemNr) {
	const Graphics::Surface *cur1Surface = _cursor1->getSurface();
	int cur1W = cur1Surface->w;
	int cur1H = cur1Surface->h;
	const Common::Rect cur1Rect(0, 0, cur1W, cur1H);

	const Graphics::Surface *itemSurface = _allInvList[itemNr].getSurface();
	int itemW = itemSurface->w;
	int itemH = itemSurface->h;

	int cur2W = cur1W + itemW / 2;
	int cur2H = cur1H + itemH / 2;

	if (_cursor2 != nullptr) {
		_cursor2->free();
		delete _cursor2;
	}
	_cursor2 = new Graphics::Surface();
	_cursor2->create(cur2W, cur2H, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect cur2Rect(0, 0, cur2W, cur2H);
	_cursor2->fillRect(cur2Rect, 255);
	_cursor2->copyRectToSurface(*cur1Surface, 0, 0, cur1Rect);

	byte *src1 = (byte *)itemSurface->getBasePtr(0, 0);
	byte *dst1 = (byte *)_cursor2->getBasePtr(cur1W, cur1H);

	if (itemH % 2 != 0) {
		itemH--;
	}
	if (itemW % 2 != 0) {
		itemW--;
	}

	for (int y = 0; y < itemH; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		if (y % 2 == 0) {
			for (int x = 0; x < itemW; x++, src2++) {
				if (x % 2 == 0) {
					if (*src2 != 0) {
						*dst2 = *src2;
					} else {
						*dst2 = 255;
					}
					dst2++;
				}
			}
			dst1 += _cursor2->pitch;
		}
		src1 += itemSurface->pitch;
	}
}

bool PrinceEngine::playNextFrame() {
	if (!_flicPlayer.isVideoLoaded())
		return false;

	const Graphics::Surface *s = _flicPlayer.decodeNextFrame();
	if (s) {
		_graph->drawTransparentSurface(_graph->_frontScreen, 0, 0, s, 255);
		_graph->change();
	} else if (_flicLooped) {
		_flicPlayer.rewind();
		playNextFrame();
	}

	return true;
}

void PrinceEngine::playSample(uint16 sampleId, uint16 loopType) {
	if (_voiceStream[sampleId]) {

		if (_mixer->isSoundIDActive(sampleId)) {
			return;
		}

		Audio::AudioStream *audioStream = Audio::makeWAVStream(_voiceStream[sampleId], DisposeAfterUse::YES);
		if (loopType) {
			audioStream = new Audio::LoopingAudioStream((Audio::RewindableAudioStream*)audioStream, 0, DisposeAfterUse::NO);
		}
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle[sampleId], audioStream, sampleId);
	}
}

void PrinceEngine::stopSample(uint16 sampleId) {
	_mixer->stopID(sampleId);
	_voiceStream[sampleId] = nullptr;
}

bool PrinceEngine::loadSample(uint32 sampleSlot, const Common::String &streamName) {
	// FIXME: This is just a workaround streamName is a path 
	// SOUND\\SCIERKA1.WAV for now only last path component is used
	Common::String normalizedPath = lastPathComponent(streamName, '\\');

	debugEngine("loadSample slot %d, name %s", sampleSlot, normalizedPath.c_str());

	_mixer->stopID(sampleSlot);
	_voiceStream[sampleSlot] = nullptr;
	_voiceStream[sampleSlot] = SearchMan.createReadStreamForMember(normalizedPath);
	if (_voiceStream[sampleSlot] == nullptr) {
		error("Can't load sample %s to slot %d", normalizedPath.c_str(), sampleSlot);
	}
	return _voiceStream[sampleSlot] == nullptr;
}

bool PrinceEngine::loadVoice(uint32 slot, uint32 sampleSlot, const Common::String &streamName) {
	debugEngine("Loading wav %s slot %d", streamName.c_str(), slot);

	if (slot > MAXTEXTS) {
		error("Text slot bigger than MAXTEXTS %d", MAXTEXTS);
		return false;
	}

	_voiceStream[sampleSlot] = SearchMan.createReadStreamForMember(streamName);
	if (!_voiceStream[sampleSlot]) {
		error("Can't open %s", streamName.c_str());
		return false;
	}

	uint32 id = _voiceStream[sampleSlot]->readUint32LE();
	if (id != MKTAG('F', 'F', 'I', 'R')) {
		error("It's not RIFF file %s", streamName.c_str());
		return false;
	}

	_voiceStream[sampleSlot]->skip(0x20);
	id = _voiceStream[sampleSlot]->readUint32LE();
	if (id != MKTAG('a', 't', 'a', 'd')) {
		error("No data section in %s id %04x", streamName.c_str(), id);
		return false;
	}

	id = _voiceStream[sampleSlot]->readUint32LE();
	debugEngine("SetVoice slot %d time %04x", slot, id); 
	id <<= 3;
	id /= 22050;
	id += 2;

	_textSlots[slot]._time = id;

	debugEngine("SetVoice slot %d time %04x", slot, id); 
	_voiceStream[sampleSlot]->seek(0);

	return true;
}

void PrinceEngine::setVoice(uint16 slot, uint32 sampleSlot, uint16 flag) {
	Common::String sampleName;
	uint32 currentString = _interpreter->getCurrentString();

	if (currentString >= 80000) {
		sampleName = Common::String::format("%05d-%02d.WAV", currentString - 80000, flag);
	} else if (currentString >= 70000) {
		sampleName = Common::String::format("inv%02d-01.WAV", currentString - 70000);
	} else if (currentString >= 60000) {
		sampleName = Common::String::format("M%04d-%02d.WAV", currentString - 60000, flag);
	} else if (currentString >= 2000) {
		return;
	} else if (flag >= 100) {
		sampleName = Common::String::format("%03d-%03d.WAV", currentString, flag);
	} else {
		sampleName = Common::String::format("%03d-%02d.WAV", currentString, flag);
	}

	loadVoice(slot, sampleSlot, sampleName);
}

bool PrinceEngine::loadAnim(uint16 animNr, bool loop) {
	Common::String streamName = Common::String::format("AN%02d", animNr);
	Common::SeekableReadStream * flicStream = SearchMan.createReadStreamForMember(streamName);

	if (!flicStream) {
		error("Can't open %s", streamName.c_str());
		return false;
	}

	if (!_flicPlayer.loadStream(flicStream)) {
		error("Can't load flic stream %s", streamName.c_str());
	}

	debugEngine("%s loaded", streamName.c_str());
	_flicLooped = loop;
	_flicPlayer.start();
	playNextFrame();
	return true;
}

bool PrinceEngine::loadZoom(byte *zoomBitmap, uint32 dataSize, const char *resourceName) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		delete stream;
		return false;
	}
	if (stream->read(zoomBitmap, dataSize) != dataSize) {
		free(zoomBitmap);
		delete stream;
		return false;
	}
	delete stream;
	return true;
}

bool PrinceEngine::loadShadow(byte *shadowBitmap, uint32 dataSize, const char *resourceName1, const char *resourceName2) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName1);
	if (!stream) {
		delete stream;
		return false;
	}

	if (stream->read(shadowBitmap, dataSize) != dataSize) {
		free(shadowBitmap);
		delete stream;
		return false;
	}

	Common::SeekableReadStream *stream2 = SearchMan.createReadStreamForMember(resourceName2);
	if (!stream2) {
		delete stream;
		delete stream2;
		return false;
	}

	byte *shadowBitmap2 = shadowBitmap + dataSize;
	if (stream2->read(shadowBitmap2, dataSize) != dataSize) {
		free(shadowBitmap);
		delete stream;
		delete stream2;
		return false;
	}

	delete stream;
	delete stream2;
	return true;
}

bool PrinceEngine::loadAllInv() {
	for (int i = 0; i < kMaxInv; i++) {
		InvItem tempInvItem;

		const Common::String invStreamName = Common::String::format("INV%02d", i);
		Common::SeekableReadStream *invStream = SearchMan.createReadStreamForMember(invStreamName);
		if (!invStream) {
			delete invStream;
			return true;
		}

		tempInvItem._x = invStream->readUint16LE();
		tempInvItem._y = invStream->readUint16LE();
		int width = invStream->readUint16LE();
		int height = invStream->readUint16LE();
		tempInvItem._surface = new Graphics::Surface();
		tempInvItem._surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

		for (int h = 0; h < tempInvItem._surface->h; h++) {
			invStream->read(tempInvItem._surface->getBasePtr(0, h), tempInvItem._surface->w);
		}

		_allInvList.push_back(tempInvItem);
		delete invStream;
	}

	return true;
}

void PrinceEngine::keyHandler(Common::Event event) {
	uint16 nChar = event.kbd.keycode;
	switch (nChar) {
	case Common::KEYCODE_d:
		if (event.kbd.hasFlags(Common::KBD_CTRL)) {
			getDebugger()->attach();
		}
		break;
	case Common::KEYCODE_LEFT:
		if(testAnimNr > 0) {
			testAnimNr--;
		}
		debug("testAnimNr: %d", testAnimNr);
		break;
	case Common::KEYCODE_RIGHT:
		testAnimNr++;
		debug("testAnimNr: %d", testAnimNr);
		break;
	case Common::KEYCODE_ESCAPE:
		_flags->setFlagValue(Flags::ESCAPED2, 1);
		break;
	case Common::KEYCODE_UP:
		_mainHero->_phase++;
		debugEngine("%d", _mainHero->_phase);
		testAnimFrame++;
		break;
	case Common::KEYCODE_DOWN:
		if(_mainHero->_phase > 0) {
			_mainHero->_phase--;
		}
		if (testAnimFrame > 0) {
			testAnimFrame--;
		}
		debugEngine("%d", _mainHero->_phase);
		break;
	case Common::KEYCODE_w:
		_mainHero->_lastDirection = _mainHero->UP;
		debugEngine("UP");
		break;
	case Common::KEYCODE_s:
		_mainHero->_lastDirection = _mainHero->DOWN;
		debugEngine("DOWN");
		break;
	case Common::KEYCODE_a:
		_mainHero->_lastDirection = _mainHero->LEFT;
		debugEngine("LEFT");
		break;
	case Common::KEYCODE_f:
		_mainHero->_lastDirection = _mainHero->RIGHT;
		debugEngine("RIGHT");
		break;
	case Common::KEYCODE_1:
		if(_mainHero->_state > 0) {
			_mainHero->_state--;
		}
		debugEngine("%d", _mainHero->_state);
		break;
	case Common::KEYCODE_2:
		_mainHero->_state++;
		debugEngine("%d", _mainHero->_state);
		break;
	case Common::KEYCODE_i:
		_mainHero->_middleY -= 5;
		break;
	case Common::KEYCODE_k:
		_mainHero->_middleY += 5;
		//addInvObj();
		break;
	case Common::KEYCODE_j:
		_mainHero->_middleX -= 5;
		//_flags->setFlagValue(Flags::CURSEBLINK, 1);
		//addInvObj();
		//_flags->setFlagValue(Flags::CURSEBLINK, 0);
		break;
	case Common::KEYCODE_l:
		_mainHero->_middleX += 5;
		break;
	case Common::KEYCODE_EQUALS:
		if (_debugger->_locationNr > 1) {
			_debugger->_locationNr--;
		}
		break;
	case Common::KEYCODE_BACKSPACE:
		if (_debugger->_locationNr < 43) {
			_debugger->_locationNr++;
		}
		break;
	}
}

int PrinceEngine::checkMob(Graphics::Surface *screen, Common::Array<Mob> &mobList) {
	Common::Point mousepos = _system->getEventManager()->getMousePos();
	Common::Point mousePosCamera(mousepos.x + _picWindowX, mousepos.y);

	int mobNumber = 0;
	for (Common::Array<Mob>::const_iterator it = mobList.begin(); it != mobList.end() ; it++) {
		const Mob& mob = *it;
		mobNumber++;

		if (mob._visible) {
			continue;
		}

		int type = mob._type & 7;
		switch (type) {
		case 0:
		case 1:
			//normal_mob
			if (!mob._rect.contains(mousePosCamera)) {
				continue;
			}
			break;
		case 3:
			//mob_obj
			if (mob._mask < _objList.size()) {
				Object &obj = *_objList[mob._mask];
				Common::Rect objectRect(obj._x, obj._y, obj._x + obj._width, obj._y + obj._height);
				if (objectRect.contains(mousePosCamera)) {
					Graphics::Surface *objSurface = obj.getSurface();
					byte *pixel = (byte *)objSurface->getBasePtr(mousePosCamera.x - obj._x, mousePosCamera.y - obj._y);
					if (*pixel != 255) {
						break;
					}
				}
			}
			continue;
			break;
		case 2:
		case 5:
			//check_ba_mob
			if (mob._mask < _backAnimList.size()) {
				int currentAnim = _backAnimList[mob._mask]._seq._currRelative;
				Anim &backAnim = _backAnimList[mob._mask].backAnims[currentAnim];
				if (!backAnim._state) {
					Common::Rect backAnimRect(backAnim._currX, backAnim._currY, backAnim._currX + backAnim._currW, backAnim._currY + backAnim._currH);
					if (backAnimRect.contains(mousePosCamera)) {
						int phase = backAnim._showFrame;
						int phaseFrameIndex = backAnim._animData->getPhaseFrameIndex(phase);
						Graphics::Surface *backAnimSurface = backAnim._animData->getFrame(phaseFrameIndex);
						byte pixel = *(byte *)backAnimSurface->getBasePtr(mousePosCamera.x - backAnim._currX, mousePosCamera.y - backAnim._currY);
						backAnimSurface->free();
						delete backAnimSurface;
						if (pixel != 255) {
							break;
						}
					}
				}
			}
			continue;
			break;
		default:
			//not_part_ba
			continue;
			break;
		}

		Common::String mobName = mob._name;

		if (getLanguage() == Common::DE_DEU) {
			for (uint i = 0; i < mobName.size(); i++) {
				switch (mobName[i]) {
				case '\xc4':
					mobName.setChar('\x83', i);
					break;
				case '\xd6':
					mobName.setChar('\x84', i);
					break;
				case '\xdc':
					mobName.setChar('\x85', i);
					break;
				case '\xdf':
					mobName.setChar('\x7f', i);
					break;
				case '\xe4':
					mobName.setChar('\x80', i);
					break;
				case '\xf6':
					mobName.setChar('\x81', i);
					break;
				case '\xfc':
					mobName.setChar('\x82', i);
					break;
				}
			}
		}

		uint16 textW = getTextWidth(mobName.c_str());

		uint16 x = mousepos.x - textW / 2;
		if (x > screen->w) {
			x = 0;
		}

		if (x + textW > screen->w) {
			x = screen->w - textW;
		}

		uint16 y = mousepos.y - _font->getFontHeight();
		if (y > screen->h) {
			y = _font->getFontHeight() - 2;
		}

		_font->drawString(screen, mobName, x, y, screen->w, 216);

		return mobNumber - 1;
	}
	return -1;
}

void PrinceEngine::printAt(uint32 slot, uint8 color, char *s, uint16 x, uint16 y) {

	debugC(1, DebugChannel::kEngine, "PrinceEngine::printAt slot %d, color %d, x %02d, y %02d, str %s", slot, color, x, y, s);

	char *strPointer = s;

	if (getLanguage() == Common::DE_DEU) {
		while (*strPointer) {
			switch(*strPointer) {
			case '\xc4':
				*strPointer = '\x83';
				break;
			case '\xd6':
				*strPointer = '\x84';
				break;
			case '\xdc':
				*strPointer = '\x85';
				break;
			case '\xdf':
				*strPointer = '\x7f';
				break;
			case '\xe4':
				*strPointer = '\x80';
				break;
			case '\xf6':
				*strPointer = '\x81';
				break;
			case '\xfc':
				*strPointer = '\x82';
				break;
			}
			strPointer++;
		}
	}

	Text &text = _textSlots[slot];
	text._str = s;
	text._x = x;
	text._y = y;
	text._color = color;
}

int PrinceEngine::calcText(const char *s) {
	int lines = 1;
	while (*s) {
		if (*s == '\n') {
			lines++;
		}
		s++;
	}
	return lines;
	//time = lines * 30
}

uint32 PrinceEngine::getTextWidth(const char *s) {
	uint16 textW = 0;
	while (*s) {
		textW += _font->getCharWidth(*s) + _font->getKerningOffset(0, 0);
		s++;
	}
	return textW;
}

void PrinceEngine::showTexts(Graphics::Surface *screen) {
	for (uint32 slot = 0; slot < MAXTEXTS; ++slot) {
		Text& text = _textSlots[slot];
		if (!text._str && !text._time) {
			continue;
		}

		Common::Array<Common::String> lines;
		_font->wordWrapText(text._str, _graph->_frontScreen->w, lines);

		int wideLine = 0;
		for (uint i = 0; i < lines.size(); i++) {
			int textLen = getTextWidth(lines[i].c_str());
			if (textLen > wideLine) {
				wideLine = textLen;
			}
		}

		int leftBorderText = 6;
		if (text._x + wideLine / 2 >  kNormalWidth - leftBorderText) {
			text._x = kNormalWidth - leftBorderText - wideLine / 2;
		}

		if (text._x - wideLine / 2 < leftBorderText) {
			text._x = leftBorderText + wideLine / 2;
		}

		int textSkip = 2;
		for (uint8 i = 0; i < lines.size(); i++) {
			int x = text._x - getTextWidth(lines[i].c_str()) / 2;
			int y = text._y - 10 - (lines.size() - i) * (_font->getFontHeight() - textSkip);
			if (x < 0) {
				x = 0;
			}
			if (y < 0) {
				y = 0;
			}
			_font->drawString(screen, lines[i], x, y, screen->w, text._color);
		}

		text._time--;
		if (text._time == 0) {
			text._str = nullptr;
		}
	}
}

bool PrinceEngine::spriteCheck(int sprWidth, int sprHeight, int destX, int destY) {
	destX -= _picWindowX;
	destY -= _picWindowY;

	 // if x1 is on visible part of screen
	if (destX < 0) {
		if (destX + sprWidth < 1) {
			//x2 is negative - out of window
			return false;
		}
	}
	 // if x1 is outside of screen on right side
	if (destX >= kNormalWidth) {
		return false;
	}

	if (destY < 0) {
		if (destY + sprHeight < 1) {
			//y2 is negative - out of window
			return false;
		}
	}
	if (destY >= kNormalHeight) {
		return false;
	}

	return true;
}

// CheckNak
void PrinceEngine::checkMasks(int x1, int y1, int sprWidth, int sprHeight, int z) {
	int x2 = x1 + sprWidth - 1;
	int y2 = y1 + sprHeight - 1;
	if (x1 < 0) {
		x1 = 0;
	}
	for (uint i = 0; i < _maskList.size() ; i++) {
		if (_maskList[i]._state != 1 && _maskList[i]._flags != 1) {
			if (_maskList[i]._z > z) {
				if (_maskList[i]._x1 <= x2 && _maskList[i]._x2 >= x1) {
					if (_maskList[i]._y1 <= y2 && _maskList[i]._y2 >= y1) {
						_maskList[i]._state = 1;
					}
				}
			}
		}
	}
}

// ClsNak
void PrinceEngine::clsMasks() {
	for (uint i = 0; i < _maskList.size(); i++) {
		if (_maskList[i]._state == 1) {
			_maskList[i]._state = 0;
		}
	}
}

// InsertNakladki
void PrinceEngine::insertMasks(Graphics::Surface *originalRoomSurface) {
	for (uint i = 0; i < _maskList.size(); i++) {
		if (_maskList[i]._state == 1) {
			showMask(i, originalRoomSurface);
		}
	}
}

// ShowNak
void PrinceEngine::showMask(int maskNr, Graphics::Surface *originalRoomSurface) {
	if (_maskList[maskNr]._flags == 0) {
		if (spriteCheck(_maskList[maskNr]._width, _maskList[maskNr]._height, _maskList[maskNr]._x1, _maskList[maskNr]._y1)) {
			int destX = _maskList[maskNr]._x1 - _picWindowX;
			int destY = _maskList[maskNr]._y1 - _picWindowY;
			DrawNode newDrawNode;
			newDrawNode.posX = destX;
			newDrawNode.posY = destY;
			newDrawNode.posZ = _maskList[maskNr]._z;
			newDrawNode.width = _maskList[maskNr]._width;
			newDrawNode.height = _maskList[maskNr]._height;
			newDrawNode.s = nullptr;
			newDrawNode.originalRoomSurface = originalRoomSurface;
			newDrawNode.data = _maskList[maskNr].getMask();
			newDrawNode.freeSurfaceSMemory = false;
			newDrawNode.drawFunction = &_graph->drawMaskDrawNode;
			_drawNodeList.push_back(newDrawNode);
		}
	}
}

void PrinceEngine::showSprite(Graphics::Surface *spriteSurface, int destX, int destY, int destZ, bool freeSurfaceMemory) {
	if (spriteCheck(spriteSurface->w, spriteSurface->h, destX, destY)) {
		destX -= _picWindowX;
		destY -= _picWindowY;
		DrawNode newDrawNode;
		newDrawNode.posX = destX;
		newDrawNode.posY = destY;
		newDrawNode.posZ = destZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.s = spriteSurface;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = nullptr;
		newDrawNode.freeSurfaceSMemory = freeSurfaceMemory;
		newDrawNode.drawFunction = &_graph->drawTransparentDrawNode;
		_drawNodeList.push_back(newDrawNode);
	} else if (freeSurfaceMemory) {
		spriteSurface->free();
		delete spriteSurface;
	}
}

void PrinceEngine::showSpriteShadow(Graphics::Surface *shadowSurface, int destX, int destY, int destZ, bool freeSurfaceMemory) {
	if (spriteCheck(shadowSurface->w, shadowSurface->h, destX, destY)) {
		destX -= _picWindowX;
		destY -= _picWindowY;
		DrawNode newDrawNode;
		newDrawNode.posX = destX;
		newDrawNode.posY = destY;
		newDrawNode.posZ = destZ;
		newDrawNode.width = 0;
		newDrawNode.height = 0;
		newDrawNode.s = shadowSurface;
		newDrawNode.originalRoomSurface = nullptr;
		newDrawNode.data = _graph->_shadowTable70;
		newDrawNode.freeSurfaceSMemory = freeSurfaceMemory;
		newDrawNode.drawFunction = &_graph->drawAsShadowDrawNode;
		_drawNodeList.push_back(newDrawNode);
	} else if (freeSurfaceMemory) {
		shadowSurface->free();
		delete shadowSurface;
	}
}

void PrinceEngine::showAnim(Anim &anim) {
	//ShowFrameCode
	//ShowAnimFrame
	int phaseCount = anim._animData->getPhaseCount();
	int frameCount = anim._animData->getFrameCount();
	int phase = anim._showFrame;
	int phaseFrameIndex = anim._animData->getPhaseFrameIndex(phase);
	int x = anim._x + anim._animData->getPhaseOffsetX(phase);
	int y = anim._y + anim._animData->getPhaseOffsetY(phase);
	int animFlag = anim._flags;
	int checkMaskFlag = (animFlag & 1);
	int maxFrontFlag = (animFlag & 2);
	int specialZFlag = anim._nextAnim;
	int z = anim._nextAnim;
	int frameWidth = anim._animData->getFrameWidth(phaseFrameIndex);
	int frameHeight = anim._animData->getFrameHeight(phaseFrameIndex);
	int shadowZ = 0;

	if (x != 0 || y != 0 || phaseCount != 1 || frameCount != 1) { // TODO - check if this needed

		if (checkMaskFlag != 0) {
			if (anim._nextAnim == 0) {
				z = y + frameHeight - 1;
			}
			checkMasks(x, y, frameWidth, frameHeight, z);
		}

		if (specialZFlag != 0) {
			z = specialZFlag;
		} else if (maxFrontFlag != 0) {
			z = kMaxPicHeight + 1;
		} else {
			z = y + frameHeight - 1;
		}
		shadowZ = z;

		anim._currX = x;
		anim._currY = y;
		anim._currW = frameWidth;
		anim._currH = frameHeight;
		Graphics::Surface *backAnimSurface = anim._animData->getFrame(phaseFrameIndex); // TODO - check for memory leak
		showSprite(backAnimSurface, x, y, z, true);
	}

	//ShowFrameCodeShadow
	//ShowAnimFrameShadow
	if (anim._shadowData != nullptr) {
		int shadowPhaseFrameIndex = anim._shadowData->getPhaseFrameIndex(phase);
		int shadowX = anim._shadowData->getBaseX() + anim._shadowData->getPhaseOffsetX(phase);
		int shadowY = anim._shadowData->getBaseY() + anim._shadowData->getPhaseOffsetY(phase);
		int shadowFrameWidth = anim._shadowData->getFrameWidth(shadowPhaseFrameIndex);
		int shadowFrameHeight = anim._shadowData->getFrameHeight(shadowPhaseFrameIndex);

		if (checkMaskFlag != 0) {
			checkMasks(shadowX, shadowY, shadowFrameWidth, shadowFrameHeight, shadowY + shadowFrameWidth - 1);
		}

		if (shadowZ == 0) {
			if (maxFrontFlag != 0) {
				shadowZ = kMaxPicHeight + 1;
			} else {
				shadowZ = shadowY + shadowFrameWidth - 1;
			}
		}

		Graphics::Surface *shadowSurface = anim._shadowData->getFrame(shadowPhaseFrameIndex); // TODO - check for memory leak
		showSpriteShadow(shadowSurface, shadowX, shadowY, shadowZ, true);
	}
}

void PrinceEngine::showNormAnims() {
	for (int i = 0; i < kMaxNormAnims; i++) {
		Anim &anim = _normAnimList[i];
		if (anim._animData != nullptr) {
			if (!anim._state) {
				if (anim._frame == anim._lastFrame - 1) { // TODO - check if correct
					if (anim._loopType) {
						if (anim._loopType == 1) {
							anim._frame = anim._loopFrame;
						} else {
							continue;
						}
					} else {
						if (anim._frame >= 1) {
							anim._frame--;
						} else {
							anim._frame = 0;
						}
					}
				} else {
					anim._frame++;
				}
				anim._showFrame = anim._frame;
				showAnim(anim);
			}
		}
	}
}

void PrinceEngine::setBackAnim(Anim &backAnim) {
	int start = backAnim._basaData._start;
	if (start != -1) {
		backAnim._frame = start;
		backAnim._showFrame = start;
		backAnim._loopFrame = start;
	}
	int end = backAnim._basaData._end;
	if (end != -1) {
		backAnim._lastFrame = end;
	}
	backAnim._state = 0;
}

void PrinceEngine::showBackAnims() {
	for (uint i = 0; i < _backAnimList.size(); i++) {
		BAS &seq = _backAnimList[i]._seq;
		int activeSubAnim = seq._currRelative;

		if (!_backAnimList[i].backAnims[activeSubAnim]._state) {
			seq._counter++;
			if (seq._type == 2) {
				if (!seq._currRelative) {
					if (seq._counter >= seq._data) {
						if (seq._anims > 2) {
							seq._currRelative = _randomSource.getRandomNumber(seq._anims - 2) + 1;
							activeSubAnim = seq._currRelative;
							seq._current = _backAnimList[i].backAnims[activeSubAnim]._basaData._num;
						}
						setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
						seq._counter = 0;
					}
				}
			}

			if (seq._type == 3) {
				if (!seq._currRelative) {
					if (seq._counter < seq._data2) {
						continue;
					} else {
						setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
					}
				}
			}

			if (_backAnimList[i].backAnims[activeSubAnim]._frame == _backAnimList[i].backAnims[activeSubAnim]._lastFrame - 1) {
				_backAnimList[i].backAnims[activeSubAnim]._frame = _backAnimList[i].backAnims[activeSubAnim]._loopFrame;
				switch (seq._type) {
				case 1:
					if (seq._anims > 1) {
						int rnd;
						do {
							rnd = _randomSource.getRandomNumber(seq._anims - 1);
						} while (rnd == seq._currRelative);
						seq._currRelative = rnd;
						seq._current = _backAnimList[i].backAnims[rnd]._basaData._num;
						activeSubAnim = rnd;
						setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
						seq._counter = 0;
					}
					break;
				case 2:
					if (seq._currRelative) {
						seq._currRelative = 0;
						seq._current = _backAnimList[i].backAnims[0]._basaData._num;
						activeSubAnim = 0;
						setBackAnim(_backAnimList[i].backAnims[activeSubAnim]);
						seq._counter = 0;
					}
					break;
				case 3:
					seq._currRelative = 0;
					seq._current = _backAnimList[i].backAnims[0]._basaData._num;
					seq._counter = 0;
					seq._data2 = _randomSource.getRandomNumber(seq._data - 1);
					continue; // for bug in original game
					break;
				}
			} else {
				_backAnimList[i].backAnims[activeSubAnim]._frame++;
			}
			_backAnimList[i].backAnims[activeSubAnim]._showFrame = _backAnimList[i].backAnims[activeSubAnim]._frame;
			showAnim(_backAnimList[i].backAnims[activeSubAnim]);
		}
	}
}

void PrinceEngine::clearBackAnimList() {
	for (uint i = 0; i < _backAnimList.size(); i++) {
		int anims = _backAnimList[i]._seq._anims != 0 ? _backAnimList[i]._seq._anims : 1;
		for (int j = 0; j < anims; j++) {
			delete _backAnimList[i].backAnims[j]._animData;
			delete _backAnimList[i].backAnims[j]._shadowData;
		}
		_backAnimList[i].backAnims.clear();
	}
	_backAnimList.clear();
}

void PrinceEngine::showObjects() {
	if (!_objList.empty()) {
		for (uint i = 0; i < _objList.size(); i++) {
			if ((_objList[i]->_mask & 0x8000) != 0) {
				_objList[i]->_zoomInTime--;
				if (_objList[i]->_zoomInTime == 0) {
					_objList[i]->_mask &= 0x7FFF;
				} else {
					// doZoomIn();
					// mov edx, d [esi.Obj_ZoomInAddr]
				}
			}
			if ((_objList[i]->_mask & 0x4000) != 0) {
				_objList[i]->_zoomInTime--;
				if (_objList[i]->_zoomInTime == 0) {
					_objList[i]->_mask &= 0xBFFF;
				} else {
					// doZoomOut();
					// mov edx, d [esi.Obj_ZoomInAddr]
				}
			}
			Graphics::Surface *objSurface = _objList[i]->getSurface();
			if (spriteCheck(objSurface->w, objSurface->h, _objList[i]->_x, _objList[i]->_y)) {
				if ((_objList[i]->_mask & 0x0200) == 0) {
					int destX = _objList[i]->_x - _picWindowX;
					int destY = _objList[i]->_y - _picWindowY;
					DrawNode newDrawNode;
					newDrawNode.posX = destX;
					newDrawNode.posY = destY;
					newDrawNode.posZ = _objList[i]->_z;
					newDrawNode.width = 0;
					newDrawNode.height = 0;
					newDrawNode.s = objSurface;
					newDrawNode.originalRoomSurface = nullptr;
					newDrawNode.data = nullptr;
					newDrawNode.freeSurfaceSMemory = false;
					newDrawNode.drawFunction = &_graph->drawTransparentDrawNode;
					_drawNodeList.push_back(newDrawNode);
				} else {
					// showBackSprite();
				}
			}
			if ((_objList[i]->_mask & 1) != 0) {
				checkMasks(_objList[i]->_x, _objList[i]->_y, objSurface->w, objSurface->h, _objList[i]->_z);
			}
		}
	}
}

void PrinceEngine::showParallax() {
	if (!_pscrList.empty()) {
		for (uint i = 0; i < _pscrList.size(); i++) {
			Graphics::Surface *pscrSurface = _pscrList[i]->getSurface();
			int x = _pscrList[i]->_x - (_pscrList[i]->_step * _picWindowX / 4);
			int y = _pscrList[i]->_y;
			int z = 1000;
			if (spriteCheck(pscrSurface->w, pscrSurface->h, x, y)) {
				showSprite(pscrSurface, x, y, z, false);
			}
		}
	}
}

bool PrinceEngine::compareDrawNodes(DrawNode d1, DrawNode d2) {
	if (d1.posZ < d2.posZ) {
		return true;
	}
	return false;
}

void PrinceEngine::runDrawNodes() {
	Common::sort(_drawNodeList.begin(), _drawNodeList.end(), compareDrawNodes);

	for (uint i = 0; i < _drawNodeList.size(); i++) {
		(*_drawNodeList[i].drawFunction)(_graph->_frontScreen, &_drawNodeList[i]);
	}
	_graph->change();
}


void PrinceEngine::freeDrawNodes() {
	for (uint i = 0; i < _drawNodeList.size(); i++) {
		if (_drawNodeList[i].freeSurfaceSMemory) {
			_drawNodeList[i].s->free();
			delete _drawNodeList[i].s;
		}
	}
	_drawNodeList.clear();
}

void PrinceEngine::drawScreen() {
	if (!_showInventoryFlag || _inventoryBackgroundRemember) {
		const Graphics::Surface *roomSurface = _roomBmp->getSurface();
		Graphics::Surface visiblePart;
		if (roomSurface) {
			visiblePart = roomSurface->getSubArea(Common::Rect(_picWindowX, 0, roomSurface->w, roomSurface->h));
			_graph->draw(_graph->_frontScreen, &visiblePart);
		}

		Graphics::Surface *mainHeroSurface = NULL;
		if (_mainHero->_visible) {
			mainHeroSurface = _mainHero->getSurface();
			if (mainHeroSurface) {
				_mainHero->showHeroShadow(mainHeroSurface);

				DrawNode newDrawNode;
				newDrawNode.posX = _mainHero->_drawX;
				newDrawNode.posY = _mainHero->_drawY;
				newDrawNode.posZ = _mainHero->_drawZ;
				newDrawNode.width = 0;
				newDrawNode.height = 0;
				newDrawNode.originalRoomSurface = nullptr;
				newDrawNode.data = nullptr;
				newDrawNode.drawFunction = &_graph->drawTransparentDrawNode;

				if (_mainHero->_zoomFactor != 0) {
					Graphics::Surface *zoomedHeroSurface = _mainHero->zoomSprite(mainHeroSurface);
					newDrawNode.s = zoomedHeroSurface;
					newDrawNode.freeSurfaceSMemory = true;
				} else {
					newDrawNode.s = mainHeroSurface;
					newDrawNode.freeSurfaceSMemory = false;
				}
				_drawNodeList.push_back(newDrawNode);
			}
		}

		showNormAnims();

		showBackAnims();

		showObjects();

		if (roomSurface) {
			insertMasks(&visiblePart);
		}

		showParallax();

		runDrawNodes();

		freeDrawNodes();

		if (_mainHero->_visible) {
			mainHeroSurface->free();
			delete mainHeroSurface;
		}

		clsMasks();

		playNextFrame();

		if (!_inventoryBackgroundRemember && !_dialogFlag) {
			if (!_optionsFlag) {
				_selectedMob = checkMob(_graph->_frontScreen, _mobList);
			}
			showTexts(_graph->_frontScreen);
			checkOptions();
		} else {
			_inventoryBackgroundRemember = false;
		}

		getDebugger()->onFrame();

	} else {
		displayInventory();
	}
}

void PrinceEngine::pause() {
	uint32 currentTime = _system->getMillis();
	int delay = 1000/15 - int32(_system->getMillis() - currentTime);
	delay = delay < 0 ? 0 : delay;
	_system->delayMillis(delay);
}

void PrinceEngine::addInvObj() {
	changeCursor(0);
	//prepareInventoryToView();

	_inventoryBackgroundRemember = true;
	drawScreen();

	Graphics::Surface *suitcase = _suitcaseBmp->getSurface();

	if (!_flags->getFlagValue(Flags::CURSEBLINK)) {

		loadSample(27, "PRZEDMIO.WAV");
		playSample(27, 0);

		_mst_shadow2 = 1;

		while (_mst_shadow2 < 512) {
			rememberScreenInv();
			_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase, 0);
			drawInvItems();
			_graph->update(_graph->_screenForInventory);
			_mst_shadow2 += 50;
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				return;
			}
			pause();
		}
		while (_mst_shadow2 > 256) {
			rememberScreenInv();
			_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase, 0);
			drawInvItems();
			_graph->update(_graph->_screenForInventory);
			_mst_shadow2 -= 42;
			Common::Event event;
			Common::EventManager *eventMan = _system->getEventManager();
			eventMan->pollEvent(event);
			if (shouldQuit()) {
				return;
			}
			pause();
		}
	} else {
		//CURSEBLINK:
		for (int i = 0; i < 3; i++) {
			_mst_shadow2 = 256;
			while (_mst_shadow2 < 512) {
				rememberScreenInv();
				_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase, 0);
				drawInvItems();
				_graph->update(_graph->_screenForInventory);
				_mst_shadow2 += 50;
				Common::Event event;
				Common::EventManager *eventMan = _system->getEventManager();
				eventMan->pollEvent(event);
				if (shouldQuit()) {
					return;
				}
				pause();
			}
			while (_mst_shadow2 > 256) {
				rememberScreenInv();
				_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase, 0);
				drawInvItems();
				_graph->update(_graph->_screenForInventory);
				_mst_shadow2 -= 50;
				Common::Event event;
				Common::EventManager *eventMan = _system->getEventManager();
				eventMan->pollEvent(event);
				if (shouldQuit()) {
					return;
				}
				pause();
			}
		}
	}
	_mst_shadow2 = 0;
	for (int i = 0; i < 20; i++) {
		rememberScreenInv();
		_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase, 0);
		drawInvItems();
		_graph->update(_graph->_screenForInventory);
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		eventMan->pollEvent(event);
		if (shouldQuit()) {
			return;
		}
		pause();
	}
	changeCursor(1); // here?
}

void PrinceEngine::rememberScreenInv() {
	_graph->_screenForInventory->copyFrom(*_graph->_frontScreen);
}

void PrinceEngine::inventoryFlagChange(bool inventoryState) {
	if (inventoryState) {
		_showInventoryFlag = true;
		_inventoryBackgroundRemember = true;
	} else {
		_showInventoryFlag = false;
	}
}

void PrinceEngine::prepareInventoryToView() {
	_invMobList.clear();
	int invItem = _mainHero->_inventory.size();
	_invLine =  invItem / 3;
	if (invItem % 3 != 0) {
		_invLine++;
	}
	if (_invLine < 4) {
		_invLine = 4;
	}
	_maxInvW = (374 - 2 * _invLine) / _invLine;
	_invLineW = _maxInvW - 2;

	int currInvX = _invLineX;
	int currInvY = _invLineY;

	Common::MemoryReadStream stream(_invTxt, _invTxtSize);
	byte c;

	uint item = 0;
	for (int i = 0 ; i < _invLines; i++) {
		for (int j = 0; j < _invLine; j++) {
			Mob tempMobItem;
			if (item < _mainHero->_inventory.size()) {
				int itemNr = _mainHero->_inventory[item];
				tempMobItem._visible = 0;
				tempMobItem._mask =  itemNr;
				tempMobItem._rect = Common::Rect(currInvX + _picWindowX, currInvY, currInvX + _picWindowX + _invLineW  - 1, currInvY + _invLineH - 1);
				tempMobItem._type = 0; // to work with checkMob()

				tempMobItem._name = "";
				tempMobItem._examText = "";
				int txtOffset = READ_UINT32(&_invTxt[itemNr * 8]);
				int examTxtOffset = READ_UINT32(&_invTxt[itemNr * 8 + 4]);

				stream.seek(txtOffset);
				while ((c = stream.readByte())) {
					tempMobItem._name += c;
				}

				stream.seek(examTxtOffset);
				while ((c = stream.readByte())) {
					tempMobItem._examText += c;
				}
				_invMobList.push_back(tempMobItem);
			}
			currInvX += _invLineW + _invLineSkipX;
			item++;
		}
		currInvX = _invLineX;
		currInvY += _invLineSkipY + _invLineH;
	}
}

void PrinceEngine::drawInvItems() {
	int currInvX = _invLineX;
	int currInvY = _invLineY;
	uint item = 0;
	for (int i = 0 ; i < _invLines; i++) {
		for (int j = 0; j < _invLine; j++) {
			if (item < _mainHero->_inventory.size()) {
				int itemNr = _mainHero->_inventory[item]; // itemNr =- 1 ?
				_mst_shadow = 0;
				if (_mst_shadow2 != 0) {
					if (!_flags->getFlagValue(Flags::CURSEBLINK)) {
						if (item + 1 == _mainHero->_inventory.size()) { // last item in inventory
							_mst_shadow = 1;
						}
					} else if (itemNr == 1 || itemNr == 3 || itemNr == 4 || itemNr == 7) {
						_mst_shadow = 1;
					}
				}

				int drawX = currInvX;
				int drawY = currInvY;
				Graphics::Surface *itemSurface = NULL;
				if (itemNr != 68) {
					itemSurface = _allInvList[itemNr].getSurface();
					if (itemSurface->h < _maxInvH) {
						drawY += (_maxInvH - itemSurface->h) / 2;
					}
				} else {
					// candle item:
					if (_candleCounter == 8) {
						_candleCounter = 0;
					}
					itemNr = _candleCounter;
					_candleCounter++;
					itemNr &= 7;
					itemNr += 71;
					itemSurface = _allInvList[itemNr].getSurface();
					drawY += _allInvList[itemNr]._y + (_maxInvH - 76) / 2 - 200;
				}
				if (itemSurface->w < _maxInvW) {
					drawX += (_maxInvW - itemSurface->w) / 2;
				}
				if (!_mst_shadow) {
					_graph->drawTransparentSurface(_graph->_screenForInventory, drawX, drawY, itemSurface, 0);
				} else {
					_mst_shadow = _mst_shadow2;
					_graph->drawTransparentWithBlendSurface(_graph->_screenForInventory, drawX, drawY, itemSurface, 0);
				}
			}
			currInvX += _invLineW + _invLineSkipX;
			item++;
		}
		currInvX = _invLineX;
		currInvY += _invLineSkipY + _invLineH;
	}
}

void PrinceEngine::leftMouseButton() {
	int option = 0;
	int optionEvent = -1;

	if (_optionsFlag) {
		if (_optionEnabled < _optionsNumber) {
			option = _optionEnabled;
			_optionsFlag = 0;
		} else {
			return;
		}
	} else {
		_optionsMob = _selectedMob;
		if (_optionsMob == -1) {
			// @@walkto - TODO
			return;
		}
		option = 0;
	}
	//do_option
	if (_currentPointerNumber != 2) {
		//skip_use_code
		int optionScriptOffset = _room->getOptionOffset(option);
		if (optionScriptOffset != 0) {
			optionEvent = _script->scanMobEvents(_optionsMob, optionScriptOffset);
		}
		if (optionEvent == -1) {
			if (option == 0) {
				//@@walkto - TODO
				return;
			} else {
				optionEvent = _script->getOptionStandardOffset(option);
			}
		}
	} else if (_selectedMode != 0) {
		//give_item
		if (_room->_itemGive != 0) {
			optionEvent = _script->scanMobEventsWithItem(_optionsMob, _room->_itemGive, _selectedItem);
		}
		if (optionEvent == -1) {
			//standard_giveitem
			optionEvent = _script->_scriptInfo.stdGiveItem;
		}
	} else {
		if (_room->_itemUse != 0) {
			optionEvent = _script->scanMobEventsWithItem(_optionsMob, _room->_itemUse, _selectedItem);
			_flags->setFlagValue(Flags::SELITEM, _selectedItem);
		}
		if (optionEvent == -1) {
			//standard_useitem
			optionEvent = _script->_scriptInfo.stdUseItem;
		}
	}
	_interpreter->storeNewPC(optionEvent);
	_flags->setFlagValue(Flags::CURRMOB, _selectedMob);
	_selectedMob = -1;
	_optionsMob = -1;
}

void PrinceEngine::rightMouseButton() {
	if (_currentPointerNumber < 2) {
		enableOptions();
	} else {
		_currentPointerNumber = 1;
		changeCursor(1);
	}
}

void PrinceEngine::inventoryLeftMouseButton() {

	_textSlots[0]._time = 0;
	_textSlots[0]._str = nullptr;
	stopSample(28);

	if (_optionsFlag == 1) {
		//check_opt
		if (_selectedMob != -1)  {
			//inv_check_mob
			if (_optionEnabled < _invOptionsNumber) {
				_optionsFlag = 0;
				//do_option
			} else {
				return;
			}
		} else {
			error("PrinceEngine::inventoryLeftMouseButton() - optionsFlag = 1, selectedMob = 0");
			// test bx, RMBMask 7996 ? right mouse button here? - > return;
			//disable_use
			if (_currentPointerNumber == 2) {
				//disableuseuse
				changeCursor(1);
				_currentPointerNumber = 1;
				//exit_normally
				_selectedMob = -1;
				_optionsMob = -1;
				return;
			} else {
				return;
			}
		}
	} else {
		if (_selectedMob != -1) {
			if (_currentPointerNumber != 2) {
				if (_invMobList[_selectedMob]._mask != 29) {
					_optionEnabled = 0;
				} else {
					// map item
					_optionEnabled = 1;
				}
				//do_option
			} else {
				//use_item_on_item
				int invObjUU = _script->scanMobEventsWithItem(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUU, _selectedItem);
				if (invObjUU == -1) {
					int textNr = 80011; // "I can't do it."
					if (_selectedItem == 31 || _invMobList[_selectedMob]._mask == 31) {
						textNr = 80020; // "Nothing is happening."
					}
					_interpreter->setCurrentString(textNr);
					printAt(0, 216, _variaTxt->getString(textNr - 80000), kNormalWidth / 2, 100);
					setVoice(0, 28, 1);
					playSample(28, 0);
					//exit_normally
					_selectedMob = -1;
					_optionsMob = -1;
					return;
				} else {
					//store_new_pc
					_interpreter->storeNewPC(invObjUU);
					_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
					//byeinv
					_showInventoryFlag = false;
				}
			}
		} else {
			return;
		}
	}
	//do_option
	if (_optionEnabled == 0) {
		int invObjExamEvent = _script->scanMobEvents(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjExam);
		if (invObjExamEvent == -1) {
			// do_standard
			printAt(0, 216, (char *)_invMobList[_selectedMob]._examText.c_str(), kNormalWidth / 2, _invExamY);
			_interpreter->setCurrentString(_invMobList[_selectedMob]._mask + 70000);
			setVoice(0, 28, 1);
			playSample(28, 0);
			// disableuseuse
			changeCursor(0);
			_currentPointerNumber = 1;
			//exit_normally
		} else {
			//store_new_pc
			_interpreter->storeNewPC(invObjExamEvent);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			//bye_inv
			_showInventoryFlag = false;
		}
	} else if (_optionEnabled == 1) {
		// not_examine
		int invObjUse = _script->scanMobEvents(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUse);
		if (invObjUse == -1) {
			// do_standard_use
			_selectedMode = 0;
			_selectedItem = _invMobList[_selectedMob]._mask;
			makeInvCursor(_invMobList[_selectedMob]._mask);
			_currentPointerNumber = 2;
			changeCursor(2);
			//exit_normally
		} else {
			//store_new_pc
			_interpreter->storeNewPC(invObjUse);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			//bye_inv
			_showInventoryFlag = false;
		}
	} else if (_optionEnabled == 4) {
		// not_use_inv
		// do_standard_give
		_selectedMode = 1;
		_selectedItem = _invMobList[_selectedMob]._mask;
		makeInvCursor(_invMobList[_selectedMob]._mask);
		_currentPointerNumber = 2;
		changeCursor(2);
		//exit_normally
	} else {
		// use_item_on_item
		int invObjUU = _script->scanMobEventsWithItem(_invMobList[_selectedMob]._mask, _script->_scriptInfo.invObjUU, _selectedItem);
		if (invObjUU == -1) {
			int textNr = 80011; // "I can't do it."
			if (_selectedItem == 31 || _invMobList[_selectedMob]._mask == 31) {
				textNr = 80020; // "Nothing is happening."
			}
			_interpreter->setCurrentString(textNr);
			printAt(0, 216, _variaTxt->getString(textNr - 80000), kNormalWidth / 2, 100);
			setVoice(0, 28, 1);
			playSample(28, 0);
			//exit_normally
		} else {
			//store_new_pc
			_interpreter->storeNewPC(invObjUU);
			_flags->setFlagValue(Flags::CURRMOB, _invMobList[_selectedMob]._mask);
			//byeinv
			_showInventoryFlag = false;
		}
	}
	//exit_normally
	_selectedMob = -1;
	_optionsMob = -1;
}

void PrinceEngine::inventoryRightMouseButton() {
	if (_textSlots[0]._str == nullptr) {
		enableOptions();
	}
}

void PrinceEngine::enableOptions() {
	if (_optionsFlag != 1) {
		changeCursor(1);
		_currentPointerNumber = 1;
		if (_selectedMob != -1) {
			//if (_mobType != 0x100) {
				Common::Point mousePos = _system->getEventManager()->getMousePos();
				int x1 = mousePos.x - _optionsWidth / 2;
				int x2 = mousePos.x + _optionsWidth / 2;
				if (x1 < 0) {
					x1 = 0;
					x2 = _optionsWidth;
				} else if (x2 >= kNormalWidth) {
					x1 = kNormalWidth - _optionsWidth;
					x2 = kNormalWidth;
				}
				int y1 = mousePos.y - 10;
				if (y1 < 0) {
					y1 = 0;
				}
				if (y1 + _optionsHeight >= kNormalHeight) {
					y1 = kNormalHeight - _optionsHeight;
				}
				_optionsMob = _selectedMob;
				_optionsX = x1;
				_optionsY = y1;
				_optionsFlag = 1;
			//}
		}
	}
}

void PrinceEngine::checkOptions() {
	if (_optionsFlag) {
		Common::Rect optionsRect(_optionsX, _optionsY, _optionsX + _optionsWidth, _optionsY + _optionsHeight);
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		if (!optionsRect.contains(mousePos)) {
			_optionsFlag = 0;
			_selectedMob = -1;
			return;
		}
		_graph->drawAsShadowSurface(_graph->_frontScreen, _optionsX, _optionsY, _optionsPic, _graph->_shadowTable50);

		_optionEnabled = -1;
		int optionsYCord = mousePos.y - (_optionsY + 16);
		if (optionsYCord >= 0) {
			int selectedOptionNr = optionsYCord / _optionsStep;
			if (selectedOptionNr < _optionsNumber) {
				_optionEnabled = selectedOptionNr;
			}
		}
		int optionsColor;
		int textY = _optionsY + 16;
		for (int i = 0; i < _optionsNumber; i++) {
			if (i != _optionEnabled) {
				optionsColor = _optionsColor1;
			} else {
				optionsColor = _optionsColor2;
			}
			Common::String optText;
			switch(getLanguage()) {
			case Common::PL_POL:
				optText = optionsTextPL[i];
				break;
			case Common::DE_DEU:
				optText = optionsTextDE[i];
				break;
			case Common::EN_ANY:
				optText = optionsTextEN[i];
				break;
			};
			uint16 textW = getTextWidth(optText.c_str());
			uint16 textX = _optionsX + _optionsWidth / 2 - textW / 2;
			_font->drawString(_graph->_frontScreen, optText, textX, textY, textW, optionsColor);
			textY += _optionsStep;
		}
	}
}

void PrinceEngine::checkInvOptions() {
	if (_optionsFlag) {
		Common::Rect optionsRect(_optionsX, _optionsY, _optionsX + _invOptionsWidth, _optionsY + _invOptionsHeight);
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		if (!optionsRect.contains(mousePos)) {
			_optionsFlag = 0;
			_selectedMob = -1;
			return;
		}
		_graph->drawAsShadowSurface(_graph->_screenForInventory, _optionsX, _optionsY, _optionsPicInInventory, _graph->_shadowTable50);

		_optionEnabled = -1;
		int optionsYCord = mousePos.y - (_optionsY + 16);
		if (optionsYCord >= 0) {
			int selectedOptionNr = optionsYCord / _invOptionsStep;
			if (selectedOptionNr < _invOptionsNumber) {
				_optionEnabled = selectedOptionNr;
			}
		}
		int optionsColor;
		int textY = _optionsY + 16;
		for (int i = 0; i < _invOptionsNumber; i++) {
			if (i != _optionEnabled) {
				optionsColor = _optionsColor1;
			} else {
				optionsColor = _optionsColor2;
			}
			Common::String invText;
			switch(getLanguage()) {
			case Common::PL_POL:
				invText = invOptionsTextPL[i];
				break;
			case Common::DE_DEU:
				invText = invOptionsTextDE[i];
				break;
			case Common::EN_ANY:
				invText = invOptionsTextEN[i];
				break;
			};
			uint16 textW = getTextWidth(invText.c_str());
			uint16 textX = _optionsX + _invOptionsWidth / 2 - textW / 2;
			_font->drawString(_graph->_screenForInventory, invText, textX, textY, _graph->_screenForInventory->w, optionsColor);
			textY += _invOptionsStep;
		}
	}
}

void PrinceEngine::displayInventory() {
	// temp:
	_mainHero->_inventory.clear();
	_mainHero->_inventory.push_back(1);
	_mainHero->_inventory.push_back(3);
	_mainHero->_inventory.push_back(7);
	_mainHero->_inventory.push_back(4);
	_mainHero->_inventory.push_back(68);

	_mainHero->_inventory.push_back(29);
	_mainHero->_inventory.push_back(13);
	_mainHero->_inventory.push_back(44);
	_mainHero->_inventory.push_back(67);

	_mainHero->_inventory.push_back(8);

	prepareInventoryToView();

	while (!shouldQuit()) {

		if (_textSlots[0]._str != nullptr) {
			changeCursor(0);
		} else {
			changeCursor(_currentPointerNumber);

			Common::Rect inventoryRect(_invX1, _invY1, _invX1 + _invWidth, _invY1 + _invHeight);
			Common::Point mousePos = _system->getEventManager()->getMousePos();

			if (!_invCurInside && inventoryRect.contains(mousePos)) {
				_invCurInside = true;
			}

			if (_invCurInside && !inventoryRect.contains(mousePos)) {
				inventoryFlagChange(false);
				_invCurInside = false;
				break;
			}
		}

		rememberScreenInv();

		Graphics::Surface *suitcase = _suitcaseBmp->getSurface();
		_graph->drawTransparentSurface(_graph->_screenForInventory, 0, 0, suitcase, 0);

		drawInvItems();

		showTexts(_graph->_screenForInventory);

		if (!_optionsFlag && _textSlots[0]._str == nullptr) {
			_selectedMob = checkMob(_graph->_screenForInventory, _invMobList);
		}

		checkInvOptions();

		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				keyHandler(event);
				break;
			case Common::EVENT_KEYUP:
				break;
			case Common::EVENT_MOUSEMOVE:
				break;
			case Common::EVENT_LBUTTONDOWN:
				inventoryLeftMouseButton();
				break;
			case Common::EVENT_RBUTTONDOWN:
				inventoryRightMouseButton();
				break;
			case Common::EVENT_LBUTTONUP:
				break;
			case Common::EVENT_RBUTTONUP:
				break;
			case Common::EVENT_QUIT:
				break;
			default:
				break;
			}
		}

		if (!_showInventoryFlag) {
			break;
		}

		if (shouldQuit())
			return;

		getDebugger()->onFrame();
		_graph->update(_graph->_screenForInventory);
		pause();
	}

	if (_currentPointerNumber == 2) {
		_flags->setFlagValue(Flags::SELITEM, _selectedItem);
	} else {
		_flags->setFlagValue(Flags::SELITEM, 0);
	}
}

void PrinceEngine::createDialogBox(int dialogBoxNr) {
	_dialogLines = 0;
	int amountOfDialogOptions = 0;
	int dialogDataValue = (int)READ_UINT32(_dialogData);

	byte c;
	int sentenceNumber;
	_dialogText = _dialogBoxAddr[dialogBoxNr];
	byte *dialogText = _dialogText;

	while ((sentenceNumber = *dialogText) != 0xFF) {
		dialogText++;
		if (!(dialogDataValue & (1 << sentenceNumber))) {
			_dialogLines += calcText((const char *)dialogText);
			amountOfDialogOptions++;
		}
		do {
			c = *dialogText;
			dialogText++;
		} while (c);
	}

	_dialogHeight = _font->getFontHeight() * _dialogLines + _dialogLineSpace * (amountOfDialogOptions + 1);
	_dialogImage = new Graphics::Surface();
	_dialogImage->create(_dialogWidth, _dialogHeight, Graphics::PixelFormat::createFormatCLUT8());
	Common::Rect dBoxRect(0, 0, _dialogWidth, _dialogHeight);
	_dialogImage->fillRect(dBoxRect, _graph->kShadowColor);
}

void PrinceEngine::runDialog() {

	_dialogFlag = true;

	while (!shouldQuit()) {

		drawScreen();
		// background iterpreter?

		int dialogX = (640 - _dialogWidth) / 2;
		int dialogY = 460 - _dialogHeight;
		_graph->drawAsShadowSurface(_graph->_frontScreen, dialogX, dialogY, _dialogImage, _graph->_shadowTable50);

		int dialogSkipLeft = 14;
		int dialogSkipUp = 10;

		int dialogTextX = dialogX + dialogSkipLeft;
		int dialogTextY = dialogY + dialogSkipUp;

		Common::Point mousePos = _system->getEventManager()->getMousePos();

		byte c;
		int sentenceNumber;
		byte *dialogText = _dialogText;
		byte *dialogCurrentText = nullptr;
		int dialogSelected = -1;
		int dialogDataValue = (int)READ_UINT32(_dialogData);

		while ((sentenceNumber = *dialogText) != 0xFF) {
			dialogText++;
			int actualColor = _dialogColor1;

			if (!(dialogDataValue & (1 << sentenceNumber))) {
				Common::Array<Common::String> lines;
				_font->wordWrapText((const char *)dialogText, _graph->_frontScreen->w, lines);

				Common::Rect dialogOption(dialogTextX, dialogTextY - dialogSkipUp / 2, dialogX + _dialogWidth - dialogSkipLeft, dialogTextY + lines.size() * _font->getFontHeight() + dialogSkipUp / 2 - 1);
				if (dialogOption.contains(mousePos)) {
					actualColor = _dialogColor2;
					dialogSelected = sentenceNumber;
					dialogCurrentText = dialogText;
				}

				for (uint j = 0; j < lines.size(); j++) {
					_font->drawString(_graph->_frontScreen, lines[j], dialogTextX, dialogTextY, _graph->_frontScreen->w, actualColor);
					dialogTextY += _font->getFontHeight();
				}
				dialogTextY += _dialogLineSpace;
			}
			do {
				c = *dialogText;
				dialogText++;
			} while (c);
		}

		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				keyHandler(event);
				break;
			case Common::EVENT_KEYUP:
				break;
			case Common::EVENT_MOUSEMOVE:
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (dialogSelected != -1) {
					dialogLeftMouseButton(dialogCurrentText, dialogSelected);
					_dialogFlag = false;
				}
				break;
			case Common::EVENT_RBUTTONDOWN:
				break;
			case Common::EVENT_LBUTTONUP:
				break;
			case Common::EVENT_RBUTTONUP:
				break;
			case Common::EVENT_QUIT:
				break;
			default:
				break;
			}
		}

		if (shouldQuit()) {
			return;
		}

		if (!_dialogFlag) {
			break;
		}

		getDebugger()->onFrame();
		_graph->update(_graph->_frontScreen);
		pause();
	}
	_dialogImage->free();
	delete _dialogImage;
	_dialogFlag = false;
	// cursor?
}

void PrinceEngine::dialogLeftMouseButton(byte *string, int dialogSelected) {
	_interpreter->setString(string);
	talkHero(0);

	int dialogDataValue = (int)READ_UINT32(_dialogData);
	int newValue = (dialogDataValue & (1 << dialogSelected)); // FIXME
	WRITE_UINT32(_dialogData, newValue); // FIXME

	_flags->setFlagValue(Flags::BOXSEL, dialogSelected + 1);
	setVoice(0, 28, dialogSelected + 1);

	int dialogOpt = dialogSelected << 4;
	_flags->setFlagValue(Flags::VOICE_H_LINE, _dialogOptLines[dialogOpt]);
	_flags->setFlagValue(Flags::VOICE_A_LINE, _dialogOptLines[dialogOpt + 1]);
	_flags->setFlagValue(Flags::VOICE_B_LINE, _dialogOptLines[dialogOpt + 2]);

	_interpreter->setString(_dialogOptAddr[dialogSelected]);
}

void PrinceEngine::talkHero(int slot) {
	// heroSlot = textSlot
	Text &text = _textSlots[slot];
	int lines = calcText((const char *)_interpreter->getString());
	int time = lines * 30;

	if (slot == 0) {
		text._color = 220; // test this
		_mainHero->_state = Hero::TALK;
		_mainHero->_talkTime = time;
		text._x = _mainHero->_middleX - _picWindowX;
		text._y = _mainHero->_middleY - _mainHero->_scaledFrameYSize;
	} else {
		text._color = 220; // test this !
		_secondHero->_state = Hero::TALK;
		_secondHero->_talkTime = time;
		text._x = _secondHero->_middleX - _picWindowX;
		text._y = _secondHero->_middleY - _secondHero->_scaledFrameYSize;
	}
	text._time = time; // changed by SETSPECVOICE?
	text._str = (const char *)_interpreter->getString();
	_interpreter->increaseString();
}

void PrinceEngine::doTalkAnim(int animNumber, int slot, AnimType animType) {
	Text &text = _textSlots[slot];
	int lines = calcText((const char *)_interpreter->getString());
	int time = lines * 30;
	if (animType == kNormalAnimation) {
		Anim &normAnim = _normAnimList[animNumber];
		if (normAnim._animData != nullptr) {
			if (!normAnim._state) {
				if (normAnim._currW && normAnim._currH) {
					text._color = _flags->getFlagValue(Flags::KOLOR);
					text._x = normAnim._currX + normAnim._currW / 2;
					text._y = normAnim._currY - 10;
				}
			}
		}
	} else if (animType == kBackgroundAnimation) {
		if ((uint)animNumber < _backAnimList.size()) {
			int currAnim = _backAnimList[animNumber]._seq._currRelative;
			Anim &backAnim = _backAnimList[animNumber].backAnims[currAnim];
			if (backAnim._animData != nullptr) {
				if (!backAnim._state) {
					if (backAnim._currW && backAnim._currH) {
						text._color = _flags->getFlagValue(Flags::KOLOR);
						text._x = backAnim._currX + backAnim._currW / 2;
						text._y = backAnim._currY - 10;
					}
				}
			}
		}
	} else {
		error("doTalkAnim() - wrong animType: %d", animType);
	}
	text._time = time;
	text._str = (const char *)_interpreter->getString();
	_interpreter->increaseString();
}

void PrinceEngine::freeNormAnim(int slot) {
	_normAnimList[slot]._state = 1;
	delete _normAnimList[slot]._animData;
	_normAnimList[slot]._animData = nullptr;
	delete _normAnimList[slot]._shadowData;
	_normAnimList[slot]._shadowData = nullptr;
	_normAnimList[slot]._currFrame = 0;
}

void PrinceEngine::freeAllNormAnims() {
	for (int i = 0; i < kMaxNormAnims; i++) {
		if (_normAnimList[i]._animData != nullptr) {
			delete _normAnimList[i]._animData;
		}
		if (_normAnimList[i]._shadowData != nullptr) {
			delete _normAnimList[i]._shadowData;
		}
	}
	_normAnimList.clear();
}

void PrinceEngine::mainLoop() {

	changeCursor(0);

	while (!shouldQuit()) {
		uint32 currentTime = _system->getMillis();

		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				keyHandler(event);
				break;
			case Common::EVENT_KEYUP:
				break;
			case Common::EVENT_MOUSEMOVE:
				break;
			case Common::EVENT_LBUTTONDOWN:
				leftMouseButton();
				break;
			case Common::EVENT_RBUTTONDOWN:
				rightMouseButton();
				break;
			case Common::EVENT_LBUTTONUP:
				break;
			case Common::EVENT_RBUTTONUP:
				break;
			case Common::EVENT_QUIT:
				break;
			default:
				break;
			}
		}

		if (shouldQuit())
			return;

		// TODO: Update all structures, animations, naks, heros etc.
		_mainHero -> showHero();
		if(_mainHero->_visible == 1) {
			_mainHero -> scrollHero();
		}

		_interpreter->step();

		drawScreen();
		_graph->update(_graph->_frontScreen);

		// Calculate the frame delay based off a desired frame time
		int delay = 1000/15 - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);

		_frameNr++;

		// inventory turning on:
		Common::Point mousePos = _system->getEventManager()->getMousePos();
		if (mousePos.y < 4 && !_showInventoryFlag) {
			_invCounter++;
		} else {
			_invCounter = 0;
		}
		if (_invCounter >= _invMaxCount) {
			if (_flags->getFlagValue(Flags::INVALLOWED) != 1) {
				// 29 - Basement, 50 - Map, 59 - Intro
				if (_locationNr != 29 && _locationNr != 50 && _locationNr != 59) {
					inventoryFlagChange(true);
				}
			}
		}

		if (_debugger->_locationNr != _locationNr)
			loadLocation(_debugger->_locationNr);
		if (_debugger->_cursorNr != _cursorNr)
			changeCursor(_debugger->_cursorNr);
	}
}

} // End of namespace Prince

/* vim: set tabstop=4 expandtab!: */
