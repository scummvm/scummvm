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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media.h"

#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "audio/audiostream.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/iavf.h"
#include "ripper/input.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/script.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const int kScenePresentationTop = 50;
static const uint kBlockingAudioCursor = 0x13;
static const uint16 kHelpCommand = 0x3b00;
static const uint kAutoPacketizedDisplayScale = 0;
static const uint kPaletteFadeStepDelayMs = 16;

struct SavedDisplayContext {
	Common::Array<byte> pixels;
	Common::Array<byte> palette;
	uint width;
	uint height;

	SavedDisplayContext() : width(0), height(0) {}
};

static bool readExact(Common::SeekableReadStream &stream, void *data, uint32 size) {
	return stream.read(data, size) == size;
}

static bool captureDisplayContext(SavedDisplayContext &context) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	context.width = screen->w;
	context.height = screen->h;
	context.pixels.resize(context.width * context.height);
	for (uint y = 0; y < context.height; ++y)
		memcpy(context.pixels.data() + y * context.width, screen->getBasePtr(0, y), context.width);
	g_system->unlockScreen();

	context.palette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(context.palette.data(), 0, 256);
	return true;
}

static bool restoreDisplayContext(const SavedDisplayContext &context) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
		(uint)screen->w != context.width || (uint)screen->h != context.height) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	for (uint y = 0; y < context.height; ++y)
		memcpy(screen->getBasePtr(0, y), context.pixels.data() + y * context.width, context.width);
	g_system->unlockScreen();
	g_system->getPaletteManager()->setPalette(context.palette.data(), 0, 256);
	g_system->updateScreen();
	return true;
}

static Common::String audioKeyFromPath(const Common::String &path) {
	uint start = 0;
	for (uint i = 0; i < path.size(); ++i) {
		if (path[i] == '/' || path[i] == '\\' || path[i] == ':')
			start = i + 1;
	}
	uint end = path.size();
	for (uint i = start; i < path.size(); ++i) {
		if (path[i] == '.') {
			end = i;
			break;
		}
	}
	return path.substr(start, end - start);
}

} // End of anonymous namespace

MediaPlayer::AudioSlot::AudioSlot() : volumePercent(100), targetVolumePercent(100),
		triggerFrame(0), volumeStartFrame(0), volumeTiming(0), volumeRampStep(0),
		volumeRampProgress(0), volumeRampDirection(0), control(0), occupied(false),
		preserve(false), volumeRampPending(false), sparseVolumeRamp(false) {
}

MediaPlayer::MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer) :
		_engine(engine), _input(input), _mixer(mixer), _stopSceneOnMouse(false) {
}

MediaPlayer::~MediaPlayer() {
	for (uint i = 0; i < kAudioSlotCount; ++i)
		_mixer->stopHandle(_audioSlots[i].handle);
}

void MediaPlayer::fadePalette(bool fadeIn, uint stepCount) {
	if (stepCount == 0)
		return;

	byte targetPalette[Graphics::PALETTE_SIZE];
	byte fadePalette[Graphics::PALETTE_SIZE];
	PaletteManager *paletteManager = g_system->getPaletteManager();
	paletteManager->grabPalette(targetPalette, 0, Graphics::PALETTE_COUNT);

	for (uint step = 1; step <= stepCount; ++step) {
		const uint scale = fadeIn ? step : stepCount - step;
		for (uint component = 0; component < Graphics::PALETTE_SIZE; ++component)
			fadePalette[component] = (byte)((uint64)targetPalette[component] * scale / stepCount);
		paletteManager->setPalette(fadePalette, 0, Graphics::PALETTE_COUNT);
		g_system->updateScreen();
		if (step != stepCount)
			g_system->delayMillis(kPaletteFadeStepDelayMs);
	}
}

bool MediaPlayer::loadAudio(const Common::String &path, bool preserve) {
	Common::File file;
	if (!file.open(Common::Path(path))) {
		warning("Ripper: could not load audio '%s'", path.c_str());
		return false;
	}

	uint slotIndex = 0;
	while (slotIndex < kAudioSlotCount && _audioSlots[slotIndex].occupied)
		++slotIndex;
	if (slotIndex == kAudioSlotCount) {
		debugC(2, kDebugAudio,
			"Ripper: audio slot load ignored path='%s' reason=all-20-slots-occupied slots=[%s]",
			path.c_str(), describeAudioSlots().c_str());
		return true;
	}

	AudioSlot &slot = _audioSlots[slotIndex];
	slot = AudioSlot();
	slot.path = path;
	slot.key = audioKeyFromPath(path);
	slot.occupied = true;
	slot.preserve = preserve;
	uint occupiedCount = 0;
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		if (_audioSlots[i].occupied)
			++occupiedCount;
	}
	debugC(2, kDebugAudio,
		"Ripper: loaded audio slot=%u key='%s' path='%s' preserve=%d occupied=%u/%u",
		slotIndex, slot.key.c_str(), slot.path.c_str(), preserve,
		occupiedCount, kAudioSlotCount);
	return true;
}

MediaPlayer::AudioSlot *MediaPlayer::findAudioSlot(const Common::String &key) {
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		if (_audioSlots[i].occupied && _audioSlots[i].key.equalsIgnoreCase(key))
			return &_audioSlots[i];
	}
	return nullptr;
}

const MediaPlayer::AudioSlot *MediaPlayer::findAudioSlot(const Common::String &key) const {
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		if (_audioSlots[i].occupied && _audioSlots[i].key.equalsIgnoreCase(key))
			return &_audioSlots[i];
	}
	return nullptr;
}

Common::String MediaPlayer::describeAudioSlots() const {
	Common::String description;
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		if (!_audioSlots[i].occupied)
			continue;
		if (!description.empty())
			description += ",";
		description += Common::String::format("%u:%s", i, _audioSlots[i].key.c_str());
	}
	return description;
}

bool MediaPlayer::startAudioSlot(AudioSlot &slot) {
	if (_mixer->isSoundHandleActive(slot.handle))
		return true;
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(slot.path))) {
		warning("Ripper: could not start audio slot key='%s' path='%s' slots=[%s]",
			slot.key.c_str(), slot.path.c_str(), describeAudioSlots().c_str());
		delete file;
		return false;
	}
	Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	if (!wavStream)
		return false;
	Audio::AudioStream *stream = wavStream;
	if ((slot.control & 1) != 0)
		stream = Audio::makeLoopingAudioStream(wavStream, 0);
	const byte volume = (byte)(slot.volumePercent * Audio::Mixer::kMaxChannelVolume / 100);
	// StartAudioTriggerSlot at 0x37297 uses control bit 0 for indefinitely
	// repeating scene beds. Route those entries through the Ambient profile;
	// one-shot trigger slots use the SFX profile.
	const Audio::Mixer::SoundType soundType = (slot.control & 1) ?
		Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;
	_mixer->playStream(soundType, &slot.handle, stream, -1, volume);
	debugC(1, kDebugAudio,
		"Ripper: started audio slot key='%s' path='%s' volume=%u trigger=%u control=%u loop=%d active=%d",
		slot.key.c_str(), slot.path.c_str(), slot.volumePercent, slot.triggerFrame,
		slot.control, (slot.control & 1) != 0, _mixer->isSoundHandleActive(slot.handle));
	return true;
}

bool MediaPlayer::configureAudio(const Common::String &key, uint volumePercent,
		uint triggerFrame, byte control) {
	AudioSlot *slot = findAudioSlot(key);
	if (!slot) {
		debugC(2, kDebugAudio,
			"Ripper: audio configuration ignored key='%s' volume=%u trigger=%u control=%u reason=slot-not-found slots=[%s]",
			key.c_str(), volumePercent, triggerFrame, control, describeAudioSlots().c_str());
		return true;
	}
	slot->volumePercent = MIN<uint>(volumePercent == 0 ? 100 : volumePercent, 100);
	slot->targetVolumePercent = slot->volumePercent;
	slot->triggerFrame = triggerFrame;
	slot->control = control;
	debugC(2, kDebugAudio,
		"Ripper: configured audio slot key='%s' volume=%u trigger=%u control=%u loop=%d immediate=%d",
		key.c_str(), slot->volumePercent, triggerFrame, control, (control & 1) != 0,
		triggerFrame == 0);
	return triggerFrame != 0 || startAudioSlot(*slot);
}

