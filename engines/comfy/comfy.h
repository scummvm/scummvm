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

#ifndef COMFY_H
#define COMFY_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/array.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/screen.h"
#include "audio/mixer.h"

#include "comfy/detection.h"
#include "comfy/midiplyr/midiplyr.h"

#define COMFY_SCREEN_WIDTH 320
#define COMFY_SCREEN_HEIGHT 200
#define COMFY_PANTHER_SCREEN_WIDTH 640
#define COMFY_PANTHER_SCREEN_HEIGHT 480
#define COMFY_PIT_INPUT_FREQUENCY 1193182
#define COMFY_PIT_TIMER_DIVISOR 0x2E9B
#define COMFY_RESOLUTION_CHANGE_CAPACITY 100
#define COMFY_PALETTE_BYTES 0x300
#define COMFY_INPUT_QUEUE_CAPACITY 20
#define COMFY_KEYBOARD_CONTACT_COUNT 24
#define COMFY_ACTOR_SIZE 0x54
#define COMFY_ACTOR_COUNT 0x79
#define COMFY_EXPR_STACK_CAPACITY 64
#define COMFY_ACTOR_PC_TABLE_COUNT 200
#define COMFY_TILE_SIZE 0x800
#define COMFY_RESOURCE_LIST_CAPACITY 32
#define COMFY_SCENE_ENTRY_OFFSET_CAPACITY 16
#define COMFY_SCENE_MIDI_INSTANCE_BYTES 0x18C
#define COMFY_SCENE_ACTOR_PC_BYTES 0x320
#define COMFY_SCENE_FRAME_BYTES 0x4E20
#define COMFY_VOC_ARG_CAPACITY 10
#define COMFY_VOC_QUEUE_CAPACITY 16
#define COMFY_MIDI_QUEUE_CAPACITY 32
#define COMFY_MIDI_CHANNEL_COUNT 2
#define COMFY_MIDI_TRACK_ENTRY_CAPACITY 5
#define COMFY_ANIM_FRAME_CAPACITY 10
#define COMFY_ANMFILE_HEADER_BYTES 16
#define COMFY_ANMFILE_MAGIC 0x4143
#define COMFY_ANMFILE_PANTHER_MAGIC 0x4643
#define COMFY_ANMFRAME_MAGIC 0x4E41
#define COMFY_ANMFRAME_PANTHER_MAGIC 0x4C46
#define COMFY_ANMFRAME_BYTES 0xFA00

namespace Comfy {

class MidiPlyrDriver;

enum ComfyEngineVersion {
	kEngineVersion1, // 1994-1995
	kEngineVersion2, // 1996-1997
	kEngineVersion3  // 1999
};

#ifdef USE_IMGUI
void onImGuiInit();
void onImGuiRender();
void onImGuiCleanup();
#endif

class ComfyEngine : public Engine {
private:
	struct VideoRectRecord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
		uint16 area;
	};

	struct InputQueue {
		uint16 words[COMFY_INPUT_QUEUE_CAPACITY];
		uint16 readIndex;
		uint16 writeIndex;
		uint16 tailIndex;
	};

	struct SpriteObjectHeader {
		uint32 fileOffset;
		uint16 dataSize;
		uint16 width;
		uint16 height;
		int16 hotspotX;
		int16 hotspotY;
		byte reserved;
		uint16 tiledSize;
	};

	struct SpriteResource {
		int16 id;
		SpriteObjectHeader header;
		Common::Array<byte> pixels;
		bool loaded;
	};

	struct SpriteCacheEntry {
		uint32 poolOffset;
		uint16 slotSize;
	};

	struct ResourceLoadList {
		uint16 ids[COMFY_RESOURCE_LIST_CAPACITY];
		uint16 count;
	};

	struct VocQueueEntry {
		uint16 soundId;
		uint16 argumentCount;
		uint16 state;
		uint16 arguments[COMFY_VOC_ARG_CAPACITY];
	};

	struct MidiQueueEntry {
		uint16 id;
		int32 time;
	};

	struct MidiQueue {
		MidiQueueEntry entries[COMFY_MIDI_QUEUE_CAPACITY];
		uint16 count;
		int32 baseTime;
		uint16 nextIndex;
		int32 nextTime;
	};

	struct MidiTrackEntry {
		uint16 songId;
		uint16 completionKey;
		byte loadFlag;
		uint16 frameCount;
		uint16 frames[COMFY_ANIM_FRAME_CAPACITY];
	};

