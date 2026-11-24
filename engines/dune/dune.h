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

#ifndef DUNE_H
#define DUNE_H

#include "dune/archive.h"
#include "dune/animation_clock.h"
#include "dune/graphics.h"
#include "dune/detection.h"
#include "dune/condit.h"
#include "dune/dialogue.h"
#include "dune/font.h"
#include "dune/flight_step_clock.h"
#include "dune/map.h"
#include "dune/map_animation.h"
#include "dune/map_globe.h"
#include "dune/pointer_buttons.h"
#include "dune/speech_cues.h"

#include "audio/mixer.h"

#include "common/array.h"
#include "common/events.h"
#include "common/random.h"
#include "common/rect.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace Dune {

// Separate filenames keep the original entry checkpoints out of numbered logs.
enum { kLastPlaceSaveSlot = 100, kNewSietchSaveSlot = 101 };

class SoundEffectStream;

class Console;
class HnmPlayer;
class MusicPlayer;
class SpriteSheet;

enum HNMVideos {
	HNM_DFL2,
	HNM_MNT1,
	HNM_MNT2,
	HNM_MNT3,
	HNM_MNT4,
	HNM_SIET,
	HNM_PALACE,
	HNM_PALACE2,
	HNM_FORT,
	HNM_FORT2,
	HNM_DEAD3,
	HNM_DEAD,
	HNM_DEAD2,
	HNM_VER,
	HNM_TITLE,
	HNM_MTG1,
	HNM_MTG2,
	HNM_MTG3,
	HNM_PLANT,
	HNM_CREDITS,
	HNM_VIRGIN,
	HNM_CRYO,
	HNM_CRYO2,
	HNM_PRESENT,
	HNM_IRULAN,
	HNM_SEQA,
	HNM_SEQL,
	HNM_SEQM,
	HNM_SEQP,
	HNM_SEQG,
	HNM_SEQJ,
	HNM_SEQK,
	HNM_SEQI,
	HNM_SEQD,
	HNM_SEQN,
	HNM_SEQR
};

enum class Cursor : uint16 {
	None = 0,
	Pointer = 0x2584,
	Hand = 0x25c8,
	North = 0x260c,
	East = 0x2650,
	South = 0x2694,
	West = 0x26d8
};

// The panel the game is played through, built out of the sprites of ICONES.HSQ.
struct PanelElement {
	int16  x;
	int16  y;
	int16  x2;
	int16  y2;
	int16  flags;
	uint16 sprite;
	uint16 handler;
};

// The pointer masks only ever ask for black and white, so any other index
// serves to mark the parts of the pointer the screen shows through.
enum {
	kCursorTransparent = 0xff
};

enum {
	kScreenWidth = 320,
	kScreenHeight = 200,
	kSceneHeight = 152
};

// Where the panel keeps the face of whoever is speaking.
// The colour the map screen's palette keeps for what is marked on the sand.
enum {
	kPlaceColour = 12
};

enum {
	kPortraitLeft = 24,
	kPortraitTop = 163,
	kPortraitWidth = 45,
	kPortraitHeight = 33,
	// How many frames of a bank are the mouth moving.
	kMouthFrames = 8
};

// Where the panel keeps what is being said.
enum {
	// The five rows of the panel's middle box, which is where the game puts
	// what there is to choose from.  The panel draws sprite 27 for these rows.
	kMenuLeft = 92,
	kMenuRight = 228,
	kMenuTop = 159,
	kMenuRows = 5,
	kMenuRowHeight = 8,
	kFirstCompassElement = 12,
	kCompassElements = 6,

	// A row is filled from 93 across and one pixel down from its own top,
	// and out to 227 behind the text: see 0xD4AE and 0xD4EC. D48A writes a
	// four-pixel space first, so the command glyphs begin at 97. A row that
	// may be chosen is written in 250 and the heading of
	// a list in 245; the one under the pointer has the two the other way
	// about, dark on light.
	kMenuTextLeft = 93,
	kMenuFillRight = 227,
	kMenuColour = 250,
	kMenuHeadingColour = 245,
	kMenuBackground = 243,

	// What is being said goes over the picture and not in the panel: the box
	// at DS:0x223C is the whole width of the screen and 0x8D69 insets it by
	// sixteen either side and one at the top.  A row is ten pixels, and the
	// block is laid so that its foot comes to 146, which is the number 0x9025
	// takes the height away from.
	kSpeechLeft = 16,
	kSpeechWidth = 288,
	kSpeechBottom = 146,
	kSpeechRowHeight = 10,

	// How wide a space is before a row is stretched to fill the box.
	kSpaceWidth = 6,

	// What a spoken line is written in.
	kSpeechColour = 15,
	// The dark navy inside the map's place-information card.  ONMAP's final
	// fifteen-colour ramp begins here; 102 belongs to the terrain and made the
	// card brown.
	kMapBoxInside = 240,

	// DOS 0xB647 scales latitude by four for the large strategic map.
	kWorldZoom = 4
};

// What became of something the player was watching.
enum PlaybackResult {
	kPlaybackFinished,  // it ran to its end
	kPlaybackSkipped,   // the player asked for the next thing
	kPlaybackAborted    // the player asked to be done with all of it
};

// The pictures the closing narration is read over, which the script does not
// name: after the credits the story is told in eight still screens, each with
// a line of the command file read over it.
enum Narration {
	kNarrStars = 0,   // the stars, and the sun
	kNarrPlanet,      // the planet approaching through them
	kNarrDesert,      // the desert below
	kNarrPaul,        // Paul's face
	kNarrBaron,       // and the Baron's
	kNarrPalace       // the empty palace the story begins in
};

class DuneEngine : public Engine {
	friend class Console;

public:
	DuneEngine(OSystem *syst, const ADGameDescription *desc);
	~DuneEngine() override;

	Common::Error run() override;
	void syncSoundSettings() override;

	bool isCD();

	int _timerTicks;
	Graphics _graphics;

	byte *_framebuffer1;
	byte *_framebuffer2;
	byte *_screen;
	int   _screenBufferSize;

	uint16  _currentBankId;
	byte   *_currentBankData;
	void loadBank(uint16 index);

	SpriteSheet *currentSpriteSheet = nullptr;

// private:
	Common::RandomSource *_rnd;
	const ADGameDescription *_gameDescription;

	Archive  _archive;
	Font     _font;
	Dialogue _dialogue;
	Map      _map;
	// MAP2.HSQ is the prospectors' density survey.  It has the same latitude
	// table and projection as the terrain map, but its low nibbles are spice
	// levels rather than ground types.
	Map      _spiceMap;
	Condit   _condit;

	Common::Array<byte> _state;
	Common::Array<byte> _staticState;
	Common::Array<byte> _dosCode;

	bool loadExecutableData();
	bool loadStaticData(bool newCampaign = true);
	void initialisePlaceSpiceData();

	// The lines everyone speaks, taken from the phrase file of the language
	// the game is being played in.
	// A line of what everyone says, counted from one: 0xCF71 takes one off the
	// number before it looks the line up.  The two files are one run, the
	// second carrying on where the first leaves off.
	Common::String phrase(uint line);
	Common::String phraseOf(uint group, uint line);

	// The same files read from nought, which is what the intro's own numbering
	// has been read as so far.  Whether that numbering really is the line's is
	// still to be settled.
	Common::String phraseByIndex(uint index, bool second = false);

	// What a line comes to once the marks in it are filled in.  A line is not
	// plain text: it carries marks where the game puts a name or a number of
	// its own, and 0x0D where it is to be broken.  Printing them as they stand
	// showed control bytes in the middle of a sentence.
	Common::String fillInLine(const Common::String &raw, uint16 reportTroop = 0);
	Common::String expandDialogueText(const Common::String &raw, uint16 reportTroop, uint depth);
	Common::String expandDialogueWord(uint16 which, uint16 reportTroop, uint depth);

	// The name of a place, which is two of COMMAND1.HSQ's words with a dash
	// between them: the region the place lies in, from its own byte at +0, and
	// the name of the place within it, from the byte at +1.
	Common::String nameOfPlace(const Place &place);
	Common::String kindOfPlaceName(byte kindOfPlace);

	// The words the interface is written with, out of the command file of the
	// language being played in.
	Common::String command(uint index);

	// What an order is called.  The command file names them from its 35th
	// line on, in the order the table of task routines at 0x6C26 has them.
	Common::String orderName(byte order) { return command(35 + order); }
	void dumpResource(const char *filename);
	void dumpScreenPPM(const Common::String &path, const byte *buffer = nullptr);

	// How far the story has come.  The game keeps it in one byte of its own
	// memory, and two hundred of the conditions ask about it.
	byte storyStage() const { return _state.size() > 0x2a ? _state[0x2a] : 0; }