void MediaPlayer::clearAudioSlot(AudioSlot &slot) {
	if (!slot.occupied)
		return;
	_mixer->stopHandle(slot.handle);
	slot = AudioSlot();
}

void MediaPlayer::clearAudio(const Common::String &key) {
	AudioSlot *slot = findAudioSlot(key);
	if (!slot) {
		debugC(2, kDebugAudio, "Ripper: audio slot clear ignored key='%s' reason=slot-not-found slots=[%s]",
			key.c_str(), describeAudioSlots().c_str());
		return;
	}
	debugC(2, kDebugAudio, "Ripper: cleared audio slot key='%s'", key.c_str());
	clearAudioSlot(*slot);
}

void MediaPlayer::stopAudio(const Common::String &key) {
	AudioSlot *slot = findAudioSlot(key);
	if (!slot) {
		debugC(2, kDebugAudio, "Ripper: audio slot stop ignored key='%s' reason=slot-not-found slots=[%s]",
			key.c_str(), describeAudioSlots().c_str());
		return;
	}
	const bool active = _mixer->isSoundHandleActive(slot->handle);
	_mixer->stopHandle(slot->handle);
	debugC(2, kDebugAudio, "Ripper: stopped audio slot key='%s' active=%d retained=1",
		key.c_str(), active);
}

void MediaPlayer::applyAudioSlotVolume(AudioSlot &slot) {
	if (_mixer->isSoundHandleActive(slot.handle)) {
		const byte volume = (byte)(slot.volumePercent * Audio::Mixer::kMaxChannelVolume / 100);
		_mixer->setChannelVolume(slot.handle, volume);
	}
}

void MediaPlayer::setAudioVolume(const Common::String &key, uint targetVolumePercent,
		uint startFrame, uint timing) {
	AudioSlot *slot = findAudioSlot(key);
	if (!slot) {
		debugC(2, kDebugAudio,
			"Ripper: audio volume change ignored key='%s' target=%u start=%u timing=%u reason=slot-not-found slots=[%s]",
			key.c_str(), targetVolumePercent, startFrame, timing, describeAudioSlots().c_str());
		return;
	}
	slot->targetVolumePercent = MIN<uint>(targetVolumePercent, 100);
	slot->volumeStartFrame = startFrame;
	slot->volumeTiming = timing;
	if (startFrame == 0) {
		slot->volumePercent = slot->targetVolumePercent;
		slot->volumeRampPending = false;
		applyAudioSlotVolume(*slot);
		debugC(2, kDebugAudio, "Ripper: applied audio volume key='%s' volume=%u immediately",
			key.c_str(), slot->volumePercent);
		return;
	}

	slot->volumeRampDirection = slot->targetVolumePercent < slot->volumePercent ? -1 : 1;
	const uint difference = slot->targetVolumePercent < slot->volumePercent ?
		slot->volumePercent - slot->targetVolumePercent :
		slot->targetVolumePercent - slot->volumePercent;
	slot->volumeRampStep = difference;
	slot->volumeRampProgress = 0;
	slot->sparseVolumeRamp = difference <= timing;
	if (!slot->sparseVolumeRamp) {
		slot->volumeRampStep = timing;
		slot->volumeTiming = difference;
	}
	slot->volumeRampPending = difference != 0;
	debugC(2, kDebugAudio,
		"Ripper: scheduled audio volume key='%s' current=%u target=%u start=%u timing=%u step=%u sparse=%d",
		key.c_str(), slot->volumePercent, slot->targetVolumePercent, startFrame,
		slot->volumeTiming, slot->volumeRampStep, slot->sparseVolumeRamp);
}

void MediaPlayer::serviceSceneAudio(uint frame) {
	// RunMediaSequence at 0x1e516 publishes one-based frame counters to
	// ServiceSceneFrameAudioAndBriefingTriggers at 0x138c9 after each frame.
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		AudioSlot &slot = _audioSlots[i];
		if (!slot.occupied)
			continue;
		if (slot.triggerFrame == frame && !_mixer->isSoundHandleActive(slot.handle)) {
			debugC(3, kDebugAudio, "Ripper: audio frame trigger slot=%u key='%s' frame=%u",
				i, slot.key.c_str(), frame);
			if (!startAudioSlot(slot))
				warning("Ripper: audio frame trigger failed slot=%u key='%s' frame=%u",
					i, slot.key.c_str(), frame);
		}
		if (!slot.volumeRampPending || frame < slot.volumeStartFrame)
			continue;
		if (slot.volumeTiming == 0) {
			slot.volumePercent = slot.targetVolumePercent;
		} else if (slot.sparseVolumeRamp) {
			slot.volumeRampProgress += slot.volumeRampStep;
			if (slot.volumeRampProgress >= slot.volumeTiming) {
				slot.volumeRampProgress -= slot.volumeTiming;
				slot.volumePercent += slot.volumeRampDirection;
			}
		} else {
			do {
				slot.volumeRampProgress += slot.volumeRampStep;
				slot.volumePercent += slot.volumeRampDirection;
				if (slot.volumePercent == slot.targetVolumePercent)
					break;
			} while (slot.volumeRampProgress < slot.volumeTiming);
			if (slot.volumeRampProgress >= slot.volumeTiming)
				slot.volumeRampProgress -= slot.volumeTiming;
		}
		applyAudioSlotVolume(slot);
		if (slot.volumePercent == slot.targetVolumePercent) {
			slot.volumeRampPending = false;
			debugC(3, kDebugAudio,
				"Ripper: completed audio volume ramp slot=%u key='%s' frame=%u volume=%u",
				i, slot.key.c_str(), frame, slot.volumePercent);
		}
	}
}

void MediaPlayer::resetSceneAudioTriggers() {
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		_audioSlots[i].triggerFrame = 0;
		_audioSlots[i].volumeStartFrame = 0;
		_audioSlots[i].volumeRampPending = false;
	}
	debugC(3, kDebugAudio, "Ripper: reset per-frame audio triggers slots=[%s]",
		describeAudioSlots().c_str());
}

void MediaPlayer::clearSceneAudio(bool includePreserved) {
	uint cleared = 0;
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		if (_audioSlots[i].occupied && (includePreserved || !_audioSlots[i].preserve)) {
			clearAudioSlot(_audioSlots[i]);
			++cleared;
		}
	}
	debugC(2, kDebugAudio,
		"Ripper: scene transition cleared audio slots=%u includePreserved=%d retained=[%s]",
		cleared, includePreserved, describeAudioSlots().c_str());
}

bool MediaPlayer::isSceneAudioActive() const {
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		if (_audioSlots[i].occupied && _mixer->isSoundHandleActive(_audioSlots[i].handle))
			return true;
	}
	return false;
}

