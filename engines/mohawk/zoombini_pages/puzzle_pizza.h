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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_PIZZA_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_PIZZA_H

#include "mohawk/zoombini_pages/puzzle_base.h"

namespace Mohawk {

/**
 * Pizza Pass puzzle page (@ref ZoombiniPageType::kPizza).
 * Route 1, Puzzle 3
 *
 * Zoombinis deliver pizzas to the pizza trolls with specific toppings.
 * The player must figure out which toppings each pizza troll wants by trial and error.
 * Deliver each pizza troll's exact requested toppings so the Zoombinis can pass.
 *
 */
class ZoombiniPuzzlePizza : public ZoombiniPuzzle {
public:
	/** Create the Pizza Pass puzzle page. */
	ZoombiniPuzzlePizza(MohawkEngine_Zoombini *vm);
	/** Release topping, troll, and Zoombini features. */
	~ZoombiniPuzzlePizza() override;
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = ZmbXferRouteInfo::kXferPageNamePizzaPass;
	static constexpr int kRouteNumber = 1;
	static constexpr int kRoutePuzzleIdx = 3;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	int getRouteNumber() const override { return kRouteNumber; }
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate troll orders and initialize the active pack. */
	void open() override;
	/** Select the Pizza Pass puzzle music. */
	void setBackgroundMusic() override;
	/** Select the Pizza Pass background bitmap. */
	void setBackgroundBitmap() override;
	/** Initialize puzzle states and departure transfer source. */
	void initStates() override;
	/** Load order, topping, troll, and Zoombini features. */
	void loadFeatures() override;
	/** Select the difficulty-specific Pizza Pass F1 help prompt before activation. */
	void initHelpPrompt() override;
	/** Activate the puzzle and handle specific activation logic. */
	void activatePage() override;
	/** Advance topping, question, delivery, and reaction state. */
	void onEveryFrame() override;
	/** Advance Pizza state after the current render and before shared ambient audio. */
	void onPostRenderFrame() override;
	/** Process postman, question, topping, and delivery callbacks. */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

	/** Start a topping or Zoombini drag and handle page controls. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Record keyboard activity before applying shared puzzle shortcuts. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;
	/** Update topping, order, and delivery hover feedback. */
	ZmbEventHandleResult onMouseMove(const Common::Point &absPos, const Common::Point &relPos) override;

protected:
	/** Pizza Pass actions accepted by the built-in debug console and keyboard dispatcher. */
	enum class BuiltinDebugAction {
		kInvalid,
		kToppings,
		kRegisterRunner,
		kState,
		kPlace,
		kDeliver,
		kQuestion,
		kResetDeliveries,
		kCycleArno,
		kCycleWilla,
		kCycleShyler
	};
	static constexpr const char *kBuiltinDebugActionToppings = "toppings";
	static constexpr const char *kBuiltinDebugActionRegisterRunner = "register-runner";
	static constexpr const char *kBuiltinDebugActionState = "state";
	static constexpr const char *kBuiltinDebugActionPlace = "place";
	static constexpr const char *kBuiltinDebugActionDeliver = "deliver";
	static constexpr const char *kBuiltinDebugActionQuestion = "question";
	static constexpr const char *kBuiltinDebugActionResetDeliveries = "reset-deliveries";
	static constexpr const char *kBuiltinDebugActionCycleArno = "cycle-arno";
	static constexpr const char *kBuiltinDebugActionCycleWilla = "cycle-willa";
	static constexpr const char *kBuiltinDebugActionCycleShyler = "cycle-shyler";
	/** Begin the departure sequence for accepted deliveries. */
	void onGoButtonActivated() override;
	/** Describe the inferred troll orders for diagnostics. */
	Common::String debugGetAnswer() const override;
	/** Report Pizza Pass mistake-budget usage for diagnostics. */
	ZmbChanceInfo debugGetChances() const override;
	/** Pizza Pass supports debugger chance adjustment. */
	bool debugCanSetChances() const override { return true; }
	/** Set Pizza Pass mistake-budget usage for diagnostics. */
	bool debugSetChances(int16 remaining) override;
	/** Handle Pizza-specific delivery setup commands. */
	bool debugDoPageCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe Pizza-specific delivery setup commands. */
	Common::String debugGetPageCommandHelp() const override;
	/** Run a Pizza built-in debug action. */
	bool debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) override;
	/** Describe Pizza built-in debug actions. */
	Common::String debugGetBuiltinDebugCommandHelp() const override;
	/** Handle the Pizza built-in debug keys. */
	ZmbEventHandleResult onDebugKeyDown(const Common::KeyState &kbd) override;
	/** Return the sound priority used by page SCRB playback. */
	const ZoombiniPage::ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;
	/** Classify Arno, Willa, and Shyler dialogue separately from Pizza effects. */
	Audio::Mixer::SoundType getFeatureSoundType(const ZmbFeature *feature, ZmbResource soundRes) const override;

private:
	/** Page-local Pizza Pass orders, toppings, trolls, and sounds. */
	enum PageResourceId : int16 {
		kResScrb0017_DebugRunner = 17,
		kResBackground5000 = 5000,

		// Terrain bitmaps
		kResBitmapTerrain100 = 100,

		// Shape bitmaps
		kResBitmapShape6000_Snoid = 6000,
		kResBitmapShape7000_Order = 7000,
		kResBitmapShape8000_Arno = 8000,
		kResBitmapShape9000_Willa = 9000,
		kResBitmapShape10000_Shyler = 10000,
		kResBitmapShape12000_Topping = 12000,