	// The hour of the game, at DS:0x0002.  The original counts the machine's
	// own tick, which it sets to a 5957th of 1193182Hz - a little over 200 a
	// second - and moves the hour on every 12000 of them, which is a minute
	// of real time.  It starts at 2.
	uint16 hour() const;
	// Answers true where the hour has turned, so that the panel can be drawn
	// again: the sun, the moon and the day all stand on it.
	bool tick();
	bool tick(bool dispatchPending);
	bool advanceClock(uint32 elapsedMs);
	bool advanceClockElapsed(uint32 elapsedMs, bool dispatchEachHour, bool dispatchPending = true);
	bool dispatchPendingHour();
	bool dispatchPendingHour(bool localNoticeVoiceStarted);
	bool dispatchHour();
	void advanceWaitHours(int16 hours);
	void dailyRound();
	void refreshPeopleAtPlaces();
	void updateHarkonnenProduction();
	void updateTroopPopulations();
	void updateResultsTotals();
	void updateResultsTrends();
	void strikeASietchIll();
	uint16 chaniCurePlace(uint32 personRecord);
	void letChaniCure();
	void hourlyRound();
	void launchHarkonnenRaid();
	void requestPlaceMapRefresh(uint32 place);
	void growHarkonnenTroops();
	void expandPlaceContext(uint32 place);
	void dailyTroopUpdate(uint troop);
	void convertCapturedFort(uint32 place);
	void removeTroop(uint troop);
	bool mergeSmallTroop(uint troop);
	void decayTroopSkills(uint troop);
	void endTroopStrike(uint32 place);

	// What a troop is worth at its work this hour, as the routine at 0x6EFD
	// works it out.
	byte troopRate(const Troop &troop) const;
	uint16 spiceProduction(uint troop) const;

	// A troop's hour of spice mining, the work order 0 sets it to.
	void mineSpice(uint troop);
	void miningHazard(uint troop);
	void queueTroopNotice(uint16 what, uint16 place);
	void consumeTroopNotice();
	void prospect(uint troop);
	void train(uint troop);
	void spy(uint troop);
	void irrigate(uint troop);
	void plantVegetation(uint32 place);
	void updateVegetationWater();
	void buildWindTrap(uint troop);
	void growBulbs(uint troop);
	void attack(uint troop);
	void massiveAttack();
	void resolveMassiveAttack();
	void waitBattleEffect(uint16 ticks, uint32 &clock, uint32 &lastPoll, bool noDelay);
	bool pollBattleWaitInput(byte &buttons);
	void pollBattleWait(uint32 elapsedMs);
	void advanceBattleAnimation(uint32 elapsedMs);
	void updateGameMusic();
	void updateMusicFade();
	void refreshInputRandom();
	bool battleHere() const;
	bool battleAt(uint place) const;
	byte battleProgress(uint place);
	void updateBattleContext();
	void finishBattleRound();
	void drawBattlefield(bool present, bool restart = false);
	void restoreBattlefield(bool present);
	void drawBattlefieldBackdrop();
	void loadBattleAssets();
	void startBattleAnimation(bool restart = false);
	void stopBattleAnimation();
	enum BattleAnimationUpdate { kBattleNoUpdate, kBattlePaletteUpdate, kBattleActorUpdate };
	BattleAnimationUpdate stepBattleAnimation(bool paused);
	void updateBattleAnimation();
	void spawnBattleActor();
	uint createBattleActor(uint sprite, int x, int y, uint16 velocity);
	void removeBattleActor(uint at);
	void drawBattleActors();
	// B45's actors and counters live in the serialized DOS data segment.
	// The driver-owned flash palette and scheduler remainder need their own
	// save fields. Never serialize the host's absolute clock.
	enum { kMaxBattleActors = (0x46d4 - 0x3cc0) / 17 };
	uint16 _battleAnimationPlace = 0;
	uint16 _battleAnimationElapsed = 0;
	uint32 _battleAnimationTime = 0;
	bool _battleAnimationClockSet = false; // Host poll anchor, including while B45 is unregistered.
	byte _battlePalette[84] = {};
	bool _battleAssetsReady = false;
	byte _battlePalettes[3][84] = {};
	int _battleSpriteWidth[46] = {};
	int _battleSpriteHeight[46] = {};
	void fightHere(bool massive);
	void finishFullDayBattle();
	void refreshBattleMenu();
	bool updateDefeat();
	bool showingDefeat() const;
	void drawDefeatScreen();
	void drawDefeatPortrait(byte reason);
	void showDefeatMenu(bool leaving = false);
	void pollDefeatMenu();
	void playDefeatSpeech();
	void playDefeatMovie(bool restarting);
	void flashDefeatPalette();
	bool waitDefeat(uint32 milliseconds, bool shortenOnInput = false);
	void restartAfterDefeat();
	void playVoice(Resource voice, const Common::String &name, int portraitPerson = -1, bool dispatchAudioHours = true,
			uint32 sampleLimit = 0);
	byte _defeatReason = 0;
	bool _defeatPlaying = false;
	bool _defeatVoiceAlternate = false;
	Common::Array<byte> _defeatScreen;
	byte _defeatPalette[768] = {};
	void attackEnemies(uint troop);
	void counterattackTroop(uint troop);
	void defeatTroop(uint troop);
	void loseBattlePlace(uint32 place);
	void winBattle(uint troop);
	void winBattlePlace(uint32 place);
	void capturePlace(uint32 place);
	void settleBattleTroops(uint32 place, bool convertEnemies);
	void finishBattleControl(uint32 place);
	void unlinkTroop(uint troop);
	byte appendTroop(uint troop, uint32 place);
	uint16 troopStrength(uint troop) const;
	void gatherTroopStrengths(uint32 place);
	void gatherTroopCounts(uint32 place);
	uint16 troopRandom(bool secondary = false);
	byte hourlyEffort(uint troop) const;
	void raiseSkill(uint troop, uint which, byte by);

	// What the player has, at DS:0x00A0.
	uint16 spice() const;

	// What Paul is thought of, at DS:0x0029.  The routine at 0x6F78 adds to
	// it and holds it to 200, which is where the game's own reckoning ends.
	byte charisma() const { return _state.size() > 0x29 ? _state[0x29] : 0; }
	void addCharisma(byte much);

	// The story ends at stage 200: the round at 0x1B19 stops turning the
	// hours once it is reached.
	bool isOver() const { return storyStage() >= 200; }
	void advanceStory(byte stage);
	void prepareIntroSpeaker(int speaker, int xOffset = 0);
	void drawIntroFeydBackground();
	void introScene(uint place, byte kind, uint16 present, int speaker, uint16 line = 0xffff, bool show = true);
	void roomFor(uint place);
	void preparePlaceNameContext(uint16 place);
	void showPicture(uint index, bool present = true);
	PlaybackResult dissolveSceneFrom(const byte *before);
	void keepScreen();
	PlaybackResult applyWipe(int kind);
	PlaybackResult fadePage(const byte *previous, const byte *next, const byte *before = nullptr,
			bool skippable = true);
	// Logical scene pages remain at y=0; intro presentation uses the DOS
	// 0592 viewport without changing retained portrait coordinates.
	Common::Array<byte> _introPage;
	int _introYOffset = 0;
	int _introSpeakerX = 0;
	void copyFrameRectToScreen(const byte *pixels, int pitch, int x, int y, int w, int h);
	void copyPresentedPageToScreen(const byte *page);
	const byte *introPresentationPage(const byte *page, Common::Array<byte> &composed) const;
	byte *_pageBefore;
	byte _paletteBefore[256 * 3];
	const char *nameOfStep(int kind, uint16 what) const;
	void drawList(uint resource, uint32 at, int over = -1, int overX = 0, int overY = 0,
			bool present = true);
	void slideIn(uint resource, int first, int second, int y);
	struct SpeakerSprite { int index, x, y; };
	void collectSpeakerSprites(int person, int frame, int animation, int mapMouth,
			Common::Array<SpeakerSprite> &sprites, uint *frameCountOut = nullptr,
			bool localMouth = false, Common::Rect *mouthWindowOut = nullptr,
			Common::Rect *pictureWindowOut = nullptr);
	void drawSpeakerFigure(int person, int frame, int animation = 0);
	void collectCurrentSpeakerSprites(Common::Array<SpeakerSprite> &sprites);
	void drawCurrentSpeakerFigure();
	bool repaintLocalSpeakerFigure();
	void rememberLocalSpeechPage();
	void finishLocalSpeakerScene();
	bool pollLocalSpeechMouth();
	void prepareLocalSpeakerIdle();
	void prepareLocalSpeakerParagraph();
	void releaseSilentLocalSpeaker();
	void releaseLocalSpeakerForScene();
	bool advanceLocalSpeakerAnimation();
	bool pollLocalSpeakerAnimation(uint16 ticks);
	void updateLocalSpeaker();
	void drawTroopPortrait(int person, int sourceLeft, int sourceTop, int animation, int mouth);
	void drawSpeakerReport(byte *screen = nullptr);
	void completeLocalSpeakerReport();
	int fremenPicture(int person, uint chief = 0) const;
	int localSpeakerPicture() const;
	int fremenAnimation(int person, uint chief = 0) const;
	int localSpeakerVariant() const;
	void selectLocalSpeakerReport(uint16 line);
	void closeLocalSpeakerReport();
	void waitLocalSpeakerReportFrame();
	int paulAnimation() const;
	// One recording to a line.  The voices are kept in sixteen sets, one to a
	// speaker and one of the game's own, and a line is numbered from the
	// speaker's own first line rather than across the whole of the dialogue -
	// see 0xA6E6 and the name 0xA8BC builds.  A speaker of -1 is the game
	// speaking for itself. Reports retain the caller's bank but use the
	// automatic-event group's base and A6F1's offset of 999.
	void playSpeech(uint16 line, int speaker, bool report = false, bool dialogueRequest = true);
	void finishLocalSpeakerBlock();
	void repeatSpeech();
	void stopSpeech();
	int speechMouth() const;
	Audio::SoundHandle _speechHandle;
	void playSoundEffect(uint effect);
	void stopSoundEffect();
	void finishSoundEffectLoop();
	Audio::SoundHandle _soundEffectHandle;
	SoundEffectStream *_soundEffectStream = nullptr;
	uint _soundEffect = 0;
	byte *_spokenLine;
	SpeechCues _speechCues;
	PlaybackResult holdSpeaking(uint32 ms, int speaker);
	bool storyIsOver() const;
	void endStory();
	void endStoryOnReaching(uint16 where);
	void arriveAt(uint index);
	void runStageSetup(byte stage);
	void nearestPlaces(uint from);
	byte placeBearing(uint16 longitude, int16 latitude, uint16 place) const;
	void prepareCaptainContactContext();
	void revealPlace(uint place);

