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

#ifndef VCRUISE_RUNTIME_H
#define VCRUISE_RUNTIME_H

#include "graphics/pixelformat.h"

#include "common/hashmap.h"
#include "common/keyboard.h"
#include "common/mutex.h"
#include "common/rect.h"

#include "vcruise/detection.h"

class OSystem;
class MidiDriver;

namespace Common {

class MemoryReadStream;
class RandomSource;
class ReadStream;
class WriteStream;

} // End of namespace Commom

namespace Audio {

class AudioStream;
class SeekableAudioStream;
class LoopingAudioStream;

} // End of namespace Audio

namespace Graphics {

struct PixelFormat;
struct WinCursorGroup;
class ManagedSurface;
class Font;
class Cursor;

} // End of namespace Graphics

namespace Video {

class AVIDecoder;

} // End of namespace Video

namespace Image {

class AniDecoder;

} // End of namespace Image

namespace VCruise {

static const uint kNumDirections = 8;
static const uint kNumHighPrecisionDirections = 256;
static const uint kHighPrecisionDirectionMultiplier = kNumHighPrecisionDirections / kNumDirections;
static const uint kNumStartConfigs = 3;

enum StartConfig {
	kStartConfigCheatMenu,
	kStartConfigInitial,
	kStartConfigAlt,
};

class AudioPlayer;
class CircuitPuzzle;
class MidiPlayer;
class MenuInterface;
class MenuPage;
class RuntimeMenuInterface;
class TextParser;
struct ScriptSet;
struct Script;
struct IScriptCompilerGlobalState;
struct Instruction;
struct RoomScriptSet;
struct SoundLoopInfo;
class SampleLoopAudioStream;
struct AD2044Graphics;

enum GameState {
	kGameStateBoot,							// Booting the game
	kGameStateWaitingForAnimation,			// Waiting for a blocking animation with no stop frame to complete, then resuming script
	kGameStateWaitingForAnimationToDelay,	// Waiting for a blocking animation with no stop frame to complete, then going to delay
	kGameStateWaitingForFacing,				// Waiting for a blocking animation with a stop frame to complete, then resuming script
	kGameStateWaitingForFacingToAnim,		// Waiting for a blocking animation to complete, then playing _postFacingAnimDef and switching to kGameStateWaitingForAnimation
	kGameStateQuit,							// Quitting
	kGameStateIdle,							// Waiting for input events
	kGameStateDelay,						// Waiting for delay completion time
	kGameStateScript,						// Running a script
	kGameStateScriptReset,					// Resetting script interpreter into a new script
	kGameStateGyroIdle,						// Waiting for mouse movement to run a gyro
	kGameStateGyroAnimation,				// Animating a gyro

	kGameStatePanLeft,
	kGameStatePanRight,

	kGameStateMenu,
};

struct AD2044AnimationDef {
	byte roomID;
	byte lookupID;
	short fwdAnimationID;
	short revAnimationID;
};

struct AnimationDef {
	AnimationDef();

	int animNum;	// May be negative if reversed
	uint firstFrame;
	uint lastFrame;	// Inclusive

	Common::Rect constraintRect;

	Common::String animName;
};

struct RoomDef {
	Common::HashMap<Common::String, AnimationDef> animations;
	Common::HashMap<Common::String, uint> vars;
	Common::HashMap<Common::String, int> values;
	Common::HashMap<Common::String, Common::String> texts;
	Common::HashMap<Common::String, int> sounds;

	Common::String name;
};

struct InteractionDef {
	InteractionDef();

	Common::Rect rect;
	uint16 interactionID;
	uint16 objectType;
};

struct MapScreenDirectionDef {
	Common::Array<InteractionDef> interactions;
};

class MapLoader {
public:
	virtual ~MapLoader();

	virtual void setRoomNumber(uint roomNumber) = 0;
	virtual const MapScreenDirectionDef *getScreenDirection(uint screen, uint direction) = 0;
	virtual void unload() = 0;

protected:
	static Common::SharedPtr<MapScreenDirectionDef> loadScreenDirectionDef(Common::ReadStream &stream);
};

struct ScriptEnvironmentVars {
	ScriptEnvironmentVars();

	uint panInteractionID;
	uint clickInteractionID;
	uint fpsOverride;
	uint lastHighlightedItem;
	uint animChangeFrameOffset;
	uint animChangeNumFrames;
	bool lmb;
	bool lmbDrag;
	bool esc;
	bool exitToMenu;
	bool animChangeSet;
	bool isEntryScript;
	bool puzzleWasSet;
};

struct SfxSound {
	Common::Array<byte> soundData;
	Common::SharedPtr<Common::MemoryReadStream> memoryStream;
	Common::SharedPtr<Audio::SeekableAudioStream> audioStream;
	Common::SharedPtr<AudioPlayer> audioPlayer;
};

struct SfxPlaylistEntry {
	SfxPlaylistEntry();

	uint frame;
	Common::SharedPtr<SfxSound> sample;
	int8 balance;
	int32 volume;
	bool isUpdate;
};

struct SfxPlaylist {
	SfxPlaylist();

	Common::Array<SfxPlaylistEntry> entries;
};

struct SfxData {
	SfxData();

	void reset();
	void load(Common::SeekableReadStream &stream, Audio::Mixer *mixer);

	typedef Common::HashMap<Common::String, Common::SharedPtr<SfxPlaylist> > PlaylistMap_t;
	typedef Common::HashMap<Common::String, Common::SharedPtr<SfxSound> > SoundMap_t;
	PlaylistMap_t playlists;
	SoundMap_t sounds;
};

struct SoundParams3D {
	SoundParams3D();

	uint minRange;
	uint maxRange;

	// Not sure what this does.  It's always shorter than the min range but after many tests, I've been
	// unable to detect any level changes from altering this parameter.
	uint unknownRange;

	void write(Common::WriteStream *stream) const;
	void read(Common::ReadStream *stream);
};


struct SoundCache {
	SoundCache();
	~SoundCache();

	Common::SharedPtr<SoundLoopInfo> loopInfo;

	Common::SharedPtr<Audio::SeekableAudioStream> stream;
	Common::SharedPtr<SampleLoopAudioStream> loopingStream;
	Common::SharedPtr<AudioPlayer> player;

	bool isLoopActive;
};

struct SoundInstance {
	SoundInstance();
	~SoundInstance();

	Common::String name;
	Common::SharedPtr<SoundCache> cache;

	uint id;

	int32 rampStartVolume;
	int32 rampEndVolume;
	int32 rampRatePerMSec;
	uint32 rampStartTime;
	bool rampTerminateOnCompletion;

	int32 volume;
	int32 balance;

	uint effectiveVolume;
	int32 effectiveBalance;

	bool is3D;
	bool isLooping;
	bool isSpeech;
	bool restartWhenAudible;
	bool tryToLoopWhenRestarted;
	int32 x;
	int32 y;

	SoundParams3D params3D;