		kResNode1000_WalkNetwork = 1000,

		kResScrb7000_Question = 7000,
		kResScrb7000_ArnoOrderBase = 7000,
		kResScrb7001_AnswerLevel1 = 7001,
		kResScrb7005_ToppingLevel1Base = 7005,
		kResScrb7015_ToppingLevel2Base = 7015,
		kResScrb7027_ToppingLevel3Base = 7027,
		kResScrb7041_ToppingLevel4Base = 7041,
		kResScrb7057_AnswerLevel1 = 7057,
		kResScrb7058_AnswerLevel2Base = 7058,
		kResScrb7063_AnswerDisplay = 7063,
		kResScrb7066_QuestionExit = 7066,
		kResScrb7067_AnswerReadyLevel1 = 7067,
		kResScrb7068_AnswerReadyLevel2 = 7068,
		kResScrb7069_ButtonHoverBase = 7069,
		kResScrb7124_ButtonHoverLast = 7124,
		kResScrb8000_ArnoOrderBase = 8000,
		kResScrb8006_ArnoOneCorrectBase = 8006,
		kResScrb8008_ArnoMultipleCorrectBase = 8008,
		kResScrb8014_ArnoReject = 8014,
		kResScrb8015_ArnoRejectBase = 8015,
		kResScrb8017_ArnoReactionBase = 8017,
		kResScrb8020_ArnoDeliveryResult = 8020,
		kResScrb8021_ArnoDeliveryAccept = 8021,
		kResScrb8022_ArnoDeliveryEvalBase = 8022,
		kResScrb8024_ArnoRejectBase = 8024,
		kResScrb8026_ArnoRejectBase = 8026,
		kResScrb8028_ArnoRejectBase = 8028,
		kResScrb8030_ArnoReactionBase = 8030,
		kResScrb8032_ArnoOrderIntro = 8032,
		kResScrb8033_ArnoOrderOverlay = 8033,
		kResScrb8034_ArnoIdleBase = 8034,
		kResScrb9000_WillaOrderBase = 9000,
		kResScrb9005_WillaMultipleCorrectBase = 9005,
		kResScrb9010_WillaReactionBase = 9010,
		kResScrb9017_WillaRejectBase = 9017,
		kResScrb9019_WillaIdleBase = 9019,
		kResScrb9021_WillaOneCorrectBase = 9021,
		kResScrb9026_WillaDeliveryResult = 9026,
		kResScrb9027_WillaDeliveryAccept = 9027,
		kResScrb9028_WillaDeliveryEvalBase = 9028,
		kResScrb9030_WillaRejectBase = 9030,
		kResScrb9032_WillaReactionBase = 9032,
		kResScrb9034_WillaOrderIntro = 9034,
		kResScrb10000_ShylerOrderBase = 10000,
		kResScrb10001_ShylerRejectBase = 10001,
		kResScrb10006_ShylerIdleBase = 10006,
		kResScrb10009_ShylerMultipleCorrectBase = 10009,
		kResScrb10014_ShylerOneCorrectBase = 10014,
		kResScrb10020_ShylerReactionBase = 10020,
		kResScrb10023_ShylerRejectBase = 10023,
		kResScrb10027_ShylerRejectBase = 10027,
		kResScrb10030_ShylerDeliveryResult = 10030,
		kResScrb10031_ShylerDeliveryAccept = 10031,
		kResScrb10032_ShylerDeliveryEvalBase = 10032,
		kResScrb10035_ShylerReject = 10035,
		kResScrb10036_ShylerReactionBase = 10036,
		kResScrb10038_ShylerOrderIntro = 10038,
		kResScrb12000_ToppingOverlay = 12000,
		kResScrb12001_ToppingOverlayBase = 12001,
		kResScrb12002_ToppingOverlay = 12002,
		kResScrb12003_ToppingOverlay = 12003,
		kResScrb12006_ToppingOverlayBase = 12006,
		kResScrb12011_ToppingOverlayBase = 12011,
		kResScrb12016_Order0ToppingBase = 12016,
		kResScrb12019_Order1ToppingBase = 12019,
		kResScrb12022_Order2ToppingBase = 12022,
		kResScrb12025_GenericToppingBase = 12025,
		kResScrb12041_ToppingRunnerBase = 12041,
		kResScrb12099_ToppingOverlayLast = 12099,

		kResScrs13000_CarryBase = 13000,
		kResScrs13005_DeliveryBase = 13005,
		kResScrs13010_DeliveryBase = 13010,
		kResScrs13015_DeliveryBase = 13015,
		kResScrs13035_CelebrationBase = 13035,
		kResScrs13099_CarryLast = 13099,
		kResScrs14000_DeliveryBase = 14000,
		kResScrs14002_DeliveryBase = 14002,
		kResScrs14004_DeliveryBase = 14004,

		// Sound resources.
		kResSound7000_ToppingButton4 = 7000,
		kResSound7001_ToppingButton3 = 7001,
		kResSound7002_ToppingButton2 = 7002,
		kResSound7003_ToppingButton1 = 7003,
		kResSound7004_ToppingButton0 = 7004,
		kResSound7005_ToppingButton7 = 7005,
		kResSound7006_ToppingButton6 = 7006,
		kResSound7007_ToppingButton5 = 7007,
		kResSound8000_ArnoSpeechBase = 8000,
		kResSound8040_ArnoSpeechLast = 8040,
		kResSound8040_DeliverySfx = 8040,
		kResSound9000_WillaSpeechBase = 9000,
		kResSound9033_WillaSpeechLast = 9033,
		kResSound10000_ShylerSpeechBase = 10000,
		kResSound10040_ShylerSpeechLast = 10040,
		kResSound15000_ArnoChant = 15000,
		kResSound15001_ArnoChant = 15001,
		kResSound15002_ArnoChant = 15002,
		kResSound15003_ArnoChant = 15003,
		kResSound15004_ArnoChant = 15004,
		kResSound15005_ArnoChant = 15005,
		kResSound15006_ArnoChant = 15006,

