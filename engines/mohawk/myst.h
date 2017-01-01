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

#ifndef MOHAWK_MYST_H
#define MOHAWK_MYST_H

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/resource_cache.h"
#include "mohawk/myst_scripts.h"

#include "common/events.h"
#include "common/random.h"

namespace Mohawk {

class MohawkEngine_Myst;
class VideoManager;
class MystGraphics;
class MystScriptParser;
class MystConsole;
class MystGameState;
class MystOptionsDialog;
class MystArea;
class MystAreaImageSwitch;
class MystAreaHover;

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
	kDebugHelp     = (1 << 8),
	kDebugCache    = (1 << 9)
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

// Transitions
enum TransitionType {
	kTransitionLeftToRight	= 0,
	kTransitionRightToLeft	= 1,
	kTransitionSlideToLeft	= 2,
	kTransitionSlideToRight	= 3,
	kTransitionDissolve		= 4,
	kTransitionTopToBottom	= 5,
	kTransitionBottomToTop	= 6,
	kTransitionSlideToTop	= 7,
	kTransitionSlideToBottom= 8,
	kTransitionPartToRight	= 9,
	kTransitionPartToLeft	= 10,
	kTransitionCopy			= 11,
	kNoTransition			= 999
};

const uint16 kMasterpieceOnly = 0xFFFF;

struct MystCondition {
	uint16 var;
	Common::Array<uint16> values;
};

struct MystSoundBlock {
	struct SoundItem {
		int16 action;
		uint16 volume;
	};

	int16 sound;
	uint16 soundVolume;
	uint16 soundVar;
	Common::Array<SoundItem> soundList;
};

// View Sound Action Type
enum {
	kMystSoundActionConditional  = -4,
	kMystSoundActionContinue     = -1,
	kMystSoundActionChangeVolume = -2,
	kMystSoundActionStop         = -3
	// Other positive values are PlayNewSound of that id
};

// View flags
enum {
	kMystZipDestination = (1 << 0)
};

struct MystView {
	uint16 flags;

	// Image Data
	Common::Array<MystCondition> conditionalImages;
	uint16 mainImage;

	// Sound Data
	MystSoundBlock soundBlock;

	// Script Resources
	enum ScriptResourceType {
		kResourceImage = 1,
		kResourceSound = 2,
		kResourceSwitch = 3,
		kResourceImageNoCache = 4,
		kResourceSoundNoCache = 5
	};

	struct ScriptResource {
		ScriptResourceType type;
		uint16 id;
		uint16 switchVar;
		ScriptResourceType switchResourceType;
		Common::Array<int16> switchResourceIds;
	};
	Common::Array<ScriptResource> scriptResources;

	// Resource ID's
	uint16 rlst;
	uint16 hint;
	uint16 init;
	uint16 exit;
};

struct MystCursorHint {
	uint16 id;
	int16 cursor;

	MystCondition variableHint;
};

class MohawkEngine_Myst : public MohawkEngine {
protected:
	Common::Error run() override;

public:
	MohawkEngine_Myst(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_Myst();

	Common::SeekableReadStream *getResource(uint32 tag, uint16 id) override;
	Common::Array<uint16> getResourceIDList(uint32 type) const;

	Common::String wrapMovieFilename(const Common::String &movieName, uint16 stack);

	void changeToStack(uint16 stack, uint16 card, uint16 linkSrcSound, uint16 linkDstSound);
	void changeToCard(uint16 card, TransitionType transition);
	uint16 getCurCard() { return _curCard; }
	uint16 getCurStack() { return _curStack; }
	void setMainCursor(uint16 cursor);
	uint16 getMainCursor() { return _mainCursor; }
	void checkCursorHints();
	MystArea *updateCurrentResource();
	void pollAndDiscardEvents();
	bool skippableWait(uint32 duration);

	MystSoundBlock readSoundBlock(Common::ReadStream *stream) const;
	void applySoundBlock(const MystSoundBlock &block);

	bool _needsUpdate;
	bool _needsPageDrop;
	bool _needsShowMap;
	bool _needsShowDemoMenu;
	bool _needsShowCredits;

	bool _showResourceRects;

	Sound *_sound;
	MystGraphics *_gfx;
	MystGameState *_gameState;
	MystScriptParser *_scriptParser;
	Common::Array<MystArea *> _resources;
	Common::RandomSource *_rnd;

	MystArea *loadResource(Common::SeekableReadStream *rlstStream, MystArea *parent);
	void setResourceEnabled(uint16 resourceId, bool enable);
	void redrawArea(uint16 var, bool update = true);
	void redrawResource(MystAreaImageSwitch *resource, bool update = true);
	void drawResourceImages();
	void drawCardBackground();
	uint16 getCardBackgroundId();

	template<class T>
	T *getViewResource(uint index);

	void setCacheState(bool state) { _cache.enabled = state; }
	bool getCacheState() { return _cache.enabled; }

	GUI::Debugger *getDebugger() override { return _console; }

	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc) override;
	bool hasFeature(EngineFeature f) const override;

private:
	MystConsole *_console;
	MystOptionsDialog *_optionsDialog;
	MystScriptParser *_prevStack;
	ResourceCache _cache;
	void cachePreload(uint32 tag, uint16 id);

	uint16 _curStack;
	uint16 _curCard;
	MystView _view;

	bool _runExitScript;

	/**
	 * Saving / Loading is only allowed from the main event loop
	 */
	bool _canSafelySaveLoad;
	bool hasGameSaveSupport() const;

	bool pollEvent(Common::Event &event);

	void dropPage();

	void loadCard();
	void unloadCard();
	void runInitScript();
	void runExitScript();

	void loadHelp(uint16 id);

	void loadResources();
	void drawResourceRects();
	void checkCurrentResource();
	int16 _curResource;
	MystAreaHover *_hoverResource;

	Common::Array<MystCursorHint> _cursorHints;
	void loadCursorHints();
	uint16 _currentCursor;
	uint16 _mainCursor; // Also defines the current page being held (white, blue, red, or none)
};

template<class T>
T *MohawkEngine_Myst::getViewResource(uint index) {
	T *resource = dynamic_cast<T *>(_resources[index]);

	if (!resource) {
		error("View resource '%d' has unexpected type", index);
	}

	return resource;
}

} // End of namespace Mohawk

#endif