	// The queue of what is waiting to happen, at DS:0x1179.
	void queueEvent(uint16 what);
	void queueEmperorDemand();
	void updateEmperorSchedule();

	// Present a game-owned PHRASE line (as used by the palace alert).
	void sayLine(uint16 packed);
	// DOS 1771 takes a CS event-bytecode offset, not a PHRASE line number.
	void beginEventScript(uint16 cursor);
	void releaseEventScript();
	bool pollEventMenu(uint16 ticks);
	void updateEventMenu();
	uint32 _eventMenuTime = 0;
	uint16 _eventMenuRemainder = 0;

	// What the last hour's mining came to, for the reports that ask.
	uint16 _lastHourMined;
	bool conditionHolds(uint index) const;

	bool runIntro();
	Common::Error runGame();

	// Saving keeps the whole of the game's own memory, which is where all of
	// the story is, and the few things beside it the engine holds: where the
	// player stands, what is being said, and who is saying it.
	bool hasFeature(EngineFeature f) const override;
	int getAutosaveSlot() const override { return -1; }
	Common::String getSaveStateName(int slot) const override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	void pauseEngineIntern(bool pause) override;

	void setCursor(Cursor shape);
	void drawSpeech(const Common::String &text, byte colour, int behind = -1, byte *page = nullptr);
	void drawDefeatText();
	void drawSpeaker();
	void lookAtWorld(int longitude, int latitude);
	Common::Point playerMapPosition() const;
	int placeNearest(int across, int down) const;
	int cabinPlaceAt(const Common::Point &position) const;
	Common::String nameUnderCabin();
	uint16 cabinDestinationAt(const Common::Point &mouse) const;
	Common::String cabinDestinationName(uint16 destination);
	void paintCabinDestination(uint16 destination);
	bool confirmCabinDestination(uint16 destination);
	bool waitCabinConfirmation(uint16 ticks);
	void pollCabinConfirmation(uint16 ticks);
	void boardOrnithopter();
	void enterCabin();
	void showCabin();
	void drawCabinMap(bool partial);
	void repaintCabinLabel();
	void drawCabinLabel();
	void hoverCabinLabel(const Common::Point &mouse);
	void beginCabinInstruction();
	bool pollCabinInstruction(uint16 ticks);
	void updateCabinInstruction();
	void paintCabinMarker();
	bool pollCabinMarker(uint16 ticks);
	void updateCabinMarker();
	Common::Array<byte> _cabinMapPage;
	uint16 _cabinMarkerRemainder = 0;
	uint32 _cabinMarkerTime = 0;
	Common::Point cabinPlayerScreenPosition() const;
	bool cabinPointsAt(const Common::Point &at, int *across, int *down) const;
	bool cabinWorldPointAt(const Common::Point &at, int *longitude, int *latitude) const;
	bool worldPointsAt(const Common::Point &at, int *across, int *down) const;
	bool _inCabin = false;
	bool _cabinFromMap = false;
	// How much of what the cabin asks for has been typed out so far.
	uint _cabinTyped = 0;
	uint16 _cabinTypingRemainder = 0;
	uint32 _cabinTypingTime = 0;
	// -2 retains 4658's instruction, even after its terminating FF.
	int _cabinPointerPoint = -1;
	void openWorldMap();
	bool closeMapTitle();
	bool dismissMapTitle();
	bool pollMapTitle(uint16 idleTicks);
	void drawMapTitle();
	uint mapPlaceCardKind(uint place) const;
	Common::Rect mapPlaceCardBounds(uint place) const;
	void drawMapTooltip(uint place);
	int mapTroopCard() const;
	Common::Rect mapTroopCardBounds() const;
	bool mapInformationContains(const Common::Point &position) const;
	void closeMapTroopCard();
	void rightClickMap(const Common::Point &position);
	void animateMapWindow(const Common::Point &anchor, const Common::Rect &bounds, bool closing = false);
	void animateMapWindowEffect(const Common::Point &anchor, const Common::Rect &bounds, bool closing);
	void showSurveyOpeningTransition(const byte *oldPage);
	void openTroopSurvey();
	void dismissMapInformation();
	void openMapPlaceCard(uint place);
	void prepareMapPlaceCard(uint place, const Common::Rect &bounds);
	void closeMapPlaceCard();
	Common::Point _mapWindowAnchor;
	Common::Rect _mapWindowBounds;
	bool _mapWindowGeometryValid = false;
	void renderMapTroopCard();
	void renderMapTroopCardContents(const Common::Rect &box, uint record, byte textBackground);
	void refreshMapTroopCard(bool updateContext);
	void refreshMapInformation();
	void restoreMapInformationBackground(Common::Rect bounds, bool present = false);
	void drawMapTroopCard();
	Common::Array<byte> _mapTroopCardPixels;
	void showWorld(bool present = true);
	void repaintMapActors(bool present, const Common::Rect *dirty = nullptr);
	void retainMapForegrounds(const byte *visible);
	void showMapEntryTransition(const byte *oldPage, const byte *oldPalette, bool raiseFace = true);
	void showMapReturnView(bool desert);
	void showRoomConversionTransition(const byte *oldPage, const byte *oldPalette);
	Common::Point whereWeStand() const;
	Common::Point placeOnScreen(const Place &place) const;
	void lookAtPlayer();
	void scrollWorld(int way);
	void showMixer();
	void drawOrnithoptersHere(byte kindOfPlace);
	void drawOrnithopter(int x, int y, uint flap);
	void animateOrnithopterTakeoff(byte kindOfPlace);
	void animateOrnithopterApproach(byte kindOfPlace);
	void animateOrnithopterLanding(byte kindOfPlace);
	bool _takingOff = false;
	void teardownStrategicMap(bool animateContact);
	void showGlobe();
	void setGlobeMenu(bool pickingArea, bool present);
	Common::Point playerGlobePosition() const;
	void centreGlobeOnMap();
	void glideToPlayerOnGlobe();
	int _globeSpin = 0;
	int _globeTilt = 0;
	void turnGlobe(int round, int down);
	void workGlobeCompass(int way);
	void drawFresco();
	void drawFrescoGround(bool results);
	void drawFrescoDoors();
	void redrawGlobe();
	void drawGlobePlayerMarker();
	void drawGlobePlayerMarker(int spin);
	bool pollGlobeAnimation(uint16 ticks);
	void updateGlobeAnimation();
	void saveGlobeAnimation(Common::WriteStream *stream) const;
	bool loadGlobeAnimation(Common::ReadStream *stream);
	void restoreCallingGlobe(int spin, int tilt, int doors, byte vision);
	bool _loadingGlobeMenu = false;
	void returnToPlayerMapFromGlobe();
	void showMapFromGlobe(const Common::Point &anchor);
	Common::Array<byte> _globeBackground;
	Common::Array<byte> _globePresented;
	GlobeRenderState _globeDrawing;
	int _globePresentedSpin = 0;
	uint32 _globeAnimationTime = 0;
	void slideFrescoDoors(bool open);
	bool _onGlobe = false;
	int _frescoDoors = 0;
	void showLeaving();
	void showResults();
	void drawResults();
	void drawResults(bool gauges);
	void initialiseResultsGauges();
	void prepareResultsGaugeTargets();
	void refreshResultsGauges();
	void paintResultsGaugeSteps(const byte *previous);
	void restoreResultsPage();
	void restoreResultsPage(byte *page);
	void presentResultsPage();
	Common::Array<byte> _resultsPage;
	bool stepResultsGauges();
	bool pollResultsGauges(uint16 ticks);
	void updateResultsGauges();
	uint32 _resultsGaugeTime = 0;
	uint16 _resultsGaugeRemainder = 0;

	// The mixer panel's list of the ways the music may be played, which the
	// original keeps in the panel's own list while the mixer is up.

