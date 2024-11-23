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

#include "common/formats/winexe.h"
#include "common/config-manager.h"
#include "common/crc.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/stream.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"
#include "graphics/wincursor.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"

#include "image/ani.h"
#include "image/bmp.h"
#include "image/icocur.h"

#include "math/utils.h"

#include "audio/decoders/wave.h"
#include "audio/decoders/vorbis.h"

#include "audio/audiostream.h"

#include "video/avi_decoder.h"

#include "gui/message.h"

#include "vcruise/ad2044_items.h"
#include "vcruise/ad2044_ui.h"
#include "vcruise/audio_player.h"
#include "vcruise/circuitpuzzle.h"
#include "vcruise/sampleloop.h"
#include "vcruise/midi_player.h"
#include "vcruise/menu.h"
#include "vcruise/runtime.h"
#include "vcruise/script.h"
#include "vcruise/textparser.h"
#include "vcruise/vcruise.h"


namespace VCruise {

struct InitialItemPlacement {
	uint roomNumber;
	uint screenNumber;
	uint itemID;
};

const InitialItemPlacement g_ad2044InitialItemPlacements[] = {
	{1, 0xb0, 18},	// Spoon
	{1, 0xb8, 24},	// Cigarette pack
	{1, 0xac, 27},	// Matches
	{1, 0x62, 2},	// "A" tag
	{1, 0x64, 58},  // Newspaper
};

struct AD2044Graphics {
	explicit AD2044Graphics(const Common::SharedPtr<Common::WinResources> &resources, bool lowQuality, const Graphics::PixelFormat &pixFmt);

	Common::SharedPtr<Graphics::Surface> invDownClicked;
	Common::SharedPtr<Graphics::Surface> invUpClicked;
	Common::SharedPtr<Graphics::Surface> musicClicked;
	Common::SharedPtr<Graphics::Surface> musicClickedDeep;
	Common::SharedPtr<Graphics::Surface> soundClicked;
	Common::SharedPtr<Graphics::Surface> soundClickedDeep;
	Common::SharedPtr<Graphics::Surface> exitClicked;
	Common::SharedPtr<Graphics::Surface> loadClicked;
	Common::SharedPtr<Graphics::Surface> saveClicked;
	Common::SharedPtr<Graphics::Surface> resizeClicked;
	Common::SharedPtr<Graphics::Surface> musicVolUpClicked;
	Common::SharedPtr<Graphics::Surface> musicVolDownClicked;
	Common::SharedPtr<Graphics::Surface> music;
	Common::SharedPtr<Graphics::Surface> musicVol;
	Common::SharedPtr<Graphics::Surface> sound;
	Common::SharedPtr<Graphics::Surface> soundVol;
	Common::SharedPtr<Graphics::Surface> musicDisabled;
	Common::SharedPtr<Graphics::Surface> musicVolDisabled;
	Common::SharedPtr<Graphics::Surface> soundDisabled;
	Common::SharedPtr<Graphics::Surface> soundVolDisabled;
	Common::SharedPtr<Graphics::Surface> examine;
	Common::SharedPtr<Graphics::Surface> examineDisabled;
	Common::SharedPtr<Graphics::Surface> invPage[8];


	void loadGraphic(Common::SharedPtr<Graphics::Surface> AD2044Graphics::*field, const Common::String &resName);
	Common::SharedPtr<Graphics::Surface> loadGraphic(const Common::String &resName) const;
	void finishLoading();

private:
	AD2044Graphics() = delete;

	bool _lowQuality;
	Common::SharedPtr<Common::WinResources> _resources;
	Common::Array<Common::WinResourceID> _resourceIDs;
	const Graphics::PixelFormat _pixFmt;
};

AD2044Graphics::AD2044Graphics(const Common::SharedPtr<Common::WinResources> &resources, bool lowQuality, const Graphics::PixelFormat &pixFmt)
	: _resources(resources), _lowQuality(lowQuality), _pixFmt(pixFmt) {

	_resourceIDs = resources->getIDList(Common::kWinBitmap);
}

void AD2044Graphics::loadGraphic(Common::SharedPtr<Graphics::Surface> AD2044Graphics::*field, const Common::String &resNameBase) {
	this->*field = loadGraphic(resNameBase);
}

Common::SharedPtr<Graphics::Surface> AD2044Graphics::loadGraphic(const Common::String &resNameBase) const {
	Common::String resName = _lowQuality ? (Common::String("D") + resNameBase) : resNameBase;

	const Common::WinResourceID *resID = nullptr;
	for (const Common::WinResourceID &resIDCandidate : _resourceIDs) {
		if (resIDCandidate.getString() == resName) {
			resID = &resIDCandidate;
			break;
		}
	}

	if (!resID)
		error("Couldn't find bitmap graphic %s", resName.c_str());

	Common::ScopedPtr<Common::SeekableReadStream> bmpResource(_resources->getResource(Common::kWinBitmap, *resID));

	if (!bmpResource)
		error("Couldn't open bitmap graphic %s", resName.c_str());

	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(*bmpResource))
		error("Couldn't load bitmap graphic %s", resName.c_str());

	const Graphics::Surface *bmpSurf = decoder.getSurface();

	Common::SharedPtr<Graphics::Surface> surf(bmpSurf->convertTo(_pixFmt, decoder.getPalette(), decoder.getPaletteColorCount()), Graphics::SurfaceDeleter());
	return surf;
}

void AD2044Graphics::finishLoading() {
	_resources.reset();
}

struct CodePageGuess {
	Common::CodePage codePage;
	Runtime::CharSet charSet;
	const char *searchString;
	const char *languageName;
};

class RuntimeMenuInterface : public MenuInterface {
public:
	explicit RuntimeMenuInterface(Runtime *runtime);

	void commitRect(const Common::Rect &rect) const override;
	bool popOSEvent(OSEvent &evt) const override;
	Graphics::Surface *getUIGraphic(uint index) const override;
	Graphics::ManagedSurface *getMenuSurface() const override;
	bool hasDefaultSave() const override;
	bool hasAnySave() const override;
	bool isInGame() const override;
	Common::Point getMouseCoordinate() const override;
	void restartGame() const override;
	void goToCredits() const override;
	void changeMenu(MenuPage *newPage) const override;
	void quitGame() const override;
	void quitToMenu() const override;
	bool canSave() const override;
	bool reloadFromCheckpoint() const override;
	void setMusicMute(bool muted) const override;

	void drawLabel(Graphics::ManagedSurface *surface, const Common::String &labelID, const Common::Rect &contentRect) const override;

private:
	Runtime *_runtime;
};


RuntimeMenuInterface::RuntimeMenuInterface(Runtime *runtime) : _runtime(runtime) {
}

void RuntimeMenuInterface::commitRect(const Common::Rect &rect) const {
	_runtime->commitSectionToScreen(_runtime->_fullscreenMenuSection, rect);
}

bool RuntimeMenuInterface::popOSEvent(OSEvent &evt) const {
	return _runtime->popOSEvent(evt);
}

Graphics::Surface *RuntimeMenuInterface::getUIGraphic(uint index) const {
	if (index >= _runtime->_uiGraphics.size())
		return nullptr;
	return _runtime->_uiGraphics[index].get();
}

Graphics::ManagedSurface *RuntimeMenuInterface::getMenuSurface() const {
	return _runtime->_fullscreenMenuSection.surf.get();
}

bool RuntimeMenuInterface::hasDefaultSave() const {
	return static_cast<VCruiseEngine *>(g_engine)->hasDefaultSave();
}

bool RuntimeMenuInterface::hasAnySave() const {
	return static_cast<VCruiseEngine *>(g_engine)->hasAnySave();
}

bool RuntimeMenuInterface::isInGame() const {
	return _runtime->_isInGame;
}

Common::Point RuntimeMenuInterface::getMouseCoordinate() const {
	return _runtime->_mousePos;
}

void RuntimeMenuInterface::restartGame() const {
	Common::SharedPtr<SaveGameSnapshot> snapshot = _runtime->generateNewGameSnapshot();

	_runtime->_mostRecentValidSaveState = snapshot;
	_runtime->restoreSaveGameSnapshot();
}

void RuntimeMenuInterface::goToCredits() const {
	_runtime->clearScreen();

	// In Schizm, exiting credits doesn't transition to the main menu screen,
	// so we must force a screen change for when the user clicks Credits after
	// leaving the credits screen
	_runtime->_forceScreenChange = true;

	if (_runtime->_gameID == GID_REAH)
		_runtime->changeToScreen(40, 0xa1);
	else if (_runtime->_gameID == GID_SCHIZM)
		_runtime->changeToScreen(1, 0xb2);
	else
		error("Don't know what screen to go to for credits for this game");
}

void RuntimeMenuInterface::changeMenu(MenuPage *newPage) const {
	_runtime->changeToMenuPage(newPage);
}

void RuntimeMenuInterface::quitToMenu() const {
	_runtime->quitToMenu();
}

void RuntimeMenuInterface::quitGame() const {
	Common::Event evt;
	evt.type = Common::EVENT_QUIT;

	g_engine->getEventManager()->pushEvent(evt);
}

bool RuntimeMenuInterface::canSave() const {
	return _runtime->canSave(false);
}

bool RuntimeMenuInterface::reloadFromCheckpoint() const {
	if (!_runtime->canSave(false))
		return false;

	_runtime->restoreSaveGameSnapshot();
	return true;
}

void RuntimeMenuInterface::setMusicMute(bool muted) const {
	_runtime->setMusicMute(muted);
}

void RuntimeMenuInterface::drawLabel(Graphics::ManagedSurface *surface, const Common::String &labelID, const Common::Rect &contentRect) const {
	_runtime->drawLabel(surface, labelID, contentRect);
}

AnimationDef::AnimationDef() : animNum(0), firstFrame(0), lastFrame(0) {
}

InteractionDef::InteractionDef() : objectType(0), interactionID(0) {
}

MapLoader::~MapLoader() {
}

Common::SharedPtr<MapScreenDirectionDef> MapLoader::loadScreenDirectionDef(Common::ReadStream &stream) {
	byte screenDefHeader[16];

	if (stream.read(screenDefHeader, 16) != 16)
		error("Error reading screen def header");

	uint16 numInteractions = READ_LE_UINT16(screenDefHeader + 0);

	if (numInteractions > 0) {
		Common::SharedPtr<MapScreenDirectionDef> screenDirectionDef(new MapScreenDirectionDef());
		screenDirectionDef->interactions.resize(numInteractions);

		for (uint i = 0; i < numInteractions; i++) {
			InteractionDef &idef = screenDirectionDef->interactions[i];

			byte interactionData[12];
			if (stream.read(interactionData, 12) != 12)
				error("Error reading interaction data");

			idef.rect = Common::Rect(READ_LE_INT16(interactionData + 0), READ_LE_INT16(interactionData + 2), READ_LE_INT16(interactionData + 4), READ_LE_INT16(interactionData + 6));
			idef.interactionID = READ_LE_UINT16(interactionData + 8);
			idef.objectType = READ_LE_UINT16(interactionData + 10);
		}

		return screenDirectionDef;
	}

	return nullptr;
}

class ReahSchizmMapLoader : public MapLoader {
public:
	ReahSchizmMapLoader();

	void setRoomNumber(uint roomNumber) override;
	const MapScreenDirectionDef *getScreenDirection(uint screen, uint direction) override;
	void unload() override;

private:
	void load();

	static const uint kNumScreens = 96;
	static const uint kFirstScreen = 0xa0;

	uint _roomNumber;
	bool _isLoaded;

	Common::SharedPtr<MapScreenDirectionDef> _screenDirections[kNumScreens][kNumDirections];
};

ReahSchizmMapLoader::ReahSchizmMapLoader() : _roomNumber(0), _isLoaded(false) {
}

void ReahSchizmMapLoader::setRoomNumber(uint roomNumber) {
	if (_roomNumber != roomNumber)
		unload();

	_roomNumber = roomNumber;
}

const MapScreenDirectionDef *ReahSchizmMapLoader::getScreenDirection(uint screen, uint direction) {
	if (screen < kFirstScreen)
		return nullptr;

	screen -= kFirstScreen;

	if (screen >= kNumScreens)
		return nullptr;

	if (!_isLoaded)
		load();

	return _screenDirections[screen][direction].get();
}

void ReahSchizmMapLoader::load() {
	// This is loaded even if the open fails
	_isLoaded = true;

	Common::Path mapFileName(Common::String::format("Map/Room%02i.map", static_cast<int>(_roomNumber)));
	Common::File mapFile;

	if (!mapFile.open(mapFileName))
		return;

	byte screenDefOffsets[kNumScreens * kNumDirections * 4];

	if (!mapFile.seek(16))
		error("Error skipping map file header");

	if (mapFile.read(screenDefOffsets, sizeof(screenDefOffsets)) != sizeof(screenDefOffsets))
		error("Error reading map offset table");

	for (uint screen = 0; screen < kNumScreens; screen++) {
		for (uint direction = 0; direction < kNumDirections; direction++) {
			uint32 offset = READ_LE_UINT32(screenDefOffsets + (kNumDirections * screen + direction) * 4);
			if (!offset)
				continue;

			// QUIRK: The stone game in the tower in Reah (Room 06) has two 0cb screens and the second one is damaged,
			// so it must be ignored.
			if (!_screenDirections[screen][direction]) {
				if (!mapFile.seek(offset))
					error("Error seeking to screen data");

				_screenDirections[screen][direction] = loadScreenDirectionDef(mapFile);
			}
		}
	}
}

void ReahSchizmMapLoader::unload() {
	for (uint screen = 0; screen < kNumScreens; screen++)
		for (uint direction = 0; direction < kNumDirections; direction++)
			_screenDirections[screen][direction].reset();

	_isLoaded = false;
}

class AD2044MapLoader : public MapLoader {
public:
	AD2044MapLoader();

	void setRoomNumber(uint roomNumber) override;
	const MapScreenDirectionDef *getScreenDirection(uint screen, uint direction) override;
	void unload() override;

private:
	struct ScreenOverride {
		uint roomNumber;
		uint screenNumber;
		int actualMapFileID;
	};

	void load();

	static const uint kFirstScreen = 0xa0;

	uint _roomNumber;
	uint _screenNumber;
	bool _isLoaded;

	Common::SharedPtr<MapScreenDirectionDef> _currentMap;

	static const ScreenOverride sk_screenOverrides[];
};

const AD2044MapLoader::ScreenOverride AD2044MapLoader::sk_screenOverrides[] = {
	// Room 1
	{1, 0xb6, 145},	// After pushing the button to open the capsule
	{1, 0x66, 138}, // Looking at banner
	{1, 0x6a, 142},	// Opening an apple on the table
	{1, 0x6b, 143}, // Clicking the tablet in the apple
	{1, 0x6c, 144}, // Table facing the center of the room with soup bowl empty

	// Room 23
	{23, 0xa3, 103}, // Looking at high mirror
	{23, 0xa4, 104}, // After taking mirror

	{23, 0xb9, 125}, // Bathroom looking down the stairs
	//{23, 0xb9, 126}, // ???
	{23, 0xbb, 127}, // Bathroom entry point
	{23, 0xbc, 128}, // Sink
	{23, 0xbd, 129}, // Looking at toilet, seat down
	{23, 0xbe, 130}, // Looking at toilet, seat up
	{23, 0x61, 133}, // Bathroom looking at boots
	{23, 0x62, 134}, // Looking behind boots
	{23, 0x63, 135}, // Standing behind toilet looking at sink
	{23, 0x64, 136}, // Looking under toilet
};

AD2044MapLoader::AD2044MapLoader() : _roomNumber(0), _screenNumber(0), _isLoaded(false) {
}

void AD2044MapLoader::setRoomNumber(uint roomNumber) {
	if (_roomNumber != roomNumber)
		unload();

	_roomNumber = roomNumber;
}

const MapScreenDirectionDef *AD2044MapLoader::getScreenDirection(uint screen, uint direction) {
	if (screen != _screenNumber)
		unload();

	_screenNumber = screen;

	if (!_isLoaded)
		load();

	return _currentMap.get();
}

void AD2044MapLoader::load() {
	// This is loaded even if the open fails
	_isLoaded = true;

	int scrFileID = -1;

	for (const ScreenOverride &screenOverride : sk_screenOverrides) {
		if (screenOverride.roomNumber == _roomNumber && screenOverride.screenNumber == _screenNumber) {
			scrFileID = screenOverride.actualMapFileID;
			break;
		}
	}

	if (_roomNumber == 87) {
		uint highDigit = (_screenNumber & 0xf0) >> 4;
		uint lowDigit = _screenNumber & 0x0f;

		scrFileID = 8700 + static_cast<int>(highDigit * 10u + lowDigit);
	}

	if (scrFileID < 0) {
		if (_screenNumber < kFirstScreen)
			return;

		uint adjustedScreenNumber = _screenNumber - kFirstScreen;

		if (adjustedScreenNumber > 99)
			return;

		scrFileID = static_cast<int>(_roomNumber * 100u + adjustedScreenNumber);
	}

	Common::Path mapFileName(Common::String::format("map/SCR%i.MAP", scrFileID));
	Common::File mapFile;

	debug(1, "Loading screen map %s", mapFileName.toString(Common::Path::kNativeSeparator).c_str());

	if (!mapFile.open(mapFileName)) {
		error("Couldn't resolve map file for room %u screen %x", _roomNumber, _screenNumber);
	}

	_currentMap = loadScreenDirectionDef(mapFile);
}

void AD2044MapLoader::unload() {
	_currentMap.reset();
	_isLoaded = false;
}


ScriptEnvironmentVars::ScriptEnvironmentVars() : lmb(false), lmbDrag(false), esc(false), exitToMenu(false), animChangeSet(false), isEntryScript(false), puzzleWasSet(false),
	panInteractionID(0), clickInteractionID(0), fpsOverride(0), lastHighlightedItem(0), animChangeFrameOffset(0), animChangeNumFrames(0) {
}

OSEvent::OSEvent() : type(kOSEventTypeInvalid), keymappedEvent(kKeymappedEventNone), timestamp(0) {
}

void Runtime::RenderSection::init(const Common::Rect &paramRect, const Graphics::PixelFormat &fmt) {
	rect = paramRect;
	pixFmt = fmt;
	if (paramRect.isEmpty())
		surf.reset();
	else {
		surf.reset(new Graphics::ManagedSurface(paramRect.width(), paramRect.height(), fmt));
		surf->fillRect(Common::Rect(0, 0, surf->w, surf->h), 0xffffffff);
	}
}

Runtime::StackValue::ValueUnion::ValueUnion() {
}

Runtime::StackValue::ValueUnion::ValueUnion(StackInt_t iVal) : i(iVal) {
}

Runtime::StackValue::ValueUnion::ValueUnion(const Common::String &strVal) : s(strVal) {
}

Runtime::StackValue::ValueUnion::ValueUnion(Common::String &&strVal) : s(Common::move(strVal)) {
}

Runtime::StackValue::ValueUnion::~ValueUnion() {
}

Runtime::StackValue::StackValue() : type(kNumber), value(0) {
	new (&value) ValueUnion(0);
}

Runtime::StackValue::StackValue(const StackValue &other) : type(kNumber), value(0) {
	(*this) = other;
}

Runtime::StackValue::StackValue(StackValue &&other) : type(kNumber), value(0) {
	(*this) = Common::move(other);
}

Runtime::StackValue::StackValue(StackInt_t i) : type(kNumber), value(i) {
}

Runtime::StackValue::StackValue(const Common::String &str) : type(kString), value(str) {
}

Runtime::StackValue::StackValue(Common::String &&str) : type(kString), value(Common::move(str)) {
}

Runtime::StackValue::~StackValue() {
	value.~ValueUnion();
}

Runtime::StackValue &Runtime::StackValue::operator=(const StackValue &other) {
	value.~ValueUnion();

	if (other.type == StackValue::kNumber)
		new (&value) ValueUnion(other.value.i);

	if (other.type == StackValue::kString)
		new (&value) ValueUnion(other.value.s);

	type = other.type;

	return *this;
}

Runtime::StackValue &Runtime::StackValue::operator=(StackValue &&other) {
	value.~ValueUnion();

	if (other.type == StackValue::kNumber)
		new (&value) ValueUnion(other.value.i);

	if (other.type == StackValue::kString)
		new (&value) ValueUnion(Common::move(other.value.s));

	type = other.type;

	return *this;
}

Runtime::CallStackFrame::CallStackFrame() : _nextInstruction(0) {
}

Runtime::Gyro::Gyro() {
	reset();
}

void Runtime::Gyro::reset() {
	currentState = 0;
	requiredState = 0;
	wrapAround = false;
	requireState = false;
	numPreviousStates = 0;
	numPreviousStatesRequired = 0;

	for (uint i = 0; i < kMaxPreviousStates; i++) {
		previousStates[i] = 0;
		requiredPreviousStates[i] = 0;
	}
}

void Runtime::Gyro::logState() {
	if (numPreviousStatesRequired > 0) {
		if (numPreviousStates < numPreviousStatesRequired)
			numPreviousStates++;
		else {
			for (uint i = 1; i < numPreviousStates; i++)
				previousStates[i - 1] = previousStates[i];
		}

		previousStates[numPreviousStates - 1] = currentState;
	}
}

Runtime::GyroState::GyroState() {
	reset();
}

void Runtime::GyroState::reset() {
	for (uint i = 0; i < kNumGyros; i++)
		gyros[i].reset();

	completeInteraction = 0;
	failureInteraction = 0;
	frameSeparation = 1;

	activeGyro = 0;
	dragMargin = 0;
	maxValue = 0;

	negAnim = AnimationDef();
	posAnim = AnimationDef();
	isVertical = false;

	dragBasePoint = Common::Point(0, 0);
	dragBaseState = 0;
	dragCurrentState = 0;
	isWaitingForAnimation = false;
}

Runtime::SubtitleDef::SubtitleDef() : color{0, 0, 0}, unknownValue1(0), durationInDeciseconds(0) {
}

SfxPlaylistEntry::SfxPlaylistEntry() : frame(0), balance(0), volume(0), isUpdate(false) {
}

SfxPlaylist::SfxPlaylist() {
}

SfxData::SfxData() {
}

void SfxData::reset() {
	playlists.clear();
	sounds.clear();
}

void SfxData::load(Common::SeekableReadStream &stream, Audio::Mixer *mixer) {
	Common::INIFile iniFile;

	iniFile.allowNonEnglishCharacters();
	iniFile.suppressValuelessLineWarning();

	if (!iniFile.loadFromStream(stream))
		warning("SfxData::load failed to parse INI file");

	const Common::INIFile::Section *samplesSection = nullptr;
	const Common::INIFile::Section *playlistsSection = nullptr;
	const Common::INIFile::Section *presetsSection = nullptr;

	Common::INIFile::SectionList sections = iniFile.getSections();	// Why does this require a copy?  Sigh.

	for (const Common::INIFile::Section &section : sections) {
		if (section.name == "samples")
			samplesSection = &section;
		else if (section.name == "playlists")
			playlistsSection = &section;
		else if (section.name == "presets")
			presetsSection = &section;
	}

	Common::HashMap<Common::String, Common::String> presets;

	if (presetsSection) {
		for (const Common::INIFile::KeyValue &keyValue : presetsSection->keys)
			presets.setVal(keyValue.key, keyValue.value);
	}

	if (samplesSection) {
		for (const Common::INIFile::KeyValue &keyValue : samplesSection->keys) {
			Common::SharedPtr<SfxSound> sample(new SfxSound());

			// Fix up the path delimiter
			Common::String sfxPath = keyValue.value;
			for (char &c : sfxPath) {
				if (c == '\\')
					c = '/';
			}

			size_t commentPos = sfxPath.find(';');
			if (commentPos != Common::String::npos) {
				sfxPath = sfxPath.substr(0, commentPos);
				sfxPath.trim();
			}

			Common::Path sfxPath_("Sfx/");
			sfxPath_.appendInPlace(sfxPath);

			Common::File f;
			if (!f.open(sfxPath_)) {
				warning("SfxData::load: Could not open sample file '%s'", sfxPath_.toString(Common::Path::kNativeSeparator).c_str());
				continue;
			}

			int64 size = f.size();
			if (size <= 0 || size > 0x1fffffffu) {
				warning("SfxData::load: File is oversized for some reason");
				continue;
			}

			sample->soundData.resize(static_cast<uint>(size));
			if (f.read(&sample->soundData[0], static_cast<uint32>(size)) != size) {
				warning("SfxData::load: Couldn't read file");
				continue;
			}

			sample->memoryStream.reset(new Common::MemoryReadStream(&sample->soundData[0], static_cast<uint32>(size)));
			sample->audioStream.reset(Audio::makeWAVStream(sample->memoryStream.get(), DisposeAfterUse::NO));
			sample->audioPlayer.reset(new AudioPlayer(mixer, sample->audioStream, Audio::Mixer::kSFXSoundType));

			this->sounds[keyValue.key] = sample;
		}
	}

	if (playlistsSection) {
		Common::SharedPtr<SfxPlaylist> playlist;

		for (const Common::INIFile::KeyValue &keyValue : playlistsSection->keys) {
			const Common::String &baseKey = keyValue.key;

			// Strip inline comments
			uint keyValidLength = 0;
			for (uint i = 0; i < baseKey.size(); i++) {
				char c = baseKey[i];
				if ((c & 0x80) == 0 && ((c & 0x7f) <= ' '))
					continue;

				if (c == ';')
					break;

				keyValidLength = i + 1;
			}

			Common::String key = baseKey.substr(0, keyValidLength);


			if (key.size() == 0)
				continue;

			if (key.size() >= 2 && key.firstChar() == '\"' && key.lastChar() == '\"') {
				if (!playlist) {
					warning("Found playlist entry outside of a playlist");
					continue;
				}

				Common::String workKey = key.substr(1, key.size() - 2);

				Common::Array<Common::String> tokens;
				for (;;) {
					uint32 spaceSpanStart = workKey.find(' ');

					if (spaceSpanStart == Common::String::npos) {
						tokens.push_back(workKey);
						break;
					}

					uint32 spaceSpanEnd = spaceSpanStart;

					while (spaceSpanEnd < workKey.size() && workKey[spaceSpanEnd] == ' ')
						spaceSpanEnd++;

					tokens.push_back(workKey.substr(0, spaceSpanStart));
					workKey = workKey.substr(spaceSpanEnd, workKey.size() - spaceSpanEnd);
				}

				// Strip leading and trailing spaces
				while (tokens.size() > 0) {
					if (tokens[0].empty()) {
						tokens.remove_at(0);
						continue;
					}

					uint lastIndex = tokens.size() - 1;
					if (tokens[lastIndex].empty()) {
						tokens.remove_at(lastIndex);
						continue;
					}

					break;
				}

				if (tokens.size() != 4) {
					warning("Found unusual playlist entry: %s", key.c_str());
					continue;
				}

				if (!presets.empty()) {
					for (uint tokenIndex = 0; tokenIndex < tokens.size(); tokenIndex++) {
						// Ignore presets for the sound name.  This fixes some breakage in e.g. Anim0134.sfx using elevator as both a sample and preset.
						if (tokenIndex == 1)
							continue;

						Common::String &tokenRef = tokens[tokenIndex];
						Common::HashMap<Common::String, Common::String>::const_iterator presetIt = presets.find(tokenRef);
						if (presetIt != presets.end())
							tokenRef = presetIt->_value;
					}
				}

				unsigned int frameNum = 0;
				int balance = 0;
				int volume = 0;

				if (!sscanf(tokens[0].c_str(), "%u", &frameNum) || !sscanf(tokens[2].c_str(), "%i", &balance) || !sscanf(tokens[3].c_str(), "%i", &volume)) {
					warning("Malformed playlist entry: %s", key.c_str());
					continue;
				}

				bool isUpdate = false;
				Common::String soundName = tokens[1];
				if (soundName.size() >= 1 && soundName[0] == '*') {
					soundName = soundName.substr(1);
					isUpdate = true;
				}

				SoundMap_t::const_iterator soundIt = this->sounds.find(soundName);
				if (soundIt == this->sounds.end()) {
					warning("Playlist entry referenced non-existent sound: %s", soundName.c_str());
					continue;
				}

				SfxPlaylistEntry plEntry;
				plEntry.balance = balance;
				plEntry.frame = frameNum;
				plEntry.volume = volume;
				plEntry.sample = soundIt->_value;
				plEntry.isUpdate = isUpdate;

				playlist->entries.push_back(plEntry);
			} else {
				playlist.reset(new SfxPlaylist());
				this->playlists[key] = playlist;
			}
		}
	}
}

SoundCache::SoundCache() : isLoopActive(false) {
}

SoundCache::~SoundCache() {
	// Dispose player first so playback stops
	this->player.reset();

	// Dispose loopingStream before stream because stream is not refcounted by loopingStream so we need to avoid late free
	this->loopingStream.reset();

	this->stream.reset();
}

SoundInstance::SoundInstance()
	: id(0), rampStartVolume(0), rampEndVolume(0), rampRatePerMSec(0), rampStartTime(0), rampTerminateOnCompletion(false),
	  volume(0), balance(0), effectiveBalance(0), effectiveVolume(0), is3D(false), isLooping(false), isSpeech(false), restartWhenAudible(false), tryToLoopWhenRestarted(false),
	  x(0), y(0), startTime(0), endTime(0), duration(0) {
}

SoundInstance::~SoundInstance() {
}

RandomAmbientSound::RandomAmbientSound() : volume(0), balance(0), frequency(0), sceneChangesRemaining(0) {
}

void RandomAmbientSound::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(name.size());
	stream->writeString(name);

	stream->writeSint32BE(volume);
	stream->writeSint32BE(balance);

	stream->writeUint32BE(frequency);
	stream->writeUint32BE(sceneChangesRemaining);
}

void RandomAmbientSound::read(Common::ReadStream *stream) {
	uint nameLen = stream->readUint32BE();
	if (stream->eos() || stream->err())
		nameLen = 0;

	name = stream->readString(0, nameLen);

	volume = stream->readSint32BE();
	balance = stream->readSint32BE();

	frequency = stream->readUint32BE();
	sceneChangesRemaining = stream->readUint32BE();
}

TriggeredOneShot::TriggeredOneShot() : soundID(0), uniqueSlot(0) {
}

bool TriggeredOneShot::operator==(const TriggeredOneShot &other) const {
	return soundID == other.soundID && uniqueSlot == other.uniqueSlot;
}

bool TriggeredOneShot::operator!=(const TriggeredOneShot &other) const {
	return !((*this) == other);
}

void TriggeredOneShot::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(soundID);
	stream->writeUint32BE(uniqueSlot);
}

void TriggeredOneShot::read(Common::ReadStream *stream) {
	soundID = stream->readUint32BE();
	uniqueSlot = stream->readUint32BE();
}

ScoreSectionDef::ScoreSectionDef() : volumeOrDurationInSeconds(0) {
}

StartConfigDef::StartConfigDef() : disc(0), room(0), screen(0), direction(0) {
}

StaticAnimParams::StaticAnimParams() : initialDelay(0), repeatDelay(0), lockInteractions(false) {
}

void StaticAnimParams::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(initialDelay);
	stream->writeUint32BE(repeatDelay);
	stream->writeByte(lockInteractions ? 1 : 0);
}

void StaticAnimParams::read(Common::ReadStream *stream) {
	initialDelay = stream->readUint32BE();
	repeatDelay = stream->readUint32BE();
	lockInteractions = (stream->readByte() != 0);
}

StaticAnimation::StaticAnimation() : currentAlternation(0), nextStartTime(0) {
}

FrameData::FrameData() : areaID{0, 0, 0, 0}, areaFrameIndex(0), frameIndex(0), frameType(0), roomNumber(0) {
}

FrameData2::FrameData2() : x(0), y(0), angle(0), frameNumberInArea(0), unknown(0) {
}

AnimFrameRange::AnimFrameRange() : animationNum(0), firstFrame(0), lastFrame(0) {
}

SoundParams3D::SoundParams3D() : minRange(0), maxRange(0), unknownRange(0) {
}

void SoundParams3D::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(minRange);
	stream->writeUint32BE(maxRange);
	stream->writeUint32BE(unknownRange);
}