bool MediaPlayer::syncGame(Common::Serializer &serializer) {
	if (serializer.getVersion() <= 2) {
		Common::String audioPath;
		byte active = 0;
		uint32 volumePercent = 100;
		byte loop = 0;
		serializer.syncString(audioPath);
		serializer.syncAsByte(active);
		serializer.syncAsUint32LE(volumePercent);
		serializer.syncAsByte(loop);
		if (serializer.err() || audioPath.size() > 256 || volumePercent > 100)
			return false;
		clearSceneAudio(true);
		if (!audioPath.empty() && !loadAudio(audioPath, false))
			return false;
		AudioSlot *slot = audioPath.empty() ? nullptr : &_audioSlots[0];
		if (slot) {
			slot->volumePercent = volumePercent;
			slot->targetVolumePercent = volumePercent;
			slot->control = loop != 0 ? 1 : 0;
		}
		if (active != 0 && slot && !startAudioSlot(*slot))
			return false;
		debugC(1, kDebugSaveLoad,
			"Ripper: restored legacy scene audio path='%s' active=%d volume=%u loop=%d",
			audioPath.c_str(), active != 0, volumePercent, loop != 0);
		return true;
	}

	if (serializer.isLoading())
		clearSceneAudio(true);
	uint restoredSlots = 0;
	for (uint i = 0; i < kAudioSlotCount; ++i) {
		AudioSlot &slot = _audioSlots[i];
		Common::String path = serializer.isSaving() ? slot.path : Common::String();
		byte occupied = slot.occupied ? 1 : 0;
		byte preserve = slot.preserve ? 1 : 0;
		byte active = slot.occupied && _mixer->isSoundHandleActive(slot.handle) ? 1 : 0;
		uint32 volumePercent = slot.volumePercent;
		uint32 targetVolumePercent = slot.targetVolumePercent;
		uint32 triggerFrame = slot.triggerFrame;
		uint32 volumeStartFrame = slot.volumeStartFrame;
		uint32 volumeTiming = slot.volumeTiming;
		uint32 volumeRampStep = slot.volumeRampStep;
		uint32 volumeRampProgress = slot.volumeRampProgress;
		int32 volumeRampDirection = slot.volumeRampDirection;
		byte control = slot.control;
		byte volumeRampPending = slot.volumeRampPending ? 1 : 0;
		byte sparseVolumeRamp = slot.sparseVolumeRamp ? 1 : 0;

		serializer.syncString(path);
		serializer.syncAsByte(occupied);
		serializer.syncAsByte(preserve);
		serializer.syncAsByte(active);
		serializer.syncAsUint32LE(volumePercent);
		serializer.syncAsUint32LE(targetVolumePercent);
		serializer.syncAsUint32LE(triggerFrame);
		serializer.syncAsUint32LE(volumeStartFrame);
		serializer.syncAsUint32LE(volumeTiming);
		serializer.syncAsUint32LE(volumeRampStep);
		serializer.syncAsUint32LE(volumeRampProgress);
		serializer.syncAsSint32LE(volumeRampDirection);
		serializer.syncAsByte(control);
		serializer.syncAsByte(volumeRampPending);
		serializer.syncAsByte(sparseVolumeRamp);
		if (serializer.err() || path.size() > 256 || (occupied != 0 && path.empty()) ||
				occupied > 1 || preserve > 1 ||
				active > 1 || volumePercent > 100 || targetVolumePercent > 100 ||
				volumeRampDirection < -1 || volumeRampDirection > 1 ||
				volumeRampPending > 1 || sparseVolumeRamp > 1)
			return false;
		if (serializer.isSaving() || occupied == 0)
			continue;
		Common::File file;
		if (!file.open(Common::Path(path)))
			return false;
		AudioSlot &restored = _audioSlots[i];
		restored.path = path;
		restored.key = audioKeyFromPath(path);
		restored.occupied = true;
		restored.preserve = preserve != 0;
		restored.volumePercent = volumePercent;
		restored.targetVolumePercent = targetVolumePercent;
		restored.triggerFrame = triggerFrame;
		restored.volumeStartFrame = volumeStartFrame;
		restored.volumeTiming = volumeTiming;
		restored.volumeRampStep = volumeRampStep;
		restored.volumeRampProgress = volumeRampProgress;
		restored.volumeRampDirection = volumeRampDirection;
		restored.control = control;
		restored.volumeRampPending = volumeRampPending != 0;
		restored.sparseVolumeRamp = sparseVolumeRamp != 0;
		if (active != 0 && !startAudioSlot(restored))
			return false;
		++restoredSlots;
	}
	if (serializer.isLoading()) {
		debugC(1, kDebugSaveLoad,
			"Ripper: restored named audio slots=%u active=%d slots=[%s]",
			restoredSlots, isSceneAudioActive(), describeAudioSlots().c_str());
	}
	return !serializer.err();
}

bool MediaPlayer::servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool allowSegmentAdvance, bool &paused, bool toolbarPaused, bool &skipToEnd,
		bool &advanceSegment,
		Audio::SoundHandle *externalAudio, bool suppressSceneMouseStop, bool allowSceneHelp) {
	skipToEnd = false;
	advanceSegment = false;
	if (_input->pollEvents()) {
		_engine->quitGame();
		return false;
	}
	if (allowSceneHelp && _input->peekKey() == kHelpCommand) {
		_input->consumeKey();
		decoder.pauseVideo(true);
		if (externalAudio)
			_mixer->pauseHandle(*externalAudio, true);
		const bool helpDisplayed = _engine->getScripts()->showHelp("interactive-media");
		const bool effectivePause = paused || toolbarPaused;
		decoder.pauseVideo(effectivePause);
		if (externalAudio)
			_mixer->pauseHandle(*externalAudio, effectivePause);
		return helpDisplayed;
	}
	if (!allowEscSpace || !_input->hasPendingKey())
		return !(_stopSceneOnMouse && !suppressSceneMouseStop &&
			_input->peekMouseState().pressed != 0);

	const uint16 command = _input->consumeKey();
	if (command == 0x1b) {
		skipToEnd = true;
		debugC(2, kDebugVideo, "Ripper: Escape advanced skippable presentation to end");
		return false;
	}
	if (command == 0x20) {
		paused = !paused;
		const bool effectivePause = paused || toolbarPaused;
		decoder.pauseVideo(effectivePause);
		if (externalAudio)
			_mixer->pauseHandle(*externalAudio, effectivePause);
		debugC(2, kDebugVideo, "Ripper: Space %s presentation", paused ? "paused" : "resumed");
	}
	if (command == 0x4d00 && allowSegmentAdvance) {
		advanceSegment = true;
		debugC(2, kDebugVideo, "Ripper: Right Arrow requested next IAVF segment");
		return false;
	}
	return true;
}

