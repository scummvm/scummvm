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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE3110_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE3110_H

#include "common/array.h"

#include "hollywood/graphics.h"
#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene3110 {
public:
	Scene3110(HollywoodEngine *vm);
	~Scene3110();

	bool play();

private:
	struct SpriteTrack {
		uint chunkIndex;
		uint descriptorCount;
		const byte *frameMap;
		uint frameMapSize;
		uint firstTick;
		uint frameMillis;
		bool visibleBeforeStart;
		bool holdLastFrame;
	};

	struct SoundCue {
		uint tick;
		uint slot;
		uint16 cueId;
		byte volumePercent;
		bool loop;
		bool stop;
	};

	struct MachineRoomState {
		byte fixedOverlayFrame;
		byte leftMemoryActorFrame;
		byte rightMemoryActorFrame;
		byte leftArcFrame;
		byte rightArcFrame;
		byte finalOverlayFrame;
		uint32 fixedOverlayAccumulator;
		uint32 leftMemoryActorAccumulator;
		uint32 rightMemoryActorAccumulator;
		uint32 leftArcAccumulator;
		uint32 rightArcAccumulator;
		uint32 finalOverlayAccumulator;
		uint32 finalCounterAccumulator;
		uint32 paletteBandAccumulator;
		uint32 palettePulseAccumulator;
		uint finalCounter;
		uint paletteBandPhase;
		bool initialGateDirty;
		bool finalOverlayDirty;
	};

	bool loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize = 0);
	bool loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize);
	bool loadFramebufferAndPalette(uint framebufferChunk, uint paletteChunk, uint paletteReadSize = kPaletteSize);
	bool prepareScene(uint framebufferChunk, uint paletteChunk, uint paletteReadSize,
		const SpriteTrack *tracks, uint trackCount);
	void runLongBranch();
	void runShortBranch();
	void runExteriorLightningSequence();
	void runExteriorStormReturnSequence();
	void drawExteriorFrame(byte foregroundFrameIndex, byte bladeFrame);
	void darkenExteriorPaletteRange(const Common::Array<byte> &referencePalette,
		uint firstColor, uint lastColor, byte threshold, bool snapLowComponentsToBlack);
	void runMachineRoomSequence();
	void runMonsterTableElectricSequence();
	void runMonsterTableFinalSequence();
	void runBlackFlashSequence();
	void runCloseupFlashSequence();
	void initializeMachineRoomState(MachineRoomState &state) const;
	void composeMachineRoomFrame(const MachineRoomState &state, bool finalPhase);
	void restoreOriginalSpriteFrameBackground(const Common::Array<byte> &resource, uint descriptorCount,
		uint descriptorIndex, Graphics::Surface &destination) const;
	void runMachineRoomInitialPhase(MachineRoomState &state);
	void runMachineRoomFinalPhase(MachineRoomState &state);
	void advanceMachineRoomState(MachineRoomState &state, uint32 elapsedMillis, bool finalPhase);
	void advanceInitialMemoryGate(MachineRoomState &state);
	void advanceFinalMemoryOverlay(MachineRoomState &state);
	void advanceLeftMemoryActor(MachineRoomState &state);
	void advanceRightMemoryActor(MachineRoomState &state);
	void advanceLeftElectricalArc(MachineRoomState &state);
	void advanceRightElectricalArc(MachineRoomState &state);
	void advanceMachineRoomPalette(MachineRoomState &state, uint32 elapsedMillis);
	void revealSavedFramebufferBand(uint sweepOffset, byte bandWidth);
	void runSpriteSequence(uint frameCount, uint frameMillis, uint16 viewportX,
		const SpriteTrack *tracks, uint trackCount,
		const SoundCue *soundCues = nullptr, uint soundCueCount = 0);
	void drawSpriteSequenceFrame(uint tick, const SpriteTrack *tracks, uint trackCount);
	void drawOriginalSpriteFrame(const Common::Array<byte> &resource, uint descriptorCount,
		uint descriptorIndex, Graphics::Surface &destination) const;
	uint frameForTrack(const SpriteTrack &track, uint tick) const;
	void initializeMemoryEffectPalette();
	void applyMemoryPaletteBand(uint tick);
	void processMemoryPalettePulseStep();
	void adjustMemoryPalettePulseRanges(int delta);
	void restoreMemoryPalettePulseRanges();
	uint16 nextMemoryRandom15Bit();
	void applySoundCues(uint tick, const SoundCue *soundCues, uint soundCueCount);
	void playSound(uint slot, uint16 cueId, byte volumePercent, bool loop);
	void stopSound(uint slot);
	void stopSounds();
	void presentFrame(uint16 viewportX);
	bool delay(uint32 millis);
	bool pollEvents();

	enum {
		kResourceChunkCount = 40,
		kFrameBufferSize = 0x78000
	};

	HollywoodEngine *_vm;
	ResourceChunkTable _chunkTable;
	Common::Array<byte> _palette;
	Common::Array<byte> _exteriorStormPalette;
	Common::Array<byte> _memoryPulseSavedPalette;
	IndexedSurfaceBuffer _baseFramebuffer;
	IndexedSurfaceBuffer _sceneFramebuffer;
	IndexedSurfaceBuffer _savedFramebuffer;
	Common::Array<byte> _chunks[kResourceChunkCount];
	Graphics::ManagedSurface _screen;
	Palette6Bit _displayPalette;
	SoundBank0Player _sound0;
	SoundBank0Player _sound1;
	SoundBank0Player _sound2;
	uint32 _memoryRandomState;
	byte _memoryPulseLevel;
	bool _memoryPulseActive;
	bool _skipRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE3110_H
