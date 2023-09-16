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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ENGINEDATA_H
#define NANCY_ENGINEDATA_H

#include "engines/nancy/commontypes.h"

namespace Nancy {

// Data types corresponding to chunks found inside BOOT

struct EngineData {
	EngineData(Common::SeekableReadStream *chunkStream);
	virtual ~EngineData() {}
};

// Boot summary. Contains data for the UI, game clock, starting a new game.
struct BSUM : public EngineData {
	BSUM(Common::SeekableReadStream *chunkStream);

	byte header[90];

	Common::String conversationTextsFilename;
	Common::String autotextFilename;

	// Game start section
	SceneChangeDescription firstScene;
	uint16 startTimeHours;
	uint16 startTimeMinutes;

	// UI
	Common::Rect extraButtonHotspot;	// Extra button is map in tvd, clock in nancy2 and up
	Common::Rect extraButtonHighlightDest;
	Common::Rect textboxScreenPosition;
	Common::Rect inventoryBoxScreenPosition;
	Common::Rect menuButtonSrc;
	Common::Rect helpButtonSrc;
	Common::Rect menuButtonDest;
	Common::Rect helpButtonDest;
	Common::Rect menuButtonHighlightSrc;
	Common::Rect helpButtonHighlightSrc;
	Common::Rect clockHighlightSrc;

	// Transparent color
	byte paletteTrans, rTrans, gTrans, bTrans;

	uint16 horizontalEdgesSize;
	uint16 verticalEdgesSize;

	uint16 numFonts;

	uint16 playerTimeMinuteLength;
	uint16 buttonPressTimeDelay;
	byte overrideMovementTimeDeltas;
	uint16 slowMovementTimeDelta;
	uint16 fastMovementTimeDelta;
};

// Contains rects defining the in-game viewport
struct VIEW : public EngineData {
	VIEW(Common::SeekableReadStream *chunkStream);

	Common::Rect screenPosition;
	Common::Rect bounds;
};

// Contains a list of .cal filenames, which are to be loaded at startup.
// .cal files themselves are just collections of image files used in dialogue.
// First introduced in nancy2.
struct PCAL : public EngineData {
	PCAL(Common::SeekableReadStream *chunkStream);

	Common::Array<Common::String> calNames;
};

// Contains definitions for all in-game items, as well as data for the
// inventory box at the bottom right of the game screen.
struct INV : public EngineData {
	struct ItemDescription {
		Common::String name;
		byte keepItem;
		Common::Rect sourceRect;
		Common::Rect highlightedSourceRect;

		Common::String specificCantText;
		Common::String generalCantText;
		SoundDescription specificCantSound;
		SoundDescription generalCantSound;
	};

	INV(Common::SeekableReadStream *chunkStream);

	Common::Rect scrollbarSrcBounds;
	Common::Point scrollbarDefaultPos;
	uint16 scrollbarMaxScroll;

	Common::Array<Common::Rect> ornamentSrcs;
	Common::Array<Common::Rect> ornamentDests;

	Common::Array<Common::Rect> curtainAnimationSrcs;
	Common::Rect curtainsScreenPosition;
	uint16 curtainsFrameTime;

	uint16 captionAutoClearTime = 3000;

	Common::String inventoryBoxIconsImageName;
	Common::String inventoryCursorsImageName;

	SoundDescription cantSound;
	Common::String cantText;

	Common::Array<ItemDescription> itemDescriptions;
};

// Contains data about the textbox at the bottom left of the game screen
struct TBOX : public EngineData {
	TBOX(Common::SeekableReadStream *chunkStream);

	Common::Rect scrollbarSrcBounds;
	Common::Rect innerBoundingBox;
	Common::Point scrollbarDefaultPos;
	uint16 scrollbarMaxScroll;

	uint16 firstLineOffset;
	uint16 lineHeight;
	uint16 borderWidth;
	uint16 maxWidthDifference;

	Common::Array<Common::Rect> ornamentSrcs;
	Common::Array<Common::Rect> ornamentDests;

	uint16 defaultFontID;
	uint16 conversationFontID;
	uint16 highlightConversationFontID;
};

// Contains data about the map state. Only used in TVD and nancy1
struct MAP : public EngineData {
	struct Location {
		Common::String description;
		Common::Rect hotspot;
		SceneChangeDescription scenes[2];

		Common::Rect labelSrc;
	};

	MAP(Common::SeekableReadStream *chunkStream);