bool MediaPlayer::playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		const SmackerPlaybackRequest &request) {
	const bool allowEscSpace = request.allowEscSpace;
	int x = request.x;
	int y = request.y;
	Audio::SoundHandle *externalAudio = request.externalAudio;
	bool *stoppedByUser = request.stoppedByUser;
	const Common::Array<uint32> *frameAudioOffsets = request.frameAudioOffsets;
	const uint32 audioByteRate = request.audioByteRate;
	const uint32 timelineStartMillis = request.timelineStartMillis;
	uint displayScale = request.displayScale;
	const bool patchInterfacePalette = request.patchInterfacePalette;
	const uint frameLimit = request.frameLimit;
	const int originY = request.originY;
	const bool patchWacMediaPalette = request.patchWacMediaPalette;
	const bool serviceSceneUi = request.serviceSceneUi;
	const bool repeatedLoopPass = request.repeatedLoopPass;
	bool *advanceSegment = request.advanceSegment;
	const uint loopStartFrame = request.loopStartFrame;
	MediaSequenceCallback *sequenceCallback = request.sequenceCallback;
	uint16 *sequenceCommand = request.sequenceCommand;
	Common::Array<byte> *sourcePalette = request.sourcePalette;
	const bool rememberVideoPalette = request.rememberVideoPalette;
	const uint firstFrame = request.firstFrame;
	uint lastFrame = request.lastFrame;
	const uint boundedLoopStartFrame = request.boundedLoopStartFrame;
	const bool transparentFirstPixel = request.transparentFirstPixel;
	if (stoppedByUser)
		*stoppedByUser = false;
	if (advanceSegment)
		*advanceSegment = false;
	if (sequenceCommand)
		*sequenceCommand = 0;
	Video::SmackerDecoder decoder;
	if (!decoder.loadStream(stream)) {
		warning("Ripper: invalid Smacker stream '%s'", name.c_str());
		return false;
	}
	const bool boundedSegment = firstFrame != 0 || lastFrame != 0xffffffff;
	const bool boundedLoop = boundedLoopStartFrame != 0xffffffff;
	if (lastFrame == 0xffffffff)
		lastFrame = decoder.getFrameCount() - 1;
	if (decoder.getFrameCount() == 0 || firstFrame > lastFrame ||
			lastFrame >= decoder.getFrameCount() ||
			(boundedLoop && (!boundedSegment || boundedLoopStartFrame < firstFrame ||
				boundedLoopStartFrame > lastFrame))) {
		warning("Ripper: invalid Smacker segment '%s' frames=%u..%u count=%u",
			name.c_str(), firstFrame, lastFrame, decoder.getFrameCount());
		return false;
	}
	// InitializeMediaPresentationDisplayModeCallback at 0x163a8 is invoked for
	// each packetized branch, not only for the IAVF container dimensions. In the
	// original 640x400 mode every branch smaller than 321x201 receives the 2:1
	// display descriptor, including PROLOG2.AVI's 320x200 branches inside a
	// 640x400 container.
	if (displayScale == kAutoPacketizedDisplayScale)
		displayScale = decoder.getHeight() < 0xc9 && decoder.getWidth() < 0x141 ? 2 : 1;
	const uint outputWidth = decoder.getWidth() * displayScale;
	const uint outputHeight = decoder.getHeight() * displayScale;
	if (x < 0)
		x = (g_system->getWidth() - outputWidth) / 2;
	else
		x *= displayScale;
	if (y < 0)
		y = (g_system->getHeight() - outputHeight) / 2;
	else
		y = y * displayScale + originY;
	debugC(repeatedLoopPass ? 3 : 1, kDebugVideo,
		"Ripper: playing Smacker '%s' frames=%u source=%ux%u output=%ux%u at %d,%d controls=%d sceneUi=%d frameLimit=%u",
		name.c_str(), decoder.getFrameCount(), decoder.getWidth(), decoder.getHeight(),
		outputWidth, outputHeight, x, y, allowEscSpace, serviceSceneUi, frameLimit);
	debugC(repeatedLoopPass ? 3 : 2, kDebugVideo, "Ripper: Smacker '%s' interfacePalettePatch=%d",
		name.c_str(), patchInterfacePalette);
	if (patchWacMediaPalette)
		debugC(2, kDebugVideo, "Ripper: Smacker '%s' WAC palette patch=10..149", name.c_str());
	const bool synchronizeToTimeline = frameAudioOffsets &&
		frameAudioOffsets->size() == decoder.getFrameCount() && audioByteRate != 0;
	if (synchronizeToTimeline) {
		debugC(2, kDebugVideo, "Ripper: Smacker '%s' follows IAVF timeline %u..%u ms clock=%s",
			name.c_str(),
			(uint32)((uint64)(*frameAudioOffsets)[0] * 1000 / audioByteRate),
			(uint32)((uint64)frameAudioOffsets->back() * 1000 / audioByteRate),
			externalAudio ? "mixer" : "system");
	}

	bool paused = false;
	bool toolbarPaused = false;
	bool completed = true;
	uint presentedFrames = 0;
	Common::Array<byte> transparentBacking;
	if (transparentFirstPixel) {
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1 || x < 0 || y < 0 ||
				x + (int)outputWidth > screen->w || y + (int)outputHeight > screen->h) {
			if (screen)
				g_system->unlockScreen();
			warning("Ripper: transparent Smacker overlay '%s' is outside the indexed display",
				name.c_str());
			decoder.close();
			return false;
		}
		transparentBacking.resize(outputWidth * outputHeight);
		for (uint row = 0; row < outputHeight; ++row)
			memcpy(transparentBacking.data() + row * outputWidth,
				screen->getBasePtr(x, y + row), outputWidth);
		g_system->unlockScreen();
		debugC(2, kDebugVideo,
			"Ripper: captured transparent Smacker backing media='%s' rect=%d,%d,%ux%u",
			name.c_str(), x, y, outputWidth, outputHeight);
	}
	auto applyDecoderPalette = [&](bool forcePalette) {
		if (forcePalette || decoder.hasDirtyPalette()) {
			byte palette[256 * 3];
			if (sourcePalette) {
				sourcePalette->resize(sizeof(palette));
				memcpy(sourcePalette->data(), decoder.getPalette(), sizeof(palette));
			}
			if (patchWacMediaPalette) {
				g_system->getPaletteManager()->grabPalette(palette, 0, 256);
				memcpy(palette + 10 * 3, decoder.getPalette() + 10 * 3, 140 * 3);
			} else {
				memcpy(palette, decoder.getPalette(), sizeof(palette));
			}
			if (patchInterfacePalette && !patchWacMediaPalette)
				_engine->getToolbar()->applySharedPalettePatch(palette, 256);
			if (!patchWacMediaPalette)
				_engine->getSettings()->applyVideoPalette(palette, 256,
					rememberVideoPalette);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
		}
	};
	auto presentFrame = [&](const Graphics::Surface *frame, bool forcePalette) {
		applyDecoderPalette(forcePalette);
		if (transparentFirstPixel) {
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen) {
				warning("Ripper: could not lock display for transparent Smacker overlay '%s'",
					name.c_str());
				completed = false;
				return;
			}
			for (uint row = 0; row < outputHeight; ++row)
				memcpy(screen->getBasePtr(x, y + row),
					transparentBacking.data() + row * outputWidth, outputWidth);
			const byte transparentColor = *(const byte *)frame->getPixels();
			for (uint sourceY = 0; sourceY < (uint)frame->h; ++sourceY) {
				const byte *source = (const byte *)frame->getBasePtr(0, sourceY);
				for (uint sourceX = 0; sourceX < (uint)frame->w; ++sourceX) {
					if (source[sourceX] == transparentColor)
						continue;
					for (uint scaleY = 0; scaleY < displayScale; ++scaleY) {
						byte *destination = (byte *)screen->getBasePtr(
							x + sourceX * displayScale,
							y + sourceY * displayScale + scaleY);
						memset(destination, source[sourceX], displayScale);
					}
				}
			}
			g_system->unlockScreen();
		} else if (displayScale == 1) {
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
		} else {
			Graphics::Surface *screen = g_system->lockScreen();
			Graphics::scaleBlit((byte *)screen->getBasePtr(x, y),
				(const byte *)frame->getPixels(), screen->pitch, frame->pitch,
				outputWidth, outputHeight, frame->w, frame->h, frame->format);
			g_system->unlockScreen();
		}
		if (serviceSceneUi) {
			_engine->getScripts()->drawDialogueOverlay(true);
			_engine->getScripts()->drawBriefingOverlay();
		}
		// Sequence callbacks may add puzzle or dialogue overlays. Defer their
		// screen submission until the callback completes so an undecorated movie
		// frame is never visible between the frame blit and overlay composition.
		if (!sequenceCallback)
			g_system->updateScreen();
	};
	// ExecutePresentationEntry at 0x1652a deactivates the shared selection
	// presentation before packetized AVI playback. Only RunMediaSequence's
	// scene-frame callback path keeps cursor, toolbar, and dialogue controls live.
	if (!serviceSceneUi)
		_engine->getCursor()->setVisible(false);
	decoder.start();
	bool sequencePaletteRefresh = false;
	auto serviceSequenceCallback = [&](uint frame) {
		if (!sequenceCallback)
			return false;
		decoder.pauseVideo(true);
		const uint16 command = sequenceCallback->service(frame);
		decoder.pauseVideo(false);
		if (command == 0)
			return false;
		if (command == MediaSequenceCallback::kContinueRefreshPalette) {
			sequencePaletteRefresh = true;
			debugC(3, kDebugVideo,
				"Ripper: interactive Smacker '%s' requested palette refresh frame=%u",
				name.c_str(), frame);
			return false;
		}
		if (sequenceCommand)
			*sequenceCommand = command;
		debugC(2, kDebugVideo,
			"Ripper: interactive Smacker '%s' returned command=0x%04x frame=%u",
			name.c_str(), command, frame);
		return true;
	};
	auto finishSequenceFramePresentation = [&](uint frame) {
		const bool stopSequence = serviceSequenceCallback(frame);
		const bool refreshedPalette = sequencePaletteRefresh;
		if (sequencePaletteRefresh) {
			applyDecoderPalette(true);
			sequencePaletteRefresh = false;
		}
		if (sequenceCallback && sequenceCallback->managesPalette()) {
			byte palette[256 * 3];
			// RunCombatEncounterScene at 0x31436 derives its temporary hit and
			// shield palettes from the active Smacker palette on every frame.
			// Rebuild that base before asking the encounter callback to transform it.
			applyDecoderPalette(true);
			g_system->getPaletteManager()->grabPalette(palette, 0, 256);
			sequenceCallback->transformPalette(palette, 256);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
		}
		g_system->updateScreen();
		debugC(11, kDebugVideo,
			"Ripper: presented interactive Smacker '%s' after callback frame=%u paletteRefresh=%d stop=%d",
			name.c_str(), frame, refreshedPalette, stopSequence);
		return stopSequence;
	};
	if (firstFrame != 0) {
		// RunTubeSwitchScene at 0x25e18 advances a newly loaded GA_TUBE decoder
		// through frames 0..14 before presenting frame 15. Decode from the start
		// here as well so delta-coded pixels and palette entries are based on the
		// complete initial surface rather than a blank seek surface.
		const Graphics::Surface *frame = nullptr;
		while (decoder.getCurFrame() < (int)firstFrame && !decoder.endOfVideo())
			frame = decoder.decodeNextFrame();
		if (!frame || decoder.getCurFrame() != (int)firstFrame) {
			warning("Ripper: could not advance Smacker '%s' to bounded segment frame=%u",
				name.c_str(), firstFrame);
			decoder.close();
			return false;
		}
		presentFrame(frame, true);
		++presentedFrames;
		debugC(3, kDebugVideo,
			"Ripper: entered bounded Smacker segment '%s' frame=%u..%u",
			name.c_str(), firstFrame, lastFrame);
		if (sequenceCallback && finishSequenceFramePresentation(firstFrame + 1)) {
			decoder.close();
			return true;
		}
		if (boundedSegment && firstFrame == lastFrame) {
			decoder.close();
			return true;
		}
	}
	while (!_engine->shouldQuit()) {
		if (decoder.endOfVideo()) {
			if (sequenceCallback && sequenceCallback->continueAfterEnd()) {
				// Async scene choosers keep servicing input over the final decoded
				// frame. HandleSceneEntryAsyncTextRequest at 0x157a1 does not stop
				// RunMediaSequence while the chooser owns the callback continuation.
				if (finishSequenceFramePresentation(decoder.getFrameCount()))
					break;
				g_system->delayMillis(10);
				continue;
			}
			if (!sequenceCallback || loopStartFrame == 0)
				break;
			if (loopStartFrame > decoder.getFrameCount()) {
				warning("Ripper: Smacker '%s' loop start frame=%u exceeds frame count=%u",
					name.c_str(), loopStartFrame, decoder.getFrameCount());
				completed = false;
				break;
			}
			const Graphics::Surface *frame = decoder.forceSeekToFrame(loopStartFrame - 1);
			if (!frame) {
				warning("Ripper: could not seek Smacker '%s' to loop start frame=%u",
					name.c_str(), loopStartFrame);
				completed = false;
				break;
			}
			presentFrame(frame, true);
			++presentedFrames;
			debugC(3, kDebugVideo,
				"Ripper: looped interactive Smacker '%s' to frame=%u",
				name.c_str(), loopStartFrame);
			if (finishSequenceFramePresentation(loopStartFrame))
				break;
			continue;
		}
		// ExecuteSceneFrameAndInteractions at 0x13277 passes
		// PollInteractionAndResolveSelection at 0x13c8d as RunMediaSequence's
		// per-frame callback. RunFrontEndActionMenu blocks that callback while the
		// pointer remains in the toolbar band, so no Smacker frame advances.
		bool playbackUiFailed = false;
		const bool toolbarOwnsInput = serviceSceneUi &&
			_engine->getScripts()->updateInteractiveCursor(
				_input->peekMouseState().position, &playbackUiFailed);
		if (playbackUiFailed) {
			completed = false;
			break;
		}
		if (serviceSceneUi && _engine->getScripts()->hasPendingSceneTransition()) {
			debugC(1, kDebugScene,
				"Ripper: interactive media '%s' returning queued scene transition",
				name.c_str());
			break;
		}
		if (toolbarOwnsInput != toolbarPaused) {
			toolbarPaused = toolbarOwnsInput;
			const bool effectivePause = paused || toolbarPaused;
			decoder.pauseVideo(effectivePause);
			if (externalAudio)
				_mixer->pauseHandle(*externalAudio, effectivePause);
			debugC(2, kDebugVideo,
				"Ripper: interactive scene media '%s' toolbarPause=%d keyboardPause=%d",
				name.c_str(), toolbarPaused, paused);
			g_system->updateScreen();
		}
		bool skipToEnd = false;
		bool advanceToNextSegment = false;
		const bool callbackOwnsInput = sequenceCallback && sequenceCallback->ownsInput();
		if (!servicePlaybackInput(decoder, allowEscSpace && !callbackOwnsInput,
				advanceSegment != nullptr,
				paused, toolbarPaused, skipToEnd, advanceToNextSegment,
				externalAudio, toolbarOwnsInput, serviceSceneUi)) {
			completed = false;
			if (advanceToNextSegment) {
				if (advanceSegment)
					*advanceSegment = true;
				completed = true;
			}
			if (skipToEnd && decoder.getFrameCount() != 0) {
				const uint finalFrame = decoder.getFrameCount() - 1;
				const Graphics::Surface *frame = decoder.forceSeekToFrame(finalFrame);
				if (frame) {
					presentFrame(frame, true);
					if (sequenceCallback)
						g_system->updateScreen();
					if (serviceSceneUi)
						serviceSceneAudio(finalFrame + 1);
					completed = true;
					debugC(2, kDebugVideo,
						"Ripper: Escape presented final Smacker frame '%s' frame=%u; completing presentation",
						name.c_str(), finalFrame);
				}
			}
			if (!advanceToNextSegment && stoppedByUser && !_engine->shouldQuit())
				*stoppedByUser = true;
			break;
		}
		uint32 audioElapsedMs = 0;
		uint32 targetAudioMs = 0;
		bool frameDue = !synchronizeToTimeline && decoder.needsUpdate();
		if (synchronizeToTimeline && !paused && !toolbarPaused) {
			const uint32 nextFrame = decoder.getCurFrame() + 1;
			targetAudioMs = (uint32)((uint64)(*frameAudioOffsets)[nextFrame] * 1000 / audioByteRate);
			if (externalAudio && _mixer->isSoundHandleActive(*externalAudio))
				audioElapsedMs = _mixer->getSoundElapsedTime(*externalAudio);
			else
				audioElapsedMs = g_system->getMillis(true) - timelineStartMillis;
			frameDue = audioElapsedMs >= targetAudioMs;
		}
		if (!paused && !toolbarPaused && frameDue) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (frame) {
				presentFrame(frame, false);
				++presentedFrames;
				if (serviceSceneUi)
					serviceSceneAudio(presentedFrames);
				if (sequenceCallback &&
						finishSequenceFramePresentation(decoder.getCurFrame() + 1))
					break;
				if (boundedSegment && (uint)decoder.getCurFrame() >= lastFrame) {
					if (boundedLoop) {
						// SeekSmackerPlaybackFrame in RunTubeSwitchScene reuses the
						// active decoder at frame 46 and resumes from frame 15. Keeping
						// this decoder alive preserves its delta surface and palette.
						const Graphics::Surface *loopFrame =
							decoder.forceSeekToFrame(boundedLoopStartFrame);
						if (!loopFrame) {
							warning("Ripper: could not loop bounded Smacker '%s' to frame=%u",
								name.c_str(), boundedLoopStartFrame);
							completed = false;
							break;
						}
						presentFrame(loopFrame, true);
						++presentedFrames;
						debugC(3, kDebugVideo,
							"Ripper: looped bounded Smacker '%s' frame=%u after endFrame=%u",
							name.c_str(), boundedLoopStartFrame, lastFrame);
						if (sequenceCallback &&
								finishSequenceFramePresentation(boundedLoopStartFrame + 1))
							break;
						continue;
					}
					debugC(2, kDebugVideo,
						"Ripper: completed bounded Smacker segment '%s' frame=%u..%u",
						name.c_str(), firstFrame, lastFrame);
					break;
				}
				if (synchronizeToTimeline) {
					debugC(11, kDebugVideo,
						"Ripper: Smacker '%s' frame=%d audioTargetMs=%u audioElapsedMs=%u driftMs=%d",
						name.c_str(), decoder.getCurFrame(), targetAudioMs, audioElapsedMs,
						(int32)audioElapsedMs - (int32)targetAudioMs);
				} else {
					debugC(11, kDebugVideo, "Ripper: Smacker '%s' frame=%d", name.c_str(), decoder.getCurFrame());
				}
				if (frameLimit != 0 && presentedFrames >= frameLimit) {
					debugC(2, kDebugVideo,
						"Ripper: stopped Smacker '%s' after bounded preview frame=%u",
						name.c_str(), presentedFrames);
					break;
				}
			}
		}
		if (frameLimit != 0 && presentedFrames >= frameLimit)
			break;
		if (toolbarPaused) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		} else if (synchronizeToTimeline) {
			if (!paused && !toolbarPaused && !frameDue)
				g_system->delayMillis(MIN<uint32>(targetAudioMs - audioElapsedMs, 10));
			else
				g_system->delayMillis(1);
		} else {
			g_system->delayMillis(MIN<uint32>(decoder.getTimeToNextFrame(), 10));
		}
	}
	if (synchronizeToTimeline && completed) {
		const uint32 elapsedMs = externalAudio && _mixer->isSoundHandleActive(*externalAudio) ?
			_mixer->getSoundElapsedTime(*externalAudio) : g_system->getMillis(true) - timelineStartMillis;
		debugC(2, kDebugVideo, "Ripper: completed IAVF Smacker '%s' targetMs=%u audioElapsedMs=%u",
			name.c_str(),
			(uint32)((uint64)frameAudioOffsets->back() * 1000 / audioByteRate),
			elapsedMs);
	}
	if (transparentFirstPixel && !transparentBacking.empty()) {
		for (uint row = 0; row < outputHeight; ++row)
			g_system->copyRectToScreen(transparentBacking.data() + row * outputWidth,
				outputWidth, x, y + row, outputWidth, 1);
		g_system->updateScreen();
		debugC(2, kDebugVideo,
			"Ripper: restored transparent Smacker backing media='%s' rect=%d,%d,%ux%u",
			name.c_str(), x, y, outputWidth, outputHeight);
	}
	decoder.close();
	return completed;
}