void SoundParams3D::read(Common::ReadStream *stream) {
	minRange = stream->readUint32BE();
	maxRange = stream->readUint32BE();
	unknownRange = stream->readUint32BE();
}

InventoryItem::InventoryItem() : itemID(0), highlighted(false) {
}

Fraction::Fraction() : numerator(0), denominator(1) {
}

Fraction::Fraction(uint pNumerator, uint pDenominator) : numerator(pNumerator), denominator(pDenominator) {
}

SaveGameSwappableState::InventoryItem::InventoryItem() : itemID(0), highlighted(false) {
}

void SaveGameSwappableState::InventoryItem::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(itemID);
	stream->writeByte(highlighted ? 1 : 0);
}

void SaveGameSwappableState::InventoryItem::read(Common::ReadStream *stream) {
	itemID = stream->readUint32BE();
	highlighted = (stream->readByte() != 0);
}

SaveGameSwappableState::Sound::Sound() : id(0), volume(0), balance(0), is3D(false), isLooping(false), tryToLoopWhenRestarted(false), isSpeech(false), x(0), y(0) {
}

void SaveGameSwappableState::Sound::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(name.size());
	stream->writeString(name);

	stream->writeUint32BE(id);
	stream->writeSint32BE(volume);
	stream->writeSint32BE(balance);

	stream->writeByte(is3D ? 1 : 0);
	stream->writeByte(isLooping ? 1 : 0);
	stream->writeByte(tryToLoopWhenRestarted ? 1 : 0);
	stream->writeByte(isSpeech ? 1 : 0);

	stream->writeSint32BE(x);
	stream->writeSint32BE(y);

	params3D.write(stream);
}

void SaveGameSwappableState::Sound::read(Common::ReadStream *stream, uint saveGameVersion) {
	uint nameLen = stream->readUint32BE();

	if (stream->eos() || stream->err() || nameLen > 256)
		nameLen = 0;

	name = stream->readString(0, nameLen);

	id = stream->readUint32BE();
	volume = stream->readSint32BE();
	balance = stream->readSint32BE();

	is3D = (stream->readByte() != 0);
	isLooping = (stream->readByte() != 0);

	if (saveGameVersion >= 8)
		tryToLoopWhenRestarted = (stream->readByte() != 0);
	else
		tryToLoopWhenRestarted = false;

	isSpeech = (stream->readByte() != 0);

	x = stream->readSint32BE();
	y = stream->readSint32BE();

	params3D.read(stream);
}

SaveGameSwappableState::SaveGameSwappableState() : roomNumber(0), screenNumber(0), direction(0), disc(0), havePendingPostSwapScreenReset(false),
												   musicTrack(0), musicVolume(100), musicActive(true), musicMuteDisabled(false), animVolume(100),
												   loadedAnimation(0), animDisplayingFrame(0), haveIdleAnimationLoop(false), idleAnimNum(0), idleFirstFrame(0), idleLastFrame(0)
{
}

SaveGameSnapshot::PagedInventoryItem::PagedInventoryItem() : page(0), slot(0), itemID(0) {
}

void SaveGameSnapshot::PagedInventoryItem::write(Common::WriteStream *stream) const {
	stream->writeByte(page);
	stream->writeByte(slot);
	stream->writeByte(itemID);
}

void SaveGameSnapshot::PagedInventoryItem::read(Common::ReadStream *stream, uint saveGameVersion) {
	page = stream->readByte();
	slot = stream->readByte();
	itemID = stream->readByte();
}

SaveGameSnapshot::PlacedInventoryItem::PlacedInventoryItem() : locationID(0), itemID(0) {
}

void SaveGameSnapshot::PlacedInventoryItem::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(locationID);
	stream->writeByte(itemID);
}

void SaveGameSnapshot::PlacedInventoryItem::read(Common::ReadStream *stream, uint saveGameVersion) {
	locationID = stream->readUint32BE();
	itemID = stream->readByte();
}

SaveGameSnapshot::SaveGameSnapshot() : hero(0), swapOutRoom(0), swapOutScreen(0), swapOutDirection(0),
	escOn(false), numStates(1), listenerX(0), listenerY(0), listenerAngle(0), inventoryPage(0), inventoryActiveItem(0) {
}

void SaveGameSnapshot::write(Common::WriteStream *stream) const {
	stream->writeUint32BE(kSaveGameIdentifier);
	stream->writeUint32BE(kSaveGameCurrentVersion);

	stream->writeUint32BE(numStates);

	for (uint sti = 0; sti < numStates; sti++) {
		stream->writeUint32BE(states[sti]->roomNumber);
		stream->writeUint32BE(states[sti]->screenNumber);
		stream->writeUint32BE(states[sti]->direction);
		stream->writeUint32BE(states[sti]->disc);
		stream->writeByte(states[sti]->havePendingPostSwapScreenReset ? 1 : 0);
		stream->writeByte(states[sti]->haveIdleAnimationLoop ? 1 : 0);
		if (states[sti]->haveIdleAnimationLoop) {
			stream->writeUint32BE(states[sti]->idleAnimNum);
			stream->writeUint32BE(states[sti]->idleFirstFrame);
			stream->writeUint32BE(states[sti]->idleLastFrame);
		}
	}

	stream->writeUint32BE(hero);
	stream->writeUint32BE(swapOutRoom);
	stream->writeUint32BE(swapOutScreen);
	stream->writeUint32BE(swapOutDirection);

	stream->writeByte(escOn ? 1 : 0);

	for (uint sti = 0; sti < numStates; sti++) {
		stream->writeSint32BE(states[sti]->musicTrack);
		stream->writeSint32BE(states[sti]->musicVolume);

		writeString(stream, states[sti]->scoreTrack);
		writeString(stream, states[sti]->scoreSection);
		stream->writeByte(states[sti]->musicActive ? 1 : 0);
		stream->writeByte(states[sti]->musicMuteDisabled ? 1 : 0);

		stream->writeUint32BE(states[sti]->loadedAnimation);
		stream->writeUint32BE(states[sti]->animDisplayingFrame);
		stream->writeSint32BE(states[sti]->animVolume);
	}

	pendingStaticAnimParams.write(stream);
	pendingSoundParams3D.write(stream);

	stream->writeSint32BE(listenerX);
	stream->writeSint32BE(listenerY);
	stream->writeSint32BE(listenerAngle);

	for (uint sti = 0; sti < numStates; sti++) {
		stream->writeUint32BE(states[sti]->inventory.size());
		stream->writeUint32BE(states[sti]->sounds.size());
	}

	stream->writeUint32BE(triggeredOneShots.size());
	stream->writeUint32BE(sayCycles.size());

	for (uint sti = 0; sti < numStates; sti++)
		stream->writeUint32BE(states[sti]->randomAmbientSounds.size());

	stream->writeUint32BE(variables.size());
	stream->writeUint32BE(timers.size());
	stream->writeUint32BE(placedItems.size());
	stream->writeUint32BE(pagedItems.size());
	stream->writeByte(inventoryPage);
	stream->writeByte(inventoryActiveItem);

	for (uint sti = 0; sti < numStates; sti++) {
		for (const SaveGameSwappableState::InventoryItem &invItem : states[sti]->inventory)
			invItem.write(stream);

		for (const SaveGameSwappableState::Sound &sound : states[sti]->sounds)
			sound.write(stream);
	}

	for (const TriggeredOneShot &triggeredOneShot : triggeredOneShots)
		triggeredOneShot.write(stream);

	for (const Common::HashMap<uint32, uint>::Node &cycle : sayCycles) {
		stream->writeUint32BE(cycle._key);
		stream->writeUint32BE(cycle._value);
	}

	for (uint sti = 0; sti < numStates; sti++) {
		for (const RandomAmbientSound &randomAmbientSound : states[sti]->randomAmbientSounds)
			randomAmbientSound.write(stream);
	}

	for (const Common::HashMap<uint32, int32>::Node &var : variables) {
		stream->writeUint32BE(var._key);
		stream->writeSint32BE(var._value);
	}

	for (const Common::HashMap<uint, uint32>::Node &timer : timers) {
		stream->writeUint32BE(timer._key);
		stream->writeUint32BE(timer._value);
	}

	for (const PlacedInventoryItem &item : placedItems)
		item.write(stream);

	for (const PagedInventoryItem &item : pagedItems)
		item.write(stream);
}

LoadGameOutcome SaveGameSnapshot::read(Common::ReadStream *stream) {
	uint32 saveIdentifier = stream->readUint32BE();
	uint32 saveVersion = stream->readUint32BE();

	if (stream->eos() || stream->err())
		return kLoadGameOutcomeMissingVersion;

	if (saveIdentifier != kSaveGameIdentifier)
		return kLoadGameOutcomeInvalidVersion;

	if (saveVersion > kSaveGameCurrentVersion)
		return kLoadGameOutcomeSaveIsTooNew;

	if (saveVersion < kSaveGameEarliestSupportedVersion)
		return kLoadGameOutcomeSaveIsTooOld;

	if (saveVersion >= 6)
		numStates = stream->readUint32BE();
	else
		numStates = 1;

	if (numStates < 1 || numStates > kMaxStates)
		return kLoadGameOutcomeSaveDataCorrupted;

	for (uint sti = 0; sti < numStates; sti++) {
		states[sti].reset(new SaveGameSwappableState());

		states[sti]->roomNumber = stream->readUint32BE();
		states[sti]->screenNumber = stream->readUint32BE();
		states[sti]->direction = stream->readUint32BE();

		if (saveVersion >= 10)
			states[sti]->disc = stream->readUint32BE();

		if (saveVersion >= 7)
			states[sti]->havePendingPostSwapScreenReset = (stream->readByte() != 0);

		if (saveVersion >= 10)
			states[sti]->haveIdleAnimationLoop = (stream->readByte() != 0);
		else
			states[sti]->haveIdleAnimationLoop = false;

		if (states[sti]->haveIdleAnimationLoop) {
			states[sti]->idleAnimNum = stream->readUint32BE();
			states[sti]->idleFirstFrame = stream->readUint32BE();
			states[sti]->idleLastFrame = stream->readUint32BE();
		}
	}

	if (saveVersion >= 6) {
		hero = stream->readUint32BE();
		swapOutScreen = stream->readUint32BE();
		swapOutRoom = stream->readUint32BE();
		swapOutDirection = stream->readUint32BE();
	} else {
		hero = 0;
		swapOutScreen = 0;
		swapOutRoom = 0;
		swapOutDirection = 0;
	}

	escOn = (stream->readByte() != 0);

	for (uint sti = 0; sti < numStates; sti++) {
		states[sti]->musicTrack = stream->readSint32BE();

		if (saveVersion >= 5)
			states[sti]->musicVolume = stream->readSint32BE();
		else
			states[sti]->musicVolume = 100;

		if (saveVersion >= 6) {
			states[sti]->scoreTrack = safeReadString(stream);
			states[sti]->scoreSection = safeReadString(stream);
			states[sti]->musicActive = (stream->readByte() != 0);
		} else {
			states[sti]->musicActive = true;
		}

		if (saveVersion >= 9)
			states[sti]->musicMuteDisabled = (stream->readByte() != 0);
		else
			states[sti]->musicMuteDisabled = false;

		states[sti]->loadedAnimation = stream->readUint32BE();
		states[sti]->animDisplayingFrame = stream->readUint32BE();

		if (saveVersion >= 6)
			states[sti]->animVolume = stream->readSint32BE();
		else
			states[sti]->animVolume = 100;
	}

	pendingStaticAnimParams.read(stream);
	pendingSoundParams3D.read(stream);

	listenerX = stream->readSint32BE();
	listenerY = stream->readSint32BE();
	listenerAngle = stream->readSint32BE();

	uint numInventory[kMaxStates] = {};
	uint numSounds[kMaxStates] = {};
	for (uint sti = 0; sti < numStates; sti++) {
		numInventory[sti] = stream->readUint32BE();
		numSounds[sti] = stream->readUint32BE();
	}

	uint numOneShots = stream->readUint32BE();

	uint numSayCycles = 0;
	uint numRandomAmbientSounds[kMaxStates] = {};
	if (saveVersion >= 4) {
		numSayCycles = stream->readUint32BE();
	}

	if (saveVersion >= 3) {
		for (uint sti = 0; sti < numStates; sti++)
			numRandomAmbientSounds[sti] = stream->readUint32BE();
	}

	uint numVars = stream->readUint32BE();
	uint numTimers = stream->readUint32BE();

	uint numPlacedItems = 0;
	uint numPagedItems = 0;

	if (saveVersion >= 10) {
		numPlacedItems = stream->readUint32BE();
		numPagedItems = stream->readUint32BE();
		this->inventoryPage = stream->readByte();
		this->inventoryActiveItem = stream->readByte();
	} else {
		this->inventoryPage = 0;
		this->inventoryActiveItem = 0;
	}

	if (stream->eos() || stream->err())
		return kLoadGameOutcomeSaveDataCorrupted;

	for (uint sti = 0; sti < numStates; sti++) {
		states[sti]->inventory.resize(numInventory[sti]);
		states[sti]->sounds.resize(numSounds[sti]);
		states[sti]->randomAmbientSounds.resize(numRandomAmbientSounds[sti]);
	}

	triggeredOneShots.resize(numOneShots);

	for (uint sti = 0; sti < numStates; sti++) {
		for (uint i = 0; i < numInventory[sti]; i++)
			states[sti]->inventory[i].read(stream);

		for (uint i = 0; i < numSounds[sti]; i++)
			states[sti]->sounds[i].read(stream, saveVersion);
	}

	for (uint i = 0; i < numOneShots; i++)
		triggeredOneShots[i].read(stream);

	for (uint i = 0; i < numSayCycles; i++) {
		uint32 key = stream->readUint32BE();
		uint value = stream->readUint32BE();

		sayCycles[key] = value;
	}

	for (uint sti = 0; sti < numStates; sti++) {
		for (uint i = 0; i < numRandomAmbientSounds[sti]; i++)
			states[sti]->randomAmbientSounds[i].read(stream);
	}

	for (uint i = 0; i < numVars; i++) {
		uint32 key = stream->readUint32BE();
		int32 value = stream->readSint32BE();

		variables[key] = value;
	}

	for (uint i = 0; i < numTimers; i++) {
		uint32 key = stream->readUint32BE();
		uint32 value = stream->readUint32BE();

		timers[key] = value;
	}

	for (uint i = 0; i < numPlacedItems; i++) {
		PlacedInventoryItem item;
		item.read(stream, saveVersion);
		placedItems.push_back(item);
	}

	for (uint i = 0; i < numPagedItems; i++) {
		PagedInventoryItem item;
		item.read(stream, saveVersion);
		pagedItems.push_back(item);
	}

	if (stream->eos() || stream->err())
		return kLoadGameOutcomeSaveDataCorrupted;

	return kLoadGameOutcomeSucceeded;
}

Common::String SaveGameSnapshot::safeReadString(Common::ReadStream *stream) {
	uint len = stream->readUint32BE();
	if (stream->eos() || stream->err())
		len = 0;

	return stream->readString(0, len);
}

void SaveGameSnapshot::writeString(Common::WriteStream *stream, const Common::String &str) {
	stream->writeUint32BE(str.size());
	stream->writeString(str);
}


FontCacheItem::FontCacheItem() : font(nullptr), size(0) {
}

Runtime::Runtime(OSystem *system, Audio::Mixer *mixer, MidiDriver *midiDrv, const Common::FSNode &rootFSNode, VCruiseGameID gameID, Common::Language defaultLanguage)
	: _system(system), _mixer(mixer), _midiDrv(midiDrv), _roomNumber(1), _screenNumber(0), _direction(0), _hero(0), _disc(0), _swapOutRoom(0), _swapOutScreen(0), _swapOutDirection(0),
	  _haveHorizPanAnimations(false), _loadedRoomNumber(0), _activeScreenNumber(0),
	  _gameState(kGameStateBoot), _gameID(gameID), _havePendingScreenChange(false), _forceScreenChange(false), _havePendingPreIdleActions(false), _havePendingReturnToIdleState(false), _havePendingPostSwapScreenReset(false),
	  _havePendingCompletionCheck(false), _havePendingPlayAmbientSounds(false), _ambientSoundFinishTime(0), _escOn(false), _debugMode(false), _fastAnimationMode(false), _preloadSounds(false),
	  _lowQualityGraphicsMode(false), _musicTrack(0), _musicActive(true), _musicMute(false), _musicMuteDisabled(false),
	  _scoreSectionEndTime(0), _musicVolume(getDefaultSoundVolume()), _musicVolumeRampStartTime(0), _musicVolumeRampStartVolume(0), _musicVolumeRampRatePerMSec(0), _musicVolumeRampEnd(0),
	  _panoramaDirectionFlags(0),
	  _loadedAnimation(0), _loadedAnimationHasSound(false),
	  _animTerminateAtStartOfFrame(true), _animPendingDecodeFrame(0), _animDisplayingFrame(0), _animFirstFrame(0), _animLastFrame(0), _animStopFrame(0), _animVolume(getDefaultSoundVolume()),
	  _animStartTime(0), _animFramesDecoded(0), _animDecoderState(kAnimDecoderStateStopped),
	  _animPlayWhileIdle(false), _idleLockInteractions(false), _idleIsOnInteraction(false), _idleIsOnOpenCircuitPuzzleLink(false), _idleIsCircuitPuzzleLinkDown(false),
	  _forceAllowSaves(false),
	  _idleHaveClickInteraction(false), _idleHaveDragInteraction(false), _idleInteractionID(0), _haveIdleStaticAnimation(false),
	  _inGameMenuState(kInGameMenuStateInvisible), _inGameMenuActiveElement(0), _inGameMenuButtonActive {false, false, false, false, false},
	  _lmbDown(false), _lmbDragging(false), _lmbReleaseWasClick(false), _lmbDownTime(0), _lmbDragTolerance(0),
	  _delayCompletionTime(0),
	  _panoramaState(kPanoramaStateInactive),
	  _listenerX(0), _listenerY(0), _listenerAngle(0), _soundCacheIndex(0),
	  _isInGame(false),
	  _subtitleFont(nullptr), _isDisplayingSubtitles(false), _isSubtitleSourceAnimation(false),
	  _languageIndex(0), _defaultLanguageIndex(0), _defaultLanguage(defaultLanguage), _language(defaultLanguage), _charSet(kCharSetLatin),
	  _isCDVariant(false), _currentAnimatedCursor(nullptr), _currentCursor(nullptr), _cursorTimeBase(0), _cursorCycleLength(0),
	  _inventoryActivePage(0), _keepStaticAnimationInIdle(false) {

	for (uint i = 0; i < kNumDirections; i++) {
		_haveIdleAnimations[i] = false;
		_havePanUpFromDirection[i] = false;
		_havePanDownFromDirection[i] = false;
	}

	for (uint i = 0; i < kPanCursorMaxCount; i++)
		_panCursors[i] = 0;

	_rng.reset(new Common::RandomSource("vcruise"));

#ifdef USE_FREETYPE2
	if (_gameID == GID_AD2044) {
		Common::File *f = new Common::File();
		if (f->open("gfx/AD2044.TTF"))
			_subtitleFontKeepalive.reset(Graphics::loadTTFFont(f, DisposeAfterUse::YES, 16, Graphics::kTTFSizeModeCharacter, 108, 72, Graphics::kTTFRenderModeLight));
		else
			delete f;
	} else
		_subtitleFontKeepalive.reset(Graphics::loadTTFFontFromArchive("NotoSans-Regular.ttf", 16, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeLight));

	_subtitleFont = _subtitleFontKeepalive.get();
#endif

	if (!_subtitleFont)
		_subtitleFont = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);

	if (!_subtitleFont)
		warning("Couldn't load subtitle font, subtitles will be disabled");

	_menuInterface.reset(new RuntimeMenuInterface(this));

	for (int32 i = 0; i < 49; i++)
		_dbToVolume[i] = decibelsToLinear(i - 49, Audio::Mixer::kMaxChannelVolume / 2, Audio::Mixer::kMaxChannelVolume / 2);
}

Runtime::~Runtime() {
}

void Runtime::initSections(const Common::Rect &gameRect, const Common::Rect &menuRect, const Common::Rect &trayRect, const Common::Rect &subtitleRect, const Common::Rect &fullscreenMenuRect, const Graphics::PixelFormat &pixFmt) {
	_gameSection.init(gameRect, pixFmt);
	_menuSection.init(menuRect, pixFmt);
	_traySection.init(trayRect, pixFmt);
	_fullscreenMenuSection.init(fullscreenMenuRect, pixFmt);

	if (!subtitleRect.isEmpty())
		_subtitleSection.init(subtitleRect, pixFmt);

	_placedItemBackBufferSection.init(Common::Rect(), pixFmt);
}

void Runtime::loadCursors(const char *exeName) {
	if (_gameID == GID_AD2044) {
		const int staticCursorIDs[] = {0, 29, 30, 31, 32, 33, 34, 35, 36, 39, 40, 41, 50, 96, 97, 99};
		const int animatedCursorIDs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

		_cursors.resize(100);

		for (int cid : staticCursorIDs) {
			Common::String cursorPath = Common::String::format("gfx/CURSOR%02i.CUR", static_cast<int>(cid));

			Common::File f;
			Image::IcoCurDecoder decoder;
			if (!f.open(Common::Path(cursorPath)) || !decoder.open(f))
				error("Couldn't load cursor %s", cursorPath.c_str());

			uint numItems = decoder.numItems();

			if (numItems < 1)
				error("Cursor %s had no items", cursorPath.c_str());

			Graphics::Cursor *cursor = decoder.loadItemAsCursor(0);
			if (!cursor)
				error("Couldn't load cursor %s", cursorPath.c_str());

			_cursors[cid] = staticCursorToAnimatedCursor(Common::SharedPtr<Graphics::Cursor>(cursor));
		}

		for (int cid : animatedCursorIDs) {
			Common::String cursorPath = Common::String::format("gfx/CURSOR%i.ani", static_cast<int>(cid));

			Common::File f;
			Image::AniDecoder decoder;
			if (!f.open(Common::Path(cursorPath)) || !decoder.open(f))
				error("Couldn't load cursor %s", cursorPath.c_str());

			_cursors[cid] = aniFileToAnimatedCursor(decoder);
		}
	} else {
		Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(exeName));
		if (!winRes)
			error("Couldn't open executable file %s", exeName);

		Common::Array<Common::WinResourceID> cursorGroupIDs = winRes->getIDList(Common::kWinGroupCursor);
		for (const Common::WinResourceID &id : cursorGroupIDs) {
			Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), id));
			if (!winRes) {
				warning("Couldn't load cursor group");
				continue;
			}

			Common::String nameStr = id.getString();
			if (nameStr.matchString("CURSOR_#")) {
				char c = nameStr[7];

				uint shortID = c - '0';
				if (shortID >= _cursorsShort.size())
					_cursorsShort.resize(shortID + 1);
				_cursorsShort[shortID] = winCursorGroupToAnimatedCursor(cursorGroup);
			} else if (nameStr.matchString("CURSOR_CUR_##")) {
				char c1 = nameStr[11];
				char c2 = nameStr[12];

				uint longID = (c1 - '0') * 10 + (c2 - '0');
				if (longID >= _cursors.size())
					_cursors.resize(longID + 1);
				_cursors[longID] = winCursorGroupToAnimatedCursor(cursorGroup);
			}
		}
	}

	if (_gameID == GID_REAH) {
		// For some reason most cursors map to their resource IDs, except for these
		_scriptCursorIDToResourceIDOverride[13] = 35;	// Points to finger (instead of back up)
		_scriptCursorIDToResourceIDOverride[22] = 13;	// Points to back up (instead of up arrow)

		_namedCursors["CUR_TYL"] = 22;		// Tyl = back
		//_namedCursors["CUR_NIC"] = ?		// Nic = nothing
		_namedCursors["CUR_WEZ"] = 90;		// Wez = call?  This is the pick-up hand.
		_namedCursors["CUR_LUPA"] = 21;		// Lupa = magnifier, could be 36 too?
		_namedCursors["CUR_NAC"] = 13;		// Nac = top?  Not sure.  But this is the finger pointer.
		_namedCursors["CUR_PRZOD"] = 1;		// Przod = forward

		// CUR_ZOSTAW is in the executable memory but appears to be unused
	}

	if (_gameID == GID_SCHIZM) {
		_namedCursors["curPress"] = 16;
		_namedCursors["curLookFor"] = 21;
		_namedCursors["curForward"] = 1;
		_namedCursors["curBack"] = 13;
		_namedCursors["curNothing"] = 0;
		_namedCursors["curPickUp"] = 90;
		_namedCursors["curDrop"] = 91;
	}

	if (_gameID == GID_AD2044) {
		_namedCursors["CUR_PRZOD"] = 4; // Przod = forward
		_namedCursors["CUR_PRAWO"] = 3;	// Prawo = right
		_namedCursors["CUR_LEWO"] = 1; // Lewo = left
		_namedCursors["CUR_LUPA"] = 6; // Lupa = magnifier
		_namedCursors["CUR_NAC"] = 5; // Nac = top?  Not sure.  But this is the finger pointer.
		_namedCursors["CUR_TYL"] = 2; // Tyl = back
		_namedCursors["CUR_OTWORZ"] = 11; // Otworz = open
		_namedCursors["CUR_WEZ"] = 8; // Wez = Pick up
		_namedCursors["CUR_ZOSTAW"] = 7; // Put down
	}

	_panCursors[kPanCursorDraggableHoriz | kPanCursorDraggableUp] = 2;
	_panCursors[kPanCursorDraggableHoriz | kPanCursorDraggableDown] = 3;
	_panCursors[kPanCursorDraggableHoriz] = 4;
	_panCursors[kPanCursorDraggableHoriz | kPanCursorDirectionRight] = 5;
	_panCursors[kPanCursorDraggableHoriz | kPanCursorDirectionLeft] = 6;
	_panCursors[kPanCursorDraggableUp] = 7;
	_panCursors[kPanCursorDraggableDown] = 8;
	_panCursors[kPanCursorDraggableUp | kPanCursorDirectionUp] = 9;
	_panCursors[kPanCursorDraggableDown | kPanCursorDirectionDown] = 10;
	_panCursors[kPanCursorDraggableUp | kPanCursorDraggableDown] = 11;
	_panCursors[kPanCursorDraggableHoriz | kPanCursorDraggableUp | kPanCursorDraggableDown] = 12;
}

void Runtime::setDebugMode(bool debugMode) {
	if (debugMode) {
		_debugMode = true;
		_gameDebugBackBuffer.init(_gameSection.rect, _gameSection.surf->format);
	}
}

void Runtime::setFastAnimationMode(bool fastAnimationMode) {
	_fastAnimationMode = fastAnimationMode;
}

void Runtime::setPreloadSounds(bool preloadSounds) {
	_preloadSounds = preloadSounds;
}

void Runtime::setLowQualityGraphicsMode(bool lowQualityGraphicsMode) {
	_lowQualityGraphicsMode = lowQualityGraphicsMode;
}

bool Runtime::runFrame() {
	bool moreActions = true;
	while (moreActions) {
		moreActions = false;
		switch (_gameState) {
		case kGameStateBoot:
			moreActions = bootGame(true);
			break;
		case kGameStateQuit:
			return false;
		case kGameStateIdle:
			moreActions = runIdle();
			break;
		case kGameStateDelay:
			moreActions = runDelay();
			break;
		case kGameStatePanLeft:
			moreActions = runHorizontalPan(false);
			break;
		case kGameStatePanRight:
			moreActions = runHorizontalPan(true);
			break;
		case kGameStateScriptReset:
			_gameState = kGameStateScript;
			moreActions = runScript();
			break;
		case kGameStateScript:
			moreActions = runScript();
			break;
		case kGameStateWaitingForAnimation:
			moreActions = runWaitForAnimation();
			break;
		case kGameStateWaitingForAnimationToDelay:
			moreActions = runWaitForAnimationToDelay();
			break;
		case kGameStateWaitingForFacing:
			moreActions = runWaitForFacing();
			break;
		case kGameStateWaitingForFacingToAnim:
			moreActions = runWaitForFacingToAnim();
			break;
		case kGameStateGyroIdle:
			moreActions = runGyroIdle();
			break;
		case kGameStateGyroAnimation:
			moreActions = runGyroAnimation();
			break;
		case kGameStateMenu:
			moreActions = _menuPage->run();

			if (_gameState != kGameStateMenu)
				_menuPage.reset();

			break;
		default:
			error("Unknown game state");
			return false;
		}
	}

	// Discard any unconsumed OS events
	OSEvent evt;
	while (popOSEvent(evt)) {
		// Do nothing
	}

	uint32 timestamp = g_system->getMillis();

	updateSounds(timestamp);
	updateSubtitles();

	refreshCursor(timestamp);

	return true;
}