	uint32 startTime;
	uint32 endTime;
	uint32 duration;
};

struct RandomAmbientSound {
	RandomAmbientSound();

	Common::String name;

	int32 volume;
	int32 balance;

	uint frequency;
	uint sceneChangesRemaining;

	void write(Common::WriteStream *stream) const;
	void read(Common::ReadStream *stream);
};

struct TriggeredOneShot {
	TriggeredOneShot();

	bool operator==(const TriggeredOneShot &other) const;
	bool operator!=(const TriggeredOneShot &other) const;

	uint soundID;
	uint uniqueSlot;

	void write(Common::WriteStream *stream) const;
	void read(Common::ReadStream *stream);
};

struct ScoreSectionDef {
	ScoreSectionDef();

	Common::String musicFileName;	// If empty, this is silent
	Common::String nextSection;
	int32 volumeOrDurationInSeconds;
};

struct ScoreTrackDef {
	typedef Common::HashMap<Common::String, ScoreSectionDef> ScoreSectionMap_t;

	ScoreSectionMap_t sections;
};

struct StartConfigDef {
	StartConfigDef();

	uint disc;
	uint room;
	uint screen;
	uint direction;
};

struct StaticAnimParams {
	StaticAnimParams();

	uint initialDelay;
	uint repeatDelay;
	bool lockInteractions;

	void write(Common::WriteStream *stream) const;
	void read(Common::ReadStream *stream);
};

struct StaticAnimation {
	StaticAnimation();

	AnimationDef animDefs[2];
	StaticAnimParams params;

	uint32 nextStartTime;
	uint currentAlternation;
};

struct FrameData {
	FrameData();

	uint32 frameIndex;
	uint16 areaFrameIndex;
	int8 roomNumber;
	uint8 frameType;	// 0x01 = Keyframe, 0x02 = Intra frame (not used in Schizm), 0x41 = Last frame
	char areaID[4];
};

struct FrameData2 {
	FrameData2();

	int32 x;
	int32 y;
	int32 angle;
	uint16 frameNumberInArea;
	uint16 unknown;	// Subarea or something?
};

struct AnimFrameRange {
	AnimFrameRange();

	uint animationNum;
	uint firstFrame;
	uint lastFrame;	// Inclusive
};

struct InventoryItem {
	InventoryItem();

	Common::SharedPtr<Graphics::Surface> graphic;
	uint itemID;
	bool highlighted;
};

struct Fraction {
	Fraction();
	Fraction(uint pNumerator, uint pDenominator);

	uint numerator;
	uint denominator;
};

enum LoadGameOutcome {
	kLoadGameOutcomeSucceeded,

	kLoadGameOutcomeSaveDataCorrupted,

	kLoadGameOutcomeMissingVersion,
	kLoadGameOutcomeInvalidVersion,
	kLoadGameOutcomeSaveIsTooNew,
	kLoadGameOutcomeSaveIsTooOld,
};

// State that is swapped when switching between characters in Schizm
struct SaveGameSwappableState {
	struct InventoryItem {
		InventoryItem();

		uint itemID;
		bool highlighted;

		void write(Common::WriteStream *stream) const;
		void read(Common::ReadStream *stream);
	};

	struct Sound {
		Sound();

		Common::String name;
		uint id;
		int32 volume;
		int32 balance;

		bool is3D;
		bool isLooping;
		bool tryToLoopWhenRestarted;
		bool isSpeech;

		int32 x;
		int32 y;

		SoundParams3D params3D;

		void write(Common::WriteStream *stream) const;
		void read(Common::ReadStream *stream, uint saveGameVersion);
	};

	SaveGameSwappableState();

	uint roomNumber;
	uint screenNumber;
	uint direction;
	uint disc;
	bool havePendingPostSwapScreenReset;

	uint loadedAnimation;
	uint animDisplayingFrame;
	bool haveIdleAnimationLoop;
	uint idleAnimNum;
	uint idleFirstFrame;
	uint idleLastFrame;

	int musicTrack;

	Common::String scoreTrack;
	Common::String scoreSection;
	bool musicActive;
	bool musicMuteDisabled;

	int32 musicVolume;
	int32 animVolume;

	Common::Array<InventoryItem> inventory;
	Common::Array<Sound> sounds;
	Common::Array<RandomAmbientSound> randomAmbientSounds;
};

struct SaveGameSnapshot {
	struct PagedInventoryItem {
		PagedInventoryItem();

		uint8 page;
		uint8 slot;
		uint8 itemID;

		void write(Common::WriteStream *stream) const;
		void read(Common::ReadStream *stream, uint saveGameVersion);
	};

	struct PlacedInventoryItem {
		PlacedInventoryItem();

		uint32 locationID;
		uint8 itemID;

		void write(Common::WriteStream *stream) const;
		void read(Common::ReadStream *stream, uint saveGameVersion);
	};

	SaveGameSnapshot();

	void write(Common::WriteStream *stream) const;
	LoadGameOutcome read(Common::ReadStream *stream);

	static const uint kSaveGameIdentifier = 0x53566372;
	static const uint kSaveGameCurrentVersion = 10;
	static const uint kSaveGameEarliestSupportedVersion = 2;
	static const uint kMaxStates = 2;

	static Common::String safeReadString(Common::ReadStream *stream);
	static void writeString(Common::WriteStream *stream, const Common::String &str);

	uint hero;
	uint swapOutRoom;
	uint swapOutScreen;
	uint swapOutDirection;
	uint8 inventoryPage;
	uint8 inventoryActiveItem;

	uint numStates;
	Common::SharedPtr<SaveGameSwappableState> states[kMaxStates];

	bool escOn;

	StaticAnimParams pendingStaticAnimParams;
	SoundParams3D pendingSoundParams3D;

	int32 listenerX;
	int32 listenerY;
	int32 listenerAngle;

	Common::Array<TriggeredOneShot> triggeredOneShots;
	Common::HashMap<uint32, uint> sayCycles;

	Common::HashMap<uint32, int32> variables;
	Common::HashMap<uint, uint32> timers;
	Common::Array<PagedInventoryItem> pagedItems;
	Common::Array<PlacedInventoryItem> placedItems;
};

enum OSEventType {
	kOSEventTypeInvalid,

	kOSEventTypeMouseMove,
	kOSEventTypeLButtonDown,
	kOSEventTypeLButtonUp,

	kOSEventTypeKeymappedEvent,
};

enum KeymappedEvent {
	kKeymappedEventNone,

	kKeymappedEventEscape,
	kKeymappedEventHelp,
	kKeymappedEventSaveGame,
	kKeymappedEventLoadGame,
	kKeymappedEventSoundSettings,
	kKeymappedEventQuit,
	kKeymappedEventPause,
	kKeymappedEventMusicToggle,
	kKeymappedEventSoundToggle,
	kKeymappedEventMusicVolumeDown,
	kKeymappedEventMusicVolumeUp,
	kKeymappedEventSoundVolumeDown,
	kKeymappedEventSoundVolumeUp,

	kKeymappedEventSkipAnimation,