	// One of the closing narration's still screens: the picture named by the
	// first word, with the line of the command file named by the second read
	// over it, held until the time the step after it is due.
	PlaybackResult narrate(uint picture, uint16 line, uint32 holdMs);
	bool finishIntroNarration();
	bool waitForDisplayInput(uint32 ms);
	void waitForDisplayFrame(uint32 started, uint32 ms);
	void showMixerMusic();
	void showMixerOrder();
	void pressMixer(const Common::Point &at);
	void drawMixerKnobs();
	bool _onMixer = false;
	bool _mixerOrdering = false;
	int placeAt(const Common::Point &position) const;
	int nearestMapPlace(const Common::Point &position, byte mode, byte kindLimit, int nearest) const;
	uint troopSpriteList(const Troop &troop, uint index) const;
	uint distanceFromPaul(uint16 longitude, int latitude) const;
	void drawMapScenery();
	Common::Rect mapUnitBounds(const MapUnitAnimation &unit) const;
	Common::Rect drawMapUnit(const MapUnitAnimation &unit, const Common::Rect *dirty = nullptr);
	void drawMapUnits(const Common::Rect *dirty = nullptr);
	void syncMapUnits(bool restoreContactRange = true);
	void rebuildMapUnits();
	void replaceMapUnit(uint troop);
	void registerMapUnit(uint troop);
	void removeMapUnit(uint troop);
	void updateTravellingMapUnit(uint troop);
	void paintMapActorChange(Common::Rect bounds);
	void clearMapUnits();
	void appendMapActor(uint key);
	void updateMapActor(uint key, bool framePointersOnly = false);
	void removeMapActor(uint key);
	void startMapRing(uint troop, int x, int y, uint16 &seed);
	void removeMapRing();
	bool hasRemovedTroopContact(uint troop) const;
	bool mapAnimationVisible() const;
	void updateMapAnimation();
	bool pollMapAnimation(uint16 ticks);
	void saveMapAnimations(Common::WriteStream *stream) const;
	bool loadMapAnimations(Common::ReadStream *stream);
	MapUnitAnimation _mapUnits[67], _mapRing;
	Common::Array<uint> _mapUnitOrder;
	Common::Array<uint> _mapActorPoolOrder; // DS:3CC0 order; key67 is the selected ring.
	int _mapRingTroop = -1;
	uint _mapRingOrder = 0; // Position among troop actors in C5CF's registration pool.
	bool _mapAnimationActive = false;
	bool _mapAnimationRedraw = false;
	bool _mapActorNeedsRepaint = false;
	bool _mapRebuildRedraw = false; // 5D82 retains foregrounds; 697C registers last.
	Common::Array<byte> _mapCleanPage; // C412: scenery retained between map rebuilds.
	uint16 _mapAnimationRemainder = 0;
	uint32 _mapAnimationTime = 0;
	int _mapUnitsLongitude = 0, _mapUnitsLatitude = 0;
	bool hasTroopContact() const;
	void animateMap();
	void updateMapSpeech();
	bool pollMapSpeechMouth();
	uint _mapAnimationFrame = 0;
	int _troopContactMouth = -1;
	int _mapSpeechPerson = -1; // A710/9197: voice mouth bank, not the retained head.
	int _troopPortraitKey = -1;
	Common::Array<byte> _troopPortraitClean, _troopPortraitVisible;
	byte _equipmentCardSpare[7] = {}; // Painted counts can outlive the stock scratch at 46FE.
	int _troopContactTop = 0;
	// DNVGA CS:35EA..3600, independent of the compact window's cache.
	Common::Point _troopWindowAnchor;
	Common::Rect _troopWindowBounds;
	bool _troopWindowGeometryValid = false;
	void animateTroopWindow(bool closing = false);
	Common::Array<Common::Rect> _troopRects;
	Common::Point troopOnScreen(const Troop &troop, uint index) const;
	int troopAt(const Common::Point &position, Common::Rect *bounds = nullptr) const;
	bool canSelectMapTroop(uint troop) const;
	int nextMapTroop() const;
	bool mapContactContains(const Common::Point &position) const;
	void updateMapPointer(const Common::Point &position);
	// Which of the panel's right hand box the pointer is over, and turning the
	// course a flight is steered on.
	int compassButtonAt(const Common::Point &at) const;
	int companionAt(const Common::Point &at) const;
	void turnFlight(int by);
	int headingWay() const;
	bool playerWorldPosition(uint16 &longitude, int16 &latitude) const;
	void setFlightCourse();
	void prepareFlightCourse(uint destination);
	void advanceCarriedPosition(bool recordTrail);
	void moveMatchingFollowers(uint16 longitude, uint16 latitude);
	void settleArrivalResidents();
	bool advanceSkippedFlight();
	void checkFlightHazards();
	void clearFlightHazard();
	void setFlightHazardEffect(byte effect);
	bool dispatchCarriedRoomRequests();
	int flightSighting() const;
	int nextFlightSighting(uint &cell) const;
	void updateFlightSteering();
	uint32 _nextFlightTurn = 0;

	// The course a flight is steered on, which the original keeps in the byte
	// at DS:0x11C7, and whether the player has taken the controls, which is
	// DS:0x11C8.
	byte _flightHeading = 0;
	bool _flightSteered = false;

	void takeOffFor(uint index, bool guided = false,
					int targetLongitude = -1, int targetLatitude = 0);
	int wayToFly() const;
	int distanceToFlightTarget() const;
	void flyTo(uint index);
	void flyTo(uint index, bool approach);
	// Which place the flight is bound for, or -1 where the player is afoot.
	int _flyingTo = -1;
	// The last hidden place spotted along the actual route. Recognition
	// reveals it and sets its course before the retained companion report.
	int _guidedFlightPlace = -1;
	bool _flightDiscoveryPrompt = false;
	// A desert click sets a free-flight bearing, not an invisible destination
	// sietch. The aircraft continues on that bearing beyond the chosen point.
	bool _flightToPoint = false;
	uint16 _flightTargetLongitude = 0;
	int16 _flightTargetLatitude = 0;
	bool beginFlightDiscovery();
	bool beginCarriedReport();
	void releaseCarriedReportForSelector();
	bool carriedPortraitVisible() const;
	void showFlightWarning();
	void showFlightReportMenu(bool warning, bool withoutCompanion);
	bool prepareCarriedRoomRequests();
	void showFlightDiscovery();
	uint32 _nextFlightStep = 0;
	// DOS 4F09 advances live flight after 0x300 ticks of its 200 Hz timer;
	// keep scripted travel and retained native intervals at the same period.
	enum { kFlightStepMs = FlightStepClock::kPeriodMs };
	void describePlace(const Place &place);

	// The five rows of the panel are a list to choose from - the original
	// gives each of them a handler of its own, at 0xD42F to 0xD443.
	void showMenu(const Common::Array<Common::String> &choices,
				  const Common::Array<uint16> *flags = nullptr, bool present = true);
	void showMenu(const Common::Array<Common::String> &choices,
				  const Common::Array<uint16> *flags, bool present, bool armTransition);
	void armPanelMenuTransition();
	void finishPanelMenuTransition();
	Common::Array<byte> _panelMenuOldPage;
	uint _panelMenuDispatchDepth = 0;
	class PanelMenuAction {
		DuneEngine &_engine;
	public:
		PanelMenuAction(DuneEngine &engine);
		~PanelMenuAction();
	};
	void debugMenuIfChanged();
	void showTalkMenu();
	void showTalkMenu(bool present);
	void showRoomMenu(bool present = true);
	void showRoomMenu(bool present, bool restorePage);
	void showMessageList(bool seen);
	void animateMessageConsole(bool scan = false);
	void finishMessageConsole();
	void paintCommunicationsIndicators(uint phase);
	void showMessage(uint which, bool seen);
	void closeMessage(byte request);
	void showMessageMenu();
	void prepareShipmentOptions();
	void showShipmentResponse();
	bool checkSpiceShipment();
	void sendSpiceShipment();
	void playSpiceShipment();
	void showPalacePlan();
	void drawPalacePlan();
	void showMirror();
	void drawMirror();
	byte musicContextEntry() const;
	Common::String songForHere() const;
	void playTheRightSong();
	bool musicIsOff() const;
	void stopTheSong();
	void orderThePlaylist();
	void shuffleThePlaylist();
	bool playlistDelayElapsed(uint16 ticks);
	void rebasePlaylistWait(uint16 ticks);
	void playNextOfPlaylist(bool fromTheTop);
	Common::String _song;
	// What the place asks for while the one playing is still fading out.
	Common::String _wantedSong;
	uint ornithoptersHere() const;
	void countThingsHere();
	void countThingsHere(bool refreshTroopContext);
	void showMapMenu(bool present = true, bool restoreParent = false);
	void contactMapTroop();
	void showFlightMenu();
	void showDesertMenu(bool present = true, bool restorePage = false);
	bool pollDesertExhaustion();
	void waitDesertExhaustion(uint16 ticks);
	void returnFromDesertExhaustion();
	void showWormMap(bool departing = true);
	void closeWormMap();
	void drawWormMap();
	bool wormMapPointsAt(const Common::Point &at, int *across, int *down) const;
	void takeWormTo(int longitude, int latitude, int place = -1);
	void completeWormDeparture(uint destination);
	void animateWormCall();
	void showPlacePressed(uint index);
	void waitUntil(bool evening);
	int nearestPlace() const;

