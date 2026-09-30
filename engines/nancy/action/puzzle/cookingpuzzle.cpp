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

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/font.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/textbox.h"
#include "engines/nancy/action/puzzle/cookingpuzzle.h"

namespace Nancy {
namespace Action {

// Cooking progresses in ticks of 20ms
static const uint32 kMsPerTick = 20;
static const float kTicksPerSecond = 50.0f;

// The stacked copies of an ingredient are drawn slightly scattered, using the random
// sequence of the game's C runtime seeded with the slot's index
static int16 nextJitter(uint32 &seed) {
	seed = seed * 214013 + 2531011;
	int value = (seed >> 16) & 0x7FFF;
	return (int16)((float)value / 32767.0f * 8.0f - 4.0f);
}

CookingPuzzle::~CookingPuzzle() {
	if (_heldFood && !_heldFood->isBin) {
		deleteFood(_heldFood);
	}

	for (Processor &processor : _processors) {
		for (Slot &slot : processor.slots) {
			deleteFood(slot.food);
		}
	}

	for (Plate &plate : _plates) {
		for (Food *food : plate.foods) {
			deleteFood(food);
		}
	}

	for (Food *bin : _bins) {
		deleteFood(bin);
	}
}

// Unlike RandomSoundBlock::readData(), the channel, loops and volume are there even
// when no sounds are named
void CookingPuzzle::readSoundBank(Common::SeekableReadStream &stream, RandomSoundBlock &sound) {
	int16 numNames = stream.readSint16LE();
	sound.names.resize(MAX<int16>(numNames, 0));
	for (Common::String &name : sound.names) {
		readFilename(stream, name);
	}

	sound.channel = stream.readSint16LE();
	sound.numLoops = stream.readSint32LE();
	sound.volume = stream.readSint16LE();
}

void CookingPuzzle::readFoodType(Common::SeekableReadStream &stream, FoodType &food) {
	readFilename(stream, food.name);

	Common::String doneness;
	readFilename(stream, doneness);
	if (doneness == "WellDone") {
		food.initialDoneness = kWellDone;
	} else if (doneness == "OverDone") {
		food.initialDoneness = kOverDone;
	}

	food.minCookTime = stream.readFloatLE();
	food.maxCookTime = stream.readFloatLE();
	food.hoverCursor = stream.readUint16LE();
	food.cursorHotspot.x = stream.readSint32LE();
	food.cursorHotspot.y = stream.readSint32LE();
	readRect(stream, food.binHotspot);

	for (uint i = 0; i < kNumDoneness; ++i) {
		readRect(stream, food.cursorSrcs[i]);
		readRect(stream, food.processorSrcs[i]);
		readRect(stream, food.plateSrcs[i]);
	}

	readSoundBank(stream, food.cookSound);
}

void CookingPuzzle::readOrder(Common::SeekableReadStream &stream, Order &order) {
	readFilename(stream, order.food);
	order.minCount = stream.readSint32LE();
	order.maxCount = stream.readSint32LE();
	readRect(stream, order.countDest);

	int16 numFlags = stream.readSint16LE();
	order.flags.resize(MAX<int16>(numFlags, 0));
	for (IngredientFlag &flag : order.flags) {
		readFilename(stream, flag.ingredient);
		flag.flag.label = stream.readSint16LE();
		flag.flag.flag = stream.readByte();
	}

	int16 numRequired = stream.readSint16LE();
	order.required.resize(MAX<int16>(numRequired, 0));
	for (Order &required : order.required) {
		readOrder(stream, required);
	}

	int16 numOptional = stream.readSint16LE();
	order.optional.resize(MAX<int16>(numOptional, 0));
	for (Order &optional : order.optional) {
		readOrder(stream, optional);
	}
}

void CookingPuzzle::readData(Common::SeekableReadStream &stream) {
	stream.skip(33); // Data version

	readFilename(stream, _imageName);

	// Foods that come out of the bins, then foods that come out of the processors
	for (uint list = 0; list < 2; ++list) {
		int16 numFoods = stream.readSint16LE();
		for (int16 i = 0; i < numFoods; ++i) {
			_foodTypes.push_back(FoodType());
			readFoodType(stream, _foodTypes.back());
		}
	}

	int16 numProcessors = stream.readSint16LE();
	_processors.resize(MAX<int16>(numProcessors, 0));
	for (Processor &processor : _processors) {
		int16 numRecipes = stream.readSint16LE();
		processor.recipes.resize(MAX<int16>(numRecipes, 0));
		for (Recipe &recipe : processor.recipes) {
			readFilename(stream, recipe.input);
			readFilename(stream, recipe.output);

			int16 numIngredients = stream.readSint16LE();
			recipe.ingredients.resize(MAX<int16>(numIngredients, 0));
			for (Common::String &ingredient : recipe.ingredients) {
				readFilename(stream, ingredient);
			}
		}

		processor.cooks = stream.readByte() == 1;
		processor.takeCursor = stream.readUint16LE();

		int16 numSlots = stream.readSint16LE();
		processor.slots.resize(MAX<int16>(numSlots, 0));
		for (Slot &slot : processor.slots) {
			readRect(stream, slot.dropHotspot);
			readRect(stream, slot.addHotspot);
			readRect(stream, slot.takeHotspot);
		}
	}

	readFilename(stream, _plateImageName);
	readRect(stream, _plateSrc);
	readRect(stream, _plateDest);
	readRect(stream, _plateBarArea);
	readRect(stream, _serveHotspot);
	readRect(stream, _ticketIconSrc);

	// Every plate's ticket takes one of these at random
	Common::Array<Common::Rect> ticketHotspots;
	readRectArray(stream, ticketHotspots, stream.readUint16LE());

	int16 numLayouts = stream.readSint16LE();
	_plateLayouts.resize(MAX<int16>(numLayouts, 0));
	for (PlateLayout &layout : _plateLayouts) {
		readFilename(stream, layout.food);
		readRectArray(stream, layout.dests, stream.readUint16LE());
	}

	_plateFoodCursor = stream.readUint16LE();
	_plateCloseCursor = stream.readUint16LE();
	_serveCursor = stream.readUint16LE();
	_ticketCursor = stream.readUint16LE();
	readRect(stream, _ticketDest);
	readFilename(stream, _countFontName);
	_countColor = stream.readUint32BE();
	_captionTime = stream.readFloatLE();

	readSoundBank(stream, _orderRightSound);
	readSoundBank(stream, _orderWrongSound);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	readSoundBank(stream, _allServedSound);

	int16 numPlates = stream.readSint16LE();
	if (numPlates > (int16)ticketHotspots.size()) {
		error("CookingPuzzle: more plates than ticket hotspots");
	}

	_plates.resize(MAX<int16>(numPlates, 0));
	for (Plate &plate : _plates) {
		uint ticket = g_nancy->_randomSource->getRandomNumber(ticketHotspots.size() - 1);
		plate.ticketHotspot = ticketHotspots[ticket];
		ticketHotspots.remove_at(ticket);

		readRect(stream, plate.ticketSrc);
		plate.acceptsBadlyCooked = stream.readByte() == 1;
		plate.badlyCookedFlag.label = stream.readSint16LE();
		plate.badlyCookedFlag.flag = stream.readByte();

		int16 numOrders = stream.readSint16LE();
		plate.orders.resize(MAX<int16>(numOrders, 0));
		for (Order &order : plate.orders) {
			readOrder(stream, order);
			rollOrderCounts(order);
		}
	}

	int16 numPages = stream.readSint16LE();
	_manualPageNames.resize(MAX<int16>(numPages, 0));
	for (Common::Path &page : _manualPageNames) {
		readFilename(stream, page);
	}

	readRect(stream, _manualPrevHotspot);
	readRect(stream, _manualNextHotspot);
	readRect(stream, _manualOpenHotspot);
	readRect(stream, _manualCloseHotspot);
	readRect(stream, _manualSrc);
	readRect(stream, _manualDest);
	_manualPrevCursor = stream.readUint16LE();
	_manualNextCursor = stream.readUint16LE();
	_manualOpenCursor = stream.readUint16LE();
	_manualCloseCursor = stream.readUint16LE();
	readSoundBank(stream, _pageFlipSound);
	readSoundBank(stream, _manualOpenSound);
	readSoundBank(stream, _manualCloseSound);

	readRect(stream, _garbageHotspot);
	readSoundBank(stream, _garbageSound);
	_heldCursor = stream.readUint16LE();

	_fireScene._sceneChange.sceneID = stream.readUint16LE();
	_fireScene._sceneChange.frameID = stream.readUint16LE();
	_fireScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_fireScene._flag.label = stream.readSint16LE();
	_fireScene._flag.flag = stream.readByte();
	readSoundBank(stream, _fireSound);

	readExitHotspot(stream);
}

void CookingPuzzle::rollOrderCounts(Order &order) {
	int32 range = order.maxCount - order.minCount;
	order.count = order.minCount + (range > 0 ? (int32)g_nancy->_randomSource->getRandomNumber(range) : 0);

	for (Order &required : order.required) {
		rollOrderCounts(required);
	}

	for (Order &optional : order.optional) {
		rollOrderCounts(optional);
	}
}

void CookingPuzzle::init() {
	initViewportSurface();
	loadImage();

	g_nancy->_resource->loadImage(_plateImageName, _plateImage);
	_plateImage.setTransparentColor(_drawSurface.getTransparentColor());

	_manualPages.resize(_manualPageNames.size());
	for (uint i = 0; i < _manualPageNames.size(); ++i) {
		g_nancy->_resource->loadImage(_manualPageNames[i], _manualPages[i]);
		_manualPages[i].setTransparentColor(_drawSurface.getTransparentColor());
	}

	// Every food type has a bin that never runs out; most of them just can't be clicked
	for (const FoodType &type : _foodTypes) {
		Food *bin = new Food();
		bin->type = &type;
		bin->isBin = true;
		bin->doneness = type.initialDoneness;
		_bins.push_back(bin);
	}

	NancySceneState.setNoHeldItem();
	_lastTickTime = g_nancy->getTotalPlayTime();
	redraw();
}

const CookingPuzzle::FoodType *CookingPuzzle::findFoodType(const Common::String &name) const {
	for (const FoodType &type : _foodTypes) {
		if (type.name == name) {
			return &type;
		}
	}

	return nullptr;
}

const CookingPuzzle::Recipe *CookingPuzzle::findRecipe(const Processor &processor, const Common::String &input) const {
	for (const Recipe &recipe : processor.recipes) {
		if (recipe.input == input) {
			return &recipe;
		}
	}

	return nullptr;
}

const CookingPuzzle::Order *CookingPuzzle::findOrder(const Common::Array<Order> &orders, const Common::String &food) {
	for (const Order &order : orders) {
		if (order.food == food) {
			return &order;
		}
	}

	return nullptr;
}

// Makes a food of the given type out of another one, which passes on its doneness,
// cooking progress and ingredients
CookingPuzzle::Food *CookingPuzzle::createFood(const Common::String &typeName, const Food *source) {
	const FoodType *type = findFoodType(typeName);
	if (!type) {
		error("CookingPuzzle: could not find food %s", typeName.c_str());
	}

	Food *food = new Food();
	food->type = type;
	food->doneness = source->doneness;

	float step = type->minCookTime + (float)g_nancy->_randomSource->getRandomNumber(32767) / 32767.0f * (type->maxCookTime - type->minCookTime);
	food->ticksPerStep = (int32)(step * kTicksPerSecond);

	if (!source->isBin) {
		if (source->ticksPerStep > 0) {
			float progress = (float)source->cookTicks / (float)source->ticksPerStep;
			food->cookTicks = (int32)(progress * (float)food->ticksPerStep);
		}

		for (const Food *ingredient : source->ingredients) {
			Food *copy = createFood(ingredient->type->name, ingredient);
			copy->visibleInProcessor = false;
			insertSorted(food->ingredients, copy);
		}
	}

	return food;
}

void CookingPuzzle::deleteFood(Food *food) {
	if (!food) {
		return;
	}

	for (Food *ingredient : food->ingredients) {
		deleteFood(ingredient);
	}

	delete food;
}

// Foods with the same name keep the order in which they were added
void CookingPuzzle::insertSorted(Common::Array<Food *> &list, Food *food) {
	uint i = 0;
	while (i < list.size() && list[i]->type->name.compareTo(food->type->name) <= 0) {
		++i;
	}

	list.insert_at(i, food);
}

void CookingPuzzle::holdFood(Food *food) {
	_heldFood = food;
}

// Discarding a food that came straight out of a bin has no effect, as bins never run out
void CookingPuzzle::releaseHeldFood(bool discard) {
	if (discard && _heldFood && !_heldFood->isBin) {
		deleteFood(_heldFood);
	}

	_heldFood = nullptr;
}

bool CookingPuzzle::clickManual(const Common::Point &pos) {
	if (_manualCloseHotspot.contains(pos)) {
		playSoundBlock(_manualCloseSound);
		_manualOpen = false;
	}

	if (_manualPages.empty()) {
		return true;
	}

	if (_manualPrevHotspot.contains(pos)) {
		playSoundBlock(_pageFlipSound);
		_manualPage = (_manualPage + _manualPages.size() - 1) % _manualPages.size();
	}

	if (_manualNextHotspot.contains(pos)) {
		playSoundBlock(_pageFlipSound);
		_manualPage = (_manualPage + 1) % _manualPages.size();
	}

	return true;
}

// Puts the held food on a plate. Food that isn't part of the plate's order,
// or that the plate already has enough of, gets thrown away.
bool CookingPuzzle::putOnPlate(Plate &plate) {
	for (const Order &order : plate.orders) {
		if (order.food != _heldFood->type->name || order.count == 0) {
			continue;
		}

		int32 numOnPlate = 0;
		for (const Food *food : plate.foods) {
			if (food->type->name == order.food) {
				++numOnPlate;
			}
		}

		if (numOnPlate == order.maxCount) {
			break;
		}

		Food *food = _heldFood->isBin ? createFood(_heldFood->type->name, _heldFood) : _heldFood;
		insertSorted(plate.foods, food);
		releaseHeldFood(false);
		return true;
	}

	releaseHeldFood(true);
	return false;
}

bool CookingPuzzle::foodMatches(const Food *food, const Order &order, Common::Array<FlagDescription> &flags,
		bool checkDoneness, bool &allWellDone) const {
	allWellDone = allWellDone && food->doneness == kWellDone;
	if (checkDoneness && food->doneness != kWellDone) {
		return false;
	}

	Common::HashMap<Common::String, int32> requiredCounts;
	Common::HashMap<Common::String, int32> optionalCounts;

	for (const Food *ingredient : food->ingredients) {
		const Common::String &name = ingredient->type->name;

		for (const IngredientFlag &flag : order.flags) {
			if (flag.ingredient == name) {
				flags.push_back(flag.flag);
			}
		}

		const Order *required = findOrder(order.required, name);
		if (required) {
			Common::Array<FlagDescription> ingredientFlags;
			bool ingredientWellDone = allWellDone;
			if (foodMatches(ingredient, *required, ingredientFlags, checkDoneness, ingredientWellDone)) {
				flags.push_back(ingredientFlags);
				allWellDone = ingredientWellDone;
				++requiredCounts.getOrCreateVal(name);
				continue;
			}
		}

		const Order *optional = findOrder(order.optional, name);
		if (optional) {
			bool ingredientWellDone = allWellDone;
			if (foodMatches(ingredient, *optional, flags, checkDoneness, ingredientWellDone)) {
				allWellDone = ingredientWellDone;
				++optionalCounts.getOrCreateVal(name);
				continue;
			}
		}

		return false;
	}

	for (const Order &required : order.required) {
		if (requiredCounts.getValOrDefault(required.food, 0) != required.count) {
			return false;
		}
	}

	// Optional ingredients only have to stay within their range when they're there at all
	for (const Order &optional : order.optional) {
		if (optionalCounts.contains(optional.food)) {
			int32 count = optionalCounts[optional.food];
			if (count < optional.minCount || count > optional.maxCount) {
				return false;
			}
		}
	}

	return true;
}

// Checks the plate against its order. Whatever the outcome, the plate is cleared.
bool CookingPuzzle::servePlate(Plate &plate) {
	bool success = true;
	bool allWellDone = true;
	Common::Array<FlagDescription> flags;

	for (const Order &order : plate.orders) {
		int32 numFound = 0;
		for (const Food *food : plate.foods) {
			if (food->type->name != order.food) {
				continue;
			}

			++numFound;
			if (!foodMatches(food, order, flags, !plate.acceptsBadlyCooked, allWellDone)) {
				success = false;
			}
		}

		if (numFound != order.count) {
			success = false;
		}

		if (!success) {
			break;
		}
	}

	for (Food *food : plate.foods) {
		deleteFood(food);
	}

	plate.foods.clear();

	if (success) {
		if (!allWellDone && plate.acceptsBadlyCooked) {
			NancySceneState.setEventFlag(plate.badlyCookedFlag);
		}

		for (const FlagDescription &flag : flags) {
			NancySceneState.setEventFlag(flag);
		}
	}

	return success;
}

bool CookingPuzzle::clickPlates(const Common::Point &pos) {
	if (_openPlate == -1) {
		for (uint i = 0; i < _plates.size(); ++i) {
			if (!_plates[i].ticketHotspot.contains(pos)) {
				continue;
			}

			if (!_heldFood || !putOnPlate(_plates[i])) {
				_openPlate = i;
			}

			return true;
		}

		return false;
	}

	Plate &plate = _plates[_openPlate];

	if (!_plateBarArea.contains(pos) && _plateDest.contains(pos)) {
		if (_serveHotspot.contains(pos)) {
			if (servePlate(plate)) {
				_plates.remove_at(_openPlate);

				if (_plates.empty()) {
					startEnding(_solveScene, _allServedSound);
				} else {
					_rightSoundPending = true;
					playSoundBlock(_orderRightSound);
				}
			} else {
				_wrongSoundPending = true;
				playSoundBlock(_orderWrongSound);
			}

			_openPlate = -1;
			return true;
		}

		for (const PlateLayout &layout : _plateLayouts) {
			for (uint i = 0; i < plate.foods.size(); ++i) {
				if (plate.foods[i]->type->name != layout.food) {
					continue;
				}

				for (const Common::Rect &dest : layout.dests) {
					if (dest.contains(pos)) {
						holdFood(plate.foods[i]);
						plate.foods.remove_at(i);
						_openPlate = -1;
						return true;
					}
				}

				break;
			}
		}

		return true;
	}

	_openPlate = -1;
	return true;
}

bool CookingPuzzle::clickProcessors(const Common::Point &pos) {
	for (Processor &processor : _processors) {
		for (Slot &slot : processor.slots) {
			if (slot.takeHotspot.contains(pos) && !_heldFood && slot.food) {
				holdFood(slot.food);
				slot.food = nullptr;
				slot.recipeInput.clear();
				return true;
			}

			if (slot.dropHotspot.contains(pos) && _heldFood && !slot.food) {
				const Recipe *recipe = findRecipe(processor, _heldFood->type->name);
				if (recipe) {
					slot.food = createFood(recipe->output, _heldFood);
					slot.recipeInput = _heldFood->type->name;
					if (!_heldFood->isBin) {
						releaseHeldFood(true);
					}

					return true;
				}
			}

			if (slot.addHotspot.contains(pos) && _heldFood && slot.food) {
				const Recipe *recipe = findRecipe(processor, slot.recipeInput);
				if (recipe) {
					for (const Common::String &ingredient : recipe->ingredients) {
						if (ingredient != _heldFood->type->name) {
							continue;
						}

						if (_heldFood->isBin) {
							insertSorted(slot.food->ingredients, createFood(ingredient, _heldFood));
						} else {
							insertSorted(slot.food->ingredients, _heldFood);
							releaseHeldFood(false);
						}

						return true;
					}
				}
			}
		}
	}

	return false;
}

// Clicking the bin of the held food puts it back
bool CookingPuzzle::clickBins(const Common::Point &pos) {
	for (Food *bin : _bins) {
		if (!bin->type->binHotspot.contains(pos)) {
			continue;
		}

		bool isSameBin = _heldFood == bin;
		if (_heldFood) {
			releaseHeldFood(true);
		}

		if (!isSameBin) {
			holdFood(bin);
		}

		return true;
	}

	return false;
}

bool CookingPuzzle::clickGarbage(const Common::Point &pos) {
	if (!_garbageHotspot.contains(pos) || !_heldFood) {
		return false;
	}

	playSoundBlock(_garbageSound);
	releaseHeldFood(true);
	return true;
}

bool CookingPuzzle::updateCursor(const Common::Point &pos) {
	if (_heldFood) {
		setDataCursor(_heldCursor);
		return true;
	}

	if (_manualOpen) {
		if (_manualCloseHotspot.contains(pos)) {
			setDataCursor(_manualCloseCursor);
			return true;
		}

		if (!_manualPages.empty()) {
			if (_manualPrevHotspot.contains(pos)) {
				setDataCursor(_manualPrevCursor);
				return true;
			}

			if (_manualNextHotspot.contains(pos)) {
				setDataCursor(_manualNextCursor);
				return true;
			}
		}

		return false;
	}

	if (_openPlate != -1) {
		if (_plateBarArea.contains(pos) || !_plateDest.contains(pos)) {
			setDataCursor(_plateCloseCursor);
			return true;
		}

		if (_serveHotspot.contains(pos)) {
			setDataCursor(_serveCursor);
			return true;
		}

		const Plate &plate = _plates[_openPlate];
		for (const PlateLayout &layout : _plateLayouts) {
			bool onPlate = false;
			for (const Food *food : plate.foods) {
				if (food->type->name == layout.food) {
					onPlate = true;
					break;
				}
			}

			if (!onPlate) {
				continue;
			}

			for (const Common::Rect &dest : layout.dests) {
				if (dest.contains(pos)) {
					setDataCursor(_plateFoodCursor);
					return true;
				}
			}
		}

		return false;
	}

	for (const Plate &plate : _plates) {
		if (plate.ticketHotspot.contains(pos)) {
			setDataCursor(_ticketCursor);
			return true;
		}
	}

	if (_manualOpenHotspot.contains(pos)) {
		setDataCursor(_manualOpenCursor);
		return true;
	}

	for (const Processor &processor : _processors) {
		for (const Slot &slot : processor.slots) {
			if (slot.takeHotspot.contains(pos) && slot.food) {
				setDataCursor(processor.takeCursor);
				return true;
			}
		}
	}

	for (const Food *bin : _bins) {
		if (bin->type->binHotspot.contains(pos)) {
			setDataCursor(bin->type->hoverCursor);
			return true;
		}
	}

	return false;
}

// Advances the food by a doneness step every time a step's worth of cooking has
// passed. A food that is already overdone sets off the fire alarm instead.
bool CookingPuzzle::cook(Food *food, int32 ticks) {
	int32 oldTicks = food->cookTicks;
	int32 newTicks = oldTicks + ticks;
	food->cookTicks = newTicks;

	if (food->ticksPerStep <= 0) {
		return false;
	}

	if (newTicks / food->ticksPerStep == oldTicks / food->ticksPerStep && oldTicks != 0) {
		return false;
	}

	playSoundBlock(food->type->cookSound);
	if (oldTicks <= 0) {
		return false;
	}

	if (food->doneness == kOverDone) {
		return true;
	}

	++food->doneness;
	return false;
}

void CookingPuzzle::update(int32 ticks) {
	for (Processor &processor : _processors) {
		if (!processor.cooks) {
			continue;
		}

		for (Slot &slot : processor.slots) {
			if (slot.food && cook(slot.food, ticks)) {
				startEnding(_fireScene, _fireSound);
				return;
			}
		}
	}

	// The served order's caption goes away a little while after its sound ends
	if (_captionTicks > 0) {
		_captionTicks = MAX<int32>(_captionTicks - ticks, 0);
		if (_captionTicks == 0) {
			NancySceneState.getTextbox().clear();
		}
	} else {
		if (_rightSoundPending && !isSoundBlockPlaying(_orderRightSound)) {
			_rightSoundPending = false;
			_captionTicks = (int32)(_captionTime * kTicksPerSecond);
		}

		if (_wrongSoundPending && !isSoundBlockPlaying(_orderWrongSound)) {
			_wrongSoundPending = false;
			_captionTicks = (int32)(_captionTime * kTicksPerSecond);
		}
	}
}

void CookingPuzzle::startEnding(const SceneChangeWithFlag &scene, const RandomSoundBlock &sound) {
	_isEnding = true;
	_endScene = scene;
	_endSound = playSoundBlock(sound);
}

void CookingPuzzle::drawFood(const Food *food, const Common::Rect &dest, bool jitter, uint32 &randSeed) {
	if (food->visibleInProcessor) {
		Common::Point offset;
		if (jitter) {
			offset.y = nextJitter(randSeed);
			offset.x = nextJitter(randSeed);
		}

		const Common::Rect &src = food->type->processorSrcs[food->doneness];
		if (!src.isEmpty()) {
			_drawSurface.blitFrom(_image, src, Common::Point(dest.left + offset.x, dest.top + offset.y));
		}
	}

	const Food *previous = nullptr;
	for (const Food *ingredient : food->ingredients) {
		bool sameAsPrevious = previous && previous->type->name == ingredient->type->name;
		drawFood(ingredient, dest, sameAsPrevious, randSeed);
		previous = ingredient;
	}
}

void CookingPuzzle::drawOrderCounts(const Order &order, const Graphics::Font *font, uint32 color) {
	if (order.count <= 0) {
		return;
	}

	font->drawString(_drawSurface.surfacePtr(), Common::String::format("%d", order.count),
		order.countDest.left, order.countDest.top, order.countDest.width(), color);

	for (const Order &required : order.required) {
		drawOrderCounts(required, font, color);
	}
}

void CookingPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (const Processor &processor : _processors) {
		for (uint i = 0; i < processor.slots.size(); ++i) {
			const Slot &slot = processor.slots[i];
			if (slot.food) {
				uint32 randSeed = i;
				drawFood(slot.food, slot.addHotspot, false, randSeed);
			}
		}
	}

