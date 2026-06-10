#include "gameobjects.h"
#include "common/memstream.h"

namespace Common {

DECLARE_SINGLETON(Macs2::GameObjects);
DECLARE_SINGLETON(Macs2::Scenes);

} // namespace Common

Common::MemoryReadStream *Macs2::Scenes::readSceneScript(uint16 sceneIndex, Common::MemoryReadStream *fileStream) {
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
	if (scriptSize == 0) {
		warning("Macs2::Scenes::ReadSceneScript: scene %u has empty script", sceneIndex);
		// Return a valid but empty stream
		return new Common::MemoryReadStream(nullptr, 0);
	}
	uint8 *scriptData = new uint8[scriptSize];
	fileStream->read(scriptData, scriptSize);
	// TODO: Consider using the endian version for all the memoryReadStreams
	return new Common::MemoryReadStream(scriptData, scriptSize);
}

Common::Array<uint32> Macs2::Scenes::readSpecialAnimsOffsets(uint16 sceneIndex, Common::MemoryReadStream *fileStream) {
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

Common::MemoryReadStream *Macs2::Scenes::readSceneStrings(uint16 sceneIndex, Common::MemoryReadStream *fileStream) {
	// Calculate the offset of the script data offset
	// This addressing can be found in the l0037_2856 code block

	uint16 sceneDataOffset = sceneIndex * 0xC;
	// Offset of the data in [0752h] global
	constexpr uint16 globalDataOffset = 0xC + 0x4;
	sceneDataOffset += globalDataOffset;
	fileStream->seek(sceneDataOffset - 0x4);
	uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);

	// This lives in l0037_A4FC:

	// Read the script from there
	// Size - lives in global [0F84h]
	uint16 size = fileStream->readUint16LE();

	// Stringdata lives in the pointer [0F80h]
	uint8 *stringData = new uint8[size];
	fileStream->read(stringData, size);
	// TODO: Consider using the endian version for all the memoryReadStreams
	return new Common::MemoryReadStream(stringData, size);
	// Note: We save the current scene number to [0F86h] - maybe "scene we have strings loaded for"?
}

Common::Array<uint8> Macs2::Scenes::readSpecialAnimBlob(uint16 index, Common::MemoryReadStream *fileStream) {
	uint32 offset = _currentSceneSpecialAnimOffsets[index - 1];
	fileStream->seek(offset, SEEK_SET);
	uint32 length = fileStream->readUint32LE();
	// Skip a string - note the original code adds 0x4 for the previously read size since
	// it does not use the stream position
	fileStream->seek(0xC, SEEK_CUR);
	Common::Array<uint8> result(length);
	fileStream->read(result.data(), length);
	return result;
}