		// Sound ranges used by getScriptSoundPriority().
		kResSoundRange0425_PageRangeBase = 425,
		kResSoundRange0475_PagePriorityBase = 475,
		kResSoundRange0499_PageRangeLast = 499,
		kResSoundRange7000_PageRangeBase = 7000,
		kResSoundRange7007_PageRangeLast = 7007,
		kResSoundRange7008_PagePriorityBase = 7008,
		kResSoundRange7009_PagePriorityLast = 7009,
		kResSoundRange8000_SystemRangeBase = 8000,
		kResSoundRange8023_SystemRangeLast = 8023,
		kResSoundRange8024_SystemPriorityBase = 8024,
		kResSoundRange8029_SystemPriorityLast = 8029,
		kResSoundRange8030_SystemRangeBase = 8030,
		kResSoundRange8999_SystemRangeLast = 8999,
		kResSoundRange9000_SystemRangeBase = 9000,
		kResSoundRange9025_SystemRangeLast = 9025,
		kResSoundRange9026_SystemRangeBase = 9026,
		kResSoundRange9999_SystemRangeLast = 9999,
		kResSoundRange10000_SystemRangeBase = 10000,
		kResSoundRange10032_SystemRangeLast = 10032,
		kResSoundRange10033_SystemRangeBase = 10033,
		kResSoundRange10099_SystemRangeLast = 10099,
		kResSoundRange12000_PagePriority = 12000,
		kResSoundRange12001_PagePriority = 12001,
		kResSoundRange12002_PagePriority = 12002,
		kResSoundRange12003_PageRangeBase = 12003,
		kResSoundRange12099_PageRangeLast = 12099,
		kResSoundRange13000_PageRangeBase = 13000,
		kResSoundRange13099_PageRangeLast = 13099,
		kResSoundRange14000_PagePriority = 14000,
		kResSoundRange15000_ArnoChant = 15000,
		kResSoundRange15099_SystemRangeLast = 15099,
	};

	/** Postman animation callbacks. */
	enum PostmanEventCode : int16 {
		kPostmanEventCode000_ToggleFacing = 0
	};

	/** Question-runner callbacks that build and classify the produced pizza. */
	enum QuestionEventCode : int16 {
		kQuestionEventCode032_LoadToppingOverlay = 32,
		kQuestionEventCode060_StartTraitReveal = 60
	};

	/** Willa delivery-result callbacks. */
	enum WillaResultEventCode : int16 {
		kWillaResultEventCode099_RelinkBehindArno = 99
	};

	/** Delivery-evaluation callbacks on the Postman. */
	enum DeliveryEventCode : int16 {
		kDeliveryEventCode000_ToggleFacing = 0,
		kDeliveryEventCode061_StartSnoidScript = 61
	};

	/** Ingredient-bit positions in a Pizza Pass meal. */
	enum MealIngredient : int16 {
		/** Black olives on the pizza. */
		kMealIngredientOlives00 = 0,
		/** Green peppers on the pizza. */
		kMealIngredientPeppers01 = 1,
		/** Pepperoni on the pizza. */
		kMealIngredientPepperoni02 = 2,
		/** Mushrooms on the pizza. */
		kMealIngredientMushrooms03 = 3,
		/** Cheese on the pizza. */
		kMealIngredientCheese04 = 4,
		/** Cherry topping on the sundae. */
		kMealIngredientCherry05 = 5,
		/** Whipped cream topping on the sundae. */
		kMealIngredientWhippedCream06 = 6,
		/** Chocolate sauce topping on the sundae. */
		kMealIngredientChocolateSauce07 = 7
	};

	/** Authored four-frame meal-shape ranges filtered by @ref ZoombiniPuzzlePizza::toppingRunner_preRenderShape(). */
	enum MealShape : int16 {
		/** Cheese animation frames, selected by @ref kMealIngredientCheese04. */
		kMealShapeCheese05_First = 5,
		kMealShapeCheese08_Last = 8,
		/** Mushroom animation frames, selected by @ref kMealIngredientMushrooms03. */
		kMealShapeMushrooms09_First = 9,
		kMealShapeMushrooms12_Last = 12,
		/** Pepperoni animation frames, selected by @ref kMealIngredientPepperoni02. */
		kMealShapePepperoni13_First = 13,
		kMealShapePepperoni16_Last = 16,
		/** Green-pepper animation frames, selected by @ref kMealIngredientPeppers01. */
		kMealShapePeppers17_First = 17,
		kMealShapePeppers20_Last = 20,
		/** Olive animation frames, selected by @ref kMealIngredientOlives00. */
		kMealShapeOlives21_First = 21,
		kMealShapeOlives24_Last = 24,
		/** Plain-sundae animation frames present from level 2 onward. */
		kMealShapePlainSundae25_First = 25,
		kMealShapePlainSundae28_Last = 28,
		/** Chocolate-sauce animation frames, selected by @ref kMealIngredientChocolateSauce07 at level 4. */
		kMealShapeChocolateSauce29_First = 29,
		kMealShapeChocolateSauce32_Last = 32,
		/** Whipped-cream animation frames, selected by @ref kMealIngredientWhippedCream06 from level 2 onward. */
		kMealShapeWhippedCream33_First = 33,
		kMealShapeWhippedCream36_Last = 36,
		/** Cherry animation frames, selected by @ref kMealIngredientCherry05 from level 2 onward. */
		kMealShapeCherry37_First = 37,
		kMealShapeCherry40_Last = 40
	};