	// A press of the pointer at a point of the screen, wherever the player is:
	// on the world, out in the open, or in a room.  Kept apart from the loop
	// that reads the events so that a way through the game can be walked
	// without a hand on the mouse.
	void pressAt(const Common::Point &at);
	void pressStrategicMap(const Common::Point &at);

	// Out in the open between one place and the next.  0x3FD2 empties the
	// place the player is at, takes that place's own longitude and latitude,
	// and steps them by the pair the table at DS:0x1454 holds for the way out
	// that was taken; 0x400C then looks at the byte of the world it lands on
	// and 0x409A finds the place standing there, if any.
	void enterDesert(int way);
	void stepThroughDesert(int way);
	void rememberFlightTrailPoint();
	bool groundIsRock() const;
	const char *desertPicture() const;
	Common::String flightLoop() const;
	void selectFlightTerrain();
	bool drawFlightFrame(const Common::String &loop, uint frame);
	byte flightFramePeriod() const;
	bool flightFrameDue(uint32 tick) const;
	void updateFlightMovie();
	void pollFlightMovement();
	bool drawFlightMovieFrame(bool advance = false, bool restart = false);
	void stopFlightMovie();
	void saveFlightMovie(Common::WriteStream *stream);
	bool loadFlightMovie(Common::SeekableReadStream *stream, int64 end, uint32 &elapsed, uint32 &stepRemaining,
		Common::Array<byte> &presentedInset);
	uint _flightFrame = 0;
	uint32 _flightFilmTime = 0;
	uint32 _flightFilmPauseTime = 0;
	uint32 _flightStepPauseTime = 0;
	bool _flightFilmClockSet = false;
	int _flightMovie = -1;
	bool _flightMovieFallback = false;
	// Only a real37F4 journey restart copies the displayed sky target.
	// Decoder reconstruction after loading a checkpoint is not that action.
	bool _flightSkyRestartPending = false;
	uint _flightLoopLastFrame = 0;
	int _approachMovie = -1; // Synchronous destination film, never saved mid-frame.
	void drawDesert(bool showInset = true, bool present = true, bool sceneOnly = false);
	void drawDesertDialogue(bool present = true);
	void refreshDesertEquipment();
	bool loadHarvesterBank();
	void stopDesertHarvester();
	void prepareDesertHarvester();
	void selectHarvesterSequence(uint sequence);
	void paintDesertHarvester();
	bool pollDesertHarvester(uint16 ticks);
	void updateDesertHarvester();
	bool validDesertHarvesterState();
	Common::Array<byte> _harvesterBank, _harvesterBackdrop, _harvesterPage;
	bool _harvesterActive = false;
	bool _restoreHarvesterPage = false;
	uint16 _harvesterRemainder = 0;
	uint32 _harvesterTime = 0;

	// The little map in the corner of the desert, which 0x4B2B draws into the
	// rectangle at DS:0x1492 while the byte at DS:0x4728 is not yet set.
	void drawMapInset();
	void resetFlightInset();
	void paintFlightInset();
	void eraseFlightInsetHead();
	void updateFlightInset();
	bool _flightInsetSet = false; // Camera/backing live in DS:197C/197E and 4C60.
	void placeTheWandering();
	void placeTheWandering(bool prepareMerchant);
	Common::Array<byte> peopleHere();
	Common::Array<byte> peopleHere(bool prepareMerchant);
	void takeUpPerson(uint who);
	void startRoomConversation(uint who);
	void selectLocalSpeaker(uint who, bool preparePortrait);
	void recordPersonConversation(uint who);
	void beginPersonNews(uint person);
	void preparePersonDialogueContext(uint who);
	void preparePersonMusterContext(uint who);
	void rememberWhereTheyStood(uint who, int x, int y);
	void magnifyRoomAroundSpeaker();
	void letGoOfPerson();
	void releaseEventSpeaker();
	void clearPersonDisplay(bool finishConversation);
	void finishRoomConversation();

	// Whoever walks with Paul, which the game keeps as a pair of bytes at
	// DS:0x1152 - 0xFF where the slot is empty - and shows in the two boxes in
	// the foot of the panel.  0x3EC9 carries them along as the player moves.
	bool isFollowing(uint person) const;
	void comeAlong(uint person);
	void comeAlong(uint person, bool recordPrepared);
	void markPersonFollowing(uint person, bool following);
	void enlistTroop(uint32 record);
	// Whose list the panel shows, where that is not who is being spoken to.
	int _listAs = -1;
	void stayBehind(uint person);
	void stayBehind(uint person, bool recordPrepared);
	void carryFollowersAlong();
	void enterDialogue(uint person, uint wanted);

	// Which part of a person's group of eight holds what.
	enum {
		kWhatHeSays  = 0,  // 93DF starts at the first of four news parts
		kMakingOff   = 4,  // what they say to Paul leaving in the middle
		kComingAlong = 5,  // their answer to being asked along
		kStayingBehind = 6 // and to being asked to stay where they are
	};
	int menuRowAt(const Common::Point &position, bool hover = false) const;
	void updateMenuHover(const Common::Point &position, bool buttonEdge);
	// A list longer than the panel is shown a page at a time; the last row
	// turns the page, and this is what it answers as its entry.
	enum { kTurnThePage = -2 };
	bool menuTurnsPages() const;
	int menuEntryOnRow(uint row) const;
	uint _menuFrom = 0;
	void chooseFromMenu(uint row);

	// Menu kinds are serialized by numeric value.
	enum MenuKind {
		kMenuLines,   // what a person has to say
		kMenuTroops,  // who is here to be given orders
		kMenuOrders,  // and what they may be told to do
		kMenuWaiting, // who is waiting to speak to Paul
		kMenuTalk,    // what Paul may say to whoever he is speaking to
		kMenuRoom,    // what there is to do where the player is standing
		kMenuMap,     // what there is to do with the world up
		kMenuFlight,  // where to fly to, once an ornithopter has been taken
		kMenuDesert,  // what there is to do out in the open
		kMenuMixer,   // the mixer panel, which is the game's own settings
		kMenuPlace,   // what pressing a place on the maps offers
		kMenuGlobe,   // what pressing the planet in the panel offers
		kMenuLeaving, // and whether the player really means to leave
		kMenuTroopContact, // one rallied troop's original map-side command console
		kMenuDiscovery, // a companion has spotted a hidden place during a flight
		kMenuEquipment, // equipment carried by that troop and unused at its place
		kMenuProspectorBriefing, // the map handed over by the first prospectors
		kMenuDensity, // the unlocked spice-density view on the ordinary map
		kMenuMoveTroop, // choosing up to three places for a troop's route
		kMenuPalaceGathering, // the first communication-room conversation
		kMenuPalacePlan, // the plan reached through the palace compass's centre
		kMenuMirror, // Paul's reflection and its save/load/game controls
		kMenuNewMessages,
		kMenuSeenMessages,
		kMenuMessageView,
		kMenuShipmentResponse,
		kMenuTroopNotice, // an hourly report from a distant Fremen troop
		kMenuReservedPalaceAlert, // Reserved save value.
		kMenuWormMap, // choosing an arbitrary desert point after calling a worm
		kMenuDefeat,
		kMenuDefeatExit,
		kMenuBook
	};

	// The original logs first readings in CS; keep that ordered list apart
	// from DS and from the dialogue reader. Browsing never executes actions.
	Common::Array<uint16> _bookEntries;
	int _bookPage = -1;
	uint32 _bookFrameTime = 0;
	byte _bookPalette[256 * 3] = {};
	byte _bookTopic = 0;
	void rememberBookRecord(const Dialogue::Record &record, uint speaker);
	void openBook();
	void closeBook();
	void showBookTopic(byte topic);
	void turnBookPage(int direction);
	void drawBook();
	void updateBookCredits();
	void stopBookCredits();
	int drawBookText(const Common::String &text);
	void pressBook(const Common::Point &at);
	uint16 bookMovie() const;