bool Runtime::bootGame(bool newGame) {
	assert(_gameState == kGameStateBoot);

	if (!ConfMan.hasKey("vcruise_increase_drag_distance") || ConfMan.getBool("vcruise_increase_drag_distance"))
		_lmbDragTolerance = 3;

	if (ConfMan.hasKey("vcruise_mute_music") && ConfMan.getBool("vcruise_mute_music"))
		_musicMute = true;
	else
		_musicMute = false;

	debug(1, "Booting V-Cruise game...");

	if (_gameID == GID_AD2044) {
		loadAD2044ExecutableResources();

		Common::File tabFile;

		if (tabFile.open(Common::Path("anims/ANIM0087.TAB")))
			loadTabData(_examineAnimIDToFrameRange, 87, &tabFile);
		else
			error("Failed to load inspection animations");
	} else
		loadReahSchizmIndex();

	debug(1, "Index loaded OK");
	findWaves();
	debug(1, "Waves indexed OK");

	if (_gameID == GID_SCHIZM) {
		loadConfig("Schizm.ini");
		debug(1, "Config indexed OK");

		loadScore();
		debug(1, "Score loaded OK");

		// Duplicate rooms must be identified in advance because they can take effect before the room logic is loaded.
		// For example, in room 37, when taking the hanging lift across, the room is changed to room 28 and then
		// animation PortD_Zwierz_morph is used, is an animation mapped to room 25, but we can't know that room 28 is
		// a duplicate of room 25 unless we check the logic file for rooms 26-28.  Additionally, we can't just scan
		// downward for missing animations elsewhere because PRZYCUMIE_KRZESELKO is mapped to animations 25 and 26,
		// but the frame range for 27 and 28 is supposed to use room 25 (the root of the duplication), not 26.
		loadDuplicateRooms();
		debug(1, "Duplicated rooms identified OK");

		loadAllSchizmScreenNames();
		debug(1, "Screen names resolved OK");
	} else if (_gameID == GID_REAH) {
		StartConfigDef &startConfig = _startConfigs[kStartConfigInitial];
		startConfig.disc = 1;
		startConfig.room = 1;
		startConfig.screen = 0xb0;
		startConfig.direction = 0;
	} else if (_gameID == GID_AD2044) {
		StartConfigDef &startConfig = _startConfigs[kStartConfigInitial];
		startConfig.disc = 1;
		startConfig.room = 1;
		startConfig.screen = 0xa5;
		startConfig.direction = 0;
	} else
		error("Don't have a start config for this game");

	if (_gameID != GID_AD2044) {
		_trayBackgroundGraphic = loadGraphic("Pocket", "", true);
		_trayHighlightGraphic = loadGraphic("Select", "", true);
		_trayCompassGraphic = loadGraphic("Select_1", "", true);
		_trayCornerGraphic = loadGraphic("Select_2", "", true);
	}

	if (_gameID == GID_AD2044)
		_backgroundGraphic = loadGraphicFromPath("SCR0.BMP", true);

	Common::Language lang = _defaultLanguage;

	if (ConfMan.hasKey("language")) {
		lang = Common::parseLanguage(ConfMan.get("language"));
		debug(2, "Using user-selected language %s", Common::getLanguageDescription(lang));
	} else {
		debug(2, "Defaulted language to %s", Common::getLanguageDescription(lang));
	}

	_languageIndex = 1;
	_defaultLanguageIndex = 1;

	if (_gameID == GID_REAH) {
		_animSpeedRotation = Fraction(21, 1);	// Probably accurate
		_animSpeedStaticAnim = Fraction(21, 1); // Probably accurate
		_animSpeedDefault = Fraction(16, 1);    // Possibly not accurate

		const Common::Language langIndexes[] = {
			Common::PL_POL,
			Common::EN_ANY,
			Common::DE_DEU,
			Common::FR_FRA,
			Common::NL_NLD,
			Common::ES_ESP,
			Common::IT_ITA,
		};

		uint langCount = sizeof(langIndexes) / sizeof(langIndexes[0]);

		for (uint li = 0; li < langCount; li++) {
			if (langIndexes[li] == _defaultLanguage)
				_defaultLanguageIndex = li;
		}

		for (uint li = 0; li < langCount; li++) {
			if (langIndexes[li] == lang) {
				_languageIndex = li;
				break;
			}
			if (langIndexes[li] == _defaultLanguage)
				_languageIndex = li;
		}
	} else if (_gameID == GID_SCHIZM) {
		_animSpeedRotation = Fraction(21, 1); // Probably accurate
		_animSpeedStaticAnim = Fraction(21, 1); // Probably accurate
		_animSpeedDefault = Fraction(21, 1);  // Probably accurate

		const Common::Language langIndexes[] = {
			Common::PL_POL,
			Common::EN_GRB,
			Common::DE_DEU,
			Common::FR_FRA,
			Common::NL_NLD,
			Common::ES_ESP,
			Common::IT_ITA,
			Common::RU_RUS,
			Common::EL_GRC,
			Common::EN_USA,

			// Additional subs present in Steam release
			Common::BG_BUL,
			Common::ZH_TWN,
			Common::JA_JPN,
			Common::HU_HUN,
			Common::ZH_CHN,
			Common::CS_CZE,
		};

		uint langCount = sizeof(langIndexes) / sizeof(langIndexes[0]);

		for (uint li = 0; li < langCount; li++) {
			if (langIndexes[li] == _defaultLanguage)
				_defaultLanguageIndex = li;
		}

		for (uint li = 0; li < langCount; li++) {
			if (langIndexes[li] == lang) {
				_languageIndex = li;
				break;
			}
			if (langIndexes[li] == _defaultLanguage)
				_languageIndex = li;
		}
	}

	_language = lang;

	debug(2, "Language index: %u   Default language index: %u", _languageIndex, _defaultLanguageIndex);

	Common::CodePage codePage = Common::CodePage::kASCII;
	resolveCodePageForLanguage(lang, codePage, _charSet);

	bool subtitlesLoadedOK = false;

	if (_gameID == GID_AD2044) {
		subtitlesLoadedOK = true;
	} else {
		subtitlesLoadedOK = loadSubtitles(codePage, false);

		if (!subtitlesLoadedOK) {
			lang = _defaultLanguage;
			_languageIndex = _defaultLanguageIndex;

			warning("Localization data failed to load, retrying with default language");

			resolveCodePageForLanguage(lang, codePage, _charSet);

			subtitlesLoadedOK = loadSubtitles(codePage, false);

			if (!subtitlesLoadedOK) {
				if (_languageIndex != 0) {
					codePage = Common::CodePage::kWindows1250;
					_languageIndex = 0;
					_defaultLanguageIndex = 0;

					warning("Localization data failed to load again, trying one more time and guessing the encoding");

					subtitlesLoadedOK = loadSubtitles(codePage, true);
				}
			}
		}
	}

	debug(2, "Final language selection: %s   Code page: %i   Language index: %u", Common::getLanguageDescription(lang), static_cast<int>(codePage), _languageIndex);

	if (subtitlesLoadedOK)
		debug(1, "Localization data loaded OK");
	else
		warning("Localization data failed to load!  Text and subtitles will be disabled.");

	if (_gameID != GID_AD2044) {
		_uiGraphics.resize(24);
		for (uint i = 0; i < _uiGraphics.size(); i++) {
			if (_gameID == GID_REAH) {
				_uiGraphics[i] = loadGraphic(Common::String::format("Image%03u", static_cast<uint>(_languageIndex * 100u + i)), "", false);
				if (_languageIndex != 0 && !_uiGraphics[i])
					_uiGraphics[i] = loadGraphic(Common::String::format("Image%03u", static_cast<uint>(i)), "", false);
			} else if (_gameID == GID_SCHIZM) {
				_uiGraphics[i] = loadGraphic(Common::String::format("Data%03u", i), "", false);
			}
		}
	}

	if (_gameID == GID_AD2044)
		_mapLoader.reset(new AD2044MapLoader());
	else
		_mapLoader.reset(new ReahSchizmMapLoader());

	_gameState = kGameStateIdle;

	if (newGame) {
		if (_gameID == GID_AD2044 || (ConfMan.hasKey("vcruise_skip_menu") && ConfMan.getBool("vcruise_skip_menu"))) {
			_mostRecentValidSaveState = generateNewGameSnapshot();
			restoreSaveGameSnapshot();
		} else {
			changeToScreen(1, 0xb1);
		}
	}

	return true;
}

void Runtime::resolveCodePageForLanguage(Common::Language lang, Common::CodePage &outCodePage, CharSet &outCharSet) {
	switch (lang) {
	case Common::PL_POL:
	case Common::CS_CZE:
	case Common::HU_HUN:
		outCodePage = Common::CodePage::kWindows1250;
		outCharSet = kCharSetLatin;
		return;
	case Common::RU_RUS:
	case Common::BG_BUL:
		outCodePage = Common::CodePage::kWindows1251;
		outCharSet = kCharSetCyrillic;
		return;
	case Common::EL_GRC:
		outCodePage = Common::CodePage::kWindows1253;
		outCharSet = kCharSetGreek;
		return;
	case Common::ZH_TWN:
		outCodePage = Common::CodePage::kBig5;
		outCharSet = kCharSetChineseTraditional;
		return;
	case Common::JA_JPN:
		outCodePage = Common::CodePage::kWindows932;	// Shift-JIS compatible
		outCharSet = kCharSetJapanese;
		return;
	case Common::ZH_CHN:
		outCodePage = Common::CodePage::kGBK;
		outCharSet = kCharSetChineseSimplified;
		return;
	default:
		outCodePage = Common::CodePage::kWindows1252;
		outCharSet = kCharSetLatin;
		return;
	}
}

void Runtime::drawLabel(Graphics::ManagedSurface *surface, const Common::String &labelID, const Common::Rect &contentRect) {
	Common::HashMap<Common::String, UILabelDef>::const_iterator labelDefIt = _locUILabels.find(labelID);
	if (labelDefIt == _locUILabels.end())
		return;

	const UILabelDef &labelDef = labelDefIt->_value;

	Common::HashMap<Common::String, Common::String>::const_iterator lineIt = _locStrings.find(labelDef.lineID);

	if (lineIt == _locStrings.end())
		return;

	Common::HashMap<Common::String, TextStyleDef>::const_iterator styleIt = _locTextStyles.find(labelDef.styleDefID);

	if (styleIt == _locTextStyles.end())
		return;

	const Graphics::Font *font = resolveFont(styleIt->_value.fontName, styleIt->_value.size);
	if (!font)
		return;

	const Common::String &textUTF8 = lineIt->_value;
	if (textUTF8.size() == 0)
		return;

	uint32 textColorRGB = styleIt->_value.colorRGB;
	uint32 shadowColorRGB = styleIt->_value.shadowColorRGB;

	uint shadowOffset = styleIt->_value.size / 10u;

	Common::U32String text = textUTF8.decode(Common::kUtf8);

	int strWidth = font->getStringWidth(text);
	int strHeight = font->getFontHeight();

	Common::Point textPos;

	switch (styleIt->_value.alignment % 10u) {
	case 1:
		textPos.x = contentRect.left + (contentRect.width() - strWidth) / 2;
		break;
	case 2:
		textPos.x = contentRect.left - strWidth;
		break;
	default:
		textPos.x = contentRect.left;
		break;
	}

	textPos.y = contentRect.top + (static_cast<int>(labelDef.graphicHeight) - strHeight) / 2;

	if (shadowColorRGB != 0) {
		Common::Point shadowPos = textPos + Common::Point(shadowOffset, shadowOffset);

		uint32 realShadowColor = surface->format.RGBToColor((shadowColorRGB >> 16) & 0xff, (shadowColorRGB >> 8) & 0xff, shadowColorRGB & 0xff);
		font->drawString(surface, text, shadowPos.x, shadowPos.y, strWidth, realShadowColor);
	}

	uint32 realTextColor = surface->format.RGBToColor((textColorRGB >> 16) & 0xff, (textColorRGB >> 8) & 0xff, textColorRGB & 0xff);
	font->drawString(surface, text, textPos.x, textPos.y, strWidth, realTextColor);
}

void Runtime::onMidiTimer() {
	Common::StackLock lock(_midiPlayerMutex);
	if (_musicMidiPlayer)
		_musicMidiPlayer->onMidiTimer();
}


bool Runtime::runIdle() {
	if (_havePendingScreenChange) {
		_havePendingScreenChange = false;

		_havePendingPreIdleActions = true;

		changeToScreen(_roomNumber, _screenNumber);
		return true;
	}

	if (_havePendingPlayAmbientSounds) {
		_havePendingPlayAmbientSounds = false;
		triggerAmbientSounds();
	}

	if (_havePendingPreIdleActions) {
		_havePendingPreIdleActions = false;

		if (triggerPreIdleActions())
			return true;
	}

	if (_havePendingReturnToIdleState) {
		_havePendingReturnToIdleState = false;

		returnToIdleState();
		drawCompass();
		return true;
	}

	uint32 timestamp = g_system->getMillis();

	// Try to keep this in sync with runDelay
	if (_animPlayWhileIdle) {
		assert(_haveIdleAnimations[_direction]);

		StaticAnimation &sanim = _idleAnimations[_direction];
		bool looping = (sanim.params.repeatDelay == 0);

		bool animEnded = false;
		continuePlayingAnimation(looping, false, animEnded);

		if (!looping && animEnded) {
			_animPlayWhileIdle = false;
			sanim.nextStartTime = timestamp + sanim.params.repeatDelay * 1000u;
			sanim.currentAlternation = 1 - sanim.currentAlternation;

			if (_idleLockInteractions) {
				_idleLockInteractions = false;
				bool changedState = dischargeIdleMouseMove();
				if (changedState) {
					drawCompass();
					return true;
				}
			}
		}
	} else if (_haveIdleAnimations[_direction]) {
		// Try to re-trigger
		StaticAnimation &sanim = _idleAnimations[_direction];
		if (sanim.nextStartTime <= timestamp) {
			const AnimationDef &animDef = sanim.animDefs[sanim.currentAlternation];
			changeAnimation(animDef, animDef.firstFrame, false, _animSpeedStaticAnim);
			_animPlayWhileIdle = true;

			_idleLockInteractions = sanim.params.lockInteractions;
			if (_idleLockInteractions) {
				_panoramaState = kPanoramaStateInactive;
				bool changedState = dischargeIdleMouseMove();
				assert(!changedState);	// Shouldn't be changing state from this!
				(void)changedState;
			}
		}
	}

	if (_debugMode)
		drawDebugOverlay();

	detectPanoramaMouseMovement(timestamp);

	OSEvent osEvent;
	while (popOSEvent(osEvent)) {
		if (osEvent.type == kOSEventTypeMouseMove) {
			detectPanoramaMouseMovement(osEvent.timestamp);

			bool changedState = dischargeIdleMouseMove();
			if (changedState) {
				drawCompass();
				return true;
			}
		} else if (osEvent.type == kOSEventTypeLButtonUp) {
			if (_inGameMenuState != kInGameMenuStateInvisible) {
				dischargeInGameMenuMouseUp();
			} else {
				PanoramaState oldPanoramaState = _panoramaState;
				_panoramaState = kPanoramaStateInactive;

				// This is the correct place for matching the original game's behavior, not switching to panorama
				resetInventoryHighlights();

				if (_lmbReleaseWasClick) {
					bool changedState = dischargeIdleClick();
					if (changedState) {
						drawCompass();
						return true;
					}
				}

				// If the released from panorama mode, pick up any interactions at the new mouse location, and change the mouse back
				if (oldPanoramaState != kPanoramaStateInactive) {
					changeToCursor(_cursors[kCursorArrow]);

					// Clear idle interaction so that if a drag occurs but doesn't trigger a panorama or other state change,
					// interactions are re-detected here.
					_idleIsOnInteraction = false;

					bool changedState = dischargeIdleMouseMove();
					if (changedState) {
						drawCompass();
						return true;
					}
				}
			}
		} else if (osEvent.type == kOSEventTypeLButtonDown) {
			bool changedState = dischargeIdleMouseDown();
			if (changedState) {
				drawCompass();
				return true;
			}
		} else if (osEvent.type == kOSEventTypeKeymappedEvent) {
			if (!_lmbDown) {
				switch (osEvent.keymappedEvent) {
				case kKeymappedEventHelp:
					changeToMenuPage(createMenuHelp(_gameID == GID_SCHIZM));
					return true;
				case kKeymappedEventLoadGame:
					if (g_engine->loadGameDialog())
						return true;
					break;
				case kKeymappedEventSaveGame:
					if (g_engine->saveGameDialog())
						return true;
					break;
				case kKeymappedEventPause:
					changeToMenuPage(createMenuPause(_gameID == GID_SCHIZM));
					return true;
				case kKeymappedEventQuit:
					changeToMenuPage(createMenuQuit(_gameID == GID_SCHIZM));
					return true;
				case kKeymappedEventPutItem:
					cheatPutItem();
					return true;
				default:
					break;
				}
			}
		}
	}

	// Yield
	return false;
}

bool Runtime::runDelay() {
	uint32 timestamp = g_system->getMillis();

	if (g_system->getMillis() >= _delayCompletionTime) {
		_gameState = kGameStateScript;
		return true;
	}

	if (_havePendingPreIdleActions) {
		_havePendingPreIdleActions = false;

		if (triggerPreIdleActions())
			return true;
	}

	// Play static animations.  Try to keep this in sync with runIdle
	if (_animPlayWhileIdle) {
		assert(_haveIdleAnimations[_direction]);

		StaticAnimation &sanim = _idleAnimations[_direction];
		bool looping = (sanim.params.repeatDelay == 0);

		bool animEnded = false;
		continuePlayingAnimation(looping, false, animEnded);

		if (!looping && animEnded) {
			_animPlayWhileIdle = false;
			sanim.nextStartTime = timestamp + sanim.params.repeatDelay * 1000u;
			sanim.currentAlternation = 1 - sanim.currentAlternation;

			if (_idleLockInteractions)
				_idleLockInteractions = false;
		}
	} else if (_haveIdleAnimations[_direction]) {
		StaticAnimation &sanim = _idleAnimations[_direction];
		if (sanim.nextStartTime <= timestamp) {
			const AnimationDef &animDef = sanim.animDefs[sanim.currentAlternation];
			changeAnimation(animDef, animDef.firstFrame, false, _animSpeedStaticAnim);
			_animPlayWhileIdle = true;

			_idleLockInteractions = sanim.params.lockInteractions;
		}
	}

	return false;
}

bool Runtime::runHorizontalPan(bool isRight) {
	bool animEnded = false;
	continuePlayingAnimation(true, false, animEnded);

	Common::Point panRelMouse = _mousePos - _panoramaAnchor;

	if (!_lmbDown) {
		debug(1, "Terminating pan: LMB is not down");
		startTerminatingHorizontalPan(isRight);
		return true;
	}

	if (!isRight && panRelMouse.x > -kPanoramaPanningMarginX) {
		debug(1, "Terminating pan: Over threshold for left movement");
		startTerminatingHorizontalPan(isRight);
		return true;
	} else if (isRight && panRelMouse.x < kPanoramaPanningMarginX) {
		debug(1, "Terminating pan: Over threshold for right movement");
		startTerminatingHorizontalPan(isRight);
		return true;
	}

	OSEvent osEvent;
	while (popOSEvent(osEvent)) {
		if (osEvent.type == kOSEventTypeLButtonUp) {
			debug(1, "Terminating pan: LMB up");
			startTerminatingHorizontalPan(isRight);
			return true;
		} else if (osEvent.type == kOSEventTypeMouseMove) {
			panRelMouse = osEvent.pos - _panoramaAnchor;

			if (!isRight && panRelMouse.x > -kPanoramaPanningMarginX) {
				debug(1, "Terminating pan: Over threshold for left movement (from queue)");
				startTerminatingHorizontalPan(isRight);
				return true;
			} else if (isRight && panRelMouse.x < kPanoramaPanningMarginX) {
				debug(1, "Terminating pan: Over threshold for right movement (from queue)");
				startTerminatingHorizontalPan(isRight);
				return true;
			}
		}
	}

	// Didn't terminate, yield
	return false;
}

bool Runtime::runWaitForAnimation() {
	bool animEnded = false;
	continuePlayingAnimation(false, false, animEnded);

	if (animEnded) {
		_gameState = kGameStateScript;
		return true;
	}

	// Still waiting, check events
	OSEvent evt;
	while (popOSEvent(evt)) {
		if (evt.type == kOSEventTypeKeymappedEvent && evt.keymappedEvent == kKeymappedEventEscape) {
			if (_escOn) {
				// Terminate the animation
				if (_animDecoderState == kAnimDecoderStatePlaying) {
					_animDecoder->pauseVideo(true);
					_animDecoderState = kAnimDecoderStatePaused;
				}
				_scriptEnv.esc = true;
				_gameState = kGameStateScript;
				return true;
			}
		} else if (evt.type == kOSEventTypeKeymappedEvent && evt.keymappedEvent == kKeymappedEventSkipAnimation) {
			_animFrameRateLock = Fraction(600, 1);
			_animFramesDecoded = 0;	// Reset decoder count so the start time resyncs
		}
	}

	// Yield
	return false;
}

bool Runtime::runWaitForAnimationToDelay() {
	bool animEnded = false;
	continuePlayingAnimation(false, false, animEnded);

	if (animEnded) {
		_gameState = kGameStateDelay;
		return true;
	}

	// Yield
	return false;
}

bool Runtime::runWaitForFacingToAnim() {
	bool animEnded = false;
	continuePlayingAnimation(true, true, animEnded);

	if (animEnded) {
		changeAnimation(_postFacingAnimDef, _postFacingAnimDef.firstFrame, true, _animSpeedDefault);
		_gameState = kGameStateWaitingForAnimation;
		return true;
	}

	// Yield
	return false;
}

bool Runtime::runWaitForFacing() {
	bool animEnded = false;
	continuePlayingAnimation(true, true, animEnded);

	if (animEnded) {
		_gameState = kGameStateScript;
		return true;
	}

	// Yield
	return false;
}

bool Runtime::runGyroIdle() {
	if (!_lmbDown) {
		exitGyroIdle();
		return true;
	}

	int32 deltaCoordinate = 0;

	if (_gyros.isVertical)
		deltaCoordinate = _gyros.dragBasePoint.y - _mousePos.y;
	else
		deltaCoordinate = _gyros.dragBasePoint.x - _mousePos.x;


	// Start the first step at half margin
	int32 halfDragMargin = _gyros.dragMargin / 2;
	if (deltaCoordinate < 0)
		deltaCoordinate -= halfDragMargin;
	else
		deltaCoordinate += halfDragMargin;

	int32 deltaState = deltaCoordinate / static_cast<int32>(_gyros.dragMargin);
	int32 targetStateInitial = static_cast<int32>(_gyros.dragBaseState) + deltaState;

	Gyro &gyro = _gyros.gyros[_gyros.activeGyro];

	int32 targetState = 0;
	if (gyro.wrapAround) {
		targetState = targetStateInitial;
	} else {
		if (targetStateInitial > 0) {
			targetState = targetStateInitial;
			if (static_cast<uint>(targetState) > _gyros.maxValue)
				targetState = _gyros.maxValue;
		}
	}

	if (targetState < _gyros.dragCurrentState) {
		AnimationDef animDef = _gyros.negAnim;

		uint initialFrame = 0;

		if (gyro.wrapAround) {
			uint maxValuePlusOne = _gyros.maxValue + 1;
			initialFrame = animDef.firstFrame + ((maxValuePlusOne - gyro.currentState) % maxValuePlusOne * _gyros.frameSeparation);
		} else
			initialFrame = animDef.firstFrame + ((_gyros.maxValue - gyro.currentState) * _gyros.frameSeparation);

		// This is intentional instead of setting the stop frame, V-Cruise can overrun the end of the animation.
		// firstFrame is left alone so playlists are based correctly.
		animDef.lastFrame = initialFrame + _gyros.frameSeparation;

		changeAnimation(animDef, initialFrame, false, _animSpeedDefault);

		gyro.logState();
		gyro.currentState--;
		_gyros.dragCurrentState--;

		if (gyro.currentState < 0)
			gyro.currentState = _gyros.maxValue;

		_gameState = kGameStateGyroAnimation;
		_havePendingCompletionCheck = true;
		return true;
	} else if (targetState > _gyros.dragCurrentState) {
		AnimationDef animDef = _gyros.posAnim;

		uint initialFrame = animDef.firstFrame + gyro.currentState * _gyros.frameSeparation;

		// This is intentional instead of setting the stop frame, V-Cruise can overrun the end of the animation.
		// firstFrame is left alone so playlists are based correctly.
		animDef.lastFrame = initialFrame + _gyros.frameSeparation;

		changeAnimation(animDef, initialFrame, false, _animSpeedDefault);

		gyro.logState();
		gyro.currentState++;
		_gyros.dragCurrentState++;

		if (static_cast<uint>(gyro.currentState) > _gyros.maxValue)
			gyro.currentState = 0;

		_gameState = kGameStateGyroAnimation;
		_havePendingCompletionCheck = true;
		return true;
	}

	OSEvent evt;
	while (popOSEvent(evt)) {
		if (evt.type == kOSEventTypeLButtonUp) {
			exitGyroIdle();
			return true;
		}
	}

	// Yield
	return false;
}

bool Runtime::runGyroAnimation() {
	bool animEnded = false;
	continuePlayingAnimation(false, false, animEnded);

	if (animEnded) {
		_gameState = kGameStateGyroIdle;
		return true;
	}

	// Yield
	return false;
}

void Runtime::exitGyroIdle() {
	_gameState = kGameStateScript;
	_havePendingPreIdleActions = true;

	// In Reah, gyro interactions stop the script.
	if (_gameID == GID_REAH)
		terminateScript();
}

void Runtime::continuePlayingAnimation(bool loop, bool useStopFrame, bool &outAnimationEnded) {
	bool terminateAtStartOfFrame = _animTerminateAtStartOfFrame;

	outAnimationEnded = false;

	if (!_animDecoder) {
		outAnimationEnded = true;
		return;
	}

	bool needsFirstFrame = false;
	if (_animDecoderState == kAnimDecoderStatePaused) {
		_animDecoder->pauseVideo(false);
		_animDecoderState = kAnimDecoderStatePlaying;
		needsFirstFrame = true;
	} else if (_animDecoderState == kAnimDecoderStateStopped) {
		_animDecoder->start();
		_animDecoderState = kAnimDecoderStatePlaying;
		needsFirstFrame = true;
	}

	uint32 millis = 0;

	// Avoid spamming event recorder as much if we don't actually need to fetch millis, but also only fetch it once here.
	if (_animFrameRateLock.numerator)
		millis = g_system->getMillis();

	for (;;) {
		bool needNewFrame = false;
		bool needInitialTimestamp = false;

		if (needsFirstFrame) {
			needNewFrame = true;
			needsFirstFrame = false;
			needInitialTimestamp = true;
		} else {
			if (_animFrameRateLock.numerator) {
				if (_animFramesDecoded == 0) {
					needNewFrame = true;
					needInitialTimestamp = true;
				} else {
					// if ((millis - startTime) / 1000 * frameRate / frameRateDenominator) >= framesDecoded
					if ((millis - _animStartTime) * static_cast<uint64>(_animFrameRateLock.numerator) >= (static_cast<uint64>(_animFramesDecoded) * static_cast<uint64>(_animFrameRateLock.denominator) * 1000u))
						needNewFrame = true;
				}
			} else {
				if (_animDecoder->getTimeToNextFrame() == 0)
					needNewFrame = true;
			}
		}

		if (!needNewFrame)
			break;

		if (!terminateAtStartOfFrame && !loop && _animPendingDecodeFrame > _animLastFrame) {
			outAnimationEnded = true;
			return;
		}

		// We check this here for timing reasons: The no-loop case after the draw terminates the animation as soon as the last frame
		// starts delaying without waiting for the time until the next frame to expire.
		// The loop check here DOES wait for the time until next frame to expire.
		if (loop && _animPendingDecodeFrame > _animLastFrame) {
			debug(4, "Looped animation at frame %u", _animLastFrame);
			if (!_animDecoder->seekToFrame(_animFirstFrame)) {
				outAnimationEnded = true;
				return;
			}

			_animPendingDecodeFrame = _animFirstFrame;
		}

		const Graphics::Surface *surface = _animDecoder->decodeNextFrame();

		// Get the start timestamp when the first frame finishes decoding so disk seeks don't cause frame skips
		if (needInitialTimestamp) {
			millis = g_system->getMillis();
			_animStartTime = millis;
		}

		if (!surface) {
			outAnimationEnded = true;
			return;
		}

		_animDisplayingFrame = _animPendingDecodeFrame;
		_animPendingDecodeFrame++;
		_animFramesDecoded++;

		if (_animDisplayingFrame < _frameData2.size()) {
			const FrameData2 &fd2 = _frameData2[_animDisplayingFrame];

			_listenerX = fd2.x;
			_listenerY = fd2.y;
			_listenerAngle = fd2.angle;
		}

		update3DSounds();

		AnimSubtitleMap_t::const_iterator animSubtitlesIt = _animSubtitles.find(_loadedAnimation);
		if (animSubtitlesIt != _animSubtitles.end()) {
			const FrameToSubtitleMap_t &frameMap = animSubtitlesIt->_value;

			FrameToSubtitleMap_t::const_iterator frameIt = frameMap.find(_animDisplayingFrame);
			if (frameIt != frameMap.end() && ConfMan.getBool("subtitles")) {
				if (!millis)
					millis = g_system->getMillis();

				const SubtitleDef &subDef = frameIt->_value;

				_subtitleQueue.clear();
				_isDisplayingSubtitles = false;
				_isSubtitleSourceAnimation = true;

				SubtitleQueueItem queueItem;
				queueItem.startTime = millis;
				queueItem.endTime = millis + 1000u;

				for (int ch = 0; ch < 3; ch++)
					queueItem.color[ch] = subDef.color[ch];

				if (subDef.durationInDeciseconds != 1)
					queueItem.endTime = queueItem.startTime + subDef.durationInDeciseconds * 100u;

				queueItem.str = subDef.str.decode(Common::kUtf8);

				_subtitleQueue.push_back(queueItem);
			}
		}

		if (_animPlaylist) {
			uint decodeFrameInPlaylist = _animDisplayingFrame - _animFirstFrame;
			for (const SfxPlaylistEntry &playlistEntry : _animPlaylist->entries) {
				if (playlistEntry.frame == decodeFrameInPlaylist) {
					VCruise::AudioPlayer &audioPlayer = *playlistEntry.sample->audioPlayer;

					if (playlistEntry.isUpdate) {
						audioPlayer.setVolumeAndBalance(applyVolumeScale(playlistEntry.volume), applyBalanceScale(playlistEntry.balance));
					} else {
						audioPlayer.stop();
						playlistEntry.sample->audioStream->seek(0);
						audioPlayer.play(applyVolumeScale(playlistEntry.volume), applyBalanceScale(playlistEntry.balance));
					}

					// No break, it's possible for there to be multiple sounds in the same frame
				}
			}
		}

		Common::Rect copyRect = Common::Rect(0, 0, surface->w, surface->h);

		if (!_animConstraintRect.isEmpty())
			copyRect = copyRect.findIntersectingRect(_animConstraintRect);

		Common::Rect constraintRect = Common::Rect(0, 0, _gameSection.rect.width(), _gameSection.rect.height());

		copyRect = copyRect.findIntersectingRect(constraintRect);

		if (copyRect.isValidRect() || !copyRect.isEmpty()) {
			Graphics::Palette p(_animDecoder->getPalette(), 256);
			_gameSection.surf->blitFrom(*surface, copyRect, copyRect, &p);
			drawSectionToScreen(_gameSection, copyRect);
		}

		if (!loop) {
			if (terminateAtStartOfFrame && _animDisplayingFrame >= _animLastFrame) {
				_animDecoder->pauseVideo(true);
				_animDecoderState = kAnimDecoderStatePaused;

				outAnimationEnded = true;
				return;
			}
		}

		if (useStopFrame && _animDisplayingFrame == _animStopFrame) {
			outAnimationEnded = true;
			return;
		}
	}
}

void Runtime::drawSectionToScreen(const RenderSection &section, const Common::Rect &rect) {
	const RenderSection *sourceSection = &section;

	if (_debugMode && (&_gameSection == &section)) {
		_gameDebugBackBuffer.surf->blitFrom(*sourceSection->surf, rect, rect);
		commitSectionToScreen(_gameDebugBackBuffer, rect);
	} else
		commitSectionToScreen(*sourceSection, rect);
}

void Runtime::commitSectionToScreen(const RenderSection &section, const Common::Rect &rect) {
	_system->copyRectToScreen(section.surf->getBasePtr(rect.left, rect.top), section.surf->pitch, rect.left + section.rect.left, rect.top + section.rect.top, rect.width(), rect.height());
}

bool Runtime::requireAvailableStack(uint n) {
	if (_scriptStack.size() < n) {
		error("Script stack underflow");
		return false;
	}

	return true;
}

void Runtime::terminateScript() {
	_scriptCallStack.clear();

	// Collect any script env vars that affect script termination, then clear so this doesn't leak into
	// future executions.
	bool puzzleWasSet = _scriptEnv.puzzleWasSet;
	bool exitToMenu = _scriptEnv.exitToMenu;

	_scriptEnv = ScriptEnvironmentVars();

	if (_gameState == kGameStateScript)
		_gameState = kGameStateIdle;

	if (_havePendingCompletionCheck) {
		_havePendingCompletionCheck = false;

		if (checkCompletionConditions())
			return;
	}

	redrawTray();

	if (exitToMenu && _gameState == kGameStateIdle) {
		quitToMenu();
		return;
	}

	if (_havePendingScreenChange) {
		// TODO: Check Reah to see if this condition is okay there too.
		// This is needed to avoid resetting static animations twice, which causes problems with,
		// for example, the second screen on Hannah's path resetting the idle animations after
		// the VO stops.
		if (_gameID == GID_SCHIZM) {
			_havePendingScreenChange = false;

			// The circuit puzzle doesn't call puzzleDone unless you zoom back into the puzzle,
			// which can cause the puzzle to leak.  Clean it up here instead.
			if (!puzzleWasSet)
				clearCircuitPuzzle();
		}

		changeToScreen(_roomNumber, _screenNumber);

		// Run any return-to-idle actions so idle mouse moves are discharged again, even if the screen didn't change.
		// This is necessary to ensure that dischargeIdleMouseMove is called after animS even if it goes back to the same
		// screen, which is necessary to make sure that clicking the pegs on top of the mechanical computer in Schizm
		// resets the mouse cursor to interactive again.
		if (_gameID == GID_SCHIZM)
			_havePendingReturnToIdleState = true;
	}
}

void Runtime::quitToMenu() {
	changeToCursor(_cursors[kCursorArrow]);

	if (_gameID == GID_SCHIZM && _musicActive) {
		_scoreTrack = "music99";
		_scoreSection = "start";
		startScoreSection();
	}

	for (Common::SharedPtr<SoundInstance> &snd : _activeSounds)
		stopSound(*snd);

	_activeSounds.clear();
	_isInGame = false;

	if (_gameID == GID_REAH || _gameID == GID_SCHIZM)
		changeToMenuPage(createMenuMain(_gameID == GID_SCHIZM));
	else
		error("Missing main menu behavior for this game");
}

