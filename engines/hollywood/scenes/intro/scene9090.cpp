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

#include "hollywood/scenes/intro/scene9090.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/path.h"
#include "common/system.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kScene9090ArchiveName = "RESOURCE.I09";
const char *const kScene9090TextArchiveName = "RESOURCE.003";
const char *const kScene9090Resource000Name = "RESOURCE.000";
const char *const kScene9090MusicArchiveName = "RESOURCE.M09";
const char *const kScene9090SoundArchiveName = "RESOURCE.S09";
const uint kScene9090StageIndex = 910;
const uint16 kScene9090MusicCueId = 0x000f;
const uint16 kScene9090NextState = 0x23aa;
const uint kScene9090SpeechRow = 4;
const byte kScene9090PrimarySpeechColor = 0xfb;
const byte kScene9090SecondarySpeechColor = 0xfd;
const int kScene9090SpeechLineHeight = 20;
const uint kScene9090Resource000TableSize = 400;
const uint kScene9090SecondaryActorBankEntry = 0x00d0;
const uint kScene9090SecondaryActorPaletteEntry = 0x0108;
const uint kScene9090ActorPaletteBytes = 0x4b;
const uint kScene9090ActorFacingCount = 6;
const uint kScene9090ActorCelsPerFacing = 13;
const uint kScene9090ActorDescriptorCount = kScene9090ActorFacingCount * kScene9090ActorCelsPerFacing;
const uint kScene9090ActorDescriptorSize = 28;
const uint kScene9090ActorFacingRunStride = 160000;
const uint kScene9090SpeechFramesPerFacing = 5;
const uint kScene9090SpeechDescriptorCount = kScene9090ActorFacingCount * kScene9090SpeechFramesPerFacing;
const uint kScene9090SpeechDescriptorSize = 16;
const uint kScene9090SpeechFacingRunStride = 16000;
const uint kScene9090DeskDescriptorCount = 0x24;
const uint kScene9090InsetDescriptorCount = 9;
const uint kScene9090ClockDescriptorCount = 0x3c;
const int kScene9090SecondaryActorX = 0x130;
const int kScene9090SecondaryActorY = 0x172;
const uint kScene9090PrimaryFrameMillis = 125;
const uint kScene9090SecondaryFrameMillis = 150;
const uint kScene9090TurnFrameMillis = 60;

const byte kScene9090TurnCelByFacing[] = { 12, 12, 12, 1, 12, 2 };

enum Scene9090Speaker {
	kScene9090DeskSpeaker,
	kScene9090SecondarySpeaker,
	kScene9090InsetSpeaker
};

struct Scene9090SpeechStep {
	byte frameIndex;
	Scene9090Speaker speaker;
	uint16 centerX;
	uint16 topY;
	byte colorIndex;
	byte red;
	byte green;
	byte blue;
	bool updatePalette;
	byte turnFacing;
	byte insetTransition;
};

const byte kScene9090NoTurn = 0xff;

const Scene9090SpeechStep kScene9090SpeechSteps[] = {
	{ 0, kScene9090DeskSpeaker,      0x078, 0x0aa, kScene9090PrimarySpeechColor,   0x00, 0x26, 0x3f, true,  kScene9090NoTurn, 0 },
	{ 1, kScene9090SecondarySpeaker, 0x276, 0x10c, kScene9090SecondarySpeechColor, 0x00, 0x00, 0x00, false, kScene9090NoTurn, 0 },
	{ 2, kScene9090InsetSpeaker,     0x0c0, 0x0c8, kScene9090PrimarySpeechColor,   0x3f, 0x3f, 0x3f, true,  4,                  0 },
	{ 3, kScene9090SecondarySpeaker, 0x276, 0x10c, kScene9090SecondarySpeechColor, 0x00, 0x00, 0x00, false, kScene9090NoTurn, 0 },
	{ 4, kScene9090DeskSpeaker,      0x078, 0x0aa, kScene9090PrimarySpeechColor,   0x00, 0x26, 0x3f, true,  5,                  1 },
	{ 5, kScene9090SecondarySpeaker, 0x276, 0x10c, kScene9090SecondarySpeechColor, 0x00, 0x00, 0x00, false, kScene9090NoTurn, 0 },
	{ 6, kScene9090InsetSpeaker,     0x0c0, 0x0c8, kScene9090PrimarySpeechColor,   0x3f, 0x3f, 0x3f, true,  kScene9090NoTurn, 0 },
	{ 7, kScene9090DeskSpeaker,      0x078, 0x0aa, kScene9090PrimarySpeechColor,   0x00, 0x26, 0x3f, true,  kScene9090NoTurn, 0 }
};

