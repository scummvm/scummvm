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

#ifndef CRYOMNI3D_VERSAILLES_ENGINE_H
#define CRYOMNI3D_VERSAILLES_ENGINE_H

#include "common/events.h"
#include "common/random.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/str.h"

#include "cryomni3d/cryomni3d.h"
#include "cryomni3d/omni3d.h"
#include "cryomni3d/sprites.h"
#include "cryomni3d/wam_parser.h"

#include "cryomni3d/versailles/documentation.h"
#include "cryomni3d/versailles/toolbar.h"
#include "cryomni3d/versailles/dialogs_manager.h"

namespace Graphics {
class ManagedSurface;
struct Surface;
}

namespace CryOmni3D {
struct FixedImageConfiguration;
class ZonFixedImage;
}

namespace CryOmni3D {
namespace Versailles {
struct PlaceStateActionKey {
	unsigned int placeId;
	unsigned int placeState;
	unsigned int actionId;
	PlaceStateActionKey(unsigned int placeId_, unsigned int placeState_, unsigned int actionId_) :
		placeId(placeId_), placeState(placeState_), actionId(actionId_) {}

	bool operator==(const PlaceStateActionKey &other) const {
		return other.placeId == placeId && other.placeState == placeState && other.actionId == actionId;
	}
};

struct PlaceActionKey {
	unsigned int placeId;
	unsigned int actionId;
	PlaceActionKey(unsigned int placeId_, unsigned int actionId_) :
		placeId(placeId_), actionId(actionId_) {}

	bool operator==(const PlaceActionKey &other) const {
		return other.placeId == placeId && other.actionId == actionId;
	}
};
}
}

namespace Common {
template<>
struct Hash<CryOmni3D::Versailles::PlaceStateActionKey> {
	uint operator()(const CryOmni3D::Versailles::PlaceStateActionKey &k) const {
		// placeState shouldn't be greater than 8 and placeId shouldn't be greater than 100
		// originalActionId shouldn't be greater than 65536
		return (k.placeId << 24 | k.placeState << 16) ^ k.actionId;
	}
};
template<>
struct Hash<CryOmni3D::Versailles::PlaceActionKey> {
	uint operator()(const CryOmni3D::Versailles::PlaceActionKey &k) const {
		// placeId shouldn't be greater than 100
		// originalActionId shouldn't be greater than 65536
		return (k.placeId << 16) ^ k.actionId;
	}
};
}

namespace CryOmni3D {
namespace Versailles {

class CryOmni3DEngine_Versailles;

enum AbortCommand {
	AbortNoAbort = 0,
	AbortQuit = 1,
	AbortLoadGame = 2,
	AbortNewGame = 3,
	AbortNextLevel = 5,
	AbortFinished = 6,
	AbortGameOver = 7
};

struct GameVariables {
	enum Var {
		// TODO: make these enum members more correct
		kCollectScore = 0, // OK       // 0
		kUnlockHiddenDoor, // OK
		kAlreadyWent3_19, // OK
		kMedalsDrawerStatus, // OK
		kCurrentTime, // OK
		kGotMedalsSolution, // OK
		kCabinetDrawerStatus, // OK
		kDecipherScore, // OK
		kCollectLampoonArchitecture, // OK
		kGotRevealedPaper, // OK
		kCollectKey, // OK             // 10
		kCollectPortfolio, // OK
		kSketchState, // OK
		kFakeSketchChatState, // OK
		kCollectFood, // OK
		kCollectQuill, // OK
		kStateLampoonReligion, // OK
		kCollectSmallKey3, // OK
		kCollectEngraving, // OK
		kCollectCord, // OK
		kCollectVaubanBlueprint1, // OK// 20
		kCollectVaubanBlueprint2, // OK
		kLadderState, // OK
		kOpenedCurtain, // OK
		kLoweredChandelier, // OK
		kCombedOrangeTree, // OK
		kMaineTalked, // OK
		kUsedBougieAllumee,
		kStateBombe,
		kInkSpilled, // OK
		kCollectedPaperOnTable, // OK  // 30
		kSafeUnlocked, // OK
		//kUselessVar,
		kCollectedPaperInTrunk = 33, // OK
		kBrushColor, // OK
		kUsedScissors, // OK
		kUnlockedAttic, // OK
		kHasPlayedLebrun, // OK
		kWarnedIncomplete,
		kUsedVaubanBlueprint1, // OK
		kUsedVaubanBlueprint2, // OK   // 40
		kSeenMemorandum, // OK
		kCollectScissors, // OK
		kSavedCountdown, // OK
		kMax
	};
};

// For random sounds we set a constant ID and avoid to use it elsewhere
struct SoundIds {
	enum {
		kOrgue = 0,
		kLeb001,
		kMax
	};
};

struct PlaceState {
	typedef void (CryOmni3DEngine_Versailles::*InitFunc)();
	typedef bool (CryOmni3DEngine_Versailles::*FilterEventFunc)(unsigned int *event);