	struct MidiChannelState {
		MidiTrackEntry entries[COMFY_MIDI_TRACK_ENTRY_CAPACITY];
		uint16 entryCount;
		int16 volumeCurrent;
		int16 volumeTarget;
		int16 volumeTicksLeft;
		int16 volumeDefault;
		int16 rateCurrent;
		int16 rateTarget;
		int16 rateTicksLeft;
		int16 rateDefault;
		int16 pitchCurrent;
		int16 pitchTarget;
		int16 pitchTicksLeft;
		int16 pitchDefault;
		uint16 loadedFrameSize;
		bool playing;
	};

	struct SoundCue {
		uint16 value;
		uint32 counterThreshold;
	};

	struct Actor {
		byte raw[COMFY_ACTOR_SIZE];
	};

	enum ActorOffset {
		kActorCurrentPc = 0x00,
		kActorCallPc = 0x04,
		kActorResetPc = 0x08,
		kActorXFixed = 0x0C,
		kActorYFixed = 0x10,
		kActorSpriteSelector = 0x14,
		kActorMoveDx = 0x18,
		kActorMoveDy = 0x1C,
		kActorTriggerPc = 0x20,
		kActorStringRef = 0x24,
		kActorSceneHandle = 0x28,
		kActorVisible = 0x2A,
		kActorActive = 0x2B,
		kActorParent = 0x2C,
		kActorChildTail = 0x2E,
		kActorChildHead = 0x30,
		kActorSiblingHead = 0x32,
		kActorNextLink = 0x34,
		kActorPrevLink = 0x36,
		kActorMoveTicks = 0x38,
		kActorBlockingMove = 0x3A,
		kActorCompletionKey = 0x3B,
		kActorTriggerKey = 0x3D,
		kActorTriggerFlags = 0x3F,
		kActorWaitTarget = 0x40,
		kActorWaitAccum = 0x42,
		kActorDirty = 0x44,
		kActorCachedRect = 0x45,
		kActorCachedVisible = 0x4F,
		kActorCachedSprite = 0x50
	};

	enum ScriptDispatchStatus {
		kScriptUnhandled,
		kScriptContinue,
		kScriptYield,
		kScriptDeactivatedRoot
	};