const byte kScene9090DeskFrameMap[] = {
	0, 31, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13,
	32, 33, 34, 35, 14, 15, 16, 16, 17, 18, 19, 29, 20, 21, 22, 23,
	23, 24, 25, 26, 30, 22, 21, 20, 16
};

const byte kScene9090InsetFrameMap[] = {
	0, 1, 2, 3, 0, 4, 5, 5, 6, 7, 8, 5, 4, 0
};

Scene9090::Scene9090(HollywoodEngine *vm) :
		IntroSceneBase(vm, "Scene 9090"),
		_resources(),
		_music(vm->introMusic()),
		_primarySpeech(),
		_secondarySpeech(),
		_clockSound(),
		_ambientSound(),
		_text(),
		_random("hollywood_scene9090"),
		_paletteResource(),
		_presentationPaletteRemapTable(),
		_baseFramebuffer(),
		_secondaryActorBank(),
		_secondarySpeechBank(),
		_primarySubtitle(),
		_secondarySubtitle(),
		_deskFrame(0x17),
		_deskFacingMode(5),
		_insetFrame(0),
		_insetVariant(1),
		_secondaryFacing(5),
		_secondaryCel(0),
		_secondarySpeechFrame(0),
		_lastDeskTalkFrame(0xff),
		_lastInsetTalkFrame(0xff),
		_deskFrameAccumulator(0),
		_insetFrameAccumulator(0),
		_secondaryFrameAccumulator(0),
		_clockAccumulator(0),
		_secondarySpeechVisible(false) {
	_paletteResource.resize(kPaletteSize);
	_presentationPaletteRemapTable.resize(256);
	_baseFramebuffer.resize(kFrameBufferSize);
	_primarySubtitle.visible = false;
	_primarySubtitle.colorIndex = kScene9090PrimarySpeechColor;
	_primarySubtitle.centerX = 0;
	_primarySubtitle.topY = 0;
	_secondarySubtitle.visible = false;
	_secondarySubtitle.colorIndex = kScene9090SecondarySpeechColor;
	_secondarySubtitle.centerX = 0;
	_secondarySubtitle.topY = 0;
	_clockFrames[0] = 10;
	_clockFrames[1] = 7;
	_clockFrames[2] = 3;
}

bool Scene9090::play() {
	if (!load())
		return false;

	initializeOfficeState();
	composeFrame();
	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	_sceneFramebuffer.clear(0);
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	_music->setArchive(Common::Path(kScene9090MusicArchiveName));
	_music->playMusicCue(kScene9090MusicCueId, 30, true);
	_ambientSound.setArchive(Common::Path(kScene9090SoundArchiveName));
	_clockSound.setArchive(Common::Path(kScene9090SoundArchiveName));
	_ambientSound.playSample(0x11, 25, true);
	revealSavedFramebufferWithCurtain();

	if (!_skipRequested && !Engine::shouldQuit())
		runDialogueSequence();

	clearSubtitles();
	clearSceneFramebufferWithCurtain();
	_primarySpeech.stop();
	_secondarySpeech.stop();
	_clockSound.stop();
	_ambientSound.stop();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9090NextState;

	return true;
}

bool Scene9090::load() {
	if (!_resources.loadChunkTable(kScene9090ArchiveName))
		return false;

	if (!_resources.validateChunkRange(kScene9090ArchiveName, _debugName, 0, 12))
		return false;

	if (!loadChunk(0, _baseFramebuffer, kFrameBufferSize) ||
			!loadChunk(1, _paletteResource, kPaletteSize))
		return false;

	_resources.allocateArena(_resources.totalChunkSize(5, 12));
	for (uint i = 5; i <= 12; ++i) {
		if (!loadArenaChunk(i))
			return false;
	}

	if (!_text.loadStage(kScene9090TextArchiveName, _debugName, kScene9090StageIndex) ||
			!loadActorResources())
		return false;

	_paletteResource[kScene9090SecondarySpeechColor * 3] = 0x3f;
	_paletteResource[kScene9090SecondarySpeechColor * 3 + 1] = 0x28;
	_paletteResource[kScene9090SecondarySpeechColor * 3 + 2] = 0x32;
	buildPresentationPaletteRemapTable(_paletteResource, _presentationPaletteRemapTable);
	return true;
}

bool Scene9090::loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9090::loadChunk(uint index, IndexedSurfaceBuffer &destination, uint fixedSize) {
	return _resources.loadFixedChunk(_debugName, index, destination, fixedSize);
}

bool Scene9090::loadArenaChunk(uint index) {
	return _resources.loadArenaChunk(_debugName, index, index);
}

