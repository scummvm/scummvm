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

#include "graphics/managed_surface.h"

#include "mtropolis/plugin/mti.h"
#include "mtropolis/plugins.h"

#include "mtropolis/miniscript.h"

#include "common/random.h"

namespace MTropolis {

namespace MTI {
	

/*
Board layout:

Layer 0:
     0  1  2  3  4  5  6  7  8  9 10 11 12
  +-----+-----+-----+-----+-----+-----+
0 +  0  |  1  |  2  |  3  |  4  |  5  |
1 +-----+-----+-----+-----+-----+-----+
2 +-----+  7  |  8  |  9  | 10  +-----+-----+
3 |  6  +-----+-----+-----+-----+ 11  | 12  |
4 +-----+ 13  | 14  | 15  | 16  +-----+-----+
5 +-----+-----+-----+-----+-----+-----+
6 + 17  | 18  | 19  | 20  | 21  | 22  |
  +-----+-----+-----+-----+-----+-----+

Layer 1:
     0  1  2  3  4  5  6  7  8  9 10 11 12
  
0 
1             +-----+-----+
2             | 23  | 24  |
3             +-----+-----+
4             | 25  | 26  |
5             +-----+-----+
6

Layer 2:
     0  1  2  3  4  5  6  7  8  9 10 11 12
  
0 
1             
2                +-----+
3                | 27  |
4                +-----+
5
6

*/

ShanghaiModifier::TileCoordinate ShanghaiModifier::_tileCoordinates[ShanghaiModifier::kNumTiles] = {
	{0, 0, 0},
	{2, 0, 0},
	{4, 0, 0},
	{6, 0, 0},
	{8, 0, 0},
	{10, 0, 0},

	{0, 3, 0},
	{2, 2, 0},
	{4, 2, 0},
	{6, 2, 0},
	{8, 2, 0},
	{10, 3, 0},
	{12, 3, 0},

	{2, 4, 0},
	{4, 4, 0},
	{6, 4, 0},
	{8, 4, 0},

	{0, 6, 0},
	{2, 6, 0},
	{4, 6, 0},
	{6, 6, 0},
	{8, 6, 0},
	{10, 6, 0},

	{4, 2, 0},
	{6, 2, 0},

	{4, 4, 1},
	{6, 4, 1},

	{5, 3, 2},
};

ShanghaiModifier::ShanghaiModifier() {
	for (uint x = 0; x < kBoardSizeX; x++)
		for (uint y = 0; y < kBoardSizeY; y++)
			for (uint z = 0; z < kBoardSizeZ; z++)
				_tileAtCoordinate[x][y][z] = -1;

	for (uint i = 0; i < kNumTiles; i++) {
		const TileCoordinate &coord = _tileCoordinates[i];
		assert(coord.x < kBoardSizeX);
		assert(coord.y < kBoardSizeY);
		assert(coord.z < kBoardSizeZ);
		_tileAtCoordinate[coord.x][coord.y][coord.z] = i;
	}
}

ShanghaiModifier::~ShanghaiModifier() {
}

bool ShanghaiModifier::respondsToEvent(const Event &evt) const {
	if (_resetTileSetWhen.respondsTo(evt))
		return true;

	return false;
}

VThreadState ShanghaiModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_resetTileSetWhen.respondsTo(msg->getEvent())) {
		uint tileFaces[kNumTiles];

		resetTiles(*runtime->getRandom(), tileFaces);

		Modifier *varMod = this->_tileSetRef.resolution.lock().get();

		if (varMod == nullptr || !varMod->isVariable()) {
			warning("Shanghai reset var ref was unavailable");
			return kVThreadError;
		}

		VariableModifier *var = static_cast<VariableModifier *>(varMod);

		Common::SharedPtr<DynamicList> list(new DynamicList());

		for (uint i = 0; i < kNumTiles; i++) {
			DynamicValue tileValue;
			tileValue.setInt(tileFaces[i]);

			list->setAtIndex(i, tileValue);
		}

		DynamicValue listValue;
		listValue.setList(list);

		MiniscriptThread thread(runtime, nullptr, nullptr, nullptr, this);
		var->varSetValue(&thread, listValue);

		return kVThreadReturn;
	}

	return kVThreadReturn;
}

void ShanghaiModifier::disable(Runtime *runtime) {
}

