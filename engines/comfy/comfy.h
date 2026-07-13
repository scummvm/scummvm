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

namespace Audio {
class QueuingAudioStream;
}

#define COMFY_SCREEN_WIDTH 320
#define COMFY_SCREEN_HEIGHT 200
#define COMFY_PANTHER_SCREEN_WIDTH 640
#define COMFY_PANTHER_SCREEN_HEIGHT 480
#define COMFY_PIT_INPUT_FREQUENCY 1193182
#define COMFY_PIT_TIMER_DIVISOR 0x2E9B
#define COMFY_RESOLUTION_CHANGE_CAPACITY 100
#define COMFY_DRAW_COMMAND_CAPACITY 200
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
#define COMFY_PANTHER_SOUND_STATE_BYTES 0xBE
#define COMFY_SCENE_MIDI_VERSION_BYTES_CLASSIC 0x167
#define COMFY_SCENE_VOC_STATE_BYTES_V3 0x24D
#define COMFY_SCENE_STATE_BYTES_V3 0x17F
#define COMFY_PANTHER_ACTOR_SIZE 0x56
#define COMFY_ACTOR_SIZE_V3 0x57
#define COMFY_SCENE_ACTOR_PC_BYTES 0x320
#define COMFY_SCENE_FRAME_BYTES 0x4E20
#define COMFY_SCENE_FRAME_BYTES_V3 0x7530
#define COMFY_VOC_ARG_CAPACITY 10
#define COMFY_VOC_QUEUE_CAPACITY 16
#define COMFY_PANTHER_VOC_QUEUE_CAPACITY 4
#define COMFY_SOUND_PCM_BLOCK_BYTES 0x4E20
#define COMFY_PANTHER_SOUND_PCM_BLOCK_BYTES 0x3E80
#define COMFY_SOUND_PITCH_COUNT_LIMIT 0x13
#define COMFY_FRAME_LOADER_DATA_BYTES 0x6000
#define COMFY_VOC_ARG_CAPACITY_1999 12
#define COMFY_MIDI_QUEUE_CAPACITY 32
#define COMFY_MIDI_CHANNEL_COUNT 2
#define COMFY_SCENE_MUSIC_CHANNEL_COUNT 6
#define COMFY_MIDI_TRACK_ENTRY_CAPACITY 5
#define COMFY_ANIM_FRAME_CAPACITY 10
#define COMFY_ANMFILE_HEADER_BYTES 16
#define COMFY_ANMFILE_MAGIC 0x4143
#define COMFY_ANMFILE_PANTHER_MAGIC 0x4643
#define COMFY_ANMFRAME_MAGIC 0x4E41
#define COMFY_ANMFRAME_PANTHER_MAGIC 0x4C46
#define COMFY_ANMFRAME_BYTES 0xFA00
#define COMFY_ANMFRAME_COMMAND_DATA_BYTES 0x78
#define COMFY_ANM_STATE_BYTES 0x68
#define COMFY_PANTHER_ANM_STATE_BYTES 0x67
#define COMFY_ANIM_DIRTY_RECT_CAPACITY 0x3C

namespace Comfy {

class MidiPlyrDriver;
struct SoundDecoderState;

#ifdef USE_IMGUI
void onImGuiInit();
void onImGuiRender();
void onImGuiCleanup();
#endif

class ComfyEngine : public Engine {
private:
	friend struct SoundDecoderState;

	struct VideoRectRecord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
		uint32 area;

		VideoRectRecord() {
			left = 0;
			top = 0;
			right = 0;
			bottom = 0;
			area = 0;
		}
	};

	struct DrawCommand {
		int16 x;
		int16 y;
		uint32 selector;
		byte mode;
		uint16 actorIndex;

		DrawCommand() {
			x = 0;
			y = 0;
			selector = 0;
			mode = 0;
			actorIndex = 0;
		}
	};

	struct InputQueue {
		uint16 words[COMFY_INPUT_QUEUE_CAPACITY];
		uint16 readIndex;
		uint16 writeIndex;
		uint16 tailIndex;

		InputQueue() {
			for (uint i = 0; i < COMFY_INPUT_QUEUE_CAPACITY; i++)
				words[i] = 0;

			readIndex = 0;
			writeIndex = 0;
			tailIndex = 0;
		}
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

