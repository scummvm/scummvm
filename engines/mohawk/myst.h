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

#ifndef MOHAWK_MYST_H
#define MOHAWK_MYST_H

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/myst_vars.h"

#include "gui/saveload.h"

namespace Mohawk {

class MohawkEngine_Myst;
class VideoManager;
class MystGraphics;
class MystScriptParser;
class MystConsole;
class MystSaveLoad;

// Engine Debug Flags
enum {
	kDebugVariable = (1 << 0),
	kDebugSaveLoad = (1 << 1),
	kDebugView     = (1 << 2),
	kDebugHint     = (1 << 3),
	kDebugResource = (1 << 4),
	kDebugINIT     = (1 << 5),
	kDebugEXIT     = (1 << 6),
	kDebugScript   = (1 << 7),
	kDebugHelp     = (1 << 8)
};

// Myst Stacks
enum {
	kChannelwoodStack = 0,	// Channelwood Age
	kCreditsStack,			// Credits
	kDemoStack,				// Demo Main Menu
	kDniStack,				// D'ni
	kIntroStack,			// Intro
	kMakingOfStack,			// Making Of Myst
	kMechanicalStack,		// Mechanical Age
	kMystStack,				// Myst Island
	kSeleniticStack,		// Selenitic Age
	kDemoSlidesStack,		// Demo Slideshow
	kDemoPreviewStack,		// Demo Myst Library Preview
	kStoneshipStack			// Stoneship Age
};

const uint16 kMasterpieceOnly = 0xFFFF;

// Myst Resource Types
// TODO: Other types and such
enum {
	kMystForwardResource = 0,
	kMystLeftResource = 1,
	kMystRightResource = 2,
	kMystDownResource = 3,
	kMystUpResource = 4,
	kMystActionResource = 5,
	kMystVideoResource = 6,
	kMystSwitchResource = 7
};

// Myst Resource Flags
// TODO: Figure out other flags
enum {
	kMystSubimageEnableFlag = (1 << 0),
	kMystHotspotEnableFlag  = (1 << 1),
	kMystUnknownFlag        = (1 << 2),
	kMystZipModeEnableFlag  = (1 << 3)
};

struct MystCondition {
	uint16 var;
	uint16 numStates;
	uint16 *values;
};

// View Sound Action Type
enum {
	kMystSoundActionConditional  = -4,
	kMystSoundActionContinue     = -1,
	kMystSoundActionChangeVolume = -2,
	kMystSoundActionStop         = -3
	// Other positive values are PlayNewSound of that id
};

struct MystView {
	uint16 flags;

	// Image Data
	uint16 conditionalImageCount;
	MystCondition *conditionalImages;
	uint16 mainImage;

	// Sound Data
	int16 sound;
	uint16 soundVolume;
	uint16 soundVar;
	uint16 soundCount;
	int16 *soundList;
	uint16 *soundListVolume;

	// Script Resources
	uint16 scriptResCount;
	struct ScriptResource {
		uint16 type;
		uint16 id; // Not used by type 3
		// TODO: Type 3 has more. Maybe use a union?
		uint16 var; // Used by type 3 only
		uint16 count; // Used by type 3 only
		uint16 u0; // Used by type 3 only
		int16 *resource_list; // Used by type 3 only
	} *scriptResources;