	Common::Array<Common::String> _menu;
	Common::Array<Common::String> _lastDebugMenu;
	Common::Array<uint> _menuParts;
	// Which row of the list the pointer is over, which the game writes dark
	// on light rather than light on dark.  Nothing is under it at -1.
	int _menuUnder = -1;
	bool _paintedTalkMenuSpeaking = false;
	// What each row of the list is: 0xD4BD reads bit 0x4000 for a row that
	// only names something and cannot be chosen, which is written in its own
	// colour, and bit 0x8000 for the one that is picked out, which is written
	// dark on light.
	Common::Array<uint16> _menuFlags;
	MenuKind _menuKind;
	// D2FD restores the actual room/desert commands beneath a follower's
	// retained portrait, not a newly generated list for the advanced story.
	Common::Array<Common::String> _conversationParentMenu;
	Common::Array<uint> _conversationParentParts;
	Common::Array<uint16> _conversationParentFlags;
	MenuKind _conversationParentKind = kMenuRoom;
	uint _conversationParentFrom = 0;
	uint _orderingTroop;
	// Next DIALOGUE part-122 record (or the prospectors' handover phase).
	uint _troopInfoPage = 0;
	bool _troopOrdersFromMap = false;
	// The Carthag-Timin prospectors have handed Paul their density survey.
	// This is engine-side state because DS:0xA is live story scratch: borrowing
	// its 0x10 bit suppresses Jessica's stage-six special-powers dialogue.
	bool _hasSpiceDensityMap = false;
	Common::Array<uint> _moveDestinations;
	bool _routePreviewActive = false;
	// MOVE TROOP pans its framed density survey without moving the large map
	// underneath it. DOS 1980/1982 are a longitude/latitude centre, not a crop.
	int _surveyLongitude = 0;
	int _surveyLatitude = 0;
	// Only the outline moves while held. DS:11C1/11C3 keeps the committed
	// standalone frame origin, so saving never persists a half-finished drag.
	bool _surveyDragging = false;
	Common::Point _surveyDragPosition;
	Common::Point _surveyDragMouse;
	// Three queued place indices per troop, stored as index + 1 so zero means
	// there is no next stop.  Unlike the temporary selection above, this must
	// survive leaving the map and saving the game.
	Common::Array<byte> _troopRoutes;

	void showTroopsHere();
	void showTroopContact(uint troop, bool fromMap);
	void prepareTroopContactContext(uint troop);
	bool updateTroopWorkState(uint troop);
	bool setTroopOrder(uint troop, byte order, bool restart = false);
	void showTroopMenu();
	void showTroopMenu(bool present, bool armTransition);
	void updateTroopMenuTemplate(uint troop);
	void showSettledTroopMenu(bool present, bool armTransition);
	void finishTroopContact();
	void finishTroopContact(bool animate);
	void finishTroopContact(bool animate, bool unwindSurvey);
	void dismissTroopContact(bool keepMap);
	void dismissTroopContact(bool keepMap, bool restoreParent);
	void sayTroopInformation();
	void sayTroopInformation(bool refreshMenu);
	void sayTroopInformation(bool refreshMenu, bool present);
	void drawTroopStatus(uint troop);
	void drawEquipmentCard(uint troop, bool refreshStock = false);
	void drawEquipmentRow(const byte *counts, int x, int y, int bottom, byte ink,
		uint hitTable = 0, Font::Face noneFace = Font::kFaceNormal, byte textBackground = 0);
	bool canOpenTroopNotice(bool allowFirstVision = false) const;
	bool firstDesertVisionReady(uint16 idleTicks) const;
	bool showFirstDesertVision();
	void waitForFirstDesertVision();
	void markAutomaticStoryRecord();
	enum NoticeDispatch { kNoticeDeferred = 0, kNoticeLocal = 1, kNoticeRemote = 2,
		kNoticeWarning4 = 38, kNoticeWarning2 = 40 };
	uint troopNoticeDispatchAction(uint16 idleTicks) const;
	bool updateTroopNoticeDispatch();
	void resetTroopNoticeIdle();
	void showNoticeWarning(uint effect);
	void showNoticeTransition(const byte *oldPage, const byte *oldPalette);
	bool showLocalTroopNotice();
	bool showLocalTroopNoticeAt(uint index, bool clearRequest);
	bool showLocalRoomRequests();
	bool walkLocalRoomRequests();
	void finishRoomDeparture();
	void finishDeferredRoomCompass();
	void finishVehicleBoarding();
	void completeTakeOffFor(uint index, bool guided);
	void completeRoomMove(byte leads, int way);
	static bool dialogueRecordAvailable(const Dialogue::Record &record, byte mask);
	void finishLocalRoomRequest();
	bool _localRoomRequestPending = false;
	int _pendingRoomExit = -1;
	int _pendingRoomCompass = -1;
	byte _pendingRoomLead = 0;
	int _pendingBoardingPlace = -1;
	bool _pendingBoardingGuided = false;
	void consumeCallerNotices(uint caller, uint16 place);
	uint32 _noticeIdleTime = 0;
	bool _noticeWarningActive = false;
	bool _noticeTransitionActive = false;
	void startNoticeCloud();
	void stopNoticeCloud();
	void composeNoticeCloud();
	void applyNoticeCloudPalette();
	bool pollNoticeCloud(uint16 elapsedTicks);
	// DNVGA's two wave words outlive an individual transmission. The clean
	// and visible pages must be distinct: this is a partial, persistent wave.
	uint16 _noticeCloudRow = 0;
	uint16 _noticeCloudPrevious = 5;
	byte _noticeCloudRemainder = 0;
	byte _noticeCloudPhase = 0;
	byte _noticeCloudPalette[64 * 3] = {};
	Common::Array<byte> _noticeCloudSource;
	Common::Array<byte> _noticeCloudVisible;
	int troopNoticeRecord() const;
	bool showPendingTroopNotice();
	uint prepareTroopNoticeCaller(uint16 what, uint16 place);
	void selectTroopNoticePose(bool randomize = true);
	void clearTroopNoticeMenu();
	void drawTroopNotice();
	void drawTroopNoticeMap(uint16 place);
	void dismissTroopNotice();
	bool handleTroopNoticeEvent(const Common::Event &event);
	void advanceTroopNotice(uint32 elapsedMs);
	void updateTroopNotice();
	void pollTroopNotice();
	// DDB0's 3000 ticks expire on underflow, after the next 5-ms tick.
	// Save only the remaining duration, never an absolute host timestamp.
	enum { kTroopNoticeDuration = 15005 };
	uint16 _troopNoticeRemaining = 0;
	uint32 _troopNoticeTime = 0;
	byte _troopNoticeButtons = 0;
	void showWaiting();
	void takeUpWaiting(uint which);
	void showOrders(uint troop);
	void checkTroopCure(uint troop);
	void finishTroopOccupationMenu();
	void giveOrder(uint troop, byte order);
	void startTroopEspionage(uint troop);
	void showEquipment(uint troop);
	void prepareEquipmentCardGeometry();
	Common::Rect equipmentCardRect(bool spare) const;
	void changeEquipment(uint troop, byte item);
	void finishEquipment(bool announce = true);
	void finishEquipment(bool announce, bool presentMenu);
	void finishEquipmentChange(bool announce = true);
	void sayEquipmentRequest(byte request);
	bool sayTroopRequest(byte request);
	bool sayTroopRequest(byte request, bool refreshMenu);
	uint16 prepareTroopRequestContext(uint16 record, byte request);
	bool requestTroopMove(uint16 destination);
	bool requestTroopMove(uint16 destination, bool refreshMenu);
	void findTroopEquipment(uint troop);
	bool takeLocalEquipment(uint troop, byte item);
	uint16 findEquipmentSupplier(uint troop, byte item);
	uint equipmentSpare(uint32 place, byte item) const;
	int equipmentAt(uint troop, const Common::Point &at) const;
	void showProspectorBriefing();
	void showProspectorBriefing(bool armTransition);
	uint16 prepareProspectorBriefingLine();
	void preparePalaceGathering(uint step);
	void continuePalaceGathering(uint step);
	void showStageEventMenu(bool withWhat, bool armTransition = false);
	bool isStageEventMenu() const;
	bool prepareStageEventParagraph(uint who);
	void continueStageEvent();
	void restoreChaniSequence();
	void drawChaniKiss(uint pose);
	void drawChaniKissCard(uint pose);
	void spiceDensityColours(byte *colours) const;
	void drawSpiceDensityCard();
	void refreshSurveyWindow();
	void prepareSurveyWindow();
	void closeSurveyWindow();
	void drawSurveyLegend();
	void resetSurveyHover();
	void drawSurveySprite(int sprite, int x, int y);
	void drawSurveyMarker(bool troopMarker);
	void drawTroopRoutePreview();
	void showDensityMap();
	void updateSurveyTerritory();
	void showMoveTroopMenu(bool resetView = true);
	void restoreTroopRouteReport();
	void cancelTroopRouteEditor();
	Common::Rect moveMapRect() const;
	bool hasSurveyMap() const;
	Common::Rect surveyMapRect() const;
	Common::Rect surveyFrameRect() const;
	bool canDragSurvey() const;
	bool beginSurveyDrag(const Common::Point &at);
	void toggleSurveyDragOutline();
	bool handleSurveyDrag(const Common::Event &event);
	void finishSurveyDrag(bool commit);
	void refreshStrategicMap();
	void rebuildStrategicMap();
	int surveyPlaceAt(const Common::Point &at) const;
	int surveyPlaceAt(const Common::Point &at, bool hover) const;
	bool moveMapCloseAt(const Common::Point &at) const;
	bool surveyLegendAt(const Common::Point &at) const;
	int moveMapScrollDirection(const Common::Point &at) const;
	void scrollMoveMap(int way);
	int moveDestinationAt(const Common::Point &at) const;
	void addMoveDestination(const Common::Point &at);
	bool waitForRoutePreview();
	void commitTroopRoute();
	void syncPendingTroopRoute();
	void beginTroopRoute(uint troop);
	void beginTroopRoute(uint troop, uint destination);
	void shiftProspectorRoute();
	void syncProspectorRoute();
	bool advanceTroopTravel(uint troop, uint substeps = 0);
	void finishTroopArrival(uint troop);
	void mobilizeBattleTroops(uint place);
	void sayChosenLine();
	void completeDialogueLine();
	void presentDialogueLine();
	void finishDialogueLine();
	void finishWaterOfLife();
	bool _waterOfLifeActive = false;
	void continueRoomConversation();
	void sayNextLine();