		SpriteObjectHeader() {
			fileOffset = 0;
			dataSize = 0;
			width = 0;
			height = 0;
			hotspotX = 0;
			hotspotY = 0;
			reserved = 0;
			tiledSize = 0;
		}
	};

	struct SpriteResource {
		int16 id;
		SpriteObjectHeader header;
		Common::Array<byte> pixels;
		bool loaded;

		SpriteResource() {
			id = 0;
			loaded = false;
		}
	};

	struct SpriteCacheEntry {
		uint32 poolOffset;
		uint16 slotSize;
	};

	struct ResourceLoadList {
		uint16 ids[COMFY_RESOURCE_LIST_CAPACITY];
		uint16 count;

		ResourceLoadList() {
			for (uint i = 0; i < COMFY_RESOURCE_LIST_CAPACITY; i++)
				ids[i] = 0;

			count = 0;
		}
	};

	struct VocQueueEntry {
		uint16 soundId;
		uint16 argumentCount;
		uint16 state;
		uint16 arguments[COMFY_VOC_ARG_CAPACITY];

		VocQueueEntry() {
			soundId = 0;
			argumentCount = 0;
			state = 0;

			for (uint i = 0; i < COMFY_VOC_ARG_CAPACITY; i++)
				arguments[i] = 0;
		}
	};

	struct VocQueueEntry1999 {
		uint16 soundId;
		uint16 argumentCount;
		uint16 state;
		uint16 arguments[COMFY_VOC_ARG_CAPACITY_1999];
		bool clearArgumentKeys;

		VocQueueEntry1999() {
			soundId = 0;
			argumentCount = 0;
			state = 0;
			clearArgumentKeys = false;

			for (uint i = 0; i < COMFY_VOC_ARG_CAPACITY_1999; i++)
				arguments[i] = 0;
		}
	};

	struct MidiQueueEntry {
		uint16 id;
		int32 time;

		MidiQueueEntry() {
			id = 0;
			time = 0;
		}
	};

	struct MidiQueue {
		MidiQueueEntry entries[COMFY_MIDI_QUEUE_CAPACITY];
		uint16 count;
		int32 baseTime;
		uint16 nextIndex;
		int32 nextTime;

		MidiQueue() {
			count = 0;
			baseTime = 0;
			nextIndex = 0;
			nextTime = 0;
		}
	};

	struct MidiTrackEntry {
		uint16 songId;
		uint16 completionKey;
		byte loadFlag;
		uint16 frameCount;
		uint16 frames[COMFY_ANIM_FRAME_CAPACITY];

		MidiTrackEntry() {
			songId = 0;
			completionKey = 0;
			loadFlag = 0;
			frameCount = 0;

			for (uint i = 0; i < COMFY_ANIM_FRAME_CAPACITY; i++)
				frames[i] = 0;
		}
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

		MidiChannelState() {
			entryCount = 0;
			volumeCurrent = 0;
			volumeTarget = 0;
			volumeTicksLeft = 0;
			volumeDefault = 0;
			rateCurrent = 0;
			rateTarget = 0;
			rateTicksLeft = 0;
			rateDefault = 0;
			pitchCurrent = 0;
			pitchTarget = 0;
			pitchTicksLeft = 0;
			pitchDefault = 0;
			loadedFrameSize = 0;
		}
	};

	struct SoundCue {
		uint16 value;
		uint32 streamPosition;
		uint32 counterThreshold;
	};

	struct Actor {
		byte raw[COMFY_ACTOR_SIZE_V3];
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
		kActorCachedSprite = 0x50,
		kActorBlitHitMouse = 0x56
	};

	enum ScriptDispatchStatus {
		kScriptContinue,
		kScriptYield
	};