	Common::Array<Common::String> mapNames;
	Common::Array<Common::String> mapPaletteNames;
	Common::Array<SoundDescription> sounds;

	// Globe section, TVD only
	uint16 globeFrameTime;
	Common::Array<Common::Rect> globeSrcs;
	Common::Rect globeDest;
	Common::Rect globeGargoyleSrc;
	Common::Rect globeGargoyleDest;

	// Button section, nancy1 only
	Common::Rect buttonSrc;
	Common::Rect buttonDest;

	Common::Rect closedLabelSrc;

	Common::Array<Location> locations;

	Common::Point cursorPosition;
};

// Contains data for the help screen.
struct HELP : public EngineData {
	HELP(Common::SeekableReadStream *chunkStream);

	Common::String imageName;
	Common::Rect buttonDest;
	Common::Rect buttonSrc;
	Common::Rect buttonHoverSrc;
};

// Contains data for the credits screen.
struct CRED : public EngineData {
	CRED(Common::SeekableReadStream *chunkStream);

	Common::String imageName;
	Common::Array<Common::String> textNames;
	Common::Rect textScreenPosition;
	uint16 updateTime;
	uint16 pixelsToScroll;
	SoundDescription sound;
};

// Contains data for the main menu.
struct MENU : public EngineData {
	MENU(Common::SeekableReadStream *chunkStream);

	Common::String _imageName;
	Common::Array<Common::Rect> _buttonDests;
	Common::Array<Common::Rect> _buttonDownSrcs;
	Common::Array<Common::Rect> _buttonHighlightSrcs;
	Common::Array<Common::Rect> _buttonDisabledSrcs;
};

// Contains data for the Setup screen (a.k.a settings menu)
struct SET : public EngineData {
	SET(Common::SeekableReadStream *chunkStream);

	Common::String _imageName;
	// Common::Rect _scrollbarsBounds
	Common::Array<Common::Rect> _scrollbarBounds;
	Common::Array<Common::Rect> _buttonDests;
	Common::Array<Common::Rect> _buttonDownSrcs;
	Common::Rect _doneButtonHighlightSrc;
	Common::Array<Common::Rect> _scrollbarSrcs;

	Common::Array<uint16> _scrollbarsCenterYPos;
	Common::Array<uint16> _scrollbarsCenterXPosL;
	Common::Array<uint16> _scrollbarsCenterXPosR;

	Common::Array<SoundDescription> _sounds;
};

// Contains data for the Save/Load screen
struct LOAD : public EngineData {
	LOAD(Common::SeekableReadStream *chunkStream);

	Common::String _imageName;

	int16 _mainFontID;
	int16 _highlightFontID;
	int16 _disabledFontID;
	int16 _fontXOffset;
	int16 _fontYOffset;

	Common::Array<Common::Rect> _saveButtonDests;
	Common::Array<Common::Rect> _loadButtonDests;
	Common::Array<Common::Rect> _textboxBounds;
	Common::Rect _doneButtonDest;
	Common::Array<Common::Rect> _saveButtonDownSrcs;
	Common::Array<Common::Rect> _loadButtonDownSrcs;

	Common::Rect _doneButtonDownSrc;
	Common::Array<Common::Rect> _saveButtonHighlightSrcs;
	Common::Array<Common::Rect> _loadButtonHighlightSrcs;

	Common::Rect _doneButtonHighlightSrc;
	Common::Array<Common::Rect> _saveButtonDisabledSrcs;
	Common::Array<Common::Rect> _loadButtonDisabledSrcs;

	Common::Rect _doneButtonDisabledSrc;
	Common::Rect _blinkingCursorSrc;
	uint16 _blinkingTimeDelay;
	Common::Array<Common::Rect> _cancelButtonSrcs;
	Common::Array<Common::Rect> _cancelButtonDests;
	Common::Rect _cancelButtonDownSrc;
	Common::Rect _cancelButtonHighlightSrc;
	Common::Rect _cancelButtonDisabledSrc;

	Common::String _gameSavedPopup;
	// Common::Rect _gameSavedBounds
};

// Contains data for the prompt that appears when exiting the game
// without saving first. Introduced in nancy3.
struct SDLG : public EngineData {
	SDLG(Common::SeekableReadStream *chunkStream);

	Common::String _imageName;

	Common::Rect _yesDest;
	Common::Rect _noDest;
	Common::Rect _cancelDest;