bool MediaPlayer::playIavf(Common::SeekableReadStream &stream, const Common::String &name,
		bool allowEscSpace, int overrideX, int overrideY, int overrideOriginY,
		bool serviceSceneUi) {
	IavfMovie movie;
	if (!parseIavf(stream, name, movie)) {
		warning("Ripper: invalid IAVF presentation '%s'", name.c_str());
		return false;
	}

	Audio::SoundHandle audioHandle;
	bool audioActive = false;
	uint32 audioTimelineOffset = 0;
	uint32 timelineStartMillis = g_system->getMillis(true);
	auto startAudioAtOffset = [&](uint32 offset) {
		if (audioActive)
			_mixer->stopHandle(audioHandle);
		audioActive = false;
		if (movie.audio.empty())
			return true;
		if (offset >= movie.audio.size())
			return false;
		Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(
			movie.audio.data() + offset, movie.audio.size() - offset, movie.sampleRate,
			Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
		if (!audioStream)
			return false;
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &audioHandle, audioStream);
		audioActive = _mixer->isSoundHandleActive(audioHandle);
		audioTimelineOffset = offset;
		timelineStartMillis = g_system->getMillis(true);
		debugC(2, kDebugVideo,
			"Ripper: started IAVF audio rate=%u offset=%u bytes=%u active=%d",
			movie.sampleRate, offset, movie.audio.size() - offset, audioActive);
		return true;
	};
	if (!startAudioAtOffset(0))
		return false;

	bool result = true;
	const uint32 audioByteRate = movie.audioByteRate;
	bool completedFinalSegment = false;
	for (uint i = 0; i < movie.segments.size() && !_engine->shouldQuit(); ++i) {
		if (movie.segments[i].clearDisplayBefore) {
			// RunPacketizedMediaPlaybackCore at 0x5b592 handles IAVF opcode 0x68 by
			// clearing the active logical page through display command 0x14.
			g_system->fillScreen(0);
			g_system->updateScreen();
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' cleared display before segment=%u from opcode 0x68",
				name.c_str(), i);
		}
		Common::SeekableReadStream *smacker = rebuildSmackerStream(movie.segments[i]);
		const int segmentX = overrideX != -1 ? overrideX : movie.segments[i].x;
		const int segmentY = overrideY != -1 ? overrideY : movie.segments[i].y;
		const int segmentOriginY = overrideY != -1 ? overrideOriginY : 0;
		if (overrideX != -1 || overrideY != -1) {
			// PreparePacketizedMediaPlaybackBranchSetup at 0x5b237 replaces each
			// packetized branch coordinate independently when the caller supplies
			// an override. Scene-space Y receives the retained viewport origin once.
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' segment=%u coordinate override raw=%d,%d caller=%d,%d originY=%d effective=%d,%d",
				name.c_str(), i, movie.segments[i].x, movie.segments[i].y,
				overrideX, overrideY, segmentOriginY, segmentX,
				segmentY + segmentOriginY);
		}
		bool stoppedByUser = false;
		bool advanceSegment = false;
		Common::Array<uint32> relativeAudioOffsets;
		const Common::Array<uint32> *frameAudioOffsets = &movie.segments[i].frameAudioOffsets;
		if (audioTimelineOffset != 0) {
			relativeAudioOffsets.reserve(frameAudioOffsets->size());
			for (uint frame = 0; frame < frameAudioOffsets->size(); ++frame) {
				relativeAudioOffsets.push_back((*frameAudioOffsets)[frame] >= audioTimelineOffset ?
					(*frameAudioOffsets)[frame] - audioTimelineOffset : 0);
			}
			frameAudioOffsets = &relativeAudioOffsets;
		}
		SmackerPlaybackRequest request;
		request.allowEscSpace = allowEscSpace;
		request.x = segmentX;
		request.y = segmentY;
		request.externalAudio = audioActive ? &audioHandle : nullptr;
		request.stoppedByUser = &stoppedByUser;
		request.frameAudioOffsets = frameAudioOffsets;
		request.audioByteRate = audioByteRate;
		request.timelineStartMillis = timelineStartMillis;
		request.displayScale = kAutoPacketizedDisplayScale;
		request.patchInterfacePalette = false;
		request.originY = segmentOriginY;
		request.serviceSceneUi = serviceSceneUi;
		request.advanceSegment = &advanceSegment;
		if (!smacker || !playSmacker(smacker,
				Common::String::format("%s#%u", name.c_str(), i), request)) {
			result = false;
			break;
		}
		if (stoppedByUser)
			break;
		if (_engine->getScripts()->hasPendingSceneTransition())
			break;
		if (advanceSegment) {
			if (i + 1 >= movie.segments.size()) {
				debugC(1, kDebugVideo,
					"Ripper: Right Arrow completed final IAVF segment '%s#%u'",
					name.c_str(), i);
				break;
			}
			const IavfSegment &nextSegment = movie.segments[i + 1];
			if (nextSegment.frameAudioOffsets.empty() ||
					!startAudioAtOffset(nextSegment.frameAudioOffsets[0])) {
				warning("Ripper: could not seek IAVF audio for '%s' segment=%u",
					name.c_str(), i + 1);
				result = false;
				break;
			}
			debugC(1, kDebugVideo,
				"Ripper: Right Arrow advanced IAVF '%s' segment=%u->%u audioOffset=%u",
				name.c_str(), i, i + 1, audioTimelineOffset);
			continue;
		}
		completedFinalSegment = i + 1 == movie.segments.size();
	}
	// RunPacketizedMediaPlaybackCore at 0x5b592 does not tear down the
	// presentation when opcode 0x70 ends packet dispatch. It waits until
	// GetManagedAudioTriggerActiveDescriptor reports that the managed-audio
	// tail has completed. Some IAVF files, including KA_BOOK.AVI, have several
	// seconds of audio after their final rendered frame.
	if (result && completedFinalSegment && audioActive &&
			!_engine->shouldQuit() && !_engine->getScripts()->hasPendingSceneTransition() &&
			_mixer->isSoundHandleActive(audioHandle)) {
		const uint32 targetAudioMs = (uint32)((uint64)(movie.audio.size() - audioTimelineOffset) *
			1000 / audioByteRate);
		debugC(2, kDebugVideo,
			"Ripper: retaining final IAVF display '%s' for managed-audio tail targetMs=%u elapsedMs=%u",
			name.c_str(), targetAudioMs, _mixer->getSoundElapsedTime(audioHandle));
		bool paused = false;
		bool skipped = false;
		while (!_engine->shouldQuit() && _mixer->isSoundHandleActive(audioHandle)) {
			if (_input->pollEvents()) {
				_engine->quitGame();
				result = false;
				break;
			}
			if (allowEscSpace && _input->hasPendingKey()) {
				const uint16 command = _input->consumeKey();
				if (command == 0x1b || command == 0x4d00) {
					skipped = true;
					debugC(2, kDebugVideo,
						"Ripper: %s completed final IAVF managed-audio tail '%s'",
						command == 0x1b ? "Escape" : "Right Arrow", name.c_str());
					break;
				}
				if (command == 0x20) {
					paused = !paused;
					_mixer->pauseHandle(audioHandle, paused);
					debugC(2, kDebugVideo, "Ripper: Space %s IAVF managed-audio tail '%s'",
						paused ? "paused" : "resumed", name.c_str());
				}
			}
			g_system->delayMillis(10);
		}
		if (paused)
			_mixer->pauseHandle(audioHandle, false);
		debugC(2, kDebugVideo,
			"Ripper: completed final IAVF managed-audio tail '%s' targetMs=%u elapsedMs=%u skipped=%d",
			name.c_str(), targetAudioMs, _mixer->getSoundElapsedTime(audioHandle), skipped);
	}
	if (audioActive)
		_mixer->stopHandle(audioHandle);
	if (movie.clearDisplayAfter && completedFinalSegment) {
		g_system->fillScreen(0);
		g_system->updateScreen();
		debugC(2, kDebugVideo,
			"Ripper: IAVF '%s' cleared display after final segment from opcode 0x68",
			name.c_str());
	}
	return result;
}

