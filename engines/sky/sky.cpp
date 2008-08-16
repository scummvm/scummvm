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
 * $URL$
 * $Id$
 *
 */



#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/timer.h"

#include "sky/control.h"
#include "sky/debug.h"
#include "sky/disk.h"
#include "sky/grid.h"
#include "sky/intro.h"
#include "sky/logic.h"
#include "sky/mouse.h"
#include "sky/music/adlibmusic.h"
#include "sky/music/gmmusic.h"
#include "sky/music/mt32music.h"
#include "sky/music/musicbase.h"
#include "sky/screen.h"
#include "sky/sky.h"
#include "sky/skydefs.h"
#include "sky/sound.h"
#include "sky/text.h"
#include "sky/compact.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "engines/metaengine.h"

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;
#endif

/*
 At the beginning the reverse engineers were happy, and did rejoice at
 their task, for the engine before them did shineth and was full of
 promise. But then they did look closer and see'th the aweful truth;
 it's code was assembly and messy (rareth was its comments). And so large
 were it's includes that did at first seem small; queereth also was its
 compact(s). Then they did findeth another version, and this was slightly
 different from the first. Then a third, and this was different again.
 All different, but not really better, for all were not really compatible.
 But, eventually, it did come to pass that Steel Sky was implemented on
 a modern platform. And the programmers looked and saw that it was indeed a
 miracle. But they were not joyous and instead did weep for nobody knew
 just what had been done. Except people who read the source. Hello.

 With apologies to the CD32 SteelSky file.
*/

static const PlainGameDescriptor skySetting =
	{"sky", "Beneath a Steel Sky" };

struct SkyVersion {
	int dinnerTableEntries;
	int dataDiskSize;
	const char *extraDesc;
	int version;
};

// TODO: Would be nice if Disk::determineGameVersion() used this table, too.
static const SkyVersion skyVersions[] = {
	{  243, -1, "pc gamer demo", 109 },
	{  247, -1, "floppy demo", 267 },
	{ 1404, -1, "floppy", 288 },
	{ 1413, -1, "floppy", 303 },
	{ 1445, 8830435, "floppy", 348 },
	{ 1445, -1, "floppy", 331 },
	{ 1711, -1, "cd demo", 365 },
	{ 5099, -1, "cd", 368 },
	{ 5097, -1, "cd", 372 },
	{ 0, 0, 0, 0 }
};

class SkyMetaEngine : public MetaEngine {
public:
	virtual const char *getName() const;
	virtual const char *getCopyright() const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual GameList getSupportedGames() const;
	virtual GameDescriptor findGame(const char *gameid) const;
	virtual GameList detectGames(const FSList &fslist) const;	

	virtual PluginError createInstance(OSystem *syst, Engine **engine) const;

	virtual SaveStateList listSaves(const char *target) const;
};

const char *SkyMetaEngine::getName() const {
	return "Beneath a Steel Sky";
}

const char *SkyMetaEngine::getCopyright() const {
	return "Beneath a Steel Sky (C) Revolution";
}

bool SkyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsDirectLoad);
}

GameList SkyMetaEngine::getSupportedGames() const {
	GameList games;
	games.push_back(skySetting);
	return games;
}

GameDescriptor SkyMetaEngine::findGame(const char *gameid) const {
	if (0 == scumm_stricmp(gameid, skySetting.gameid))
		return skySetting;
	return GameDescriptor();
}