	kKeymappedEventPutItem,
};

struct OSEvent {
	OSEvent();

	OSEventType type;
	Common::Point pos;
	KeymappedEvent keymappedEvent;
	uint32 timestamp;
};

struct TextStyleDef {
	Common::String fontName;
	uint size;
	uint unknown1;
	uint unknown2;
	uint unknown3;	// Seems to always be 0 for English, other values for other languages
	uint colorRGB;
	uint shadowColorRGB;
	uint alignment;	// Modulo 10 seems to be alignment: 0 = left, 1 = center, 2 = right
	uint unknown5;	// Possibly drop shadow offset
};

struct UILabelDef {
	Common::String lineID;
	Common::String styleDefID;
	uint graphicLeft;
	uint graphicTop;
	uint graphicWidth;
	uint graphicHeight;
};

struct FontCacheItem {
	FontCacheItem();

	Common::String fname;
	uint size;

	const Graphics::Font *font;
	Common::SharedPtr<Graphics::Font> keepAlive;
};

typedef Common::HashMap<Common::String, uint> ScreenNameToRoomMap_t;
typedef Common::HashMap<uint, ScreenNameToRoomMap_t> RoomToScreenNameToRoomMap_t;

struct AnimatedCursor {
	struct FrameDef {
		uint imageIndex;
		uint delay;
	};

	Common::Array<FrameDef> frames;
	Common::Array<Graphics::Cursor *> images;

	Common::Array<Common::SharedPtr<Graphics::Cursor> > cursorKeepAlive;
	Common::SharedPtr<Graphics::WinCursorGroup> cursorGroupKeepAlive;
};

class Runtime {
public:
	friend class RuntimeMenuInterface;

	enum CharSet {
		kCharSetLatin,
		kCharSetGreek,
		kCharSetCyrillic,
		kCharSetJapanese,
		kCharSetChineseTraditional,
		kCharSetChineseSimplified,
	};

	Runtime(OSystem *system, Audio::Mixer *mixer, MidiDriver *midiDrv, const Common::FSNode &rootFSNode, VCruiseGameID gameID, Common::Language defaultLanguage);
	virtual ~Runtime();

	void initSections(const Common::Rect &gameRect, const Common::Rect &menuRect, const Common::Rect &trayRect, const Common::Rect &subtitleRect, const Common::Rect &fullscreenMenuRect, const Graphics::PixelFormat &pixFmt);

	void loadCursors(const char *exeName);
	void setDebugMode(bool debugMode);
	void setFastAnimationMode(bool fastAnimationMode);
	void setPreloadSounds(bool preloadSounds);
	void setLowQualityGraphicsMode(bool lowQualityGraphicsMode);

	bool runFrame();
	void drawFrame();

	void onLButtonDown(int16 x, int16 y);
	void onLButtonUp(int16 x, int16 y);
	void onMouseMove(int16 x, int16 y);
	void onKeymappedEvent(KeymappedEvent evt);

	bool canSave(bool onCurrentScreen) const;
	bool canLoad() const;

	void recordSaveGameSnapshot();
	void recordSounds(SaveGameSwappableState &state);
	void restoreSaveGameSnapshot();
	Common::SharedPtr<SaveGameSnapshot> generateNewGameSnapshot() const;

	void saveGame(Common::WriteStream *stream) const;
	LoadGameOutcome loadGame(Common::ReadStream *stream);

	bool bootGame(bool newGame);
	static void resolveCodePageForLanguage(Common::Language lang, Common::CodePage &outCodePage, CharSet &outCharSet);

	void drawLabel(Graphics::ManagedSurface *surface, const Common::String &labelID, const Common::Rect &contentRect);
	void getLabelDef(const Common::String &labelID, const Graphics::Font *&outFont, const Common::String *&outTextUTF8, uint32 &outColor, uint32 &outShadowColor, uint32 &outShadowOffset);

	void onMidiTimer();

private:
	enum IndexParseType {
		kIndexParseTypeNone,
		kIndexParseTypeRoom,
		kIndexParseTypeRRoom,	// Rectangle room (constrains animation areas)
		kIndexParseTypeYRoom,	// Yes room (variable/ID mappings)
		kIndexParseTypeVRoom,	// Value room (value/ID mappings?)
		kIndexParseTypeTRoom,	// Text
		kIndexParseTypeCRoom,	// Const
		kIndexParseTypeSRoom,	// Sound
		kIndexParseTypeNameRoom,
	};

	enum AnimDecoderState {
		kAnimDecoderStateStopped,
		kAnimDecoderStatePlaying,
		kAnimDecoderStatePaused,
	};

	struct IndexPrefixTypePair {
		const char *prefix;
		IndexParseType parseType;
	};

	struct RenderSection {
		Common::SharedPtr<Graphics::ManagedSurface> surf;
		Common::Rect rect;
		Graphics::PixelFormat pixFmt;

		void init(const Common::Rect &paramRect, const Graphics::PixelFormat &fmt);
	};

	struct Gyro {
		static const uint kMaxPreviousStates = 3;

		int32 currentState;
		int32 requiredState;
		int32 previousStates[kMaxPreviousStates];
		int32 requiredPreviousStates[kMaxPreviousStates];
		uint numPreviousStates;
		uint numPreviousStatesRequired;
		bool wrapAround;
		bool requireState;

		Gyro();

		void reset();
		void logState();
	};

	struct GyroState {
		GyroState();

		void reset();

		static const uint kNumGyros = 5;

		Gyro gyros[kNumGyros];

		uint completeInteraction;
		uint failureInteraction;
		uint frameSeparation;

		uint activeGyro;
		uint dragMargin;
		uint maxValue;

		AnimationDef negAnim;
		AnimationDef posAnim;
		bool isVertical;

		Common::Point dragBasePoint;
		uint dragBaseState;
		int32 dragCurrentState;
		bool isWaitingForAnimation;
	};

	enum PanoramaCursorFlags {
		kPanCursorDraggableHoriz	= (1 << 0),
		kPanCursorDraggableUp		= (1 << 1),
		kPanCursorDraggableDown		= (1 << 2),

		kPanCursorDirectionUp		= (0 << 3),
		kPanCursorDirectionLeft		= (1 << 3),
		kPanCursorDirectionRight	= (2 << 3),
		kPanCursorDirectionDown		= (3 << 3),

		kPanCursorMaxCount			= (1 << 5),
	};

	enum PanoramaState {
		kPanoramaStateInactive,

		kPanoramaStatePanningUncertainDirection,

		kPanoramaStatePanningLeft,
		kPanoramaStatePanningRight,
		kPanoramaStatePanningUp,
		kPanoramaStatePanningDown,
	};

	enum InGameMenuState {
		kInGameMenuStateInvisible,
		kInGameMenuStateVisible,
		kInGameMenuStateHoveringInactive,
		kInGameMenuStateHoveringActive,
		kInGameMenuStateClickingOver,		// Mouse was pressed on a button and is holding on it
		kInGameMenuStateClickingNotOver,	// Mouse was pressed on a button and dragged off
		kInGameMenuStateClickingInactive,
	};