	const ComfyGameDescription *_game;
	byte _engineVersion;
	bool _isPanther = false;
	uint16 _actorSize = COMFY_ACTOR_SIZE;
	uint16 _actorCachedVisibleOffset = kActorCachedVisible;
	uint16 _actorCachedSpriteOffset = kActorCachedSprite;
	bool _actorCachedAreaIs32Bit = false;
	uint16 _vocQueueCapacity = COMFY_VOC_QUEUE_CAPACITY;
	Common::Path _gameDirectory;
	Common::Path _introDirectory;
	Common::Path _gameDataPath;
	bool _hasDataPath = false;
	Common::Path _languageDirectories[16];
	uint16 _language = 2;
	bool _multiLanguage = true;
	Common::RandomSource _randomSource;
	Graphics::Screen *_screen = nullptr;
	byte *_framebufPtr = nullptr;
	byte *_presentBuffer = nullptr;
	Common::Array<byte> _backgroundFramebuf;
	uint16 _backgroundFrame = 0;
	uint16 _logicalScreenWidth;
	uint16 _logicalScreenHeight;
	uint16 _videoMode = 2;
	int16 _videoScale = 1;
	uint16 _screenWidth = 0;
	uint16 _screenHeight = 0;
	uint16 _renderWidth = 0;
	uint16 _renderHeight = 0;
	int16 _viewOffsetX = 0;
	int16 _viewOffsetY = 0;
	VideoRectRecord _resolutionChanges[COMFY_RESOLUTION_CHANGE_CAPACITY];
	uint16 _resolutionChangeCount = 0;
	uint16 _renderDirtyCount = 0;
	bool _renderInterleaved = false;
	Common::SeekableReadStream *_colorDatStream = nullptr;
	byte _paletteFadeSource[COMFY_PALETTE_BYTES];
	byte _paletteTarget[COMFY_PALETTE_BYTES];
	byte _paletteDisplay[COMFY_PALETTE_BYTES];
	byte _logicalPalette[256 * 4];
	byte *_paletteDataPtr = nullptr;
	uint16 _vsyncPending = 0;
	uint16 _fadeMax = 0;
	uint16 _fadeStep = 0;
	byte _palettePage = 0;
	bool _paletteFading = false;
	int16 _timerCurrent = 1;
	int16 _timer0 = 1;
	int16 _timer1 = 1;
	int16 _timer2 = 1;
	uint32 _midiTimeCounter = 0;
	uint32 _midiInstanceEventTime = 0;
	int32 _midiEventBaseTime = 0;
	int32 _midiTimeScale = 0x400;
	int16 _midiTimeDelta = 0;
	int16 _midiCounterAdjustment = 0;
	MidiPlyrDriver *_midiPlyrDriver = nullptr;
	byte *_keyBits = nullptr;
	uint32 _keyBitsSize = 0;
	InputQueue _inputQueue;
	byte _keyboardKeyToBit[256];
	bool _keyboardMapLoaded = false;
	uint32 _keyboardActiveMask = 0;
	uint32 _keyboardLatchedMask = 0;
	uint32 _keymapperActiveMask = 0;
	uint32 _keymapperLatchedMask = 0;
	uint32 _toyKeyboardActiveMask = 0;
	uint32 _toyKeyboardLatchedMask = 0;
	uint32 _toyKeyboardHoldMask = 0;
	uint32 _lptPrevScanState = 0;
	uint16 _lptPortBase = 0x0378;
	uint16 _inputDeviceMode = 0;
	uint16 _inputDevicePreference = 0;
	uint16 _inputInitialDeviceMode = 0;
	uint32 _inputPreviousComfyboardState = 0;
	uint32 _inputPreviousKeyboardState = 0;
	uint16 _inputComfyboardActivityCount = 0;
	uint16 _inputKeyboardActivityCount = 0;
	bool _inputKeyboardBit7Held = false;
	bool _inputKeyboardResetRequested = false;
	bool _inputKeyboardSyntheticBit7 = false;
	bool _inputComfyboardSuppressBit1 = false;
	uint16 _inputKeyboardPulseTicks = 0;
	bool _inputKeyboardPulseLatched = false;
	uint32 _inputKeyboardPulseTime = 0;
	bool _keyboardUiVisible = true;
	Common::Array<byte> _comfyObjData;
	Common::Array<byte> _picFileData;
	Common::Array<byte> _midiFileData;
	Common::Array<SpriteObjectHeader> _spriteHeaders;
	Common::Array<SpriteResource> _spriteResources;
	SpriteResource _frameSpriteResource;
	uint16 _stringCount = 0;
	uint16 _sceneCount = 0;
	uint16 _keyBitCount = 0;
	uint16 _resourceHandleCount = 0;
	uint16 _midiEntryCount = 0;
	uint32 _picDataSize = 0;
	bool _usesAnimFile = false;
	bool _picFileDatOpen = false;
	bool _comfyObjOpen = false;
	bool _xmsEnvAllocated = false;
	bool _soundLoaded = false;
	bool _picFileMapped = false;
	bool _sceneOpen = false;
	bool _sceneEntryListActive = false;
	Common::Array<uint16> _stringTable;
	Common::Array<uint16> _sceneHandles;
	Common::Array<uint16> _midiHandles;
	Common::Array<Actor> _actors;
	Common::Array<byte> _sceneMemoryBlock;
	Common::Array<byte> _sceneRunBuffer;
	byte *_sceneRunPtr = nullptr;
	Common::Array<byte> _scenePoolData;
	Common::Array<byte> _frameLoaderData;
	Common::Array<byte> _environmentData;
	Common::Array<byte> _headerXmsData;
	Common::Array<byte> _sceneFrameData;
	Common::Array<SpriteCacheEntry> _objectCacheEntries;
	Common::Array<SpriteCacheEntry> _frameCacheEntries;
	ResourceLoadList _spriteConversionLoads;
	SpriteObjectHeader _spriteLastHeader;
	uint16 _spriteDecompressLastSize = 0;
	bool _spriteDecompressLastComplete = false;
	uint32 _objFileOffset = 0;
	uint16 _objFileStride = 0;
	uint32 _keymaskBits = 0;
	uint16 _keymaskLastIndex = 0xFFFF;
	uint16 _keymaskCount = 0;
	uint16 _keymaskSpriteWord = 0;
	uint16 _keymaskArg0 = 0;
	uint16 _keymaskArg1 = 0;
	uint16 _keymaskResult = 0;
	int16 _keymaskX = 0;
	int16 _keymaskY = 0;
	VideoRectRecord _keymaskCurrentRecord;
	VideoRectRecord _keymaskRects[COMFY_RESOLUTION_CHANGE_CAPACITY];
	VideoRectRecord _keymaskOldRects[COMFY_RESOLUTION_CHANGE_CAPACITY];
	VideoRectRecord _keymaskInvalidationRects[COMFY_RESOLUTION_CHANGE_CAPACITY];
	VideoRectRecord _animFrameDirtyRects[COMFY_ANIM_DIRTY_RECT_CAPACITY];
	uint16 _animFrameDirtyRectCount = 0;
	DrawCommand _drawCommands[COMFY_DRAW_COMMAND_CAPACITY];
	uint16 _drawCommandCount = 0;
	uint32 _sceneEntryOffsets[COMFY_SCENE_ENTRY_OFFSET_CAPACITY];
	uint16 _sceneEntryVolumes[COMFY_SCENE_MUSIC_CHANNEL_COUNT];
	uint16 _sceneEntryCompletionKeys[COMFY_SCENE_MUSIC_CHANNEL_COUNT];
	uint32 _sceneMidiInstanceOffset = 0;
	uint32 _sceneSoundStateOffset = 0;
	uint32 _sceneEntryListOffset = 0;
	uint32 _sceneActorPcOffset = 0;
	uint32 _sceneStringTableOffset = 0;
	uint32 _sceneHandlesOffset = 0;
	uint32 _sceneActorsOffset = 0;
	uint32 _sceneKeyBitsOffset = 0;
	uint32 _sceneAnimStateOffset = 0;
	uint32 _scenePoolCursor = 0;
	uint32 _scenePoolEvictCursor = 0;
	uint32 _scenePoolSize = 0;
	uint32 _headerXmsObjectTableBase = 0;
	uint32 _headerXmsObjectTableBytes = 0;
	uint32 _headerXmsMidiEntriesBase = 0;
	uint32 _headerXmsMidiEntriesBytes = 0;
	uint32 _headerXmsSoundHeadersBase = 0;
	uint32 _headerXmsSoundHeadersBytes = 0;
	uint16 _selectorPoolEntries[10];
	uint16 _selectorRing = 0;
	bool _selectorPoolInitialized = false;
	uint16 _activeSceneCount = 0;
	uint16 _sceneEntryCount = 0;
	uint16 _sceneEntryFrameSize = 0;
	uint16 _numObjects = 0;
	uint16 _numFrames = 0;
	uint32 _numSprites = 0;
	uint32 _envNumSprites = 0;
	byte _midiFileMode = 0;
	bool _mirrorMode = false;
	uint32 _actorPcTable[COMFY_ACTOR_PC_TABLE_COUNT];
	uint16 _currentActor = 0;
	uint16 _currentScene = 0;
	uint16 _pendingScene = 0;
	uint16 _musicEventMask = 0;
	byte _musicEventFlag = 0;
	bool _musicEnabled = false;
	bool _usesWcomfy99ScriptOps = false;
	bool _languageSessionRestartRequested = false;
	int16 _mouseX = 0;
	int16 _mouseY = 0;
	bool _mouseLeftButton = false;
	bool _mouseRightButton = false;
	bool _mouseLeftButtonEdge = false;
	bool _mouseRightButtonEdge = false;
	uint16 _mouseActorSceneHandle = 0;
	Actor *_mouseActor = nullptr;
	SpriteResource *_mouseCursorSprite = nullptr;
	uint16 _mouseCursorSpriteId = 0;
	uint16 _mouseFlags = 0;
	bool _waveOutputActive = false;
	byte _wcomfy99VocState0 = 0;
	byte _wcomfy99VocState1 = 0;
	byte _wcomfy99VocState2 = 0;
	byte _wcomfy99VocState3 = 0;
	byte _wcomfy99VocState6 = 0;
	bool _wcomfy99HostMediaValueAvailable = false;
	uint16 _wcomfy99HostMediaValue = 0;
	uint16 _wcomfy99HostMediaProgress = 0;
	uint16 _wcomfy99Sensitivity = 0;
	bool _wcomfy99RecordHostEnabled = false;
	uint16 _wcomfy99SubsystemWord = 0;
	uint16 _wcomfy99MixedHostFirstWord = 0;
	uint16 _wcomfy99MixedHostSecondWord = 0;
	uint16 _wcomfy99MixedHostThirdWord = 0;
	uint16 _wcomfy99MixedHostFourthWord = 0;
	uint16 _wcomfy99HostWordA = 0;
	uint16 _wcomfy99HostWordB = 0;
	uint16 _wcomfy99WaveVolumePercent = 0;
	uint16 _wcomfy99WaveLeftPercent = 0;
	uint16 _wcomfy99WaveRightPercent = 0;
	uint16 _wcomfy99MixerVolumePercent = 0;
	uint16 _wcomfy99MixerAltPercent = 0;
	uint16 _wcomfy99RangeHostStart = 0;
	uint16 _wcomfy99RangeHostEnd = 0;
	uint16 _wcomfy99RangeHostCount = 0;
	bool _actorDestroyedCurrent = false;
	uint16 _lastKey = 0xFFFF;
	VocQueueEntry _vocQueue[COMFY_VOC_QUEUE_CAPACITY];
	VocQueueEntry1999 _vocQueue1999[COMFY_VOC_QUEUE_CAPACITY];
	uint16 _soundEventIndex = 0;
	uint16 _soundEventMaximum = 0;
	uint16 _soundEventSubIndex = 0xFFFF;
	uint16 _soundEventPreviousSubIndex = 0xFFFF;
	MidiQueue _midiEvents;
	MidiQueue _midiTracks;
	MidiChannelState _midiChannels[COMFY_MIDI_CHANNEL_COUNT];
	uint32 _midiInstanceTrackBase = 1;
	Common::Array<byte> _vocFileData;
	Common::Array<SoundCue> _soundCues;
	SoundDecoderState *_soundDecoderState = nullptr;
	Audio::SoundHandle _soundHandle;
	Audio::QueuingAudioStream *_soundQueueStream = nullptr;
	uint16 _soundWaveBufferIndex = 0;
	Common::Array<byte> _soundDecoderData;
	uint32 _soundStreamPosition = 0;
	uint32 _soundEventArgument = 0;
	int32 _soundEventLength = 0;
	uint32 _soundDataBase = 0;
	uint32 _soundDataEnd = 0;
	uint32 _soundPlayPosition = 0;
	uint32 _soundLoopPosition = 0;
	uint32 _soundCursor = 0;
	uint32 _soundBlockBase = 0;
	uint32 _soundRate = 0;
	uint16 _soundFillCount = 0;
	int16 _soundLoopCount = 0;
	byte _soundSample = 0xA5;
	byte _soundNextSample = 0;
	bool _soundStopped = true;
	bool _soundStarted = false;
	uint16 _soundBitOffset = 0;
	uint16 _soundLoopBitOffset = 0;
	uint16 _soundBlockOffset = 0;
	uint16 _soundBitAccumulator = 1;
	byte _soundBitWidth = 4;
	byte _soundPredictorDistance = 1;
	bool _soundNeedInit = false;
	Common::Array<byte> _soundPcmBlocks[2];
	uint16 _soundEntryCount = 0;
	uint32 _soundSampleRate = 0x2B11;
	uint _soundNextCue = 0;
	bool _soundCompressed = false;
	bool _soundUsesAnimData = false;
	bool _soundPaused = false;
	bool _soundTimingPaused = false;
	uint16 _soundVocBlockCount = 0;
	uint32 _soundVocCounterSnapshot = 0;
	int16 _soundVocTimingDelta = 0;
	Common::Array<byte> _animFileData;
	Common::Array<uint32> _animIndexTable;
	Common::Array<byte> _animFrameBuffer;
	Common::Array<byte> _animFrameStorage;
	uint32 _animStorageChunkFileOffsets[6];
	uint32 _animStorageChunkOffsets[6];
	uint16 _animStorageChunkSizes[6];
	byte _animFrameHeader[COMFY_ANMFILE_HEADER_BYTES];
	byte _animFrameCommandData[COMFY_ANMFRAME_COMMAND_DATA_BYTES];
	uint32 _animPosition = 0;
	uint32 _animSavedStreamPosition = 0;
	uint32 _animSoundDataPosition = 0;
	uint16 _animFrameCommandDataSize = 0;
	uint16 _animFrameCommandArgument = 0;
	bool _animFrameCommandFlag = false;
	uint16 _animCurrentIndex = 0;
	uint16 _animCurrentActorSceneHandle = 0;
	uint16 _animCurrentFrameKey = 0;
	uint16 _animVocClockHz = 0;
	uint16 _animVocTargetCounter = 0;
	uint16 _animVocDeltaB = 0;
	uint16 _animVocDeltaA = 0;
	uint32 _animVocCounterStartA = 0;
	uint32 _animVocCounterEndA = 0;
	uint32 _animVocCounterStartB = 0;
	uint32 _animVocCounterEndB = 0;
	bool _animIndexLoaded = false;
	bool _animPantherFormat = false;
	bool _animActive = false;
	bool _animFrameReady = false;
	bool _animUsesWaveVocCounter = false;
	bool _animShutdownBeforeSceneStart = false;
	uint16 _exprStack[COMFY_EXPR_STACK_CAPACITY];
	uint16 _exprStackTop = 0;
	bool _scriptFault = false;
	bool _gameInitialized = false;
	bool _videoInitialized = false;
	bool _timerInitialized = false;
	bool _lptKeyboardInitialized = false;

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
	void renderInvalidateFullFrame();
	void framebufCopyAll(byte *destination, byte *source);
	void framebufClear(byte *destination, uint16 color);
	void backgroundTransitionFrames(uint16 frame, uint16 previousFrame);
	void backgroundRestoreDirtyRects();
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
	void midiHandleAddTo(uint16 handleIndex, int16 delta);
	void midiHandleCopy(uint16 destination, uint16 source);
	void midiHandleSet(uint16 handleIndex, uint16 value);
	uint16 midiGetHandle(uint16 handleIndex);
	uint16 midiGetVersion();
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
	void midiFileReadEntries(byte *destination);
	byte *soundReadTiledData();
	void soundGetTileParams(uint16 *entryCount, uint16 *entrySize);
	void spriteCache(int16 spriteId);
	SpriteResource *spriteLookup(uint16 spriteId, int16 x, int16 y);
	void spriteInvalidateHostCache(SpriteResource &sprite);
	void objHdrReadFromXms(byte *destination, uint32 base, uint16 size, uint16 row);
	void objHdrRead(SpriteObjectHeader &destination, uint16 index);
	void scenePoolEvict();
	void scenePoolReserveSlot(uint32 size);
	SpriteResource *spriteGetPtr(int16 spriteId);
	void spriteGetConvPtr(int16 spriteId);
	bool spriteDecompressTile(SpriteResource &sprite, const byte *source, uint32 sourceSize);
	void spriteBlitClipped(int16 spriteId, int16 x, int16 y);
	bool spriteCoversPoint(int16 spriteId, int16 x, int16 y, int16 pointX, int16 pointY);
	void spriteBlitRle(byte *destination, const byte *source, uint32 sourceSize);
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
	void actorSetPos(uint16 mode, int16 value);
	Actor *actorResolve(uint16 sceneOrActor, uint16 fallbackActor);
	uint16 actorAlloc(uint16 sceneSlot);
	void actorFreeSlot(uint16 sceneSlot);
	void actorInsertChild(uint16 childIndex, uint16 parentIndex);
	void actorInsertSibling(uint16 actorIndex, uint16 ownerIndex);
	void actorUnlink(uint16 actorIndex);
	void actorSetPc(Actor &actor, uint32 pc);
	uint32 actorPopPc(Actor &actor);
	void actorFreePcChain(Actor &actor);
	void actorFreeTreePc(uint16 actorIndex);
	void actorClearDirtyTree(uint16 actorIndex);
	void actorSetAllVisible();
	void actorClearFirstAnimFrameSelector();
	uint16 actorInit(uint16 sceneSlot, uint16 parentSlot, byte visible, byte active,
		uint32 pc, int16 x, int16 y, int16 sprite, byte insertAsChild);
	bool actorRunScript(uint16 actorIndex, bool &descendChildren);
	bool actorTickTree(uint16 actorIndex);
	bool actorDraw(uint16 actorIndex, int16 x, int16 y);
	void actorDrawInternal(uint16 actorIndex, int16 x, int16 y, bool visible);
	bool actorDrawLegacyInternal(uint16 actorIndex, int16 x, int16 y);
	void actorDrawList(uint16 actorIndex, int16 x, int16 y, bool visible);
	void renderQueueDrawCommand(int16 x, int16 y, uint32 selector, byte mode, uint16 actorIndex);
	void renderFlushDrawCommands();
	void renderFlushCachedDirtyRects();
	uint16 scriptEvalKeyMask(uint32 pc, uint16 mode, VideoRectRecord &maskRecord,
		VideoRectRecord *rects, int16 baseX, int16 baseY);
	void actorEvalFrameSelection(uint16 actorIndex, int16 x, int16 y);
	VideoRectRecord actorReadCachedRect(Actor &actor);
	void actorWriteCachedRect(Actor &actor, VideoRectRecord rect);
	void timerInit();
	void timerShutdown();
	void lptKeyboardInit();
	void lptKeyboardShutdown();
	void gameMainLoop(uint16 argument);
	uint16 sceneRun(uint16 sceneId, bool checkNext, bool exitFlag);
	void processEvents();
	void mouseSetActor(Actor *actor);
	Actor *mouseGetActor();
	void mouseClearButtonEdges();
	void mouseUpdateCursor();