	const ADGameDescription *_gameDescription;
	ComfyEngineVersion _engineVersion;
	Common::Path _gameDirectory;
	Common::Path _introDirectory;
	Common::Path _gameDataPath;
	Common::Path _languageDirectories[16];
	uint16 _language;
	bool _multiLanguage;
	Common::RandomSource _randomSource;
	Graphics::Screen *_screen;
	byte *_framebufPtr;
	byte *_presentBuffer;
	uint16 _logicalScreenWidth;
	uint16 _logicalScreenHeight;
	VideoRectRecord _resolutionChanges[COMFY_RESOLUTION_CHANGE_CAPACITY];
	uint16 _resolutionChangeCount;
	uint16 _renderDirtyCount;
	bool _renderInterleaved;
	Common::SeekableReadStream *_colorDatStream;
	byte _paletteFadeSource[COMFY_PALETTE_BYTES];
	byte _paletteTarget[COMFY_PALETTE_BYTES];
	byte _paletteDisplay[COMFY_PALETTE_BYTES];
	byte _logicalPalette[256 * 4];
	byte *_paletteDataPtr;
	uint16 _vsyncPending;
	uint16 _fadeMax;
	uint16 _fadeStep;
	byte _palettePage;
	bool _paletteFading;
	int16 _timerCurrent;
	int16 _timer0;
	int16 _timer1;
	int16 _timer2;
	uint32 _midiTimeCounter;
	uint32 _midiInstanceEventTime;
	int32 _midiEventBaseTime;
	int32 _midiTimeScale;
	int16 _midiTimeDelta;
	int16 _midiCounterAdjustment;
	MidiPlyrDriver *_midiPlyrDriver;
	byte *_keyBits;
	uint32 _keyBitsSize;
	InputQueue _inputQueue;
	byte _keyboardKeyToBit[256];
	bool _keyboardMapLoaded;
	uint32 _keyboardActiveMask;
	uint32 _keyboardLatchedMask;
	uint32 _keymapperActiveMask;
	uint32 _keymapperLatchedMask;
	uint32 _toyKeyboardActiveMask;
	uint32 _toyKeyboardLatchedMask;
	uint32 _toyKeyboardHoldMask;
	uint32 _lptPrevScanState;
	uint16 _inputDeviceMode;
	uint16 _inputDevicePreference;
	uint32 _inputPreviousComfyboardState;
	uint32 _inputPreviousKeyboardState;
	uint16 _inputComfyboardActivityCount;
	uint16 _inputKeyboardActivityCount;
	bool _keyboardUiInitialized;
	bool _keyboardUiVisible;
	Common::Array<byte> _comfyObjData;
	Common::Array<byte> _picFileData;
	Common::Array<byte> _midiFileData;
	Common::Array<SpriteObjectHeader> _spriteHeaders;
	Common::Array<SpriteResource> _spriteResources;
	SpriteResource _frameSpriteResource;
	uint16 _stringCount;
	uint16 _sceneCount;
	uint16 _keyBitCount;
	uint16 _resourceHandleCount;
	uint16 _midiEntryCount;
	uint32 _picDataSize;
	bool _usesAnimFile;
	bool _sceneOpen;
	bool _sceneEntryListActive;
	Common::Array<uint16> _stringTable;
	Common::Array<uint16> _sceneHandles;
	Common::Array<uint16> _midiHandles;
	Common::Array<Actor> _actors;
	Common::Array<byte> _sceneMemoryBlock;
	Common::Array<byte> _scenePoolData;
	Common::Array<byte> _environmentData;
	Common::Array<byte> _headerXmsData;
	Common::Array<byte> _sceneFrameData;
	Common::Array<SpriteCacheEntry> _objectCacheEntries;
	Common::Array<SpriteCacheEntry> _frameCacheEntries;
	ResourceLoadList _spriteConversionLoads;
	uint32 _sceneEntryOffsets[COMFY_SCENE_ENTRY_OFFSET_CAPACITY];
	uint32 _sceneMidiInstanceOffset;
	uint32 _sceneEntryListOffset;
	uint32 _sceneActorPcOffset;
	uint32 _sceneStringTableOffset;
	uint32 _sceneHandlesOffset;
	uint32 _sceneActorsOffset;
	uint32 _sceneKeyBitsOffset;
	uint32 _scenePoolCursor;
	uint32 _scenePoolEvictCursor;
	uint32 _scenePoolSize;
	uint32 _headerXmsObjectTableBase;
	uint32 _headerXmsObjectTableBytes;
	uint32 _headerXmsPicEntriesBase;
	uint32 _headerXmsPicEntriesBytes;
	uint32 _headerXmsSoundHeadersBase;
	uint32 _headerXmsSoundHeadersBytes;
	uint16 _selectorPoolEntries[10];
	uint16 _selectorRing;
	bool _selectorPoolInitialized;
	uint16 _activeSceneCount;
	uint16 _sceneEntryCount;
	uint16 _sceneEntryFrameSize;
	uint16 _numObjects;
	uint16 _numFrames;
	uint32 _numSprites;
	uint32 _envNumSprites;
	byte _midiFileMode;
	bool _mirrorMode;
	uint32 _actorPcTable[COMFY_ACTOR_PC_TABLE_COUNT];
	uint16 _currentActor;
	uint16 _pendingScene;
	uint16 _musicEventMask;
	byte _musicEventFlag;
	bool _musicEnabled;
	bool _usesWcomfy99ScriptOps;
	uint16 _wcomfy99FeatureWords[16];
	byte _wcomfy99FeatureWordCount;
	uint16 _wcomfy99Sensitivity;
	bool _wcomfy99RecordHostEnabled;
	uint16 _wcomfy99SubsystemWord;
	uint16 _wcomfy99MixedHostFirstWord;
	uint16 _wcomfy99MixedHostSecondWord;
	uint16 _wcomfy99MixedHostThirdWord;
	uint16 _wcomfy99MixedHostFourthWord;
	uint16 _wcomfy99HostWordA;
	uint16 _wcomfy99HostWordB;
	uint16 _wcomfy99WaveVolumePercent;
	uint16 _wcomfy99WaveLeftPercent;
	uint16 _wcomfy99WaveRightPercent;
	uint16 _wcomfy99MixerVolumePercent;
	uint16 _wcomfy99MixerAltPercent;
	uint16 _wcomfy99RangeHostStart;
	uint16 _wcomfy99RangeHostEnd;
	uint16 _wcomfy99RangeHostCount;
	bool _actorDestroyedCurrent;
	uint16 _lastKey;
	VocQueueEntry _vocQueue[COMFY_VOC_QUEUE_CAPACITY];
	uint16 _soundEventIndex;
	uint16 _soundEventMaximum;
	uint16 _soundEventSubIndex;
	uint16 _soundEventPreviousSubIndex;
	MidiQueue _midiEvents;
	MidiQueue _midiTracks;
	MidiChannelState _midiChannels[COMFY_MIDI_CHANNEL_COUNT];
	uint32 _midiInstanceTrackBase;
	Common::Array<byte> _vocFileData;
	Common::Array<byte> _soundPcm;
	Common::Array<SoundCue> _soundCues;
	Audio::SoundHandle _soundHandle;
	uint16 _soundTileStride;
	uint32 _soundSampleRate;
	uint _soundNextCue;
	bool _soundCompressed;
	bool _soundPaused;
	Common::Array<byte> _animFileData;
	Common::Array<uint32> _animIndexTable;
	Common::Array<byte> _animFrameBuffer;
	Common::Array<byte> _animFrameStorage;
	byte _animFrameHeader[COMFY_ANMFILE_HEADER_BYTES];
	byte _animPendingDirtyRect[16];
	uint32 _animPosition;
	uint16 _animPendingDirtyRectSize;
	uint16 _animCurrentIndex;
	uint16 _animCurrentActorSceneHandle;
	uint16 _animCurrentFrameKey;
	byte _animVocCounterMode;
	uint16 _animVocClockHz;
	uint16 _animVocTargetCounter;
	uint16 _animVocBaseCounter;
	uint16 _animVocDeltaA;
	uint32 _animVocCounterStartA;
	uint32 _animVocCounterStartB;
	bool _animIndexLoaded;
	bool _animPantherFormat;
	bool _animActive;
	uint16 _exprStack[COMFY_EXPR_STACK_CAPACITY];
	uint16 _exprStackTop;
	bool _scriptFault;
	bool _gameInitialized;
	bool _videoInitialized;
	bool _timerInitialized;
	bool _lptKeyboardInitialized;

