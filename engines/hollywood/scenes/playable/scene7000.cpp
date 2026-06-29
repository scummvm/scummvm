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

#include "hollywood/scenes/playable/scene7000.h"

#include "common/array.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/path.h"
#include "common/textconsole.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kChapter7MusicArchiveName = "RESOURCE.M07";
const char *const kResource000Name = "RESOURCE.000";
const uint16 kScene7000MusicCueId = 0x000c;
const byte kScene7000MusicVolumePercent = 10;
const uint kResource000HeaderByteCount = 1;
const uint kResource000TableByteCount = 400;
const uint kResource000InventoryActionTablesEntry = 0xc8;
const uint kResource000FixedInventoryVerbTableOffset = 0xec54;

Scene7000::Scene7000(HollywoodEngine *vm) :
		_vm(vm) {
}

bool Scene7000::play() {
	GameplayState &state = _vm->gameState();
	state.initializeForState7000();
	if (!loadSueInventoryResourceTables())
		return false;

	MusicPlayer *music = _vm->gameplayMusic();
	music->setArchive(Common::Path(kChapter7MusicArchiveName));
	music->playMusicCue(kScene7000MusicCueId, kScene7000MusicVolumePercent);

	debugC(1, kDebugScene, "Scene 7000 initialized gameplay bootstrap; next state=0x%04x", state.mainFlowStateId);
	return true;
}

bool Scene7000::loadSueInventoryResourceTables() {
	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s inventory action tables", kResource000Name);
		return false;
	}

	if ((uint32)file.size() < kResource000HeaderByteCount + 2 * kResource000TableByteCount) {
		warning("%s is too small for inventory action tables", kResource000Name);
		return false;
	}

	Common::Array<byte> offsetTable;
	offsetTable.resize(kResource000TableByteCount);
	file.seek(kResource000HeaderByteCount);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size()) {
		warning("Failed to read %s offset table", kResource000Name);
		return false;
	}

	if (kResource000InventoryActionTablesEntry + 4 > offsetTable.size()) {
		warning("%s inventory action table entry is out of range", kResource000Name);
		return false;
	}

	const uint32 tableOffset = READ_LE_UINT32(offsetTable.data() + kResource000InventoryActionTablesEntry);
	const uint32 fixedTableOffset = tableOffset + kResource000FixedInventoryVerbTableOffset;
	const uint fixedTableEntryCount = GameplayState::kFixedInventoryActionTableEntryCount - 1;
	const uint relationTableEntryCount = GameplayState::kInventoryItemRelationTableEntryCount;
	const uint32 requiredTableBytes = fixedTableEntryCount * 2 + relationTableEntryCount * 2 * 2;
	if (fixedTableOffset > (uint32)file.size() ||
			requiredTableBytes > (uint32)file.size() - fixedTableOffset) {
		warning("%s inventory action tables are out of range", kResource000Name);
		return false;
	}

	GameplayState &state = _vm->gameState();
	state.clearInventoryActionTables();

	file.seek(fixedTableOffset);
	for (uint i = 1; i < GameplayState::kFixedInventoryActionTableEntryCount; ++i)
		state.fixedInventoryVerbHandlerIdsByItemAndStrip[i] = file.readUint16LE();
	for (uint i = 0; i < GameplayState::kInventoryItemRelationTableEntryCount; ++i)
		state.dialogueRelationMode1HandlerIdsByItemPair[i] = file.readUint16LE();
	for (uint i = 0; i < GameplayState::kInventoryItemRelationTableEntryCount; ++i)
		state.dialogueRelationMode2HandlerIdsByItemPair[i] = file.readUint16LE();
	if (file.err()) {
		warning("Failed to read %s inventory action tables", kResource000Name);
		return false;
	}

	state.sueInventoryResourceTablesLoaded = true;
	return true;
}

} // End of namespace Hollywood