GameList SkyMetaEngine::detectGames(const FSList &fslist) const {
	GameList detectedGames;
	bool hasSkyDsk = false;
	bool hasSkyDnr = false;
	int dinnerTableEntries = -1;
	int dataDiskSize = -1;

	// Iterate over all files in the given directory
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			const char *fileName = file->getName().c_str();

			if (0 == scumm_stricmp("sky.dsk", fileName)) {
				Common::File dataDisk;
				if (dataDisk.open(file->getPath())) {
					hasSkyDsk = true;
					dataDiskSize = dataDisk.size();
				}
			}

			if (0 == scumm_stricmp("sky.dnr", fileName)) {
				Common::File dinner;
				if (dinner.open(file->getPath())) {
					hasSkyDnr = true;
					dinnerTableEntries = dinner.readUint32LE();
				}
			}
		}
	}

	if (hasSkyDsk && hasSkyDnr) {
		// Match found, add to list of candidates, then abort inner loop.
		// The game detector uses US English by default. We want British
		// English to match the recorded voices better.
		GameDescriptor dg(skySetting.gameid, skySetting.description, Common::UNK_LANG, Common::kPlatformUnknown);
		const SkyVersion *sv = skyVersions;
		while (sv->dinnerTableEntries) {
			if (dinnerTableEntries == sv->dinnerTableEntries &&
				(sv->dataDiskSize == dataDiskSize || sv->dataDiskSize == -1)) {
				char buf[32];
				snprintf(buf, sizeof(buf), "v0.0%d %s", sv->version, sv->extraDesc);
				dg.updateDesc(buf);
				break;
			}
			++sv;
		}
		detectedGames.push_back(dg);
	}

	return detectedGames;
}

PluginError SkyMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	*engine = new Sky::SkyEngine(syst);
	return kNoError;
}

SaveStateList SkyMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	// Load the descriptions
	Common::StringList savenames;
	savenames.resize(MAX_SAVE_GAMES+1);

	Common::InSaveFile *inf;
	inf = saveFileMan->openForLoading("SKY-VM.SAV");
	if (inf != NULL) {
		char *tmpBuf =  new char[MAX_SAVE_GAMES * MAX_TEXT_LEN];
		char *tmpPtr = tmpBuf;
		inf->read(tmpBuf, MAX_SAVE_GAMES * MAX_TEXT_LEN);
		for (int i = 0; i < MAX_SAVE_GAMES; ++i) {
			savenames[i] = tmpPtr;
			tmpPtr += savenames[i].size() + 1;
		}
		delete inf;
		delete[] tmpBuf;
	}

	// Find all saves
	Common::StringList filenames;
	filenames = saveFileMan->listSavefiles("SKY-VM.???");
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	// Slot 0 is the autosave, if it exists.
	// TODO: Check for the existence of the autosave -- but this require us
	// to know which SKY variant we are looking at.
	saveList.insert_at(0, SaveStateDescriptor(0, "*AUTOSAVE*", ""));

	// Prepare the list of savestates by looping over all matching savefiles
	for (Common::StringList::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Extract the extension
		Common::String ext = file->c_str() + file->size() - 3;
		ext.toUppercase();
		if (isdigit(ext[0]) && isdigit(ext[1]) && isdigit(ext[2])){
			int slotNum = atoi(ext.c_str());
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				saveList.push_back(SaveStateDescriptor(slotNum+1, savenames[slotNum], *file));
				delete in;
			}
		}
	}

	return saveList;
}

#if PLUGIN_ENABLED_DYNAMIC(SKY)
	REGISTER_PLUGIN_DYNAMIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#endif