void Macs2::GameObjects::init() {
	_objectNames.resize(0xFF);
	// TODO: Make a text file instead - demo and full game differ
	_objectNames[0x02] = "Laib Brot"; // aufgeschnitten
	_objectNames[0x03] = "Laib Brot";
	_objectNames[0x04] = "Schnapsflasche"; // umwickelt
	_objectNames[0x05] = "Schnapsflasche";
	_objectNames[0x07] = "Spitzhacke";
	_objectNames[0x08] = "Brett"; // leer
	_objectNames[0x09] = "Kerze"; // an
	_objectNames[0x0A] = "Kerze"; // aus
	_objectNames[0x0B] = "Kerzen"; // 4 Kerzen
	_objectNames[0x0D] = "Korkenzieher";
	_objectNames[0x0E] = "Lampe";
	_objectNames[0x10] = "Holzfass"; // leer
	_objectNames[0x11] = "Bowiemesser";
	_objectNames[0x12] = "Wagenrad";
	_objectNames[0x14] = "Salpeterpulver";
	_objectNames[0x15] = "Schaufelspitze";
	_objectNames[0x16] = "Lampenschirm";
	_objectNames[0x17] = "Hutschachtel"; // leer und offen
	_objectNames[0x18] = "Damenhut";     // Mit Schleier
	_objectNames[0x19] = "Schwarzpulver";
	_objectNames[0x1a] = "Eimer";        // leer
	_objectNames[0x1b] = "Feuerhaken";
	_objectNames[0x1c] = "Topflappen";
	_objectNames[0x1d] = "Windlicht";
	_objectNames[0x1e] = "Wolle"; // getränkt
	_objectNames[0x23] = "Tasse"; // leer
	_objectNames[0x24] = "Axt";
	_objectNames[0x24] = "Whiskyglas"; // leer
	_objectNames[0x2E] = "Whiykglas";
	_objectNames[0x30] = "Schürhaken";
	_objectNames[0x43] = "Koffer"; // geschlossen
	_objectNames[0x47] = "Messer";
	_objectNames[0x49] = "Kartonschachtel";
	_objectNames[0x51] = "Stoffbeutel"; // leer
	_objectNames[0x52] = "Stoffbeutel"; // Mit Murmeln
	_objectNames[0x53] = "Blasebalg";
	_objectNames[0x54] = "Brennholz";
	_objectNames[0x55] = "Brot";
	_objectNames[0x56] = "Brotmesser";
	_objectNames[0x57] = "Brot";
	_objectNames[0x58] = "Bücher";
	_objectNames[0x59] = "Clownpuppe";
	_objectNames[0x5A] = "Blechdose"; // grün, mit Schwefel
	_objectNames[0x5B] = "Blechdose";
	_objectNames[0x5C] = "Papierdrachen";
	_objectNames[0x5D] = "Papierdrache";
	_objectNames[0x5E] = "Blecheimer";
	_objectNames[0x5F] = "Blecheimer"; // gefüllt
	_objectNames[0x61] = "Schnapsflasche";
	_objectNames[0x62] = "Kerze"; // mit Flamme
	_objectNames[0x64] = "Kartoffeln";
	_objectNames[0x65] = "Spitzhacke";
	_objectNames[0x66] = "Kerze";
	_objectNames[0x67] = "Kerzen";
	_objectNames[0x6B] = "Korkenzieher";
	_objectNames[0x6A] = "Holzkohle";
	_objectNames[0x6B] = "Korkenzieher";
	_objectNames[0x6C] = "Kronleuchter"; // ohne Schirme
	_objectNames[0x6D] = "Kronleuchter"; // mit Schirmen
	_objectNames[0x6F] = "Murmeln";
	_objectNames[0x70] = "Musketen";
	_objectNames[0x72] = "Rad";
	_objectNames[0x74] = "Spaten";
	_objectNames[0x70] = "Musketen";
	_objectNames[0x73] = "Salpeter";
	_objectNames[0x76] = "Lampenschirm";
	_objectNames[0x77] = "Lampenschirme";
	_objectNames[0x78] = "Messingschlüssel";
	_objectNames[0x7C] = "Schüssel"; // mit Schwefel
	_objectNames[0x7D] = "Schüssel";
	_objectNames[0x7E] = "Schwarzpulver";
	_objectNames[0x7F] = "Schwefel";
	_objectNames[0x80] = "Hanfseil";
	_objectNames[0x81] = "Hanfschnur";
	_objectNames[0x82] = "Seil";
	_objectNames[0x83] = "Socken";
	_objectNames[0x84] = "Spachtel";
	_objectNames[0x85] = "Holzente";
	_objectNames[0x86] = "Streichholz";
	_objectNames[0x82] = "Seil";
	_objectNames[0x87] = "Tasse"; // leer
	_objectNames[0x88] = "Tasse"; // mit Öl
	_objectNames[0x89] = "Teig";
	_objectNames[0x8A] = "Topflappen";
	_objectNames[0x8B] = "Windlicht";
	_objectNames[0x8C] = "Wolle";
	_objectNames[0x8D] = "Wolle"; // mit Alkohol
	_objectNames[0x8E] = "Holzwürfel";
	_objectNames[0x8F] = "Brief";
	_objectNames[0x96] = "Lederbeutel"; // leer
	_objectNames[0x97] = "Lederbeutel"; // voll Sand
	_objectNames[0x99] = "Brecheisen";
	_objectNames[0x9B] = "Dynamit";
	_objectNames[0x9A] = "Sand";   // brennend
	_objectNames[0x9C] = "Fackel"; // brennend
	_objectNames[0x9D] = "Fackel"; //
	_objectNames[0x9F] = "Figur";  // zusammengesetzt
	_objectNames[0x98] = "Bohlen";
	_objectNames[0x9D] = "Holzfigur"; // Von links
	_objectNames[0xA0] = "Holzfigur"; // von Rechts
	_objectNames[0xA1] = "Seilzug";
	_objectNames[0xA2] = "Haken";
	_objectNames[0xA3] = "Haken und Seil";
	_objectNames[0xA4] = "Kacheln";
	_objectNames[0xA6] = "Nase";
	_objectNames[0xAC] = "Sand";
	_objectNames[0xAD] = "Griff";
	_objectNames[0xAE] = "Seil";
	_objectNames[0xAF] = "Sicheln";
	_objectNames[0xB0] = "Eisen"; // Zwei Eisen
	_objectNames[0xB1] = "Eisen";
	_objectNames[0xB2] = "Schraubenzieher";
	_objectNames[0xB3] = "Eisenstange";
	_objectNames[0xB4] = "Tomahawk";
}