bool MediaPlayer::play(const Common::String &path, bool allowEscSpace, int x, int y,
		bool sceneViewport) {
	// ExecutePresentationEntry at 0x1652a routes WAV entries to
	// PlayBlockingAudioClip at 0x1f0ea before considering either video path.
	// The original blocking-audio loop always permits Escape, independently of
	// the presentation control argument used by AVI and Smacker playback.
	if (path.hasSuffixIgnoreCase(".wav")) {
		debugC(2, kDebugAudio,
			"Ripper: dispatching media presentation '%s' as blocking audio",
			path.c_str());
		const bool result = playBlockingAudio(path);
		_input->drainKeys();
		return result;
	}

	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open media '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[8];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);

	// ExecutePresentationEntry at 0x1652a passes its keyboard-control flag to
	// RunMediaPresentation at 0x168af. Controlled IAVF media redraws the saved
	// logical page afterward; uncontrolled media leaves its final frame visible.
	const bool isSmacker = memcmp(magic, "SMK2", 4) == 0 || memcmp(magic, "SMK4", 4) == 0;
	const bool isIavf = memcmp(magic, "IAVF2.00", 8) == 0;
	const bool restoreIavfDisplay = isIavf && allowEscSpace;
	SavedDisplayContext displayContext;
	const bool displayContextCaptured = restoreIavfDisplay && captureDisplayContext(displayContext);
	if (isIavf) {
		debugC(2, kDebugVideo,
			"Ripper: IAVF display policy media='%s' keyboardControls=%d restore=%d captured=%d size=%ux%u",
			path.c_str(), allowEscSpace, restoreIavfDisplay, displayContextCaptured,
			displayContext.width, displayContext.height);
	}

	bool result = false;
	if (isSmacker) {
		// RunMediaSequence at 0x1e516 draws direct scene-script Smackers against
		// the active scene display descriptor. Its logical y=0 is the top of the
		// 640x300 scene page, which begins at physical y=50 in ScummVM's retained
		// 640x400 framebuffer.
		const int originY = sceneViewport ? kScenePresentationTop : 0;
		SmackerPlaybackRequest request;
		request.allowEscSpace = allowEscSpace;
		request.x = x;
		request.y = y;
		request.originY = originY;
		result = playSmacker(file, path, request);
	} else if (isIavf) {
		const int originY = sceneViewport ? kScenePresentationTop : 0;
		result = playIavf(*file, path, allowEscSpace, x, y, originY);
		delete file;
	} else {
		warning("Ripper: unsupported media magic for '%s'", path.c_str());
		delete file;
	}
	if (displayContextCaptured && !_engine->shouldQuit()) {
		const bool restored = restoreDisplayContext(displayContext);
		debugC(restored ? 2 : 1, kDebugVideo,
			"Ripper: restored script media display context media='%s' success=%d size=%ux%u",
			path.c_str(), restored, displayContext.width, displayContext.height);
		if (!restored)
			result = false;
	} else if (isIavf && !restoreIavfDisplay && result && !_engine->shouldQuit()) {
		debugC(2, kDebugVideo,
			"Ripper: retained final IAVF display media='%s' keyboardControls=%d",
			path.c_str(), allowEscSpace);
	}

	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacMedia(const Common::String &path, int x, int y) {
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open WAC media '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[4];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);
	if (memcmp(magic, "SMK2", 4) != 0 && memcmp(magic, "SMK4", 4) != 0) {
		warning("Ripper: unsupported WAC media magic for '%s'", path.c_str());
		delete file;
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering WAC media presentation media='%s' position=%d,%d palette=10..149",
		path.c_str(), x, y);
	SmackerPlaybackRequest request;
	request.x = x;
	request.y = y;
	request.patchInterfacePalette = false;
	request.patchWacMediaPalette = true;
	const bool result = playSmacker(file, path, request);
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacInterfaceSequence(const Common::String &path, int x, int y,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command) {
	Common::SeekableReadStream *stream =
		_engine->getResources()->interface().createReadStreamForMember(path);
	if (!stream) {
		warning("Ripper: could not open WAC interface media '%s'", path.c_str());
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering WAC interface sequence media='%s' position=%d,%d palette=10..149 loopStartFrame=%u callback=%d",
		path.c_str(), x, y, loopStartFrame, callback != nullptr);
	SmackerPlaybackRequest request;
	request.x = x;
	request.y = y;
	request.patchInterfacePalette = false;
	request.patchWacMediaPalette = true;
	request.loopStartFrame = loopStartFrame;
	request.sequenceCallback = callback;
	request.sequenceCommand = command;
	return playSmacker(stream, path, request);
}

bool MediaPlayer::playInterfaceSequence(const Common::String &path, int x, int y,
		Common::Array<byte> &sourcePalette) {
	Common::SeekableReadStream *stream =
		_engine->getResources()->interface().createReadStreamForMember(path);
	if (!stream) {
		warning("Ripper: could not open interface media '%s'", path.c_str());
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering interface media presentation media='%s' position=%d,%d",
		path.c_str(), x, y);
	SmackerPlaybackRequest request;
	request.x = x;
	request.y = y;
	request.sourcePalette = &sourcePalette;
	request.rememberVideoPalette = false;
	return playSmacker(stream, path, request);
}

bool MediaPlayer::playBlockingAudio(const Common::String &path) {
	Common::SeekableReadStream *audioStream = nullptr;
	Common::String source;
	Common::File *file = new Common::File();
	if (file->open(Common::Path(path))) {
		audioStream = file;
		source = "filesystem";
	} else {
		delete file;
		ResourceManager *resources = _engine->getResources();
		if (resources && resources->sound().hasMember(path)) {
			audioStream = resources->sound().createReadStreamForMember(path);
			source = "sound-library";
		}
	}
	if (!audioStream) {
		warning("Ripper: could not open blocking audio '%s' from the filesystem or sound library",
			path.c_str());
		return false;
	}
	Audio::SeekableAudioStream *stream = Audio::makeWAVStream(audioStream, DisposeAfterUse::YES);
	if (!stream)
		return false;

	Audio::SoundHandle handle;
	// PlayBlockingAudioClip at 0x1f0ea is part of the same presentation path as
	// packetized dialogue/video audio, which the Remote Control names VIDEO VOL.
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, stream);
	_engine->getCursor()->update(kBlockingAudioCursor);
	g_system->updateScreen();
	debugC(2, kDebugAudio,
		"Ripper: started blocking audio '%s' source=%s cursor=%u input=keyboard-only presentation=serviced",
		path.c_str(), source.c_str(), kBlockingAudioCursor);
	bool stoppedByEscape = false;
	while (!_engine->shouldQuit() && _mixer->isSoundHandleActive(handle)) {
		_engine->getCursor()->update(kBlockingAudioCursor);
		if (_input->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_input->hasPendingKey()) {
			const uint16 command = _input->consumeKey();
			if (command == 0x1b) {
				stoppedByEscape = true;
				break;
			}
		}
		if (stoppedByEscape)
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	_mixer->stopHandle(handle);
	_input->discardMouseTransitions();
	debugC(2, kDebugAudio,
		"Ripper: completed blocking audio '%s' source=%s stoppedByEscape=%d",
		path.c_str(), source.c_str(), stoppedByEscape);
	return !_engine->shouldQuit();
}

bool MediaPlayer::playSoundEffect(const Common::String &path, Audio::SoundHandle &handle,
		uint volumePercent, bool loop) {
	Common::SeekableReadStream *audioStream = nullptr;
	Common::String source;
	Common::File *file = new Common::File();
	if (file->open(Common::Path(path))) {
		audioStream = file;
		source = "filesystem";
	} else {
		delete file;
		ResourceManager *resources = _engine->getResources();
		if (resources && resources->sound().hasMember(path)) {
			audioStream = resources->sound().createReadStreamForMember(path);
			source = "sound-library";
		}
	}
	if (!audioStream) {
		warning("Ripper: could not open sound effect '%s' from the filesystem or sound library",
			path.c_str());
		return false;
	}

	Audio::SeekableAudioStream *stream = Audio::makeWAVStream(audioStream, DisposeAfterUse::YES);
	if (!stream)
		return false;
	stopSoundEffect(handle);
	const byte volume = (byte)(MIN<uint>(volumePercent, 100) * Audio::Mixer::kMaxChannelVolume / 100);
	Audio::AudioStream *playbackStream = stream;
	if (loop)
		playbackStream = Audio::makeLoopingAudioStream(stream, 0);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, playbackStream, -1, volume);
	debugC(2, kDebugAudio,
		"Ripper: started sound effect '%s' source=%s volume=%u loop=%d",
		path.c_str(), source.c_str(), volumePercent, loop);
	return true;
}

bool MediaPlayer::stopSoundEffect(Audio::SoundHandle &handle) {
	const bool active = _mixer->isSoundHandleActive(handle);
	if (active)
		_mixer->stopHandle(handle);
	return active;
}

void MediaPlayer::setSoundEffectVolume(Audio::SoundHandle &handle, uint volumePercent) {
	_mixer->setChannelVolume(handle,
		(byte)(MIN<uint>(volumePercent, 100) * Audio::Mixer::kMaxChannelVolume / 100));
}

bool MediaPlayer::isSoundEffectActive(const Audio::SoundHandle &handle) const {
	return _mixer->isSoundHandleActive(handle);
}

bool MediaPlayer::playPuzzleSequence(const Common::String &path, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command) {
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open puzzle media sequence '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[4];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);
	if (memcmp(magic, "SMK2", 4) != 0 && memcmp(magic, "SMK4", 4) != 0) {
		warning("Ripper: unsupported puzzle media sequence '%s'", path.c_str());
		delete file;
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering puzzle Smacker sequence media='%s' loopStartFrame=%u callback=%d",
		path.c_str(), loopStartFrame, callback != nullptr);
	SmackerPlaybackRequest request;
	request.x = 0;
	request.y = 0;
	request.originY = kScenePresentationTop;
	request.loopStartFrame = loopStartFrame;
	request.sequenceCallback = callback;
	request.sequenceCommand = command;
	return playSmacker(file, path, request);
}

bool MediaPlayer::playPuzzleSequenceSegment(const Common::String &path, uint firstFrame,
		uint lastFrame, int x, int y, MediaSequenceCallback *callback, uint16 *command,
		uint boundedLoopStartFrame) {
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open puzzle media segment '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[4];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);
	if (memcmp(magic, "SMK2", 4) != 0 && memcmp(magic, "SMK4", 4) != 0) {
		warning("Ripper: unsupported puzzle media segment '%s'", path.c_str());
		delete file;
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering puzzle Smacker segment media='%s' frames=%u..%u loopStart=%d position=%d,%d callback=%d",
		path.c_str(), firstFrame, lastFrame,
		boundedLoopStartFrame == 0xffffffff ? -1 : (int)boundedLoopStartFrame,
		x, y, callback != nullptr);
	SmackerPlaybackRequest request;
	request.x = x;
	request.y = y;
	request.originY = kScenePresentationTop;
	request.sequenceCallback = callback;
	request.sequenceCommand = command;
	request.firstFrame = firstFrame;
	request.lastFrame = lastFrame;
	request.boundedLoopStartFrame = boundedLoopStartFrame;
	return playSmacker(file, path, request);
}

bool MediaPlayer::playCombatSequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open combat media sequence '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[4];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);
	if (memcmp(magic, "SMK2", 4) != 0 && memcmp(magic, "SMK4", 4) != 0) {
		warning("Ripper: unsupported combat media sequence '%s'", path.c_str());
		delete file;
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering combat Smacker sequence media='%s' callback=%d",
		path.c_str(), callback != nullptr);
	SmackerPlaybackRequest request;
	request.x = 0;
	request.y = 0;
	request.displayScale = kAutoPacketizedDisplayScale;
	request.patchInterfacePalette = false;
	request.sequenceCallback = callback;
	request.sequenceCommand = command;
	request.rememberVideoPalette = false;
	return playSmacker(file, path, request);
}