	/** Authored four-frame order-display shape ranges filtered by @ref ZoombiniPuzzlePizza::orderFeature_preRenderShape(). */
	enum class OrderFeatureShape : int16 {
		/** Cheese frames, selected by @ref kMealIngredientCheese04. */
		kCheese156_First = 0x9C,
		kCheese159_Last = 0x9F,
		/** Mushroom frames, selected by @ref kMealIngredientMushrooms03. */
		kMushrooms160_First = 0xA0,
		kMushrooms163_Last = 0xA3,
		/** Pepperoni frames, selected by @ref kMealIngredientPepperoni02. */
		kPepperoni164_First = 0xA4,
		kPepperoni167_Last = 0xA7,
		/** Green-pepper frames, selected by @ref kMealIngredientPeppers01. */
		kPeppers168_First = 0xA8,
		kPeppers171_Last = 0xAB,
		/** Olive frames, selected by @ref kMealIngredientOlives00. */
		kOlives172_First = 0xAC,
		kOlives175_Last = 0xAF,
		/** Plain-sundae frames present from level 2 onward. */
		kPlainSundae176_First = 0xB0,
		kPlainSundae179_Last = 0xB3,
		/** Chocolate-sauce frames, selected by @ref kMealIngredientChocolateSauce07 at level 4. */
		kChocolateSauce180_First = 0xB4,
		kChocolateSauce183_Last = 0xB7,
		/** Whipped-cream frames, selected by @ref kMealIngredientWhippedCream06 from level 2 onward. */
		kWhippedCream184_First = 0xB8,
		kWhippedCream187_Last = 0xBB,
		/** Cherry frames, selected by @ref kMealIngredientCherry05 from level 2 onward. */
		kCherry188_First = 0xBC,
		kCherry191_Last = 0xBF,
		/** Arno-and-stone occlusion patch authored into Willa's scripts. */
		kArnoAndStoneOcclusionPatch212 = 0xD4
	};

	/** Identifies the Arno, Willa, and Shyler order lines. */
	enum class TrollOrderLine : int16 {
		/** Arno's always-present order line. */
		kArno00 = 0,
		/** Willa's order line, present from level 2 onward. */
		kWilla01 = 1,
		/** Shyler's order line, present from level 3 onward. */
		kShyler02 = 2
	};

	/** Classification of the submitted meal against one troll order. */
	enum class SubmittedMealClassification : int16 {
		/** Exactly one selected topping is not requested by the troll. */
		kOneUnwantedTopping00 = 0,
		/** No selected topping is unwanted, but at least one request is missing. */
		kMissingRequestedToppings01 = 1,
		/** The submitted topping set exactly matches the troll's request set. */
		kExactMatch02 = 2,
		/**
		 * Original consumer-only reaction value not returned by @ref ZoombiniPuzzlePizza::classifySubmittedMeal().
		 * Its former trigger and gameplay meaning cannot be recovered from the surviving classifier.
		 */
		kUnusedReaction03 = 3,
		/** More than one selected topping is not requested by the troll. */
		kMultipleUnwantedToppings04 = 4
	};

	/** Lifecycle state of one troll order line. */
	enum class TrollOrderState : int16 {
		/** This troll has no order at the current difficulty. */
		kInactive00 = 0,
		/** This troll is waiting for an exact meal match. */
		kActive01 = 1,
		/** An exact meal matched and its reaction is awaiting the acceptance transition. */
		kMatched02 = 2,
		/** The terminal accept-and-eat reaction has been installed for this troll. */
		kAccepted03 = 3
	};

	// --- Phase tracking for feature animations ---
	enum FeaturePhase {
		kPhaseNone = 0,
		kPhaseIntro,
		kPhasePostIntroAmbient,
		kPhaseServeReaction,
		kPhaseDeliveryEval,
		kPhaseDeliveryResult, // Delivery result SCRBs (8020/9026/10030)
		kPhaseExitCallback,
		kPhaseToppingOverlay,
		kPhaseToppingDelivery,
		kPhaseQuestionSetup,
		kPhaseSpawnAnswer,
		kPhaseAcceptTransition,
	};

	// --- Initialization ---
	/** Select difficulty-dependent topping, order, and delivery counts. */
	void setDifficultyParams();
	/** Generate the topping categories used by the current puzzle. */
	void generateToppingSet();
	/** Distribute generated toppings into the authored order lines. */
	void distributeToppings();

	// --- Ingredient toggle & submit ---
	/** Toggle one ingredient in the currently prepared pizza. */
	void handleIngredientToggle(int16 ingredientIdx);
	/** Submit the current pizza for delivery evaluation. */
	void handleSubmit();