RoomScriptSet *Runtime::getRoomScriptSetForCurrentRoom() const {
	if (!_scriptSet)
		return nullptr;

	uint roomNumber = _roomNumber;
	if (roomNumber < _roomDuplicationOffsets.size())
		roomNumber -= _roomDuplicationOffsets[roomNumber];

	RoomScriptSetMap_t::const_iterator it = _scriptSet->roomScripts.find(roomNumber);
	if (it == _scriptSet->roomScripts.end())
		return nullptr;

	return it->_value.get();
}

bool Runtime::checkCompletionConditions() {
	bool succeeded = true;
	for (uint i = 0; i < GyroState::kNumGyros; i++) {
		const Gyro &gyro = _gyros.gyros[i];
		if (gyro.requireState && gyro.currentState != gyro.requiredState) {
			succeeded = false;
			break;
		}

		if (gyro.numPreviousStates != gyro.numPreviousStatesRequired) {
			succeeded = false;
			break;
		}

		bool prevStatesMatch = true;
		for (uint j = 0; j < gyro.numPreviousStates; j++) {
			if (gyro.previousStates[j] != gyro.requiredPreviousStates[j]) {
				prevStatesMatch = false;
				break;
			}
		}

		if (!prevStatesMatch) {
			succeeded = false;
			break;
		}
	}

	// Activate the corresponding failure or success interaction if present
	if (_scriptSet) {
		RoomScriptSet *roomScriptSet = getRoomScriptSetForCurrentRoom();

		if (roomScriptSet) {
			const ScreenScriptSetMap_t &screenScriptsMap = roomScriptSet->screenScripts;
			ScreenScriptSetMap_t::const_iterator screenScriptIt = screenScriptsMap.find(_screenNumber);
			if (screenScriptIt != screenScriptsMap.end()) {
				const ScreenScriptSet &screenScriptSet = *screenScriptIt->_value;

				ScriptMap_t::const_iterator interactionScriptIt = screenScriptSet.interactionScripts.find(succeeded ? _gyros.completeInteraction : _gyros.failureInteraction);
				if (interactionScriptIt != screenScriptSet.interactionScripts.end()) {
					const Common::SharedPtr<Script> &script = interactionScriptIt->_value;
					if (script) {
						activateScript(script, false, ScriptEnvironmentVars());
						return true;
					}
				}
			}
		}
	}

	return false;
}

void Runtime::startTerminatingHorizontalPan(bool isRight) {
	// Figure out what slice this is.  The last frame is 1 less than usual.
	uint slice = (_animDisplayingFrame - _animFirstFrame) * kNumDirections / (_animLastFrame - _animFirstFrame + 1);

	// Compute an end frame at the end of the slice.
	_animLastFrame = (slice + 1) * (_animLastFrame - _animFirstFrame + 1) / kNumDirections + _animFirstFrame;

	debug(1, "Terminating pan at frame slice %u -> frame %u", slice, _animLastFrame);

	if (isRight)
		_direction = (slice + 1) % kNumDirections;
	else
		_direction = kNumDirections - 1 - slice;

	_gameState = kGameStateWaitingForAnimation;
	_panoramaState = kPanoramaStateInactive;

	// Need to return to idle after direction change
	_havePendingPreIdleActions = true;
}

bool Runtime::popOSEvent(OSEvent &evt) {
	OSEvent tempEvent;

	while (_pendingEvents.size() > 0) {
		tempEvent = _pendingEvents[0];
		_pendingEvents.remove_at(0);

		// Button events automatically inject a mouse move position
		if (tempEvent.type == kOSEventTypeMouseMove) {
			// If this didn't actually change the mouse position, which is common with synthetic mouse events,
			// discard the event.
			if (_mousePos == tempEvent.pos)
				continue;

			if (_lmbDown && tempEvent.pos != _lmbDownPos) {
				bool isDrag = true;

				if (_lmbDragTolerance > 0) {
					int xDelta = tempEvent.pos.x - _lmbDownPos.x;
					int yDelta = tempEvent.pos.y - _lmbDownPos.y;

					if (xDelta >= -_lmbDragTolerance && xDelta <= _lmbDragTolerance && yDelta >= -_lmbDragTolerance && yDelta <= _lmbDragTolerance)
						isDrag = false;
				}

				_lmbDragging = isDrag;
			}

			_mousePos = tempEvent.pos;
		} else if (tempEvent.type == kOSEventTypeLButtonDown) {
			// Discard LButtonDown events missing a matching release (can happen if e.g. user holds button down
			// and then alt-tabs out of the process on Windows)
			if (_lmbDown)
				continue;

			_lmbDown = true;
			_lmbDragging = false;
			_lmbDownTime = tempEvent.timestamp;
			_lmbDownPos = tempEvent.pos;
		} else if (tempEvent.type == kOSEventTypeLButtonUp) {
			// Discard LButtonUp events missing a matching down
			if (!_lmbDown)
				continue;

			_lmbReleaseWasClick = !_lmbDragging;
			_lmbDown = false;
			_lmbDragging = false;
		} else if (tempEvent.type == kOSEventTypeKeymappedEvent) {
			processUniversalKeymappedEvents(tempEvent.keymappedEvent);
		}

		evt = tempEvent;
		return true;
	}

	return false;
}

void Runtime::processUniversalKeymappedEvents(KeymappedEvent evt) {
	const int soundSettingGranularity = 25;

	switch (evt) {
	case kKeymappedEventMusicToggle: {
			if (ConfMan.hasKey("vcruise_mute_music") && !ConfMan.getBool("vcruise_mute_music"))
				setMusicMute(false);
			else
				setMusicMute(true);

			ConfMan.setBool("vcruise_mute_music", _musicMute, ConfMan.getActiveDomainName());

			if (_menuPage)
				_menuPage->onSettingsChanged();
		} break;
	case kKeymappedEventMusicVolumeUp: {
			int newVol = ConfMan.getInt("music_volume") + soundSettingGranularity;
			if (newVol > Audio::Mixer::kMaxMixerVolume)
				newVol = Audio::Mixer::kMaxMixerVolume;
			ConfMan.setInt("music_volume", newVol, ConfMan.getActiveDomainName());
			g_engine->syncSoundSettings();
			if (_menuPage)
				_menuPage->onSettingsChanged();
		} break;
	case kKeymappedEventMusicVolumeDown: {
			int newVol = ConfMan.getInt("music_volume") - soundSettingGranularity;
			if (newVol < 0)
				newVol = 0;
			ConfMan.setInt("music_volume", newVol, ConfMan.getActiveDomainName());
			g_engine->syncSoundSettings();
			if (_menuPage)
				_menuPage->onSettingsChanged();
		} break;
	case kKeymappedEventSoundToggle:
		ConfMan.setBool("vcruise_mute_sound", !(ConfMan.hasKey("vcruise_mute_sound")) || !(ConfMan.getBool("vcruise_mute_sound")), ConfMan.getActiveDomainName());
		g_engine->syncSoundSettings();
		if (_menuPage)
			_menuPage->onSettingsChanged();
		break;
	case kKeymappedEventSoundVolumeUp: {
			int newVol = ConfMan.getInt("sfx_volume") + soundSettingGranularity;
			if (newVol > Audio::Mixer::kMaxMixerVolume)
				newVol = Audio::Mixer::kMaxMixerVolume;
			ConfMan.setInt("sfx_volume", newVol, ConfMan.getActiveDomainName());
			g_engine->syncSoundSettings();
			if (_menuPage)
				_menuPage->onSettingsChanged();
		} break;
	case kKeymappedEventSoundVolumeDown: {
			int newVol = ConfMan.getInt("sfx_volume") - soundSettingGranularity;
			if (newVol < 0)
				newVol = 0;
			ConfMan.setInt("sfx_volume", newVol, ConfMan.getActiveDomainName());
			g_engine->syncSoundSettings();
			if (_menuPage)
				_menuPage->onSettingsChanged();
		} break;
	default:
		break;
	}
}

void Runtime::queueOSEvent(const OSEvent &evt) {
	OSEvent timedEvt = evt;
	timedEvt.timestamp = g_system->getMillis();

	_pendingEvents.push_back(timedEvt);
}

void Runtime::loadReahSchizmIndex() {
	const char *indexPath = "Log/Index.txt";

	Common::INIFile iniFile;
	iniFile.allowNonEnglishCharacters();
	if (!iniFile.loadFromFile(indexPath))
		error("Failed to load main logic index");

	IndexParseType indexParseType = kIndexParseTypeNone;

	static const IndexPrefixTypePair parsePrefixes[] = {
		{"Room", kIndexParseTypeRoom},
		{"RRoom", kIndexParseTypeRRoom},
		{"YRoom", kIndexParseTypeYRoom},
		{"VRoom", kIndexParseTypeVRoom},
		{"TRoom", kIndexParseTypeTRoom},
		{"CRoom", kIndexParseTypeCRoom},
		{"SRoom", kIndexParseTypeSRoom},
	};

	for (const Common::INIFile::Section &section : iniFile.getSections()) {
		uint roomNumber = 0;

		if (section.name == "NameRoom") {
			indexParseType = kIndexParseTypeNameRoom;
		} else {
			bool foundType = false;
			uint prefixLen = 0;
			for (const IndexPrefixTypePair &prefixTypePair : parsePrefixes) {
				if (section.name.hasPrefix(prefixTypePair.prefix)) {
					indexParseType = prefixTypePair.parseType;
					foundType = true;
					prefixLen = strlen(prefixTypePair.prefix);
					break;
				}
			}

			if (!foundType)
				error("Unknown index heading type %s", section.name.c_str());


			if (!sscanf(section.name.c_str() + prefixLen, "%u", &roomNumber))
				error("Malformed room def '%s'", section.name.c_str());

			allocateRoomsUpTo(roomNumber);
		}

		for (const Common::INIFile::KeyValue &keyValue : section.getKeys()) {
			if (!parseIndexDef(indexParseType, roomNumber, keyValue.key, keyValue.value))
				break;
		}
	}
}

void Runtime::loadAD2044ExecutableResources() {
	const byte searchPattern[] = {0x01, 0x01, 0xa1, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0x00, 0xc7, 0x00, 0x00, 0x00};

	Common::File f;
	if (!f.open("ad2044.exe") || f.size() > 2u * 1024u * 1024u)
		error("Couldn't open ad2044.exe to read animation index");

	uint32 exeSize = static_cast<uint32>(f.size());

	Common::Array<byte> exeContents;
	exeContents.resize(exeSize);

	if (exeSize < sizeof(searchPattern) || f.read(&exeContents[0], exeSize) != exeSize)
		error("Couldn't load executable to scan for animation table");

	uint endPoint = exeSize - sizeof(searchPattern);

	bool foundAnimTable = false;
	uint32 animTablePos = 0;

	for (uint i = 0; i <= endPoint; i++) {
		bool match = true;
		for (uint j = 0; j < sizeof(searchPattern); j++) {
			if (exeContents[i + j] != searchPattern[j]) {
				match = false;
				break;
			}
		}

		if (match) {
			if (foundAnimTable)
				error("Found multiple byte patterns matching animation table in ad2044.exe");

			foundAnimTable = true;
			animTablePos = i;
		}
	}

	if (!foundAnimTable)
		error("Couldn't find animation table in ad2044.exe");

	// Parse the animation table.  The unparsed values are the same for all entries.
	for (uint entryPos = animTablePos; entryPos < exeSize - 15u; entryPos += 16) {
		const byte *entry = &exeContents[entryPos];

		AD2044AnimationDef animDef;
		animDef.roomID = entry[1];
		animDef.lookupID = entry[2];

		animDef.fwdAnimationID = READ_LE_INT16(entry + 10);
		animDef.revAnimationID = READ_LE_INT16(entry + 12);

		if (animDef.lookupID == 0)	// Terminator
			break;

		_ad2044AnimationDefs.push_back(animDef);
	}

	f.seek(0);

	Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(&f));

	if (!winRes)
		error("Couldn't open executable resources");

	_ad2044Graphics.reset(new AD2044Graphics(winRes, _lowQualityGraphicsMode, _gameSection.pixFmt));

	_ad2044Graphics->loadGraphic(&AD2044Graphics::invDownClicked, "GDOL");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::invUpClicked, "GGORA");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::musicClickedDeep, "GUZ03");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::soundClickedDeep, "GUZ06");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::exitClicked, "GUZ1");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::saveClicked, "GUZ10");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::resizeClicked, "GUZ2");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::musicClicked, "GUZ3");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::soundClicked, "GUZ6");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::musicVolUpClicked, "GUZ7");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::musicVolDownClicked, "GUZ8");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::loadClicked, "GUZ9");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::music, "GUZN3");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::musicVol, "GUZN4");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::sound, "GUZN6");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::soundVol, "GUZN7");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::musicDisabled, "NIC3");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::musicVolDisabled, "NIC4");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::soundDisabled, "NIC6");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::soundVolDisabled, "NIC7");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::examine, "OKO");
	_ad2044Graphics->loadGraphic(&AD2044Graphics::examineDisabled, "OKOZ");

	for (int i = 0; i < 8; i++)
		_ad2044Graphics->invPage[i] = _ad2044Graphics->loadGraphic(Common::String::format("POJ%i", static_cast<int>(i + 1)));

	_ad2044Graphics->finishLoading();

	Common::HashMap<uint32, Common::Pair<uint32, uint> > stringHashToFilePosAndLength;

	for (const AD2044ItemInfo &itemInfo : g_ad2044ItemInfos) {
		if (_language == Common::PL_POL)
			stringHashToFilePosAndLength[itemInfo.plNameCRC] = Common::Pair<uint32, uint>(0, 0);
		else
			stringHashToFilePosAndLength[itemInfo.enNameCRC] = Common::Pair<uint32, uint>(0, 0);
	}

	stringHashToFilePosAndLength.erase(0);

	// Scan for strings
	Common::CRC32 crc;

	uint32 strStartPos = 0;
	uint32 rollingCRC = crc.getInitRemainder();

	for (uint i = 0; i < exeContents.size(); i++) {
		byte b = exeContents[i];
		if (b == 0) {
			uint32 strLength = i - strStartPos;
			rollingCRC = crc.finalize(rollingCRC);
			if (strLength != 0) {
				Common::HashMap<uint32, Common::Pair<uint32, uint> >::iterator it = stringHashToFilePosAndLength.find(rollingCRC);
				if (it != stringHashToFilePosAndLength.end())
					it->_value = Common::Pair<uint32, uint> (strStartPos, strLength);
			}

#if 1
			if (strStartPos == 100460) {
				debug(1, "Check CRC was %u", rollingCRC);
			}
#endif

			rollingCRC = crc.getInitRemainder();
			strStartPos = i + 1;
		} else
			rollingCRC = crc.processByte(b, rollingCRC);
	}

	_ad2044ItemNames.clear();
	_ad2044ItemNames.reserve(ARRAYSIZE(g_ad2044ItemInfos));

	for (const AD2044ItemInfo &itemInfo : g_ad2044ItemInfos) {
		Common::String itemInfoUTF8;
		uint32 hash = itemInfo.enNameCRC;

		if (_language == Common::PL_POL)
			hash = itemInfo.plNameCRC;

		if (hash != 0) {
			Common::HashMap<uint32, Common::Pair<uint32, uint> >::const_iterator strIt = stringHashToFilePosAndLength.find(hash);

			if (strIt != stringHashToFilePosAndLength.end()) {
				uint32 filePos = strIt->_value.first;
				uint length = strIt->_value.second;

				if (length > 0) {
					Common::String str(reinterpret_cast<const char *>(&exeContents[filePos]), length);

					itemInfoUTF8 = str.decode(Common::CodePage::kWindows1250).encode(Common::kUtf8);
				}
			}
		}

		_ad2044ItemNames.push_back(itemInfoUTF8);
	}
}

void Runtime::findWaves() {
	if (_gameID == GID_AD2044) {
		for (int disc = 0; disc < 2; disc++) {
			for (int cat = 0; cat < 3; cat++) {
				if (disc == 1 && cat == 0)
					continue;

				char subdir[3] = {static_cast<char>('0' + disc), static_cast<char>('0' + cat), 0};

				Common::String searchPattern = Common::String("sfx/WAVE-") + subdir + "/########.WAV";

				Common::ArchiveMemberList waves;
				SearchMan.listMatchingMembers(waves, Common::Path(searchPattern, '/'), true);

				for (const Common::ArchiveMemberPtr &wave : waves) {
					Common::String name = wave->getFileName();

					// Strip .wav extension
					name = name.substr(0, name.size() - 4);

					// Make case-insensitive
					name.toLowercase();

					_waves[Common::String(subdir) + "-" + name] = wave;
				}
			}
		}
	} else {
		Common::ArchiveMemberList waves;
		SearchMan.listMatchingMembers(waves, "Sfx/Waves-##/####*.wav", true);

		for (const Common::ArchiveMemberPtr &wave : waves) {
			Common::String name = wave->getFileName();

			// Strip .wav extension
			name = name.substr(0, name.size() - 4);

			// Make case-insensitive
			name.toLowercase();

			_waves[name] = wave;
		}
	}
}

void Runtime::loadConfig(const char *filePath) {
	Common::INIFile configINI;
	if (!configINI.loadFromFile(filePath))
		error("Couldn't load config '%s'", filePath);

	for (uint i = 0; i < kNumStartConfigs; i++) {
		Common::String cfgKey = Common::String::format("dwStart%02u", i);
		Common::String startConfigValue;

		if (!configINI.getKey(cfgKey, "TextSettings", startConfigValue))
			error("Config key '%s' is missing", cfgKey.c_str());

		StartConfigDef &startDef = _startConfigs[i];
		if (sscanf(startConfigValue.c_str(), "0x%02x,0x%02x,0x%02x,0x%02x", &startDef.disc, &startDef.room, &startDef.screen, &startDef.direction) != 4)
			error("Start config key '%s' is malformed", cfgKey.c_str());
	}

	_isCDVariant = false;

	Common::String cdVersionValue;
	if (configINI.getKey("bStatusVersionCD", "ValueSettings", cdVersionValue)) {
		uint boolValue = 0;
		if (sscanf(cdVersionValue.c_str(), "%u", &boolValue) == 1)
			_isCDVariant = (boolValue != 0);
	}
}

void Runtime::loadScore() {
	Common::INIFile scoreINI;
	if (scoreINI.loadFromFile("Sfx/score.ini")) {

		for (const Common::INIFile::Section &section : scoreINI.getSections()) {
			ScoreTrackDef &trackDef = _scoreDefs[section.name];

			for (const Common::INIFile::KeyValue &kv : section.keys) {

				uint32 firstSpacePos = kv.value.find(' ', 0);
				if (firstSpacePos != Common::String::npos) {
					uint32 secondSpacePos = kv.value.find(' ', firstSpacePos + 1);

					if (secondSpacePos == Common::String::npos) {
						// Silent section
						Common::String durationSlice = kv.value.substr(0, firstSpacePos);
						Common::String nextSectionSlice = kv.value.substr(firstSpacePos + 1);

						uint duration = 0;
						if (sscanf(durationSlice.c_str(), "%u", &duration) == 1) {
							ScoreSectionDef &sectionDef = trackDef.sections[kv.key];
							sectionDef.nextSection = nextSectionSlice;
							sectionDef.volumeOrDurationInSeconds = duration;
						} else
							warning("Couldn't parse score silent section duration");
					} else {
						Common::String fileNameSlice = kv.value.substr(0, firstSpacePos);
						Common::String volumeSlice = kv.value.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
						Common::String nextSectionSlice = kv.value.substr(secondSpacePos + 1);

						int volume = 0;
						if (sscanf(volumeSlice.c_str(), "%i", &volume) == 1) {
							ScoreSectionDef &sectionDef = trackDef.sections[kv.key];
							sectionDef.nextSection = nextSectionSlice;
							sectionDef.volumeOrDurationInSeconds = volume;
							sectionDef.musicFileName = fileNameSlice;
						} else
							warning("Couldn't parse score section volume");
					}
				}
			}
		}
	} else
		warning("Couldn't load music score");
}

void Runtime::loadDuplicateRooms() {
	assert(_gameID == GID_SCHIZM);

	Common::ArchiveMemberList logics;
	SearchMan.listMatchingMembers(logics, "Log/Room##.log", true);

	for (const Common::ArchiveMemberPtr &logic : logics) {
		Common::String name = logic->getName();

		char d10 = name[4];
		char d1 = name[5];

		uint roomNumber = (d10 - '0') * 10 + (d1 - '0');

		Common::SharedPtr<Common::SeekableReadStream> stream(logic->createReadStream());
		if (stream) {
			if (checkSchizmLogicForDuplicatedRoom(*stream, stream->size())) {
				if (_roomDuplicationOffsets.size() <= roomNumber)
					_roomDuplicationOffsets.resize(roomNumber + 1);
				_roomDuplicationOffsets[roomNumber] = 1;
			}
		} else {
			warning("Logic for room %u couldn't be checked for duplication", roomNumber);
		}
	}

	for (uint i = 1; i < _roomDuplicationOffsets.size(); i++) {
		if (_roomDuplicationOffsets[i])
			_roomDuplicationOffsets[i] += _roomDuplicationOffsets[i - 1];
	}
}

void Runtime::loadAllSchizmScreenNames() {
	assert(_gameID == GID_SCHIZM);

	Common::ArchiveMemberList logics;
	SearchMan.listMatchingMembers(logics, "Log/Room##.log", true);

	Common::Array<uint> roomsToCompile;

	for (const Common::ArchiveMemberPtr &logic : logics) {
		Common::String name = logic->getName();

		char d10 = name[4];
		char d1 = name[5];

		uint roomNumber = (d10 - '0') * 10 + (d1 - '0');

		// Rooms 1 and 3 are always compiled.  2 is a cheat room that contains garbage.  We still need to compile room 1
		// to get the START screen to start the game though.
		if (roomNumber > 3 || roomNumber == 1)
			roomsToCompile.push_back(roomNumber);
	}

	Common::sort(roomsToCompile.begin(), roomsToCompile.end());

	for (uint roomNumber : roomsToCompile) {
		if (roomNumber >= _roomDuplicationOffsets.size() || _roomDuplicationOffsets[roomNumber] == 0) {
			uint roomSetToCompile[3] = {1, 3, roomNumber};

			uint numRooms = 3;
			if (roomNumber == 1)
				numRooms = 2;

			Common::SharedPtr<ScriptSet> scriptSet = compileSchizmLogicSet(roomSetToCompile, numRooms);

			for (const RoomScriptSetMap_t::Node &rssNode : scriptSet->roomScripts) {
				if (rssNode._key != roomNumber)
					continue;

				for (const ScreenNameMap_t::Node &snNode : rssNode._value->screenNames)
					_globalRoomScreenNameToScreenIDs[roomNumber][snNode._key] = snNode._value;
			}
		}
	}
}

Common::SharedPtr<SoundInstance> Runtime::loadWave(const Common::String &soundName, uint soundID, const Common::ArchiveMemberPtr &archiveMemberPtr) {
	for (const Common::SharedPtr<SoundInstance> &activeSound : _activeSounds) {
		if (activeSound->name == soundName)
			return activeSound;
	}

	Common::SharedPtr<SoundInstance> soundInstance(new SoundInstance());

	soundInstance->name = soundName;
	soundInstance->id = soundID;

	bool foundExisting = false;
	for (Common::SharedPtr<SoundInstance> &existingSound : _activeSounds) {
		if (existingSound->id == soundID) {
			existingSound = soundInstance;
			foundExisting = true;
			break;
		}
	}

	if (!foundExisting)
		_activeSounds.push_back(soundInstance);

	return soundInstance;
}

SoundCache *Runtime::loadCache(SoundInstance &sound) {
	if (sound.cache)
		return sound.cache.get();

	// See if this is already in the cache
	for (const Common::Pair<Common::String, Common::SharedPtr<SoundCache> > &cacheItem : _soundCache) {
		if (cacheItem.first == sound.name) {
			sound.cache = cacheItem.second;
			return sound.cache.get();
		}
	}

	Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator waveIt = _waves.find(sound.name);

	if (waveIt == _waves.end())
		return nullptr;

	Common::SeekableReadStream *stream = waveIt->_value->createReadStream();
	if (!stream) {
		warning("Couldn't open read stream for sound '%s'", sound.name.c_str());
		return nullptr;
	}

	if (_preloadSounds) {
		if (stream->size() > static_cast<int64>(0xffffffffu)) {
			warning("Sound stream is too large");
			delete stream;
			return nullptr;
		}

		uint32 streamSize = static_cast<uint32>(stream->size());

		byte *streamContents = new byte[streamSize];

		if (stream->read(streamContents, streamSize) != streamSize) {
			warning("Couldn't preload sound contents for sound '%s'", sound.name.c_str());
			delete[] streamContents;
			delete stream;
			return nullptr;
		}

		Common::MemoryReadStream *memStream = new Common::MemoryReadStream(streamContents, streamSize);
		delete stream;

		stream = memStream;
	}

	Common::SharedPtr<SoundLoopInfo> loopInfo;

	if (_gameID == GID_SCHIZM) {
		loopInfo = SoundLoopInfo::readFromWaveFile(*stream);
		if (!stream->seek(0)) {
			warning("Couldn't reset stream to 0 after reading sample table for sound '%s'", sound.name.c_str());
			delete stream;
			return nullptr;
		}
	}

	Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Couldn't open audio stream for sound '%s'", sound.name.c_str());
		return nullptr;
	}

	Common::SharedPtr<SoundCache> cachedSound(new SoundCache());

	cachedSound->stream.reset(audioStream);
	cachedSound->loopInfo = loopInfo;

	_soundCache[_soundCacheIndex].first = sound.name;
	_soundCache[_soundCacheIndex].second = cachedSound;

	_soundCacheIndex++;
	if (_soundCacheIndex == kSoundCacheSize)
		_soundCacheIndex = 0;

	sound.cache = cachedSound;

	return cachedSound.get();
}

void Runtime::resolveSoundByName(const Common::String &soundName, bool load, StackInt_t &outSoundID, SoundInstance *&outWave) {
	Common::String sndName = soundName;

	uint soundID = 0;

	if (_gameID == GID_AD2044) {
		for (uint i = 0; i < 2; i++)
			soundID = soundID * 10u + (sndName[i] - '0');
		for (uint i = 0; i < 5; i++)
			soundID = soundID * 10u + (sndName[6 + i] - '0');
	} else {
		for (uint i = 0; i < 4; i++)
			soundID = soundID * 10u + (sndName[i] - '0');
	}

	sndName.toLowercase();

	outSoundID = soundID;
	outWave = nullptr;

	for (const Common::SharedPtr<SoundInstance> &snd : _activeSounds) {
		if (snd->name == sndName) {
			outWave = snd.get();
			return;
		}
	}

	if (load) {
		Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator waveIt = _waves.find(sndName);

		if (waveIt != _waves.end()) {
			Common::SharedPtr<SoundInstance> snd = loadWave(sndName, soundID, waveIt->_value);
			outWave = snd.get();
		}
	}
}

SoundInstance *Runtime::resolveSoundByID(uint soundID) {
	for (const Common::SharedPtr<SoundInstance> &snd : _activeSounds) {
		if (snd->id == soundID)
			return snd.get();
	}

	return nullptr;
}

void Runtime::resolveSoundByNameOrID(const StackValue &stackValue, bool load, StackInt_t &outSoundID, SoundInstance *&outWave) {
	outSoundID = 0;
	outWave = nullptr;

	if (stackValue.type == StackValue::kNumber) {
		outSoundID = stackValue.value.i;

		for (const Common::SharedPtr<SoundInstance> &snd : _activeSounds) {
			if (snd->id == static_cast<uint>(stackValue.value.i)) {
				outWave = snd.get();
				break;
			}
		}
		return;
	}

	if (stackValue.type == StackValue::kString)
		resolveSoundByName(stackValue.value.s, load, outSoundID, outWave);
}

void Runtime::changeToScreen(uint roomNumber, uint screenNumber) {
	bool changedRoom = (roomNumber != _loadedRoomNumber);
	bool changedScreen = (screenNumber != _activeScreenNumber) || changedRoom || _forceScreenChange;

	_forceScreenChange = false;

	_roomNumber = roomNumber;
	_screenNumber = screenNumber;

	_loadedRoomNumber = roomNumber;
	_activeScreenNumber = screenNumber;

	if (changedRoom) {
		// This shouldn't happen when running a script
		assert(_scriptCallStack.empty());

		if (_gameID == GID_SCHIZM) {
			uint roomsToCompile[3] = {1, 3, 0};
			uint numRoomsToCompile = 2;

			if (roomNumber != 1 && roomNumber != 3)
				roomsToCompile[numRoomsToCompile++] = roomNumber;

			_scriptSet.reset();
			_scriptSet = compileSchizmLogicSet(roomsToCompile, numRoomsToCompile);
		} else if (_gameID == GID_REAH) {
			_scriptSet.reset();

			Common::Path logicFileName(Common::String::format("Log/Room%02i.log", static_cast<int>(roomNumber)));
			Common::File logicFile;
			if (logicFile.open(logicFileName)) {
				_scriptSet = compileReahLogicFile(logicFile, static_cast<uint>(logicFile.size()), logicFileName);

				logicFile.close();
			}
		} else if (_gameID == GID_AD2044) {
			_scriptSet.reset();

			Common::Path logicFileName(Common::String::format("log/kom%i.log", static_cast<int>(roomNumber)));
			Common::File logicFile;
			if (logicFile.open(logicFileName)) {
				_scriptSet = compileAD2044LogicFile(logicFile, static_cast<uint>(logicFile.size()), logicFileName);

				logicFile.close();
			}
		} else
			error("Don't know how to compile scripts for this game");

		_mapLoader->unload();

		_mapLoader->setRoomNumber(roomNumber);
	}

	if (changedScreen) {
		_gyros.reset();

		_swapOutDirection = 0;
		_swapOutRoom = 0;
		_swapOutScreen = 0;

		if (_scriptSet) {
			RoomScriptSet *roomScriptSet = getRoomScriptSetForCurrentRoom();

			if (roomScriptSet) {
				const ScreenScriptSetMap_t &screenScriptsMap = roomScriptSet->screenScripts;
				ScreenScriptSetMap_t::const_iterator screenScriptIt = screenScriptsMap.find(_screenNumber);
				if (screenScriptIt != screenScriptsMap.end()) {
					const Common::SharedPtr<Script> &script = screenScriptIt->_value->entryScript;
					if (script)
						activateScript(script, true, ScriptEnvironmentVars());
				}
			}
		}

		_haveHorizPanAnimations = false;
		for (uint i = 0; i < kNumDirections; i++) {
			_havePanUpFromDirection[i] = false;
			_havePanDownFromDirection[i] = false;
		}

		if (!_keepStaticAnimationInIdle)
			clearIdleAnimations();

		_forceAllowSaves = false;

		recordSaveGameSnapshot();

		if (_keepStaticAnimationInIdle)
			_keepStaticAnimationInIdle = false;

		_placedItemRect = Common::Rect();
		if (_gameID == GID_AD2044) {
			const MapScreenDirectionDef *screenDef = _mapLoader->getScreenDirection(_screenNumber, _direction);
			if (screenDef) {
				for (const InteractionDef &interaction : screenDef->interactions) {
					if (interaction.objectType == 2) {
						_placedItemRect = interaction.rect;
						break;
					}
				}
			}

			_placedItemRect = _placedItemRect.findIntersectingRect(Common::Rect(640, 480));

			Common::Rect renderRect = _placedItemRect;
			renderRect.translate(_gameSection.rect.left, _gameSection.rect.top);

			_placedItemBackBufferSection.init(renderRect, _placedItemBackBufferSection.pixFmt);

			if (!_placedItemRect.isEmpty())
				_placedItemBackBufferSection.surf->blitFrom(*_gameSection.surf, _placedItemRect, Common::Point(0, 0));

			updatePlacedItemCache();

			drawPlacedItemGraphic();
		}
	}
}

void Runtime::clearIdleAnimations() {
	for (uint i = 0; i < kNumDirections; i++)
		_haveIdleAnimations[i] = false;

	_haveIdleStaticAnimation = false;
	_idleCurrentStaticAnimation.clear();
	_havePendingPreIdleActions = true;
	_havePendingPlayAmbientSounds = true;
}

