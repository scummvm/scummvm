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
#include "mohawk/myst_actions.h"
#include "mohawk/resource_cache.h"
#include "mohawk/video.h"

#include "audio/mixer.h"

#include "common/events.h"
#include "common/random.h"

namespace Mohawk {

class MohawkEngine_Myst;
class VideoManager;
class MystGraphics;
class MystScriptParser;
class MystConsole;
class MystGameState;
struct MystLanguage;
class MystOptionsWidget;
class MystSound;
class MystArea;
class MystAreaImageSwitch;
class MystAreaHover;
class MystCard;

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
enum MystStack {
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
	kStoneshipStack,		// Stoneship Age
	kMenuStack				// Main menu
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

typedef Common::SharedPtr<MystCard> MystCardPtr;
typedef Common::SharedPtr<MystScriptParser> MystScriptParserPtr;

class MohawkEngine_Myst : public MohawkEngine {
protected:
	Common::Error run() override;

public:
	MohawkEngine_Myst(OSystem *syst, const MohawkGameDescription *gamedesc);
	~MohawkEngine_Myst() override;

	Common::SeekableReadStream *getResource(uint32 tag, uint16 id) override;
	Common::Array<uint16> getResourceIDList(uint32 type) const;
	void cachePreload(uint32 tag, uint16 id);

	void changeToStack(MystStack stackId, uint16 card, uint16 linkSrcSound, uint16 linkDstSound);
	void changeToCard(uint16 card, TransitionType transition);
	MystCard *getCard() { return _card.get(); };
	MystCardPtr getCardPtr() { return _card; };
	void setMainCursor(uint16 cursor);
	uint16 getMainCursor() { return _mainCursor; }
	void refreshCursor();
	bool wait(uint32 duration, bool skippable = false);

	/** Update the game state according to events and update the screen */
	void doFrame();

	MystSoundBlock readSoundBlock(Common::ReadStream *stream) const;
	void applySoundBlock(const MystSoundBlock &block);

	bool _showResourceRects;

	VideoManager *_video;
	MystSound *_sound;
	MystGraphics *_gfx;
	MystGameState *_gameState;
	MystScriptParserPtr _stack;
	Common::RandomSource *_rnd;

	MystArea *loadResource(Common::SeekableReadStream *rlstStream, MystArea *parent);
	void redrawResource(MystAreaImageSwitch *resource, bool update = true);

	void setCacheState(bool state) { _cache.enabled = state; }
	bool getCacheState() { return _cache.enabled; }

	VideoEntryPtr playMovie(const Common::String &name, MystStack stack);
	VideoEntryPtr playMovieFullscreen(const Common::String &name, MystStack stack);
	VideoEntryPtr findVideo(const Common::String &name, MystStack stack);
	void playMovieBlocking(const Common::String &name, MystStack stack, uint16 x, uint16 y);
	void playFlybyMovie(MystStack stack);
	void playSkippableMovie(const VideoEntryPtr &video, bool looping);
	void waitUntilMovieEnds(const VideoEntryPtr &video);
	Common::String selectLocalizedMovieFilename(const Common::String &movieName);

	void playSoundBlocking(uint16 id);

	/**
	 * Is the game currently interactive
	 *
	 * When the game is interactive, the user can interact with the game world
	 * and perform other operations such as loading saved games, ...
	 */
	bool isInteractive() const;
	bool isGameStarted() const;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::String getSaveStateName(int slot) const override {
		return Common::String::format("myst-%03d.mys", slot);
	}

	bool hasFeature(EngineFeature f) const override;

	void applyGameSettings() override;
	static Common::Array<Common::Keymap *> initKeymaps(const char *target);

	void resumeFromMainMenu();

	void runOptionsDialog();
	void runCredits();

	bool canDoAction(MystEventAction action);
	void doAction(MystEventAction action);
	void scheduleAction(MystEventAction action);

	static const MystLanguage *getLanguageDesc(Common::Language language);
	Common::Language getLanguage() const override;

private:
	ResourceCache _cache;

	MystScriptParserPtr _prevStack;

	MystCardPtr _card;
	MystCardPtr _prevCard;

	bool hasGameSaveSupport() const;
	void pauseEngineIntern(bool pause) override;

	void goToMainMenu();

	void dropPage();

	Common::String wrapMovieFilename(const Common::String &movieName, uint16 stack);

	void loadStackArchives(MystStack stackId);
	void loadArchive(const char *archiveName, const char *language, bool mandatory);

	// Input
	bool _mouseClicked;
	bool _mouseMoved;
	bool _escapePressed;
	bool _waitingOnBlockingOperation;

	uint16 _currentCursor;
	uint16 _mainCursor; // Also defines the current page being held (white, blue, red, or none)

	Common::Language _currentLanguage;
	MystEventAction _scheduledAction;
};

} // End of namespace Mohawk

#endif
