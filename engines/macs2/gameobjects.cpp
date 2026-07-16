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

#include "macs2/gameobjects.h"
#include "common/memstream.h"
#include "macs2/macs2.h"

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
	byte *scriptData = (byte *)malloc(scriptSize);
	fileStream->read(scriptData, scriptSize);
	return new Common::MemoryReadStream(scriptData, scriptSize, DisposeAfterUse::YES);
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
	byte *stringData = (byte *)malloc(size);
	fileStream->read(stringData, size);
	return new Common::MemoryReadStream(stringData, size, DisposeAfterUse::YES);
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
	// Object names from game string dumps. Index matches the object ID used in scripts.
	if (g_engine->isDemo()) {
		_objectNames[0x02] = "Laib Brot";      // sliced
		_objectNames[0x03] = "Laib Brot";      // with key inside
		_objectNames[0x04] = "Schnapsflasche"; // wrapped with rope
		_objectNames[0x05] = "Schnapsflasche";
		_objectNames[0x07] = "Spitzhacke";
		_objectNames[0x08] = "Hackenspitze"; // rusty pick point
		_objectNames[0x09] = "Kerze";        // lit
		_objectNames[0x0A] = "Kerze";        // from dismantled lamp
		_objectNames[0x0B] = "Kerzen";       // 4 candles
		_objectNames[0x0D] = "Korkenzieher";
		_objectNames[0x0E] = "Lampe";
		_objectNames[0x10] = "Lampe";     // second instance
		_objectNames[0x11] = "Holzpfahl"; // sturdy wooden stake
		_objectNames[0x12] = "Wagenrad";
		_objectNames[0x14] = "Salpeterpulver";
		_objectNames[0x15] = "Schaufelspitze"; // blunt
		_objectNames[0x16] = "Lampenschirm";
		_objectNames[0x17] = "Lampenschirme"; // plural, from lamp
		_objectNames[0x18] = "Messingschl\x81ssel";
		_objectNames[0x19] = "Schwarzpulver";
		_objectNames[0x1A] = "Hanfseile"; // tied together
		_objectNames[0x1B] = "Streichholz";
		_objectNames[0x1C] = "Topflappen";
		_objectNames[0x1D] = "Windlicht";
		_objectNames[0x1E] = "Wolle";       // soaked in alcohol
		_objectNames[0x51] = "Stoffbeutel"; // empty
		_objectNames[0x52] = "Stoffbeutel"; // with marbles
		_objectNames[0x53] = "Blasebalg";
		_objectNames[0x54] = "Brennholz";
		_objectNames[0x56] = "Brotmesser";
		_objectNames[0x58] = "B\x81"
							 "cher"; // adventure books
		_objectNames[0x59] = "Clownpuppe";
		_objectNames[0x5A] = "Blechdose";     // green, with sulphur
		_objectNames[0x5B] = "Blechdose";     // empty
		_objectNames[0x5C] = "Papierdrachen"; // with string tail
		_objectNames[0x5D] = "Papierdrachen"; // without tail
		_objectNames[0x5E] = "Blecheimer";    // empty
		_objectNames[0x5F] = "Blecheimer";    // with water
		_objectNames[0x64] = "Kartoffeln";
		_objectNames[0x6F] = "Murmeln";
		_objectNames[0x70] = "Musketen";
		_objectNames[0x74] = "Schaufel";      // old and blunt
		_objectNames[0x7C] = "Schwarzpulver"; // mixing state
		_objectNames[0x7D] = "Sch\x81ssel";   // with dough
		_objectNames[0x7F] = "Schwefel";
		_objectNames[0x80] = "Hanfseil"; // short
		_objectNames[0x81] = "Hanfschnur";
		_objectNames[0x83] = "Socken"; // red
		_objectNames[0x84] = "Spachtel";
		_objectNames[0x85] = "Holzente"; // on wheels
		_objectNames[0x87] = "Tasse";    // empty
		_objectNames[0x88] = "Tasse";    // with oil
		_objectNames[0x89] = "Teig";
		_objectNames[0x8C] = "Wolle"; // ball of wool
		_objectNames[0x8E] = "Holzw\x81rfel";
		_objectNames[0x8F] = "Brief";

		// Characters/NPCs - from output-demo strings + Schatz-Demo RESOURCE.MCS dialogue
		_objectNames[0x06] = "Tramp";              // strings_object006 [0]
		_objectNames[0x0C] = "Droll";              // scene 28/30 dialogue (Tante Droll)
		_objectNames[0x0F] = "Cornel";             // scene 5: Cornel Brinkley (obj_0xf)
		_objectNames[0x13] = "Tramp";              // strings_object019 [0]; tramp henchman
		_objectNames[0x45] = "Rafter";              // scene 30 dialogue
		_objectNames[0x4D] = "Tramp";              // scene 5 tramp henchman
		_objectNames[0x69] = "Wirt";               // scene 28 gatekeeper dialogue
		_objectNames[0x6E] = "Mrs. Butler";        // strings_object110 [0]
	} else {
		// Characters/NPCs - labels from strings_object*.txt [0] and/or scene dialogue (demacs2)
		_objectNames[0x01] = "Old Firehand";       // strings_object001 [0]
		_objectNames[0x02] = "Kapit\xe4n";          // strings_object002 [0-1]
		_objectNames[0x04] = "Bootsjunge";         // strings_object004 [0]
		_objectNames[0x06] = "Tramp";              // strings_object006 [0]; tramp henchman
		_objectNames[0x07] = "M\xe4""dchen";       // strings_object007 [0]
		_objectNames[0x09] = "Panther";            // strings_object009 (panther scene)
		_objectNames[0x0C] = "Droll";              // strings_object012 [0]
		_objectNames[0x0D] = "Patterson";           // strings_object013 [0]
		_objectNames[0x0F] = "Cornel";             // scene 9: Cornel Brinkley (obj_0xf)
		_objectNames[0x12] = "Wachposten";         // scene 12 dialogue
		_objectNames[0x13] = "Tramp";              // strings_object019 [0]; tramp henchman
		_objectNames[0x16] = "Passagierin";        // scene 2 dialogue
		_objectNames[0x21] = "Matrose";             // scene 10 dialogue
		_objectNames[0x27] = "Branshky";            // strings_object039 [0]
		_objectNames[0x35] = "Dieb";               // scene 24 dialogue
		_objectNames[0x45] = "Rafter";              // scene 18: "wir sind Rafter"
		_objectNames[0x4D] = "Tramp";              // tramp henchman, scenes 5/18
		_objectNames[0x69] = "Wirt";               // scene 28 barkeeper dialogue
		_objectNames[0x6E] = "Mrs. Butler";        // strings_object110 [0]
		_objectNames[0x90] = "Bandit";             // scene 29
		_objectNames[0x91] = "Bandit";             // scene 29
		_objectNames[0x92] = "Bandit";             // scene 29
		_objectNames[0x93] = "Winnetou";           // scene 29 dialogue
		_objectNames[0x95] = "Grosser B\xe4""r";   // scene 45: greeted by Winnetou
		_objectNames[0xA7] = "Winnetou";           // scene 45 dialogue
		_objectNames[0xA8] = "Kleiner B\xe4""r";   // scene 45: greeted by Winnetou
		_objectNames[0xB5] = "Winnetou";           // strings_object181; ending scenes

		// Full game - verified against strings_object*.txt dumps
		_objectNames[0x08] = "Brett";           // board
		_objectNames[0x0E] = "Eimer";           // bucket, full of water
		_objectNames[0x10] = "Holzfass";        // wooden barrel, empty
		_objectNames[0x11] = "Bowiemesser";     // bowie knife
		_objectNames[0x14] = "Eimer";           // bucket, full of water
		_objectNames[0x17] = "Hutschachtel";    // hatbox, empty and open
		_objectNames[0x18] = "Damenhut";        // lady's hat with veil
		_objectNames[0x19] = "Hutschachtel";    // strings_object025 [4]
		_objectNames[0x1A] = "Metalleimer";     // metal bucket, empty
		_objectNames[0x1B] = "Feuerhaken";      // fire poker
		_objectNames[0x1C] = "Topflappen";      // pot holder
		_objectNames[0x1D] = "Brett";           // board, wrapped in cloth
		_objectNames[0x1E] = "Kohlenschaufel";  // coal shovel
		_objectNames[0x20] = "Kakerlake";       // cockroach
		_objectNames[0x22] = "Tasse";           // cup, full (coffee)
		_objectNames[0x23] = "Tasse";           // cup, empty
		_objectNames[0x24] = "Axt";             // axe
		_objectNames[0x25] = "Axtklinge";       // axe blade
		_objectNames[0x26] = "Reservestiel";    // spare handle
		_objectNames[0x28] = "Brief";           // letter
		_objectNames[0x29] = "Brot";            // bread, moldy
		_objectNames[0x2A] = "Brot";            // bread, stale
		_objectNames[0x2B] = "Kuvert";          // envelope, open
		_objectNames[0x2C] = "Kuvert";          // envelope, sealed
		_objectNames[0x2D] = "Waschb\x84rm\x81tze"; // strings_object045 [0]
		_objectNames[0x2E] = "Whiskyglas";      // whisky glass
		_objectNames[0x2F] = "Lederg\x81rtel";  // leather belt
		_objectNames[0x30] = "Sch\x81rhaken";   // poker
		_objectNames[0x31] = "Wachhund";           // strings_object049 [0]
		_objectNames[0x34] = "Brett";           // board, solid
		_objectNames[0x36] = "Vogelk\x84"
							 "fig"; // birdcage, with bird
		_objectNames[0x37] = "Vogelk\x84"
							 "fig"; // birdcage, broken
		_objectNames[0x38] = "Vogelk\x84"
							 "fig"; // birdcage, empty
		_objectNames[0x39] = "Vogelk\x84"
							 "fig";             // birdcage, with bird
		_objectNames[0x3A] = "Landkarte";       // map
		_objectNames[0x3B] = "Kerze";           // candle
		_objectNames[0x3C] = "Kieselsteine";    // pebbles
		_objectNames[0x3D] = "Koffer";          // strings_object061 [0]
		_objectNames[0x3E] = "Kleider";         // strings_object062 [0]
		_objectNames[0x3F] = "Lederbeutel";     // leather pouch, empty
		_objectNames[0x40] = "Knallfr\x94sche"; // strings_object064 [5]
		_objectNames[0x41] = "Knallfr\x94sche"; // firecrackers
		_objectNames[0x42] = "Koffer";          // suitcase, open
		_objectNames[0x43] = "Koffer";          // suitcase, closed
		_objectNames[0x44] = "Papier";          // paper, crumpled
		_objectNames[0x47] = "Messer";          // knife, rusty
		_objectNames[0x48] = "Kartonschachtel"; // cardboard box, open and empty
		_objectNames[0x49] = "Kartonschachtel"; // cardboard box
		_objectNames[0x4A] = "Schal";             // strings_object074 [0-1]
		_objectNames[0x4B] = "Schilfrohr";      // reeds, dry
		_objectNames[0x4C] = "Schilfrohr";      // reeds, straight
		_objectNames[0x4F] = "Schnapsflasche";  // liquor bottle
		// Objects 81+ (0x51+)
		_objectNames[0x51] = "Stoffbeutel"; // cloth bag, empty
		_objectNames[0x52] = "Stoffbeutel"; // cloth bag, with marbles
		_objectNames[0x53] = "Blasebalg";   // bellows
		_objectNames[0x54] = "Brennholz";   // firewood
		_objectNames[0x55] = "Laib Brot";   // loaf of bread, sliced
		_objectNames[0x56] = "Brotmesser";  // bread knife
		_objectNames[0x57] = "Laib Brot";   // loaf of bread, with key
		_objectNames[0x58] = "B\x81"
							 "cher";                // books
		_objectNames[0x59] = "Clownpuppe";          // clown doll
		_objectNames[0x5A] = "Blechdose";           // tin can, green with sulphur
		_objectNames[0x5B] = "Blechdose";           // tin can, empty
		_objectNames[0x5C] = "Papierdrachen";       // paper kite, with string
		_objectNames[0x5D] = "Papierdrachen";       // paper kite, without tail
		_objectNames[0x5E] = "Blecheimer";          // tin bucket, empty
		_objectNames[0x5F] = "Blecheimer";          // tin bucket, full of water
		_objectNames[0x60] = "Schnapsflasche";      // liquor bottle, wrapped
		_objectNames[0x61] = "Schnapsflasche";      // liquor bottle
		_objectNames[0x62] = "Spitzhacke";          // pickaxe
		_objectNames[0x63] = "Hackenspitze";        // pick point, rusty
		_objectNames[0x64] = "Kartoffeln";          // potatoes
		_objectNames[0x65] = "Kerze";               // candle, lit
		_objectNames[0x66] = "Kerze";               // candle, from lamp
		_objectNames[0x67] = "Kerzen";              // candles, four
		_objectNames[0x6A] = "Holzkohle";           // charcoal
		_objectNames[0x6B] = "Korkenzieher";        // corkscrew
		_objectNames[0x6C] = "Lampe";               // lamp
		_objectNames[0x6D] = "Lampe";               // lamp, second instance
		_objectNames[0x6F] = "Murmeln";             // marbles
		_objectNames[0x70] = "Musketen";            // muskets
		_objectNames[0x71] = "Holzpfahl";           // wooden stake
		_objectNames[0x72] = "Wagenrad";            // wagon wheel
		_objectNames[0x73] = "Salpeterpulver";      // saltpeter
		_objectNames[0x74] = "Schaufel";            // shovel
		_objectNames[0x75] = "Schaufelspitze";      // shovel blade
		_objectNames[0x76] = "Lampenschirm";        // lamp shade
		_objectNames[0x77] = "Lampenschirme";       // lamp shades
		_objectNames[0x78] = "Messingschl\x81ssel"; // brass key
		_objectNames[0x7C] = "Schwarzpulver";       // gunpowder, mixing
		_objectNames[0x7D] = "Sch\x81ssel";         // bowl, with dough
		_objectNames[0x7E] = "Schwarzpulver";       // gunpowder
		_objectNames[0x7F] = "Schwefel";            // sulphur
		_objectNames[0x80] = "Hanfseil";            // hemp rope, short
		_objectNames[0x81] = "Hanfschnur";          // hemp string
		_objectNames[0x82] = "Hanfseile";           // hemp ropes, tied together
		_objectNames[0x83] = "Socken";              // socks, red
		_objectNames[0x84] = "Spachtel";            // spatula
		_objectNames[0x85] = "Holzente";            // wooden duck
		_objectNames[0x86] = "Streichholz";         // match
		_objectNames[0x87] = "Tasse";               // cup, empty
		_objectNames[0x88] = "Tasse";               // cup, with oil
		_objectNames[0x89] = "Teig";                // dough
		_objectNames[0x8A] = "Topflappen";          // pot holder
		_objectNames[0x8B] = "Windlicht";           // lantern
		_objectNames[0x8C] = "Wolle";               // wool
		_objectNames[0x8D] = "Wolle";               // wool, soaked in alcohol
		_objectNames[0x8E] = "Holzw\x81rfel";       // wooden blocks
		_objectNames[0x8F] = "Brief";               // letter
		_objectNames[0x94] = "Lore";                // mine cart
		_objectNames[0x96] = "Lederbeutel";         // leather pouch, empty
		_objectNames[0x97] = "Lederbeutel";         // leather pouch, full of sand
		_objectNames[0x98] = "Bohlen";              // planks
		_objectNames[0x99] = "Brecheisen";          // crowbar
		_objectNames[0x9A] = "Sand";                // sand
		_objectNames[0x9B] = "Dynamit";             // dynamite
		_objectNames[0x9C] = "Fackel";              // torch, burning
		_objectNames[0x9D] = "Fackel";              // torch
		_objectNames[0x9E] = "Holzfigur";           // wooden figure, empty
		_objectNames[0x9F] = "Figur";               // figure, assembled
		_objectNames[0xA0] = "Holzfigur";           // wooden figure, empty
		_objectNames[0xA1] = "Flaschenzug";         // pulley
		_objectNames[0xA2] = "Haken";               // hook
		_objectNames[0xA3] = "Haken und Seil";      // hook and rope
		_objectNames[0xA4] = "Kacheln";             // tiles
		_objectNames[0xA6] = "Nase";                // nose
		_objectNames[0xAB] = "Quarzsand";           // quartz sand
		_objectNames[0xAC] = "Quarzsand";           // quartz sand
		_objectNames[0xAD] = "Griff";               // handle
		_objectNames[0xAE] = "Hanfseil";            // hemp rope
		_objectNames[0xAF] = "Sicheln";             // sickles
		_objectNames[0xB0] = "Eisen";               // irons, two angled
		_objectNames[0xB1] = "Eisen";               // iron
		_objectNames[0xB2] = "Schraubenzieher";     // screwdriver
		_objectNames[0xB3] = "Eisenstange";         // iron bar
		_objectNames[0xB4] = "Tomahawk";            // tomahawk
	}
}

