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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/serializer.h"
#include "common/stream.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/speech.h"

#include "asylum/views/scene.h"

namespace Asylum {

// inventory ring is centered at (-20, 20) and has radius 80
static const int16 inventoryRingPoints[36][2] = {
	{ -20,  100},
	{ -20,  100}, { -20,  -60},
	{ -20,  100}, { -89,  -20}, {  49,  -20},
	{ -20,  100}, {-100,   20}, { -20,  -60}, {  60,   20},
	{ -20,  100}, { -96,   45}, { -67,  -45}, {  27,  -45}, {  56,   45},
	{ -20,  100}, { -89,   60}, { -89,  -20}, { -20,  -60}, {  49,  -20}, {  49,   60},
	{ -20,  100}, { -82,   70}, { -98,    3}, { -56,  -51}, {  13,  -53}, {  57,   -1}, {  45,   67},
	{ -20,  100}, { -77,   77}, {-100,   20}, { -77,  -37}, { -20,  -60}, {  37,  -37}, {  60,   20}, {  37,   77}
};

// first 8 triangular numbers
static const uint inventoryRingOffsets[8] = {0, 1, 3, 6, 10, 15, 21, 28};

static const int inventoryDescriptionIndices[12][11] = {
	{ 61,  69,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{107, 134, 104, 113,  -1, 112, 117, 109, 108, 111, 106},
	{170, 182, 181, 172, 171, 169,   0,   0,   0,   0,   0},
	{ 61,  -1,  66,  67,  68,  69,  70,  78,  77,   0,   0},
	{197,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{ 59,  81,  60,  84,  88,  54,  74, 139,  97, 121,   0},
	{239, 234, 249, 259, 260, 272, 237, 262,   0,   0,   0},
	{ 58,  59,  60, 111,  75,  76,  77,  78,   0,   0,   0},
	{284, 285, 286, 329, 330, 331, 332, 322,   0, 465,   0},
	{ -1,  -1,  -1,  -1,  -1,   0,   0,   0,   0,   0,   0},
	{ 69,  70,  78,   0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}
};

Inventory::Inventory(AsylumEngine *vm, int32 &multiple) : _vm(vm), _selectedItem(0), _multiple(multiple) {
	memset(_items, 0, sizeof(_items));
}

void Inventory::load(Common::SeekableReadStream *stream) {
	for (int i = 0; i < 8; i++)
		_items[i] = stream->readUint32LE();

	_selectedItem = stream->readUint32LE();
}

void Inventory::saveLoadWithSerializer(Common::Serializer &s) {
	for (int i = 0; i < 8; i++)
		s.syncAsUint32LE(_items[i]);

	s.syncAsUint32LE(_selectedItem);
}

uint Inventory::find(uint item) const {
	int i;

	for (i = 0; i < 8; i++)
		if (_items[i] == item)
			break;

	return i;
}

void Inventory::add(uint item, uint multipleIncr) {
	if (item > 16)
		return;

	if (!contains(item, 0)) {
		int i = find();

		if (i == 8)
			return;

		_items[i] = item;
	}

	_multiple += multipleIncr;

	getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 0));
}

void Inventory::remove(uint item, uint multipleDecr) {
	if (item > 16)
		return;

	_multiple = (_multiple >= (int32)multipleDecr) ? _multiple - multipleDecr : 0;

	if (!multipleDecr || !_multiple) {
		int i = find(item);

		if (i == 8)
			return;

		if (i < 7)
			memmove(&_items[i], &_items[i + 1], (7 - i) * 4);

		_items[7] = 0;
	}
}

bool Inventory::contains(uint item, uint multiple) const {
	if (item > 16)
		return false;

	int i = find(item);

	if (i == 8)
		return false;

	if (multiple)
		return (_multiple >= (int32)multiple);

	return true;
}

void Inventory::copyFrom(Inventory &inventory) {
	for (int i = 0; i < 8; i++)
		_items[i] = inventory[i];
}

Common::Point Inventory::getInventoryRingPoint(AsylumEngine *vm, uint nPoints, uint index) {
	const int16 (*pointPtr)[2];

	if (vm->scene()->worldstats()->chapter == kChapter11)
		pointPtr = &inventoryRingPoints[inventoryRingOffsets[7] + index + 3];
	else
		pointPtr = &inventoryRingPoints[inventoryRingOffsets[nPoints - 1] + index];

	return Common::Point((*pointPtr)[0], (*pointPtr)[1]);
}

void Inventory::describe(AsylumEngine *vm, uint itemIndex) {
	uint index;
	ResourceId resourceId;
	ChapterIndex chapter = vm->scene()->worldstats()->chapter;

	if (chapter == kChapterNone || itemIndex > 10)
		return;

	if (chapter == kChapter2 && itemIndex == 4)
		index = vm->isGameFlagSet(kGameFlag186) ? 362 : 110;
	else if (chapter == kChapter4 && itemIndex == 1)
		index = vm->scene()->getActor()->getNumberValue01() != 1 ? 65 : 64;
	else if (chapter == kChapter10 && itemIndex < 5)
		index = 91 + itemIndex;
	else
		index = inventoryDescriptionIndices[chapter - 1][itemIndex];

	switch (vm->scene()->worldstats()->actorType) {
	default:
		resourceId = (ResourceId)index;
		break;

	case kActorMax:
		resourceId = MAKE_RESOURCE(kResourcePackSpeech, index < 259 ? index : index - 9);
		break;

	case kActorSarah:
		resourceId = MAKE_RESOURCE(kResourcePackSharedSound, index + 1927);
		break;

	case kActorCyclops:
		resourceId = MAKE_RESOURCE(kResourcePackSharedSound, index + 2084);
		break;

	case kActorAztec:
		resourceId = MAKE_RESOURCE(kResourcePackSharedSound, index + 2234);
		break;
	}

	if (vm->speech()->getSoundResourceId() != resourceId || !vm->sound()->isPlaying(resourceId))
		vm->speech()->playPlayer(index);
}

} // End of namespace Asylum