	Common::Rect _yesHighlightSrc;
	Common::Rect _noHighlightSrc;
	Common::Rect _cancelHighlightSrc;

	Common::Rect _yesDownSrc;
	Common::Rect _noDownSrc;
	Common::Rect _cancelDownSrc;
};

// Contains data for the hint system. Only used in nancy1.
struct HINT : public EngineData {
	HINT(Common::SeekableReadStream *chunkStream);

	Common::Array<uint16> numHints;
};

// Contains data for the slider puzzle. First used in nancy1
struct SPUZ : public EngineData {
	SPUZ(Common::SeekableReadStream *chunkStream);

	Common::Array<Common::Array<int16>> tileOrder;
};

// Contains data for the clock UI that appears at the bottom left of the screen (top left in TVD)
// Not used in nancy1 but still present in the data.
struct CLOK : public EngineData {
	CLOK(Common::SeekableReadStream *chunkStream);

	Common::Array<Common::Rect> animSrcs;
	Common::Array<Common::Rect> animDests;

	Common::Array<Common::Rect> hoursHandSrcs;
	Common::Array<Common::Rect> minutesHandSrcs;
	Common::Rect screenPosition;
	Common::Array<Common::Rect> hoursHandDests;
	Common::Array<Common::Rect> minutesHandDests;

	Common::Rect staticImageSrc;
	Common::Rect staticImageDest;

	uint32 timeToKeepOpen;
	uint16 frameTime;

	uint32 nancy5CountdownTime;
	Common::Array<Common::Rect> nancy5DaySrcs;
	Common::Array<Common::Rect> nancy5CountdownSrcs;
};

// Contains data for special effects (fades between scenes/fades to black).
// Introduced in nancy2.
struct SPEC : public EngineData {
	SPEC(Common::SeekableReadStream *chunkStream);

	byte fadeToBlackNumFrames;
	uint16 fadeToBlackFrameTime;
	byte crossDissolveNumFrames;
};

// Contains data for the raycast puzzle in nancy3. Specifically, this is the
// data for the different "themes" that appear in the 3D space.
struct RCLB : public EngineData {
	struct Theme {
		Common::String themeName;

		Common::Array<uint32> wallIDs;

		Common::Array<uint16> exitFloorIDs;
		Common::Array<uint16> floorIDs;
		Common::Array<uint16> ceilingIDs;

		Common::Array<uint32> doorIDs;
		Common::Array<uint32> transparentwallIDs;
		Common::Array<uint32> objectwallIDs;
		Common::Array<uint16> objectWallHeights;

		uint16 generalLighting;
		uint16 hasLightSwitch;

		int16 transparentWallDensity;
		int16 objectWallDensity;
		int16 doorDensity;
	};

	RCLB(Common::SeekableReadStream *chunkStream);

	uint16 lightSwitchID;
	uint16 unk2;
	Common::Array<Theme> themes;
};

// Contains data about the raycast puzzle in nancy3. Specifically, this is the
// data for the debug map and the names of the textures to be used when rendering.
struct RCPR : public EngineData {
	RCPR(Common::SeekableReadStream *chunkStream);

	Common::Array<Common::Rect> screenViewportSizes;
	uint16 viewportSizeUsed;

	byte wallColor[3];
	byte playerColor[3];
	byte doorColor[3];
	byte lightSwitchColor[3];
	byte exitColor[3];
	byte uColor6[3];
	byte uColor7[3];
	byte uColor8[3];
	byte transparentWallColor[3];
	byte uColor10[3];

	Common::Array<Common::String> wallNames;
	Common::Array<Common::String> specialWallNames;
	Common::Array<Common::String> ceilingNames;
	Common::Array<Common::String> floorNames;
};

// Contains the name and dimensions of an image.
struct ImageChunk : public EngineData {
	ImageChunk(Common::SeekableReadStream *chunkStream);

	Common::String imageName;
	uint16 width;
	uint16 height;
};

// Contains text data. Every string is tagged with a key via which
// it can be accessed. Used to store dialogue and journal (autotext) strings.
// NOT found inside BOOT; these are stored in their own cifs, the names of which
// can be found inside BSUM. Introduced in nancy6. 
struct CVTX : public EngineData {
	CVTX(Common::SeekableReadStream *chunkStream);

	Common::HashMap<Common::String, Common::String> texts;
};

} // End of namespace Nancy

#endif // NANCY_ENGINEDATA_H