bool Macs2::GameObjects::isNpcIndex(uint16 objectIndex) {
	if (objectIndex == 0)
		return false;

	if (g_engine->isDemo()) {
		static const uint16 kDemoNpcIndices[] = {
			0x06, 0x0C, 0x0F, 0x13, 0x45, 0x4D, 0x69, 0x6E, 0
		};
		for (uint i = 0; kDemoNpcIndices[i] != 0; ++i) {
			if (kDemoNpcIndices[i] == objectIndex)
				return true;
		}
		return false;
	}

	static const uint16 kFullGameNpcIndices[] = {
		0x01, 0x02, 0x04, 0x06, 0x07, 0x09, 0x0C, 0x0D, 0x0F, 0x12, 0x13, 0x16,
		0x21, 0x27, 0x35, 0x45, 0x4D, 0x69, 0x6E, 0x90, 0x91, 0x92, 0x93, 0x95,
		0xA7, 0xA8, 0xB5, 0
	};
	for (uint i = 0; kFullGameNpcIndices[i] != 0; ++i) {
		if (kFullGameNpcIndices[i] == objectIndex)
			return true;
	}
	return false;
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
	byte *stringData = (byte *)malloc(size);
	fileStream->read(stringData, size);
	return new Common::MemoryReadStream(stringData, size, DisposeAfterUse::YES);
	// Note: We save the current scene number to [0F86h] - maybe "scene we have strings loaded for"?
}