namespace Sky {

void *SkyEngine::_itemList[300];

SystemVars SkyEngine::_systemVars = {0, 0, 0, 0, 4316, 0, 0, false, false };

SkyEngine::SkyEngine(OSystem *syst)
	: Engine(syst), _fastMode(0), _debugger(0) {
}

SkyEngine::~SkyEngine() {

	_timer->removeTimerProc(&timerHandler);

	delete _skyLogic;
	delete _skySound;
	delete _skyMusic;
	delete _skyText;
	delete _skyMouse;
	delete _skyScreen;
	delete _debugger;
	delete _skyDisk;
	delete _skyControl;
	delete _skyCompact;
	if (_skyIntro)
		delete _skyIntro;

	for (int i = 0; i < 300; i++)
		if (_itemList[i])
			free(_itemList[i]);
}

GUI::Debugger *SkyEngine::getDebugger() {
	return _debugger;
}

void SkyEngine::initVirgin() {
	_skyScreen->showScreen(60110);
}

void SkyEngine::handleKey(void) {

	if (_keyPressed.keycode && _systemVars.paused) {
		_skySound->fnUnPauseFx();
		_systemVars.paused = false;
		_skyScreen->setPaletteEndian((uint8 *)_skyCompact->fetchCpt(SkyEngine::_systemVars.currentPalette));
	} else if (_keyPressed.flags == Common::KBD_CTRL) {
		if (_keyPressed.keycode == 'f')
			_fastMode ^= 1;
		else if (_keyPressed.keycode == 'g')
			_fastMode ^= 2;
		else if (_keyPressed.keycode == 'd')
			_debugger->attach();
	} else if (_keyPressed.keycode) {
		switch (_keyPressed.keycode) {
		case Common::KEYCODE_BACKQUOTE:
		case Common::KEYCODE_HASH:
			_debugger->attach();
			break;
		case Common::KEYCODE_F5:
			_skyControl->doControlPanel();
			break;

		case Common::KEYCODE_ESCAPE:
			if (!_systemVars.pastIntro)
				_skyControl->restartGame();
			break;

		case Common::KEYCODE_PERIOD:
			_skyMouse->logicClick();
			break;

		case Common::KEYCODE_p:
			_skyScreen->halvePalette();
			_skySound->fnPauseFx();
			_systemVars.paused = true;
			break;

		default:
			break;
		}
	}
	_keyPressed.reset();
}

int SkyEngine::go() {

	_keyPressed.reset();

	uint16 result = 0;
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			result = _skyControl->quickXRestore(ConfMan.getInt("save_slot"));
	}

	if (result != GAME_RESTORED) {
		bool introSkipped = false;
		if (_systemVars.gameVersion > 267) { // don't do intro for floppydemos
			_skyIntro = new Intro(_skyDisk, _skyScreen, _skyMusic, _skySound, _skyText, _mixer, _system);
			introSkipped = !_skyIntro->doIntro(_floppyIntro);
		}

		if (!quit()) {
			_skyLogic->initScreen0();
			if (introSkipped)
				_skyControl->restartGame();
		}
	}

	_lastSaveTime = _system->getMillis();

	uint32 delayCount = _system->getMillis();
	while (!quit()) {
		if (_debugger->isAttached())
			_debugger->onFrame();

		if (shouldPerformAutoSave(_lastSaveTime)) {
			if (_skyControl->loadSaveAllowed()) {
				_lastSaveTime = _system->getMillis();
				_skyControl->doAutoSave();
			} else
				_lastSaveTime += 30 * 1000; // try again in 30 secs
		}
		_skySound->checkFxQueue();
		_skyMouse->mouseEngine();
		handleKey();
		if (_systemVars.paused) {
			do {
				_system->updateScreen();
				delay(50);
				handleKey();
			} while (_systemVars.paused);
			delayCount = _system->getMillis();
		}

		_skyLogic->engine();
		_skyScreen->recreate();
		_skyScreen->spriteEngine();
		if (_debugger->showGrid()) {
			_skyScreen->showGrid(_skyLogic->_skyGrid->giveGrid(Logic::_scriptVariables[SCREEN]));
			_skyScreen->forceRefresh();
		}
		_skyScreen->flip();

		if (_fastMode & 2)
			delay(0);
		else if (_fastMode & 1)
			delay(10);
		else {
			delayCount += _systemVars.gameSpeed;
			int needDelay = delayCount - (int)_system->getMillis();
			if ((needDelay < 0) || (needDelay > 4 * _systemVars.gameSpeed)) {
				needDelay = 0;
				delayCount = _system->getMillis();
			}
			delay(needDelay);
		}
	}

	_skyControl->showGameQuitMsg();
	_skyMusic->stopMusic();
	ConfMan.flushToDisk();
	delay(1500);
	return _eventMan->shouldRTL();
}