	enum SoundLoopBehavior {
		kSoundLoopBehaviorNo,
		kSoundLoopBehaviorYes,
		kSoundLoopBehaviorAuto,
	};

	static const uint kPanLeftInteraction = 1;
	static const uint kPanDownInteraction = 2;
	static const uint kPanRightInteraction = 3;
	static const uint kPanUpInteraction = 4;

	static const uint kPanoramaLeftFlag = 1;
	static const uint kPanoramaRightFlag = 2;
	static const uint kPanoramaUpFlag = 4;
	static const uint kPanoramaDownFlag = 8;
	static const uint kPanoramaHorizFlags = (kPanoramaLeftFlag | kPanoramaRightFlag);

	static const uint kNumInventorySlots = 6;
	static const uint kNumInventoryPages = 8;

	typedef int32 ScriptArg_t;
	typedef int32 StackInt_t;

	struct StackValue {
		enum StackValueType {
			kNumber,
			kString,
		};

		union ValueUnion {
			StackInt_t i;
			Common::String s;

			ValueUnion();
			explicit ValueUnion(StackInt_t iVal);
			explicit ValueUnion(const Common::String &strVal);
			explicit ValueUnion(Common::String &&strVal);
			~ValueUnion();
		};

		StackValue();
		StackValue(const StackValue &other);
		StackValue(StackValue &&other);
		explicit StackValue(StackInt_t i);
		explicit StackValue(const Common::String &str);
		explicit StackValue(Common::String &&str);
		~StackValue();

		StackValue &operator=(const StackValue &other);
		StackValue &operator=(StackValue &&other);

		StackValueType type;
		ValueUnion value;
	};

	struct CallStackFrame {
		CallStackFrame();

		Common::SharedPtr<Script> _script;
		uint _nextInstruction;
	};

	struct SubtitleDef {
		SubtitleDef();

		uint8 color[3];
		uint unknownValue1;
		uint durationInDeciseconds;
		Common::String str;
	};

	struct SubtitleQueueItem {
		Common::U32String str;
		uint8 color[3];
		uint32 startTime;
		uint32 endTime;
	};

	bool runIdle();
	bool runDelay();
	bool runHorizontalPan(bool isRight);
	bool runScript();
	bool requireAvailableStack(uint n);
	bool runWaitForAnimation();
	bool runWaitForAnimationToDelay();
	bool runWaitForFacing();
	bool runWaitForFacingToAnim();
	bool runGyroIdle();
	bool runGyroAnimation();
	void exitGyroIdle();
	void continuePlayingAnimation(bool loop, bool useStopFrame, bool &outEndedAnimation);
	void drawSectionToScreen(const RenderSection &section, const Common::Rect &rect);
	void commitSectionToScreen(const RenderSection &section, const Common::Rect &rect);
	void terminateScript();
	void quitToMenu();
	RoomScriptSet *getRoomScriptSetForCurrentRoom() const;
	bool checkCompletionConditions();

	void startTerminatingHorizontalPan(bool isRight);

	bool popOSEvent(OSEvent &evt);
	void queueOSEvent(const OSEvent &evt);

	void processUniversalKeymappedEvents(KeymappedEvent evt);

	void loadReahSchizmIndex();
	void loadAD2044ExecutableResources();
	void findWaves();
	void loadConfig(const char *cfgPath);
	void loadScore();
	void loadDuplicateRooms();
	void loadAllSchizmScreenNames();
	Common::SharedPtr<SoundInstance> loadWave(const Common::String &soundName, uint soundID, const Common::ArchiveMemberPtr &archiveMemberPtr);
	SoundCache *loadCache(SoundInstance &sound);
	void resolveSoundByName(const Common::String &soundName, bool load, StackInt_t &outSoundID, SoundInstance *&outWave);
	SoundInstance *resolveSoundByID(uint soundID);
	void resolveSoundByNameOrID(const StackValue &stackValue, bool load, StackInt_t &outSoundID, SoundInstance *&outWave);

	void changeToScreen(uint roomNumber, uint screenNumber);
	void clearIdleAnimations();
	void changeHero();
	void changeToExamineItem();
	void returnFromExaminingItem();
	bool triggerPreIdleActions();
	void returnToIdleState();
	void changeToCursor(const Common::SharedPtr<AnimatedCursor> &cursor);
	void refreshCursor(uint32 currentTime);
	bool dischargeIdleMouseMove();
	bool dischargeIdleMouseDown();
	bool dischargeIdleClick();
	void loadFrameData(Common::SeekableReadStream *stream);
	void loadFrameData2(Common::SeekableReadStream *stream);
	void loadTabData(Common::HashMap<int, AnimFrameRange> &animIDToFrameRangeMap, uint animNumber, Common::SeekableReadStream *stream);

	void changeMusicTrack(int musicID);
	void startScoreSection();
	void setMusicMute(bool muted);

	void changeAnimation(const AnimationDef &animDef, bool consumeFPSOverride);
	void changeAnimation(const AnimationDef &animDef, uint initialFrame, bool consumeFPSOverride);
	void changeAnimation(const AnimationDef &animDef, uint initialFrame, bool consumeFPSOverride, const Fraction &defaultFrameRate);
	void applyAnimationVolume();

	void setSound3DParameters(SoundInstance &sound, int32 x, int32 y, const SoundParams3D &soundParams3D);
	void triggerSound(SoundLoopBehavior loopBehavior, SoundInstance &sound, int32 volume, int32 balance, bool is3D, bool isSpeech);
	void triggerSoundRamp(SoundInstance &sound, uint durationMSec, int32 newVolume, bool terminateOnCompletion);
	void stopSound(SoundInstance &sound);
	void convertLoopingSoundToNonLooping(SoundInstance &sound);
	void updateSounds(uint32 timestamp);
	void updateSubtitles();
	void update3DSounds();
	bool computeEffectiveVolumeAndBalance(SoundInstance &snd);
	void triggerAmbientSounds();
	uint decibelsToLinear(int db, uint baseVolume, uint maxVolume) const;
	int32 getSilentSoundVolume() const;
	int32 getDefaultSoundVolume() const;
	uint applyVolumeScale(int32 volume) const;
	int applyBalanceScale(int32 balance) const;

	void triggerWaveSubtitles(const SoundInstance &sound, const Common::String &id);
	void stopSubtitles();

	AnimationDef stackArgsToAnimDef(const StackInt_t *args) const;
	void consumeAnimChangeAndAdjustAnim(AnimationDef &animDef);
	void pushAnimDef(const AnimationDef &animDef);

	void activateScript(const Common::SharedPtr<Script> &script, bool isEntryScript, const ScriptEnvironmentVars &envVars);
	Common::SharedPtr<ScriptSet> compileSchizmLogicSet(const uint *roomNumbers, uint numRooms) const;