	// Resource ID's
	uint16 rlst;
	uint16 hint;
	uint16 init;
	uint16 exit;
};

struct MystScriptEntry {
	uint16 opcode;
	uint16 var;
	uint16 numValues;
	uint16 *values;
};

class MystResource {
public:
	MystResource(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResource() {}

	MystResource *_parent;

	bool contains(Common::Point point) { return _rect.contains(point); }
	virtual void drawDataToScreen() {}
	virtual void handleAnimation() {}
	virtual Common::Rect getRect() { return _rect; }
	bool isEnabled() { return _enabled; }
	void setEnabled(bool enabled) { _enabled = enabled; }
	uint16 getDest() { return _dest; }
	virtual uint16 getType8Var() { return 0xFFFF; }
	
	// Mouse interface
	virtual void handleMouseUp();
	virtual void handleMouseDown() {}
	virtual void handleMouseEnter() {}
	virtual void handleMouseLeave() {}

protected:
	MohawkEngine_Myst *_vm;

	uint16 _flags;
	Common::Rect _rect;
	uint16 _dest;
	bool _enabled;
};

class MystResourceType5 : public MystResource {
public:
	MystResourceType5(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp();

protected:
	uint16 _scriptCount;
	MystScriptEntry *_scripts;
};

class MystResourceType6 : public MystResourceType5 {
public:
	MystResourceType6(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleAnimation();

protected:
	static Common::String convertMystVideoName(Common::String name);
	Common::String _videoFile;
	uint16 _left;
	uint16 _top;
	uint16 _loop;
	uint16 _u0;
	uint16 _playBlocking;
	uint16 _playOnCardChange;
	uint16 _u3;

private:
	bool _videoRunning;
};

struct MystResourceType7 : public MystResource {
public:
	MystResourceType7(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	virtual ~MystResourceType7() {}

	virtual void drawDataToScreen();
	virtual void handleAnimation();

	virtual void handleMouseUp();
	virtual void handleMouseDown();
	virtual void handleMouseEnter();
	virtual void handleMouseLeave();

protected:
	uint16 _var7;
	uint16 _numSubResources;
	Common::Array<MystResource*> _subResources;
};

class MystResourceType8 : public MystResourceType7 {
public:
	MystResourceType8(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void drawDataToScreen();
	uint16 getType8Var();

protected:
	uint16 _var8;
	uint16 _numSubImages;
	struct SubImage {
		uint16 wdib;
		Common::Rect rect;
	} *_subImages;
};

// No MystResourceType9!

class MystResourceType10 : public MystResourceType8 {
public:
	MystResourceType10(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp();

protected:
	uint16 _kind;
	Common::Rect _rect10;
	uint16 _u0;
	uint16 _u1;
	uint16 _mouseDownOpcode;
	uint16 _mouseDragOpcode;
	uint16 _mouseUpOpcode;
	struct {
		uint16 listCount;
		uint16 *list;
	} _lists[4];
};

class MystResourceType11 : public MystResourceType8 {
public:
	MystResourceType11(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp();

protected:
	uint16 _kind;
	Common::Rect _rect11;
	uint16 _u0;
	uint16 _u1;
	uint16 _mouseDownOpcode;
	uint16 _mouseDragOpcode;
	uint16 _mouseUpOpcode;
	struct {
		uint16 listCount;
		uint16 *list;
	} _lists[3];
};

class MystResourceType12 : public MystResourceType8 {
public:
	MystResourceType12(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleAnimation();
	void handleMouseUp();

protected:
	uint16 _kind;
	Common::Rect _rect11;
	uint16 _state0Frame;
	uint16 _state1Frame;
	uint16 _mouseDownOpcode;
	uint16 _mouseDragOpcode;
	uint16 _mouseUpOpcode;
	struct {
		uint16 listCount;
		uint16 *list;
	} _lists[3];

	uint16 _numFrames;
	uint16 _firstFrame;
	Common::Rect _frameRect;

private:
	bool _doAnimation;
	uint16 _currentFrame;
};

class MystResourceType13 : public MystResource {
public:
	MystResourceType13(MohawkEngine_Myst *vm, Common::SeekableReadStream *rlstStream, MystResource *parent);
	void handleMouseUp();
	void handleMouseEnter();
	void handleMouseLeave();

protected:
	uint16 _enterOpcode;
	uint16 _leaveOpcode;
};

struct MystCursorHint {
	uint16 id;
	int16 cursor;

	MystCondition variableHint;
};

class MohawkEngine_Myst : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_Myst(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_Myst();

	Common::String wrapMovieFilename(Common::String movieName, uint16 stack);

	void reloadSaveList();
	void runLoadDialog();
	void runSaveDialog();

	void changeToStack(uint16 stack);
	void changeToCard(uint16 card);
	uint16 getCurCard() { return _curCard; }
	uint16 getCurStack() { return _curStack; }
	void setMainCursor(uint16 cursor);

	MystVar *_varStore;

	bool _zipMode;
	bool _transitionsEnabled;
	bool _tweaksEnabled;
	bool _needsUpdate;

	MystView _view;
	MystGraphics *_gfx;
	MystSaveLoad *_saveLoad;
	MystScriptParser *_scriptParser;

	bool _showResourceRects;
	void setResourceEnabled(uint16 resourceId, bool enable);

	GUI::Debugger *getDebugger() { return _console; }
	
	bool canLoadGameStateCurrently() { return !(getFeatures() & GF_DEMO); }
	bool canSaveGameStateCurrently() { return !(getFeatures() & GF_DEMO); }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *desc);
	bool hasFeature(EngineFeature f) const;

private:
	MystConsole *_console;
	GUI::SaveLoadChooser *_loadDialog;
	MystOptionsDialog *_optionsDialog;

	uint16 _curStack;
	uint16 _curCard;

	bool _runExitScript;

	void loadCard();
	void unloadCard();
	void runInitScript();
	void runExitScript();

	void loadHelp(uint16 id);

	Common::Array<MystResource*> _resources;
	void loadResources();
	void drawResourceRects();
	void checkCurrentResource();
	int16 _curResource;

	uint16 _cursorHintCount;
	MystCursorHint *_cursorHints;
	void loadCursorHints();
	void checkCursorHints();
	Common::Point _mousePos;
	uint16 _currentCursor;
	uint16 _mainCursor;	// Also defines the current page being held (white, blue, red, or none)
};

} // End of namespace Mohawk

#endif