int SkyEngine::init() {
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	if (ConfMan.getBool("sfx_mute")) {
		SkyEngine::_systemVars.systemFlags |= SF_FX_OFF;
	}
	if (ConfMan.getBool("music_mute")) {
		SkyEngine::_systemVars.systemFlags |= SF_MUS_OFF;
	}
	 _mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	 _mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	 _mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_floppyIntro = ConfMan.getBool("alt_intro");

	_skyDisk = new Disk();
	_skySound = new Sound(_mixer, _skyDisk, Audio::Mixer::kMaxChannelVolume);

	_systemVars.gameVersion = _skyDisk->determineGameVersion();

	int midiDriver = MidiDriver::detectMusicDriver(MDT_ADLIB | MDT_MIDI | MDT_PREFER_MIDI);
	if (midiDriver == MD_ADLIB) {
		_systemVars.systemFlags |= SF_SBLASTER;
		_skyMusic = new AdlibMusic(_mixer, _skyDisk);
	} else {
		_systemVars.systemFlags |= SF_ROLAND;
		if ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"))
			_skyMusic = new MT32Music(MidiDriver::createMidi(midiDriver), _skyDisk);
		else
			_skyMusic = new GmMusic(MidiDriver::createMidi(midiDriver), _skyDisk);
	}

	if (isCDVersion()) {
		if (ConfMan.hasKey("nosubtitles")) {
			warning("Configuration key 'nosubtitles' is deprecated. Use 'subtitles' instead");
			if (!ConfMan.getBool("nosubtitles"))
				_systemVars.systemFlags |= SF_ALLOW_TEXT;
		}

		if (ConfMan.getBool("subtitles"))
			_systemVars.systemFlags |= SF_ALLOW_TEXT;

		if (!ConfMan.getBool("speech_mute"))
			_systemVars.systemFlags |= SF_ALLOW_SPEECH;

	} else
		_systemVars.systemFlags |= SF_ALLOW_TEXT;

	_systemVars.systemFlags |= SF_PLAY_VOCS;
	_systemVars.gameSpeed = 50;

	_skyIntro = 0;
	_skyCompact = new SkyCompact();
	_skyText = new Text(_skyDisk, _skyCompact);
	_skyMouse = new Mouse(_system, _skyDisk, _skyCompact);
	_skyScreen = new Screen(_system, _skyDisk, _skyCompact);

	initVirgin();
	initItemList();
	loadFixedItems();
	_skyLogic = new Logic(_skyCompact, _skyScreen, _skyDisk, _skyText, _skyMusic, _skyMouse, _skySound);
	_skyMouse->useLogicInstance(_skyLogic);

	// initialize timer *after* _skyScreen has been initialized.
	_timer->installTimerProc(&timerHandler, 1000000 / 50, this); //call 50 times per second

	_skyControl = new Control(_saveFileMan, _skyScreen, _skyDisk, _skyMouse, _skyText, _skyMusic, _skyLogic, _skySound, _skyCompact, _system);
	_skyLogic->useControlInstance(_skyControl);

	switch (Common::parseLanguage(ConfMan.get("language"))) {
	case Common::EN_USA:
		_systemVars.language = SKY_USA;
		break;
	case Common::DE_DEU:
		_systemVars.language = SKY_GERMAN;
		break;
	case Common::FR_FRA:
		_systemVars.language = SKY_FRENCH;
		break;
	case Common::IT_ITA:
		_systemVars.language = SKY_ITALIAN;
		break;
	case Common::PT_BRA:
		_systemVars.language = SKY_PORTUGUESE;
		break;
	case Common::ES_ESP:
		_systemVars.language = SKY_SPANISH;
		break;
	case Common::SE_SWE:
		_systemVars.language = SKY_SWEDISH;
		break;
	case Common::EN_GRB:
		_systemVars.language = SKY_ENGLISH;
		break;
	default:
		_systemVars.language = SKY_ENGLISH;
		break;
	}

	if (!_skyDisk->fileExists(60600 + SkyEngine::_systemVars.language * 8)) {
		warning("The language you selected does not exist in your BASS version.");
		if (_skyDisk->fileExists(60600))
			SkyEngine::_systemVars.language = SKY_ENGLISH; // default to GB english if it exists..
		else if (_skyDisk->fileExists(60600 + SKY_USA * 8))
			SkyEngine::_systemVars.language = SKY_USA;		// try US english...
		else
			for (uint8 cnt = SKY_ENGLISH; cnt <= SKY_SPANISH; cnt++)
				if (_skyDisk->fileExists(60600 + cnt * 8)) {	// pick the first language we can find
					SkyEngine::_systemVars.language = cnt;
					break;
				}
	}

	_skyMusic->setVolume(ConfMan.getInt("music_volume") >> 1);

	_debugger = new Debugger(_skyLogic, _skyMouse, _skyScreen, _skyCompact);
	return 0;
}