	bool parseIndexDef(IndexParseType parseType, uint roomNumber, const Common::String &key, const Common::String &value);
	void allocateRoomsUpTo(uint roomNumber);

	void drawDebugOverlay();

	Common::SharedPtr<Script> findScriptForInteraction(uint interactionID) const;

	void detectPanoramaDirections();
	void detectPanoramaMouseMovement(uint32 timestamp);
	void panoramaActivate();

	bool computeFaceDirectionAnimation(uint desiredDirection, const AnimationDef *&outAnimDef, uint &outInitialFrame, uint &outStopFrame);

	void inventoryAddItem(uint item);
	void inventoryRemoveItem(uint item);
	void clearScreen();
	void redrawTray();
	void clearTray();
	void redrawSubtitleSection();
	void clearSubtitleSection();
	void drawSubtitleText(const Common::Array<Common::U32String> &lines, const uint8 (&color)[3]);
	void drawInventory(uint slot);
	void drawCompass();
	bool isTrayVisible() const;
	void resetInventoryHighlights();
	void loadInventoryFromPage();
	void copyInventoryToPage();
	void cheatPutItem();
	static uint32 getLocationForScreen(uint roomNumber, uint screenNumber);
	void updatePlacedItemCache();
	void drawPlacedItemGraphic();
	void clearPlacedItemGraphic();
	void drawActiveItemGraphic();
	void clearActiveItemGraphic();
	void drawInventoryItemGraphic(uint slot);
	void clearInventoryItemGraphic(uint slot);
	void dropActiveItem();
	void pickupPlacedItem();
	void stashActiveItemToInventory(uint slot);
	void pickupInventoryItem(uint slot);

	void getFileNamesForItemGraphic(uint itemID, Common::String &outGraphicFileName, Common::String &outAlphaFileName) const;
	Common::SharedPtr<Graphics::Surface> loadGraphic(const Common::String &graphicName, const Common::String &alphaName, bool required);
	Common::SharedPtr<Graphics::Surface> loadGraphicFromPath(const Common::Path &path, bool required);

	bool loadSubtitles(Common::CodePage codePage, bool guessCodePage);

	void changeToMenuPage(MenuPage *menuPage);

	void checkInGameMenuHover();
	void dismissInGameMenu();
	void dischargeInGameMenuMouseUp();
	void drawInGameMenuButton(uint element);

	const Graphics::Font *resolveFont(const Common::String &textStyle, uint size);

	bool resolveCircuitPuzzleInteraction(const Common::Point &relMouse, Common::Point &outCoord, bool &outIsDown, Common::Rect &outHighlightRect) const;
	void clearCircuitPuzzle();
	void clearCircuitHighlightRect(const Common::Rect &rect);
	void drawCircuitHighlightRect(const Common::Rect &rect);
	static Common::Rect padCircuitInteractionRect(const Common::Rect &rect);

	static Common::SharedPtr<AnimatedCursor> winCursorGroupToAnimatedCursor(const Common::SharedPtr<Graphics::WinCursorGroup> &cursorGroup);
	static Common::SharedPtr<AnimatedCursor> aniFileToAnimatedCursor(Image::AniDecoder &aniDecoder);
	static Common::SharedPtr<AnimatedCursor> staticCursorToAnimatedCursor(const Common::SharedPtr<Graphics::Cursor> &cursor);

	// Script things
	void scriptOpNumber(ScriptArg_t arg);
	void scriptOpRotate(ScriptArg_t arg);
	void scriptOpAngle(ScriptArg_t arg);
	void scriptOpAngleGGet(ScriptArg_t arg);
	void scriptOpSpeed(ScriptArg_t arg);
	void scriptOpSAnimL(ScriptArg_t arg);
	void scriptOpChangeL(ScriptArg_t arg);

	void scriptOpAnimR(ScriptArg_t arg);
	void scriptOpAnimF(ScriptArg_t arg);
	void scriptOpAnimN(ScriptArg_t arg);
	void scriptOpAnimG(ScriptArg_t arg);
	void scriptOpAnimS(ScriptArg_t arg);
	void scriptOpAnim(ScriptArg_t arg);

	void scriptOpStatic(ScriptArg_t arg);
	void scriptOpVarLoad(ScriptArg_t arg);
	void scriptOpVarStore(ScriptArg_t arg);
	void scriptOpVarAddAndStore(ScriptArg_t arg);
	void scriptOpVarGlobalLoad(ScriptArg_t arg);
	void scriptOpVarGlobalStore(ScriptArg_t arg);

	void scriptOpItemCheck(ScriptArg_t arg);
	void scriptOpItemRemove(ScriptArg_t arg);
	void scriptOpItemHighlightSet(ScriptArg_t arg);
	void scriptOpItemAdd(ScriptArg_t arg);
	void scriptOpItemHaveSpace(ScriptArg_t arg);
	void scriptOpItemClear(ScriptArg_t arg);

	void scriptOpSetCursor(ScriptArg_t arg);
	void scriptOpSetRoom(ScriptArg_t arg);
	void scriptOpLMB(ScriptArg_t arg);
	void scriptOpLMB1(ScriptArg_t arg);
	void scriptOpSoundS1(ScriptArg_t arg);
	void scriptOpSoundS2(ScriptArg_t arg);
	void scriptOpSoundS3(ScriptArg_t arg);
	void scriptOpSoundL1(ScriptArg_t arg);
	void scriptOpSoundL2(ScriptArg_t arg);
	void scriptOpSoundL3(ScriptArg_t arg);
	void scriptOp3DSoundS2(ScriptArg_t arg);
	void scriptOp3DSoundL2(ScriptArg_t arg);
	void scriptOp3DSoundL3(ScriptArg_t arg);
	void scriptOpStopAL(ScriptArg_t arg);
	void scriptOpRange(ScriptArg_t arg);
	void scriptOpAddXSound(ScriptArg_t arg);
	void scriptOpClrXSound(ScriptArg_t arg);
	void scriptOpStopSndLA(ScriptArg_t arg);
	void scriptOpStopSndLO(ScriptArg_t arg);

	void scriptOpMusic(ScriptArg_t arg);
	void scriptOpMusicVolRamp(ScriptArg_t arg);
	void scriptOpParm0(ScriptArg_t arg);
	void scriptOpParm1(ScriptArg_t arg);
	void scriptOpParm2(ScriptArg_t arg);
	void scriptOpParm3(ScriptArg_t arg);
	void scriptOpParmG(ScriptArg_t arg);
	void scriptOpSParmX(ScriptArg_t arg);
	void scriptOpSAnimX(ScriptArg_t arg);