bool Scene9090::loadActorResources() {
	Common::File file;
	if (!file.open(Common::Path(kScene9090Resource000Name))) {
		warning("Failed to open %s for %s actors", kScene9090Resource000Name, _debugName);
		return false;
	}

	if ((uint32)file.size() < 1 + 2 * kScene9090Resource000TableSize) {
		warning("%s is too small for actor tables", kScene9090Resource000Name);
		return false;
	}

	file.seek(1);
	Common::Array<byte> offsetTable;
	Common::Array<byte> sizeTable;
	offsetTable.resize(kScene9090Resource000TableSize);
	sizeTable.resize(kScene9090Resource000TableSize);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s actor tables", kScene9090Resource000Name);
		return false;
	}

	const uint32 bankOffset = readUint32LE(offsetTable, kScene9090SecondaryActorBankEntry);
	if (bankOffset >= (uint32)file.size()) {
		warning("%s secondary actor bank is out of range", kScene9090Resource000Name);
		return false;
	}

	_secondaryActorBank.runStreams.resize(kScene9090ActorFacingCount * kScene9090ActorFacingRunStride);
	memset(_secondaryActorBank.runStreams.data(), 0, _secondaryActorBank.runStreams.size());
	_secondaryActorBank.descriptors.resize(kScene9090ActorDescriptorCount);
	for (uint i = 0; i < _secondaryActorBank.descriptors.size(); ++i)
		_secondaryActorBank.descriptors[i] = ActorSpriteDescriptor();

	file.seek(bankOffset);
	for (uint facing = 0; facing < kScene9090ActorFacingCount; ++facing) {
		const uint32 segmentSize = readUint32LE(sizeTable,
			kScene9090SecondaryActorBankEntry + facing * 4);
		if (segmentSize > kScene9090ActorFacingRunStride ||
				segmentSize > (uint32)file.size() - (uint32)file.pos()) {
			warning("%s secondary actor facing %u is out of range", kScene9090Resource000Name, facing);
			return false;
		}
		if (file.read(_secondaryActorBank.runStreams.data() + facing * kScene9090ActorFacingRunStride,
				segmentSize) != segmentSize) {
			warning("Failed to read %s secondary actor facing %u", kScene9090Resource000Name, facing);
			return false;
		}
	}

	const uint32 descriptorSize = readUint32LE(sizeTable,
		kScene9090SecondaryActorBankEntry + kScene9090ActorFacingCount * 4);
	if (descriptorSize % kScene9090ActorDescriptorSize != 0 ||
			descriptorSize > (uint32)file.size() - (uint32)file.pos()) {
		warning("%s secondary actor descriptors are invalid", kScene9090Resource000Name);
		return false;
	}

	Common::Array<byte> descriptorData;
	descriptorData.resize(descriptorSize);
	if (file.read(descriptorData.data(), descriptorData.size()) != descriptorData.size()) {
		warning("Failed to read %s secondary actor descriptors", kScene9090Resource000Name);
		return false;
	}

	const uint descriptorCount = MIN<uint>(kScene9090ActorDescriptorCount,
		descriptorData.size() / kScene9090ActorDescriptorSize);
	for (uint i = 0; i < descriptorCount; ++i) {
		const uint offset = i * kScene9090ActorDescriptorSize;
		ActorSpriteDescriptor &descriptor = _secondaryActorBank.descriptors[i];
		descriptor.runStreamOffset = readUint32LE(descriptorData, offset);
		descriptor.opaqueRunCount = readUint32LE(descriptorData, offset + 4);
		descriptor.paletteRunCount = readUint32LE(descriptorData, offset + 8);
		descriptor.anchorX = (int16)readUint16LE(descriptorData, offset + 12);
		descriptor.anchorY = (int16)readUint16LE(descriptorData, offset + 16);
		descriptor.width = readUint16LE(descriptorData, offset + 20);
		descriptor.height = readUint16LE(descriptorData, offset + 24);
	}

	_secondarySpeechBank.runStreams.resize(kScene9090ActorFacingCount * kScene9090SpeechFacingRunStride);
	memset(_secondarySpeechBank.runStreams.data(), 0, _secondarySpeechBank.runStreams.size());
	_secondarySpeechBank.descriptors.resize(kScene9090SpeechDescriptorCount);
	for (uint i = 0; i < _secondarySpeechBank.descriptors.size(); ++i)
		_secondarySpeechBank.descriptors[i] = ActorSpeechDescriptor();

	for (uint facing = 0; facing < kScene9090ActorFacingCount; ++facing) {
		const uint segment = 7 + facing;
		const uint32 segmentSize = readUint32LE(sizeTable,
			kScene9090SecondaryActorBankEntry + segment * 4);
		if (segmentSize > kScene9090SpeechFacingRunStride ||
				segmentSize > (uint32)file.size() - (uint32)file.pos()) {
			warning("%s secondary speech facing %u is out of range", kScene9090Resource000Name, facing);
			return false;
		}
		if (file.read(_secondarySpeechBank.runStreams.data() + facing * kScene9090SpeechFacingRunStride,
				segmentSize) != segmentSize) {
			warning("Failed to read %s secondary speech facing %u", kScene9090Resource000Name, facing);
			return false;
		}
	}

	const uint32 speechDescriptorSize = readUint32LE(sizeTable,
		kScene9090SecondaryActorBankEntry + 13 * 4);
	if (speechDescriptorSize % kScene9090SpeechDescriptorSize != 0 ||
			speechDescriptorSize > (uint32)file.size() - (uint32)file.pos()) {
		warning("%s secondary speech descriptors are invalid", kScene9090Resource000Name);
		return false;
	}

	Common::Array<byte> speechDescriptorData;
	speechDescriptorData.resize(speechDescriptorSize);
	if (file.read(speechDescriptorData.data(), speechDescriptorData.size()) != speechDescriptorData.size()) {
		warning("Failed to read %s secondary speech descriptors", kScene9090Resource000Name);
		return false;
	}

	const uint speechDescriptorCount = MIN<uint>(kScene9090SpeechDescriptorCount,
		speechDescriptorData.size() / kScene9090SpeechDescriptorSize);
	for (uint i = 0; i < speechDescriptorCount; ++i) {
		const uint offset = i * kScene9090SpeechDescriptorSize;
		ActorSpeechDescriptor &descriptor = _secondarySpeechBank.descriptors[i];
		descriptor.runStreamOffset = readUint32LE(speechDescriptorData, offset);
		descriptor.runCount = readUint32LE(speechDescriptorData, offset + 4);
		descriptor.anchorX = (int16)readUint16LE(speechDescriptorData, offset + 8);
		descriptor.anchorY = (int16)readUint16LE(speechDescriptorData, offset + 12);
	}

	const uint32 paletteOffset = readUint32LE(offsetTable, kScene9090SecondaryActorPaletteEntry);
	const uint paletteDestination = 0xd0 * 3;
	if (paletteOffset > (uint32)file.size() ||
			kScene9090ActorPaletteBytes > (uint32)file.size() - paletteOffset ||
			paletteDestination + kScene9090ActorPaletteBytes > _paletteResource.size()) {
		warning("%s secondary actor palette is out of range", kScene9090Resource000Name);
		return false;
	}

	file.seek(paletteOffset);
	if (file.read(_paletteResource.data() + paletteDestination, kScene9090ActorPaletteBytes) !=
			kScene9090ActorPaletteBytes) {
		warning("Failed to read %s secondary actor palette", kScene9090Resource000Name);
		return false;
	}

	return true;
}