void SkyEngine::initItemList() {

	//See List.asm for (cryptic) item # descriptions

	for (int i = 0; i < 300; i++)
		_itemList[i] = NULL;
}

void SkyEngine::loadFixedItems(void) {

	_itemList[49] = _skyDisk->loadFile(49);
	_itemList[50] = _skyDisk->loadFile(50);
	_itemList[73] = _skyDisk->loadFile(73);
	_itemList[262] = _skyDisk->loadFile(262);

	if (!isDemo()) {
		_itemList[36] = _skyDisk->loadFile(36);
		_itemList[263] = _skyDisk->loadFile(263);
		_itemList[264] = _skyDisk->loadFile(264);
		_itemList[265] = _skyDisk->loadFile(265);
		_itemList[266] = _skyDisk->loadFile(266);
		_itemList[267] = _skyDisk->loadFile(267);
		_itemList[269] = _skyDisk->loadFile(269);
		_itemList[271] = _skyDisk->loadFile(271);
		_itemList[272] = _skyDisk->loadFile(272);
	}
}

void *SkyEngine::fetchItem(uint32 num) {

	return _itemList[num];
}

void SkyEngine::timerHandler(void *refCon) {

	((SkyEngine *)refCon)->gotTimerTick();
}

void SkyEngine::gotTimerTick(void) {

	_skyScreen->handleTimer();
}

void SkyEngine::delay(int32 amount) {

	Common::Event event;

	uint32 start = _system->getMillis();
	_keyPressed.reset();

	if (amount < 0)
		amount = 0;

	do {
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				_keyPressed = event.kbd;
				break;
			case Common::EVENT_MOUSEMOVE:
				if (!(_systemVars.systemFlags & SF_MOUSE_LOCKED))
					_skyMouse->mouseMoved(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (!(_systemVars.systemFlags & SF_MOUSE_LOCKED))
					_skyMouse->mouseMoved(event.mouse.x, event.mouse.y);
				_skyMouse->buttonPressed(2);
				break;
			case Common::EVENT_RBUTTONDOWN:
				if (!(_systemVars.systemFlags & SF_MOUSE_LOCKED))
					_skyMouse->mouseMoved(event.mouse.x, event.mouse.y);
				_skyMouse->buttonPressed(1);
				break;
			default:
				break;
			}
		}

		if (amount > 0)
			_system->delayMillis((amount > 10) ? 10 : amount);

	} while (_system->getMillis() < start + amount);
}

bool SkyEngine::isDemo(void) {
	switch (_systemVars.gameVersion) {
	case 109: // pc gamer demo
	case 267: // floppy demo
	case 365: // cd demo
		return true;
	case 288:
	case 303:
	case 331:
	case 348:
	case 368:
	case 372:
		return false;
	default:
		error("Unknown game version %d", _systemVars.gameVersion);
	}
}

bool SkyEngine::isCDVersion(void) {

	switch (_systemVars.gameVersion) {
	case 109:
	case 267:
	case 288:
	case 303:
	case 331:
	case 348:
		return false;
	case 365:
	case 368:
	case 372:
		return true;
	default:
		error("Unknown game version %d", _systemVars.gameVersion);
	}
}

} // End of namespace Sky