	void scriptOpVolumeDn2(ScriptArg_t arg);
	void scriptOpVolumeDn3(ScriptArg_t arg);
	void scriptOpVolumeDn4(ScriptArg_t arg);
	void scriptOpVolumeUp3(ScriptArg_t arg);
	void scriptOpRandom(ScriptArg_t arg);
	void scriptOpDrop(ScriptArg_t arg);
	void scriptOpDup(ScriptArg_t arg);
	void scriptOpSwap(ScriptArg_t arg);
	void scriptOpSay1(ScriptArg_t arg);
	void scriptOpSay2(ScriptArg_t arg);
	void scriptOpSay3(ScriptArg_t arg);
	void scriptOpSay3Get(ScriptArg_t arg);
	void scriptOpSetTimer(ScriptArg_t arg);
	void scriptOpGetTimer(ScriptArg_t arg);
	void scriptOpDelay(ScriptArg_t arg);
	void scriptOpLoSet(ScriptArg_t arg);
	void scriptOpLoGet(ScriptArg_t arg);
	void scriptOpHiSet(ScriptArg_t arg);
	void scriptOpHiGet(ScriptArg_t arg);

	void scriptOpNot(ScriptArg_t arg);
	void scriptOpAnd(ScriptArg_t arg);
	void scriptOpOr(ScriptArg_t arg);
	void scriptOpAdd(ScriptArg_t arg);
	void scriptOpSub(ScriptArg_t arg);
	void scriptOpNegate(ScriptArg_t arg);
	void scriptOpCmpEq(ScriptArg_t arg);
	void scriptOpCmpLt(ScriptArg_t arg);
	void scriptOpCmpGt(ScriptArg_t arg);

	void scriptOpBitLoad(ScriptArg_t arg);
	void scriptOpBitSet0(ScriptArg_t arg);
	void scriptOpBitSet1(ScriptArg_t arg);

	void scriptOpDisc1(ScriptArg_t arg);
	void scriptOpDisc2(ScriptArg_t arg);
	void scriptOpDisc3(ScriptArg_t arg);

	void scriptOpGoto(ScriptArg_t arg);

	void scriptOpEscOn(ScriptArg_t arg);
	void scriptOpEscOff(ScriptArg_t arg);
	void scriptOpEscGet(ScriptArg_t arg);
	void scriptOpBackStart(ScriptArg_t arg);
	void scriptOpSaveAs(ScriptArg_t arg);
	void scriptOpSave0(ScriptArg_t arg);
	void scriptOpExit(ScriptArg_t arg);
	void scriptOpAllowSaves(ScriptArg_t arg);

	void scriptOpAnimName(ScriptArg_t arg);
	void scriptOpValueName(ScriptArg_t arg);
	void scriptOpVarName(ScriptArg_t arg);
	void scriptOpSoundName(ScriptArg_t arg);
	void scriptOpCursorName(ScriptArg_t arg);
	void scriptOpDubbing(ScriptArg_t arg);

	void scriptOpCheckValue(ScriptArg_t arg);
	void scriptOpJump(ScriptArg_t arg);

	void scriptOpVerticalPanSet(bool *flags);
	void scriptOpVerticalPanGet();

	// Schizm ops
	void scriptOpCallFunction(ScriptArg_t arg);

	void scriptOpMusicStop(ScriptArg_t arg);
	void scriptOpMusicPlayScore(ScriptArg_t arg);
	void scriptOpScoreAlways(ScriptArg_t arg);
	void scriptOpScoreNormal(ScriptArg_t arg);
	void scriptOpSndPlay(ScriptArg_t arg);
	void scriptOpSndPlayEx(ScriptArg_t arg);
	void scriptOpSndPlay3D(ScriptArg_t arg);
	void scriptOpSndPlaying(ScriptArg_t arg);
	void scriptOpSndWait(ScriptArg_t arg);
	void scriptOpSndHalt(ScriptArg_t arg);
	void scriptOpSndToBack(ScriptArg_t arg);
	void scriptOpSndStop(ScriptArg_t arg);
	void scriptOpSndStopAll(ScriptArg_t arg);
	void scriptOpSndAddRandom(ScriptArg_t arg);
	void scriptOpSndClearRandom(ScriptArg_t arg);
	void scriptOpVolumeAdd(ScriptArg_t arg);
	void scriptOpVolumeChange(ScriptArg_t arg);
	void scriptOpAnimVolume(ScriptArg_t arg);
	void scriptOpAnimChange(ScriptArg_t arg);
	void scriptOpScreenName(ScriptArg_t arg);
	void scriptOpExtractByte(ScriptArg_t arg);
	void scriptOpInsertByte(ScriptArg_t arg);
	void scriptOpString(ScriptArg_t arg);
	void scriptOpCmpNE(ScriptArg_t arg);
	void scriptOpCmpLE(ScriptArg_t arg);
	void scriptOpCmpGE(ScriptArg_t arg);
	void scriptOpReturn(ScriptArg_t arg);
	void scriptOpSpeech(ScriptArg_t arg);
	void scriptOpSpeechEx(ScriptArg_t arg);
	void scriptOpSpeechTest(ScriptArg_t arg);
	void scriptOpSay(ScriptArg_t arg);
	void scriptOpRandomInclusive(ScriptArg_t arg);
	void scriptOpHeroOut(ScriptArg_t arg);
	void scriptOpHeroGetPos(ScriptArg_t arg);
	void scriptOpHeroSetPos(ScriptArg_t arg);
	void scriptOpHeroGet(ScriptArg_t arg);
	void scriptOpGarbage(ScriptArg_t arg);
	void scriptOpGetRoom(ScriptArg_t arg);
	void scriptOpBitAnd(ScriptArg_t arg);
	void scriptOpBitOr(ScriptArg_t arg);
	void scriptOpAngleGet(ScriptArg_t arg);
	void scriptOpIsDVDVersion(ScriptArg_t arg);
	void scriptOpIsCDVersion(ScriptArg_t arg);
	void scriptOpDisc(ScriptArg_t arg);
	void scriptOpHidePanel(ScriptArg_t arg);
	void scriptOpRotateUpdate(ScriptArg_t arg);
	void scriptOpMul(ScriptArg_t arg);
	void scriptOpDiv(ScriptArg_t arg);
	void scriptOpMod(ScriptArg_t arg);
	void scriptOpGetDigit(ScriptArg_t arg);
	void scriptOpPuzzleInit(ScriptArg_t arg);
	void scriptOpPuzzleCanPress(ScriptArg_t arg);
	void scriptOpPuzzleDoMove1(ScriptArg_t arg);
	void scriptOpPuzzleDoMove2(ScriptArg_t arg);
	void scriptOpPuzzleDone(ScriptArg_t arg);
	void scriptOpPuzzleWhoWon(ScriptArg_t arg);
	void scriptOpFn(ScriptArg_t arg);
	void scriptOpItemHighlightSetTrue(ScriptArg_t arg);