	Common::Error gameInit();
	void gameShutdown();
	void gameConfigInit();
	void findLanguageDirectories();
	bool iniReadGameConfig();
	bool iniGetGameDataPath(uint16 sceneId);
	void iniWriteLanguage(uint16 language);
	Common::Path getLanguageDirectory(uint16 language);
	void pathSetGameDataDir(const Common::Path &path);
	Common::Path pathBuild(const Common::Path &filename, bool useGamePath);
	Common::SeekableReadStream *pathFOpen(const Common::Path &filename, bool useGamePath);
	void videoInit();
	void videoShutdown(byte restorePalette);
	void videoSetResolution();
	void videoFindBestMode(VideoRectRecord record);
	void videoPresentFrame();
	void renderSetDirty();
	void renderFlushDirty();
	void framebufCopyAll(byte *destination, byte *source);
	void framebufClear(uint16 color);
	uint32 framebufferBytes();
	void colorDatOpen();
	void colorDatClose();
	void colorDatReadEntry(uint16 paletteId);
	void vsyncSetPalettePtr(byte *palette);
	void paletteInterpolate(uint16 step, uint16 maximum);
	void paletteLoadWithFade(uint16 paletteId, uint16 fadeTicks);
	void paletteApplyBrightness(uint16 brightness);
	void paletteVsyncFlip();
	void paletteConvertRgbToLogical(byte *source, byte *destination);
	void paletteRealize(byte *rawPalette);
	uint16 midiTick();
	void midiSetTimeScale(int16 delta);
	int16 midiGetCounterAdjustment();
	bool midiPlyrStart();
	void midiPlyrStop();
	bool keyBitAllocate(uint16 keyBitCount);
	void keyBitFree();
	bool keyBitTest(uint16 bitIndex);
	void keyBitCopyRange(uint16 destination, uint16 count, uint32 source);
	void keyBitSet(uint16 bitIndex);
	void keyBitClear(uint16 bitIndex);
	void inputQueuePushKey(uint16 key);
	void inputQueuePushChar(uint16 key);
	bool inputQueueHasItems();
	uint16 inputQueueDequeue();
	void inputQueueReset();
	uint16 lptReadKeyOrNext();
	void lptKeyToFlags(uint16 key);
	void hostKeyboardResetMap();
	bool hostKeyboardLoadDatMap();
	void hostKeyboardSetKeyState(uint16 key, bool pressed);
	uint16 hostKeyboardVirtualKey(Common::KeyCode key);
	uint32 lptKeyboardScan();
	uint16 inputDetectDevice(uint32 comfyboardState, uint32 keyboardState);
	void lptKeyboardDispatchEvents(uint32 scanState);
	void setKeyboardContact(uint16 contact, bool pressed, bool keymapper);
	bool readAssetFile(const Common::Path &filename, Common::Array<byte> &data);
	uint32 assetsAlignEven32(uint32 value);
	uint16 assetsReadLe16At(Common::Array<byte> &data, uint32 offset);
	uint32 assetsReadLe32At(Common::Array<byte> &data, uint32 offset);
	int32 assetsXmsCopy(byte *destination, uint32 destinationSize, uint32 destinationOffset,
		const byte *source, uint32 sourceSize, uint32 sourceOffset, uint32 size);
	bool selPoolInit();
	void selPoolFree();
	bool assetsLoad(uint32 budget, byte *scenePtr);
	void assetsUnload(byte freeAudio);
	bool sceneLoadAndInit(uint16 sceneCount, uint16 actorCount, uint16 keyBitCount,
		uint16 handleCount, uint32 &numSprites);
	void midiShutdown();
	bool comfyObjOpen();
	bool picFileOpen();
	bool midiFileOpen();
	void objFileReadEntries(byte *destination);
	byte *soundReadTiledData();
	void soundGetTileParams(uint16 *tileStride, uint16 *fieldCount);
	void spriteCache(int16 spriteId);
	void spriteInvalidateHostCache(SpriteResource &sprite);
	void objHdrReadFromXms(byte *destination, uint32 base, uint16 size, uint16 row);
	void objHdrRead(SpriteObjectHeader &destination, uint16 index);
	void scenePoolEvict();
	void scenePoolReserveSlot(uint32 size);
	SpriteResource *spriteGetPtr(int16 spriteId);
	bool spriteDecompressTile(SpriteResource &sprite, const byte *source, uint32 sourceSize);
	void spriteBlitClipped(int16 spriteId, int16 x, int16 y);
	void spriteBlitRle(const byte *source, uint32 sourceSize);
	bool sceneOpen(uint32 sceneEntryListOffset);
	void sceneShutdown();
	void sceneEntryReadFromXms(byte *destination, uint16 row, uint16 size);
	byte *sceneFrameGetPtr(uint16 kind, uint16 size);
	byte *spriteLoadFromFile(uint16 fileOffset, uint16 index);
	void sceneBlockPackRuntimeState();
	void sceneBlockUnpackRuntimeState();
	uint16 sceneGetHandle(uint16 sceneIndex);
	uint16 sceneGetActiveCount();
	bool envXmsToConv(byte *destination, uint16 index);
	void envConvToXms(byte *source, uint16 index);
	void sceneGoto(uint16 count);
	void sceneStop();
	bool sceneEntryLoad(uint16 descriptor, uint16 index);
	bool scriptHasRange(uint32 pc, uint32 width);
	byte scriptReadByte(uint32 pc);
	uint16 scriptReadWord(uint32 pc);
	uint32 scriptReadDword(uint32 pc);
	uint16 scriptReadStringIndex(uint32 pc);
	uint32 scriptReadArgs(uint32 pc, uint16 fallbackActor, const char *format, ...);
	uint16 scriptEvalExpr(uint32 &pc, uint16 fallbackActor);
	ScriptDispatchStatus scriptDispatch(Actor &actor, byte opcode, uint32 &pc);
	ScriptDispatchStatus scriptStep(Actor &actor, uint32 &pc);
	uint32 actorReadU32(Actor &actor, uint offset);
	uint16 actorReadU16(Actor &actor, uint offset);
	byte actorReadU8(Actor &actor, uint offset);
	void actorWriteU32(Actor &actor, uint offset, uint32 value);
	void actorWriteU16(Actor &actor, uint offset, uint16 value);
	void actorWriteU8(Actor &actor, uint offset, byte value);
	Actor *rootActor();
	Actor *actorGetPtr(uint16 actorIndex);
	void actorSetFrame(int16 frame);
	uint16 actorGetFrame();
	Actor *actorResolve(uint16 sceneOrActor, uint16 fallbackActor);
	uint16 actorAlloc(uint16 sceneSlot);
	void actorFreeSlot(uint16 sceneSlot);
	void actorInsertChild(uint16 childIndex, uint16 parentIndex);
	void actorInsertSibling(uint16 actorIndex, uint16 ownerIndex);
	void actorUnlink(uint16 actorIndex);
	void actorSetPc(Actor &actor, uint32 pc);
	uint32 actorPopPc(Actor &actor);
	void actorFreePcChain(Actor &actor);
	void actorFreeTree(uint16 actorIndex);
	void actorClearDirtyTree(uint16 actorIndex);
	void actorSetAllVisible();
	uint16 actorInit(uint16 sceneSlot, uint16 parentSlot, byte visible, byte active,
		uint32 pc, int16 x, int16 y, int16 sprite, byte insertAsChild);
	bool actorRunScript(uint16 actorIndex, bool &descendChildren);
	bool actorTickTree(uint16 actorIndex);
	bool actorDraw(uint16 actorIndex, int16 x, int16 y);
	void actorDrawList(uint16 actorIndex, int16 x, int16 y);
	void actorDrawScripted(uint16 actorIndex, uint32 selector, int16 x, int16 y);
	VideoRectRecord actorReadCachedRect(Actor &actor);
	void actorWriteCachedRect(Actor &actor, VideoRectRecord rect);
	void actorInvalidateDrawTree(uint16 actorIndex);
	void timerInit();
	void timerShutdown();
	void lptKeyboardInit();
	void lptKeyboardShutdown();
	void gameMainLoop(uint16 argument);
	uint16 sceneRun(uint16 sceneId, bool checkNext, bool exitFlag);
	void processEvents();