void Scene9090::initializeOfficeState() {
	_deskFrame = 0x17;
	_deskFacingMode = 5;
	_insetFrame = 0;
	_insetVariant = 1;
	_secondaryFacing = 5;
	_secondaryCel = 0;
	_secondarySpeechFrame = 0;
	_secondarySpeechVisible = false;
	_clockFrames[0] = 10;
	_clockFrames[1] = 7;
	_clockFrames[2] = 3;
	_lastDeskTalkFrame = 0xff;
	_lastInsetTalkFrame = 0xff;
	_deskFrameAccumulator = 0;
	_insetFrameAccumulator = 0;
	_secondaryFrameAccumulator = 0;
	_clockAccumulator = 0;
	clearSubtitles();
}

void Scene9090::composeFrame() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	drawOfficePatch(6);
	drawDeskActor();
	drawSecondaryActor();
	drawInsetActor();
	drawOfficePatch(8);
	drawClock();
}

void Scene9090::drawOfficePatch(uint chunkIndex) {
	if (chunkIndex >= IntroResourceSet::kResourceChunkCount)
		return;
	drawResourceBlockList(_resources.arena, _resources.chunkOffsets[chunkIndex],
		_sceneFramebuffer.surface());
}

void Scene9090::drawDeskActor() {
	if (_deskFrame >= ARRAYSIZE(kScene9090DeskFrameMap))
		return;
	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[5], 0,
		kScene9090DeskDescriptorCount, kScene9090DeskFrameMap[_deskFrame],
		_sceneFramebuffer.surface());
}

void Scene9090::drawInsetActor() {
	if (_insetFrame >= ARRAYSIZE(kScene9090InsetFrameMap))
		return;
	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[7], 0,
		kScene9090InsetDescriptorCount, kScene9090InsetFrameMap[_insetFrame],
		_sceneFramebuffer.surface());
}

