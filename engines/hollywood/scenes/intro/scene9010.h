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

#include "hollywood/music.h"
#include "hollywood/resource.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9010 {
public:
	Scene9010(HollywoodEngine *vm);

	bool play();

private:
	bool playScene9010();
	bool playScene9030();

	bool loadScene9010Resources();
	bool loadI01Chunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadStage003Descriptors();
	bool loadI03Scene();

	bool runPoseTransition(bool targetAlternatePose);
	bool playSpeechExchange(byte descriptorIndex);
	bool playI02Animation();
	bool readI02StreamFrame(Common::File &file);
	void setI02PaletteFrame(uint frameIndex);
	void drawResourceBlockList(const Common::Array<byte> &blockList);
	uint16 getScene9010SpeechSample(byte descriptorIndex) const;
	byte nextTalkingFrameVariant();

	void drawCharacterFrame(byte frameIndex);
	void restoreSpriteBackground(uint16 descriptorIndex);
	void drawStripSpriteFrame(uint16 descriptorIndex);

	void updateScene9010PaletteFade();
	bool fadeInPalette(uint32 stepMillis);
	bool fadeOutPalette(uint32 stepMillis);
	void presentFrame(uint rowOffset = 0, uint xOffset = 0);

	void clearFinalSweepBand(uint rowOffset, uint sweepOffset, byte bandWidth);
	void clearSceneFramebufferRun(int y, int x, int width);

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
		kStage003DescriptorTableSize = 0x186a0,
		kCharacterFrameDescriptorCount = 17,
		kFrameDescriptorSize = 14
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
	Common::Array<byte> _frameDecodeBuffer;
	Common::Array<byte> _sceneFramebuffer;
	Common::Array<byte> _screen;
	Common::Array<byte> _stage003Descriptors;
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