	// Shows the next part of what is being said, or answers false where there
	// is none left and the next line is owed instead.
	bool sayRestOfLine();

	// Breaks a line at the marks that say the rest of it waits for a press,
	// and puts the first part up.
	void sayLineInParts(const Common::String &line);
	void sayLineInParts(const Common::String &line, bool forceDisplay);
	void updateSpeechContinuation();
	bool forcedSpeechCaption() const;
	void prepareLocalBalloonCaption();
	bool retainedLocalBalloonCaption(const Common::String &text) const;
	bool drawLocalBalloonCaption(const Common::String &text, byte *page);
	void updateSpeechWordCount();
	void prepareDialogueLine(const Dialogue::Record &record, uint person, uint16 reportTroop = 0);
	void resolveDialogueVoiceVariant(const Common::String &raw, uint depth = 0);
	void runLineAction(const Dialogue::Record &record);
	void talkTo(uint figure);
	bool talkMenuShowsSpeaking() const;
	void drawMenu();
	void drawFace();
	void raiseOpeningPanelFace();
	void lowerPanelFace(bool local = true, bool automatic = false);
	void lowerRetainedPanelFace();
	void raisePanelFace(bool opening = false);
	void paintPanelFacePhase();
	void drawTimeOfDay();
	void drawTimeOfDay(SpriteSheet &icons);
	void refreshPanelClock();
	void resetPanel();
	void drawPanel();
	void drawCompanionPanel(const byte *visible);
	void updateCompanionPanelBlink(uint16 ticks);
	void drawCompass();

	// The box in the panel's right hand corner is drawn out of six entries of
	// the panel's own table, which the game writes over as the player moves
	// between a room, the world, a flight and the globe. See setCompass.
	enum CompassKind {
		kCompassRoom,
		kCompassWorld,
		kCompassFlight,
		kCompassGlobe
	};

	void setCompass(CompassKind kind);

	// Lays the four ways out of the room the player is in into the compass,
	// which is what 0x2FFB does: an entry whose way is shut is marked 0x20 and
	// passed over when the panel is drawn.
	void updateCompass();

	// The five bytes the game keeps for a room: what the room looks like, and
	// then the room each of the four ways out of it leads to, north, east,
	// south and west.  0x3EFE finds them - the table of pointers at DS:0x13C4
	// under the kind of place, and five bytes for every room past the first -
	// and returns nothing where the place names no rooms at all.
	const byte *roomRecord(byte kindOfPlace, byte room) const;
	// `present` false leaves what has been drawn in the buffer for a caller
	// that means to draw over it before it is shown.
	void drawScene(bool present = true);
	void drawEndingCard(bool present = true);
	void drawEndingPortrait(int person, int frame, int animation, int down);
	void drawEndingCouple(int down);
	bool loadEndingPortraitBank(bool chani);
	void prepareEndingCouple();
	bool validEndingCoupleState();
	void stopEndingCouple();
	bool paintEndingActor(bool chani, bool initial);
	bool advanceEndingActor(bool chani);
	bool pollEndingCouple(uint16 ticks, bool present = true);
	void updateEndingCouple();
	bool fadeEndingPaletteRange(uint first, uint count, uint frames, bool fadingIn);
	void playEndingTransition();
	void drawEndingCastFrame(uint index, bool present = true);
	void fadeEndingCast(const byte *previous, const byte *next);
	void playEndingCast();
	void drawFinalStill(bool present = true);
	Common::Array<byte> peoplePresent(uint places) const;
	void drawStandingPerson(byte present, uint index, uint16 x, uint16 y,
			uint16 flags, byte palOffset);
	void redraw();

	// Where the player is.  A place is one byte to the game: the high nibble
	// picks the bank of sprites, taken from GENERIC.HSQ onwards, and the low
	// nibble the scene of the scene file.  The routine at 0x3B59 that draws a
	// scene takes it apart exactly this way.
	byte _place;
	Common::String _salFile;

	// The picture a room stands on.  DS0.HSQ and its like are one sprite of
	// 320 by 152 each: the sietches, the palace rooms, the Harkonnen fort and
	// the villages, one picture to a room.
	Common::String _background;

	int _scene;
	bool _onWorld;
	int _worldLongitude = 0;
	int _worldLatitude = 0;
	// Unquantized longitude/latitude updates of an airborne ornithopter, in
	// DOS CS:E40C..E85C ring order. Longitude wraps through int16.
	Common::Array<Common::Point> _flightTrail;

	// Whether the player is out in the open, and which frame of the looping
	// movie the desert is drawn out of is up.
	bool _inDesert = false;
	// Where the player came out of, so that BACK TO STARTING POINT knows.
	uint _cameFromPlace = 0;
	// The side of the place through which Paul entered the desert.  The DOS
	// coordinates keep a fractional step beside the place; retaining the side
	// lets the coarser event-driven walk stop there and then step back inside.
	int _desertEntryWay = -1;
	// Which place the pointer was last over on the map, so that its name is
	// only written again when it moves from one to another.
	int _pointedAtPlace = -1;
	int _pressedPlace = -1;
	// The world point last named in the worm selector, encoded as row * width
	// plus column. It is transient UI state; the selected target itself is kept
	// in the persisted flight target fields below.
	int _wormPointerPoint = -1;
	Common::String _speech;
	// A line may be too much to read at once: 0xFE in it says the rest waits
	// for a press.  These are the parts it breaks into and which of them is up.
	Common::Array<Common::String> _speechParts;
	bool _speechDisplayForced = false; // Derived from the current resource record on load.
	uint _speechPart = 0;
	// A042 skips the selected record's action/mark while a PHRASE page
	// remains. Legacy saves have already applied them on the first page.
	bool _dialogueActionPending = false;
	Common::String _speaker;
	uint _dialoguePart;
	uint _dialogueLine;
	// 9F31/9F8B follower answers leave the ordinary 47BE/47BA news cursor.
	// Retain it while the selected answer serves WHAT and its text pages.
	uint _personNewsPart = 0xffff;
	uint _personNewsLine = 0xffff;
	byte _personNewsMask = 0x80;
	bool _personNewsEnded = false;
	// DS:0x47C2 selects which already-marked records the dialogue reader
	// passes over: ordinary news uses 0x80, answers to requests use 0x20.
	byte _dialogueMask = 0x80;
	int _speakerFrame;
	// Which of the animations at the end of the speaker's own bank the
	// current line selected.  Most lines use the first (the ordinary talking
	// pose); several action-ten lines select a gesture of their own, such as
	// Duncan holding up the spice report.
	int _speakerAnimation = 0;
	// 9F1C selects a short speech block separately from the idle/gesture pose.
	// The block is transient: loading a save stops its recording too.
	bool _speakerVoiceActive = false;
	bool _speakerSpeechMode = false;
	bool _speakerIdleActive = false;
	uint32 _speakerIdleTime = 0; // 200-Hz wall-clock tick at the last 99BE poll.
	uint16 _speakerIdleRemainder = 0; // D9D2's DC6E, modulo 16 ticks.
	int _speakerVoiceAnimation = 0;
	int _speakerVoiceFrame = 0;
	int _speakerVoiceRemaining = 0;
	int _speakerIdleNextFrame = 0;
	int _speakerIdleFrameCount = 0;
	int _speakerIdleRemaining = 0;
	int _speakerIdlePerson = -1;
	int _speakerIdlePicture = -1;
	int _speakerIdleVariant = -1; // 9123/915C's prepared Fremen identity, independent of presence refreshes.
	// 0F13/998E retain two actors in DS:47C4 and DS:4DFC. Their source
	// cursors and expanded part histories are saved with DS, not reselected.
	bool _endingCoupleActive = false;
	uint32 _endingCoupleTime = 0;
	uint16 _endingCoupleRemainder = 0;
	Common::Array<byte> _endingCoupleBanks[2];
	Common::Array<byte> _endingCoupleBackdrop; // C412's shared spare page.
	Common::Array<byte> _endingCouplePage; // Retained visible scene, without panel.
	// 9DE3 keeps distinct clean/visible pages for compound mouth parts.
	// Like the voice handle, this cache is transient and rebuilt on loading.
	Common::Array<byte> _localSpeechClean;
	Common::Array<byte> _localSpeechVisible;
	Common::Rect _localSpeechWindow;
	int _localSpeechPerson = -1;
	int _localSpeechMouth = -1;
	// 900B/9046 builds ordinary subtitles on a transparent page; 9025/908C
	// restore that retained raster, not new glyphs over the current scenery.
	// 8F28/C370 retains a separate-mode raster for the original ICONES balloon.
	Common::Array<byte> _speechCaptionPixels;
	Common::String _speechCaptionText;
	Common::String _localSpeakerBalloonText; // Box committed beneath the current local portrait.
	byte _speechCaptionShadow = 0; // Prepared ink; survives event return and native saves.
	byte _speechCaptionMode = 0; // 0: outlined subtitle, 1: retained balloon.
	Font::Face _speechCaptionFace = Font::kFaceNormal;
	// 9BB1 restores changed pose parts from the room, not the last portrait.
	// Rebuilt by the full scene painter, including after a load or view change.
	Common::Array<byte> _localSpeakerBackdrop;
	Common::Array<SpeakerSprite> _localSpeakerSprites;
	int _localSpeakerPerson = -1;
	// The little value card some speaking animations carry.  One is Duncan's
	// current spice stock, two and three are an improving or falling daily
	// production report.
	byte _speakerReport = 0;
	// 9B09 rewrites the bank's digit offsets once on opening, not on every
	// paint. An accepted purchase can change the price before 9B49 lowers it.
	uint16 _speakerCardValue = 0;
	bool _speakerCardValueValid = false;
	bool _speakerReportLowered = false; // Last 9B49 pose survives until 99BE paints again.
	bool _speakerReportLowering = false;
	Common::String _speakerCardText; // 9AB4 formats stock text once, after opening.