void Runtime::changeHero() {
	assert(canSave(true));

	recordSaveGameSnapshot();

	SaveGameSnapshot *snapshot = _mostRecentlyRecordedSaveState.get();

	Common::SharedPtr<SaveGameSwappableState> currentState = snapshot->states[0];
	Common::SharedPtr<SaveGameSwappableState> alternateState = snapshot->states[1];

	if (_swapOutRoom && _swapOutScreen) {
		// Some scripts may kick the player out to another location on swap back,
		// such as the elevator in the first area on Hannah's quest.  This is done
		// via the "heroOut" op.
		currentState->roomNumber = _swapOutRoom;
		currentState->screenNumber = _swapOutScreen;
		currentState->direction = _swapOutDirection;
		currentState->havePendingPostSwapScreenReset = true;
	}

	snapshot->states[0] = alternateState;
	snapshot->states[1] = currentState;

	snapshot->hero ^= 1u;

	changeToCursor(_cursors[kCursorArrow]);

	_mostRecentValidSaveState = _mostRecentlyRecordedSaveState;

	restoreSaveGameSnapshot();
}

void Runtime::changeToExamineItem() {
	assert(canSave(true));
	assert(_hero == 0);

	InventoryItem itemToExamine = _inventoryActiveItem;

	_inventoryActiveItem = InventoryItem();

	recordSaveGameSnapshot();

	SaveGameSnapshot *snapshot = _mostRecentlyRecordedSaveState.get();

	Common::SharedPtr<SaveGameSwappableState> currentState = snapshot->states[0];
	Common::SharedPtr<SaveGameSwappableState> alternateState = snapshot->states[1];

	// Move inventory into the new state
	alternateState->inventory.clear();
	alternateState->inventory = Common::move(currentState->inventory);

	// For some reason the screen number translation converts decimal to hex
	uint highDigit = itemToExamine.itemID / 10u;
	uint lowDigit = itemToExamine.itemID % 10u;

	Common::HashMap<int, AnimFrameRange>::const_iterator frameRangeIt = _examineAnimIDToFrameRange.find(8700 + static_cast<int>(itemToExamine.itemID));

	if (frameRangeIt == _examineAnimIDToFrameRange.end())
		error("Couldn't resolve animation frame range to examine item %u", static_cast<uint>(itemToExamine.itemID));

	alternateState->loadedAnimation = frameRangeIt->_value.animationNum;
	alternateState->animDisplayingFrame = frameRangeIt->_value.firstFrame;

	alternateState->havePendingPostSwapScreenReset = true;
	alternateState->roomNumber = 87;
	alternateState->screenNumber = (highDigit * 0x10u) + lowDigit;
	alternateState->direction = 0;

	alternateState->musicActive = currentState->musicActive;
	alternateState->musicMuteDisabled = currentState->musicMuteDisabled;
	alternateState->musicTrack = currentState->musicTrack;
	alternateState->musicVolume = currentState->musicVolume;

	snapshot->states[0] = alternateState;
	snapshot->states[1] = currentState;

	snapshot->hero ^= 1u;

	changeToCursor(_cursors[kCursorArrow]);

	_mostRecentValidSaveState = _mostRecentlyRecordedSaveState;

	restoreSaveGameSnapshot();
}

void Runtime::returnFromExaminingItem() {
	assert(canSave(true));
	assert(_hero == 1);

	InventoryItem itemToReturnWith = _inventoryActiveItem;

	_inventoryActiveItem = InventoryItem();

	recordSaveGameSnapshot();

	SaveGameSnapshot *snapshot = _mostRecentlyRecordedSaveState.get();

	Common::SharedPtr<SaveGameSwappableState> currentState = snapshot->states[0];
	Common::SharedPtr<SaveGameSwappableState> alternateState = snapshot->states[1];

	// Move inventory into the new state
	alternateState->inventory.clear();
	alternateState->inventory = Common::move(currentState->inventory);

	snapshot->inventoryActiveItem = itemToReturnWith.itemID;

	snapshot->states[0] = alternateState;
	snapshot->states[1] = currentState;

	snapshot->hero ^= 1u;

	changeToCursor(_cursors[kCursorArrow]);

	_mostRecentValidSaveState = _mostRecentlyRecordedSaveState;

	restoreSaveGameSnapshot();
}

bool Runtime::triggerPreIdleActions() {
	debug(1, "Triggering pre-idle actions in room %u screen 0%x facing direction %u", _roomNumber, _screenNumber, _direction);

	_havePendingReturnToIdleState = true;

	uint32 timestamp = g_system->getMillis();

	_animPlayWhileIdle = false;
	_idleLockInteractions = false;

	if (_haveIdleAnimations[_direction]) {
		StaticAnimation &sanim = _idleAnimations[_direction];
		sanim.currentAlternation = 0;
		sanim.nextStartTime = timestamp + sanim.params.initialDelay * 1000u;

		if (sanim.params.initialDelay == 0) {
			changeAnimation(sanim.animDefs[0], sanim.animDefs[0].firstFrame, false, _animSpeedStaticAnim);
			_animPlayWhileIdle = true;
			sanim.currentAlternation = 1;
		}

		_havePendingPostSwapScreenReset = false;
	} else if (_havePendingPostSwapScreenReset) {
		_havePendingPostSwapScreenReset = false;

		if (_haveHorizPanAnimations) {
			AnimationDef animDef = _panRightAnimationDef;
			uint rotationFrame = _direction * (animDef.lastFrame - animDef.firstFrame) / kNumDirections + animDef.firstFrame;
			animDef.firstFrame = rotationFrame;
			animDef.lastFrame = rotationFrame;

			changeAnimation(animDef, false);

			if (_gameState == kGameStateScript || _gameState == kGameStateIdle || _gameState == kGameStateScriptReset)
				_gameState = kGameStateWaitingForAnimation;
			else if (_gameState == kGameStateDelay)
				_gameState = kGameStateWaitingForAnimationToDelay;
			else
				error("Triggered pre-idle actions from an unexpected game state");
		}

		return true;
	}

	return false;
}

void Runtime::returnToIdleState() {
	debug(1, "Returned to idle state in room %u screen 0%x facing direction %u", _roomNumber, _screenNumber, _direction);

	if (canSave(true))
		_mostRecentValidSaveState = _mostRecentlyRecordedSaveState;
	
	_idleIsOnInteraction = false;
	_idleHaveClickInteraction = false;
	_idleHaveDragInteraction = false;

	// Do this before detectPanoramaMouseMovement so continuous panorama keeps the correct cursor
	changeToCursor(_cursors[kCursorArrow]);

	detectPanoramaDirections();

	_panoramaState = kPanoramaStateInactive;
	detectPanoramaMouseMovement(g_system->getMillis());

	(void) dischargeIdleMouseMove();
}

void Runtime::changeToCursor(const Common::SharedPtr<AnimatedCursor> &cursor) {
	if (!cursor)
		CursorMan.showMouse(false);
	else {
		_currentAnimatedCursor = cursor.get();

		_cursorCycleLength = 0;
		for (const AnimatedCursor::FrameDef &frame : cursor->frames)
			_cursorCycleLength += frame.delay;

		_cursorTimeBase = g_system->getMillis(true);

		refreshCursor(_cursorTimeBase);
		CursorMan.showMouse(true);
	}
}

void Runtime::refreshCursor(uint32 currentTime) {
	if (!_currentAnimatedCursor)
		return;

	uint32 timeSinceTimeBase = currentTime - _cursorTimeBase;

	uint stepTime = 0;

	if (_cursorCycleLength > 0) {
		// 3 ticks at 60Hz is 50ms, so this will reduce the precision of the math that we have to do
		timeSinceTimeBase %= _cursorCycleLength * 50u;
		_cursorTimeBase = currentTime - timeSinceTimeBase;

		stepTime = (timeSinceTimeBase * 60u / 1000u) % _cursorCycleLength;
	}

	uint imageIndex = 0;

	if (_currentAnimatedCursor) {
		uint frameStartTime = 0;
		for (const AnimatedCursor::FrameDef &frame : _currentAnimatedCursor->frames) {
			if (frameStartTime > stepTime)
				break;

			imageIndex = frame.imageIndex;
			frameStartTime += frame.delay;
		}
	}

	if (imageIndex >= _currentAnimatedCursor->images.size())
		error("Out-of-bounds animated cursor image index");

	Graphics::Cursor *cursor = _currentAnimatedCursor->images[imageIndex];

	if (!cursor)
		error("Missing cursor");

	CursorMan.replaceCursor(cursor);
}

bool Runtime::dischargeIdleMouseMove() {
	const MapScreenDirectionDef *sdDef = _mapLoader->getScreenDirection(_screenNumber, _direction);

	if (_inGameMenuState != kInGameMenuStateInvisible) {
		checkInGameMenuHover();

		// If still in the menu, ignore anything else
		if (_inGameMenuState != kInGameMenuStateInvisible)
			return false;
	}

	if (_panoramaState != kPanoramaStateInactive) {
		uint interactionID = 0;

		Common::Point panRelMouse = _mousePos - _panoramaAnchor;
		if (_haveHorizPanAnimations) {
			if (panRelMouse.x <= -kPanoramaPanningMarginX)
				interactionID = kPanLeftInteraction;
			else if (panRelMouse.x >= kPanoramaPanningMarginX)
				interactionID = kPanRightInteraction;
		}

		if (!interactionID) {
			if (_havePanUpFromDirection[_direction] && panRelMouse.y <= -kPanoramaPanningMarginY) {
				interactionID = kPanUpInteraction;
			} else if (_havePanDownFromDirection[_direction] && panRelMouse.y >= kPanoramaPanningMarginY) {
				interactionID = kPanDownInteraction;
			}
		}

		if (interactionID) {
			// If there's an interaction script for this direction, execute it
			Common::SharedPtr<Script> script = findScriptForInteraction(interactionID);

			if (script) {
				resetInventoryHighlights();

				ScriptEnvironmentVars vars;
				vars.panInteractionID = interactionID;
				activateScript(script, false, vars);
				return true;
			}
		}
	}

	Common::Point relMouse(_mousePos.x - _gameSection.rect.left, _mousePos.y - _gameSection.rect.top);

	bool isOnInteraction = false;
	uint interactionID = 0;
	if (sdDef && !_idleLockInteractions) {
		for (const InteractionDef &idef : sdDef->interactions) {
			if (idef.objectType == 1 && interactionID == 0 && idef.rect.contains(relMouse)) {
				isOnInteraction = true;
				interactionID = idef.interactionID;
			}

			if (_gameID == GID_AD2044 && idef.objectType == 3 && idef.rect.contains(relMouse)) {
				uint32 locationID = getLocationForScreen(_roomNumber, _screenNumber);
				if (_placedItems.find(locationID) == _placedItems.end()) {
					if (_inventoryActiveItem.itemID != 0) {
						isOnInteraction = true;
						interactionID = kObjectDropInteractionID;
					}
				} else {
					if (_inventoryActiveItem.itemID == 0) {
						isOnInteraction = true;
						interactionID = kObjectPickupInteractionID;
					}
				}
				break;
			}
		}
	}

	if (_gameID == GID_SCHIZM && !isOnInteraction) {
		if (_traySection.rect.contains(_mousePos) && (_traySection.rect.right - _mousePos.x) < (int) 88u && canSave(true)) {
			isOnInteraction = true;
			interactionID = kHeroChangeInteractionID;
		}
	}

	if (_gameID == GID_AD2044 && !isOnInteraction) {
		Common::Rect invSlotRect = AD2044Interface::getFirstInvSlotRect();

		for (uint i = 0; i < kNumInventorySlots; i++) {
			bool isItemInInventory = (_inventory[i].itemID != 0);
			bool isItemActive = (_inventoryActiveItem.itemID != 0);

			if (invSlotRect.contains(_mousePos)) {
				if (isItemInInventory && !isItemActive) {
					isOnInteraction = true;
					interactionID = kPickupInventorySlot0InteractionID + i;
				} else if (!isItemInInventory && isItemActive) {
					isOnInteraction = true;
					interactionID = kReturnInventorySlot0InteractionID + i;
				}

				break;
			}

			invSlotRect.translate(static_cast<int16>(AD2044Interface::getInvSlotSpacing()), 0);
		}

		if (_inventoryActiveItem.itemID != 0 && _hero == 0) {
			if (g_ad2044ItemInfos[_inventoryActiveItem.itemID].canBeExamined) {
				Common::Rect examineRect = AD2044Interface::getRectForUI(AD2044InterfaceRectID::ExamineButton);

				if (examineRect.contains(_mousePos)) {
					isOnInteraction = true;
					interactionID = kExamineItemInteractionID;
				}
			}
		}
	}

	if (_idleIsOnInteraction && (!isOnInteraction || interactionID != _idleInteractionID)) {
		// Mouse left the previous interaction
		_idleIsOnInteraction = false;
		_idleHaveClickInteraction = false;
		_idleHaveDragInteraction = false;
		changeToCursor(_cursors[kCursorArrow]);
		resetInventoryHighlights();

		if (_gameID == GID_AD2044 && _tooltipText.size() > 0) {
			_tooltipText.clear();
			redrawSubtitleSection();
		}
	}

	bool changedCircuitState = false;
	bool isOnCircuitLink = false;
	bool isCircuitLinkDown = false;

	bool wasOnOpenCircuitLink = _idleIsOnOpenCircuitPuzzleLink;

	Common::Point circuitCoord;
	Common::Rect circuitHighlightRect;
	if (_circuitPuzzle) {
		isOnCircuitLink = resolveCircuitPuzzleInteraction(relMouse, circuitCoord, isCircuitLinkDown, circuitHighlightRect);

		if (isOnCircuitLink != _idleIsOnOpenCircuitPuzzleLink)
			changedCircuitState = true;
		else {
			if (isOnCircuitLink && (circuitCoord != _idleCircuitPuzzleCoord || isCircuitLinkDown != _idleIsCircuitPuzzleLinkDown))
				changedCircuitState = true;
		}
	}

	if (changedCircuitState) {
		_idleIsOnOpenCircuitPuzzleLink = isOnCircuitLink;

		if (wasOnOpenCircuitLink)
			clearCircuitHighlightRect(_idleCircuitPuzzleLinkHighlightRect);

		if (isOnCircuitLink) {
			// Started being on a circuit link
			_idleCircuitPuzzleLinkHighlightRect = circuitHighlightRect;
			drawCircuitHighlightRect(_idleCircuitPuzzleLinkHighlightRect);

			_idleCircuitPuzzleCoord = circuitCoord;
			_idleIsCircuitPuzzleLinkDown = isCircuitLinkDown;
		} else {
			// No longer on a circuit link
			_idleCircuitPuzzleCoord = Common::Point(0, 0);
			_idleIsCircuitPuzzleLinkDown = false;
		}

		_idleCircuitPuzzleLinkHighlightRect = circuitHighlightRect;
	}

	if (isOnInteraction && (_idleIsOnInteraction == false || changedCircuitState)) {
		_idleIsOnInteraction = true;
		_idleInteractionID = interactionID;

		if (interactionID == kHeroChangeInteractionID) {
			changeToCursor(_cursors[16]);
			_idleHaveClickInteraction = true;
		} else if (interactionID == kExamineItemInteractionID) {
			changeToCursor(_cursors[5]);
			_idleHaveClickInteraction = true;
		} else if (interactionID == kObjectDropInteractionID || (interactionID >= kReturnInventorySlot0InteractionID && interactionID <= kReturnInventorySlot5InteractionID)) {
			changeToCursor(_cursors[7]);
			_idleHaveClickInteraction = true;
		} else if (interactionID == kObjectPickupInteractionID || (interactionID >= kPickupInventorySlot0InteractionID && interactionID <= kPickupInventorySlot5InteractionID)) {
			changeToCursor(_cursors[8]);
			_idleHaveClickInteraction = true;
		} else {
			// New interaction, is there a script?
			Common::SharedPtr<Script> script = findScriptForInteraction(interactionID);

			if (script) {
				ScriptEnvironmentVars envVars;
				envVars.clickInteractionID = interactionID;
				activateScript(script, false, envVars);
				return true;
			}
		}
	}

	if (_panoramaState == kPanoramaStateInactive)
		checkInGameMenuHover();

	// Didn't do anything
	return false;
}

bool Runtime::dischargeIdleMouseDown() {
	if (_inGameMenuState != kInGameMenuStateInvisible) {
		if (_inGameMenuState == kInGameMenuStateHoveringActive) {
			_inGameMenuState = kInGameMenuStateClickingOver;
			drawInGameMenuButton(_inGameMenuActiveElement);
		}
		return false;
	}

	if (_idleIsOnInteraction && _idleHaveDragInteraction) {
		// Interaction, is there a script?
		Common::SharedPtr<Script> script = findScriptForInteraction(_idleInteractionID);

		_idleIsOnInteraction = false; // ?
		resetInventoryHighlights();

		if (script) {
			ScriptEnvironmentVars vars;
			vars.lmbDrag = true;
			vars.clickInteractionID = _idleInteractionID;

			activateScript(script, false, vars);
			return true;
		}
	}

	// Didn't do anything
	return false;
}

bool Runtime::dischargeIdleClick() {
	if (_idleIsOnInteraction && _idleHaveClickInteraction) {
		if (_gameID == GID_SCHIZM && _idleInteractionID == kHeroChangeInteractionID) {
			changeHero();
			return true;
		} else if (_gameID == GID_AD2044 && _idleInteractionID == kObjectDropInteractionID) {
			dropActiveItem();
			recordSaveGameSnapshot();
			_havePendingReturnToIdleState = true;
			return true;
		} else if (_gameID == GID_AD2044 && _idleInteractionID == kObjectPickupInteractionID) {
			pickupPlacedItem();
			recordSaveGameSnapshot();
			_havePendingReturnToIdleState = true;
			return true;
		} else if (_gameID == GID_AD2044 && _idleInteractionID == kExamineItemInteractionID) {
			changeToExamineItem();
			return true;
		} else if (_gameID == GID_AD2044 && _idleInteractionID >= kPickupInventorySlot0InteractionID && _idleInteractionID <= kPickupInventorySlot5InteractionID) {
			pickupInventoryItem(_idleInteractionID - kPickupInventorySlot0InteractionID);
			recordSaveGameSnapshot();
			_havePendingReturnToIdleState = true;
			return true;
		} else if (_gameID == GID_AD2044 && _idleInteractionID >= kReturnInventorySlot0InteractionID && _idleInteractionID <= kReturnInventorySlot5InteractionID) {
			stashActiveItemToInventory(_idleInteractionID - kReturnInventorySlot0InteractionID);
			recordSaveGameSnapshot();
			_havePendingReturnToIdleState = true;
			return true;
		} else {
			// Interaction, is there a script?
			Common::SharedPtr<Script> script = findScriptForInteraction(_idleInteractionID);

			_idleIsOnInteraction = false; // Clear so new interactions at the same mouse coord are detected

			if (script) {
				ScriptEnvironmentVars vars;
				vars.lmb = true;
				vars.clickInteractionID = _idleInteractionID;

				activateScript(script, false, vars);
				return true;
			}
		}
	}

	// Didn't do anything
	return false;
}

void Runtime::loadFrameData(Common::SeekableReadStream *stream) {
	int64 size = stream->size();
	if (size < 2048 || size > 0xffffffu)
		error("Unexpected DTA size");

	uint numFrameDatas = (static_cast<uint>(size) - 2048u) / 16;

	if (!stream->seek(2048))
		error("Error skipping DTA header");

	_frameData.resize(numFrameDatas);

	for (uint i = 0; i < numFrameDatas; i++) {
		byte frameData[16];

		if (stream->read(frameData, 16) != 16)
			error("Error reading DTA frame data");

		FrameData &fd = _frameData[i];
		fd.frameType = frameData[0];
		fd.frameIndex = frameData[1] | (frameData[2] << 8) | (frameData[3] << 16);
		fd.roomNumber = static_cast<int8>(frameData[4]);
		memcpy(fd.areaID, frameData + 8, 4);

		char decAreaFrameIndex[4];
		memcpy(decAreaFrameIndex, frameData + 12, 4);

		bool isPadFrame = false;

		if (_gameID == GID_AD2044) {
			isPadFrame = true;

			for (uint j = 0; j < 8; j++) {
				if (frameData[j + 8] != 0) {
					isPadFrame = false;
					break;
				}
			}
		}

		uint areaFrameIndex = 0;
		if (!isPadFrame) {
			for (int digit = 0; digit < 4; digit++) {
				char c = decAreaFrameIndex[digit];
				if (c < '0' || c > '9')
					error("Invalid area frame index in DTA data");

				areaFrameIndex = areaFrameIndex * 10u + static_cast<uint>(c - '0');
			}
		}

		fd.areaFrameIndex = areaFrameIndex;

		if (i != fd.frameIndex)
			error("DTA frame index was out-of-line, don't know how to handle this");
	}
}

void Runtime::loadFrameData2(Common::SeekableReadStream *stream) {
	int64 size = stream->size();
	if (size > 0xffffffu)
		error("Unexpected 2DT size");

	uint numFrameDatas = static_cast<uint>(size) / 16;

	if (numFrameDatas == 0)
		return;

	_frameData2.resize(numFrameDatas);

	uint32 numBytesToRead = numFrameDatas * 16;
	if (stream->read(&_frameData2[0], numBytesToRead) != numBytesToRead)
		error("Failed to read 2DT data");

	for (uint i = 0; i < numFrameDatas; i++) {
		FrameData2 &fd2 = _frameData2[i];
		fd2.x = READ_LE_INT32(&fd2.x);
		fd2.y = READ_LE_INT32(&fd2.y);
		fd2.angle = READ_LE_INT32(&fd2.angle);
		fd2.frameNumberInArea = READ_LE_UINT16(&fd2.frameNumberInArea);
		fd2.unknown = READ_LE_UINT16(&fd2.unknown);
	}
}

void Runtime::loadTabData(Common::HashMap<int, AnimFrameRange> &animIDToFrameRangeMap, uint animNumber, Common::SeekableReadStream *stream) {
	animIDToFrameRangeMap.clear();

	int64 size64 = stream->size() - stream->pos();

	if (size64 > UINT_MAX || size64 < 0)
		error("Internal error: Oversized TAB file");

	uint32 fileSize = static_cast<uint32>(size64);

	Common::Array<Common::String> lines;

	Common::Array<char> chars;
	chars.resize(fileSize);

	if (fileSize > 0 && stream->read(&chars[0], fileSize) != fileSize)
		error("Failed to read TAB file data");

	uint searchStart = 0;
	while (searchStart < chars.size()) {
		uint endPos = searchStart;
		while (endPos != chars.size()) {
			if (chars[endPos] == '\n' || chars[endPos] == '\r')
				break;

			endPos++;
		}

		if (endPos == searchStart)
			lines.push_back(Common::String(""));
		else
			lines.push_back(Common::String(&chars[searchStart], endPos - searchStart));


		if (endPos != chars.size() && chars[endPos] == '\r')
			endPos++;
		if (endPos != chars.size() && chars[endPos] == '\n')
			endPos++;

		searchStart = endPos;
	}

	for (const Common::String &line : lines) {
		if (line.hasPrefix("//"))
			continue;

		uint32 openBracePos = line.find('{');
		if (openBracePos == Common::String::npos)
			continue;

		uint32 closeBracePos = line.find("},", openBracePos + 1);
		if (closeBracePos == Common::String::npos)
			error("Strangely-formatted animation table line: %s", line.c_str());

		Common::String enclosedContents = line.substr(1, closeBracePos - 1);

		uint32 firstCommaPos = enclosedContents.find(',');
		if (firstCommaPos == Common::String::npos)
			error("Strangely-formatted animation table line: %s", line.c_str());

		uint32 secondCommaPos = enclosedContents.find(',', firstCommaPos + 1);
		if (secondCommaPos == Common::String::npos)
			error("Strangely-formatted animation table line: %s", line.c_str());

		Common::String numbers[3] = {
			enclosedContents.substr(0, firstCommaPos),
			enclosedContents.substr(firstCommaPos + 1, secondCommaPos - (firstCommaPos + 1)),
			enclosedContents.substr(secondCommaPos + 1)
		};

		int parsedNumbers[3] = {0, 0, 0};

		uint i = 0;
		for (Common::String &str : numbers) {
			str.trim();

			if (sscanf(str.c_str(), "%i", &parsedNumbers[i]) != 1)
				error("Strangely-formatted animation table line: %s", line.c_str());

			i++;
		}

		AnimFrameRange frameRange;
		frameRange.animationNum = animNumber;
		frameRange.firstFrame = static_cast<uint>(parsedNumbers[1]);
		frameRange.lastFrame = static_cast<uint>(parsedNumbers[2]);

		// Animation ID 9099 is duplicated but it doesn't really matter since the duplicate is identical
		if (animIDToFrameRangeMap.find(parsedNumbers[0]) == animIDToFrameRangeMap.end())
			animIDToFrameRangeMap[parsedNumbers[0]] = frameRange;
		else
			warning("Animation ID %i was duplicated", parsedNumbers[0]);
	}
}

void Runtime::changeMusicTrack(int track) {
	if (track == _musicTrack && (_musicWavePlayer.get() != nullptr || _musicMidiPlayer.get() != nullptr))
		return;

	_musicWavePlayer.reset();
	if (_musicMidiPlayer)
	{
		Common::StackLock lock(_midiPlayerMutex);
		_musicMidiPlayer.reset();
	}
	_musicTrack = track;

	if (!_musicActive)
		return;

	if (_musicMute && !_musicMuteDisabled)
		return;

	Common::String musicPathStr;

	if (_gameID == GID_AD2044) {
		if (!_midiDrv)
			return;

		musicPathStr = Common::String::format("sfx/music%02i.mid", static_cast<int>(track));
	} else
		musicPathStr = Common::String::format("Sfx/Music-%02i.wav", static_cast<int>(track));

	Common::Path musicFileName(musicPathStr);
	Common::File *musicFile = new Common::File();
	if (musicFile->open(musicFileName)) {
		if (_gameID == GID_AD2044) {
			Common::ScopedPtr<Common::File> fileHolder(musicFile);

			uint musicFileSize = static_cast<uint>(musicFile->size());

			if (musicFileSize > 0) {
				Common::Array<byte> musicData;
				musicData.resize(musicFileSize);

				musicFile->read(&musicData[0], musicFileSize);

				Common::StackLock lock(_midiPlayerMutex);
				_musicMidiPlayer.reset(new MidiPlayer(_midiDrv, Common::move(musicData), _musicVolume));
			}
		} else {
			if (Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(musicFile, DisposeAfterUse::YES)) {
				Common::SharedPtr<Audio::AudioStream> loopingStream(Audio::makeLoopingAudioStream(audioStream, 0));

				_musicWavePlayer.reset(new AudioPlayer(_mixer, loopingStream, Audio::Mixer::kMusicSoundType));
				_musicWavePlayer->play(applyVolumeScale(_musicVolume), 0);
			}
		}
	} else {
		warning("Music file '%s' is missing", musicFileName.toString(Common::Path::kNativeSeparator).c_str());
		delete musicFile;
	}
}

void Runtime::startScoreSection() {
	_musicWavePlayer.reset();
	_scoreSectionEndTime = 0;

	if (!_musicActive)
		return;

	if (_musicMute && !_musicMuteDisabled)
		return;

#ifdef USE_VORBIS
	Common::HashMap<Common::String, ScoreTrackDef>::const_iterator trackIt = _scoreDefs.find(_scoreTrack);
	if (trackIt != _scoreDefs.end()) {
		const ScoreTrackDef::ScoreSectionMap_t &sectionMap = trackIt->_value.sections;

		ScoreTrackDef::ScoreSectionMap_t::const_iterator sectionIt = sectionMap.find(_scoreSection);
		if (sectionIt != sectionMap.end()) {
			const ScoreSectionDef &sectionDef = sectionIt->_value;

			if (sectionDef.musicFileName.empty()) {
				_scoreSectionEndTime = sectionDef.volumeOrDurationInSeconds * 1000u + g_system->getMillis();
			} else {
				Common::Path trackFileName("Sfx/");
				trackFileName.appendInPlace(sectionDef.musicFileName);

				Common::File *trackFile = new Common::File();
				if (trackFile->open(trackFileName)) {
					if (Audio::SeekableAudioStream *audioStream = Audio::makeVorbisStream(trackFile, DisposeAfterUse::YES)) {
						_musicWavePlayer.reset(new AudioPlayer(_mixer, Common::SharedPtr<Audio::AudioStream>(audioStream), Audio::Mixer::kMusicSoundType));
						_musicWavePlayer->play(applyVolumeScale(sectionDef.volumeOrDurationInSeconds), 0);

						_scoreSectionEndTime = static_cast<uint32>(audioStream->getLength().msecs()) + g_system->getMillis();
					} else {
						warning("Couldn't create Vorbis stream for music file '%s'", trackFileName.toString(Common::Path::kNativeSeparator).c_str());
						delete trackFile;
					}
				} else {
					warning("Music file '%s' is missing", trackFileName.toString(Common::Path::kNativeSeparator).c_str());
				}
			}
		}
	}
#endif
}

void Runtime::setMusicMute(bool muted) {
	if (muted == _musicMute)
		return;

	bool prevIsActuallyMuted = (_musicMute && !_musicMuteDisabled);

	_musicMute = muted;

	bool isActuallyMuted = (_musicMute && !_musicMuteDisabled);

	if (prevIsActuallyMuted != isActuallyMuted) {
		if (isActuallyMuted) {
			// Became muted
			_musicWavePlayer.reset();
			if (_musicMidiPlayer)
			{
				Common::StackLock lock(_midiPlayerMutex);
				_musicMidiPlayer.reset();
			}
			_scoreSectionEndTime = 0;
		} else {
			// Became unmuted
			if (_gameID == GID_REAH || _gameID == GID_AD2044)
				changeMusicTrack(_musicTrack);
			else if (_gameID == GID_SCHIZM)
				startScoreSection();
		}
	}
}

void Runtime::changeAnimation(const AnimationDef &animDef, bool consumeFPSOverride) {
	changeAnimation(animDef, animDef.firstFrame, consumeFPSOverride);
}

void Runtime::changeAnimation(const AnimationDef &animDef, uint initialFrame, bool consumeFPSOverride) {
	changeAnimation(animDef, initialFrame, consumeFPSOverride, Fraction(0, 1));
}