	if (_openPlate == -1) {
		for (const Plate &plate : _plates) {
			_drawSurface.blitFrom(_plateImage, _ticketIconSrc, plate.ticketHotspot.origin());
		}
	} else {
		const Plate &plate = _plates[_openPlate];
		_drawSurface.blitFrom(_plateImage, _plateSrc, _plateDest.origin());

		for (const PlateLayout &layout : _plateLayouts) {
			uint slot = 0;
			for (const Food *food : plate.foods) {
				if (food->type->name != layout.food) {
					continue;
				}

				if (slot >= layout.dests.size()) {
					error("CookingPuzzle: not enough plate positions for food %s", food->type->name.c_str());
				}

				const Common::Rect &src = food->type->plateSrcs[food->doneness];
				if (!src.isEmpty()) {
					_drawSurface.blitFrom(_image, src, layout.dests[slot].origin());
				}

				++slot;
			}
		}

		const Font *countFont = g_nancy->_graphics->getFont(_countFontName);
		if (countFont && countFont->getSystemFont()) {
			uint32 color = argbToColor(_drawSurface.format, _countColor);
			for (const Order &order : plate.orders) {
				drawOrderCounts(order, countFont->getSystemFont(), color);
			}
		}

		_drawSurface.blitFrom(_plateImage, plate.ticketSrc, _ticketDest.origin());
	}