void Scene9090::drawClock() {
	for (uint i = 0; i < 3; ++i) {
		drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[10 + i], 0,
			kScene9090ClockDescriptorCount, _clockFrames[i], _sceneFramebuffer.surface());
	}
}

void Scene9090::drawSecondaryActor() {
	const int speechBottomY = _secondarySpeechVisible ? drawSecondarySpeechFrame() : -1;
	drawActorSpriteFrame(_secondaryActorBank, _secondaryFacing, _secondaryCel,
		kScene9090SecondaryActorX, kScene9090SecondaryActorY, speechBottomY,
		_sceneFramebuffer.surface(), _presentationPaletteRemapTable);
}

int Scene9090::drawSecondarySpeechFrame() {
	if (_secondaryFacing >= kScene9090ActorFacingCount ||
			_secondarySpeechFrame >= kScene9090SpeechFramesPerFacing)
		return -1;

	const uint descriptorIndex = _secondaryFacing * kScene9090SpeechFramesPerFacing +
		_secondarySpeechFrame;
	if (descriptorIndex >= _secondarySpeechBank.descriptors.size())
		return -1;

	const ActorSpeechDescriptor &descriptor = _secondarySpeechBank.descriptors[descriptorIndex];
	const uint runBase = _secondaryFacing * kScene9090SpeechFacingRunStride;
	if (runBase + descriptor.runStreamOffset >= _secondarySpeechBank.runStreams.size())
		return -1;

	const int spriteX = kScene9090SecondaryActorX - descriptor.anchorX;
	const int spriteY = kScene9090SecondaryActorY - descriptor.anchorY;
	return drawActorRunStream(_secondarySpeechBank.runStreams, descriptor.runStreamOffset,
		runBase, descriptor.runCount, spriteX, spriteY, -1,
		_sceneFramebuffer.surface(), nullptr);
}

void Scene9090::runDialogueSequence() {
	setDeskFacing(2);
	runSpeechLine(0);
	if (_deskFacingMode == 7)
		returnDeskActorToForward();

	runSpeechLine(1);
	setInsetVariant(2);
	runSpeechLine(2);
	runSpeechLine(3);
	setDeskFacing(0);
	runSpeechLine(4);
	if (_deskFacingMode == 7)
		returnDeskActorToForward();
	runConcurrentSpeechLines(5, 6);
	setDeskFacing(2);
	runSpeechLine(7);
	if (_deskFacingMode == 7)
		returnDeskActorToForward();
}

void Scene9090::setDeskFacing(byte facingMode) {
	if (_skipRequested || Engine::shouldQuit())
		return;

	const byte targetMode = facingMode == 0 ? (byte)(5 + 2 * _random.getRandomNumber(1)) :
		(byte)(3 + 2 * facingMode);
	if (targetMode == _deskFacingMode)
		return;
	if (targetMode == 5) {
		returnDeskActorToForward();
		return;
	}

	animateDeskFrames(0x1b, 0x1f);
	_deskFacingMode = 7;
	_deskFrame = 0x1f;
}

void Scene9090::returnDeskActorToForward() {
	animateDeskFrames(0x24, 0x28);
	_deskFacingMode = 5;
	_deskFrame = 0x17;
}

void Scene9090::animateDeskFrames(byte firstFrame, byte lastFrame) {
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		_deskFrame = frame;
		composeFrame();
		presentFrame();
		if (delay(kScene9090PrimaryFrameMillis))
			return;
		advanceClock(kScene9090PrimaryFrameMillis);
	}
}

void Scene9090::setInsetVariant(byte variant) {
	if (variant == _insetVariant || _skipRequested || Engine::shouldQuit())
		return;

	const byte firstFrame = variant == 2 ? 4 : 0x0b;
	const byte lastFrame = variant == 2 ? 6 : 0x0d;
	for (byte frame = firstFrame; frame <= lastFrame && !_skipRequested && !Engine::shouldQuit(); ++frame) {
		_insetFrame = frame;
		composeFrame();
		presentFrame();
		if (delay(kScene9090PrimaryFrameMillis))
			return;
		advanceClock(kScene9090PrimaryFrameMillis);
	}
	_insetVariant = variant;
	_insetFrame = variant == 2 ? 7 : 0;
}

void Scene9090::animateSecondaryTurn(byte facing) {
	if (facing >= ARRAYSIZE(kScene9090TurnCelByFacing) || facing == _secondaryFacing ||
			_skipRequested || Engine::shouldQuit())
		return;

	_secondaryFacing = facing;
	_secondarySpeechVisible = false;
	_secondarySpeechFrame = 0;
	_secondaryCel = kScene9090TurnCelByFacing[facing];
	composeFrame();
	presentFrame();
	if (delay(kScene9090TurnFrameMillis))
		return;

	_secondaryCel = 0;
	composeFrame();
	presentFrame();
}