bool MediaPlayer::playTransparentSmackerOverlay(const Common::String &path, int x, int y) {
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open transparent Smacker overlay '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[4];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);
	if (memcmp(magic, "SMK2", 4) != 0 && memcmp(magic, "SMK4", 4) != 0) {
		warning("Ripper: unsupported transparent Smacker overlay '%s'", path.c_str());
		delete file;
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering transparent Smacker overlay media='%s' position=%d,%d",
		path.c_str(), x, y);
	SmackerPlaybackRequest request;
	request.x = x;
	request.y = y;
	request.originY = kScenePresentationTop;
	request.transparentFirstPixel = true;
	return playSmacker(file, path, request);
}

bool MediaPlayer::playScene(const Common::String &path, int x, int y, bool firstFrameOnly,
		bool loopUntilInput, bool allowEscSpace, MediaSequenceCallback *callback,
		uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering scene presentation media='%s' firstFrameOnly=%d loopUntilInput=%d controls=%d callback=%d",
		path.c_str(), firstFrameOnly, loopUntilInput, allowEscSpace, callback != nullptr);
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open scene media '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[8];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);

	// HandleSceneEntryMediaPreviewOrPrompt at 0x15b03 supplies a target of one to
	// MediaSequenceCounterEqualsTarget at 0x15ac8. RunMediaSequence at 0x1e516
	// calls it after presenting the frame, so this path retains frame one onscreen.
	const uint frameLimit = firstFrameOnly ? 1 : 0;
	debugC(2, kDebugVideo,
		"Ripper: scene media '%s' mode=%s scriptPosition=%d,%d originY=%d controls=%d",
		path.c_str(), firstFrameOnly ? "first-frame-preview" : "sequence", x, y,
		kScenePresentationTop, allowEscSpace);
	bool result = false;
	_stopSceneOnMouse = loopUntilInput && !firstFrameOnly;
	const bool loop = loopUntilInput && !firstFrameOnly;
	uint pass = 0;
	do {
		const bool repeatedLoopPass = pass++ != 0;
		if (repeatedLoopPass) {
			file = new Common::File();
			if (!file->open(Common::Path(path)))
				return false;
		}
	if (memcmp(magic, "SMK2", 4) == 0 || memcmp(magic, "SMK4", 4) == 0) {
		SmackerPlaybackRequest request;
		request.allowEscSpace = allowEscSpace;
		request.x = x;
		request.y = y;
		request.frameLimit = frameLimit;
		request.originY = kScenePresentationTop;
		request.serviceSceneUi = true;
		request.repeatedLoopPass = repeatedLoopPass;
		request.sequenceCallback = callback;
		request.sequenceCommand = command;
		result = playSmacker(file, path, request);
		if (!result && _stopSceneOnMouse && _input->peekMouseState().pressed != 0) {
			result = true;
			debugC(1, kDebugVideo,
				"Ripper: interactive scene media stopped by mouse; returning to hotspot polling");
		}
	} else if (memcmp(magic, "IAVF2.00", 8) == 0 && !firstFrameOnly) {
		if (callback) {
			warning("Ripper: scene callback is unsupported for packetized media '%s'",
				path.c_str());
			delete file;
			result = false;
			break;
		}
		result = playIavf(*file, path, allowEscSpace, x, y,
			kScenePresentationTop, true);
		delete file;
	} else {
		warning("Ripper: unsupported scene media mode for '%s'", path.c_str());
		delete file;
	}
	} while (loop && !_input->peekMouseState().pressed && !_engine->shouldQuit() && result &&
		!_engine->getScripts()->hasPendingSceneTransition());
	_input->drainKeys();
	_stopSceneOnMouse = false;
	return result;
}

} // End of namespace Ripper
