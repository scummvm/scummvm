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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9010_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9010_H

#include "common/array.h"
#include "common/file.h"
#include "common/str.h"

#include "hollywood/graphics.h"
#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9010 {
public:
	Scene9010(HollywoodEngine *vm);

	bool play();
	bool skipRequested() const { return _skipRequested; }
	const IndexedSurfaceBuffer &transitionFramebuffer() const { return _sceneFramebuffer; }
	const Common::Array<byte> &transitionPalette() const { return _paletteCurrent; }

private:
	struct PopupDescriptor {
		uint16 textRecordId;
		byte continuationCount;
		uint16 voiceSampleId;
	};

	struct SpeechTextStyle {
		uint16 centerX;
		uint16 topY;
		byte colorIndex;
		byte red;
		byte green;
		byte blue;
	};

	struct SubtitleOverlay {
		bool visible;
		byte colorIndex;
		uint16 centerX;
		uint16 topY;
		Common::Array<Common::String> lines;
	};

	enum I02FramePayloadFormat {
		kI02FramePayloadUnknown,
		kI02FramePayloadBlockList,
		kI02FramePayloadScreenRows,
		kI02FramePayloadSceneRows
	};

	bool playScene9010();

	bool loadScene9010Resources();
	bool loadI01Chunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadI01Chunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize);
	bool loadI02StillFrameResource();
	bool loadStage003Descriptors();
	bool validateI02AnimationResources();

	bool runPoseTransition(bool targetAlternatePose);
	bool playSpeechExchange(byte descriptorIndex);
	bool playI02Animation();
	uint detectI02ChunkedFrameCount(ResourceChunkTable &chunkTable) const;
	bool loadI02ChunkedFrame(uint frameIndex);
	bool readI02StreamFrame(Common::File &file);
	void setI02PaletteFrame(uint frameIndex);
	I02FramePayloadFormat detectI02FramePayloadFormat(const Common::Array<byte> &payload) const;
	bool isValidI02BlockListFrame(const Common::Array<byte> &payload) const;
	void drawI02FramePayload(const Common::Array<byte> &payload);
	void drawRawI02ScreenRows(const Common::Array<byte> &payload);
	void drawRawI02SceneRows(const Common::Array<byte> &payload);
	void drawResourceBlockList(const Common::Array<byte> &blockList);
	byte nextTalkingFrameVariant();

	void drawCharacterFrame(byte frameIndex);
	void restoreSpriteBackground(uint16 descriptorIndex);
	void drawStripSpriteFrame(uint16 descriptorIndex);

	void updateScene9010PaletteFade();
	bool fadeInPalette(uint32 stepMillis);
	bool fadeOutPalette(uint32 stepMillis);
	void presentFrame(uint rowOffset = 0, uint xOffset = 0);
	void beginSubtitle(const PopupDescriptor &popup, uint segmentIndex);
	void clearSubtitle();
	void drawSubtitleOverlay();
	void wrapActorSpeechText(const Common::String &text, uint16 anchorSceneX, Common::Array<Common::String> &lines) const;
	Common::String getStage003LargeTextRecord(uint16 recordId) const;
	uint actorSpeechTextWidth(const Common::String &text) const;
	void calculatePrimarySubtitleBounds(const Common::Array<Common::String> &lines,
		const SpeechTextStyle &speechTextStyle, uint16 &centerX, uint16 &topY) const;
	PopupDescriptor getStage003PopupDescriptor(byte descriptorIndex) const;
	SpeechTextStyle getCurrentSpeechTextStyle() const;

	bool pollEvents();
	bool delay(uint32 millis);
	bool delayScene9010(uint32 millis);
	void stopAudio();

	uint16 readUint16(const Common::Array<byte> &source, uint offset) const;
	uint32 readUint32(const Common::Array<byte> &source, uint offset) const;

	enum {
		kFrameDecodeBufferSize = 0x78000,
		kSceneFramebufferSize = 0x100000,
		kPaletteSize = 0x300,
		kRawScreenFrameSize = HollywoodEngine::kScreenWidth * HollywoodEngine::kScreenHeight,
		kRawSceneFrameSize = HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight,
		kStage003DescriptorTableSize = 0x186a0,
		kStage003SmallRowSize = 0x29,
		kStage003LargeRowSize = 0x141,
		kStage003LargeRowBaseIndex = 500,
		kCharacterFrameDescriptorCount = 17,
		kFrameDescriptorSize = 14,
		kOriginalSpeechLineHeight = 20
	};

	HollywoodEngine *_vm;
	MusicPlayer _music;
	SpeechPlayer _speech;
	ResourceChunkTable _i01ChunkTable;
	Common::Array<byte> _paletteSource;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _resourceArena;
	Common::Array<byte> _i02PaletteTable;
	Common::Array<byte> _i02FramePayload;
	I02FramePayloadFormat _i02FramePayloadFormat;
	bool _i02SingleFrameOnly;
	IndexedSurfaceBuffer _frameDecodeBuffer;
	IndexedSurfaceBuffer _sceneFramebuffer;
	Graphics::ManagedSurface _screen;
	Palette6Bit _displayPalette;
	Common::Array<byte> _stage003DecodeKey;
	Common::Array<byte> _stage003Descriptors;
	Common::Array<byte> _stage003LargeRows;
	SubtitleOverlay _subtitle;
	bool _skipRequested;
	bool _alternatePoseActive;
	byte _characterFrameIndex;
	byte _lastTalkingFrameVariant;
	byte _scene9010FadeCountdown;
	bool _scene9010FadeComplete;
	uint32 _scene9010FadeAccumulator;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9010_H