void Scene9090::runSpeechLine(byte stepIndex) {
	runSpeechSteps(&stepIndex, 1);
}

void Scene9090::runConcurrentSpeechLines(byte firstStepIndex, byte secondStepIndex) {
	const byte steps[] = { firstStepIndex, secondStepIndex };
	runSpeechSteps(steps, ARRAYSIZE(steps));
}

void Scene9090::runSpeechSteps(const byte *stepIndices, uint stepCount) {
	if (stepCount == 0 || stepCount > 2)
		return;

	uint16 textRecordIds[2] = { 0, 0 };
	uint16 voiceSampleIds[2] = { 0, 0 };
	byte segmentCounts[2] = { 0, 0 };
	uint maxSegmentCount = 0;
	for (uint i = 0; i < stepCount; ++i) {
		if (stepIndices[i] >= ARRAYSIZE(kScene9090SpeechSteps))
			continue;
		byte continuationCount = 0;
		if (_text.getStageCue(kScene9090SpeechRow, kScene9090SpeechSteps[stepIndices[i]].frameIndex,
				textRecordIds[i], continuationCount, voiceSampleIds[i])) {
			segmentCounts[i] = MAX<byte>(1, continuationCount);
			maxSegmentCount = MAX<uint>(maxSegmentCount, segmentCounts[i]);
		}
	}

	for (uint segment = 0; segment < maxSegmentCount && !_skipRequested && !Engine::shouldQuit(); ++segment) {
		uint32 durations[2] = { 0, 0 };
		bool activeStep[2] = { false, false };
		for (uint i = 0; i < stepCount; ++i) {
			if (segment >= segmentCounts[i])
				continue;

			const Scene9090SpeechStep &step = kScene9090SpeechSteps[stepIndices[i]];
			if (step.updatePalette) {
				_paletteCurrent[step.colorIndex * 3] = step.red;
				_paletteCurrent[step.colorIndex * 3 + 1] = step.green;
				_paletteCurrent[step.colorIndex * 3 + 2] = step.blue;
			}

			SubtitleOverlay &subtitle = step.speaker == kScene9090SecondarySpeaker ?
				_secondarySubtitle : _primarySubtitle;
			beginSubtitle(subtitle, textRecordIds[i] + segment, step.colorIndex,
				step.centerX, step.topY);

			SpeechPlayer &player = step.speaker == kScene9090SecondarySpeaker ?
				_secondarySpeech : _primarySpeech;
			const uint16 sampleId = voiceSampleIds[i] == 0 ? 0 : voiceSampleIds[i] + segment;
			const bool started = sampleId != 0 && player.playSample(sampleId, 100);
			durations[i] = started ? MAX<uint32>(player.lastSampleDurationMillis(), 750) :
				MAX<uint32>(1200, subtitle.lines.size() * 1100);
			activeStep[i] = true;
			if (step.speaker == kScene9090SecondarySpeaker) {
				_secondarySpeechFrame = 0;
				_secondaryFrameAccumulator = 0;
				_secondarySpeechVisible = true;
			}
		}

		composeFrame();
		presentFrame();
		const uint32 startMillis = g_system->getMillis();
		if (segment == 0) {
			for (uint i = 0; i < stepCount && !_skipRequested && !Engine::shouldQuit(); ++i) {
				const Scene9090SpeechStep &step = kScene9090SpeechSteps[stepIndices[i]];
				if (step.turnFacing != kScene9090NoTurn)
					animateSecondaryTurn(step.turnFacing);
				if (step.insetTransition != 0)
					setInsetVariant(step.insetTransition);
			}
		}

		while (!_skipRequested && !Engine::shouldQuit()) {
			const uint32 elapsed = g_system->getMillis() - startMillis;
			bool deskSpeaking = false;
			bool secondarySpeaking = false;
			bool insetSpeaking = false;
			bool anyActive = false;
			for (uint i = 0; i < stepCount; ++i) {
				if (!activeStep[i])
					continue;
				SpeechPlayer &player = kScene9090SpeechSteps[stepIndices[i]].speaker ==
					kScene9090SecondarySpeaker ? _secondarySpeech : _primarySpeech;
				const bool active = player.isPlaying() || elapsed < durations[i];
				if (!active)
					continue;
				anyActive = true;
				switch (kScene9090SpeechSteps[stepIndices[i]].speaker) {
				case kScene9090DeskSpeaker:
					deskSpeaking = true;
					break;
				case kScene9090SecondarySpeaker:
					secondarySpeaking = true;
					break;
				case kScene9090InsetSpeaker:
					insetSpeaking = true;
					break;
				}
			}
			if (!anyActive)
				break;
			if (pollEvents())
				return;
			g_system->delayMillis(10);
			advanceDialogueAnimations(10, deskSpeaking, secondarySpeaking, insetSpeaking);
		}

		for (uint i = 0; i < stepCount; ++i) {
			if (!activeStep[i])
				continue;
			if (kScene9090SpeechSteps[stepIndices[i]].speaker == kScene9090SecondarySpeaker) {
				_secondarySubtitle.visible = false;
				_secondarySubtitle.lines.clear();
				_secondarySpeechFrame = 0;
				_secondarySpeechVisible = false;
			} else {
				_primarySubtitle.visible = false;
				_primarySubtitle.lines.clear();
				if (kScene9090SpeechSteps[stepIndices[i]].speaker == kScene9090InsetSpeaker)
					_insetFrame = _insetVariant == 2 ? 7 : 0;
				else if (_deskFacingMode == 3)
					_deskFrame = 0x0f;
				else if (_deskFacingMode == 5)
					_deskFrame = 0x17;
				else if (_deskFacingMode == 7)
					_deskFrame = 0x20;
			}
		}
		composeFrame();
		presentFrame();
	}
}