	if (_manualOpen && _manualPage < _manualPages.size()) {
		_drawSurface.blitFrom(_manualPages[_manualPage], _manualSrc, _manualDest.origin());
	}

	if (_heldFood) {
		const Common::Rect &src = _heldFood->type->cursorSrcs[_heldFood->doneness];
		if (!src.isEmpty()) {
			_drawSurface.blitFrom(_image, src, getHeldPiecePos(_mousePos, src, _heldFood->type->cursorHotspot));
		}
	}

	_needsRedraw = true;
}

void CookingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_isEnding) {
			if (_endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound)) {
				_state = kActionTrigger;
			}

			break;
		}

		uint32 now = g_nancy->getTotalPlayTime();
		int32 ticks = (now - _lastTickTime) / kMsPerTick;
		_lastTickTime += ticks * kMsPerTick;

		if (ticks > 0) {
			update(ticks);
		}

		redraw();
		break;
	}
	case kActionTrigger:
		if (_heldFood) {
			releaseHeldFood(true);
		}

		if (_exitRequested) {
			_exitScene.execute();
		} else {
			_endScene.execute();
		}

		finishExecution();
		break;
	}
}

void CookingPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isEnding || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	_mousePos = pos;

	// Right clicking puts back food taken out of a bin
	if ((input.input & NancyInput::kRightMouseButtonUp) && _heldFood && _heldFood->isBin) {
		releaseHeldFood(false);
	}

	bool cursorSet = updateCursor(pos);

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		bool handled = false;
		if (_manualOpen) {
			handled = clickManual(pos);
		}

		if (!handled) {
			handled = clickPlates(pos);
		}

		if (!handled && _manualOpenHotspot.contains(pos)) {
			playSoundBlock(_manualOpenSound);
			_manualOpen = true;
			handled = true;
		}

		if (!handled) {
			handled = clickProcessors(pos) || clickBins(pos) || clickGarbage(pos);
		}

		if (handled || _isEnding) {
			input.eatMouseInput();
			return;
		}
	}

	if (!cursorSet && hoverExitHotspot(input)) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
		}
	}

	input.eatMouseInput();
}

} // End of namespace Action
} // End of namespace Nancy