Macs2::GameObject *Macs2::GameObjects::getProtagonistObject() {
	return instance().getObjectByIndex(1);
}

Macs2::GameObject *Macs2::GameObjects::getObjectByIndex(uint16 index) {
	if ((uint)(index - 1) >= instance()._objects.size()) {
		return nullptr;
	}
	return instance()._objects[index - 1];
}

Common::MemoryReadStream *Macs2::GameObjects::readGameObjectStrings(uint16 index, Common::MemoryReadStream *fileStream) {
	// TODO: The original binary caches the last loaded object's string data in memory
	// (g_wStringDecodeCacheObjectId at DS:0f86 / g_pSavedScriptState). It skips the file
	// read if the same object is requested again. We re-read from file every time.
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

	// This lives in l0037_A4FC:

	// Read the script from there
	// Size - lives in global [0F84h]
	uint16 size = fileStream->readUint16LE();

	// Stringdata lives in the pointer [0F80h]
	uint8 *stringData = new uint8[size];
	fileStream->read(stringData, size);
	// TODO: Consider using the endian version for all the memoryReadStreams
	return new Common::MemoryReadStream(stringData, size);
	// Note: We save the current scene number to [0F86h] - maybe "scene we have strings loaded for"?
}

Common::MemoryReadStream *Macs2::GameObject::getScriptStream() {
	return new Common::MemoryReadStream(_script.data(), _script.size());
}

Macs2::AnimationReader::AnimationReader(const Common::Array<uint8> &blob) {
	_readStream = new Common::MemoryReadStreamEndian(blob.data(), blob.size(), false);
}

uint16 Macs2::AnimationReader::readNumAnimations() {
	// Read the header

	_readStream->seek(0, SEEK_SET);

	// bp-22h
	_readStream->readUint16();
	// bp-6h
	_readStream->readUint16();
	// bp-8h
	_readStream->readUint16();
	// bp-0Ah
	_readStream->readUint16();
	// bp-10h
	_readStream->readUint16();
	// Offset 0xA: number of command bytes in the control section (bp-0Eh in advanceAnimFrame)
	uint16 commandSectionLength = _readStream->readUint16() + 1;

	// Frame count (bp-24h) is stored right after the header + command section
	_readStream->seek(0x0B + commandSectionLength);

	// bp-24h
	uint16 result = _readStream->readUint16();
	return result;
}

void Macs2::AnimationReader::seekToAnimation(uint16 index) {
	// Read bp-0Eh directly
	_readStream->seek(0xA, SEEK_SET);
	// Offset 0xA: command section length (bp-0Eh in advanceAnimFrame)
	uint16 commandSectionLength = _readStream->readUint16() + 1;
	// Skip reading bp-24h
	_readStream->seek(0x0B + commandSectionLength + 0x2, SEEK_SET);
	for (int i = 0; i < index; i++) {
		skipCurrentAnimationFrame();
	}
}

void Macs2::AnimationReader::skipCurrentAnimationFrame() {
	_readStream->readUint16(); // value1
	_readStream->readUint16(); // value2
	_readStream->seek(2, SEEK_CUR);
	uint16 width = _readStream->readUint16();
	uint16 height = _readStream->readUint16();
	_readStream->seek(width * height, SEEK_CUR);
}