bool ShanghaiModifier::load(const PlugInModifierLoaderContext &context, const Data::MTI::ShanghaiModifier &data) {
	if (data.resetWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_resetTileSetWhen.load(data.resetWhen.value.asEvent))
		return false;

	if (data.tileSetVar.type != Data::PlugInTypeTaggedValue::kVariableReference)
		return false;

	_tileSetRef = VarReference(data.tileSetVar.value.asVarRefGUID, "");

	return true;
}

void ShanghaiModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	_tileSetRef.linkInternalReferences(scope);
}

void ShanghaiModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	_tileSetRef.visitInternalReferences(visitor);
}

void ShanghaiModifier::resetTiles(Common::RandomSource &rng, uint (&tileFaces)[kNumTiles]) const {
	uint possibleFaces[kNumFaces];
	uint numPossibleFaces = kNumFaces;

	for (uint i = 0; i < kNumFaces; i++)
		possibleFaces[i] = i + 1;

	uint facesToInsert[kNumTiles / 2];
	uint numFacesToInsert = kNumTiles / 2;

	// Pick random faces, each one gets inserted twice
	for (uint i = 0; i < kNumTiles / 4u; i++) {
		uint faceToInsert = selectAndRemoveOne(rng, possibleFaces, numPossibleFaces);
		facesToInsert[i * 2 + 0] = faceToInsert;
		facesToInsert[i * 2 + 1] = faceToInsert;
	}

	// We build the board by adding all tiles and then randomly picking 2 exposed tiles and
	// assigning them a matching pair.  A pair is only valid if the resulting board state has
	// valid moves.
	BoardState_t boardState = emptyBoardState();
	for (uint i = 0; i < kNumTiles; i++)
		boardState = boardState | boardStateBit(i);

	for (uint pair = 0; pair < kNumTiles / 2u; pair++) {
		uint exposedTiles[kNumTiles];
		uint numExposedTiles = 0;

		for (uint i = 0; i < kNumTiles; i++) {
			if (boardState & boardStateBit(i)) {
				if (tileIsExposed(boardState, i))
					exposedTiles[numExposedTiles++] = i;
			}
		}

		uint firstExposedTile = selectAndRemoveOne(rng, exposedTiles, numExposedTiles);

		BoardState_t withFirstRemoved = boardState ^ boardStateBit(firstExposedTile);

		uint secondExposedTile = selectAndRemoveOne(rng, exposedTiles, numExposedTiles);
		BoardState_t withBothRemoved = withFirstRemoved ^ boardStateBit(secondExposedTile);

		if (numExposedTiles > 0) {
			// If this isn't the last move, validate that this won't result in a stuck board state (e.g. only one tile exposed)
			// If it would result in such a state, pick a different move.
			for (;;) {
				if (boardStateHasValidMove(withBothRemoved))
					break;

				if (numExposedTiles == 0) {
					error("Shanghai board creation failed, board state was %x, removed %u to produce board state %x", static_cast<uint>(boardState), firstExposedTile, static_cast<uint>(withFirstRemoved));
					break;
				}

				secondExposedTile = selectAndRemoveOne(rng, exposedTiles, numExposedTiles);
				withBothRemoved = withFirstRemoved ^ boardStateBit(secondExposedTile);
			}
		}

		boardState = withBothRemoved;

		uint faceToInsert = selectAndRemoveOne(rng, facesToInsert, numFacesToInsert);
		tileFaces[firstExposedTile] = faceToInsert;
		tileFaces[secondExposedTile] = faceToInsert;

		debug(2, "Shanghai randomizer: Move %u is %u + %u", pair, firstExposedTile, secondExposedTile);
	}
}

uint ShanghaiModifier::selectAndRemoveOne(Common::RandomSource &rng, uint *valuesList, uint &listSize) {
	if (listSize == 0) {
		error("Internal error: selectAndRemoveOne ran out of values");
		return 0;
	}

	if (listSize == 1) {
		listSize = 0;
		return valuesList[0];
	}

	uint selectedIndex = rng.getRandomNumber(listSize - 1);
	uint selectedValue = valuesList[selectedIndex];

	valuesList[selectedIndex] = valuesList[listSize - 1];
	listSize--;

	return selectedValue;
}

bool ShanghaiModifier::boardStateHasValidMove(BoardState_t boardState) const {
	uint numExposedTiles = 0;
	for (uint i = 0; i < kNumTiles; i++) {
		if (boardState & boardStateBit(i)) {
			if (tileIsExposed(boardState, i)) {
				numExposedTiles++;
				if (numExposedTiles == 2)
					return true;
			}
		}
	}

	return false;
}