	PlaceState() : initPlace(nullptr), filterEvent(nullptr), docImage(nullptr), state(0) {}
	PlaceState(InitFunc initPlace_, FilterEventFunc filterEvent_, const char *docImage_) :
		initPlace(initPlace_), filterEvent(filterEvent_), docImage(docImage_), state(0) {}

	InitFunc initPlace;
	FilterEventFunc filterEvent;
	const char *docImage;
	unsigned int state;
};

struct LevelInitialState {
	unsigned int placeId;
	double alpha;
	double beta;
};

struct FakeTransitionActionPlace {
	unsigned int actionId;
	unsigned int placeId;
};

typedef void (CryOmni3DEngine_Versailles::*FixedImgCallback)(ZonFixedImage *);

struct MsgBoxParameters {
	int font;
	byte foreColor;
	unsigned int lineHeight;
	unsigned int spaceWidth;
	unsigned int charSpacing;
	unsigned int initialWidth;
	unsigned int incrementWidth;
	unsigned int initialHeight;
	unsigned int incrementHeight;
	unsigned int timeoutChar;
};

class CryOmni3DEngine_Versailles : public CryOmni3DEngine {
	friend class Versailles_DialogsManager;
protected:
	Common::Error run() override;

public:
	CryOmni3DEngine_Versailles(OSystem *syst, const CryOmni3DGameDescription *gamedesc);
	virtual ~CryOmni3DEngine_Versailles();

	Common::String prepareFileName(const Common::String &baseName, const char *extension) const {
		const char *const extensions[] = { extension, nullptr };
		return prepareFileName(baseName, extensions);
	}
	virtual Common::String prepareFileName(const Common::String &baseName,
	                                       const char *const *extensions) const override;

	void setupPalette(const byte *colors, uint start, uint num) override { setupPalette(colors, start, num, true); }
	void makeTranslucent(Graphics::Surface &dst, const Graphics::Surface &src) const override;

	virtual bool displayToolbar(const Graphics::Surface *original) override { return _toolbar.displayToolbar(original); };
	virtual bool hasPlaceDocumentation() override;
	virtual bool displayPlaceDocumentation() override;
	virtual unsigned int displayOptions() override;
	virtual bool shouldAbort() override { return g_engine->shouldQuit() || _abortCommand != AbortNoAbort; }

private:
	void setupFonts();
	void setupSprites();
	void loadCursorsPalette();
	void calculateTransparentMapping();
	void setupMessages();
	void setupObjects();
	void setupDialogVariables();
	void setupImgScripts();
	void setupPaintingsTitles();

	void syncOmni3DSettings();
	void syncSoundSettings();

	void playTransitionEndLevel(int level);
	void changeLevel(int level);
	void initNewLevel(int level);
	void setupLevelWarps(int level);
	void initPlacesStates();
	void initWhoSpeaksWhere();
	void initDocPeopleRecord();
	void setupLevelActionsMask();

	unsigned int currentGameTime() const { return _gameVariables[GameVariables::kCurrentTime]; }
	void setGameTime(unsigned int newTime, unsigned int level);
	void updateGameTimeDialVariables();

	void gameStep();
	void doGameOver();

	void setPlaceState(unsigned int placeId, unsigned int newState);
	void doPlaceChange();
	void executeTransition(unsigned int nextPlaceId);
	void fakeTransition(unsigned int dstPlaceId);
	unsigned int determineTransitionAnimation(unsigned int srcId, unsigned int dstId,
	        const Transition **transition);

	unsigned int getFakeTransition(unsigned int actionId) const;
	void fixActionId(unsigned int *actionId) const;

	int handleWarp();
	bool handleWarpMouse(unsigned int *actionId, unsigned int movingCuror);
	void animateWarpTransition(const Transition *transition);
	void redrawWarp();