void Runtime::changeAnimation(const AnimationDef &animDef, uint initialFrame, bool consumeFPSOverride, const Fraction &defaultFrameRate) {
	debug("changeAnimation: Anim: %i  Range: %u -> %u  Initial %u", animDef.animNum, animDef.firstFrame, animDef.lastFrame, initialFrame);

	_animPlaylist.reset();

	int animFile = animDef.animNum;
	if (animFile < 0)
		animFile = -animFile;

	bool isAD2044 = (_gameID == GID_AD2044);

	if (_loadedAnimation != static_cast<uint>(animFile)) {
		_loadedAnimation = animFile;
		_frameData.clear();
		_frameData2.clear();
		_animDecoder.reset();
		_animDecoderState = kAnimDecoderStateStopped;

		Common::Path aviFileName(Common::String::format(isAD2044 ? "anims/ANIM%04i.AVI" : "Anims/Anim%04i.avi", animFile));
		Common::File *aviFile = new Common::File();

		if (aviFile->open(aviFileName)) {
			_animDecoder.reset(new Video::AVIDecoder());

			if (ConfMan.hasKey("vcruise_fast_video_decoder") && ConfMan.getBool("vcruise_fast_video_decoder"))
				_animDecoder->setVideoCodecAccuracy(Image::CodecAccuracy::Fast);

			if (!_animDecoder->loadStream(aviFile)) {
				warning("Animation file %i could not be loaded", animFile);
				return;
			}
		} else {
			error("Animation file %i is missing", animFile);
			delete aviFile;
		}

		applyAnimationVolume();

		_sfxData.reset();

		if (!isAD2044) {
			Common::Path sfxFileName(Common::String::format("Sfx/Anim%04i.sfx", animFile));
			Common::File sfxFile;

			if (sfxFile.open(sfxFileName))
				_sfxData.load(sfxFile, _mixer);
			sfxFile.close();
		}

		Common::Path dtaFileName(Common::String::format(isAD2044 ? "anims/ANIM0001.DTA" : "Anims/Anim%04i.dta", animFile));
		Common::File dtaFile;

		if (dtaFile.open(dtaFileName))
			loadFrameData(&dtaFile);
		dtaFile.close();

		if (!isAD2044) {
			Common::Path twoDtFileName(Common::String::format("Dta/Anim%04i.2dt", animFile));
			Common::File twoDtFile;

			if (twoDtFile.open(twoDtFileName))
				loadFrameData2(&twoDtFile);
			twoDtFile.close();
		}

		if (isAD2044) {
			Common::Path tabFileName(Common::String::format("anims/ANIM%04i.TAB", animFile));
			Common::File tabFile;

			if (tabFile.open(tabFileName))
				loadTabData(_currentRoomAnimIDToFrameRange, animFile, &tabFile);
		}

		_loadedAnimationHasSound = (_animDecoder->getAudioTrackCount() > 0);

		if (_isSubtitleSourceAnimation)
			stopSubtitles();
	}

	if (_animDecoderState == kAnimDecoderStatePlaying) {
		_animDecoder->pauseVideo(true);
		_animDecoderState = kAnimDecoderStatePaused;
	}

	assert(initialFrame >= animDef.firstFrame && initialFrame <= animDef.lastFrame);

	_animDecoder->seekToFrame(initialFrame);
	_animPendingDecodeFrame = initialFrame;
	_animFirstFrame = animDef.firstFrame;
	_animLastFrame = animDef.lastFrame;
	_animConstraintRect = animDef.constraintRect;
	_animFrameRateLock = Fraction();
	_animTerminateAtStartOfFrame = true;

	SfxData::PlaylistMap_t::const_iterator playlistIt = _sfxData.playlists.find(animDef.animName);

	if (playlistIt != _sfxData.playlists.end())
		_animPlaylist = playlistIt->_value;

	if (consumeFPSOverride && _scriptEnv.fpsOverride) {
		_animFrameRateLock = Fraction(_scriptEnv.fpsOverride, 1);
		_scriptEnv.fpsOverride = 0;
	} else {
		if (_animDecoder && !_loadedAnimationHasSound) {
			if (_fastAnimationMode)
				_animFrameRateLock = Fraction(25, 1);
			else
				_animFrameRateLock = defaultFrameRate;
		}
	}

	_animFramesDecoded = 0;
	_animStartTime = 0;

	debug(1, "Animation last frame set to %u", animDef.lastFrame);
}

void Runtime::applyAnimationVolume() {
	if (_animDecoder) {
		_animDecoder->setVolume(applyVolumeScale(_animVolume));
	}
}

void Runtime::setSound3DParameters(SoundInstance &snd, int32 x, int32 y, const SoundParams3D &soundParams3D) {
	snd.x = x;
	snd.y = y;
	snd.params3D = soundParams3D;
}

void Runtime::triggerSound(SoundLoopBehavior soundLoopBehavior, SoundInstance &snd, int32 volume, int32 balance, bool is3D, bool isSpeech) {
	snd.volume = volume;
	snd.balance = balance;
	snd.is3D = is3D;
	snd.isSpeech = isSpeech;

	computeEffectiveVolumeAndBalance(snd);

	if (volume == getSilentSoundVolume()) {
		if (snd.cache) {
			if (snd.cache->player)
				snd.cache->player.reset();

			snd.cache.reset();
		}

		snd.isLooping = (soundLoopBehavior == kSoundLoopBehaviorYes);
		snd.restartWhenAudible = true;
		snd.tryToLoopWhenRestarted = (soundLoopBehavior == kSoundLoopBehaviorAuto);
		snd.endTime = 0;
		snd.duration = 0;
		return;
	}

	snd.restartWhenAudible = false;

	SoundCache *cache = loadCache(snd);

	if (!cache)
		return;

	switch (soundLoopBehavior) {
	case kSoundLoopBehaviorYes:
		snd.isLooping = true;
		break;
	case kSoundLoopBehaviorNo:
		snd.isLooping = false;
		break;
	case kSoundLoopBehaviorAuto:
		snd.isLooping = (cache->loopInfo.get() != nullptr);
		break;
	default:
		error("Invalid sound loop behavior");
	};

	snd.duration = cache->stream->getLength().msecs();

	// Reset if transitioning from loop to non-loop
	if (cache->isLoopActive && !snd.isLooping) {
		cache->player.reset();
		cache->loopingStream.reset();
		cache->stream->rewind();
		cache->isLoopActive = false;
	}

	// Construct looping stream if needed and none exists
	if (snd.isLooping && !cache->isLoopActive) {
		cache->player.reset();
		cache->loopingStream.reset();
		cache->loopingStream.reset(new SampleLoopAudioStream(cache->stream.get(), cache->loopInfo.get()));
		cache->isLoopActive = true;
	}

	const Audio::Mixer::SoundType soundType = (isSpeech ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType);

	if (cache->player) {
		// If there is already a player and this is non-looping, start over
		if (!snd.isLooping) {
			cache->player->stop();
			cache->stream->rewind();
			cache->player->play(snd.effectiveVolume, snd.effectiveBalance);
		} else {
			// Adjust volume and balance at least
			cache->player->setVolumeAndBalance(snd.effectiveVolume, snd.effectiveBalance);
		}
	} else {
		if (!snd.isLooping)
			cache->stream->rewind();

		cache->player.reset(new AudioPlayer(_mixer, snd.isLooping ? cache->loopingStream.staticCast<Audio::AudioStream>() : cache->stream.staticCast<Audio::AudioStream>(), soundType));
		cache->player->play(snd.effectiveVolume, snd.effectiveBalance);
	}

	snd.startTime = g_system->getMillis();
	if (snd.isLooping)
		snd.endTime = 0;
	else
		snd.endTime = snd.startTime + snd.duration + 1000u;
}

void Runtime::triggerSoundRamp(SoundInstance &snd, uint durationMSec, int32 newVolume, bool terminateOnCompletion) {
	snd.rampStartVolume = snd.volume;
	snd.rampEndVolume = newVolume;
	snd.rampTerminateOnCompletion = terminateOnCompletion;
	snd.rampStartTime = g_system->getMillis();
	snd.rampRatePerMSec = 65536;

	if (snd.isLooping && newVolume == getSilentSoundVolume())
		snd.rampTerminateOnCompletion = true;

	if (durationMSec)
		snd.rampRatePerMSec = 65536 / durationMSec;
}

void Runtime::triggerWaveSubtitles(const SoundInstance &snd, const Common::String &id) {
	if (!ConfMan.getBool("subtitles"))
		return;

	char appendedCode[4] = {'_', '0', '0', '\0'};

	char digit1 = '0';
	char digit2 = '0';

	stopSubtitles();

	uint32 currentTime = g_system->getMillis(true);

	uint32 soundEndTime = currentTime + snd.duration;

	for (;;) {
		if (digit2 == '9') {
			digit2 = '0';

			if (digit1 == '9')
				return;	// This should never happen

			digit1++;
		} else
			digit2++;

		appendedCode[1] = digit1;
		appendedCode[2] = digit2;

		Common::String subtitleID = id + appendedCode;

		WaveSubtitleMap_t::const_iterator subIt = _waveSubtitles.find(subtitleID);

		if (subIt != _waveSubtitles.end()) {
			const SubtitleDef &subDef = subIt->_value;

			SubtitleQueueItem queueItem;
			queueItem.startTime = currentTime;
			queueItem.endTime = soundEndTime + 1000u;

			if (_subtitleQueue.size() > 0)
				queueItem.startTime = _subtitleQueue.back().endTime;

			for (int ch = 0; ch < 3; ch++)
				queueItem.color[ch] = subDef.color[ch];

			if (subDef.durationInDeciseconds != 1)
				queueItem.endTime = queueItem.startTime + subDef.durationInDeciseconds * 100u;

			queueItem.str = subDef.str.decode(Common::kUtf8);

			_subtitleQueue.push_back(queueItem);
		}
	}
}

void Runtime::stopSubtitles() {
	_subtitleQueue.clear();
	_isDisplayingSubtitles = false;
	_isSubtitleSourceAnimation = false;

	if (_gameID == GID_AD2044)
		redrawSubtitleSection();
	else
		redrawTray();
}

void Runtime::stopSound(SoundInstance &sound) {
	if (!sound.cache)
		return;

	sound.cache->player.reset();
	sound.cache.reset();
	sound.endTime = 0;
	sound.restartWhenAudible = false;
	sound.tryToLoopWhenRestarted = false;
}

void Runtime::convertLoopingSoundToNonLooping(SoundInstance &sound) {
	if (!sound.cache)
		return;

	if (sound.cache->loopingStream) {
		sound.cache->loopingStream->stopLooping();
		sound.cache->isLoopActive = false;
		sound.isLooping = false;

		uint32 currentTime = g_system->getMillis();

		uint32 alreadyPlayedTime = ((currentTime - sound.startTime) % sound.duration);
		sound.endTime = currentTime + sound.duration - alreadyPlayedTime;
	}
}

void Runtime::updateSounds(uint32 timestamp) {
	for (uint sndIndex = 0; sndIndex < _activeSounds.size(); sndIndex++) {
		SoundInstance &snd = *_activeSounds[sndIndex];

		if (snd.rampRatePerMSec) {
			int32 ramp = snd.rampRatePerMSec * static_cast<int32>(timestamp - snd.rampStartTime);
			int32 newVolume = snd.volume;
			if (ramp >= 65536) {
				snd.rampRatePerMSec = 0;
				newVolume = snd.rampEndVolume;
				if (snd.rampTerminateOnCompletion)
					stopSound(snd);
			} else {
				int32 rampedVolume = (snd.rampStartVolume * (65536 - ramp)) + (snd.rampEndVolume * ramp);
				newVolume = rampedVolume >> 16;
			}

			if (snd.volume != newVolume) {
				snd.volume = newVolume;

				if (snd.cache) {
					SoundCache *cache = snd.cache.get();
					if (cache->player) {
						computeEffectiveVolumeAndBalance(snd);
						cache->player->setVolumeAndBalance(snd.effectiveVolume, snd.effectiveBalance);
					}
				}
			}
		}

		// Cache-evict stopped sounds
		if (snd.endTime && snd.endTime <= timestamp) {
			snd.cache.reset();
			snd.endTime = 0;
		}

		if (snd.isLooping) {
			if (snd.volume <= getSilentSoundVolume()) {
				if (!snd.restartWhenAudible) {
					if (snd.cache) {
						snd.cache->player.reset();
						snd.cache.reset();
					}
					snd.restartWhenAudible = true;
				}
			} else {
				if (snd.restartWhenAudible) {
					triggerSound(kSoundLoopBehaviorYes, snd, snd.volume, snd.balance, snd.is3D, snd.isSpeech);
					assert(snd.restartWhenAudible == false);
				}
			}
		} else {
			if (snd.volume > getSilentSoundVolume()) {
				if (snd.restartWhenAudible) {
					SoundLoopBehavior loopBehavior = kSoundLoopBehaviorNo;
					if (snd.tryToLoopWhenRestarted) {
						loopBehavior = kSoundLoopBehaviorAuto;
						snd.tryToLoopWhenRestarted = false;
					}

					triggerSound(loopBehavior, snd, snd.volume, snd.balance, snd.is3D, snd.isSpeech);
					assert(snd.restartWhenAudible == false);
				}
			}
		}
	}

	if (_musicVolumeRampRatePerMSec != 0) {
		bool negative = (_musicVolumeRampRatePerMSec < 0);

		uint32 rampMax = 0;
		uint32 absRampRate = 0;
		if (negative) {
			rampMax = _musicVolumeRampStartVolume - _musicVolumeRampEnd;
			absRampRate = -_musicVolumeRampRatePerMSec;
		} else {
			rampMax = _musicVolumeRampEnd - _musicVolumeRampStartVolume;
			absRampRate = _musicVolumeRampRatePerMSec;
		}

		uint32 rampTime = timestamp - _musicVolumeRampStartTime;

		uint32 ramp = (rampTime * absRampRate) >> 16;
		if (ramp > rampMax)
			ramp = rampMax;

		int32 newVolume = _musicVolumeRampStartVolume;
		if (negative)
			newVolume -= static_cast<int32>(ramp);
		else
			newVolume += static_cast<int32>(ramp);

		if (newVolume != _musicVolume) {
			if (_musicWavePlayer)
				_musicWavePlayer->setVolume(applyVolumeScale(newVolume));

			if (_musicMidiPlayer) {
				Common::StackLock lock(_midiPlayerMutex);
				_musicMidiPlayer->setVolume(newVolume);
			}
			_musicVolume = newVolume;
		}

		if (newVolume == _musicVolumeRampEnd)
			_musicVolumeRampRatePerMSec = 0;
	}

	if (_scoreSectionEndTime != 0 && _scoreSectionEndTime < timestamp) {

#ifdef USE_VORBIS
		Common::HashMap<Common::String, ScoreTrackDef>::const_iterator trackIt = _scoreDefs.find(_scoreTrack);
		if (trackIt != _scoreDefs.end()) {
			const ScoreTrackDef::ScoreSectionMap_t &sectionMap = trackIt->_value.sections;

			ScoreTrackDef::ScoreSectionMap_t::const_iterator sectionIt = sectionMap.find(_scoreSection);
			if (sectionIt != sectionMap.end())
				_scoreSection = sectionIt->_value.nextSection;

			startScoreSection();
		}
#endif
	}
}

void Runtime::updateSubtitles() {
	uint32 timestamp = g_system->getMillis(true);

	while (_subtitleQueue.size() > 0) {
		const SubtitleQueueItem &queueItem = _subtitleQueue[0];

		if (_isDisplayingSubtitles) {
			assert(_subtitleQueue.size() > 0);

			if (queueItem.endTime <= timestamp) {
				_subtitleQueue.remove_at(0);
				_isDisplayingSubtitles = false;

				if (_subtitleQueue.size() == 0) {
					// Queue was exhausted

					// Is this really what we want to be doing?
					if (_escOn) {
						if (_gameID == GID_AD2044)
							clearSubtitleSection();
						else
							clearTray();
					} else {
						if (_gameID == GID_AD2044)
							redrawSubtitleSection();
						else
							redrawTray();
					}
				}
			} else
				break;
		} else {
			Common::Array<Common::U32String> lines;

			uint lineStart = 0;
			for (;;) {
				uint lineEnd = queueItem.str.find(static_cast<Common::u32char_type_t>('\\'), lineStart);
				if (lineEnd == Common::U32String::npos) {
					lines.push_back(queueItem.str.substr(lineStart));
					break;
				}

				lines.push_back(queueItem.str.substr(lineStart, lineEnd - lineStart));
				lineStart = lineEnd + 1;
			}

			if (_gameID == GID_AD2044)
				clearSubtitleSection();
			else
				clearTray();

			drawSubtitleText(lines, queueItem.color);

			_isDisplayingSubtitles = true;
		}
	}
}

void Runtime::update3DSounds() {
	for (const Common::SharedPtr<SoundInstance> &sndPtr : _activeSounds) {
		SoundInstance &snd = *sndPtr;

		if (!snd.is3D)
			continue;

		bool changed = computeEffectiveVolumeAndBalance(snd);

		if (changed) {
			if (snd.cache) {
				SoundCache *cache = snd.cache.get();
				if (cache) {
					VCruise::AudioPlayer *player = cache->player.get();
					if (player)
						player->setVolumeAndBalance(snd.effectiveVolume, snd.effectiveBalance);
				}
			}
		}
	}
}

bool Runtime::computeEffectiveVolumeAndBalance(SoundInstance &snd) {
	uint effectiveVolume = applyVolumeScale(snd.volume);
	int32 effectiveBalance = applyBalanceScale(snd.balance);

	double radians = Math::deg2rad<double>(_listenerAngle);
	int32 cosAngle = static_cast<int32>(cos(radians) * (1 << 15));
	int32 sinAngle = static_cast<int32>(sin(radians) * (1 << 15));

	if (snd.is3D) {
		int32 dx = snd.x - _listenerX;
		int32 dy = snd.y - _listenerY;

		double dxf = dx;
		double dyf = dy;

		uint distance = static_cast<uint>(sqrt(dxf * dxf + dyf * dyf));

		if (distance >= snd.params3D.maxRange)
			effectiveVolume = 0;
		else if (distance > snd.params3D.minRange) {
			uint rangeDelta = snd.params3D.maxRange - snd.params3D.minRange;

			effectiveVolume = (snd.params3D.maxRange - distance) * effectiveVolume / rangeDelta;
		}

		int32 dxNormalized = 0;
		int32 dyNormalized = 0;
		if (distance > 0) {
			dxNormalized = dx * (1 << 10) / static_cast<int32>(distance);
			dyNormalized = dy * (1 << 10) / static_cast<int32>(distance);
		}

		int32 balance16 = (sinAngle * dxNormalized - cosAngle * dyNormalized) >> 9;

		// Reduce to 3/5 intensity.  This means that at full balance, the opposing volume will be 1/4 the facing volume.
		balance16 = (balance16 * 9830 + (1 << 13)) >> 14;

		if (balance16 > 65536)
			balance16 = 65536;
		else if (balance16 < -65536)
			balance16 = -65536;

		uint rightVolume = ((65536u + balance16) * effectiveVolume) >> 16;
		uint leftVolume = ((65536u - balance16) * effectiveVolume) >> 16;

		if (leftVolume == 0 && rightVolume == 0) {
			// This should never happen
			effectiveVolume = 0;
			effectiveBalance = 0;
		} else {
			if (leftVolume <= rightVolume) {
				effectiveVolume = rightVolume;
				effectiveBalance = 127 - (leftVolume * 127 / rightVolume);
			} else {
				effectiveVolume = leftVolume;
				effectiveBalance = (rightVolume * 127 / leftVolume) - 127;
			}
		}
	}

	bool changed = (effectiveVolume != snd.effectiveVolume || effectiveBalance != snd.effectiveBalance);

	snd.effectiveVolume = effectiveVolume;
	snd.effectiveBalance = effectiveBalance;

	return changed;
}

void Runtime::triggerAmbientSounds() {
	if (_randomAmbientSounds.size() == 0)
		return;

	uint32 timestamp = g_system->getMillis();

	if (timestamp < _ambientSoundFinishTime)
		return;

	// This has been mostly confirmed to match Reah's behavior, including not decrementing sound scene change
	// counters if an existing sound was playing when this is checked.
	for (uint i = 0; i < _randomAmbientSounds.size(); i++) {
		if (_randomAmbientSounds[i].sceneChangesRemaining == 0) {
			// Found a sound to play
			RandomAmbientSound sound = Common::move(_randomAmbientSounds[i]);
			_randomAmbientSounds.remove_at(i);

			if (sound.frequency > 0)
				sound.sceneChangesRemaining = sound.frequency - 1;

			StackInt_t soundID = 0;
			SoundInstance *cachedSound = nullptr;
			resolveSoundByName(sound.name, true, soundID, cachedSound);

			if (cachedSound) {
				triggerSound(kSoundLoopBehaviorNo, *cachedSound, sound.volume, sound.balance, false, false);
				if (cachedSound->cache)
					_ambientSoundFinishTime = timestamp + static_cast<uint>(cachedSound->cache->stream->getLength().msecs());
			}

			// Requeue at the end
			_randomAmbientSounds.push_back(Common::move(sound));
			return;
		}
	}

	// No ambient sound was ready
	for (RandomAmbientSound &snd : _randomAmbientSounds)
		snd.sceneChangesRemaining--;
}

uint Runtime::decibelsToLinear(int db, uint baseVolume, uint maxVol) const {
	double linearized = floor(pow(1.1220184543019634355910389464779, db) * static_cast<double>(baseVolume) + 0.5);

	if (linearized > static_cast<double>(maxVol))
		return maxVol;

	return static_cast<uint>(linearized);
}

int32 Runtime::getSilentSoundVolume() const {
	if (_gameID == GID_SCHIZM)
		return -50;
	else
		return 0;
}

int32 Runtime::getDefaultSoundVolume() const {
	if (_gameID == GID_SCHIZM)
		return 0;
	else
		return 100;
}

uint Runtime::applyVolumeScale(int32 volume) const {
	if (_gameID == GID_SCHIZM) {
		if (volume >= 0)
			return Audio::Mixer::kMaxChannelVolume;
		else if (volume < -49)
			return 0;

		return _dbToVolume[volume + 49];
	} else {
		if (volume > 100)
			return Audio::Mixer::kMaxChannelVolume;
		else if (volume < 0)
			return 0;

		return volume * Audio::Mixer::kMaxChannelVolume / 200;
	}
}

int Runtime::applyBalanceScale(int32 balance) const {
	if (balance < -100)
		balance = -100;
	else if (balance > 100)
		balance = 100;

	// Avoid undefined divide rounding behavior, round toward zero
	if (balance < 0)
		return -((-balance) * 127 / 100);
	else
		return balance * 127 / 100;
}

AnimationDef Runtime::stackArgsToAnimDef(const StackInt_t *args) const {
	AnimationDef def;
	def.animNum = args[0];
	def.firstFrame = args[1];
	def.lastFrame = args[2];

	def.constraintRect.left = args[3];
	def.constraintRect.top = args[4];
	def.constraintRect.right = args[5];
	def.constraintRect.bottom = args[6];

	def.animName = _animDefNames[args[7]];

	return def;
}

void Runtime::consumeAnimChangeAndAdjustAnim(AnimationDef &animDef) {
	if (_scriptEnv.animChangeSet) {
		uint origFirstFrame = animDef.firstFrame;
		uint origLastFrame = animDef.lastFrame;

		uint newFirstFrame = origFirstFrame + _scriptEnv.animChangeFrameOffset;
		uint newLastFrame = newFirstFrame + _scriptEnv.animChangeNumFrames;

		if (newLastFrame > origLastFrame || newFirstFrame > origLastFrame)
			warning("animChange ops overran the original animation bounds");

		animDef.firstFrame = newFirstFrame;
		animDef.lastFrame = newLastFrame;

		_scriptEnv.animChangeSet = false;
	}
}

void Runtime::pushAnimDef(const AnimationDef &animDef) {
	_scriptStack.push_back(StackValue(animDef.animNum));
	_scriptStack.push_back(StackValue(animDef.firstFrame));
	_scriptStack.push_back(StackValue(animDef.lastFrame));

	_scriptStack.push_back(StackValue(animDef.constraintRect.left));
	_scriptStack.push_back(StackValue(animDef.constraintRect.top));
	_scriptStack.push_back(StackValue(animDef.constraintRect.right));
	_scriptStack.push_back(StackValue(animDef.constraintRect.bottom));

	uint animNameIndex = 0;
	Common::HashMap<Common::String, uint>::const_iterator nameIt = _animDefNameToIndex.find(animDef.animName);
	if (nameIt == _animDefNameToIndex.end()) {
		animNameIndex = _animDefNames.size();
		_animDefNameToIndex[animDef.animName] = animNameIndex;
		_animDefNames.push_back(animDef.animName);
	} else
		animNameIndex = nameIt->_value;

	_scriptStack.push_back(StackValue(animNameIndex));
}

void Runtime::activateScript(const Common::SharedPtr<Script> &script, bool isEntryScript, const ScriptEnvironmentVars &envVars) {
	if (script->instrs.size() == 0)
		return;

	assert(_gameState != kGameStateScript);

	_scriptEnv = envVars;
	_scriptEnv.isEntryScript = isEntryScript;

	CallStackFrame frame;
	frame._script = script;
	frame._nextInstruction = 0;

	_scriptCallStack.resize(1);
	_scriptCallStack[0] = frame;

	_gameState = kGameStateScript;
}

Common::SharedPtr<ScriptSet> Runtime::compileSchizmLogicSet(const uint *roomNumbers, uint numRooms) const {
	Common::SharedPtr<IScriptCompilerGlobalState> gs = createScriptCompilerGlobalState();

	Common::SharedPtr<ScriptSet> scriptSet(new ScriptSet());

	for (uint i = 0; i < numRooms; i++) {
		uint roomNumber = roomNumbers[i];
		uint roomFile = roomNumber;

		if (roomNumber < _roomDuplicationOffsets.size())
			roomFile -= _roomDuplicationOffsets[roomNumber];

		Common::Path logicFileName(Common::String::format("Log/Room%02u.log", roomFile));

		Common::File logicFile;
		if (logicFile.open(logicFileName)) {
			debug(1, "Compiling script %s...", logicFileName.toString(Common::Path::kNativeSeparator).c_str());
			compileSchizmLogicFile(*scriptSet, roomNumber, roomFile, logicFile, static_cast<uint>(logicFile.size()), logicFileName, gs.get());
			logicFile.close();
		}
	}

	gs->dumpFunctionNames(scriptSet->functionNames);

	uint numFunctions = gs->getNumFunctions();

	scriptSet->functions.resize(numFunctions);

	for (uint i = 0; i < numFunctions; i++) {
		Common::SharedPtr<Script> function = gs->getFunction(i);
		scriptSet->functions[i] = function;

		if (!function)
			warning("Function '%s' was referenced but not defined", scriptSet->functionNames[i].c_str());
	}

	optimizeScriptSet(*scriptSet);

	return scriptSet;
}

bool Runtime::parseIndexDef(IndexParseType parseType, uint roomNumber, const Common::String &key, const Common::String &value) {
	switch (parseType) {
	case kIndexParseTypeNameRoom: {
		uint nameRoomNumber = 0;

		if (!sscanf(value.c_str(), "%u", &nameRoomNumber))
			error("Malformed NameRoom def '%s'", value.c_str());

		allocateRoomsUpTo(nameRoomNumber);
		_roomDefs[nameRoomNumber]->name = key;
	} break;
	case kIndexParseTypeRoom: {
		int animNum = 0;
		uint firstFrame = 0;
		uint lastFrame = 0;
		if (sscanf(value.c_str(), "%i, %u, %u", &animNum, &firstFrame, &lastFrame) != 3)
			error("Malformed room animation def '%s'", value.c_str());

		AnimationDef &animDef = _roomDefs[roomNumber]->animations[key];
		animDef.animNum = animNum;
		animDef.firstFrame = firstFrame;
		animDef.lastFrame = lastFrame;
		animDef.animName = key;
	} break;
	case kIndexParseTypeRRoom: {
		Common::String name;


		int left = 0;
		int top = 0;
		int width = 0;
		int height = 0;

		int numValuesRead = sscanf(value.c_str(), "%i, %i, %i, %i", &left, &top, &width, &height);

		// Work around bad def in Schizm at line 5899
		if (numValuesRead != 4)
			numValuesRead = sscanf(value.c_str(), "%i ,%i, %i, %i", &left, &top, &width, &height);

		if (numValuesRead == 4) {
			AnimationDef &animDef = _roomDefs[roomNumber]->animations[key];

			animDef.constraintRect = Common::Rect(left, top, left + width, top + height);
		} else {
			// Line 4210 in Reah contains an animation def instead of a rect def, so we need to tolerate invalid values here
			warning("Invalid rect def in logic index '%s'", value.c_str());
		}
	} break;
	case kIndexParseTypeYRoom: {
		uint varSlot = 0;

		if (!sscanf(value.c_str(), "%u", &varSlot))
			error("Malformed var def '%s'", value.c_str());

		_roomDefs[roomNumber]->vars[key] = varSlot;
	} break;
	case kIndexParseTypeVRoom: {
		Common::String name;

		int val = 0;

		if (!sscanf(value.c_str(), "%i", &val))
			error("Malformed value def '%s'", value.c_str());

		_roomDefs[roomNumber]->values[key] = val;
	} break;
	case kIndexParseTypeTRoom: {
		_roomDefs[roomNumber]->texts[key] = value;
	} break;
	case kIndexParseTypeCRoom: {
		// This is only used for one entry ("PrzedDrzwiamiDoZsypu" = "In front of the door to the chute") in Reah
		// and doesn't seem to be referenced in any scripts or anything else.  Discard it.
	} break;
	case kIndexParseTypeSRoom: {
		Common::String name;

		int soundID = 0;

		if (!sscanf(value.c_str(), "%i", &soundID))
			error("Malformed sound def '%s'", value.c_str());

		_roomDefs[roomNumber]->values[key] = soundID;
	} break;
	default:
		assert(false);
		return false;
	}

	return true;
}

void Runtime::allocateRoomsUpTo(uint roomNumber) {
	while (_roomDefs.size() <= roomNumber) {
		_roomDefs.push_back(Common::SharedPtr<RoomDef>(new RoomDef()));
	}
}

void Runtime::drawDebugOverlay() {
	if (!_debugMode)
		return;

	const Graphics::PixelFormat pixFmt = _gameDebugBackBuffer.surf->format;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

	uint32 whiteColor = pixFmt.ARGBToColor(255, 255, 255, 255);
	uint32 blackColor = pixFmt.ARGBToColor(255, 0, 0, 0);

	const MapScreenDirectionDef *sdDef = _mapLoader->getScreenDirection(_screenNumber, _direction);
	if (sdDef) {
		for (const InteractionDef &idef : sdDef->interactions) {
			Common::Rect rect = idef.rect;

			Common::String label = Common::String::format("0%x %i", static_cast<int>(idef.interactionID), static_cast<int>(idef.objectType));

			Graphics::ManagedSurface *surf = _gameDebugBackBuffer.surf.get();

			if (font) {
				Common::Point pt = Common::Point(rect.left + 2, rect.top + 2);

				font->drawString(surf, label, pt.x + 1, pt.y + 1, rect.width(), blackColor);
				font->drawString(surf, label, pt.x, pt.y, rect.width(), whiteColor);
			}

			surf->frameRect(Common::Rect(rect.left + 1, rect.top + 1, rect.right + 1, rect.bottom + 1), blackColor);
			surf->frameRect(rect, whiteColor);
		}
	}

	commitSectionToScreen(_gameDebugBackBuffer, Common::Rect(0, 0, _gameDebugBackBuffer.rect.width(), _gameDebugBackBuffer.rect.height()));
}

Common::SharedPtr<Script> Runtime::findScriptForInteraction(uint interactionID) const {
	if (_scriptSet) {
		RoomScriptSet *roomScriptSet = getRoomScriptSetForCurrentRoom();

		if (roomScriptSet) {
			ScreenScriptSetMap_t::const_iterator screenScriptIt = roomScriptSet->screenScripts.find(_screenNumber);
			if (screenScriptIt != roomScriptSet->screenScripts.end()) {
				const ScreenScriptSet &screenScriptSet = *screenScriptIt->_value;

				ScriptMap_t::const_iterator interactionScriptIt = screenScriptSet.interactionScripts.find(interactionID);
				if (interactionScriptIt != screenScriptSet.interactionScripts.end())
					return interactionScriptIt->_value;
			}
		}
	}

	return nullptr;
}

void Runtime::detectPanoramaDirections() {
	_panoramaDirectionFlags = 0;

	if (_haveHorizPanAnimations)
		_panoramaDirectionFlags |= kPanoramaHorizFlags;

	if (_havePanDownFromDirection[_direction])
		_panoramaDirectionFlags |= kPanoramaDownFlag;

	if (_havePanUpFromDirection[_direction])
		_panoramaDirectionFlags |= kPanoramaUpFlag;
}

void Runtime::detectPanoramaMouseMovement(uint32 timestamp) {
	if (_panoramaState == kPanoramaStateInactive && _inGameMenuState == kInGameMenuStateInvisible && (_lmbDragging || (_lmbDown && (timestamp - _lmbDownTime) >= 500)) && !_idleLockInteractions)
		panoramaActivate();
}