	uint16 timerTick();
	void midiTrackTickAndRemove();
	void animFileTickCommands();
	bool animFileOpen();
	void animFrameShutdown(bool freeBuffers);
	void animFileShutdown();
	void animFileLoadFrame(uint16 animIndex, uint16 frameKey, uint16 actorSceneHandle);
	uint32 animDecompressRle(const byte *source, uint32 sourceSize, byte *destination, uint32 destinationSize);
	uint32 animDecodePanther(const byte *source, uint32 sourceSize);
	bool animFrameIsPending();
	bool animFrameShouldDraw(uint16 phase);
	int16 animFrameVocCounterDelta();
	void animFrameRecordVocCounter(uint16 phase);
	void animFrameWaitForVocCounter();
	void animFrameSetReady(bool ready);
	bool animFrameBlitAt(int16 x, int16 y);
	bool animFrameCoversPoint(int16 x, int16 y, int16 pointX, int16 pointY);
	void animFrameInvalidateRects(int16 x, int16 y, byte mode);
	bool animFrameGetDimensions(uint16 &width, uint16 &height);
	void animFrameClear();
	void animFilePackState(byte *state);
	void animFileRestoreStreamPosition(const byte *state);
	void animFileRebuildStorage(uint32 targetSize);
	void animFileUnpackState(const byte *state);
	void animFrameInvalidateActorRect();
	bool animFrameIsActive();
	bool animFrameIsReady();
	void animFrameReadStorage(byte *destination, uint32 offset, uint16 size);
	void animFilePushStorageChunk(uint32 fileOffset, uint32 storageOffset, uint16 size);
	void animFrameSetVocCounter(uint32 counter);
	void sceneTickEvent();
	void sceneStartWithMusic(uint16 scene);
	void midiPollChannels(uint16 ticks);
	void midiInitInstanceAt();
	void midiInitInstance();
	void midiInitChannels();
	void midiSyncAndScan();
	void midiFindNext(MidiQueue &queue);
	void midiQueueAdd(MidiQueue &queue, uint16 id, int16 delta);
	bool midiQueueRemove(MidiQueue &queue, uint16 id);
	void midiAddEvent(uint16 id, int16 delta);
	void midiAddTrack(uint16 id, int16 delta);
	void midiRemoveTrack(uint16 id);
	void midiAddTrackEntry(uint16 channel, uint16 songId, uint16 completionKey, byte loadFlag,
		uint16 frameCount, uint16 *frames);
	void midiClearChannel(uint16 channel);
	void midiFireClearMarker(uint16 channel, int16 marker);
	void midiPollSceneEntries();
	void midiResumeAll();
	void midiStopChannel(uint16 channel);
	void midiStopAll();
	void midiStopSong(uint16 channel);
	void midiPlaySongAtFrame(uint16 channel, uint16 frame);
	void sceneEntrySetVolume(uint16 channel, uint16 volume);
	void midiFinishChannel(uint16 channel);
	void midiStopAndFireKeys(uint16 channel);
	void midiStopAndRemove(uint16 channel);
	void midiSetChannelParam(uint16 channel, byte parameter, uint16 value, uint16 ticks);
	int16 midiApproachTarget(int16 current, int16 target, int16 &ticksLeft, int16 ticks);
	void musicSetEnabled(byte value);
	void paletteFadeStep(uint16 ticks);
	void lptKeyboardScanAndProcess();
	bool vocQueuePush(uint16 soundId, uint16 argumentCount, uint32 pc);
	void vocQueuePlayAll();
	bool vocQueueIsIdle();
	void wcomfy99SetWaveBalancePercent(uint16 value);
	void wcomfy99SetWaveLeftPercent(uint16 value);
	void wcomfy99SetWaveRightPercent(uint16 value);
	void wcomfy99SetMixerVolumePercent(uint16 value);
	void wcomfy99SetHostMediaRangePercent(uint16 value);
	void wcomfy99SetHostMediaMode(byte mode);
	void wcomfy99RestoreHostStateAfterSceneStart();
	bool soundInit();
	void soundShutdown();
	void soundHdrReadFromXms(byte *destination, uint16 index, uint16 size);
	void soundUpdateVocTiming();
	void soundServiceWaveBuffers();
	bool soundLoadEntry(uint16 index);
	bool soundPrepareDecoderState(uint16 index);
	bool soundDecodePcmBlock(uint16 bufferIndex, byte *&buffer, uint32 &decodedSize);
	bool soundQueuePcmBlock(uint16 bufferIndex);
	void soundPackState(byte *state);
	void soundUnpackState(byte *state);
	void soundPlayEntry(uint16 index);
	void soundAdvanceTick();

protected:
	// Engine APIs
	Common::Error run() override;

public:
	ComfyEngine(OSystem *syst, const ComfyGameDescription *gameDesc);
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

	bool _keyboardUiInitialized = false;
};

extern ComfyEngine *g_engine;
#define SHOULD_QUIT ::Comfy::g_engine->shouldQuit()

} // End of namespace Comfy

#endif // COMFY_H