	void handleFixedImg(const FixedImgCallback &callback);
	void executeSeeAction(unsigned int actionId);

	void executeSpeakAction(unsigned int actionId);
	void setupDialogShows();
	bool preprocessDialog(const Common::String &sequence);
	void postprocessDialog(const Common::String &sequence);

	void executeDocAction(unsigned int actionId);

	void drawMenuTitle(Graphics::ManagedSurface *surface, byte color);
	unsigned int displayFilePicker(const Graphics::Surface *bgFrame, bool saveMode,
	                               Common::String &saveName);
	unsigned int displayYesNoBox(Graphics::ManagedSurface &surface, const Common::Rect &position,
	                             unsigned int msg_id);
	void displayMessageBox(const MsgBoxParameters &params, const Graphics::Surface *surface,
	                       unsigned int msg_id, const Common::Point &position,
	                       const Common::Functor0<void> &callback) { displayMessageBox(params, surface, _messages[msg_id], position, callback); }
	void displayMessageBox(const MsgBoxParameters &params, const Graphics::Surface *surface,
	                       const Common::String &msg, const Common::Point &position,
	                       const Common::Functor0<void> &callback);
	void displayMessageBoxWarp(const Common::String &message);
	void displayMessageBoxWarp(unsigned int msg_id) { displayMessageBoxWarp(_messages[msg_id]); }
	void displayCredits();

	void warpMsgBoxCB();

	bool canVisit() const;
	Common::String getSaveFileName(bool visit, unsigned int saveNum) const;
	void getSavesList(bool visit, Common::Array<Common::String> &saveNames);
	void saveGame(bool visit, unsigned int saveNum, const Common::String &saveName);
	bool loadGame(bool visit, unsigned int saveNum);

	void animateCursor(const Object *object);
	void collectObject(Object *object, const ZonFixedImage *fimg = nullptr,
	                   bool showObject = true);
	void collectObject(unsigned int nameID, const ZonFixedImage *fimg = nullptr,
	                   bool showObject = true) { collectObject(_objects.findObjectByNameID(nameID), fimg, showObject); }
	typedef void (CryOmni3DEngine_Versailles::*DisplayObjectHook)(Graphics::ManagedSurface &surface);
	void displayObject(const Common::String &imgName, DisplayObjectHook hook = nullptr);

	void setMainPaletteColor(byte color, byte red, byte green, byte blue);
	void setupPalette(const byte *colors, uint start, uint num, bool commit);

	bool showSubtitles() const;

	void playInGameVideo(const Common::String &filename, bool restoreCursorPalette = true);

	void loadBMPs(const char *pattern, Graphics::Surface *bmps, unsigned int count);

	unsigned int getMusicId(unsigned int level, unsigned int placeId) const;
	bool musicWouldChange(unsigned int level, unsigned int placeId) const;
	void musicUpdate();
	void musicPause();
	void musicResume();
	void musicStop();
	void musicSetQuiet(bool quiet);

	Common::StringArray _messages;
	static const unsigned int kSpritesMapTable[];
	static const unsigned int kSpritesMapTableSize;
	static const LevelInitialState kLevelInitialStates[];
	static const FakeTransitionActionPlace kFakeTransitions[];
	Common::HashMap<unsigned int, FixedImgCallback> _imgScripts;
	Common::Array<Common::String> _paintingsTitles;

	Toolbar _toolbar;

	byte *_mainPalette;
	byte *_cursorPalette;
	bool _fadedPalette;
	bool _forcePaletteUpdate;
	bool _forceRedrawWarp;

	byte *_transparentPaletteMap;
	unsigned int _transparentSrcStart;
	unsigned int _transparentSrcStop;
	unsigned int _transparentDstStart;
	unsigned int _transparentDstStop;
	unsigned int _transparentNewStart;
	unsigned int _transparentNewStop;

	bool _isPlaying;
	bool _isVisiting;
	AbortCommand _abortCommand;
	unsigned int _loadedSave;

	int _omni3dSpeed;

	unsigned int _currentLevel;
	Versailles_DialogsManager _dialogsMan;

	Omni3DManager _omni3dMan;
	ZonFixedImage *_fixedImage;