	// Who is speaking, of the seventeen the dialogue is divided between, or
	// -1 when nobody is.  Their figure is drawn over the room.
	int _speakerPerson;
	// Stage setup may announce a change while a character's record is still
	// being put on screen.  The original keeps that game line in its own word;
	// it must not replace the character subtitle and talk menu mid-sentence.
	bool _inStageSetup = false;
	uint32 _nextMouthTime;
	// Keep the host-clock anchor separate from the saved DOS countdown/tick
	// fraction, so loading a journey does not lose partial timer progress.
	uint32 _lastClockTime = 0;
	AnimationClock _animationClock;
	uint32 animationClockMillis() const;
	uint32 animationClockTicks() const;
	uint16 animationElapsedTicks(uint32 previous, uint32 now) const;
	void updateRegisteredAnimations();
	uint32 _hourElapsedMs = 0; // Fractional five-ms tick; DS:46DB holds the countdown.
	byte _portraitBox[kPortraitWidth * kPortraitHeight];
	// Where the people in the room stand, and which figure of PERS.HSQ each
	// of them is.
	struct Standing {
		Common::Point at;
		// Whose figure it is, counting the people the way the dialogue's own
		// groups do.  It used to be halved where it was read, on the belief
		// that it held the number of the sprite instead, which had everybody
		// answering as the person before them - pressing Jessica brought up
		// Duke Leto.
		uint16 who;
		// Which of the chiefs represented by person fifteen this is, or 0xff.
		byte chief;
	};

	Common::Array<Standing> _sceneMarkers;

	// The panel's own table, which the game writes over as it runs: the boxes
	// in the corners take whatever the player is carrying, and the six entries
	// of the right hand box are swapped out for whichever way of moving about
	// is in hand.  It begins as kPanelElements.
	PanelElement _panel[24];
	// Transient uncovered rectangle for the opening/local-talk face animation.
	byte _openingPanelFaceBacking[20 * 23] = {};
	// C446 restores the retained unzoomed room during5A3A's face raise;
	// it must not substitute either the contacted portrait or new map terrain.
	byte _roomPanelFaceBacking[20 * 23] = {};
	bool _roomPanelFaceValid = false;
	bool _retainRoomPanelFace = false;

	Cursor cursorForPosition(const Common::Point &position) const;
	int compassWayAt(const Common::Point &position) const;
	int worldMapScrollDirection(const Common::Point &position) const;
	int mapPanDirection(const Common::Point &position) const;
	int cabinMapScrollDirection(const Common::Point &position) const;
	int globeControlAt(const Common::Point &position) const;
	int repeatCompassDirection(const Common::Point &position) const;
	int standingAt(const Common::Point &position) const;
	void goThroughEdge(Cursor cursor);
	void prepareRoomEntryCheckpoints();
	void saveRoomEntryCheckpoint(int slot);

	Console *_console;
	MusicPlayer *_music;

	HnmPlayer *_video;
	void loadVideo(uint16 videoId);
	void presentMovieFrame();
	PlaybackResult playVideo(uint16 videoId, uint32 holdMs = 0);

	// A movie by the resource it is, rather than by its place in the list of
	// the thirty-four the intro names: the six looping ones the desert is made
	// of are resources like any other and are not in that list.
	PlaybackResult playMovieResource(uint16 resource, bool loop);

	// One frame of a movie, drawn into the page rather than played: the desert
	// is one of the looping movies held still and stepped on a beat of its
	// own.
	void drawMovieFrame(uint16 resource, int frame);
	PlaybackResult pollSkip();
	bool pollWormMovieTimers(uint16 previousButtons, uint16 buttons);
	// Transient hardware latch, also refreshed by blocking playback polls.
	enum { kPointerClickSpace = 1, kPointerClickEnter = 2, kPointerClickKeypadEnter = 4 };
	byte _pointerClickKeys = 0;
	PointerButtons _pointerButtons;
	MapPointerGlide _mapPointerGlide;
	byte _mapPointerPendingButtons = 0;
	Common::Point _mapPointerDistance, _mapPointerFraction;
	uint16 _mapPointerTick = 0;
	byte _mapPointerKeys = 0;
	bool _mapPointerControl = false;
	static int mapPointerKey(Common::KeyCode key);
	bool canMoveMapPointer() const;
	void updateMapKeyboardPointer();
	int _heldDesertKeyWay = -1;
	uint32 _nextDesertWalk = 0;
	byte updatePointerClickKeys(const Common::Event &event);
	bool pollPointerEvent(Common::Event &event, bool &idleSample);
	void resetPointerButtons();
	PlaybackResult playVideoUntil(uint16 videoId, int stopAt);
	uint16 _openMovie;

	PlaybackResult hold(uint32 ms);
	uint32 getMillis() const { return _system->getMillis(); }

	void dumpSAL(const char *fn);
	void openSpriteSheet(const char *filename, bool takeItsPalette = true);
	void drawSprite(int index, int x, int y, uint16 word = 0, byte palOverride = 0);

	// A sprite put down by its middle rather than its top left corner, which
	// is what 0xC343 does: it takes half the width off x and half the height
	// off y before it draws.
	void drawSpriteCentred(int index, int x, int y);

	void applyPalette(Common::ReadStream *reader);

	void loadBank();
	void loadResourceByIndex();

	void applyPaletteBlock(Common::ReadStream *reader, int onlyBelow = 256);

	// The colours the map is painted with - the ramp of the ground and the
	// colour a place is marked in - which ONMAP.HSQ keeps in the two blocks
	// below the eightieth.  The little map in the corner of the desert needs
	// them without the rest of ONMAP's palette, which would put out the sky.
	// 80 leaves the deeper ground ramp alone, 96 takes it too.
	void applyMapColours(int below = 80);

	// Colours straight out of a run of six bit triples, which is how the sky's
	// palettes are kept.
	void applyPalette(int index, int count, const byte *sixBit);

	// The sky a room that looks out of doors stands against.  0x395C works out
	// which of the palettes SKY.HSQ and SKYDN.HSQ keep belongs to the hour,
	// 0x398C lays it down, and 0x38BD tiles the sky itself over the top of the
	// picture.  The wide one is the palace's own balcony, which needs colours
	// for its architecture as well as for the sky.
	int skyPaletteForHour() const;
	int skyPaletteForHour(uint16 hour) const;
	void applySkyPalette(bool wide);
	void restartFlightSkyPalette();
	bool readSkyPalette(bool wide, int selected, byte *run, bool &panel);
	bool loadSkyFadeTarget(bool wide, int selected);
	void retargetSkyFade();
	void startSkyFade(int selected);
	void prepareWaitSkyPalette(uint16 targetHour);
	void finishSkyFade();
	void stepSkyFade();
	bool pollSkyFade(uint16 ticks);
	void updateSkyFade();
	void stopSkyFade();
	// 3916 owns a DA25 registration, not every positive46D7 (D45 uses it too).
	bool _skyFadeActive = false;
	uint16 _skyFadeRemainder = 0;
	uint32 _skyFadeTime = 0;
	// DNVGA keeps targets for ALL channels. An obscured panel or a bank change
	// must not discard targets outside the newly loaded sky range.
	byte _skyFadeTarget[768] = {};
	void drawSky();

	// The picture the first room of a place stands on, which is the place
	// seen from outside.  0x381B takes the kind of place through the table at
	// DS:0x1972 names the first picture and the number of nearby views.
	// The fractional desert-position byte selects a view; indoors it is zero.
	// A village's first picture also depends on its type byte.
	const char *outsidePicture(byte kindOfPlace, int view, byte placeType) const;

	// Which of the five kinds a place is, which 0x5E4F works out from the byte
	// at its +8: a sietch under 0x20, the palace at 0x20, a village under
	// 0x28, a fort under 0x30 and a Harkonnen place above it.
	static int placeClass(byte kindOfPlace);
};

} // End of namespace Dune

#endif