	uint16 timerTick();
	void midiTrackTickAndRemove();
	void animFileTickCommands();
	bool animFileOpen();
	void animFileShutdown(bool closeFile);
	void animFileLoadFrame(uint16 animIndex, uint16 frameKey, uint16 actorSceneHandle);
	uint32 animDecompressRle(const byte *source, uint32 sourceSize, byte *destination, uint32 destinationSize);
	uint32 animDecodePanther(const byte *source, uint32 sourceSize);
	bool animFrameShouldWaitForVoc();
	bool animFrameShouldDraw(uint16 phase);
	int16 animFrameVocCounterDelta();
	void animFrameRecordVocCounter(uint16 phase);
	void animFrameWaitForVocCounter();
	void sceneTickEvent();
	void sceneStartWithMusic(uint16 scene);
	void midiPollChannels(uint16 ticks);
	void midiInitInstance();
	void midiInitChannels();
	void midiSyncAndScan();
	void midiFindNext(MidiQueue &queue);
	void midiQueueAdd(MidiQueue &queue, uint16 id, int16 delta);
	void midiQueueRemove(MidiQueue &queue, uint16 id);
	void midiAddEvent(uint16 id, int16 delta);
	void midiAddTrack(uint16 id, int16 delta);
	void midiRemoveTrack(uint16 id);
	void midiAddTrackEntry(uint16 channel, uint16 songId, uint16 completionKey, byte loadFlag,
		uint16 frameCount, uint16 *frames);
	void midiClearChannel(uint16 channel);
	void midiResumeAll();
	void midiStopChannel(uint16 channel);
	void midiStopAll();
	void midiStopSong(uint16 channel);
	void midiPlaySongAtFrame(uint16 channel, uint16 frame);
	void midiFinishChannel(uint16 channel);
	void midiStopAndFireKeys(uint16 channel);
	void midiStopAndRemove(uint16 channel);
	void midiSetChannelParam(uint16 channel, byte parameter, uint16 value, uint16 ticks);
	int16 midiApproachTarget(int16 current, int16 target, int16 &ticksLeft, int16 ticks);
	void musicSetEnabled(byte value);
	void paletteFadeStep(uint16 ticks);
	void lptKeyboardScanAndProcess();
	void vocQueuePush(uint16 soundId, uint16 argumentCount, uint32 pc);
	void vocQueuePlayAll();
	bool vocQueueIsIdle();
	bool soundInit();
	void soundShutdown();
	void soundHdrReadFromXms(byte *destination, uint16 index, uint16 size);
	bool soundDecodeEntry(uint16 index);
	bool soundDecodeCompressedEntry(uint32 dataOffset, uint32 dataSize);
	bool soundLoadEntry(uint16 index);
	void soundPlayEntry(uint16 index);
	void soundAdvanceTick();

protected:
	// Engine APIs
	Common::Error run() override;

public:
	ComfyEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~ComfyEngine() override;
	void setToyKeyboardState(uint32 activeMask, uint32 latchedMask, uint32 holdMask);

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsReturnToLauncher);
	};

};

extern ComfyEngine *g_engine;
#define SHOULD_QUIT ::Comfy::g_engine->shouldQuit()

} // End of namespace Comfy

#endif // COMFY_H