void Runtime::panoramaActivate() {
	assert(_panoramaState == kPanoramaStateInactive);
	_panoramaState = kPanoramaStatePanningUncertainDirection;
	_panoramaAnchor = _mousePos;

	uint cursorID = 0;
	if (_haveHorizPanAnimations || _havePanUpFromDirection[_direction] || _havePanDownFromDirection[_direction]) {
		uint panCursor = 0;
		if (_panoramaDirectionFlags & kPanoramaHorizFlags)
			panCursor |= kPanCursorDraggableHoriz;
		if (_panoramaDirectionFlags & kPanoramaUpFlag)
			panCursor |= kPanCursorDraggableUp;
		if (_panoramaDirectionFlags & kPanoramaDownFlag)
			panCursor |= kPanCursorDraggableDown;

		cursorID = _panCursors[panCursor];
	}

	debug(1, "Changing cursor to panorama cursor %u", cursorID);
	changeToCursor(_cursors[cursorID]);

	// We don't reset inventory highlights here.  It'd make sense, but doesn't match the original game's behavior.
	// Inventory highlights only reset from panoramas if a rotation occurs, or the mouse button is released.
}

bool Runtime::computeFaceDirectionAnimation(uint desiredDirection, const AnimationDef *&outAnimDef, uint &outInitialFrame, uint &outStopFrame) {
	if (_direction == desiredDirection || !_haveHorizPanAnimations)
		return false;

	uint leftPanDistance = ((_direction + kNumDirections) - desiredDirection) % kNumDirections;
	uint rightPanDistance = ((desiredDirection + kNumDirections) - _direction) % kNumDirections;

	if (rightPanDistance <= leftPanDistance) {
		uint currentSlice = _direction;
		uint desiredSlice = desiredDirection;

		outAnimDef = &_panRightAnimationDef;
		outInitialFrame = currentSlice * (_panRightAnimationDef.lastFrame - _panRightAnimationDef.firstFrame) / kNumDirections + _panRightAnimationDef.firstFrame;
		outStopFrame = desiredSlice * (_panRightAnimationDef.lastFrame - _panRightAnimationDef.firstFrame) / kNumDirections + _panRightAnimationDef.firstFrame;
	} else {
		uint reverseCurrentSlice = (kNumDirections - _direction);
		if (reverseCurrentSlice == kNumDirections)
			reverseCurrentSlice = 0;

		uint reverseDesiredSlice = (kNumDirections - desiredDirection);
		if (reverseDesiredSlice == kNumDirections)
			reverseDesiredSlice = 0;

		outAnimDef = &_panLeftAnimationDef;
		outInitialFrame = reverseCurrentSlice * (_panLeftAnimationDef.lastFrame - _panLeftAnimationDef.firstFrame) / kNumDirections + _panLeftAnimationDef.firstFrame;
		outStopFrame = reverseDesiredSlice * (_panLeftAnimationDef.lastFrame - _panLeftAnimationDef.firstFrame) / kNumDirections + _panLeftAnimationDef.firstFrame;
	}

	return true;
}

void Runtime::inventoryAddItem(uint item) {
	uint firstOpenSlot = kNumInventorySlots;

	for (uint i = 0; i < kNumInventorySlots; i++) {
		if (_inventory[i].itemID == 0 && firstOpenSlot == kNumInventorySlots)
			firstOpenSlot = i;
	}

	if (firstOpenSlot == kNumInventorySlots)
		error("Tried to add an inventory item but ran out of slots");

	Common::String itemFileName;
	Common::String alphaFileName;
	getFileNamesForItemGraphic(item, itemFileName, alphaFileName);

	_inventory[firstOpenSlot].itemID = item;
	_inventory[firstOpenSlot].graphic = loadGraphic(itemFileName, alphaFileName, false);

	drawInventory(firstOpenSlot);
}

void Runtime::inventoryRemoveItem(uint itemID) {
	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];

		if (item.itemID == static_cast<uint>(itemID)) {
			item.highlighted = false;
			item.itemID = 0;
			item.graphic.reset();
			drawInventory(slot);
			break;
		}
	}
}

void Runtime::clearScreen() {
	if (_gameID == GID_AD2044) {
		_fullscreenMenuSection.surf->blitFrom(*_backgroundGraphic);
		commitSectionToScreen(_fullscreenMenuSection, _fullscreenMenuSection.rect);
	} else
		_system->fillScreen(_system->getScreenFormat().RGBToColor(0, 0, 0));
}

void Runtime::redrawTray() {
	if (_subtitleQueue.size() != 0)
		return;

	clearTray();

	drawCompass();

	for (uint slot = 0; slot < kNumInventorySlots; slot++)
		drawInventory(slot);
}

void Runtime::clearTray() {
	Common::Rect trayRect;
	if (_gameID == GID_AD2044) {
		trayRect = _traySection.rect;
		trayRect.translate(-trayRect.left, -trayRect.top);
		_traySection.surf->blitFrom(*_backgroundGraphic, _traySection.rect, trayRect);
	} else {
		uint32 blackColor = _traySection.surf->format.RGBToColor(0, 0, 0);
		trayRect = Common::Rect(0, 0, _traySection.surf->w, _traySection.surf->h);

		_traySection.surf->fillRect(trayRect, blackColor);
	}

	drawSectionToScreen(_traySection, trayRect);
}

void Runtime::redrawSubtitleSection() {
	if (_subtitleQueue.size() != 0)
		return;

	clearSubtitleSection();

	if (!_tooltipText.empty()) {
		Common::CodePage codePage = Common::kWindows1250;

		Common::Array<Common::U32String> lines;

		uint32 lastStringStart = 0;
		for (;;) {
			uint32 backslashPos = _tooltipText.find('\\', lastStringStart);
			if (backslashPos == Common::String::npos)
				break;

			Common::String slice = _tooltipText.substr(lastStringStart, backslashPos - lastStringStart);
			lines.push_back(slice.decode(codePage));
			lastStringStart = backslashPos + 1;
		}

		Common::String lastSlice = _tooltipText.substr(lastStringStart, _tooltipText.size() - lastStringStart);
		lines.push_back(lastSlice.decode(codePage));

		uint8 color[3] = {255, 255, 0};
		drawSubtitleText(lines, color);
	}
}

void Runtime::clearSubtitleSection() {
	Common::Rect stRect;
	if (_gameID == GID_AD2044) {
		stRect = _subtitleSection.rect;
		stRect.translate(-stRect.left, -stRect.top);
		_subtitleSection.surf->blitFrom(*_backgroundGraphic, _subtitleSection.rect, stRect);
	}

	this->commitSectionToScreen(_subtitleSection, stRect);
}

void Runtime::drawSubtitleText(const Common::Array<Common::U32String> &lines, const uint8 (&color)[3]) {
	RenderSection &stSection = (_gameID == GID_AD2044) ? _subtitleSection : _traySection;
	Graphics::ManagedSurface *surf = stSection.surf.get();

	if (_subtitleFont) {
		int lineHeight = (_gameID == GID_AD2044) ? 24 : _subtitleFont->getFontHeight();

		int xOffset = 0;
		int topY = 0;
		if (_gameID == GID_AD2044) {
			topY = 13;
			xOffset = 5;
		} else
			topY = (surf->h - lineHeight * static_cast<int>(lines.size())) / 2;

		uint32 textColor = surf->format.RGBToColor(color[0], color[1], color[2]);

		for (uint lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
			const Common::U32String &line = lines[lineIndex];
			int lineWidth = _subtitleFont->getStringWidth(line);

			int xPos = (surf->w - lineWidth) / 2 + xOffset;
			int yPos = topY + static_cast<int>(lineIndex) * lineHeight;

			_subtitleFont->drawString(surf, line, xPos + 2, yPos + 2, lineWidth, 0);
			_subtitleFont->drawString(surf, line, xPos, yPos, lineWidth, textColor);
		}
	}

	commitSectionToScreen(stSection, Common::Rect(0, 0, stSection.rect.width(), stSection.rect.height()));
}

void Runtime::drawInventory(uint slot) {
	if (!isTrayVisible())
		return;

	if (_gameID == GID_AD2044) {
		drawInventoryItemGraphic(slot);
		return;
	}

	Common::Rect trayRect = _traySection.rect;
	trayRect.translate(-trayRect.left, -trayRect.top);

	const uint slotWidth = 79;
	const uint firstItemX = 82;

	const uint slotStartX = firstItemX + slot * slotWidth;
	Common::Rect sliceRect = Common::Rect(slotStartX, 0, slotStartX + slotWidth, trayRect.height());

	const bool highlighted = _inventory[slot].highlighted;

	if (highlighted)
		_traySection.surf->blitFrom(*_trayHighlightGraphic, sliceRect, sliceRect);
	else
		_traySection.surf->fillRect(sliceRect, 0);

	const Graphics::Surface *surf = _inventory[slot].graphic.get();

	// TODO: Highlighted items
	if (surf) {
		const uint itemWidth = surf->w;
		const uint itemHeight = surf->h;

		const uint itemTopY = (static_cast<uint>(trayRect.height()) - itemHeight) / 2u;
		const uint itemLeftY = slotStartX + (slotWidth - itemWidth) / 2u;

		if (highlighted) {
			uint32 blackColor = surf->format.ARGBToColor(255, 0, 0, 0);
			_traySection.surf->transBlitFrom(*surf, Common::Point(itemLeftY, itemTopY), blackColor);
		} else
			_traySection.surf->blitFrom(*surf, Common::Point(itemLeftY, itemTopY));
	}

	commitSectionToScreen(_traySection, sliceRect);
}

void Runtime::drawCompass() {
	if (!isTrayVisible())
		return;

	if (_gameID == GID_AD2044)
		return;

	bool haveHorizontalRotate = false;
	bool haveUp = false;
	bool haveDown = false;
	bool haveLocation = false;

	switch (_gameState) {
	case kGameStateIdle:
	case kGameStateGyroIdle:
	case kGameStateGyroAnimation:
		haveHorizontalRotate = _haveHorizPanAnimations;
		haveUp = _havePanUpFromDirection[_direction];
		haveDown = _havePanDownFromDirection[_direction];
		break;
	case kGameStatePanLeft:
	case kGameStatePanRight:
		haveHorizontalRotate = _haveHorizPanAnimations;
		break;
	default:
		break;
	}

	// Try to keep this logic in sync with canSave(true)
	haveLocation = (haveHorizontalRotate || _forceAllowSaves);
	//haveLocation = haveLocation || haveUp || haveDown;

	const Common::Rect blackoutRects[4] = {
		Common::Rect(0, 40, 36, 62),  // Left
		Common::Rect(52, 40, 88, 62), // Right
		Common::Rect(35, 12, 53, 38), // Up
		Common::Rect(35, 56, 54, 78), // Down
	};

	const bool drawSections[4] = {haveHorizontalRotate, haveHorizontalRotate, haveUp, haveDown};

	Common::Rect compassRect = Common::Rect(0, 0, _trayCompassGraphic->w, _trayCompassGraphic->h);

	int16 vertOffset = (_traySection.rect.height() - compassRect.height()) / 2;
	const int horizOffset = 0;

	compassRect.translate(horizOffset, vertOffset);

	_traySection.surf->blitFrom(*_trayCompassGraphic, Common::Point(compassRect.left, compassRect.top));

	const uint32 blackColor = _traySection.surf->format.ARGBToColor(255, 0, 0, 0);

	for (uint i = 0; i < 4; i++) {
		if (!drawSections[i]) {
			Common::Rect blackoutRect = blackoutRects[i];
			blackoutRect.translate(horizOffset, vertOffset);

			_traySection.surf->fillRect(blackoutRect, blackColor);
		}
	}

	Common::Rect lowerRightRect = Common::Rect(_traySection.rect.right - 88, 0, _traySection.rect.right, 88);

	if (_gameID == GID_REAH) {

		if (haveLocation)
			_traySection.surf->blitFrom(*_trayCornerGraphic, Common::Point(lowerRightRect.left, lowerRightRect.top));
		else
			_traySection.surf->blitFrom(*_trayBackgroundGraphic, lowerRightRect, Common::Point(lowerRightRect.left, lowerRightRect.top));
	} else if (_gameID == GID_SCHIZM) {
		Common::Rect graphicRect = Common::Rect(0u + _hero * 176u, 0, 0u + _hero * 176u + 88, 88);

		if (!haveLocation)
			graphicRect.translate(88, 0);

		_traySection.surf->blitFrom(*_trayCornerGraphic, graphicRect, Common::Point(lowerRightRect.left, lowerRightRect.top));
	}

	commitSectionToScreen(_traySection, compassRect);
	commitSectionToScreen(_traySection, lowerRightRect);
}

bool Runtime::isTrayVisible() const {
	if (_subtitleQueue.size() == 0 && _isInGame && (_gameState != kGameStateMenu)) {
		// In Reah, animations with sounds are cutscenes that hide the tray.  In Schizm, the tray continues displaying.
		//
		// This is important in some situations, e.g. after "reuniting" with Hannah in the lower temple, if you go left,
		// a ghost will give you a key.  Since that animation has sound, you'll return to idle in that animation,
		// which will keep the tray hidden because it has sound.
		//
		// Ignore this condition if we're at the last frame (fixes inventory not drawing after trading weights in Reah)
		if (_gameID == GID_REAH && _loadedAnimationHasSound && _animDisplayingFrame != _animLastFrame)
			return false;

		// Don't display tray during the intro cinematic.
		if (_gameID == GID_SCHIZM && _loadedAnimation == 200)
			return false;

		return true;
	}

	return false;
}

void Runtime::resetInventoryHighlights() {
	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];
		if (item.highlighted) {
			item.highlighted = false;
			drawInventory(slot);
		}
	}
}

void Runtime::loadInventoryFromPage() {
	for (uint slot = 0; slot < kNumInventorySlots; slot++)
		_inventory[slot] = _inventoryPages[_inventoryActivePage][slot];
}

void Runtime::copyInventoryToPage() {
	for (uint slot = 0; slot < kNumInventorySlots; slot++)
		_inventoryPages[_inventoryActivePage][slot] = _inventory[slot];
}

void Runtime::cheatPutItem() {
	uint32 location = getLocationForScreen(_roomNumber, _screenNumber);

	uint8 &pid = _placedItems[location];
	pid++;

	if (pid == 30 || pid == 45 || pid == 49 || pid == 59)
		pid++;
	else if (pid == 62)
		pid += 2;
	else if (pid == 74)
		pid = 1;

	updatePlacedItemCache();

	clearPlacedItemGraphic();
	drawPlacedItemGraphic();
}

uint32 Runtime::getLocationForScreen(uint roomNumber, uint screenNumber) {
	return roomNumber * 10000u + screenNumber;
}

void Runtime::updatePlacedItemCache() {
	uint32 placedItemLocationID = getLocationForScreen(_roomNumber, _screenNumber);
	Common::HashMap<uint32, uint8>::const_iterator placedItemIt = _placedItems.find(placedItemLocationID);
	if (placedItemIt != _placedItems.end()) {
		uint8 itemID = placedItemIt->_value;

		if (_inventoryPlacedItemCache.itemID != itemID) {
			Common::String itemFileName;
			Common::String alphaFileName;

			_inventoryPlacedItemCache.itemID = itemID;
			getFileNamesForItemGraphic(itemID, itemFileName, alphaFileName);
			_inventoryPlacedItemCache.graphic = loadGraphic(itemFileName, alphaFileName, false);
		}
	} else {
		_inventoryPlacedItemCache = InventoryItem();
	}
}

void Runtime::drawPlacedItemGraphic() {
	const Graphics::Surface *surf = _inventoryPlacedItemCache.graphic.get();
	if (surf) {
		Common::Point drawPos((_placedItemRect.left + _placedItemRect.right - surf->w) / 2, (_placedItemRect.top + _placedItemRect.bottom - surf->h) / 2);

		_gameSection.surf->blitFrom(*surf, drawPos);
		drawSectionToScreen(_gameSection, _placedItemRect);
	}
}

void Runtime::clearPlacedItemGraphic() {
	if (!_placedItemRect.isEmpty()) {
		_gameSection.surf->blitFrom(*_placedItemBackBufferSection.surf, Common::Point(_placedItemRect.left, _placedItemRect.top));
		drawSectionToScreen(_gameSection, _placedItemRect);
	}
}

void Runtime::drawActiveItemGraphic() {
	if (_inventoryActiveItem.graphic) {
		Common::Rect itemRect = AD2044Interface::getRectForUI(AD2044InterfaceRectID::ActiveItemRender);

		_fullscreenMenuSection.surf->blitFrom(*_inventoryActiveItem.graphic, Common::Point(itemRect.left, itemRect.top));
		drawSectionToScreen(_fullscreenMenuSection, itemRect);
	}

	if (g_ad2044ItemInfos[_inventoryActiveItem.itemID].canBeExamined && _hero == 0) {
		Common::Rect examineRect = AD2044Interface::getRectForUI(AD2044InterfaceRectID::ExamineButton);

		_fullscreenMenuSection.surf->blitFrom(*_ad2044Graphics->examine, Common::Point(examineRect.left, examineRect.top));
		drawSectionToScreen(_fullscreenMenuSection, examineRect);
	}
}

void Runtime::clearActiveItemGraphic() {
	Common::Rect rectsToClear[] = {
		AD2044Interface::getRectForUI(AD2044InterfaceRectID::ActiveItemRender),
		AD2044Interface::getRectForUI(AD2044InterfaceRectID::ExamineButton),
	};

	for (const Common::Rect &rectToClear : rectsToClear) {
		_fullscreenMenuSection.surf->blitFrom(*_backgroundGraphic, rectToClear, rectToClear);
		drawSectionToScreen(_fullscreenMenuSection, rectToClear);
	}
}

void Runtime::drawInventoryItemGraphic(uint slot) {
	if (_inventory[slot].graphic) {
		Common::Rect rect = AD2044Interface::getRectForUI(static_cast<AD2044InterfaceRectID>(static_cast<uint>(AD2044InterfaceRectID::InventoryRender0) + slot));

		_fullscreenMenuSection.surf->blitFrom(*_inventory[slot].graphic, Common::Point(rect.left, rect.top));
		drawSectionToScreen(_fullscreenMenuSection, rect);
	}
}

void Runtime::clearInventoryItemGraphic(uint slot) {
	Common::Rect rect = AD2044Interface::getRectForUI(static_cast<AD2044InterfaceRectID>(static_cast<uint>(AD2044InterfaceRectID::InventoryRender0) + slot));

	_fullscreenMenuSection.surf->blitFrom(*_backgroundGraphic, rect, rect);
	drawSectionToScreen(_fullscreenMenuSection, rect);
}

void Runtime::dropActiveItem() {
	if (_inventoryActiveItem.itemID == 0)
		return;

	uint32 locationID = getLocationForScreen(_roomNumber, _screenNumber);
	uint8 &placedItemIDRef = _placedItems[locationID];

	if (placedItemIDRef == 0) {
		placedItemIDRef = static_cast<uint8>(_inventoryActiveItem.itemID);
		_inventoryPlacedItemCache = _inventoryActiveItem;
		_inventoryActiveItem = InventoryItem();
	}

	drawPlacedItemGraphic();
	clearActiveItemGraphic();
}

void Runtime::pickupPlacedItem() {
	if (_inventoryActiveItem.itemID != 0)
		return;

	uint32 locationID = getLocationForScreen(_roomNumber, _screenNumber);
	Common::HashMap<uint32, uint8>::iterator placedItemIt = _placedItems.find(locationID);
	if (placedItemIt == _placedItems.end())
		return;

	if (placedItemIt->_value != _inventoryPlacedItemCache.itemID)
		error("Placed item cache desynced somehow, please report this as a bug");

	_placedItems.erase(placedItemIt);
	_inventoryActiveItem = _inventoryPlacedItemCache;
	_inventoryPlacedItemCache = InventoryItem();

	clearPlacedItemGraphic();
	drawActiveItemGraphic();
}

void Runtime::stashActiveItemToInventory(uint slot) {
	if (_inventoryActiveItem.itemID == 0)
		return;

	if (_inventory[slot].itemID != 0)
		return;

	_inventory[slot] = _inventoryActiveItem;
	_inventoryActiveItem = InventoryItem();


	clearActiveItemGraphic();
	drawInventoryItemGraphic(slot);
}

void Runtime::pickupInventoryItem(uint slot) {
	if (_inventoryActiveItem.itemID != 0)
		return;

	if (_inventory[slot].itemID == 0)
		return;

	_inventoryActiveItem = _inventory[slot];
	_inventory[slot] = InventoryItem();

	clearInventoryItemGraphic(slot);
	drawActiveItemGraphic();
}

void Runtime::getFileNamesForItemGraphic(uint itemID, Common::String &outFileName, Common::String &outAlphaFileName) const {
	if (_gameID == GID_REAH)
		outFileName = Common::String::format("Thing%u", itemID);
	else if (_gameID == GID_SCHIZM)
		outFileName = Common::String::format("Item%u", itemID);
	else if (_gameID == GID_AD2044) {
		outFileName = Common::String::format(_lowQualityGraphicsMode ? "RZB%u" : "RZE%u", itemID);
		outAlphaFileName = Common::String::format("MAS%u", itemID);
	} else
		error("Unknown game, can't format inventory item");
}

Common::SharedPtr<Graphics::Surface> Runtime::loadGraphic(const Common::String &graphicName, const Common::String &alphaName, bool required) {
	Common::Path filePath((_gameID == GID_AD2044) ? "rze/" : "Gfx/");

	filePath.appendInPlace(graphicName);
	filePath.appendInPlace((_gameID == GID_AD2044) ? ".BMP" : ".bmp");

	Common::SharedPtr<Graphics::Surface> surf = loadGraphicFromPath(filePath, required);

	if (surf && !alphaName.empty()) {
		Common::SharedPtr<Graphics::Surface> alphaSurf = loadGraphic(alphaName, "", required);
		if (alphaSurf) {
			if (surf->w != alphaSurf->w || surf->h != alphaSurf->h)
				error("Mismatched graphic sizes");

			int h = surf->h;
			int w = surf->w;
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					uint32 alphaSurfPixel = alphaSurf->getPixel(x, y);

					uint8 r = 0;
					uint8 g = 0;
					uint8 b = 0;
					uint8 a = 0;
					alphaSurf->format.colorToARGB(alphaSurfPixel, a, r, g, b);
					if (r < 128)
						surf->setPixel(x, y, 0);
				}
			}
		}
	}

	return surf;
}

Common::SharedPtr<Graphics::Surface> Runtime::loadGraphicFromPath(const Common::Path &filePath, bool required) {
	Common::File f;
	if (!f.open(filePath)) {
		warning("Couldn't open BMP file '%s'", filePath.toString(Common::Path::kNativeSeparator).c_str());
		return nullptr;
	}

	// 1-byte BMPs are placeholders for no file
	if (f.size() == 1)
		return nullptr;

	Image::BitmapDecoder bmpDecoder;
	if (!bmpDecoder.loadStream(f)) {
		warning("Failed to load BMP file '%s'", filePath.toString(Common::Path::kNativeSeparator).c_str());
		return nullptr;
	}

	Common::SharedPtr<Graphics::Surface> surf(new Graphics::Surface(), Graphics::SurfaceDeleter());
	surf->copyFrom(*bmpDecoder.getSurface());
	surf = Common::SharedPtr<Graphics::Surface>(surf->convertTo(Graphics::createPixelFormat<8888>(), bmpDecoder.getPalette(), bmpDecoder.getPaletteColorCount()), Graphics::SurfaceDeleter());

	return surf;
}

bool Runtime::loadSubtitles(Common::CodePage codePage, bool guessCodePage) {
	Common::Path filePath(Common::String::format("Log/Speech%02u.txt", _languageIndex));

	Common::INIFile ini;

	ini.allowNonEnglishCharacters();

	if (!ini.loadFromFile(filePath)) {
		warning("Couldn't load subtitle data");
		return false;
	}

	if (guessCodePage) {
		bool guessedCodePage = false;

		Common::String checkString;
		if (ini.getKey("szQuestion2", "szTextData", checkString)) {
			const CodePageGuess guesses[] = {
				{Common::CodePage::kWindows1252, kCharSetLatin, "previously", "English"},
				{Common::CodePage::kWindows1252, kCharSetLatin, "\x7c" "berschrieben", "German"},
				{Common::CodePage::kWindows1250, kCharSetLatin, "poprzedni", "Polish"},
				{Common::CodePage::kWindows1252, kCharSetLatin, "pr\xe9" "c\xe9" "dement", "French"},
				{Common::CodePage::kWindows1252, kCharSetLatin, "opgeslagen", "Dutch"},
				{Common::CodePage::kWindows1252, kCharSetLatin, "partida", "Spanish"},
				{Common::CodePage::kWindows1252, kCharSetLatin, "precedentemente", "Italian"},
				{Common::CodePage::kWindows1251, kCharSetCyrillic, "\xf1\xee\xf5\xf0\xe0\xed\xe5\xed\xed\xf3\xfe", "Russian"},
				{Common::CodePage::kWindows1253, kCharSetGreek, "\xf0\xf1\xef\xe7\xe3\xef\xfd\xec\xe5\xed\xef", "Greek"},

				{Common::CodePage::kWindows1251, kCharSetCyrillic, "\xe7\xe0\xef\xe8\xf1\xe0\xed\xe0\xf2\xe0", "Bulgarian"},
				{Common::CodePage::kBig5, kCharSetChineseTraditional, "\xc0\x78\xa6\x73", "Chinese (Traditional)"},
				{Common::CodePage::kGBK, kCharSetChineseSimplified, "\xb4\xa2\xb4\xe6", "Chinese (Simplified)"},
				{Common::CodePage::kWindows1250, kCharSetLatin, "j\x6at\xe9k\xe1ll\xe1st", "Hungarian"},
				{Common::CodePage::kWindows932, kCharSetJapanese, "\x8f\xe3\x8f\x91\x82\xab", "Japanese"},
				{Common::CodePage::kWindows1250, kCharSetLatin, "p\xf8" "edchoz\xed", "Czech"},
			};

			for (const CodePageGuess &guess : guesses) {
				if (checkString.contains(guess.searchString)) {
					codePage = guess.codePage;
					warning("Fallback language detection: Guessed language as %s", guess.languageName);
					guessedCodePage = true;
					break;
				}
			}
		}

		if (!guessedCodePage)
			warning("Couldn't guess text encoding from localization content, please report this as a bug!");
	}

	for (const Common::INIFile::Section &section : ini.getSections()) {
		if (section.name == "Anims")
			continue;	// Ignore

		FrameToSubtitleMap_t *frameMap = nullptr;
		bool isWave = false;

		if (section.name.hasPrefix("Disc-"))
			isWave = true;
		else if (section.name.size() == 8 && section.name.hasPrefix("Anim")) {
			uint animID = 0;
			if (sscanf(section.name.substr(4, 4).c_str(), "%u", &animID) == 1)
				frameMap = &_animSubtitles[animID];
		}

		bool isTextData = (section.name == "szTextData");
		bool isFontData = (section.name == "szFontData");
		bool isStringData = (section.name.hasPrefix("szData"));

		for (const Common::INIFile::KeyValue &kv : section.getKeys()) {
			// Tokenize the line
			Common::Array<Common::String> tokens;

			{
				const Common::String &valueStr = kv.value;

				uint currentTokenStart = 0;
				uint nextCharPos = 0;
				bool isQuotedString = false;

				while (nextCharPos < valueStr.size()) {
					char c = valueStr[nextCharPos];
					nextCharPos++;

					if (isQuotedString) {
						if (c == '\"')
							isQuotedString = false;
						continue;
					}

					if (c == '\"') {
						isQuotedString = true;
						continue;
					}

					if (c == ',') {
						while (valueStr[currentTokenStart] == ' ')
							currentTokenStart++;

						tokens.push_back(valueStr.substr(currentTokenStart, (nextCharPos - currentTokenStart) - 1u));

						currentTokenStart = nextCharPos;
					}

					if (c == ';') {
						nextCharPos--;
						break;
					}
				}

				while (currentTokenStart < nextCharPos && valueStr[currentTokenStart] == ' ')
					currentTokenStart++;

				while (nextCharPos > currentTokenStart && valueStr[nextCharPos - 1] == ' ')
					nextCharPos--;

				if (currentTokenStart < nextCharPos)
					tokens.push_back(valueStr.substr(currentTokenStart, (nextCharPos - currentTokenStart)));
			}

			if (frameMap != nullptr || isWave) {
				if (tokens.size() != 4)
					continue;

				const Common::String &textToken = tokens[3];

				if (textToken[0] != '\"' || textToken[textToken.size() - 1] != '\"')
					continue;

				uint colorCode = 0;
				uint param1 = 0;
				uint param2 = 0;
				if (sscanf(tokens[0].c_str(), "0x%x", &colorCode) && sscanf(tokens[1].c_str(), "0x%x", &param1) && sscanf(tokens[2].c_str(), "%u", &param2)) {
					SubtitleDef *subDef = nullptr;

					if (isWave)
						subDef = &_waveSubtitles[kv.key];
					else if (frameMap) {
						uint frameNum = 0;
						if (kv.key.size() >= 4 && kv.key.substr(kv.key.size() - 3) == "_01" && sscanf(kv.key.substr(0, kv.key.size() - 3).c_str(), "%u", &frameNum) == 1)
							subDef = &(*frameMap)[frameNum];
					}

					if (subDef) {
						subDef->color[0] = ((colorCode >> 16) & 0xff);
						subDef->color[1] = ((colorCode >> 8) & 0xff);
						subDef->color[2] = (colorCode & 0xff);
						subDef->unknownValue1 = param1;
						subDef->durationInDeciseconds = param2;
						subDef->str = textToken.substr(1, textToken.size() - 2).decode(codePage).encode(Common::kUtf8);
					}
				}
			} else if (isTextData) {
				if (tokens.size() != 1)
					continue;

				const Common::String &textToken = tokens[0];

				if (textToken[0] != '\"' || textToken[textToken.size() - 1] != '\"')
					continue;

				_locStrings[kv.key] = textToken.substr(1, textToken.size() - 2).decode(codePage).encode(Common::kUtf8);
			} else if (isFontData) {
				if (tokens.size() != 9)
					continue;

				const Common::String &fontToken = tokens[0];

				if (fontToken[0] != '\"' || fontToken[fontToken.size() - 1] != '\"')
					continue;

				TextStyleDef tsDef;
				tsDef.fontName = fontToken.substr(1, fontToken.size() - 2);

				if (sscanf(tokens[1].c_str(), "%u", &tsDef.size) &&
					sscanf(tokens[2].c_str(), "%u", &tsDef.unknown1) &&
					sscanf(tokens[3].c_str(), "%u", &tsDef.unknown2) &&
					sscanf(tokens[4].c_str(), "%u", &tsDef.unknown3) &&
					sscanf(tokens[5].c_str(), "0x%x", &tsDef.colorRGB) &&
					sscanf(tokens[6].c_str(), "0x%x", &tsDef.shadowColorRGB) &&
					sscanf(tokens[7].c_str(), "%u", &tsDef.alignment) &&
					sscanf(tokens[8].c_str(), "%u", &tsDef.unknown5)) {
					_locTextStyles[kv.key] = tsDef;
				}
			} else if (isStringData) {
				if (tokens.size() != 6)
					continue;

				UILabelDef labelDef;
				labelDef.lineID = tokens[0];
				labelDef.styleDefID = tokens[1];

				if (sscanf(tokens[2].c_str(), "%u", &labelDef.graphicLeft) &&
					sscanf(tokens[3].c_str(), "%u", &labelDef.graphicTop) &&
					sscanf(tokens[4].c_str(), "%u", &labelDef.graphicWidth) &&
					sscanf(tokens[5].c_str(), "%u", &labelDef.graphicHeight)) {
					_locUILabels[kv.key] = labelDef;
				}
			}
		}
	}

	return true;
}

void Runtime::changeToMenuPage(MenuPage *menuPage) {
	_menuPage.reset(menuPage);

	_gameState = kGameStateMenu;

	changeToCursor(_cursors[kCursorArrow]);

	menuPage->init(_menuInterface.get());
	menuPage->start();
}