void Scene9090::advanceDialogueAnimations(uint32 deltaMillis, bool deskSpeaking,
		bool secondarySpeaking, bool insetSpeaking) {
	bool dirty = false;
	if (deskSpeaking) {
		int baseFrame = -1;
		if (_deskFacingMode == 3)
			baseFrame = 0x0f;
		else if (_deskFacingMode == 5)
			baseFrame = 0x17;
		else if (_deskFacingMode == 7)
			baseFrame = 0x20;

		_deskFrameAccumulator += deltaMillis;
		if (baseFrame >= 0 && _deskFrameAccumulator >= kScene9090PrimaryFrameMillis) {
			_deskFrameAccumulator %= kScene9090PrimaryFrameMillis;
			_lastDeskTalkFrame = nextFrameExcluding(4, _lastDeskTalkFrame);
			_deskFrame = (byte)(baseFrame + _lastDeskTalkFrame);
			dirty = true;
		}
	} else {
		_deskFrameAccumulator += deltaMillis;
		if (_deskFrameAccumulator >= kScene9090PrimaryFrameMillis) {
			_deskFrameAccumulator %= kScene9090PrimaryFrameMillis;
			if (_deskFacingMode == 5 && _deskFrame == 0x1b) {
				_deskFrame = 0x17;
				dirty = true;
			} else if (_deskFacingMode == 5 && _random.getRandomNumber(14) == 0) {
				_deskFrame = 0x1b;
				dirty = true;
			}
		}
	}

	if (insetSpeaking) {
		_insetFrameAccumulator += deltaMillis;
		if (_insetFrameAccumulator >= kScene9090PrimaryFrameMillis) {
			_insetFrameAccumulator %= kScene9090PrimaryFrameMillis;
			_lastInsetTalkFrame = nextFrameExcluding(4, _lastInsetTalkFrame);
			_insetFrame = (byte)((_insetVariant == 2 ? 7 : 0) + _lastInsetTalkFrame);
			dirty = true;
		}
	} else {
		_insetFrameAccumulator += deltaMillis;
		if (!deskSpeaking && _insetFrameAccumulator >= kScene9090PrimaryFrameMillis) {
			_insetFrameAccumulator %= kScene9090PrimaryFrameMillis;
			const byte idleFrame = _insetVariant == 2 ? 0x0b : 4;
			const byte baseFrame = _insetVariant == 2 ? 7 : 0;
			if (_insetFrame == idleFrame) {
				_insetFrame = baseFrame;
				dirty = true;
			} else if (_random.getRandomNumber(14) == 0) {
				_insetFrame = idleFrame;
				dirty = true;
			}
		}
	}

	if (secondarySpeaking) {
		_secondarySpeechVisible = true;
		_secondaryFrameAccumulator += deltaMillis;
		if (_secondaryFrameAccumulator >= kScene9090SecondaryFrameMillis) {
			_secondaryFrameAccumulator %= kScene9090SecondaryFrameMillis;
			_secondarySpeechFrame = nextFrameExcluding(4, _secondarySpeechFrame);
			dirty = true;
		}
	} else {
		_secondaryFrameAccumulator += deltaMillis;
		if (_secondaryFrameAccumulator >= kScene9090SecondaryFrameMillis) {
			_secondaryFrameAccumulator %= kScene9090SecondaryFrameMillis;
			if (_secondarySpeechVisible) {
				_secondarySpeechFrame = 0;
				_secondarySpeechVisible = false;
				dirty = true;
			} else if (_random.getRandomNumber(14) == 0) {
				_secondarySpeechFrame = 4;
				_secondarySpeechVisible = true;
				dirty = true;
			}
		}
	}

	const byte oldSecondFrame = _clockFrames[2];
	advanceClock(deltaMillis);
	if (_clockFrames[2] != oldSecondFrame)
		dirty = true;
	if (dirty) {
		composeFrame();
		presentFrame();
	}
}