Common::Array<uint8> *Macs2::GameObject::getAnimSlotBlob(uint16 slot) {
	if (slot < 1 || slot > 0x15)
		return nullptr;
	if (slot == 0x15) {
		if (_blobs.size() > 20 && !_blobs[20].empty())
			return &_blobs[20];
		return &_overloadAnimation;
	}
	const uint index = slot - 1;
	if (index >= _blobs.size())
		return nullptr;
	return &_blobs[index];
}

const Common::Array<uint8> *Macs2::GameObject::getAnimSlotBlob(uint16 slot) const {
	return const_cast<GameObject *>(this)->getAnimSlotBlob(slot);
}

bool Macs2::GameObject::isAnimSlotLoaded(uint16 orient) const {
	if (_overloadAnimTriggerDirection != 0x7FFF &&
		(int16)_overloadAnimTriggerDirection >= 0 &&
		_overloadAnimTriggerDirection == orient) {
		const Common::Array<uint8> *blob = getAnimSlotBlob(0x15);
		return blob != nullptr && !blob->empty();
	}
	if (orient == 0x15) {
		const Common::Array<uint8> *blob = getAnimSlotBlob(0x15);
		return blob != nullptr && !blob->empty();
	}
	if (orient < 1 || orient > 0x14)
		return false;
	const uint slot = orient - 1;
	if (slot < _blobs.size() && !_blobs[slot].empty())
		return true;
	// Binary drawAllCharacters (1008:90a2): bSlotLoaded at slot+0x33; file speeds often 0x010x.
	if (slot < _blobWalkSpeeds.size() && (_blobWalkSpeeds[slot] & 0xFF00) != 0)
		return true;
	return false;
}

Common::MemoryReadStream *Macs2::GameObject::getScriptStream() {
	return new Common::MemoryReadStream(_script.data(), _script.size());
}

Macs2::AnimationReader::AnimationReader(const Common::Array<uint8> &blob) {
	_readStream = new Common::MemoryReadStreamEndian(blob.data(), blob.size(), false);
}

Macs2::AnimationReader::~AnimationReader() {
	delete _readStream;
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