void Runtime::checkInGameMenuHover() {
	// TODO
	if (_gameID == GID_AD2044)
		return;

	if (_inGameMenuState == kInGameMenuStateInvisible) {
		if (_menuSection.rect.contains(_mousePos) && _isInGame) {
			// Figure out what elements should be visible

			// Help
			_inGameMenuButtonActive[0] = true;

			// Save
			_inGameMenuButtonActive[1] = (_mostRecentlyRecordedSaveState != nullptr);

			// Load
			_inGameMenuButtonActive[2] = static_cast<VCruiseEngine *>(g_engine)->hasAnySave();

			// Sound
			_inGameMenuButtonActive[3] = true;

			// Quit
			_inGameMenuButtonActive[4] = true;

			_inGameMenuState = kInGameMenuStateVisible;
			for (uint i = 0; i < 5; i++)
				drawInGameMenuButton(i);
		}
	}

	if (_inGameMenuState == kInGameMenuStateInvisible)
		return;

	if (!_menuSection.rect.contains(_mousePos) || !_isInGame) {
		if (_inGameMenuState != kInGameMenuStateClickingOver && _inGameMenuState != kInGameMenuStateClickingNotOver && _inGameMenuState != kInGameMenuStateClickingInactive) {
			dismissInGameMenu();
			return;
		}
	}

	uint activeElement = 0;
	if (_mousePos.x >= _menuSection.rect.left && _mousePos.y < _menuSection.rect.right)
		activeElement = static_cast<uint>(_mousePos.x - _menuSection.rect.left) / 128u;

	assert(activeElement < 5);

	switch (_inGameMenuState) {
	case kInGameMenuStateVisible:
		if (_inGameMenuButtonActive[activeElement]) {
			_inGameMenuState = kInGameMenuStateHoveringActive;
			_inGameMenuActiveElement = activeElement;
			drawInGameMenuButton(activeElement);
		}
		break;
	case kInGameMenuStateHoveringActive:
		if (activeElement != _inGameMenuActiveElement) {
			uint oldElement = _inGameMenuActiveElement;

			if (_inGameMenuButtonActive[activeElement]) {
				_inGameMenuState = kInGameMenuStateHoveringActive;
				_inGameMenuActiveElement = activeElement;
				drawInGameMenuButton(activeElement);
			} else
				_inGameMenuState = kInGameMenuStateVisible;

			drawInGameMenuButton(oldElement);
		}
		break;
	case kInGameMenuStateClickingOver:
		if (activeElement != _inGameMenuActiveElement || _mousePos.y >= _menuSection.rect.bottom) {
			_inGameMenuState = kInGameMenuStateClickingNotOver;
			drawInGameMenuButton(_inGameMenuActiveElement);
		}
		break;
	case kInGameMenuStateClickingNotOver:
		if (activeElement == _inGameMenuActiveElement && _mousePos.y < _menuSection.rect.bottom) {
			_inGameMenuState = kInGameMenuStateClickingOver;
			drawInGameMenuButton(_inGameMenuActiveElement);
		}
		break;
	case kInGameMenuStateClickingInactive:
		break;
	default:
		error("Invalid menu state");
		break;
	}
}

void Runtime::dismissInGameMenu() {
	const Common::Rect menuRect(0, 0, _menuSection.surf->w, _menuSection.surf->h);

	uint32 blackColor = _menuSection.surf->format.RGBToColor(0, 0, 0);
	_menuSection.surf->fillRect(menuRect, blackColor);

	commitSectionToScreen(_menuSection, menuRect);

	_inGameMenuState = kInGameMenuStateInvisible;
}

void Runtime::dischargeInGameMenuMouseUp() {
	if (_inGameMenuState == kInGameMenuStateClickingOver) {
		dismissInGameMenu();

		// Handle click event
		switch (_inGameMenuActiveElement) {
		case 0:
			changeToMenuPage(createMenuHelp(_gameID == GID_SCHIZM));
			break;
		case 1:
			g_engine->saveGameDialog();
			break;
		case 2:
			g_engine->loadGameDialog();
			break;
		case 3:
			changeToMenuPage(createMenuSound(_gameID == GID_SCHIZM));
			break;
		case 4:
			changeToMenuPage(createMenuQuit(_gameID == GID_SCHIZM));
			break;
		default:
			break;
		}
	} else {
		_inGameMenuState = kInGameMenuStateVisible;
		drawInGameMenuButton(_inGameMenuActiveElement);

		checkInGameMenuHover();
	}
}

void Runtime::drawInGameMenuButton(uint element) {
	Common::Rect buttonDestRect = Common::Rect(element * 128u, 0, element * 128u + 128u, _menuSection.rect.height());

	int buttonState = 0;
	if (_inGameMenuButtonActive[element])
		buttonState = 1;

	switch (_inGameMenuState) {
	case kInGameMenuStateVisible:
		break;
	case kInGameMenuStateHoveringActive:
		if (element == _inGameMenuActiveElement)
			buttonState = 2;
		break;
	case kInGameMenuStateClickingOver:
		if (element == _inGameMenuActiveElement)
			buttonState = 3;
		break;
	case kInGameMenuStateClickingNotOver:
		if (element == _inGameMenuActiveElement)
			buttonState = 2;
		break;
	case kInGameMenuStateClickingInactive:
		break;
	default:
		error("Invalid menu state");
		break;
	}

	Common::Point buttonTopLeftPoint = Common::Point(buttonDestRect.left, buttonDestRect.top);
	buttonTopLeftPoint.y += buttonState * 44;

	Common::Rect buttonSrcRect = Common::Rect(buttonTopLeftPoint.x, buttonTopLeftPoint.y, buttonTopLeftPoint.x + 128, buttonTopLeftPoint.y + _menuSection.rect.height());

	_menuSection.surf->blitFrom(*_uiGraphics[4], buttonSrcRect, buttonDestRect);

	if (_gameID == GID_SCHIZM) {
		int labelNumber = static_cast<int>(element) + 1 + buttonState * 5;

		Common::String labelID = Common::String::format("szData004_%02i", labelNumber);

		drawLabel(_menuSection.surf.get(), labelID, buttonDestRect);
	}

	commitSectionToScreen(_menuSection, buttonDestRect);
}

const Graphics::Font *Runtime::resolveFont(const Common::String &textStyle, uint size) {
	for (const Common::SharedPtr<FontCacheItem> &item : _fontCache) {
		if (item->fname == textStyle && item->size == size)
			return item->font;
	}

	Common::SharedPtr<FontCacheItem> fcItem(new FontCacheItem());
	fcItem->fname = textStyle;
	fcItem->size = size;

#ifdef USE_FREETYPE2
	const char *fontFile = nullptr;

	if (textStyle == "Verdana") {
		switch (_charSet) {
		case kCharSetGreek:
		case kCharSetLatin:
		case kCharSetCyrillic:
		default:
			fontFile = "NotoSans-Bold.ttf";
			break;
		case kCharSetChineseSimplified:
			fontFile = "NotoSansSC-Bold.ttf";
			break;
		case kCharSetChineseTraditional:
			fontFile = "NotoSansTC-Bold.ttf";
			break;
		case kCharSetJapanese:
			fontFile = "NotoSansJP-Bold.ttf";
			break;
		}
	} else if (textStyle == "Arial") {
		switch (_charSet) {
		case kCharSetGreek:
		case kCharSetLatin:
		case kCharSetCyrillic:
		default:
			fontFile = "LiberationSans-Bold.ttf";
			break;
		case kCharSetChineseSimplified:
			fontFile = "NotoSansSC-Bold.ttf";
			break;
		case kCharSetChineseTraditional:
			fontFile = "NotoSansTC-Bold.ttf";
			break;
		case kCharSetJapanese:
			fontFile = "NotoSansJP-Bold.ttf";
			break;
		}
	}

	if (fontFile) {
		// Pass as 61dpi to account for weird scaling
		fcItem->keepAlive.reset(Graphics::loadTTFFontFromArchive(fontFile, size, Graphics::kTTFSizeModeCharacter, 61, 61, Graphics::kTTFRenderModeLight));
		fcItem->font = fcItem->keepAlive.get();
	}
#endif

	if (!fcItem->font)
		fcItem->font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);

	_fontCache.push_back(fcItem);

	return fcItem->font;
}

bool Runtime::resolveCircuitPuzzleInteraction(const Common::Point &relMouse, Common::Point &outCoord, bool &outIsDown, Common::Rect &outHighlightRect) const {
	if (!_circuitPuzzle)
		return false;

	for (uint cy = 0; cy < CircuitPuzzle::kBoardHeight; cy++) {
		for (uint cx = 0; cx < CircuitPuzzle::kBoardWidth; cx++) {
			Common::Point cellCoord(cx, cy);

			const CircuitPuzzle::CellRectSpec *rectSpec = _circuitPuzzle->getCellRectSpec(cellCoord);
			if (_circuitPuzzle->isCellDownLinkOpen(cellCoord)) {
				if (padCircuitInteractionRect(rectSpec->_downLinkRect).contains(relMouse)) {
					outCoord = cellCoord;
					outIsDown = true;
					outHighlightRect = rectSpec->_downLinkRect;
					return true;
				}
			}
			if (_circuitPuzzle->isCellRightLinkOpen(cellCoord)) {
				if (padCircuitInteractionRect(rectSpec->_rightLinkRect).contains(relMouse)) {
					outCoord = cellCoord;
					outIsDown = false;
					outHighlightRect = rectSpec->_rightLinkRect;
					return true;
				}
			}
		}
	}

	return false;
}

void Runtime::clearCircuitPuzzle() {
	_circuitPuzzle.reset();
}

void Runtime::clearCircuitHighlightRect(const Common::Rect &rectBase) {
	// This is an invert, so these operations are symmetrical
	drawCircuitHighlightRect(rectBase);
}

void Runtime::drawCircuitHighlightRect(const Common::Rect &rectBase) {
	Common::Rect rect(rectBase.left, rectBase.top, rectBase.right + 1, rectBase.bottom + 1);

	Graphics::ManagedSurface *surf = _gameSection.surf.get();
	uint32 invertMask = surf->format.ARGBToColor(0, 255, 255, 255);

	const uint kNumSpans = 4;

	void *spanStarts[kNumSpans];
	int32 spanSteps[kNumSpans];
	uint spanNumPixels[kNumSpans];

	spanStarts[0] = surf->getBasePtr(rect.left, rect.top);
	spanStarts[1] = surf->getBasePtr(rect.left, rect.bottom - 1);
	spanStarts[2] = surf->getBasePtr(rect.left, rect.top + 1);
	spanStarts[3] = surf->getBasePtr(rect.right - 1, rect.top + 1);

	spanSteps[0] = surf->format.bytesPerPixel;
	spanSteps[1] = surf->format.bytesPerPixel;
	spanSteps[2] = surf->pitch;
	spanSteps[3] = surf->pitch;

	spanNumPixels[0] = rect.width();
	spanNumPixels[1] = rect.width();
	spanNumPixels[2] = rect.height() - 2;
	spanNumPixels[3] = rect.height() - 2;

	for (uint spanIndex = 0; spanIndex < kNumSpans; spanIndex++) {
		void *pixelPtr = spanStarts[spanIndex];
		int32 step = spanSteps[spanIndex];
		uint numPixels = spanNumPixels[spanIndex];

		switch (surf->format.bytesPerPixel) {
		case 1:
			while (numPixels > 0) {
				(*static_cast<byte *>(pixelPtr)) ^= invertMask;
				numPixels--;
				pixelPtr = static_cast<byte *>(pixelPtr) + step;
			}
			break;
		case 2:
			while (numPixels > 0) {
				(*static_cast<uint16 *>(pixelPtr)) ^= invertMask;
				numPixels--;
				pixelPtr = static_cast<byte *>(pixelPtr) + step;
			}
			break;
		case 4:
			while (numPixels > 0) {
				(*static_cast<uint32 *>(pixelPtr)) ^= invertMask;
				numPixels--;
				pixelPtr = static_cast<byte *>(pixelPtr) + step;
			}
			break;
		default:
			break;
		}
	}

	drawSectionToScreen(_gameSection, rect);
}

Common::Rect Runtime::padCircuitInteractionRect(const Common::Rect &rect) {
	Common::Rect result = rect;
	result.right += 4;
	result.bottom += 4;
	result.top -= 3;
	result.left -= 3;

	return result;
}

Common::SharedPtr<AnimatedCursor> Runtime::winCursorGroupToAnimatedCursor(const Common::SharedPtr<Graphics::WinCursorGroup> &cursorGroup) {
	Common::SharedPtr<AnimatedCursor> result(new AnimatedCursor());

	result->cursorGroupKeepAlive = cursorGroup;
	result->images.push_back(cursorGroup->cursors[0].cursor);

	AnimatedCursor::FrameDef frameDef;
	frameDef.delay = 1;
	frameDef.imageIndex = 0;

	result->frames.push_back(frameDef);

	return result;
}

Common::SharedPtr<AnimatedCursor> Runtime::aniFileToAnimatedCursor(Image::AniDecoder &aniDecoder) {
	Common::SharedPtr<AnimatedCursor> result(new AnimatedCursor());

	const Image::AniDecoder::Metadata &metadata = aniDecoder.getMetadata();

	if (!metadata.isCURFormat)
		error("ANI file isn't CUR format");

	for (uint step = 0; step < metadata.numSteps; step++) {
		const Image::AniDecoder::FrameDef frame = aniDecoder.getSequenceFrame(step);

		AnimatedCursor::FrameDef outFrameDef;
		outFrameDef.delay = frame.delay;
		outFrameDef.imageIndex = frame.imageIndex;

		result->frames.push_back(outFrameDef);
	}

	for (uint frame = 0; frame < metadata.numFrames; frame++) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(aniDecoder.openImageStream(frame));

		if (!stream)
			error("Couldn't open animated cursor frame");

		Image::IcoCurDecoder icoCurDecoder;
		icoCurDecoder.open(*stream);

		Graphics::Cursor *cursor = icoCurDecoder.loadItemAsCursor(0);

		if (!cursor)
			error("Couldn't load cursor frame");

		result->cursorKeepAlive.push_back(Common::SharedPtr<Graphics::Cursor>(cursor));
		result->images.push_back(cursor);
	}

	return result;
}

Common::SharedPtr<AnimatedCursor> Runtime::staticCursorToAnimatedCursor(const Common::SharedPtr<Graphics::Cursor> &cursor) {
	Common::SharedPtr<AnimatedCursor> result(new AnimatedCursor());

	result->cursorKeepAlive.push_back(cursor);
	result->images.push_back(cursor.get());

	AnimatedCursor::FrameDef frameDef;
	frameDef.delay = 1;
	frameDef.imageIndex = 0;

	result->frames.push_back(frameDef);

	return result;
}

void Runtime::onLButtonDown(int16 x, int16 y) {
	onMouseMove(x, y);

	OSEvent evt;
	evt.type = kOSEventTypeLButtonDown;
	evt.pos = Common::Point(x, y);

	queueOSEvent(evt);
}

void Runtime::onLButtonUp(int16 x, int16 y) {
	onMouseMove(x, y);

	OSEvent evt;
	evt.type = kOSEventTypeLButtonUp;
	evt.pos = Common::Point(x, y);

	queueOSEvent(evt);
}

void Runtime::onMouseMove(int16 x, int16 y) {
	OSEvent evt;
	evt.type = kOSEventTypeMouseMove;
	evt.pos = Common::Point(x, y);

	queueOSEvent(evt);
}

void Runtime::onKeymappedEvent(KeymappedEvent kme) {
	OSEvent evt;
	evt.type = kOSEventTypeKeymappedEvent;
	evt.keymappedEvent = kme;

	queueOSEvent(evt);
}

bool Runtime::canSave(bool onCurrentScreen) const {
	if (onCurrentScreen) {
		return (_mostRecentlyRecordedSaveState.get() != nullptr && (_haveHorizPanAnimations || _forceAllowSaves || _gameID == GID_AD2044));
	} else {
		return _mostRecentValidSaveState.get() != nullptr && _isInGame;
	}
}

bool Runtime::canLoad() const {
	return _gameState == kGameStateIdle || _gameState == kGameStateMenu;
}

void Runtime::recordSaveGameSnapshot() {
	if (!_isInGame)
		return;

	_mostRecentlyRecordedSaveState.reset();

	uint32 timeBase = g_system->getMillis();

	Common::SharedPtr<SaveGameSnapshot> snapshot(new SaveGameSnapshot());

	_mostRecentlyRecordedSaveState = snapshot;

	snapshot->states[0].reset(new SaveGameSwappableState());
	if (_gameID == GID_REAH)
		snapshot->numStates = 1;
	else if (_gameID == GID_SCHIZM || _gameID == GID_AD2044) {
		snapshot->numStates = 2;
		snapshot->states[1] = _altState;
	}

	SaveGameSwappableState *mainState = snapshot->states[0].get();

	if (_gameID == GID_AD2044) {
		copyInventoryToPage();
	} else {
		for (const InventoryItem &inventoryItem : _inventory) {
			SaveGameSwappableState::InventoryItem saveItem;
			saveItem.itemID = inventoryItem.itemID;
			saveItem.highlighted = inventoryItem.highlighted;

			mainState->inventory.push_back(saveItem);
		}
	}

	mainState->roomNumber = _roomNumber;
	mainState->screenNumber = _screenNumber;
	mainState->direction = _direction;
	mainState->havePendingPostSwapScreenReset = false;
	snapshot->hero = _hero;
	snapshot->inventoryPage = _inventoryActivePage;
	snapshot->inventoryActiveItem = _inventoryActiveItem.itemID;

	snapshot->pendingStaticAnimParams = _pendingStaticAnimParams;

	snapshot->variables = _variables;

	for (const Common::HashMap<uint, uint32>::Node &timerNode : _timers)
		snapshot->timers[timerNode._key] = timerNode._value - timeBase;

	snapshot->escOn = _escOn;

	mainState->musicTrack = _musicTrack;
	mainState->musicActive = _musicActive;
	mainState->musicMuteDisabled = _musicMuteDisabled;

	mainState->musicVolume = _musicVolume;

	// If music volume is ramping, use the end volume and skip the ramp
	if (_musicVolumeRampRatePerMSec != 0)
		mainState->musicVolume = _musicVolumeRampEnd;

	mainState->scoreSection = _scoreSection;
	mainState->scoreTrack = _scoreTrack;

	mainState->loadedAnimation = _loadedAnimation;
	mainState->animDisplayingFrame = _animDisplayingFrame;

	if (_gameID == GID_AD2044) {
		mainState->haveIdleAnimationLoop = _haveIdleAnimations[_direction];

		if (mainState->haveIdleAnimationLoop) {
			mainState->idleAnimNum = _idleAnimations[0].animDefs[0].animNum;
			mainState->idleFirstFrame = _idleAnimations[0].animDefs[0].firstFrame;
			mainState->idleLastFrame = _idleAnimations[0].animDefs[0].lastFrame;
		}
	} else
		mainState->haveIdleAnimationLoop = false;

	recordSounds(*mainState);

	snapshot->pendingSoundParams3D = _pendingSoundParams3D;

	snapshot->triggeredOneShots = _triggeredOneShots;
	snapshot->sayCycles = _sayCycles;

	snapshot->listenerX = _listenerX;
	snapshot->listenerY = _listenerY;
	snapshot->listenerAngle = _listenerAngle;

	for (const Common::HashMap<uint32, uint8>::Node &placedItem : _placedItems) {
		SaveGameSnapshot::PlacedInventoryItem saveItem;
		saveItem.locationID = placedItem._key;
		saveItem.itemID = placedItem._value;

		snapshot->placedItems.push_back(saveItem);
	}

	for (uint page = 0; page < kNumInventoryPages; page++) {
		for (uint slot = 0; slot < kNumInventorySlots; slot++) {
			uint itemID = _inventoryPages[page][slot].itemID;
			if (itemID != 0) {
				SaveGameSnapshot::PagedInventoryItem pagedItem;
				pagedItem.page = page;
				pagedItem.slot = slot;
				pagedItem.itemID = itemID;

				snapshot->pagedItems.push_back(pagedItem);
			}
		}
	}
}

void Runtime::recordSounds(SaveGameSwappableState &state) {
	state.sounds.clear();

	state.randomAmbientSounds = _randomAmbientSounds;

	for (const Common::SharedPtr<SoundInstance> &soundPtr : _activeSounds) {
		const SoundInstance &sound = *soundPtr;

		SaveGameSwappableState::Sound saveSound;
		saveSound.name = sound.name;

		saveSound.id = sound.id;

		saveSound.volume = sound.volume;
		saveSound.balance = sound.balance;

		// Skip ramp
		if (sound.rampRatePerMSec != 0) {
			if (sound.rampTerminateOnCompletion)
				continue; // Don't even save this

			saveSound.volume = sound.rampEndVolume;
		}

		saveSound.is3D = sound.is3D;
		saveSound.isLooping = sound.isLooping;
		saveSound.tryToLoopWhenRestarted = sound.tryToLoopWhenRestarted;
		saveSound.isSpeech = sound.isSpeech;
		saveSound.x = sound.x;
		saveSound.y = sound.y;

		saveSound.params3D = sound.params3D;

		state.sounds.push_back(saveSound);
	}
}

void Runtime::restoreSaveGameSnapshot() {
	_mostRecentlyRecordedSaveState = _mostRecentValidSaveState;

	SaveGameSnapshot *snapshot = _mostRecentValidSaveState.get();

	uint32 timeBase = g_system->getMillis();

	_altState = snapshot->states[1];

	SaveGameSwappableState *mainState = snapshot->states[0].get();

	if (_gameID == GID_AD2044) {
		for (uint page = 0; page < kNumInventoryPages; page++)
			for (uint slot = 0; slot < kNumInventorySlots; slot++)
				_inventoryPages[page][slot].itemID = 0;

		for (const SaveGameSnapshot::PagedInventoryItem &pagedItem : snapshot->pagedItems) {
			if (pagedItem.page >= kNumInventoryPages || pagedItem.slot >= kNumInventorySlots)
				error("Invalid item slot in save game snapshot");

			InventoryItem &invItem = _inventoryPages[pagedItem.page][pagedItem.slot];
			invItem.itemID = pagedItem.itemID;			

			if (invItem.itemID) {
				Common::String itemFileName;
				Common::String alphaFileName;
				getFileNamesForItemGraphic(invItem.itemID, itemFileName, alphaFileName);
				invItem.graphic = loadGraphic(itemFileName, alphaFileName, false);
			}
		}

		for (uint page = 0; page < kNumInventoryPages; page++) {
			for (uint slot = 0; slot < kNumInventorySlots; slot++) {
				InventoryItem &invItem = _inventoryPages[page][slot];
				if (!invItem.itemID)
					invItem.graphic.reset();
			}
		}

		_inventoryActiveItem.itemID = snapshot->inventoryActiveItem;
		if (_inventoryActiveItem.itemID) {
			Common::String itemFileName;
			Common::String alphaFileName;
			getFileNamesForItemGraphic(_inventoryActiveItem.itemID, itemFileName, alphaFileName);
			_inventoryActiveItem.graphic = loadGraphic(itemFileName, alphaFileName, false);
		} else {
			_inventoryActiveItem.graphic.reset();
		}

		_inventoryPlacedItemCache = InventoryItem();

		if (snapshot->inventoryPage >= kNumInventoryPages)
			error("Invalid inventory page");

		_inventoryActivePage = snapshot->inventoryPage;

		loadInventoryFromPage();

		_placedItems.clear();
		for (const SaveGameSnapshot::PlacedInventoryItem &placedItem : snapshot->placedItems) {
			_placedItems[placedItem.locationID] = placedItem.itemID;
		}
	} else {
		for (uint i = 0; i < kNumInventorySlots && i < mainState->inventory.size(); i++) {
			const SaveGameSwappableState::InventoryItem &saveItem = mainState->inventory[i];

			_inventory[i].itemID = saveItem.itemID;
			_inventory[i].highlighted = saveItem.highlighted;

			if (saveItem.itemID) {
				Common::String itemFileName;
				Common::String alphaFileName;
				getFileNamesForItemGraphic(saveItem.itemID, itemFileName, alphaFileName);
				_inventory[i].graphic = loadGraphic(itemFileName, alphaFileName, false);
			} else {
				_inventory[i].graphic.reset();
			}
		}
	}

	_keepStaticAnimationInIdle = false;
	_roomNumber = mainState->roomNumber;
	_screenNumber = mainState->screenNumber;
	_direction = mainState->direction;
	_disc = mainState->disc;
	_havePendingPostSwapScreenReset = mainState->havePendingPostSwapScreenReset;
	_hero = snapshot->hero;
	_swapOutRoom = snapshot->swapOutRoom;
	_swapOutScreen = snapshot->swapOutScreen;
	_swapOutDirection = snapshot->swapOutDirection;

	_pendingStaticAnimParams = snapshot->pendingStaticAnimParams;

	_variables.clear();
	_variables = snapshot->variables;

	_timers.clear();

	_havePendingPostSwapScreenReset = true;

	for (const Common::HashMap<uint, uint32>::Node &timerNode : snapshot->timers)
		_timers[timerNode._key] = timerNode._value + timeBase;

	_escOn = snapshot->escOn;

	_musicVolume = mainState->musicVolume;
	_musicVolumeRampStartTime = 0;
	_musicVolumeRampStartVolume = 0;
	_musicVolumeRampRatePerMSec = 0;
	_musicVolumeRampEnd = _musicVolume;

	_musicActive = mainState->musicActive;

	if (_musicActive) {
		bool musicMutedBeforeRestore = (_musicMute && !_musicMuteDisabled);
		bool musicMutedAfterRestore = (_musicMute && !mainState->musicMuteDisabled);
		bool isNewTrack = (_scoreTrack != mainState->scoreTrack);

		_musicMuteDisabled = mainState->musicMuteDisabled;
		_scoreTrack = mainState->scoreTrack;

		if (_gameID == GID_REAH || _gameID == GID_AD2044)
			changeMusicTrack(mainState->musicTrack);
		if (_gameID == GID_SCHIZM) {
			// Only restart music if a new track is playing
			if (isNewTrack)
				_scoreSection = mainState->scoreSection;

			if (!musicMutedBeforeRestore && musicMutedAfterRestore) {
				_musicWavePlayer.reset();
				{
					Common::StackLock lock(_midiPlayerMutex);
					_musicMidiPlayer.reset();
				}
				_scoreSectionEndTime = 0;
			} else if (!musicMutedAfterRestore && (isNewTrack || musicMutedBeforeRestore))
				startScoreSection();
		}
	} else {
		_musicWavePlayer.reset();
		if (_musicMidiPlayer) {
			Common::StackLock lock(_midiPlayerMutex);
			_musicMidiPlayer.reset();
		}
		_scoreSectionEndTime = 0;
	}

	// Stop all sounds since the player instances are stored in the sound cache.
	for (Common::SharedPtr<SoundInstance> &snd : _activeSounds)
		stopSound(*snd);

	_activeSounds.clear();

	_pendingSoundParams3D = snapshot->pendingSoundParams3D;

	_triggeredOneShots = snapshot->triggeredOneShots;
	_sayCycles = snapshot->sayCycles;

	_listenerX = snapshot->listenerX;
	_listenerY = snapshot->listenerY;
	_listenerAngle = snapshot->listenerAngle;

	_randomAmbientSounds = mainState->randomAmbientSounds;

	for (const SaveGameSwappableState::Sound &sound : mainState->sounds) {
		Common::SharedPtr<SoundInstance> si(new SoundInstance());

		si->name = sound.name;
		si->id = sound.id;
		si->volume = sound.volume;
		si->balance = sound.balance;
		si->is3D = sound.is3D;
		si->isLooping = sound.isLooping;
		si->tryToLoopWhenRestarted = sound.tryToLoopWhenRestarted;
		si->isSpeech = sound.isSpeech;
		si->x = sound.x;
		si->y = sound.y;
		si->params3D = sound.params3D;

		_activeSounds.push_back(si);

		if (sound.isLooping)
			triggerSound(kSoundLoopBehaviorYes, *si, si->volume, si->balance, si->is3D, si->isSpeech);
		else if (sound.tryToLoopWhenRestarted)
			triggerSound(kSoundLoopBehaviorAuto, *si, getSilentSoundVolume(), si->balance, si->is3D, si->isSpeech);
	}

	uint anim = mainState->loadedAnimation;
	uint frame = mainState->animDisplayingFrame;

	AnimationDef animDef;
	animDef.animNum = anim;
	animDef.firstFrame = frame;
	animDef.lastFrame = frame;

	changeAnimation(animDef, false);

	_gameState = kGameStateWaitingForAnimation;
	_isInGame = true;

	_havePendingScreenChange = true;
	_forceScreenChange = true;

	stopSubtitles();
	clearScreen();
	redrawTray();

	if (_gameID == GID_AD2044) {
		drawActiveItemGraphic();

		clearIdleAnimations();

		if (mainState->haveIdleAnimationLoop) {
			_keepStaticAnimationInIdle = true;

			
			AnimationDef idleAnimDef;
			idleAnimDef.animNum = mainState->idleAnimNum;
			idleAnimDef.firstFrame = mainState->idleFirstFrame;
			idleAnimDef.lastFrame = mainState->idleLastFrame;

			_keepStaticAnimationInIdle = true;

			_haveIdleAnimations[0] = true;

			StaticAnimation staticAnim;
			staticAnim.animDefs[0] = idleAnimDef;
			staticAnim.animDefs[1] = idleAnimDef;

			_idleAnimations[0] = staticAnim;
		}
	}
}

Common::SharedPtr<SaveGameSnapshot> Runtime::generateNewGameSnapshot() const {
	Common::SharedPtr<SaveGameSnapshot> snapshot(new SaveGameSnapshot());

	snapshot->numStates = 1;
	snapshot->states[0].reset(new SaveGameSwappableState());

	SaveGameSwappableState *mainState = snapshot->states[0].get();

	mainState->roomNumber = _startConfigs[kStartConfigInitial].room;
	mainState->screenNumber = _startConfigs[kStartConfigInitial].screen;
	mainState->direction = _startConfigs[kStartConfigInitial].direction;

	if (_gameID == GID_SCHIZM) {
		mainState->loadedAnimation = 200;

		snapshot->numStates = 2;
		snapshot->states[1].reset(new SaveGameSwappableState());

		SaveGameSwappableState *altState = snapshot->states[1].get();

		altState->roomNumber = _startConfigs[kStartConfigAlt].room;
		altState->screenNumber = _startConfigs[kStartConfigAlt].screen;
		altState->direction = _startConfigs[kStartConfigAlt].direction;

		altState->loadedAnimation = altState->screenNumber;
	} else
		mainState->loadedAnimation = 1;

	// AD2044 new game normally loads a pre-packaged save.  Unlike Reah and Schizm,
	// it doesn't appear to have a startup script, so we need to set up everything
	// that it needs here.
	if (_gameID == GID_AD2044) {
		mainState->animDisplayingFrame = 345;
		mainState->musicActive = true;
		mainState->musicTrack = 1;

		SaveGameSnapshot::PagedInventoryItem item;
		item.page = 0;
		item.slot = 1;
		item.itemID = 54;	// Electronic goaler (sic)

		for (const InitialItemPlacement &itemPlacement : g_ad2044InitialItemPlacements) {
			SaveGameSnapshot::PlacedInventoryItem placedItem;
			placedItem.locationID = getLocationForScreen(itemPlacement.roomNumber, itemPlacement.screenNumber);
			placedItem.itemID = itemPlacement.itemID;

			snapshot->placedItems.push_back(placedItem);
		}

		snapshot->pagedItems.push_back(item);

		// Alt state is for item inspection
		snapshot->numStates = 2;
		snapshot->states[1].reset(new SaveGameSwappableState());
	}

	return snapshot;
}

void Runtime::saveGame(Common::WriteStream *stream) const {
	_mostRecentValidSaveState->write(stream);
}

LoadGameOutcome Runtime::loadGame(Common::ReadStream *stream) {
	assert(canLoad());

	Common::SharedPtr<SaveGameSnapshot> snapshot(new SaveGameSnapshot());
	LoadGameOutcome outcome = snapshot->read(stream);

	if (outcome != kLoadGameOutcomeSucceeded)
		return outcome;

	_mostRecentValidSaveState = snapshot;
	restoreSaveGameSnapshot();

	return outcome;
}

void Runtime::drawFrame() {
	_system->updateScreen();
}

} // End of namespace VCruise