void Scene9090::advanceClock(uint32 deltaMillis) {
	_clockAccumulator += deltaMillis;
	while (_clockAccumulator >= 1000) {
		_clockAccumulator -= 1000;
		_clockFrames[2] = (_clockFrames[2] + 1) % kScene9090ClockDescriptorCount;
		if (_clockFrames[2] == 0) {
			_clockFrames[1] = (_clockFrames[1] + 1) % kScene9090ClockDescriptorCount;
			if (_clockFrames[1] % 5 == 0)
				_clockFrames[0] = (_clockFrames[0] + 1) % kScene9090ClockDescriptorCount;
		}
		_clockSound.playSample(0x0b, 100);
	}
}

byte Scene9090::nextFrameExcluding(byte maximumFrame, byte previousFrame) {
	byte frame = 0;
	do {
		frame = (byte)_random.getRandomNumber(maximumFrame);
	} while (frame == previousFrame && maximumFrame != 0);
	return frame;
}

void Scene9090::beginSubtitle(SubtitleOverlay &subtitle, uint16 textRecordId,
		byte colorIndex, uint16 centerX, uint16 topY) {
	subtitle.visible = false;
	subtitle.lines.clear();
	if (!_vm->subtitlesEnabled() || !_vm->font() || !_vm->font()->isLoaded())
		return;

	const Common::String text = _text.largeTextRecord(textRecordId);
	if (text.empty())
		return;

	subtitle.colorIndex = colorIndex;
	subtitle.centerX = centerX;
	subtitle.topY = topY;
	wrapSubtitleText(text, centerX, subtitle.lines);
	subtitle.visible = !subtitle.lines.empty();
}

void Scene9090::clearSubtitles() {
	_primarySubtitle.visible = false;
	_primarySubtitle.lines.clear();
	_secondarySubtitle.visible = false;
	_secondarySubtitle.lines.clear();
}

void Scene9090::drawFrameOverlays() {
	drawSubtitle(_primarySubtitle);
	drawSubtitle(_secondarySubtitle);
}

void Scene9090::drawSubtitle(const SubtitleOverlay &subtitle) {
	if (!subtitle.visible || !_vm->font() || !_vm->font()->isLoaded())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);
	for (uint lineIndex = 0; lineIndex < subtitle.lines.size(); ++lineIndex) {
		const Common::String &line = subtitle.lines[lineIndex];
		const int lineWidth = subtitleTextWidth(line);
		int x = (int)subtitle.centerX - (lineWidth >> 1);
		x = CLIP<int>(x, 0, MAX<int>(0, HollywoodEngine::kScreenWidth - lineWidth));
		const int y = (int)subtitle.topY + lineIndex * kScene9090SpeechLineHeight;
		font->drawString(_screen.surfacePtr(), line, x, y, lineWidth, subtitle.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene9090::wrapSubtitleText(const Common::String &text, uint16 anchorSceneX,
		Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = anchorSceneX < 0xa0 || HollywoodEngine::kScreenWidth - anchorSceneX < 0xa0 ? 0x24 : 0x32;
	const char *source = text.c_str();
	const uint textLength = text.size();
	uint cursor = 0;
	while (cursor < textLength && lines.size() < 10) {
		uint end = textLength;
		if (cursor + maxChars < textLength) {
			end = cursor + maxChars;
			while (end > cursor && (byte)source[end] != 0x20 && source[end] != 0)
				--end;
			while (end > cursor && (byte)source[end - 1] == 0x20)
				--end;
			if (end == cursor)
				end = MIN<uint>(textLength, cursor + maxChars);
		}
		lines.push_back(Common::String(source + cursor, end - cursor));
		cursor = end;
		while (cursor < textLength && (byte)source[cursor] == 0x20)
			++cursor;
		maxChars = maxChars > 2 ? maxChars - 2 : 1;
	}
}

uint Scene9090::subtitleTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;
	return _vm->font()->getStringWidth(text) + 2;
}

void Scene9090::stopAudio() {
	clearSubtitles();
	_primarySpeech.stop();
	_secondarySpeech.stop();
	_clockSound.stop();
	_ambientSound.stop();
	_music->stop();
}

} // End of namespace Hollywood