	// --- Produce button ---
	/** Register or reload the visible Produce-button feature. */
	void registerProduceButton();
	/** Filter Produce-button meal shapes from the live topping selection. */
	void produceButton_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);

	// --- Order classification & delivery ---
	/** Classify the submitted meal against one troll's generated topping mask. */
	SubmittedMealClassification classifySubmittedMeal(TrollOrderLine orderLine) const;
	/** Serve the next topping required by an order line. */
	void serveNextTopping(TrollOrderLine orderLine);
	/** Place one topping according to the current machine mode. */
	void placeTopping(int16 mode, int16 hintSlot);
	/** Evaluate the prepared pizza against the active order. */
	void evaluateDelivery();
	/** Load the SCRB matching the current delivery result. */
	void loadDeliveryResultScrb();
	/** Advance to the next order/delivery slot. */
	void advanceToNextDeliverySlot();
	/** Advance the authored introduction sequence. */
	void advanceIntroSequence();
	/** Trigger ambient animation on the active order feature. */
	void triggerOrderFeatureAmbientAnim();
	/** Run the long-inactivity troll animation selector. */
	void runOrderFeatureAmbientIdleDriver();
	/** Complete a disposal result immediately when input skips its remaining frames. */
	void finishDisposalResultOnInput();
	/** Start the Produce-button ready flash for the next Postman action. */
	void spawnPostmanSnoid();
	/** Start the returning Postman's animation and restore the Produce button. */
	void animatePostman();
	/** Configure the solved-order troll reaction runners. */
	void setupQuestionRunners();
	/** Finish topping delivery and update the delivery chain. */
	void onToppingDelivered();

	// Synchronous load chant ---
	/** Play the load chant selected by the route state. */
	void playOpeningChant(ZmbSfxGroupFlags sfxGroupFlags);
	/** Play the selected variant of Arno's page-entry pizza song. */
	void playArnoOpeningChant(int16 chantVariant);
	/** Play a chant sound while continuing bounded frame processing. */
	bool playChantSoundSync(int16 sndId);
	/** Return whether an input event requests to skip the load chant. */
	static bool isLoadChantSkipEvent(const Common::Event &event);
	/** Pump the loading wait and optionally honor the skip action. */
	bool pumpLoadWait(uint32 durationMs, bool allowSkip);

	// --- Topping runner management ---
	/** Register the next topping animation runner. */
	void registerToppingRunner();
	/** Complete registration and place a topping runner by resource category. */
	void finishToppingRunnerRegistration(ZmbFeature *newFeature, int16 runnerCategory);
	/** Select a random topping from one troll order line. */
	int16 pickRandomToppingFromOrderLine(TrollOrderLine orderLine);
	/**
	 * Create the four two-topping reject examples displayed in the pit when level 4 opens.
	 *
	 * The examples combine two toppings from the largest troll order with one topping selected from each of the other two orders.
	 * Each combination is retained as a topping runner before the initial page render.
	 */
	void createLevel4RejectExamples(int16 dominantOrderToppingA, int16 dominantOrderToppingB, int16 otherOrderToppingA, int16 otherOrderToppingB);
	/** Link active topping runners into the authored display order. */
	void linkToppingRunners();
	/** Restore a seated troll's accepted-pizza stack. */
	void linkSettledAcceptedToppingStack(int16 orderLine);
	/** Prepare topping runner hotspots for the current mask. */
	void toppingRunner_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Prepare order feature hotspots for the current order state. */
	void orderFeature_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Attach the current order filter to a feature. */
	void attachOrderFilter(ZmbFeature *feature);
	/** Prepare meal-filter hotspots for the current order. */
	void mealFilter_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Attach the current meal filter to a feature. */
	void attachMealFilter(ZmbFeature *feature);
	/** Hide and invalidate the topping overlay. */
	void hideToppingOverlay();

	// --- Callback event handlers ---
	/** Process question-runner and Willa delivery-result callbacks. */
	void handleZmbExitEvent(ZmbFeature *feature, int16 eventCode);
	/** Process a delivery Zoombini callback. */
	void handleZmbDeliveryEvent(ZmbFeature *feature, int16 eventCode);
	/** Mark an order line complete and advance its phase. */
	void handleOrderLineComplete(int16 orderLine);
	/** Restore active-pack Snoids before the shared departure writer runs. */
	void resetActivePackForDeparture();

	// --- Topping bitmask history ---
	/** Build the bitmask represented by the current active pack. */
	byte packToppingBitmask() const;
	/** Return whether the current topping mask matches the selected order. */
	bool checkToppingMaskMatch() const;

	// --- Helpers ---
	/** Return whether the current difficulty provides a machine button for an ingredient. */
	bool hasToppingButtonForIngredient(int16 ingredientIdx) const;
	/** Return the current difficulty's off/on SCRB ID for an ingredient, or -1 when it has no button. */
	int16 getToppingButtonScrbId(int16 ingredientIdx, bool selected) const;
	/** Update the TLC v2 machine-button hover state. */
	void updateV2MachineButtonHover();
	/** Return whether a point hits one topping button. */
	bool isToppingButtonAtPoint(int16 ingredientIdx, const Common::Point &point) const;
	/** Reload a feature's SCRB while preserving its animation owner. */
	void reloadScrbAnimation(ZmbFeature *feature, int16 scrbId);
	/** Return the canonical trait index selected by an order slot. */
	int16 getTraitIndexForOrder(int16 orderSlot) const;
	/** Parse one console-facing Pizza action name. */
	static BuiltinDebugAction parseBuiltinDebugAction(const Common::String &action);
	/** Run one typed Pizza keyboard-debug branch. */
	bool runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output);
	struct SlotInfo {
		/** Category name used in a generated troll line. */
		const char *category;
		/** Numeric topping value used by the order text. */
		int number;
		/** Display name used in a generated troll line. */
		const char *name;
	};

	// -----------------------------------------------------------------------
	// Static data
	// -----------------------------------------------------------------------
	static constexpr Common::Point kSnoidPositions[16]{
		Common::Point(288, 389),
		Common::Point(240, 386),
		Common::Point(257, 434),
		Common::Point(202, 396),
		Common::Point(224, 437),
		Common::Point(186, 443),
		Common::Point(158, 400),
		Common::Point(151, 455),
		Common::Point(126, 391),
		Common::Point(118, 446),
		Common::Point(89, 403),
		Common::Point(86, 456),
		Common::Point(48, 396),
		Common::Point(51, 440),
		Common::Point(20, 416),
		Common::Point(18, 457),
	};
	/** DRAW_ON_REG machine-seat position for the current deliverer. */
	static constexpr Common::Point kMachineSeatPosition = Common::Point(270, 334);
	/**
	 * Topping SCRB base ID table indexed by difficulty.
	 * Each topping uses two consecutive IDs for its off/on visuals.
	 * Level 2 has no cheese button, so the cherry and whipped-cream pairs immediately follow the mushroom pair.
	 */
	static constexpr int16 kToppingScrbTable[4]{
		kResScrb7005_ToppingLevel1Base,
		kResScrb7015_ToppingLevel2Base,
		kResScrb7027_ToppingLevel3Base,
		kResScrb7041_ToppingLevel4Base,
	};
	/** TLC v2 topping-button press sounds indexed by topping. */
	static constexpr int16 kToppingButtonSoundTable[8]{
		kResSound7004_ToppingButton0,
		kResSound7003_ToppingButton1,
		kResSound7002_ToppingButton2,
		kResSound7001_ToppingButton3,
		kResSound7000_ToppingButton4,
		kResSound7007_ToppingButton5,
		kResSound7006_ToppingButton6,
		kResSound7005_ToppingButton7,
	};
	/** TLC v2 topping-button hit rectangles indexed by difficulty and topping. */
	static constexpr int16 kToppingClickRectTable[4][8][4]{
		{{48, 185, 100, 210}, {48, 211, 100, 236}, {48, 237, 100, 263}, {48, 264, 100, 288}, {48, 289, 100, 317}, {88, 195, 115, 234}, {88, 236, 115, 273}, {88, 276, 115, 314}},
		{{33, 196, 85, 221}, {33, 222, 85, 247}, {33, 248, 85, 275}, {33, 276, 85, 301}, {33, 289, 85, 317}, {88, 215, 115, 254}, {88, 255, 115, 294}, {88, 276, 115, 314}},
		{{33, 182, 85, 210}, {33, 211, 85, 236}, {33, 237, 85, 262}, {33, 263, 85, 288}, {33, 289, 85, 316}, {88, 215, 115, 254}, {88, 255, 115, 295}, {88, 296, 115, 314}},
		{{33, 182, 85, 210}, {33, 211, 85, 236}, {33, 237, 85, 262}, {33, 263, 85, 288}, {33, 289, 85, 316}, {88, 195, 115, 232}, {88, 237, 115, 274}, {88, 276, 115, 314}},
	};
	/** SCRB offset from answer visuals to v2 hover visuals. */
	static const uint16 kButtonHoverScrbDelta = kResScrb7069_ButtonHoverBase - kResScrb7001_AnswerLevel1;
	/**
	 * Hit rectangles are instance members because Common::Rect requires runtime
	 * construction and ScummVM prohibits global C++ constructors.
	 */
	const Common::Rect _produceButtonClickRect = Common::Rect(33, 97, 120, 157);
	const Common::Rect _machineMouthClickRect = Common::Rect(130, 99, 174, 308);

	// -----------------------------------------------------------------------
	// Difficulty parameters
	// -----------------------------------------------------------------------

	/** Number of logical ingredient slots used by the current difficulty. */
	int16 _machineToppingSlotCount = 5;
	/** Minimum number of topping categories selected by generation. */
	int16 _minimumGeneratedToppingCount = 2;
	/** Per-pass random threshold for including a topping category. */
	int16 _toppingGenerationThreshold = 500;
	/** Remaining non-exact submissions allowed before a deliverer is lost. */
	int16 _remainingMistakeAllowance = 6;
	/** Initial non-exact-submission allowance for this difficulty. */
	int16 _initialMistakeAllowance = 6;

	// -----------------------------------------------------------------------
	// Topping state arrays
	// -----------------------------------------------------------------------
	/** Topping categories distributed among the active troll orders. */
	bool _generatedToppings[8] = {};
	/** Toppings requested by Arno. */
	bool _arnoToppings[8] = {};
	/** Toppings requested by Willa. */
	bool _willaToppings[8] = {};
	/** Toppings requested by Shyler. */
	bool _shylerToppings[8] = {};
	/** Machine selection captured for the delivery currently being evaluated. */
	bool _submittedMealToppings[8] = {};
	/** Live topping-button selection used to prepare the next meal. */
	bool _selectedMealToppings[8] = {};

	// -----------------------------------------------------------------------
	// Troll order state
	// -----------------------------------------------------------------------
	/** Lifecycle state of the Arno, Willa, and Shyler order lines. */
	TrollOrderState _trollOrderStates[3] = {};

	// -----------------------------------------------------------------------
	// Delivery tracking
	// -----------------------------------------------------------------------
	/** Zero-based pack sequence index of the current deliverer. */
	int16 _delivererSequenceIdx = -1;
	/** Whether the latest non-exact attempt remained within the mistake allowance. */
	bool _delivererSurvivedAttempt = false;
	/** Transient counter marking the attempt that consumed the final safe mistake. */
	int16 _finalSafeAttemptCounter = 0;
	/** Whether no pack Snoid remains available as a deliverer. */
	bool _delivererPoolExhausted = false;
	/**
	 * Whether every active troll order has reached the exact-match state.
	 * Go enables after the final troll reaction completes.
	 */
	bool _allTrollOrdersMatched = false;
	/**
	 * Whether the player may start a new delivery with Produce.
	 * Ingredient toggles remain separately available while a delivery chain advances.
	 */
	bool _produceSubmissionEnabled = false;
	/** Scratch count incremented for an exact order classification and reset otherwise. */
	int16 _exactOrderMatchCount = 0;
	/** Nonzero token that preserves an accepted troll's runner link during return setup. */
	int16 _preserveAcceptedTrollLinkCount = 0;
	/** Whether the submitted meal bitmask already exists in the history. */
	bool _submittedMealAlreadyTried = false;
	/** Number of troll orders accepted from the meal currently being processed. */
	int16 _acceptedOrdersThisMeal = 0;
	/** One-based troll order slot that owns the current retained topping runner. */
	int16 _toppingOwnerOrderSlot = 0;
	/** Topping-runner resource category: generic 4, Arno 5, Willa 6, or Shyler 7. */
	int16 _toppingRunnerCategory = 0;
	/** Whether the next delivery SCRS event must be consumed without playback. */
	bool _skipDeliveryScriptEvent = false;
	/** Whether the final-safe-attempt evaluation must replay through the next order. */
	bool _finalSafeAttemptReplayPending = false;
	/** One-based troll order slot awaiting a non-exact delivery result. */
	int16 _pendingResultOrderSlot = 0;
	/** Number of deliverers lost after exhausting the mistake allowance. */
	int16 _lostDelivererCount = 0;
	/** Whether the ready-flash completion must select the next deliverer. */
	bool _nextDelivererRequired = false;
	/** Absolute frame of the last Pizza click, key, or order-serving activity. */
	uint32 _lastActivityFrame = 0;
	/** Whether an order runner completed during the current render pass. */
	bool _trollFeatureCompletedThisFrame = false;

	// -----------------------------------------------------------------------
	// @ref ZoombiniPuzzlePizza::onEveryFrame() polls these deliverer-walk and delivery-chain watchers.
	// Each slot tracks the liveness of one hotspot group.
	// -----------------------------------------------------------------------
	/** Whether the surviving deliverer is walking back to the machine seat. */
	bool _delivererReturnPending = false;
	/** Whether a newly selected deliverer is walking into the machine seat. */
	bool _delivererWalkInPending = false;
	/** Whether topping-overlay completion still needs classification. */
	bool _toppingOverlayCompletionPending = false;

	// -----------------------------------------------------------------------
	// Topping bitmask history
	// -----------------------------------------------------------------------
	/** Recent topping masks used to avoid repeating generated orders. */
	byte _toppingMaskHistory[28] = {};
	/** Last occupied slot in @ref _toppingMaskHistory, or -1 when empty. */
	int16 _lastToppingMaskHistoryIdx = -1;

	// -----------------------------------------------------------------------
	// Intro sequence
	// -----------------------------------------------------------------------
	/**
	 * State-machine step that selects the next troll introduction SCRB to play.
	 *
	 * Pizza Pass advances it as each troll intro animation completes.
	 * A zero value marks the completed state.
	 */
	int16 _introSequenceStep = 1;
	/** Whether the introduction has completed. */
	bool _introComplete = false;

	// -----------------------------------------------------------------------
	// Animation cycling counters (per order line)
	// -----------------------------------------------------------------------
	/** Arno SCRB step for a meal with no unwanted toppings but missing requests. */
	int16 _arnoMissingToppingAnimStep = 0;
	/** Arno SCRB cycle for exactly one unwanted topping. */
	int16 _arnoOneUnwantedAnimCycle = 0;
	/** Arno SCRB cycle for multiple unwanted toppings. */
	int16 _arnoMultipleUnwantedAnimCycle = 0;
	/** Willa SCRB step for a meal with no unwanted toppings but missing requests. */
	int16 _willaMissingToppingAnimStep = 0;
	/** Willa SCRB cycle for exactly one unwanted topping. */
	int16 _willaOneUnwantedAnimCycle = 0;
	/** Willa SCRB cycle for multiple unwanted toppings. */
	int16 _willaMultipleUnwantedAnimCycle = 0;
	/** Shyler SCRB step for a meal with no unwanted toppings but missing requests. */
	int16 _shylerMissingToppingAnimStep = 0;
	/** Shyler SCRB cycle for exactly one unwanted topping. */
	int16 _shylerOneUnwantedAnimCycle = 0;
	/** Shyler SCRB cycle for multiple unwanted toppings. */
	int16 _shylerMultipleUnwantedAnimCycle = 0;

	// -----------------------------------------------------------------------
	// Phase tracking per feature
	// -----------------------------------------------------------------------
	/** Animation phase of Arno's feature. */
	FeaturePhase _arnoPhase = kPhaseNone;
	/** Animation phase of Willa's feature. */
	FeaturePhase _willaPhase = kPhaseNone;
	/** Animation phase of Shyler's feature. */
	FeaturePhase _shylerPhase = kPhaseNone;
	/** Animation phase of the topping overlay. */
	FeaturePhase _overlayPhase = kPhaseNone;
	/** Animation phase of the pizza-machine feature. */
	FeaturePhase _machinePhase = kPhaseNone;
	/** Animation phase of the visible Produce button. */
	FeaturePhase _produceButtonPhase = kPhaseNone;

	// -----------------------------------------------------------------------
	// Postman state
	// -----------------------------------------------------------------------
	/** Postman Snoid used by the delivery animation. */
	ZmbSnoid *_postmanSnoid = nullptr;

	// -----------------------------------------------------------------------
	// Puzzle state
	// -----------------------------------------------------------------------
	/** Whether the authored Produce and machine-mouth click zones accept input. */
	bool _produceClickZonesEnabled = false;
	/** Whether the load chant may be skipped on the next input. */
	bool _loadChantSkipPending = false;

	// -----------------------------------------------------------------------
	// Celebration animation (hoorah fidget)
	// Per-puzzle SCRS scheduled on idle snoids after correct answers or puzzle milestones.
	// Distinct from the global idle fidget system
	// which is driven by user inactivity through @ref kSnoidAnimState006_Fidget and @ref ZmbSnoid::onSnoidAnimTick().
	// Pizza owns eligibility and completion; matching celebration fields in other
	// puzzles do not define a common state machine.
	// -----------------------------------------------------------------------
	/** Number of Snoids targeted by the current celebration batch. */
	int16 _celebrationTarget = 0;
	/** Number of celebration animations already played. */
	int16 _celebrationsPlayed = 0;
	/** Accepted order completions awaiting the current celebration batch. */
	int16 _pendingCelebrationBatchCount = 0;
	/** Frame at which the last celebration started. */
	uint32 _lastCelebrationFrame = 0;
	/** Non-repeating random pool state for celebration selection. */
	uint32 _celebrationRandomPool = 0;

	// -----------------------------------------------------------------------
	// Feature runners
	// -----------------------------------------------------------------------
	/** Visible Produce-button feature. */
	ZmbFeature *_produceButtonFeature = nullptr;
	/** Machine topping-button features indexed by ingredient; unavailable ingredient slots remain null. */
	ZmbFeature *_toppingButtonFeatures[8] = {};
	/** Most recent pointer position used to recompute TLC v2 button hover. */
	Common::Point _lastMachineMousePos = Common::Point(-1, -1);
	/** Willa order and reaction feature. */
	ZmbFeature *_willaFeature = nullptr;
	/** Arno order and reaction feature. */
	ZmbFeature *_arnoFeature = nullptr;
	/** Shyler order and reaction feature. */
	ZmbFeature *_shylerFeature = nullptr;
	/** Serving-stone feature used as the Pizza runner-list Z-order anchor. */
	ZmbFeature *_servingStoneFeature = nullptr;
	/** Pizza-machine feature that owns SCRB 7000 and the 7066 production callback. */
	ZmbFeature *_machineFeature = nullptr;
	/** Live topping overlay feature. */
	ZmbFeature *_toppingOverlayFeature = nullptr;

	// -----------------------------------------------------------------------
	// Topping runner tracking
	// -----------------------------------------------------------------------
	struct ToppingRunnerSlot {
		/** Feature runner owned by this topping slot. */
		ZmbFeature *feature = nullptr;
		/** Submitted-meal bitmask represented by the runner. */
		byte mask = 0;
		/** SCRB resource currently loaded on the runner. */
		int16 scrbId = 0;
		// @ref ZoombiniPuzzlePizza::registerToppingRunner() marks completion-trigger runners.
		// Those runners call @ref ZoombiniPuzzlePizza::spawnAnswerPostman() on completion.
		// Demonstration runners leave this behavior disabled.
		/** Whether completion spawns the answer Zoombini. */
		bool spawnOnComplete = false;
	};
	/** Registered topping runners in creation order. */
	ToppingRunnerSlot _toppingRunnerSlots[28] = {};
	/** Last occupied slot in @ref _toppingRunnerSlots, or -1 when empty. */
	int16 _lastToppingRunnerSlotIdx = -1;
	/** Cyclic SCRB offset for generic retained topping runners. */
	int16 _genericToppingRunnerCycle = -1;
	/** Cyclic accepted-pizza runner slot for Arno. */
	int16 _arnoToppingRunnerCycle = -1;
	/** Cyclic accepted-pizza runner slot for Willa. */
	int16 _willaToppingRunnerCycle = -1;
	/** Cyclic accepted-pizza runner slot for Shyler. */
	int16 _shylerToppingRunnerCycle = -1;
	/** Whether the generic SCRB cycle has wrapped and must reuse an old runner. */
	bool _genericToppingRunnersWrapped = false;
	/** Accepted-pizza runners assigned to Arno. */
	ZmbFeature *_arnoToppingRunnerSlots[3] = {};
	/** Accepted-pizza runners assigned to Willa. */
	ZmbFeature *_willaToppingRunnerSlots[3] = {};
	/** Accepted-pizza runners assigned to Shyler. */
	ZmbFeature *_shylerToppingRunnerSlots[3] = {};
	/** Starting ID for dynamically created topping features. */
	uint16 _nextDynamicFeatureId = 30000;
	/** Pizza debug state advanced by R, O, and D. */
	int16 _builtinDebugUnlockLevel = 0;
	/** Built-in debug SCRB cycle indices for Arno, Willa, and Shyler. */
	int16 _builtinDebugScrbCycle[3] = {};

	/** Create a topping runner with its initial SCRB and frame interval. */
	ZmbFeature *createToppingRunnerFeature(int16 scrbId, uint32 frameInterval);
	/** Return the topping bitmask represented by a runner feature. */
	byte getToppingRunnerMask(const ZmbFeature *feature) const;
};

} // End of namespace Mohawk

#endif