bool ShanghaiModifier::tileIsExposed(BoardState_t boardState, uint tile) const {
	uint tileX = _tileCoordinates[tile].x;
	uint tileY = _tileCoordinates[tile].y;
	uint tileZ = _tileCoordinates[tile].z;

	uint blockMinY = tileY;
	uint blockMaxY = tileY;
	if (blockMinY > 0)
		blockMinY--;
	if (blockMaxY < kBoardSizeY - 1u)
		blockMaxY++;

	bool blockedOnLeft = false;

	if (tileX >= 2) {
		// Check for left-side blocks
		for (uint y = blockMinY; y <= blockMaxY; y++) {
			if (tileExistsAtCoordinate(boardState, tileX - 2, y, tileZ)) {
				blockedOnLeft = true;
				break;
			}
		}
	}

	if (blockedOnLeft) {
		bool blockedOnRight = false;

		// Check for right-side blocks
		if (tileX < kBoardSizeX - 2u) {
			for (uint y = blockMinY; y <= blockMaxY; y++) {
				if (tileExistsAtCoordinate(boardState, tileX + 2, y, tileZ)) {
					blockedOnRight = true;
					break;
				}
			}
		}

		// Tile is blocked on left and right
		if (blockedOnRight)
			return false;
	}

	// Check upper blocks
	uint blockMinX = tileX;
	uint blockMaxX = tileX;
	if (blockMinX > 0)
		blockMinX--;
	if (blockMaxX < kBoardSizeX - 1u)
		blockMaxX++;

	for (uint z = tileZ + 1; z < kBoardSizeZ; z++) {
		for (uint x = blockMinX; x <= blockMaxX; x++) {
			for (uint y = blockMinY; y <= blockMaxY; y++) {
				if (tileExistsAtCoordinate(boardState, x, y, z))
					return false;
			}
		}
	}

	return true;
}

bool ShanghaiModifier::tileExistsAtCoordinate(BoardState_t boardState, uint x, uint y, uint z) const {
	assert(x < kBoardSizeX);
	assert(y < kBoardSizeY);
	assert(z < kBoardSizeZ);

	int8 tile = _tileAtCoordinate[x][y][z];

	if (tile < 0)
		return false;

	if (boardState & boardStateBit(static_cast<uint>(tile)))
		return true;

	return false;
}

ShanghaiModifier::BoardState_t ShanghaiModifier::boardStateBit(uint bit) {
	return static_cast<BoardState_t>(1) << bit;
}

ShanghaiModifier::BoardState_t ShanghaiModifier::emptyBoardState() {
	return 0;
}


#ifdef MTROPOLIS_DEBUG_ENABLE
void ShanghaiModifier::debugInspect(IDebugInspectionReport *report) const {
}
#endif

Common::SharedPtr<Modifier> ShanghaiModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ShanghaiModifier(*this));
}

const char *ShanghaiModifier::getDefaultName() const {
	return "Shanghai Modifier";	// ???
}

PrintModifier::PrintModifier() {
}

PrintModifier::~PrintModifier() {
}

bool PrintModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt);
}

VThreadState PrintModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	warning("Print modifier is not implemented");
	return kVThreadReturn;
}

void PrintModifier::disable(Runtime *runtime) {
}

bool PrintModifier::load(const PlugInModifierLoaderContext &context, const Data::MTI::PrintModifier &data) {
	if (data.executeWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (data.filePath.type != Data::PlugInTypeTaggedValue::kString)
		return false;

	_filePath = data.filePath.value.asString;

	if (!_executeWhen.load(data.executeWhen.value.asEvent))
		return false;

	return true;
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PrintModifier::debugInspect(IDebugInspectionReport *report) const {
}
#endif

Common::SharedPtr<Modifier> PrintModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PrintModifier(*this));
}

const char *PrintModifier::getDefaultName() const {
	return "Print Modifier";
}

MTIPlugIn::MTIPlugIn()
	: _shanghaiModifierFactory(this), _printModifierFactory(this) {
}

void MTIPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("Shanghai", &_shanghaiModifierFactory);
	registrar->registerPlugInModifier("Print", &_printModifierFactory);
}


} // namespace MTI

namespace PlugIns {

Common::SharedPtr<PlugIn> createMTI() {
	return Common::SharedPtr<PlugIn>(new MTI::MTIPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
