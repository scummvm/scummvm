#include "gameobjects.h"
#include "common/memstream.h"

namespace Common {

DECLARE_SINGLETON(Macs2::GameObjects);
DECLARE_SINGLETON(Macs2::Scenes);

} // namespace Common

Common::MemoryReadStream *Macs2::Scenes::ReadSceneScript(uint16 sceneIndex, Common::MemoryReadStream *fileStream) {
	// Calculate the offset of the script data offset
	// This addressing can be found in the l0037_2856 code block

	uint16 sceneDataOffset = sceneIndex * 0xC;
	// Offset of the data in [0752h] global
	constexpr uint16 globalDataOffset = 0xC + 0x4;
	sceneDataOffset += globalDataOffset;
	fileStream->seek(sceneDataOffset - 0x8);
	uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);
	

	// Read the script from there
	// We read 80h bytes
	fileStream->seek(0x80, SEEK_CUR);
	uint16 scriptSize = fileStream->readUint16LE();
	uint8 *scriptData = new uint8[scriptSize];
	fileStream->read(scriptData, scriptSize);
	// TODO: Consider using the endian version for all the memoryReadStreams
	return new Common::MemoryReadStream(scriptData, scriptSize);
}

Common::Array<uint32> Macs2::Scenes::ReadSpecialAnimsOffsets(uint16 sceneIndex, Common::MemoryReadStream *fileStream) {
	Common::Array<uint32> result;
	result.resize(0x80 / 4);


	// Calculate the offset of the script data offset
	// This addressing can be found in the l0037_2856 code block
	// TODO: This part is copy-pasted and could be refactored into one proper loading function


	uint16 sceneDataOffset = sceneIndex * 0xC;
	// Offset of the data in [0752h] global
	constexpr uint16 globalDataOffset = 0xC + 0x4;
	sceneDataOffset += globalDataOffset;
	fileStream->seek(sceneDataOffset - 0x8);
	uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);

	// Read the script from there
	// We read 80h bytes for the special animations offsets
	fileStream->read(result.data(), 0x80);
	
	return result;
}

Common::MemoryReadStream *Macs2::Scenes::ReadSceneStrings(uint16 sceneIndex, Common::MemoryReadStream *fileStream) {
	// Calculate the offset of the script data offset
	// This addressing can be found in the l0037_2856 code block

	uint16 sceneDataOffset = sceneIndex * 0xC;
	// Offset of the data in [0752h] global
	constexpr uint16 globalDataOffset = 0xC + 0x4;
	sceneDataOffset += globalDataOffset;
	fileStream->seek(sceneDataOffset - 0x4);
	uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);

	// TODO: Continue here:
	// This lives in l0037_A4FC:

	// Read the script from there
	// Size - lives in global [0F84h]
	uint16 size = fileStream->readUint16LE();

	// TODO: More things to document from l0037_A532:

	// Stringdata lives in the pointer [0F80h]
	uint8 *stringData = new uint8[size];
	fileStream->read(stringData, size);
	// TODO: Consider using the endian version for all the memoryReadStreams
	return new Common::MemoryReadStream(stringData, size);
	// Note: We save the current scene number to [0F86h] - maybe "scene we have strings loaded for"?
}

Common::Array<uint8> Macs2::Scenes::ReadSpecialAnimBlob(uint16 index, Common::MemoryReadStream *fileStream) {
	uint32 offset = CurrentSceneSpecialAnimOffsets[index - 1];
	fileStream->seek(offset, SEEK_SET);
	uint32 length = fileStream->readUint32LE();
	// Skip a string - note the original code adds 0x4 for the previously read size since
	// it does not use the stream position
	fileStream->seek(0xC, SEEK_CUR);
	Common::Array<uint8> result(length);
	fileStream->read(result.data(), length);
	return result;
}

// TODO: Add all to namespace?
Macs2::GameObject *Macs2::GameObjects::GetProtagonistObject() {
	return instance().Objects[0];
}

Macs2::GameObject *Macs2::GameObjects::GetObjectByIndex(uint16 index) {
	if (index - 1 >= instance().Objects.size()) {
		return nullptr;
	}
	return instance().Objects[index - 1];
}

Common::MemoryReadStream *Macs2::GameObjects::ReadGameObjectStrings(uint16 index, Common::MemoryReadStream *fileStream) {
	// TODO: Copy&Pasted code from ReadSceneStrings
	// Calculate the offset of the script data offset
	// This addressing can be found in the l0037_2856 code block

	uint16 gameObjectDataOffset = index * 0xC;
	// Offset of the data in [0752h] global
	constexpr uint16 globalDataOffset = 0xC + 0x4;
	gameObjectDataOffset += globalDataOffset;
	fileStream->seek(gameObjectDataOffset + 0x17FC);
	uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);

	// TODO: Continue here:
	// This lives in l0037_A4FC:

	// Read the script from there
	// Size - lives in global [0F84h]
	uint16 size = fileStream->readUint16LE();

	// TODO: More things to document from l0037_A532:

	// Stringdata lives in the pointer [0F80h]
	uint8 *stringData = new uint8[size];
	fileStream->read(stringData, size);
	// TODO: Consider using the endian version for all the memoryReadStreams
	return new Common::MemoryReadStream(stringData, size);
	// Note: We save the current scene number to [0F86h] - maybe "scene we have strings loaded for"?
}

Common::MemoryReadStream *Macs2::GameObject::GetScriptStream() {
	return new Common::MemoryReadStream(Script.data(), Script.size());
}

Macs2::AnimationReader::AnimationReader(const Common::Array<uint8> &blob) {
	readStream = new Common::MemoryReadStreamEndian(blob.data(), blob.size(), false);

}

uint16 Macs2::AnimationReader::readNumAnimations() {
	// Read the header

	readStream->seek(0, SEEK_SET);

	// bp-22h
	readStream->readUint16();
	// bp-6h
	readStream->readUint16();
	// bp-8h
	readStream->readUint16();
	// bp-0Ah
	readStream->readUint16();
	// bp-10h
	readStream->readUint16();

	// bp-0Eh
	// TODO: Naming probably off
	uint16 firstFrameAdditionalOffset = readStream->readUint16() + 1;

	// Navigate to where bp-24h is saved
	readStream->seek(0x0B + firstFrameAdditionalOffset);

	// bp-24h
	uint16 result = readStream->readUint16();
	return result;
}

void Macs2::AnimationReader::SeekToAnimation(uint16 index) {
	// Read bp-0Eh directly
	readStream->seek(0xA, SEEK_SET);
	// bp-0Eh
	// TODO: Naming probably off
	uint16 firstFrameAdditionalOffset = readStream->readUint16() + 1;
	// Skip reading bp-24h
	readStream->seek(0x0B + firstFrameAdditionalOffset + 0x2, SEEK_SET);
	// TODO: Check for one-off errors
	for (int i = 0; i < index; i++) {
		SkipCurrentAnimationFrame();
	}
	
}

void Macs2::AnimationReader::SkipCurrentAnimationFrame() {
	uint16 value1 = readStream->readUint16();
	uint16 value2 = readStream->readUint16();
	readStream->seek(2, SEEK_CUR);
	uint16 width = readStream->readUint16();
	uint16 height = readStream->readUint16();
	readStream->seek(width * height, SEEK_CUR);
}