	Common::Array<unsigned int> _gameVariables;
	Common::Array<PlaceState> _placeStates;
	Common::HashMap<PlaceStateActionKey, unsigned int> _actionMasks;
	Common::HashMap<PlaceActionKey, Common::String> _whoSpeaksWhere;
	Common::HashMap<unsigned int, const char *> _docPeopleRecord;
	bool _transitionAnimateWarp;
	unsigned int _nextPlaceId;
	WAMParser _wam;
	unsigned int _currentPlaceId;
	const Place *_currentPlace;
	const Image::ImageDecoder *_currentWarpImage;

	const char *_musicCurrentFile;
	Audio::SoundHandle _musicHandle;
	float _musicVolumeFactor;
	static const char *kMusicFiles[8][8];

	Versailles_Documentation _docManager;

	static const MsgBoxParameters kWarpMsgBoxParameters;
	static const MsgBoxParameters kFixedimageMsgBoxParameters;
	static const FixedImageConfiguration kFixedImageConfiguration;

	// Countdown
	void initCountdown();
	void syncCountdown();
	inline bool countDown() { if (_countingDown) { return doCountDown(); } else { return false; } }
	inline void drawCountdown(Graphics::ManagedSurface *surface = nullptr) { if (_countingDown) { doDrawCountdown(surface); } }
	void drawCountdownVideo(unsigned int frameNum) { drawCountdown(); }

	bool _countingDown;
	unsigned int _countdownNextEvent;
	char _countdownValue[6];
	Graphics::ManagedSurface _countdownSurface;
	bool doCountDown();
	void doDrawCountdown(Graphics::ManagedSurface *surface);

	// Objects
	template<unsigned int ID>
	void genericDisplayObject();
	void obj_105();
	void obj_106();
	void obj_107();
	void obj_118();
	void obj_121();
	void obj_125();
	void obj_126();
	void obj_126hk(Graphics::ManagedSurface &surface);
	void obj_142();
	void obj_142hk(Graphics::ManagedSurface &surface);

	// Fixed image
	template<unsigned int ID>
	void genericDumbImage(ZonFixedImage *fimg);
	template<unsigned int ID>
	void genericPainting(ZonFixedImage *fimg);
#define IMG_CB(name) void img_ ## name(ZonFixedImage *fimg)
	IMG_CB(31101);
	IMG_CB(31101b);
	IMG_CB(31142);
	IMG_CB(31142b);
	IMG_CB(31142c);
	IMG_CB(31142d);
	IMG_CB(31143);
	IMG_CB(31143b);
	IMG_CB(31143c);
	IMG_CB(31143d);
	IMG_CB(32120);
	IMG_CB(32120b);
	IMG_CB(32120c);
	IMG_CB(32201);
	IMG_CB(32202);
	IMG_CB(32203);
	IMG_CB(32204);
	IMG_CB(32204b);
	IMG_CB(34131);
	IMG_CB(34132);
	IMG_CB(34172);
	IMG_CB(34173);
	IMG_CB(34173b);
	IMG_CB(34173c);
	IMG_CB(34174);
	IMG_CB(34174b);
	IMG_CB(34174c);
	IMG_CB(34174d);
	IMG_CB(34174e);
	IMG_CB(34174f);
	static const unsigned int kSafeDigitsCount = 12;
	static const unsigned int kSafeDigitsX[];
	static const unsigned int kSafeDigitsY[];
	static const char *kSafeDates[];
	bool handleSafe(ZonFixedImage *fimg);
	void drawSafeDigits(Graphics::ManagedSurface &surface, const Graphics::Surface(&bmpDigits)[10],
	                    const unsigned char (&safeDigits)[kSafeDigitsCount]);
	bool checkSafeDigits(unsigned char (&safeDigits)[kSafeDigitsCount]);

	IMG_CB(41202);
	IMG_CB(41202b);
	IMG_CB(41801);
	IMG_CB(41801b);
	IMG_CB(41801c);
	IMG_CB(41802);
	IMG_CB(41802b);
	IMG_CB(41802c);
	IMG_CB(41802d);
	IMG_CB(43143);
	IMG_CB(43143b);
	IMG_CB(43145);
	IMG_CB(43145b);
	IMG_CB(43145c);
	IMG_CB(43146);
	IMG_CB(43146b);
	IMG_CB(43146c);
	IMG_CB(43160);
	IMG_CB(43160b);
	IMG_CB(43160c);
	IMG_CB(43160d);
	IMG_CB(43190);
	IMG_CB(43190b);
	IMG_CB(43190c);
	IMG_CB(43190d);
	IMG_CB(43190e);
	IMG_CB(43190f);
	IMG_CB(44071);
	IMG_CB(44071b);
	IMG_CB(44161);
	IMG_CB(44161b);
	IMG_CB(44161c);
	IMG_CB(44161d);
	IMG_CB(44161e);
	IMG_CB(44161f);
	IMG_CB(45130);
	IMG_CB(45270);
	IMG_CB(45270b);
	IMG_CB(45270c);
	IMG_CB(45270d);
	IMG_CB(45280);
	static const unsigned int kEpigraphMaxLetters = 32;
	static const char *kEpigraphContent;
	static const char *kEpigraphPassword;
	bool handleEpigraph(ZonFixedImage *fimg);
	void drawEpigraphLetters(Graphics::ManagedSurface &surface,
	                         const Graphics::Surface(&bmpLetters)[26], const Common::String &letters);