	// AD2044 ops
	void scriptOpAnimAD2044(bool isForward);
	void scriptOpAnimT(ScriptArg_t arg);
	void scriptOpAnimForward(ScriptArg_t arg);
	void scriptOpAnimReverse(ScriptArg_t arg);
	void scriptOpAnimKForward(ScriptArg_t arg);
	void scriptOpNoUpdate(ScriptArg_t arg);
	void scriptOpNoClear(ScriptArg_t arg);
	void scriptOpSay1_AD2044(ScriptArg_t arg);
	void scriptOpSay2_AD2044(ScriptArg_t arg);
	void scriptOpSay1Rnd(ScriptArg_t arg);
	void scriptOpM(ScriptArg_t arg);
	void scriptOpEM(ScriptArg_t arg);
	void scriptOpSE(ScriptArg_t arg);
	void scriptOpSDot(ScriptArg_t arg);
	void scriptOpE(ScriptArg_t arg);
	void scriptOpDot(ScriptArg_t arg);
	void scriptOpSound(ScriptArg_t arg);
	void scriptOpISound(ScriptArg_t arg);
	void scriptOpUSound(ScriptArg_t arg);
	void scriptOpSayCycle_AD2044(const StackInt_t *values, uint numValues);
	void scriptOpSay2K(ScriptArg_t arg);
	void scriptOpSay3K(ScriptArg_t arg);
	void scriptOpRGet(ScriptArg_t arg);
	void scriptOpRSet(ScriptArg_t arg);
	void scriptOpEndRSet(ScriptArg_t arg);
	void scriptOpStop(ScriptArg_t arg);

	Common::Array<Common::SharedPtr<AnimatedCursor> > _cursors;      // Cursors indexed as CURSOR_CUR_##
	Common::Array<Common::SharedPtr<AnimatedCursor> > _cursorsShort;      // Cursors indexed as CURSOR_#

	InventoryItem _inventory[kNumInventorySlots];
	InventoryItem _inventoryPages[kNumInventoryPages][kNumInventorySlots];
	Common::HashMap<uint32, uint8> _placedItems;
	uint8 _inventoryActivePage;
	InventoryItem _inventoryActiveItem;
	InventoryItem _inventoryPlacedItemCache;
	Common::Rect _placedItemRect;

	Common::SharedPtr<Graphics::Surface> _trayCompassGraphic;
	Common::SharedPtr<Graphics::Surface> _trayBackgroundGraphic;
	Common::SharedPtr<Graphics::Surface> _trayHighlightGraphic;
	Common::SharedPtr<Graphics::Surface> _trayCornerGraphic;
	Common::SharedPtr<Graphics::Surface> _backgroundGraphic;

	Common::Array<Common::SharedPtr<Graphics::Surface> > _uiGraphics;

	uint _panCursors[kPanCursorMaxCount];

	Common::HashMap<Common::String, StackInt_t> _namedCursors;
	Common::HashMap<StackInt_t, uint> _scriptCursorIDToResourceIDOverride;

	OSystem *_system;
	uint _roomNumber;	// Room number can be changed independently of the loaded room, the screen doesn't change until a command changes it
	uint _screenNumber;
	uint _direction;
	uint _hero;
	uint _disc;

	uint _swapOutRoom;
	uint _swapOutScreen;
	uint _swapOutDirection;

	GyroState _gyros;

	AnimationDef _panLeftAnimationDef;
	AnimationDef _panRightAnimationDef;
	bool _haveHorizPanAnimations;
	bool _havePanUpFromDirection[kNumDirections];
	bool _havePanDownFromDirection[kNumDirections];

	StaticAnimation _idleAnimations[kNumDirections];
	bool _haveIdleAnimations[kNumDirections];
	bool _haveIdleStaticAnimation;
	bool _keepStaticAnimationInIdle;
	Common::String _idleCurrentStaticAnimation;
	StaticAnimParams _pendingStaticAnimParams;

	uint32 _delayCompletionTime;

	AnimationDef _postFacingAnimDef;

	Common::SharedPtr<CircuitPuzzle> _circuitPuzzle;
	AnimationDef _circuitPuzzleBlockAnimation;
	AnimationDef _circuitPuzzleConnectAnimation;

	Common::HashMap<uint32, int32> _variables;
	Common::HashMap<uint, uint32> _timers;

	uint _panoramaDirectionFlags;

	uint _loadedRoomNumber;
	uint _activeScreenNumber;
	bool _havePendingScreenChange;
	bool _forceScreenChange;

	// returnToIdleState executes any actions that must be executed upon returning to idle state from either
	// a panorama or the first script that executes upon reaching a screen.
	//
	// Unfortunately, this was done slightly prematurely since Schizm plays idle animations during pre-idle
	// delays and Reah never needs to do that, so _havePendingPreIdleActions exists to handle those actions
	// during pre-idle upon arriving at a screen.
	//
	// Pre-idle actions are executed once upon either entering Idle OR Delay state.
	bool _havePendingPreIdleActions;
	bool _havePendingReturnToIdleState;
	bool _havePendingPostSwapScreenReset;

	bool _havePendingCompletionCheck;
	GameState _gameState;

	Common::SharedPtr<MenuPage> _menuPage;
	Common::SharedPtr<MenuInterface> _menuInterface;

	bool _havePendingPlayAmbientSounds;
	uint32 _ambientSoundFinishTime;

	bool _escOn;
	bool _debugMode;
	bool _fastAnimationMode;
	bool _preloadSounds;
	bool _lowQualityGraphicsMode;

	VCruiseGameID _gameID;

	Common::Array<Common::SharedPtr<RoomDef> > _roomDefs;
	Common::Array<uint> _roomDuplicationOffsets;
	RoomToScreenNameToRoomMap_t _globalRoomScreenNameToScreenIDs;
	Common::SharedPtr<ScriptSet> _scriptSet;
	Common::Array<AD2044AnimationDef> _ad2044AnimationDefs;

	Common::Array<CallStackFrame> _scriptCallStack;

	Common::Array<StackValue> _scriptStack;
	ScriptEnvironmentVars _scriptEnv;

	Common::SharedPtr<Common::RandomSource> _rng;

	Common::SharedPtr<AudioPlayer> _musicWavePlayer;
	Common::Mutex _midiPlayerMutex;
	Common::SharedPtr<MidiPlayer> _musicMidiPlayer;
	int _musicTrack;
	int32 _musicVolume;
	bool _musicActive;
	bool _musicMute;
	bool _musicMuteDisabled;

	Common::String _scoreTrack;
	Common::String _scoreSection;
	uint32 _scoreSectionEndTime;
	Common::HashMap<Common::String, ScoreTrackDef> _scoreDefs;

	uint32 _musicVolumeRampStartTime;
	int32 _musicVolumeRampStartVolume;
	int32 _musicVolumeRampRatePerMSec;
	int32 _musicVolumeRampEnd;

	SfxData _sfxData;

	Common::SharedPtr<Video::AVIDecoder> _animDecoder;
	Common::SharedPtr<SfxPlaylist> _animPlaylist;
	AnimDecoderState _animDecoderState;
	bool _animTerminateAtStartOfFrame;
	uint _animPendingDecodeFrame;
	uint _animDisplayingFrame;
	uint _animFirstFrame;
	uint _animLastFrame;
	uint _animStopFrame;
	uint _animVolume;
	Fraction _animFrameRateLock;
	Common::Rect _animConstraintRect;
	uint32 _animStartTime;
	uint32 _animFramesDecoded;
	uint _loadedAnimation;
	bool _loadedAnimationHasSound;
	bool _animPlayWhileIdle;

