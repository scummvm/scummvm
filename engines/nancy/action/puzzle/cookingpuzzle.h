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

#ifndef NANCY_ACTION_COOKINGPUZZLE_H
#define NANCY_ACTION_COOKINGPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 diner cooking puzzle (AR 194). Ingredients are taken from bins and put into
// processors (microwave, grill, burger buns), which turn them into other foods and cook
// them over time. Finished foods are put on the plates of the customers' order tickets
// and served. Leaving food to cook for too long sets off the fire alarm.
class CookingPuzzle : public PuzzleRecord {
public:
	CookingPuzzle() : PuzzleRecord(7) {}
	virtual ~CookingPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "CookingPuzzle"; }

	enum Doneness : byte {
		kUnderDone	= 0,
		kWellDone	= 1,
		kOverDone	= 2
	};

	static const uint kNumDoneness = 3;

	struct FoodType {
		Common::String name;
		byte initialDoneness = kUnderDone;
		// Seconds per doneness step, picked at random within the range
		float minCookTime = 0;
		float maxCookTime = 0;
		uint16 hoverCursor = 0;
		Common::Point cursorHotspot;	// moves the held food away from the cursor's center
		Common::Rect binHotspot;	// empty for foods that only come out of a processor
		Common::Rect cursorSrcs[kNumDoneness];
		Common::Rect processorSrcs[kNumDoneness];
		Common::Rect plateSrcs[kNumDoneness];
		RandomSoundBlock cookSound;
	};

	// A food in play. The ones standing in for the bins are never used up.
	struct Food {
		const FoodType *type = nullptr;
		bool isBin = false;
		byte doneness = kUnderDone;
		int32 cookTicks = 0;
		int32 ticksPerStep = 0;
		bool visibleInProcessor = true;
		Common::Array<Food *> ingredients;	// sorted by name
	};

	struct Recipe {
		Common::String input;
		Common::String output;
		Common::Array<Common::String> ingredients;
	};

	struct Slot {
		Common::Rect dropHotspot;
		Common::Rect addHotspot;	// where ingredients go, and where the food is drawn
		Common::Rect takeHotspot;
		Food *food = nullptr;
		Common::String recipeInput;
	};

	struct Processor {
		Common::Array<Recipe> recipes;
		bool cooks = false;
		uint16 takeCursor = 0;
		Common::Array<Slot> slots;
	};

	struct IngredientFlag {
		Common::String ingredient;
		FlagDescription flag;
	};

	struct Order {
		Common::String food;
		int32 minCount = 0;
		int32 maxCount = 0;
		int32 count = 0;	// rolled between the two
		Common::Rect countDest;
		Common::Array<IngredientFlag> flags;
		Common::Array<Order> required;
		Common::Array<Order> optional;
	};

	struct Plate {
		Common::Rect ticketHotspot;
		Common::Rect ticketSrc;
		bool acceptsBadlyCooked = false;
		FlagDescription badlyCookedFlag;
		Common::Array<Order> orders;
		Common::Array<Food *> foods;	// sorted by name
	};

	struct PlateLayout {
		Common::String food;
		Common::Array<Common::Rect> dests;
	};

	static void readSoundBank(Common::SeekableReadStream &stream, RandomSoundBlock &sound);
	static void readFoodType(Common::SeekableReadStream &stream, FoodType &food);
	static void readOrder(Common::SeekableReadStream &stream, Order &order);
	static void rollOrderCounts(Order &order);

	const FoodType *findFoodType(const Common::String &name) const;
	const Recipe *findRecipe(const Processor &processor, const Common::String &input) const;
	static const Order *findOrder(const Common::Array<Order> &orders, const Common::String &food);

	Food *createFood(const Common::String &typeName, const Food *source);
	static void deleteFood(Food *food);
	static void insertSorted(Common::Array<Food *> &list, Food *food);

	void holdFood(Food *food);
	void releaseHeldFood(bool discard);

	bool clickManual(const Common::Point &pos);
	bool clickPlates(const Common::Point &pos);
	bool clickProcessors(const Common::Point &pos);
	bool clickBins(const Common::Point &pos);
	bool clickGarbage(const Common::Point &pos);
	bool putOnPlate(Plate &plate);
	bool servePlate(Plate &plate);

	// Cheat: serves every order that is left, the way plates with just what they need
	// would have been served
	void serveAllPlates();
	static void addRequiredIngredientFlags(const Order &order, Common::Array<FlagDescription> &flags);
	bool foodMatches(const Food *food, const Order &order, Common::Array<FlagDescription> &flags,
		bool checkDoneness, bool &allWellDone) const;

	bool updateCursor(const Common::Point &pos);
	void update(int32 ticks);
	bool cook(Food *food, int32 ticks);

	void redraw();
	void drawFood(const Food *food, const Common::Rect &dest, bool jitter, uint32 &randSeed);
	void drawOrderCounts(const Order &order, const Graphics::Font *font, uint32 color);

	void startEnding(const SceneChangeWithFlag &scene, const RandomSoundBlock &sound);

	// -- File data --
	Common::Array<FoodType> _foodTypes;
	Common::Array<Processor> _processors;

	Common::Path _plateImageName;
	Common::Rect _plateSrc;
	Common::Rect _plateDest;
	Common::Rect _plateBarArea;			// clicking it, or outside the plate, puts the plate away
	Common::Rect _serveHotspot;
	Common::Rect _ticketIconSrc;
	Common::Array<PlateLayout> _plateLayouts;
	uint16 _plateFoodCursor = 0;
	uint16 _plateCloseCursor = 0;
	uint16 _serveCursor = 0;
	uint16 _ticketCursor = 0;
	Common::Rect _ticketDest;
	Common::String _countFontName;
	uint32 _countColor = 0;				// ARGB
	float _captionTime = 0;				// seconds the served-order caption stays up after its sound
	RandomSoundBlock _orderRightSound;
	RandomSoundBlock _orderWrongSound;
	RandomSoundBlock _allServedSound;
	Common::Array<Plate> _plates;

	Common::Array<Common::Path> _manualPageNames;
	Common::Rect _manualPrevHotspot;
	Common::Rect _manualNextHotspot;
	Common::Rect _manualOpenHotspot;
	Common::Rect _manualCloseHotspot;
	Common::Rect _manualSrc;
	Common::Rect _manualDest;
	uint16 _manualPrevCursor = 0;
	uint16 _manualNextCursor = 0;
	uint16 _manualOpenCursor = 0;
	uint16 _manualCloseCursor = 0;
	RandomSoundBlock _pageFlipSound;
	RandomSoundBlock _manualOpenSound;
	RandomSoundBlock _manualCloseSound;

	Common::Rect _garbageHotspot;
	RandomSoundBlock _garbageSound;
	uint16 _heldCursor = 0;
	SceneChangeWithFlag _fireScene;
	RandomSoundBlock _fireSound;

	// -- Runtime state --
	Common::Array<Food *> _bins;
	Food *_heldFood = nullptr;
	int _openPlate = -1;
	bool _manualOpen = false;
	uint _manualPage = 0;

	bool _rightSoundPending = false;
	bool _wrongSoundPending = false;
	int32 _captionTicks = 0;

	uint32 _lastTickTime = 0;
	bool _isEnding = false;
	bool _exitRequested = false;
	SceneChangeWithFlag _endScene;
	SoundDescription _endSound;
	Common::Point _mousePos;

	Graphics::ManagedSurface _plateImage;
	Common::Array<Graphics::ManagedSurface> _manualPages;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_COOKINGPUZZLE_H