	IMG_CB(88001);
	IMG_CB(88001b);
	IMG_CB(88001c);
	IMG_CB(88002);
	IMG_CB(88004);
	IMG_CB(88004b);
#undef IMG_CB

#define FILTER_EVENT(level, place) bool filterEventLevel ## level ## Place ## place(unsigned int *event)
#define INIT_PLACE(level, place) void initPlaceLevel ## level ## Place ## place()
	FILTER_EVENT(1, 1);
	FILTER_EVENT(1, 2);
	INIT_PLACE(1, 3);
	FILTER_EVENT(1, 3);
	//FILTER_EVENT(1, 7); // Not used
	FILTER_EVENT(1, 14);

	FILTER_EVENT(2, 1);
	FILTER_EVENT(2, 2);
	FILTER_EVENT(2, 5);
	INIT_PLACE(2, 9);
	FILTER_EVENT(2, 9);
	FILTER_EVENT(2, 11);
	FILTER_EVENT(2, 12);
	FILTER_EVENT(2, 14);

	FILTER_EVENT(3, 3);
	FILTER_EVENT(3, 10);
	FILTER_EVENT(3, 13);
	FILTER_EVENT(3, 15);
	FILTER_EVENT(3, 17);
	FILTER_EVENT(3, 18);
	FILTER_EVENT(3, 19);
	FILTER_EVENT(3_5, 20);
	FILTER_EVENT(3, 22);
	FILTER_EVENT(3, 23);
	bool filterEventLevel3Obj23151();
	void collectLampoonArchitecture(const ZonFixedImage *fimg = nullptr);

	INIT_PLACE(4, 9);
	FILTER_EVENT(4, 10);
	FILTER_EVENT(4, 12_13_14);
	FILTER_EVENT(4, 15);
	FILTER_EVENT(4, 16);
	FILTER_EVENT(4, 17);

	INIT_PLACE(5, 6);
	FILTER_EVENT(5, 9);
	FILTER_EVENT(5, 14);
	FILTER_EVENT(5, 15);
	FILTER_EVENT(5, 16);
	void filterEventLevel5UpdatePlaceStates();
	//FILTER_EVENT(3_5, 20);
	FILTER_EVENT(5, 23);
	FILTER_EVENT(5, 27);
	FILTER_EVENT(5, 28);
	FILTER_EVENT(5, 29);
	FILTER_EVENT(5, 33);
	FILTER_EVENT(5, 34);

	FILTER_EVENT(6, 1);
	FILTER_EVENT(6, 3);
	FILTER_EVENT(6, Orangery);
	FILTER_EVENT(6, 19);

#undef FILTER_EVENT
#undef INIT_PLACE

	// Dialogs shows
	void dialogShowBontempsShowThird();
	void dialogShowHuissierShowPamphlet();
	void dialogShowMonseigneurSorts();
	void dialogShowLeBrunWatches();
	void dialogShowDoorsOpen();
	void dialogShowSwissGuardGives();
	void dialogShowLullyCorrects();
	void dialogShowBontempsGivesAuth();
	void dialogShowCroissyLeave();
	void dialogShowMaintenonGives();
	void dialogShowLaChaizeGivesBack();
	void dialogShowLaChaizeWrites();
	void dialogShowLaChaizeGivesPamphlet();
	void dialogShowBontempsGivesKey();
	void dialogShowDuMaineLeaves();
	void dialogShowTransitionScene();
	void dialogShowEndOfGame();
	void dialogShowLeBrunGives();
	void dialogShowLeBrunLeave();
};

} // End of namespace Versailles
} // End of namespace CryOmni3D

#endif