	Common::Array<FrameData> _frameData;
	Common::Array<FrameData2> _frameData2;
	//uint32 _loadedArea;

	// Reah/Schizm animation map
	Common::Array<Common::String> _animDefNames;
	Common::HashMap<Common::String, uint> _animDefNameToIndex;

	// AD2044 animation map
	Common::HashMap<int, AnimFrameRange> _currentRoomAnimIDToFrameRange;
	Common::HashMap<int, AnimFrameRange> _examineAnimIDToFrameRange;

	bool _idleLockInteractions;
	bool _idleIsOnInteraction;
	bool _idleHaveClickInteraction;
	bool _idleHaveDragInteraction;
	uint _idleInteractionID;

	bool _idleIsOnOpenCircuitPuzzleLink;
	Common::Rect _idleCircuitPuzzleLinkHighlightRect;

	bool _idleIsCircuitPuzzleLinkDown;
	Common::Point _idleCircuitPuzzleCoord;

	bool _forceAllowSaves;

	InGameMenuState _inGameMenuState;
	uint _inGameMenuActiveElement;
	bool _inGameMenuButtonActive[5];

	Audio::Mixer *_mixer;
	MidiDriver *_midiDrv;

	Common::SharedPtr<MapLoader> _mapLoader;

	RenderSection _gameSection;
	RenderSection _gameDebugBackBuffer;
	RenderSection _menuSection;
	RenderSection _traySection;
	RenderSection _fullscreenMenuSection;
	RenderSection _subtitleSection;
	RenderSection _placedItemBackBufferSection;

	Common::Point _mousePos;
	Common::Point _lmbDownPos;
	uint32 _lmbDownTime;
	int _lmbDragTolerance;
	bool _lmbDown;
	bool _lmbDragging;
	bool _lmbReleaseWasClick;	// If true, then the mouse didn't move at all since the LMB down

	PanoramaState _panoramaState;
	// The anchor point behavior is kind of weird.  The way it works in Reah is that if the camera is panning left or right and the mouse is moved
	// back across the anchor threshold at all, then the anchor point is reset to the mouse position.  This means it can drift, because the center
	// is moved by roughly the margin amount.
	// Vertical panning on the other hand resets the anchor vertical coordinate every time.
	Common::Point _panoramaAnchor;

	Common::Array<OSEvent> _pendingEvents;

	Common::HashMap<Common::String, Common::ArchiveMemberPtr> _waves;
	Common::Array<Common::SharedPtr<SoundInstance> > _activeSounds;
	SoundParams3D _pendingSoundParams3D;

	Common::Array<TriggeredOneShot> _triggeredOneShots;
	Common::HashMap<uint32, uint> _sayCycles;
	Common::Array<RandomAmbientSound> _randomAmbientSounds;

	int32 _listenerX;
	int32 _listenerY;
	int32 _listenerAngle;

	Fraction _animSpeedRotation;
	Fraction _animSpeedStaticAnim;
	Fraction _animSpeedDefault;

	static const uint kAnimDefStackArgs = 8;

	static const uint kCursorArrow = 0;
	static const uint kCursorWait = 29;

	static const int kPanoramaPanningMarginX = 11;
	static const int kPanoramaPanningMarginY = 11;

	static const uint kSoundCacheSize = 16;

	static const uint kExamineItemInteractionID = 0xfffffff0u;

	static const uint kReturnInventorySlot0InteractionID = 0xfffffff1u;
	static const uint kReturnInventorySlot1InteractionID = 0xfffffff2u;
	static const uint kReturnInventorySlot2InteractionID = 0xfffffff3u;
	static const uint kReturnInventorySlot3InteractionID = 0xfffffff4u;
	static const uint kReturnInventorySlot4InteractionID = 0xfffffff5u;
	static const uint kReturnInventorySlot5InteractionID = 0xfffffff6u;

	static const uint kPickupInventorySlot0InteractionID = 0xfffffff7u;
	static const uint kPickupInventorySlot1InteractionID = 0xfffffff8u;
	static const uint kPickupInventorySlot2InteractionID = 0xfffffff9u;
	static const uint kPickupInventorySlot3InteractionID = 0xfffffffau;
	static const uint kPickupInventorySlot4InteractionID = 0xfffffffbu;
	static const uint kPickupInventorySlot5InteractionID = 0xfffffffcu;

	static const uint kObjectPickupInteractionID = 0xfffffffdu;
	static const uint kObjectDropInteractionID = 0xfffffffeu;

	static const uint kHeroChangeInteractionID = 0xffffffffu;

	Common::Pair<Common::String, Common::SharedPtr<SoundCache> > _soundCache[kSoundCacheSize];
	uint _soundCacheIndex;

	Common::SharedPtr<SaveGameSnapshot> _mostRecentValidSaveState;			// Always valid
	Common::SharedPtr<SaveGameSnapshot> _mostRecentlyRecordedSaveState;		// Might be invalid, becomes valid if the player returns to idle
	Common::SharedPtr<SaveGameSwappableState> _altState;
	bool _isInGame;

	const Graphics::Font *_subtitleFont;
	Common::SharedPtr<Graphics::Font> _subtitleFontKeepalive;
	uint _defaultLanguageIndex;
	uint _languageIndex;
	Common::Language _language;
	CharSet _charSet;
	bool _isCDVariant;
	StartConfigDef _startConfigs[kNumStartConfigs];

	Common::Language _defaultLanguage;

	typedef Common::HashMap<uint, SubtitleDef> FrameToSubtitleMap_t;
	typedef Common::HashMap<uint, FrameToSubtitleMap_t> AnimSubtitleMap_t;
	typedef Common::HashMap<Common::String, SubtitleDef> WaveSubtitleMap_t;

	AnimSubtitleMap_t _animSubtitles;
	Common::HashMap<Common::String, SubtitleDef> _waveSubtitles;
	Common::Array<SubtitleQueueItem> _subtitleQueue;
	bool _isDisplayingSubtitles;
	bool _isSubtitleSourceAnimation;

	Common::HashMap<Common::String, Common::String> _locStrings;
	Common::HashMap<Common::String, TextStyleDef> _locTextStyles;
	Common::HashMap<Common::String, UILabelDef> _locUILabels;

	Common::Array<Common::SharedPtr<FontCacheItem> > _fontCache;

	AnimatedCursor *_currentAnimatedCursor;
	Graphics::Cursor *_currentCursor;
	uint32 _cursorTimeBase;
	uint32 _cursorCycleLength;

	int32 _dbToVolume[49];

	// AD2044 tooltips
	Common::String _tooltipText;
	Common::String _subtitleText;

	Common::SharedPtr<AD2044Graphics> _ad2044Graphics;
	Common::Array<Common::String> _ad2044ItemNames;
};

} // End of namespace VCruise

#endif
