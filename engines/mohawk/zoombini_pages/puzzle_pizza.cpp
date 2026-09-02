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

#include "mohawk/zoombini_pages/puzzle_pizza.h"
#include "common/events.h"
#include "common/str.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_resource.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

constexpr Common::Point ZoombiniPuzzlePizza::kSnoidPositions[16];
constexpr Common::Point ZoombiniPuzzlePizza::kMachineSeatPosition;
constexpr int16 ZoombiniPuzzlePizza::kToppingScrbTable[4];
constexpr int16 ZoombiniPuzzlePizza::kToppingButtonSoundTable[8];
constexpr int16 ZoombiniPuzzlePizza::kToppingClickRectTable[4][8][4];

bool ZoombiniPuzzlePizza::isLoadChantSkipEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
		return true;
	default:
		return false;
	}
}

ZoombiniPuzzlePizza::ZoombiniPuzzlePizza(MohawkEngine_Zoombini *vm) : ZoombiniPuzzle(vm, ZoombiniPageType::kPizza, ZmbSrcPageKind::kPizza_04) {
}

ZoombiniPuzzlePizza::~ZoombiniPuzzlePizza() {
}

const ZoombiniPage::ScriptSoundPriorityRanges &ZoombiniPuzzlePizza::getScriptSoundPriorityRanges() const {
	// Original shared page dispatch automatically prepends SND 996-997 at priority 32.
	static const ZoombiniPage::ScriptSoundPriorityRanges kRanges = {
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kSysResSoundRange20000_Base, kSysResSoundRange29999_Last},
		{kResSoundRange8024_SystemPriorityBase, kResSoundRange8029_SystemPriorityLast},
		{kResSoundRange15000_ArnoChant, kResSoundRange15099_SystemRangeLast},
		{kResSoundRange10000_SystemRangeBase, kResSoundRange10032_SystemRangeLast},
		{kResSoundRange9000_SystemRangeBase, kResSoundRange9025_SystemRangeLast},
		{kResSoundRange8000_SystemRangeBase, kResSoundRange8023_SystemRangeLast},
		{kSysResSoundRange0996_DepartSFX, kSysResSoundRange0997_ArriveSFX},
		{kResSoundRange12000_PagePriority, kResSoundRange12000_PagePriority},
		{kResSoundRange7008_PagePriorityBase, kResSoundRange7009_PagePriorityLast},
		{kResSoundRange12001_PagePriority, kResSoundRange12001_PagePriority},
		{kResSoundRange0475_PagePriorityBase, kResSoundRange0499_PageRangeLast},
		{kResSoundRange14000_PagePriority, kResSoundRange14000_PagePriority},
		{kResSoundRange0425_PageRangeBase, kResSoundRange0499_PageRangeLast},
		{kResSoundRange8030_SystemRangeBase, kResSoundRange8999_SystemRangeLast},
		{kResSoundRange9026_SystemRangeBase, kResSoundRange9999_SystemRangeLast},
		{kResSoundRange10033_SystemRangeBase, kResSoundRange10099_SystemRangeLast},
		{kResSoundRange12002_PagePriority, kResSoundRange12002_PagePriority},
		{kResSoundRange7000_PageRangeBase, kResSoundRange7007_PageRangeLast},
		{kResSoundRange12003_PageRangeBase, kResSoundRange12099_PageRangeLast},
		{kResSoundRange13000_PageRangeBase, kResSoundRange13099_PageRangeLast}};
	return kRanges;
}

Audio::Mixer::SoundType ZoombiniPuzzlePizza::getFeatureSoundType(const ZmbFeature *feature, ZmbResource sndRes) const {
	if (sndRes._archiveKind == ZmbResource::kPage) {
		if (feature == _arnoFeature && kResSound8000_ArnoSpeechBase <= sndRes._id && sndRes._id <= kResSound8040_ArnoSpeechLast)
			return Audio::Mixer::kSpeechSoundType;
		if (feature == _willaFeature && kResSound9000_WillaSpeechBase <= sndRes._id && sndRes._id <= kResSound9033_WillaSpeechLast)
			return Audio::Mixer::kSpeechSoundType;
		if (feature == _shylerFeature && kResSound10000_ShylerSpeechBase <= sndRes._id && sndRes._id <= kResSound10040_ShylerSpeechLast)
			return Audio::Mixer::kSpeechSoundType;
	}
	return ZoombiniPuzzle::getFeatureSoundType(feature, sndRes);
}

void ZoombiniPuzzlePizza::open() {
	openArchive(ZMB_MHK_PIZZA);
}

void ZoombiniPuzzlePizza::setBackgroundMusic() {
	// Help narration is available through F1 and does not start on page load.
}

void ZoombiniPuzzlePizza::setBackgroundBitmap() {
	_vm->_gfx->setPalette(kResBackground5000);
	_vm->_gfx->drawBackground(kResBackground5000);
}

// ---------------------------------------------------------------------------
// Apply per-level constants.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::setDifficultyParams() {
	// Per-level topping and delivery parameters:
	// Level 1: slots=5, minimum generated=2, threshold=500, mistake allowance=6
	// Level 2: slots=7, minimum generated=3, threshold=800, mistake allowance=7
	// Level 3: slots=7, minimum generated=3, threshold=1000, mistake allowance=7
	// Level 4: slots=8, minimum generated=4, threshold=1000, mistake allowance=7
	static constexpr int16 kSlots[4] = {5, 7, 7, 8};
	static constexpr int16 kTarget[4] = {2, 3, 3, 4};
	static constexpr int16 kThreshold[4] = {500, 800, 1000, 1000};
	static constexpr int16 kMistakeAllowance[4] = {6, 7, 7, 7};

	_machineToppingSlotCount = kSlots[_difficultyLevel - 1];
	_minimumGeneratedToppingCount = kTarget[_difficultyLevel - 1];
	_toppingGenerationThreshold = kThreshold[_difficultyLevel - 1];
	_remainingMistakeAllowance = kMistakeAllowance[_difficultyLevel - 1];
	_initialMistakeAllowance = kMistakeAllowance[_difficultyLevel - 1];

	// Order line activation
	_trollOrderStates[0] = TrollOrderState::kActive01; // Arno always active
	if (kPuzzleLevel2 <= _difficultyLevel)
		_trollOrderStates[1] = TrollOrderState::kActive01;
	else
		_trollOrderStates[1] = TrollOrderState::kInactive00;
	if (kPuzzleLevel3 <= _difficultyLevel)
		_trollOrderStates[2] = TrollOrderState::kActive01;
	else
		_trollOrderStates[2] = TrollOrderState::kInactive00;
}

void ZoombiniPuzzlePizza::initStates() {
	// Apply per-level constants
	setDifficultyParams();
	_lastActivityFrame = _vm->getAnimationFrameCounter(_vm->_system->getMillis());
	_trollFeatureCompletedThisFrame = false;

	// Disable Produce until the intro reaches its interactive boundary.
	_produceSubmissionEnabled = false;

	// Start the intro sequence.
	_introSequenceStep = 1;
}

void ZoombiniPuzzlePizza::loadFeatures() {
	// Render Pizza runners in linked-list order.
	// Initial registration establishes the base layering, and later delivery overlays use explicit relinking.
	_manualZOrder = true;

	// Load NODE and PATH for walk network
	loadNodePath(ZmbResource(ZmbResource::kPage, kResNode1000_WalkNetwork));

	// Load terrain barrier bitmap (tBMP 100)
	loadTerrainBitmap(kResBitmapTerrain100);

	// Preload shape images
	// Shapes at tBMP 6000
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Snoid));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Order));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Arno));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Willa));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Shyler));
	_vm->_gfx->preloadShapes(ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Topping));

	// Load main features: 69 SCRBs at 7000
	ZmbFeature *mainFeature = createPuzzleMainFeatureHead();

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 36; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Arno), kResScrb8000_ArnoOrderBase + i);
		}
	}

	{
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 45; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Topping), kResScrb12000_ToppingOverlay + i);
		}
	}

	// Conditional feature groups for difficulty levels 1+
	if (kPuzzleLevel2 <= _difficultyLevel) {
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 35; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Willa), kResScrb9000_WillaOrderBase + i);
		}
	}

	if (kPuzzleLevel3 <= _difficultyLevel) {
		ZmbFeature *parent = mainFeature;
		for (uint16 i = 0; i < 39; i++) {
			parent = loadSubFeature(parent, ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Shyler), kResScrb10000_ShylerOrderBase + i);
		}
	}

	// Group 0 uses NORMAL state 9 with tBMP 3100 for the six weak-hit and ejection scripts.
	registerScrsGroup(kResScrs14000_DeliveryBase, 6);
	for (uint16 i = 0; i < 6; i++) {
		loadSnoid(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Snoid),
				  kResScrs14000_DeliveryBase + i,
				  ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00020000_SKIP_RENDER);
	}

	// Group 1 uses REJECT state 8 with tBMP 3000 for carrying and celebration scripts.
	registerScrsGroup(kResScrs13000_CarryBase, 40);
	for (uint16 i = 0; i < 40; i++) {
		loadSnoid(ZmbResource(ZmbResource::kPage, kResBitmapShape6000_Snoid),
				  kResScrs13000_CarryBase + i,
				  ZmbFeature::FLAG_00000001_TYPE_SNOID | ZmbFeature::FLAG_00020000_SKIP_RENDER);
	}

	// SCRB 7063 is the hidden DRAW_ON_REG target at the delivery position.
	// It owns drop-target coverage only; the visible produce button uses a separate runner.
	loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Order), kResScrb7063_AnswerDisplay, 7, kMachineSeatPosition,
		ZmbFeature::FLAG_00002000_DRAW_ON_REG | ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_01000000_DEFER_RENDER);

	// The machine runner keeps SCRB 7000 materialized while dormant,
	// then replaces it with SCRB 7066 when the player produces a pizza.
	_machineFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Order), kResScrb7000_Question, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);

	{
		for (int16 i = 0; i < _machineToppingSlotCount; i++) {
			const int16 toppingScrbId = getToppingButtonScrbId(i, false);
			if (toppingScrbId < 0)
				continue;

			_toppingButtonFeatures[i] = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Order), toppingScrbId, 6,
				ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
		}
	}

	// The original generates the orders only after the machine and topping runners exist.
	// Level 4 also registers and renders four authored demonstration runners here.
	distributeToppings();

	// Register the visible machine runner before the Zoombinis.
	// Its later SCRB reloads retain this position behind every Postman.
	registerProduceButton();

	// Load Zoombinis from active pack at 16 pedestal positions
	loadZoombinisFromPack(kSnoidPositions, ARRAYSIZE(kSnoidPositions));
	// Each accepted order starts a batch of up to three celebrations.
	// The all-orders-ready path later expands the final batch to the loaded count minus one.
	_celebrationTarget = CLIP<int16>(getPackSnoidCount() - 1, 0, 3);
	_celebrationsPlayed = 0;
	_pendingCelebrationBatchCount = 0;
	_lastCelebrationFrame = 0;
	_celebrationRandomPool = 0;

	// Layout and stagger walk-in (200ms walk delay)
	layoutStaticAndWalkIn(200, false);
	assignStaggeredWalkDelays(30, 45);
	// Clear acceptance inherited from the previous puzzle.
	schedulePackSnoids(false, false);

	// Set up Go/Map/Help buttons
	configureStandardPuzzleControlRects();
	loadStandardPuzzleControlFeatures(kResBitmapShape6000_Snoid);

	// The opening chant consumes this selector before the common post-load initialization point.
	initSfxGroupFlags();

	// The troll load chant plays synchronously between resource setup and the intro sequence.
	// It accompanies the animated eye cursor between the Xfer map and the Pizza page.
	// At this point, the Pizza background exists only on the back screen.
	// Keep the display black during the voice SFX, then reveal the page.
	_vm->_gfx->fillArea(ZoombiniGraphics::kShapeScreen, ZoombiniGraphics::kBlackKey);
	_vm->_gfx->flushScreens();
	_vm->_system->updateScreen();
	playOpeningChant(_entrySfxGroupFlags);

	// Initial Z-order (back->front): Willa -> Arno -> Shyler.
	// The original registers these runners after the synchronous load chant.
	if (kPuzzleLevel2 <= _difficultyLevel) {
		_willaFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape9000_Willa), kResScrb9034_WillaOrderIntro, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
	}

	_arnoFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Arno), kResScrb8032_ArnoOrderIntro, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);

	if (kPuzzleLevel3 <= _difficultyLevel) {
		_shylerFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape10000_Shyler), kResScrb10038_ShylerOrderIntro, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
	}

	_servingStoneFeature = loadScrbFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape8000_Arno), kResScrb8033_ArnoOrderOverlay, 6,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY);

	// Materialize the complete initial runner list before activation starts the intro.
	renderFeatures();
}

void ZoombiniPuzzlePizza::initHelpPrompt() {
	int16 helpSoundId;
	if (kPuzzleLevel1 < _difficultyLevel)
		helpSoundId = kSysResSound20072_HelpHard;
	else
		helpSoundId = kSysResSound20071_HelpEasy;
	_activeHelpSoundId = ZmbResource(ZmbResource::kSystem, helpSoundId);
}

void ZoombiniPuzzlePizza::activatePage() {
	ZoombiniPage::activatePage();

	// Disable Produce until the intro reaches its interactive boundary.
	_produceSubmissionEnabled = false;

	// Start the intro sequence.
	_introSequenceStep = 1;
	advanceIntroSequence();

	// Enable the pizza generate button.
	_produceClickZonesEnabled = true;
}

// ---------------------------------------------------------------------------
// Start a delivery attempt.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::onGoButtonActivated() {
	// Cancel a still-pending narrator watch before departing.
	if (_arnoPhase == kPhaseQuestionSetup)
		_arnoPhase = kPhaseNone;
	_vm->_sound->stopAllSoundQueues();
	if (_pendingCelebrationBatchCount)
		resetActivePackForDeparture();

	// Clear so the pack's walk-off animation may route normally again.
	setSnoidNodePathDisabled(false);
	// Rebuild the continuing set from visible survivors.
	// Rejected deliverers are hidden, so @ref ZoombiniPage::schedulePackSnoids() leaves them out.
	schedulePackSnoids(false, true);

	startDepartWalkAnimation(Common::Point(690, 250));
	ZoombiniInteractive::onGoButtonActivated();
}

void ZoombiniPuzzlePizza::resetActivePackForDeparture() {
	// Restore every visible pack runner to its assigned pedestal pose before the one-shot
	// departure enrollment. Otherwise a runner in SCRS 13035-13039 is not idle and
	// is omitted from the walk-off snapshot.
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *snoid = *it;
		if (!snoid || !snoid->isPackSnoid() || !snoid->isRenderActivated())
			continue;
		const int16 packIdx = static_cast<int16>(snoid->getId()) - 10000;
		if (packIdx < 0 || static_cast<int16>(ARRAYSIZE(kSnoidPositions)) <= packIdx)
			continue;

		addExternalDirtyRect(snoid->getClickRect());
		if (snoid->getActiveScrsId() != 0)
			snoid->finishScrsPlayback(false);
		snoid->setAnimTargetPos(kSnoidPositions[packIdx]);
		snoid->setAnimState(kSnoidAnimState000_Idle, &kSnoidPositions[packIdx]);
		snoid->setupIdleHotspots();
		snoid->setNeedsRedraw(true);
	}

	_pendingCelebrationBatchCount = 0;
	_celebrationsPlayed = 0;
	_lastCelebrationFrame = 0;
	_celebrationRandomPool = 0;
}

ZmbChanceInfo ZoombiniPuzzlePizza::debugGetChances() const {
	// An exact topping combination is accepted without calling @ref ZoombiniPuzzlePizza::evaluateDelivery().
	// Only a non-exact delivery decrements the round's unsuccessful-delivery allowance.
	return {ZmbChanceInfo::ZmbChanceType::kMistake, _initialMistakeAllowance,
			CLIP<int16>(static_cast<int16>(_initialMistakeAllowance - _remainingMistakeAllowance), 0, _initialMistakeAllowance),
			"unsuccessful pizza or sundae delivery"};
}

Common::String ZoombiniPuzzlePizza::debugGetPageCommandHelp() const {
	Common::String output;
	output += "  delivery-test <arno|willa|shyler> <partial|full|reject|reject-multi>\n";
	output += "      Prepare a first-delivery renderer test from the generated troll order.\n";
	output += "      Click Produce after closing the debugger to run the normal delivery chain.\n";
	return output;
}

Common::String ZoombiniPuzzlePizza::debugGetBuiltinDebugCommandHelp() const {
	Common::String output;
	output += Common::String::format("  %-7s (%s)\n", "A/a", kBuiltinDebugActionToppings);
	output += "    Draw eight topping flags for each troll whose order is active, plus the current meal.\n";
	output += "    This is read-only.\n";
	output += Common::String::format("  %-7s (%s)\n", "B/b", kBuiltinDebugActionRegisterRunner);
	output += "    Add one inert standard runner retaining SCRB ID 17.\n";
	output += "    The retail Pizza archive has no SCRB 17, so this normally has no visible result.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+R", kBuiltinDebugActionState);
	output += "    Set the private debug unlock state to 1 while it is 0-2; increment it after state 3.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+O", kBuiltinDebugActionPlace);
	output += "    Advance only private debug state 1 to 2.\n";
	output += Common::String::format("  %-7s (%s)\n", "Shift+D", kBuiltinDebugActionDeliver);
	output += "    Advance only private debug state 2 to 3.\n";
	output += "    P/p, Space, and N/W/S do nothing until Shift+R, Shift+O, Shift+D reach state 3.\n";
	output += Common::String::format("  %-7s (%s)\n", "P/p", kBuiltinDebugActionQuestion);
	output += "    Load question-reaction SCRBs for Arno and every present optional troll.\n";
	output += "    The debug path suppresses the completion narrator and does not enable Go.\n";
	output += Common::String::format("  %-7s (%s)\n", "Space", kBuiltinDebugActionResetDeliveries);
	output += "    Restore the initial remaining-delivery count only; toppings and troll states are unchanged.\n";
	output += Common::String::format("  %-7s (%s)\n", "N/n", kBuiltinDebugActionCycleArno);
	output += "    Load Arno SCRBs 8000-8035 in order, wrapping independently.\n";
	output += Common::String::format("  %-7s (%s)\n", "W/w", kBuiltinDebugActionCycleWilla);
	output += "    Load Willa SCRBs 9000-9034 in order on levels 2-4, wrapping independently.\n";
	output += Common::String::format("  %-7s (%s)\n", "S/s", kBuiltinDebugActionCycleShyler);
	output += "    Load Shyler SCRBs 10000-10038 in order on levels 3-4, wrapping independently.\n";
	output += "    Each cycle reloads the troll runner and rebuilds the topping-runner draw order.\n";
	return output;
}

bool ZoombiniPuzzlePizza::debugDoBuiltinDebugCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 3) {
		output = "Usage: page builtin_debug <Pizza action>\n";
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}

	const BuiltinDebugAction action = parseBuiltinDebugAction(argv[2]);
	if (action == BuiltinDebugAction::kInvalid) {
		_lastActivityFrame = _vm->getAnimationFrameCounter(_vm->_system->getMillis());
		output = Common::String::format("Unknown Pizza built-in debug action '%s'.\n", argv[2]);
		output += debugGetBuiltinDebugCommandHelp();
		return true;
	}
	return runBuiltinDebugAction(action, output);
}

ZoombiniPuzzlePizza::BuiltinDebugAction ZoombiniPuzzlePizza::parseBuiltinDebugAction(const Common::String &action) {
	if (action.equalsIgnoreCase(kBuiltinDebugActionToppings))
		return BuiltinDebugAction::kToppings;
	if (action.equalsIgnoreCase(kBuiltinDebugActionRegisterRunner))
		return BuiltinDebugAction::kRegisterRunner;
	if (action.equalsIgnoreCase(kBuiltinDebugActionState))
		return BuiltinDebugAction::kState;
	if (action.equalsIgnoreCase(kBuiltinDebugActionPlace))
		return BuiltinDebugAction::kPlace;
	if (action.equalsIgnoreCase(kBuiltinDebugActionDeliver))
		return BuiltinDebugAction::kDeliver;
	if (action.equalsIgnoreCase(kBuiltinDebugActionQuestion))
		return BuiltinDebugAction::kQuestion;
	if (action.equalsIgnoreCase(kBuiltinDebugActionResetDeliveries))
		return BuiltinDebugAction::kResetDeliveries;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleArno))
		return BuiltinDebugAction::kCycleArno;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleWilla))
		return BuiltinDebugAction::kCycleWilla;
	if (action.equalsIgnoreCase(kBuiltinDebugActionCycleShyler))
		return BuiltinDebugAction::kCycleShyler;
	return BuiltinDebugAction::kInvalid;
}

bool ZoombiniPuzzlePizza::runBuiltinDebugAction(BuiltinDebugAction action, Common::String &output) {
	_lastActivityFrame = _vm->getAnimationFrameCounter(_vm->_system->getMillis());

	if (action == BuiltinDebugAction::kToppings) {
		output = debugGetAnswer();
		const Common::String arno = Common::String::format("Arno    %d %d %d %d %d %d %d %d",
														   _arnoToppings[0], _arnoToppings[1], _arnoToppings[2], _arnoToppings[3],
														   _arnoToppings[4], _arnoToppings[5], _arnoToppings[6], _arnoToppings[7]);
		const Common::String willa = Common::String::format("Willa   %d %d %d %d %d %d %d %d",
															_willaToppings[0], _willaToppings[1], _willaToppings[2], _willaToppings[3],
															_willaToppings[4], _willaToppings[5], _willaToppings[6], _willaToppings[7]);
		const Common::String shyler = Common::String::format("Shyler  %d %d %d %d %d %d %d %d",
															 _shylerToppings[0], _shylerToppings[1], _shylerToppings[2], _shylerToppings[3],
															 _shylerToppings[4], _shylerToppings[5], _shylerToppings[6], _shylerToppings[7]);
		const Common::String meal = Common::String::format("Meal     %d %d %d %d %d %d %d %d",
														   _submittedMealToppings[0], _submittedMealToppings[1], _submittedMealToppings[2], _submittedMealToppings[3],
														   _submittedMealToppings[4], _submittedMealToppings[5], _submittedMealToppings[6], _submittedMealToppings[7]);

		drawBuiltinDebugPanel(Common::Rect(400, 1, 600, 80));
		if (_trollOrderStates[0] == TrollOrderState::kActive01)
			drawBuiltinDebugPanelText(arno, Common::Rect(400, 1, 600, 20));
		if (_trollOrderStates[1] == TrollOrderState::kActive01)
			drawBuiltinDebugPanelText(willa, Common::Rect(400, 21, 600, 40));
		if (_trollOrderStates[2] == TrollOrderState::kActive01)
			drawBuiltinDebugPanelText(shyler, Common::Rect(400, 41, 600, 60));
		drawBuiltinDebugPanelText(meal, Common::Rect(400, 61, 600, 80));
		return false;
	}
	if (action == BuiltinDebugAction::kRegisterRunner) {
		ZmbFeature *feature = loadVirtualFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Topping), _nextDynamicFeatureId, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00080000_DEFER_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE);
		// This branch retains the authored SCRB field without parsing that resource.
		const ZmbResource debugResource(ZmbResource::kPage, kResScrb0017_DebugRunner);
		feature->setScrbId(debugResource._id);
		_nextDynamicFeatureId += 1;
		output = "Registered Pizza debug runner with SCRB 17.\n";
		return false;
	}
	if (action == BuiltinDebugAction::kState) {
		if (_builtinDebugUnlockLevel <= 2)
			_builtinDebugUnlockLevel = 1;
		else
			_builtinDebugUnlockLevel += 1;
		output = Common::String::format("Pizza built-in debug state %d.\n", _builtinDebugUnlockLevel);
		return false;
	}
	if (action == BuiltinDebugAction::kPlace) {
		if (_builtinDebugUnlockLevel == 1)
			_builtinDebugUnlockLevel = 2;
		output = Common::String::format("Pizza built-in debug state %d.\n", _builtinDebugUnlockLevel);
		return false;
	}
	if (action == BuiltinDebugAction::kDeliver) {
		if (_builtinDebugUnlockLevel == 2)
			_builtinDebugUnlockLevel = 3;
		output = Common::String::format("Pizza built-in debug state %d.\n", _builtinDebugUnlockLevel);
		return false;
	}

	if (_builtinDebugUnlockLevel < 3) {
		output = Common::String::format("This Pizza action requires built-in debug state 3. Use %s, %s, then %s.\n",
										kBuiltinDebugActionState, kBuiltinDebugActionPlace, kBuiltinDebugActionDeliver);
		return true;
	}

	if (action == BuiltinDebugAction::kQuestion) {
		setupQuestionRunners();
		// The original debug handler discards the registered completion group immediately.
		// Keep the question reactions, but do not queue the normal solved-page narrator or enable Go.
		_arnoPhase = kPhaseNone;
		output = "Pizza solved-order reactions initialized without the narrator or Go unlock.\n";
		return false;
	}
	if (action == BuiltinDebugAction::kResetDeliveries) {
		_remainingMistakeAllowance = _initialMistakeAllowance;
		output = Common::String::format("Pizza mistake allowance reset to %d.\n", _remainingMistakeAllowance);
		return false;
	}

	int16 orderLine = -1;
	int16 scrbBase = 0;
	int16 scrbCount = 0;
	ZmbFeature *feature = nullptr;
	if (action == BuiltinDebugAction::kCycleArno) {
		orderLine = 0;
		scrbBase = kResScrb8000_ArnoOrderBase;
		scrbCount = 36;
		feature = _arnoFeature;
	} else if (action == BuiltinDebugAction::kCycleWilla) {
		orderLine = 1;
		scrbBase = kResScrb9000_WillaOrderBase;
		scrbCount = 35;
		feature = _willaFeature;
	} else if (action == BuiltinDebugAction::kCycleShyler) {
		orderLine = 2;
		scrbBase = kResScrb10000_ShylerOrderBase;
		scrbCount = 39;
		feature = _shylerFeature;
	} else {
		return true;
	}

	if (!feature) {
		output = "The selected Pizza troll runner is not present at this difficulty.\n";
		return true;
	}
	if (scrbCount <= _builtinDebugScrbCycle[orderLine])
		_builtinDebugScrbCycle[orderLine] = 0;
	const int16 scrbId = static_cast<int16>(scrbBase + _builtinDebugScrbCycle[orderLine]);
	_builtinDebugScrbCycle[orderLine] += 1;
	loadScrbOntoFeature(feature, scrbId);
	linkToppingRunners();
	output = Common::String::format("Pizza SCRB %d loaded on order runner %d.\n", scrbId, orderLine);
	return false;
}

ZmbEventHandleResult ZoombiniPuzzlePizza::onDebugKeyDown(const Common::KeyState &kbd) {
	if (!kbd.hasFlags(0) && !kbd.hasFlags(Common::KBD_SHIFT))
		return ZmbEventHandleResult::kPassthrough;

	BuiltinDebugAction action = BuiltinDebugAction::kInvalid;
	switch (kbd.ascii) {
	case 'A':
	case 'a':
		action = BuiltinDebugAction::kToppings;
		break;
	case 'B':
	case 'b':
		action = BuiltinDebugAction::kRegisterRunner;
		break;
	case 'D':
		action = BuiltinDebugAction::kDeliver;
		break;
	case 'N':
	case 'n':
		action = BuiltinDebugAction::kCycleArno;
		break;
	case 'O':
		action = BuiltinDebugAction::kPlace;
		break;
	case 'P':
	case 'p':
		action = BuiltinDebugAction::kQuestion;
		break;
	case 'R':
		action = BuiltinDebugAction::kState;
		break;
	case 'S':
	case 's':
		action = BuiltinDebugAction::kCycleShyler;
		break;
	case 'W':
	case 'w':
		action = BuiltinDebugAction::kCycleWilla;
		break;
	case ' ':
		action = BuiltinDebugAction::kResetDeliveries;
		break;
	default:
		return ZmbEventHandleResult::kPassthrough;
	}

	Common::String output;
	return runBuiltinDebugAction(action, output) ? ZmbEventHandleResult::kPassthrough : ZmbEventHandleResult::kConsumed;
}

bool ZoombiniPuzzlePizza::debugDoPageCommand(int argc, const char **argv, Common::String &output) {
	if (argc != 4 || scumm_stricmp(argv[1], "delivery-test") != 0 ||
		scumm_stricmp(argv[2], "--help") == 0 || scumm_stricmp(argv[2], "-h") == 0 ||
		scumm_stricmp(argv[3], "--help") == 0 || scumm_stricmp(argv[3], "-h") == 0) {
		output = "Prepare one isolated Pizza delivery using the generated order.\n";
		output += "Usage: page delivery-test <arno|willa|shyler> <partial|full|reject|reject-multi>\n";
		output += "  partial: select a strict subset of the requested toppings\n";
		output += "  full: select the complete requested topping set\n";
		output += "  reject: select one topping absent from the order\n";
		output += "  reject-multi: select two toppings absent from the order\n\n";
		output += "Run this only after the intro and before the first delivery.\n";
		output += "Close the debugger and click Produce to start the normal delivery chain.\n";
		output += "Options:\n";
		output += "  -h, --help  Show this help text and exit.\n";
		return true;
	}

	int16 orderLine = -1;
	if (scumm_stricmp(argv[2], "arno") == 0)
		orderLine = 0;
	else if (scumm_stricmp(argv[2], "willa") == 0)
		orderLine = 1;
	else if (scumm_stricmp(argv[2], "shyler") == 0)
		orderLine = 2;

	if (orderLine < 0) {
		output = Common::String::format("Unknown Pizza troll '%s'. Use arno, willa, or shyler.\n", argv[2]);
		return true;
	}
	if ((orderLine == 1 && _difficultyLevel < kPuzzleLevel2) ||
		(orderLine == 2 && _difficultyLevel < kPuzzleLevel3)) {
		output = Common::String::format("%s is not present at Pizza level %d.\n", argv[2], _difficultyLevel);
		return true;
	}

	if (!_pageActive || !_produceSubmissionEnabled ||
		!_postmanSnoid || _postmanSnoid->getAnimState() != kSnoidAnimState000_Idle ||
		_machinePhase != kPhaseNone || _overlayPhase != kPhaseNone || _arnoPhase != kPhaseNone || _willaPhase != kPhaseNone || _shylerPhase != kPhaseNone ||
		0 <= _lastToppingMaskHistoryIdx || _preserveAcceptedTrollLinkCount != 0 || _lostDelivererCount != 0) {
		output = "Pizza delivery-test requires the idle page after its intro and before the first delivery.\n";
		return true;
	}

	const bool *orderToppings = _arnoToppings;
	if (orderLine == 1)
		orderToppings = _willaToppings;
	else if (orderLine == 2)
		orderToppings = _shylerToppings;
	bool testToppings[8] = {false, false, false, false, false, false, false, false};
	int16 orderToppingCount = 0;
	int16 firstOrderTopping = -1;
	int16 extraToppingCount = 0;

	for (int16 i = 0; i < _machineToppingSlotCount; i++) {
		if (orderToppings[i]) {
			orderToppingCount += 1;
			if (firstOrderTopping < 0)
				firstOrderTopping = i;
		}
	}

	if (scumm_stricmp(argv[3], "full") == 0) {
		for (int16 i = 0; i < _machineToppingSlotCount; i++)
			testToppings[i] = orderToppings[i];
	} else if (scumm_stricmp(argv[3], "partial") == 0) {
		if (orderToppingCount < 1) {
			output = "The selected troll has no requested topping for a partial test.\n";
			return true;
		}
		if (1 < orderToppingCount)
			testToppings[firstOrderTopping] = 1;
	} else if (scumm_stricmp(argv[3], "reject") == 0 ||
			   scumm_stricmp(argv[3], "reject-multi") == 0) {
		const int16 requiredExtras = scumm_stricmp(argv[3], "reject-multi") == 0 ? 2 : 1;
		for (int16 i = 0; i < _machineToppingSlotCount && extraToppingCount < requiredExtras; i++) {
			const bool selectable = !(i == 4 && _difficultyLevel == kPuzzleLevel2) &&
									!(5 <= i && i <= 6 && _difficultyLevel < kPuzzleLevel2) &&
									!(i == 7 && _difficultyLevel < kPuzzleLevel4);
			if (selectable && !orderToppings[i]) {
				testToppings[i] = 1;
				extraToppingCount += 1;
			}
		}
		if (extraToppingCount < requiredExtras) {
			output = Common::String::format("The selected order has fewer than %d selectable extra toppings.\n", requiredExtras);
			return true;
		}
	} else {
		output = Common::String::format("Unknown Pizza result '%s'. Use partial, full, reject, or reject-multi.\n", argv[3]);
		return true;
	}

	_trollOrderStates[0] = TrollOrderState::kInactive00;
	_trollOrderStates[1] = TrollOrderState::kInactive00;
	_trollOrderStates[2] = TrollOrderState::kInactive00;
	_trollOrderStates[orderLine] = TrollOrderState::kActive01;
	_allTrollOrdersMatched = false;
	_delivererPoolExhausted = false;
	_acceptedOrdersThisMeal = 0;
	_pendingResultOrderSlot = 0;
	_toppingOwnerOrderSlot = 0;
	_toppingRunnerCategory = 0;
	_submittedMealAlreadyTried = false;
	_exactOrderMatchCount = 0;

	Common::String selectedSlots;
	for (int16 i = 0; i < _machineToppingSlotCount; i++) {
		_selectedMealToppings[i] = hasToppingButtonForIngredient(i) && testToppings[i];
		if (_toppingButtonFeatures[i]) {
			const int16 toppingScrbId = getToppingButtonScrbId(i, _selectedMealToppings[i]);
			loadScrbOntoFeature(_toppingButtonFeatures[i], toppingScrbId);
		}
		if (_selectedMealToppings[i]) {
			if (!selectedSlots.empty())
				selectedSlots += ",";
			selectedSlots += Common::String::format("%d", i);
		}
	}
	if (selectedSlots.empty())
		selectedSlots = "none";

	linkToppingRunners();
	registerProduceButton();
	output = Common::String::format("Pizza delivery test prepared: %s %s, topping slots %s. Close the debugger and click Produce.\n",
									argv[2], argv[3], selectedSlots.c_str());
	return false;
}

bool ZoombiniPuzzlePizza::debugSetChances(int16 remaining) {
	if (remaining < 0 || _initialMistakeAllowance < remaining)
		return false;

	_remainingMistakeAllowance = remaining;
	return true;
}

Common::String ZoombiniPuzzlePizza::debugGetAnswer() const {
	// [*] Ingredient identities keep the same mask index at every difficulty.
	// Level 1 uses indices 0-4.
	// Level 2 uses 0-6 but blocks index 4, leaving four pizza toppings and two sundae toppings.
	// Level 3 uses indices 0-6, and level 4 uses all indices 0-7.
	static constexpr SlotInfo kIngredientSlots[8] = {
		{"Pizza", 1, "Olive"},
		{"Pizza", 2, "Pepper"},
		{"Pizza", 3, "Salami"},
		{"Pizza", 4, "Mushroom"},
		{"Pizza", 5, "Cheese"},
		{"Sundae", 1, "Cherry"},
		{"Sundae", 2, "Cream"},
		{"Sundae", 3, "Chocolate"},
	};

	// Build categorized Pizza and Sundae lines from an inclusion mask.
	const auto buildTrollLine = [this](const bool toppingEnables[]) -> Common::String {
		Common::String line;
		bool hasPizza = false;
		bool hasSundae = false;
		for (int i = 0; i < _machineToppingSlotCount; i++) {
			if (!toppingEnables[i])
				continue;
			const SlotInfo &si = kIngredientSlots[i];
			if (Common::String(si.category) == "Pizza") {
				if (!hasPizza) {
					line += "    Pizza:\n";
					hasPizza = true;
				}
				line += Common::String::format("      %d: %s\n", si.number, si.name);
			} else {
				if (!hasSundae) {
					line += "    Sundae:\n";
					hasSundae = true;
				}
				line += Common::String::format("      %d: %s\n", si.number, si.name);
			}
		}
		if (!hasPizza && !hasSundae)
			line = "    (none)\n";
		return line;
	};

	Common::String s = getDebugBanner();
	s += "\n";
	// Trolls present by level: Arno (level 1+), Willamaen (level 2+), Shyler (level 3+).
	// Each troll must be satisfied to complete the puzzle.
	s += "  Arno:\n" + buildTrollLine(_arnoToppings);
	if (kPuzzleLevel2 <= _difficultyLevel)
		s += "  Willamaen:\n" + buildTrollLine(_willaToppings);
	if (kPuzzleLevel3 <= _difficultyLevel)
		s += "  Shyler:\n" + buildTrollLine(_shylerToppings);

	// "Not Wanted": slots that are selectable by the player but no troll wants them.
	// Cheese is blocked at level 2, so its fixed slot is excluded.
	bool toppingEnables[8] = {false, false, false, false, false, false, false, false};
	const int forbiddenSlot = (_difficultyLevel == kPuzzleLevel2) ? kMealIngredientCheese04 : -1;
	for (int i = 0; i < _machineToppingSlotCount; i++) {
		if (i != forbiddenSlot && !_arnoToppings[i] && !_willaToppings[i] && !_shylerToppings[i])
			toppingEnables[i] = true;
	}
	s += "  Not wanted:\n" + buildTrollLine(toppingEnables);

	return s;
}

// ---------------------------------------------------------------------------
// Generate the topping set.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::generateToppingSet() {
	for (int16 i = 0; i < ARRAYSIZE(_generatedToppings); i++)
		_generatedToppings[i] = false;

	// At level 2, forbid topping slot 4
	int16 forbiddenSlot = (_difficultyLevel == kPuzzleLevel2) ? 4 : -1;

	int16 remaining = _minimumGeneratedToppingCount;
	bool nonePlaced = true;

	do {
		for (int16 i = 0; i < _machineToppingSlotCount; i++) {
			if (_vm->_rnd->getRandomNumber(1000) < _toppingGenerationThreshold) {
				if (!_generatedToppings[i] && i != forbiddenSlot) {
					_generatedToppings[i] = true;
					remaining -= 1;
					nonePlaced = false;
				}
			}
		}
	} while (0 < remaining);

	if (nonePlaced) {
		int16 slot = _vm->_rnd->getRandomNumber(3);
		_generatedToppings[slot] = true;
	}
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: generated toppings for %d slots with a minimum of %d", _machineToppingSlotCount, _minimumGeneratedToppingCount);
}

// ---------------------------------------------------------------------------
// Distribute toppings among the ingredient categories.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::distributeToppings() {
	for (int16 i = 0; i < ARRAYSIZE(_arnoToppings); i++) {
		_arnoToppings[i] = false;
		_willaToppings[i] = false;
		_shylerToppings[i] = false;
	}

	generateToppingSet();

	if (_difficultyLevel == kPuzzleLevel1) {
		// Level 1 assigns every generated topping to Arno.
		for (int16 i = 0; i < _machineToppingSlotCount; i++) {
			_arnoToppings[i] = _generatedToppings[i];
		}
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: assigned level 1 generated toppings to Arno");
		return;
	}

	int16 arnoToppingCount = 0;
	int16 willaToppingCount = 0;
	int16 shylerToppingCount = 0;

	if (_difficultyLevel == kPuzzleLevel2) {
		// Level 2 assigns each generated topping to Arno or Willa with equal probability.
		for (int16 i = 0; i < _machineToppingSlotCount; i++) {
			if (_generatedToppings[i]) {
				if (!_vm->_rnd->getRandomBool()) {
					_arnoToppings[i] = true;
					arnoToppingCount += 1;
				} else {
					_willaToppings[i] = true;
					willaToppingCount += 1;
				}
			}
		}

		// Ensure at least one is assigned
		if (arnoToppingCount == 0 && willaToppingCount == 0) {
			int16 slot = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
			if (500 <= _vm->_rnd->getRandomNumber(1000)) {
				_arnoToppings[slot] = true;
			} else {
				_willaToppings[slot] = true;
			}
		}
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: assigned level 2 toppings; Arno=%d, Willa=%d", arnoToppingCount, willaToppingCount);
		return;
	}

	// Levels 3-4 distribute generated toppings among all three trolls.
	for (int16 i = 0; i < _machineToppingSlotCount; i++) {
		if (_generatedToppings[i]) {
			int16 category = _vm->_rnd->getRandomNumber(0, 2);
			switch (category) {
			case 0:
				_arnoToppings[i] = true;
				arnoToppingCount += 1;
				break;
			case 1:
				_willaToppings[i] = true;
				willaToppingCount += 1;
				break;
			default:
				_shylerToppings[i] = true;
				shylerToppingCount += 1;
				break;
			}
		}
	}

	// Rebalancing: one pass can repair all empty categories, then repeats only
	// while the first two categories still need a donor transfer.
	do {
		if (arnoToppingCount == 0) {
			int16 slot;
			if (willaToppingCount <= shylerToppingCount) {
				do {
					slot = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
				} while (!_shylerToppings[slot]);
				_shylerToppings[slot] = false;
				shylerToppingCount -= 1;
			} else {
				do {
					slot = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
				} while (!_willaToppings[slot]);
				_willaToppings[slot] = false;
				willaToppingCount -= 1;
			}
			_arnoToppings[slot] = true;
			arnoToppingCount = 1;
		}

		if (willaToppingCount == 0) {
			int16 slot;
			if (arnoToppingCount <= shylerToppingCount) {
				do {
					slot = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
				} while (!_shylerToppings[slot]);
				_shylerToppings[slot] = false;
				shylerToppingCount -= 1;
			} else {
				do {
					slot = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
				} while (!_arnoToppings[slot]);
				_arnoToppings[slot] = false;
				arnoToppingCount -= 1;
			}
			_willaToppings[slot] = true;
			willaToppingCount = 1;
		}

		if (shylerToppingCount == 0) {
			int16 slot;
			if (arnoToppingCount <= willaToppingCount) {
				do {
					slot = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
				} while (!_willaToppings[slot]);
				_willaToppings[slot] = false;
				willaToppingCount -= 1;
			} else {
				do {
					slot = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
				} while (!_arnoToppings[slot]);
				_arnoToppings[slot] = false;
				arnoToppingCount -= 1;
			}
			_shylerToppings[slot] = true;
			shylerToppingCount = 1;
		}
	} while (arnoToppingCount == 0 || willaToppingCount == 0);
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: assigned level %d toppings; Arno=%d, Willa=%d, Shyler=%d", _difficultyLevel,
		   arnoToppingCount, willaToppingCount, shylerToppingCount);

	// At maximum difficulty, create the four reject examples already present in the pit when the page opens.
	// Pick two toppings from the largest order and one from each remaining order, then cross them into four distinct pairs.
	if (_difficultyLevel == kPuzzleLevel4) {
		TrollOrderLine dominantOrderLine = TrollOrderLine::kArno00;
		if (willaToppingCount <= arnoToppingCount) {
			if (arnoToppingCount < shylerToppingCount)
				dominantOrderLine = TrollOrderLine::kShyler02;
		} else {
			dominantOrderLine = TrollOrderLine::kWilla01;
			if (willaToppingCount < shylerToppingCount)
				dominantOrderLine = TrollOrderLine::kShyler02;
		}

		const int16 dominantOrderToppingA = pickRandomToppingFromOrderLine(dominantOrderLine);
		int16 dominantOrderToppingB;
		do {
			dominantOrderToppingB = pickRandomToppingFromOrderLine(dominantOrderLine);
		} while (dominantOrderToppingB == dominantOrderToppingA);

		TrollOrderLine otherOrderLineA = TrollOrderLine::kWilla01;
		TrollOrderLine otherOrderLineB = TrollOrderLine::kShyler02;
		switch (dominantOrderLine) {
		case TrollOrderLine::kArno00:
			break;
		case TrollOrderLine::kWilla01:
			otherOrderLineA = TrollOrderLine::kArno00;
			break;
		case TrollOrderLine::kShyler02:
			otherOrderLineA = TrollOrderLine::kArno00;
			otherOrderLineB = TrollOrderLine::kWilla01;
			break;
		}
		const int16 otherOrderToppingA = pickRandomToppingFromOrderLine(otherOrderLineA);
		const int16 otherOrderToppingB = pickRandomToppingFromOrderLine(otherOrderLineB);

		createLevel4RejectExamples(dominantOrderToppingA, dominantOrderToppingB, otherOrderToppingA, otherOrderToppingB);
	}
}

// ---------------------------------------------------------------------------
// Pick a random topping from a troll order line.
// Spins random indices until one with an enabled entry is found.
// ---------------------------------------------------------------------------
int16 ZoombiniPuzzlePizza::pickRandomToppingFromOrderLine(TrollOrderLine orderLine) {
	const bool *toppingTable = _arnoToppings;
	switch (orderLine) {
	case TrollOrderLine::kArno00:
		break;
	case TrollOrderLine::kWilla01:
		toppingTable = _willaToppings;
		break;
	case TrollOrderLine::kShyler02:
		toppingTable = _shylerToppings;
		break;
	}

	while (true) {
		int16 toppingIdx = _vm->_rnd->getRandomNumber(0, _machineToppingSlotCount - 1);
		if (toppingTable[toppingIdx])
			return toppingIdx;
	}
}

// ---------------------------------------------------------------------------
// Create the four level-4 reject examples.
// Page setup registers and renders these persistent two-topping runners so all four are already visible in the reject pit when the page opens.
// Their masks are the four pairings between two toppings from the largest troll order and one topping selected from each remaining order.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::createLevel4RejectExamples(int16 dominantOrderToppingA, int16 dominantOrderToppingB, int16 otherOrderToppingA,
													 int16 otherOrderToppingB) {
	const int16 rejectExampleToppings[4][2] = {
		{dominantOrderToppingA, otherOrderToppingA},
		{dominantOrderToppingB, otherOrderToppingA},
		{dominantOrderToppingB, otherOrderToppingB},
		{dominantOrderToppingA, otherOrderToppingB},
	};

	const int16 savedRunnerCategory = _toppingRunnerCategory;
	_toppingRunnerCategory = 4; // Select the generic path in @ref ZoombiniPuzzlePizza::registerToppingRunner().

	static constexpr uint32 kRejectExampleFrameIntervals[4] = {6, 6, 6, 0};

	for (int exampleIdx = 0; exampleIdx < 4; exampleIdx++) {
		// Build and retain the two-topping mask for this reject example.
		for (int16 mealIdx = 0; mealIdx < ARRAYSIZE(_submittedMealToppings); mealIdx++)
			_submittedMealToppings[mealIdx] = false;
		const int16 firstToppingIdx = rejectExampleToppings[exampleIdx][0];
		const int16 secondToppingIdx = rejectExampleToppings[exampleIdx][1];
		_submittedMealToppings[firstToppingIdx] = true;
		_submittedMealToppings[secondToppingIdx] = true;

		_lastToppingMaskHistoryIdx += 1;
		_toppingMaskHistory[_lastToppingMaskHistoryIdx] = packToppingBitmask();

		_genericToppingRunnerCycle += 1;
		int16 visualScrbId = static_cast<int16>(kResScrb12041_ToppingRunnerBase + _genericToppingRunnerCycle);
		int16 slotScrbId = static_cast<int16>(kResScrb12025_GenericToppingBase + _genericToppingRunnerCycle);

		_lastToppingRunnerSlotIdx += 1;
		ToppingRunnerSlot &slot = _toppingRunnerSlots[_lastToppingRunnerSlotIdx];
		slot.mask = _toppingMaskHistory[_lastToppingMaskHistoryIdx];
		slot.scrbId = slotScrbId;
		slot.feature = createToppingRunnerFeature(visualScrbId, kRejectExampleFrameIntervals[exampleIdx]);
		linkToppingRunners();
		renderFeatures();
	}

	// Clear the scratch mask and restore the runner category used by normal deliveries.
	for (int16 i = 0; i < ARRAYSIZE(_submittedMealToppings); i++)
		_submittedMealToppings[i] = false;
	_toppingRunnerCategory = savedRunnerCategory;
}

// ---------------------------------------------------------------------------
// Update hover feedback on every host frame.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::onEveryFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	updateV2MachineButtonHover();
}

// ---------------------------------------------------------------------------
// Advance the Pizza controller after the current render.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::onPostRenderFrame() {
	if (_isUpdating || !_pageActive)
		return;
	PuzzleUpdateGuard updateGuard(_isUpdating);

	const bool orderFeatureCompletedThisFrame = _trollFeatureCompletedThisFrame;
	_trollFeatureCompletedThisFrame = false;

	// Pending Go departure -- skip normal frame logic
	if (isDeparturePending()) {
		return;
	}

	const uint32 now = getCurrentFrameCounter();
	if (3600 < now - _lastActivityFrame) {
		_lastActivityFrame = now;
		if (!orderFeatureCompletedThisFrame && !_vm->hasDialogOpened())
			runOrderFeatureAmbientIdleDriver();
	}

	// Classification waits for both the Postman and delivery overlay to finish.
	// @ref ZoombiniPuzzlePizza::_toppingOverlayCompletionPending is raised when the overlay completes.
	// Hold classification while the deliverer is still playing its carry SCRS.
	if (_toppingOverlayCompletionPending) {
		SnoidAnimState carrySt = _postmanSnoid ? _postmanSnoid->getAnimState() : kSnoidAnimState000_Idle;
		if (carrySt != kSnoidAnimState009_ScriptNormal && carrySt != kSnoidAnimState008_ScriptReject) {
			_toppingOverlayCompletionPending = false;
			onToppingDelivered();
		}
	}

	// Once the deliverer's post-reaction turn (anim 1) finished, walk it back to the answer seat (anim 7, target).
	if (_delivererReturnPending && _postmanSnoid &&
		_postmanSnoid->getAnimState() == kSnoidAnimState000_Idle) {
		_delivererReturnPending = false;
		if (_postmanSnoid->getPointLoc() != kMachineSeatPosition)
			_postmanSnoid->initWalkToTarget(kMachineSeatPosition);
	}

	// Restore the normal tick rate after the deliverer arrives.
	// After a lost deliverer, also release the produce lock raised
	// by @ref ZoombiniPuzzlePizza::advanceToNextDeliverySlot().
	if (_delivererWalkInPending && _postmanSnoid &&
		_postmanSnoid->getAnimState() == kSnoidAnimState000_Idle) {
		_delivererWalkInPending = false;
		_postmanSnoid->setFrameInterval(6);
		if (_lostDelivererCount)
			_produceSubmissionEnabled = true;
	}

	// The ready-flash SCRB 7067/7068 finished -- allow another Produce submission and,
	// when the previous delivery lost its deliverer, advance to the next one.
	// PLAY_ONCE deactivates the runner's render at end-of-cycle, which is exactly the liveness condition polled here.
	if (_produceButtonPhase == kPhaseSpawnAnswer && _produceButtonFeature &&
		!_produceButtonFeature->isRenderActivated()) {
		_produceButtonPhase = kPhaseNone;
		_produceSubmissionEnabled = true;
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "pizza: enabled Produce submission after ready flash");
		// @ref ZoombiniPuzzlePizza::advanceToNextDeliverySlot() consumes @ref ZoombiniPuzzlePizza::_nextDelivererRequired.
		if (_nextDelivererRequired)
			advanceToNextDeliverySlot();
	}

	// Celebration scheduling (hoorah fidget).
	// Every accepted order arms the driver only while at least five Snoids survive.
	const int16 survivingSnoids = getPackSnoidCount() - _lostDelivererCount;
	if (_pendingCelebrationBatchCount && survivingSnoids < 5)
		_pendingCelebrationBatchCount = 0;

	if (_pendingCelebrationBatchCount && _celebrationsPlayed < _celebrationTarget) {
		const uint32 celebrationFrame = getCurrentFrameCounter();
		if (_lastCelebrationFrame + 30 < celebrationFrame) {
			_lastCelebrationFrame = celebrationFrame;

			// Advance the stable full-pack pool until one eligible runner is found.
			// Rejected, busy, and current-Postman entries still consume pool positions.
			bool started = false;
			while (!started) {
				const uint16 poolIdx = _vm->_rnd->getNonRepeatRandom(static_cast<uint16>(getPackSnoidCount()), _celebrationRandomPool);
				ZmbSnoid *snoid = _snoidMap.find(static_cast<uint16>(10000 + poolIdx));
				if (!snoid || snoid == _postmanSnoid || !snoid->isPackSnoid() ||
					!snoid->isRenderActivated() || snoid->getAnimState() != kSnoidAnimState000_Idle ||
					snoid->_trait._feet == 0)
					continue;

				const int16 scrsId = static_cast<int16>(kResScrs13035_CelebrationBase + snoid->_trait._feet - 1);
				started = startSnoidScrs(snoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle);
				if (started)
					_celebrationsPlayed += 1;
			}
		}
	}

	// Reset the batch state as soon as the target number of animations has started.
	// The final SCRS may continue after this point.
	if (0 < _celebrationTarget && _celebrationTarget <= _celebrationsPlayed) {
		_celebrationRandomPool = 0;
		_lastCelebrationFrame = 0;
		_pendingCelebrationBatchCount = 0;
		_celebrationsPlayed = 0;
	}
}

// ---------------------------------------------------------------------------
// Dispatch animation events by feature identity and phase.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) {
	// --- Pizza-machine events (SCRB 7066 production callback chain) ---
	if (feature == _machineFeature) {
		if (_machinePhase == kPhaseExitCallback) {
			handleZmbExitEvent(feature, eventCode);
		}
		return;
	}

	// --- Order base feature events (Arno) ---
	if (feature == _arnoFeature) {
		if (eventCode == kAnimEventM1_End && _arnoPhase != kPhaseNone)
			_trollFeatureCompletedThisFrame = true;
		const bool receivesDeliveryEvents = _arnoPhase == kPhaseDeliveryEval ||
											(_arnoPhase == kPhaseServeReaction && _finalSafeAttemptReplayPending);
		if (receivesDeliveryEvents && eventCode != kAnimEventM1_End) {
			handleZmbDeliveryEvent(feature, eventCode);
			return;
		}
		if (eventCode == kAnimEventM1_End) {
			switch (_arnoPhase) {
			case kPhaseIntro:
				_arnoPhase = kPhaseNone;
				advanceIntroSequence();
				break;
			case kPhasePostIntroAmbient:
				// The post-intro ambient completion starts draw-on-reg ready SCRB 7067 or 7068.
				// Slot 40 reports the SCRB completion.
				// @ref ZoombiniPuzzlePizza::onEveryFrame() then enables the next submission through the ready-flash poll.
				// The first post-intro order reaction follows the road detour.
				// Every later Postman takes a straight path.
				_arnoPhase = kPhaseNone;
				setSnoidNodePathDisabled(true);
				spawnPostmanSnoid();
				if (_produceButtonPhase != kPhaseSpawnAnswer) {
					_produceButtonPhase = kPhaseNone;
					_produceSubmissionEnabled = true;
				}
				break;
			case kPhaseServeReaction:
				_arnoPhase = kPhaseNone;
				if (_finalSafeAttemptReplayPending) {
					// The final-chance replay shares this runner's completion event with
					// the delivery callback. Complete the Postman's return before loading
					// the troll's delivery-result SCRB.
					_finalSafeAttemptReplayPending = false;
					handleZmbDeliveryEvent(feature, kAnimEventM1_End);
				} else {
					handleOrderLineComplete(0);
				}
				break;
			case kPhaseDeliveryEval:
				_arnoPhase = kPhaseNone;
				handleZmbDeliveryEvent(feature, kAnimEventM1_End);
				break;
			case kPhaseDeliveryResult:
				// Advance to the next deliverer later, when the ready-flash completes in slot 40.
				// The pit or stone runner then triggers @ref ZoombiniPuzzlePizza::spawnPostmanSnoid().
				_arnoPhase = kPhaseNone;
				registerToppingRunner();
				break;
			case kPhaseQuestionSetup: {
				// Play narrator SND 20045-20048 when some Snoids were lost.
				// Play narrator SND 20055-20063 when none were lost.
				_arnoPhase = kPhaseNone;
				int16 sndId;
				if (_lostDelivererCount)
					sndId = selectPartialSuccessNarratorSoundId();
				else
					sndId = selectCompletionNarratorSoundId();
				queueNarratorSound(sndId);
				break;
			}
			default:
				break;
			}
		}
		return;
	}

	// --- Order 1 feature events (Willa) ---
	if (feature == _willaFeature) {
		if (eventCode == kAnimEventM1_End && _willaPhase != kPhaseNone)
			_trollFeatureCompletedThisFrame = true;
		if (_willaPhase == kPhaseDeliveryResult &&
			eventCode == kWillaResultEventCode099_RelinkBehindArno) {
			handleZmbExitEvent(feature, eventCode);
			return;
		}
		if (_willaPhase == kPhaseDeliveryEval && eventCode != kAnimEventM1_End) {
			handleZmbDeliveryEvent(feature, eventCode);
			return;
		}
		if (eventCode == kAnimEventM1_End) {
			switch (_willaPhase) {
			case kPhaseIntro:
				_willaPhase = kPhaseNone;
				advanceIntroSequence();
				break;
			case kPhasePostIntroAmbient:
				_willaPhase = kPhaseNone;
				setSnoidNodePathDisabled(true); // Raise
				spawnPostmanSnoid();
				if (_produceButtonPhase != kPhaseSpawnAnswer) {
					_produceButtonPhase = kPhaseNone;
					_produceSubmissionEnabled = true;
				}
				break;
			case kPhaseServeReaction:
				_willaPhase = kPhaseNone;
				handleOrderLineComplete(1);
				break;
			case kPhaseDeliveryEval:
				_willaPhase = kPhaseNone;
				handleZmbDeliveryEvent(feature, kAnimEventM1_End);
				break;
			case kPhaseDeliveryResult:
				// Advance after the ready-flash completion.
				_willaPhase = kPhaseNone;
				registerToppingRunner();
				break;
			default:
				break;
			}
		}
		return;
	}

	// --- Order 2 feature events (Shyler) ---
	if (feature == _shylerFeature) {
		if (eventCode == kAnimEventM1_End && _shylerPhase != kPhaseNone)
			_trollFeatureCompletedThisFrame = true;
		if (_shylerPhase == kPhaseDeliveryEval && eventCode != kAnimEventM1_End) {
			handleZmbDeliveryEvent(feature, eventCode);
			return;
		}
		if (eventCode == kAnimEventM1_End) {
			switch (_shylerPhase) {
			case kPhaseIntro:
				_shylerPhase = kPhaseNone;
				advanceIntroSequence();
				break;
			case kPhasePostIntroAmbient:
				_shylerPhase = kPhaseNone;
				setSnoidNodePathDisabled(true); // Raise
				spawnPostmanSnoid();
				if (_produceButtonPhase != kPhaseSpawnAnswer) {
					_produceButtonPhase = kPhaseNone;
					_produceSubmissionEnabled = true;
				}
				break;
			case kPhaseServeReaction:
				_shylerPhase = kPhaseNone;
				handleOrderLineComplete(2);
				break;
			case kPhaseDeliveryEval:
				_shylerPhase = kPhaseNone;
				handleZmbDeliveryEvent(feature, kAnimEventM1_End);
				break;
			case kPhaseDeliveryResult:
				// Advance after the ready-flash completion.
				_shylerPhase = kPhaseNone;
				registerToppingRunner();
				break;
			default:
				break;
			}
		}
		return;
	}

	// --- Topping overlay events ---
	if (feature == _toppingOverlayFeature) {
		if (eventCode == kAnimEventM1_End) {
			if (feature->getScriptSoundPolicy() ==
				ZmbFeature::ScriptSoundPolicy::kPriorityQueue) {
				feature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kInheritPage);
			}
			if (_overlayPhase == kPhaseToppingDelivery) {
				// Defer classification until the carry SCRS also finishes.
				_overlayPhase = kPhaseNone;
				_toppingOverlayCompletionPending = true;
			} else if (_overlayPhase == kPhaseToppingOverlay) {
				_overlayPhase = kPhaseNone;
				// Initial overlay done -- this is handled via exit callback
			}
		}
		return;
	}

	// --- Topping runner (pit/stone pizza) events ---
	// When a thrown or placed pizza settles, its runner queues ready-flash SCRB 7067 or 7068.
	for (int16 i = 0; i <= _lastToppingRunnerSlotIdx && i < 28; i++) {
		if (_toppingRunnerSlots[i].feature == feature) {
			if (eventCode == kAnimEventM1_End) {
				if (_toppingRunnerSlots[i].spawnOnComplete) {
					// Put every accepted troll behind the first persistent pizza before
					// the ready flash advances the delivery chain.
					ZmbFeature *firstToppingRunner = _toppingRunnerSlots[0].feature;
					if (firstToppingRunner) {
						if (_trollOrderStates[0] == TrollOrderState::kAccepted03)
							manualLinkBefore(_arnoFeature, firstToppingRunner);
						if (_trollOrderStates[1] == TrollOrderState::kAccepted03)
							manualLinkBefore(_willaFeature, firstToppingRunner);
						if (_trollOrderStates[2] == TrollOrderState::kAccepted03)
							manualLinkBefore(_shylerFeature, firstToppingRunner);
					}
					spawnPostmanSnoid();
				}
			}
			return;
		}
	}

	// --- Topping feature events ---
	for (int16 i = 0; i < _machineToppingSlotCount; i++) {
		if (feature == _toppingButtonFeatures[i]) {
			return;
		}
	}

	// --- Snoid events ---
	if (feature->hasFlag(ZmbFeature::FLAG_00000001_TYPE_SNOID)) {
		ZmbSnoid *snoid = static_cast<ZmbSnoid *>(feature);
		if (handleBodyArrangementScriptEvent(*snoid, eventCode)) {
			snoid->setNeedsRedraw(true);
		} else if (eventCode == kPostmanEventCode000_ToggleFacing) {
			// Event 0 toggles the runner's facing-left state, not its render-enabled state.
			// The delivery callback is reinstalled when the Postman starts reaction SCRS 14000-14005.
			// These flips steer the Postman's mirror direction during the reaction animation.
			snoid->setFacingLeft(!snoid->isFacingLeft());
			applyPendingBodyArrangement(*snoid);
			snoid->setNeedsRedraw(true);
		} else if (eventCode == kAnimEventM1_End) {
			SnoidAnimState state = snoid->getAnimState();
			if (state == kSnoidAnimState009_ScriptNormal || state == kSnoidAnimState008_ScriptReject) {
				snoid->setAnimState(kSnoidAnimState000_Idle);
				snoid->setupIdleHotspots();
			}
		}
		return;
	}

	// --- Produce-button ready flash ---
	// Re-enable @ref ZoombiniPuzzlePizza::_produceSubmissionEnabled when the ready-flash SCRB 7067 or 7068 finishes.
	// If the delivery callback requested a slot advance, pick the next deliverer.
	// @ref ZoombiniPuzzlePizza::onEveryFrame() polls the same runner; whichever path fires first resets the phase.
	if (feature == _produceButtonFeature) {
		if (eventCode == kAnimEventM1_End && _produceButtonPhase == kPhaseSpawnAnswer) {
			_produceButtonPhase = kPhaseNone;
			_produceSubmissionEnabled = true;
			debugC(2, MohawkEngine_Zoombini::kDebugPage02, "pizza: enabled Produce submission after ready callback");
			if (_nextDelivererRequired)
				advanceToNextDeliverySlot();
		}
		return;
	}
}

// ---------------------------------------------------------------------------
// Handle a mouse-button press.
// ---------------------------------------------------------------------------
ZmbEventHandleResult ZoombiniPuzzlePizza::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	_lastActivityFrame = _vm->getAnimationFrameCounter(_vm->_system->getMillis());
	finishDisposalResultOnInput();

	// Let base class handle Go/Map/Help buttons
	ZmbEventHandleResult result = ZoombiniInteractive::onLButtonDown(absPos, relPos);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	// Apply the shared puzzle-active and drag guards before case-specific input.
	if (!_pageActive)
		return ZmbEventHandleResult::kPassthrough;
	if (isDragging())
		return ZmbEventHandleResult::kPassthrough;

	// --- Check ingredient toggle clicks (on topping features) ---
	// Toppings can change only when no exit callback is pending.
	if (_machinePhase == kPhaseNone) {
		for (int16 i = 0; i < _machineToppingSlotCount; i++) {
			if (isToppingButtonAtPoint(i, absPos)) {
				handleIngredientToggle(i);
				return ZmbEventHandleResult::kConsumed;
			}
		}
	}

	// --- Check the authored Produce button and machine-mouth click zones. ---
	// Require an idle delivery path, unresolved orders, no troll reaction, and an active answer-display runner.
	if (_produceButtonClickRect.contains(absPos) || _machineMouthClickRect.contains(absPos)) {
		// No order-line reaction may still be running when a new pizza is produced.
		if (_produceSubmissionEnabled && !_allTrollOrdersMatched && !_delivererPoolExhausted &&
			_arnoPhase == kPhaseNone && _willaPhase == kPhaseNone && _shylerPhase == kPhaseNone &&
			_produceClickZonesEnabled) {
			// If no deliverer picked yet (slot 14 == -1),
			// @ref ZoombiniPuzzlePizza::advanceToNextDeliverySlot() picks the first deliverer and walks it in.
			if (!_postmanSnoid) {
				advanceToNextDeliverySlot();
			}
			if (_postmanSnoid) {
				handleSubmit();
			}
			return ZmbEventHandleResult::kConsumed;
		}
	}

	// This is a guarded internal drop path, not free Snoid dragging.
	if (findSnoidAtPoint(absPos))
		return ZmbEventHandleResult::kPassthrough;

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniPuzzlePizza::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	_lastActivityFrame = _vm->getAnimationFrameCounter(_vm->_system->getMillis());
	return ZoombiniInteractive::onKeyDown(kbd, kbdRepeat);
}

void ZoombiniPuzzlePizza::finishDisposalResultOnInput() {
	ZmbFeature *orderFeature = nullptr;
	int16 scrbId = 0;

	if (_arnoPhase == kPhaseDeliveryResult) {
		orderFeature = _arnoFeature;
		scrbId = kResScrb8034_ArnoIdleBase + _vm->_rnd->getRandomNumber(1);
		_arnoPhase = kPhaseNone;
	} else if (_willaPhase == kPhaseDeliveryResult) {
		orderFeature = _willaFeature;
		scrbId = kResScrb9019_WillaIdleBase + _vm->_rnd->getRandomNumber(1);
		_willaPhase = kPhaseNone;
	} else if (_shylerPhase == kPhaseDeliveryResult) {
		orderFeature = _shylerFeature;
		scrbId = kResScrb10006_ShylerIdleBase + _vm->_rnd->getRandomNumber(2);
		_shylerPhase = kPhaseNone;
	}

	if (!orderFeature)
		return;

	loadScrbOntoFeature(orderFeature, scrbId);
	registerToppingRunner();
	_pendingResultOrderSlot = 0;
}

ZmbEventHandleResult ZoombiniPuzzlePizza::onMouseMove(const Common::Point &absPos, const Common::Point &relPos) {
	_lastMachineMousePos = absPos;
	updateV2MachineButtonHover();
	return ZoombiniInteractive::onMouseMove(absPos, relPos);
}

bool ZoombiniPuzzlePizza::hasToppingButtonForIngredient(int16 ingredientIdx) const {
	if (ingredientIdx < 0 || _machineToppingSlotCount <= ingredientIdx)
		return false;

	return _difficultyLevel != kPuzzleLevel2 || ingredientIdx != kMealIngredientCheese04;
}

int16 ZoombiniPuzzlePizza::getToppingButtonScrbId(int16 ingredientIdx, bool selected) const {
	if (!hasToppingButtonForIngredient(ingredientIdx))
		return -1;

	int16 buttonOrdinal = ingredientIdx;
	// Level 2 preserves the shared ingredient indices but packs six physical button pairs around the absent cheese slot.
	if (_difficultyLevel == kPuzzleLevel2 && kMealIngredientCheese04 < ingredientIdx)
		buttonOrdinal -= 1;

	return static_cast<int16>(kToppingScrbTable[_difficultyLevel - 1] + buttonOrdinal * 2 + (selected ? 1 : 0));
}

void ZoombiniPuzzlePizza::updateV2MachineButtonHover() {
	if (!_vm->isVersionFamilyTlcV2())
		return;

	const int16 produceNormalScrbId = static_cast<int16>(kResScrb7001_AnswerLevel1 + (_difficultyLevel - 1));
	const int16 produceHoverScrbId = static_cast<int16>(produceNormalScrbId + kButtonHoverScrbDelta);
	if (_produceButtonFeature && _produceSubmissionEnabled) {
		const bool hovered = _produceButtonClickRect.contains(_lastMachineMousePos);
		const int16 targetScrbId = hovered ? produceHoverScrbId : produceNormalScrbId;
		if (_produceButtonFeature->getScrbId() != targetScrbId) {
			loadScrbOntoFeature(_produceButtonFeature, targetScrbId);
		}
	}

	for (int16 i = 0; i < _machineToppingSlotCount; i++) {
		ZmbFeature *feature = _toppingButtonFeatures[i];
		if (!feature)
			continue;

		const int16 normalScrbId = getToppingButtonScrbId(i, _selectedMealToppings[i]);
		const int16 hoverScrbId = static_cast<int16>(normalScrbId + kButtonHoverScrbDelta);
		const bool hovered = isToppingButtonAtPoint(i, _lastMachineMousePos);
		const int16 targetScrbId = hovered ? hoverScrbId : normalScrbId;
		if (feature->getScrbId() != targetScrbId) {
			loadScrbOntoFeature(feature, targetScrbId);
		}
	}
}

bool ZoombiniPuzzlePizza::isToppingButtonAtPoint(int16 ingredientIdx, const Common::Point &point) const {
	if (_machineToppingSlotCount <= ingredientIdx || !hasToppingButtonForIngredient(ingredientIdx) || !_toppingButtonFeatures[ingredientIdx])
		return false;

	if (!_vm->isVersionFamilyTlcV2())
		return _toppingButtonFeatures[ingredientIdx]->findDrawRecordAtPoint(point) != nullptr;

	const int16 *rect = kToppingClickRectTable[_difficultyLevel - 1][ingredientIdx];
	return Common::Rect(rect[0], rect[1], rect[2], rect[3]).contains(point);
}

// ---------------------------------------------------------------------------
// Toggle a topping selection.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::handleIngredientToggle(int16 ingredientIdx) {
	if (!hasToppingButtonForIngredient(ingredientIdx))
		return;

	// Toggle the selected ingredient.
	_selectedMealToppings[ingredientIdx] = !_selectedMealToppings[ingredientIdx];

	// The toggle writes only the live selection.
	// It must not touch @ref ZoombiniPuzzlePizza::_submittedMealToppings, which snapshots an in-flight delivery.
	// Changing that snapshot mid-delivery reclassifies the running meal and leaves the delivery state locked.
	// @ref ZoombiniPuzzlePizza::handleSubmit() refreshes the snapshot when a new delivery starts.

	// Swap the topping SCRB to its off/on visual.
	const int16 targetScrb = getToppingButtonScrbId(ingredientIdx, _selectedMealToppings[ingredientIdx]);

	if (_toppingButtonFeatures[ingredientIdx]) {
		loadScrbOntoFeature(_toppingButtonFeatures[ingredientIdx], targetScrb);
	}
	// TLC v2 topping SCRBs omit the frame sound, so the button queues it separately.
	if (_vm->isVersionFamilyTlcV2()) {
		queueScriptSoundForNextRenderPass(ZmbResource(ZmbResource::kPage, kToppingButtonSoundTable[ingredientIdx]));
	}

	// Refresh the large preview button after toggling.
	registerProduceButton();
	debugC(4, MohawkEngine_Zoombini::kDebugPage02, "pizza: topping %d selection is now %d", ingredientIdx, _selectedMealToppings[ingredientIdx] ? 1 : 0);
}

// ---------------------------------------------------------------------------
// Classify the submitted meal against one troll order.
// Track selected extras, selected matches, and total toppings in the order.
// ---------------------------------------------------------------------------
ZoombiniPuzzlePizza::SubmittedMealClassification ZoombiniPuzzlePizza::classifySubmittedMeal(TrollOrderLine orderLine) const {
	const bool *orderArray = _arnoToppings;
	switch (orderLine) {
	case TrollOrderLine::kArno00:
		break;
	case TrollOrderLine::kWilla01:
		orderArray = _willaToppings;
		break;
	case TrollOrderLine::kShyler02:
		orderArray = _shylerToppings;
		break;
	}

	int16 nonMatching = 0; // Selected toppings absent from the order.
	int16 matching = 0;    // Selected toppings present in the order.
	int16 orderCount = 0;  // Total toppings requested by the order.

	for (int16 i = 0; i < _machineToppingSlotCount; i++) {
		if (orderArray[i])
			orderCount += 1;
		if (_submittedMealToppings[i]) {
			if (orderArray[i])
				matching += 1;
			else
				nonMatching += 1;
		}
	}

	// One non-matching extra is a full rejection.
	if (nonMatching == 1)
		return SubmittedMealClassification::kOneUnwantedTopping00;

	// Multiple non-matching extras select the alternate rejection.
	if (1 < nonMatching)
		return SubmittedMealClassification::kMultipleUnwantedToppings04;

	// With no extra toppings, selecting the complete order is an exact match.
	if (orderCount == matching)
		return SubmittedMealClassification::kExactMatch02;

	// A partial subset is a non-exact match.
	return SubmittedMealClassification::kMissingRequestedToppings01;
}

// ---------------------------------------------------------------------------
// Serve the next topping.
// Classify the current meal against the given order line and play the appropriate reaction animation.
// Sets @ref ZoombiniPuzzlePizza::_pendingResultOrderSlot for a non-exact match and @ref ZoombiniPuzzlePizza::_trollOrderStates
// for an exact match.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::serveNextTopping(TrollOrderLine orderLine) {
	_lastActivityFrame = getCurrentFrameCounter();
	const int16 orderLineIndex = static_cast<int16>(orderLine);

	if (_trollOrderStates[orderLineIndex] == TrollOrderState::kAccepted03)
		return;

	const SubmittedMealClassification classification = classifySubmittedMeal(orderLine);
	// TODO: Consider an opt-in experimental mode that explicitly substitutes kUnusedReaction03 after canonical classification.
	// Keep @ref ZoombiniPuzzlePizza::classifySubmittedMeal() unchanged: the original classifier returns only 0, 1, 2, or 4.
	// The retained consumers select Arno SCRB 8015-8016, Willa SCRB 9017-9018, or Shyler SCRB 10027-10029.
	// Any future trigger is new game design because the original case-3 producer and semantic meaning are unknown.
	ZmbFeature *orderFeature = nullptr;
	int16 scrbId = 0;
	FeaturePhase *phase = nullptr;
	bool setPendingDelivery = false;

	switch (orderLine) {
	case TrollOrderLine::kArno00:
		orderFeature = _arnoFeature;
		phase = &_arnoPhase;
		switch (classification) {
		case SubmittedMealClassification::kOneUnwantedTopping00:
			scrbId = kResScrb8006_ArnoOneCorrectBase + _arnoOneUnwantedAnimCycle;
			_arnoOneUnwantedAnimCycle = (_arnoOneUnwantedAnimCycle + 1) % 2;
			_exactOrderMatchCount = 0;
			_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kMissingRequestedToppings01:
			scrbId = kResScrb8000_ArnoOrderBase + _arnoMissingToppingAnimStep;
			if (_arnoMissingToppingAnimStep < 5)
				_arnoMissingToppingAnimStep += 1;
			if (!_toppingOwnerOrderSlot) {
				_toppingOwnerOrderSlot = 1;
				_toppingRunnerCategory = 5;
			}
			_exactOrderMatchCount = 0;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kExactMatch02:
			scrbId = kResScrb8017_ArnoReactionBase + _vm->_rnd->getRandomNumber(0, 2);
			_exactOrderMatchCount += 1;
			break;
		case SubmittedMealClassification::kUnusedReaction03:
			scrbId = kResScrb8015_ArnoRejectBase + _vm->_rnd->getRandomNumber(0, 1);
			_exactOrderMatchCount = 0;
			_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kMultipleUnwantedToppings04:
			scrbId = kResScrb8008_ArnoMultipleCorrectBase + _arnoMultipleUnwantedAnimCycle;
			_arnoMultipleUnwantedAnimCycle = (_arnoMultipleUnwantedAnimCycle + 1) % 6;
			_exactOrderMatchCount = 0;
			_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		}
		break;
	case TrollOrderLine::kWilla01:
		orderFeature = _willaFeature;
		phase = &_willaPhase;
		switch (classification) {
		case SubmittedMealClassification::kOneUnwantedTopping00:
			scrbId = kResScrb9000_WillaOrderBase + _willaOneUnwantedAnimCycle;
			_willaOneUnwantedAnimCycle = (_willaOneUnwantedAnimCycle + 1) % 5;
			_exactOrderMatchCount = 0;
			if (!_toppingOwnerOrderSlot)
				_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kMissingRequestedToppings01:
			scrbId = kResScrb9021_WillaOneCorrectBase + _willaMissingToppingAnimStep;
			if (_willaMissingToppingAnimStep < 4)
				_willaMissingToppingAnimStep += 1;
			if (!_toppingOwnerOrderSlot) {
				_toppingOwnerOrderSlot = 2;
				_toppingRunnerCategory = 6;
			}
			_exactOrderMatchCount = 0;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kExactMatch02:
			scrbId = kResScrb9010_WillaReactionBase + _vm->_rnd->getRandomNumber(0, 6);
			_exactOrderMatchCount += 1;
			break;
		case SubmittedMealClassification::kUnusedReaction03:
			scrbId = kResScrb9017_WillaRejectBase + _vm->_rnd->getRandomNumber(0, 1);
			_exactOrderMatchCount = 0;
			_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kMultipleUnwantedToppings04:
			scrbId = kResScrb9005_WillaMultipleCorrectBase + _willaMultipleUnwantedAnimCycle;
			_willaMultipleUnwantedAnimCycle = (_willaMultipleUnwantedAnimCycle + 1) % 5;
			_exactOrderMatchCount = 0;
			if (!_toppingOwnerOrderSlot)
				_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		}
		break;
	case TrollOrderLine::kShyler02:
		orderFeature = _shylerFeature;
		phase = &_shylerPhase;
		switch (classification) {
		case SubmittedMealClassification::kOneUnwantedTopping00:
			scrbId = kResScrb10014_ShylerOneCorrectBase + _shylerOneUnwantedAnimCycle;
			_shylerOneUnwantedAnimCycle = (_shylerOneUnwantedAnimCycle + 1) % 6;
			_exactOrderMatchCount = 0;
			if (!_toppingOwnerOrderSlot)
				_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kMissingRequestedToppings01:
			scrbId = kResScrb10009_ShylerMultipleCorrectBase + _shylerMissingToppingAnimStep;
			if (_shylerMissingToppingAnimStep < 4)
				_shylerMissingToppingAnimStep += 1;
			if (!_toppingOwnerOrderSlot) {
				_toppingOwnerOrderSlot = 3;
				_toppingRunnerCategory = 7;
			}
			_exactOrderMatchCount = 0;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kExactMatch02:
			scrbId = kResScrb10023_ShylerRejectBase + _vm->_rnd->getRandomNumber(0, 3);
			_exactOrderMatchCount += 1;
			break;
		case SubmittedMealClassification::kUnusedReaction03:
			scrbId = kResScrb10027_ShylerRejectBase + _vm->_rnd->getRandomNumber(0, 2);
			_exactOrderMatchCount = 0;
			_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		case SubmittedMealClassification::kMultipleUnwantedToppings04:
			scrbId = kResScrb10020_ShylerReactionBase + _shylerMultipleUnwantedAnimCycle;
			_shylerMultipleUnwantedAnimCycle = (_shylerMultipleUnwantedAnimCycle + 1) % 3;
			_exactOrderMatchCount = 0;
			if (!_toppingOwnerOrderSlot)
				_toppingRunnerCategory = 4;
			setPendingDelivery = true;
			break;
		}
		break;
	}

	if (orderFeature && scrbId) {
		loadScrbOntoFeature(orderFeature, scrbId);

		// Rebuild the shared runner chain immediately after replacing a reaction SCRB.
		// Arno's exact-match reaction is the only serve variant that preserves its current link.
		if (orderLine != TrollOrderLine::kArno00 || classification != SubmittedMealClassification::kExactMatch02)
			linkToppingRunners();
		if (classification == SubmittedMealClassification::kExactMatch02)
			_trollOrderStates[orderLineIndex] = TrollOrderState::kMatched02;

		if (phase)
			*phase = kPhaseServeReaction;

		// Every serve installs @ref ZoombiniPuzzlePizza::attachMealFilter() on the order runner.
		// The reaction SCRB then draws only the submitted meal's toppings on the inspected pizza.
		attachMealFilter(orderFeature);

		// Store the one-based order line for a pending non-exact delivery.
		if (setPendingDelivery)
			_pendingResultOrderSlot = orderLineIndex + 1;
		debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: served order %d with result %d; SCRB %d, pending delivery %d", orderLineIndex,
			   static_cast<int>(classification), scrbId, setPendingDelivery ? _pendingResultOrderSlot : 0);
	}

	// All orders are ready when every non-inactive order has reached matched or accepted state.
	bool allReady = true;
	for (int16 i = 0; i < 3; i++) {
		if (_trollOrderStates[i] == TrollOrderState::kActive01) {
			allReady = false;
			break;
		}
	}
	if (allReady) {
		_allTrollOrdersMatched = true;
		// Reserve one non-celebrating Snoid from the original loaded group.
		// Rejected runners remain in the random pool and can consume a retry slot.
		const int16 zmbCount = getPackSnoidCount();
		_celebrationTarget = (0 < zmbCount) ? (zmbCount - 1) : 0;
	}
}

// ---------------------------------------------------------------------------
// Evaluate a non-exact submission.
// Decrement the mistake allowance and determine whether the current deliverer
// survives. Then take the fast path or load the delivery-evaluation SCRB.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::evaluateDelivery() {
	_acceptedOrdersThisMeal = 0;

	_remainingMistakeAllowance -= 1;
	_delivererSurvivedAttempt = 0 <= _remainingMistakeAllowance;

	// Mark the exact attempt that consumes the last safe mistake.
	if (_remainingMistakeAllowance == 0)
		_finalSafeAttemptCounter += 1;
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: evaluating delivery; allowance=%d, survives=%d, final-safe count=%d", _remainingMistakeAllowance,
		   _delivererSurvivedAttempt ? 1 : 0, _finalSafeAttemptCounter);

	// A surviving attempt before the final safe boundary skips the evaluation animation.
	if (!_finalSafeAttemptCounter && _delivererSurvivedAttempt) {
		// Skip the delivery evaluation animation and load the result directly.
		animatePostman();
		_skipDeliveryScriptEvent = true;
		// Skip directly to the delivery-result SCRB.
		loadDeliveryResultScrb();
		return;
	}

	// Load the evaluation SCRB on the first active order feature.
	// Use the delivery callback for Postman SCRS playback.
	ZmbFeature *evalFeature = nullptr;
	int16 evalScrbId = 0;
	FeaturePhase *phase = nullptr;

	if (_trollOrderStates[0] == TrollOrderState::kActive01) {
		evalFeature = _arnoFeature;
		evalScrbId = kResScrb8022_ArnoDeliveryEvalBase + _delivererSurvivedAttempt;
		phase = &_arnoPhase;

		if (_finalSafeAttemptCounter) {
			// The pending replay still completes the delivery callback before loading the result SCRB.
			_finalSafeAttemptReplayPending = true;
			*phase = kPhaseServeReaction;
		} else {
			*phase = kPhaseDeliveryEval;
		}
	} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
		evalFeature = _willaFeature;
		evalScrbId = kResScrb9028_WillaDeliveryEvalBase + _delivererSurvivedAttempt;
		phase = &_willaPhase;
		*phase = kPhaseDeliveryEval;
	} else if (_trollOrderStates[2] == TrollOrderState::kActive01) {
		evalFeature = _shylerFeature;
		evalScrbId = kResScrb10032_ShylerDeliveryEvalBase + _delivererSurvivedAttempt;
		phase = &_shylerPhase;
		*phase = kPhaseDeliveryEval;
	}

	if (evalFeature && evalScrbId)
		loadScrbOntoFeature(evalFeature, evalScrbId);

	_skipDeliveryScriptEvent = false;
	_finalSafeAttemptCounter = 0;
}

// ---------------------------------------------------------------------------
// Load the delivery-result SCRB.
// Called after evaluation to load delivery result SCRBs (8020/9026/10030).
// These show the pizza being delivered to the troll.
// The skip path calls @ref ZoombiniPuzzlePizza::loadDeliveryResultScrb() directly.
// Otherwise, the evaluation SCRB completion calls it.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::loadDeliveryResultScrb() {
	// Release the serving-stone overlay before the disposal runner takes ownership of the pizza.
	hideToppingOverlay();

	// Advance the history here instead of in @ref ZoombiniPuzzlePizza::onToppingDelivered(),
	// or the first submission would match itself in @ref ZoombiniPuzzlePizza::checkToppingMaskMatch().
	_lastToppingMaskHistoryIdx += 1;
	if (_lastToppingMaskHistoryIdx < 28) {
		_toppingMaskHistory[_lastToppingMaskHistoryIdx] = packToppingBitmask();
	}

	// A retained topping runner owner overrides the pending result slot.
	if (_toppingOwnerOrderSlot)
		_pendingResultOrderSlot = _toppingOwnerOrderSlot;

	// Convert the troll order slot from 1-based to 0-based.
	_pendingResultOrderSlot -= 1;
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: loading delivery result for order slot %d", _pendingResultOrderSlot);

	ZmbFeature *orderFeature = nullptr;
	int16 scrbId = 0;

	if (_pendingResultOrderSlot <= 0) {
		// Arno handles this delivery with SCRB 8020.
		orderFeature = _arnoFeature;
		scrbId = kResScrb8020_ArnoDeliveryResult;
		_arnoPhase = kPhaseDeliveryResult;
	} else if (_pendingResultOrderSlot == 1) {
		// A pending replay skips this delivery slot and evaluates the next one.
		if (_finalSafeAttemptReplayPending) {
			_pendingResultOrderSlot += 1;
			evaluateDelivery();
			return;
		}
		orderFeature = _willaFeature;
		scrbId = kResScrb9026_WillaDeliveryResult;
		_willaPhase = kPhaseDeliveryResult;
	} else if (_pendingResultOrderSlot == 2) {
		// Shyler handles this delivery with SCRB 10030.
		orderFeature = _shylerFeature;
		scrbId = kResScrb10030_ShylerDeliveryResult;
		_shylerPhase = kPhaseDeliveryResult;
	}

	if (orderFeature && scrbId) {
		loadScrbOntoFeature(orderFeature, scrbId);
		if (_pendingResultOrderSlot == 1) {
			linkToppingRunners();
			// SCRB 9026 moves Willa in front of the serving stone.
			// The delivery overlay creation already placed every older pizza behind the stone.
			manualLinkAfter(orderFeature, _servingStoneFeature);
			if (_postmanSnoid)
				manualLinkAfter(_postmanSnoid, orderFeature);
		} else {
			// Draw the troll's disposal animation in front of the evaluation stone
			// so the carried pizza and its toppings remain visible.
			manualLinkAfter(orderFeature, _servingStoneFeature);
			// Arno keeps the Postman in front while safe mistakes remain.
			// Shyler's final result does not relink the Postman.
			if (_pendingResultOrderSlot <= 0 && 0 < _remainingMistakeAllowance && _postmanSnoid)
				manualLinkAfter(_postmanSnoid, orderFeature);
		}
		// Filter the disposal SCRB so its pizza shows the submitted toppings.
		attachMealFilter(orderFeature);

		// The result loads while this runner is already in the current pre-render pass.
		// Materialize frame zero immediately so the serving-stone pizza transfers
		// to the disposal runner without a blank frame.
		prepareFeatureVisualCoverage(orderFeature);
		markPreparedFeatureVisualCoverageDirty(orderFeature, true);
	}

	_pendingResultOrderSlot = 0;
}

// ---------------------------------------------------------------------------
// Advance to the next deliverer.
// Pick the next Zoombini from pack order and walk it to the machine seat.
// This method runs only for the first pick, whose index is -1 after the first produce click,
// or after a lost deliverer sets @ref ZoombiniPuzzlePizza::_nextDelivererRequired.
// Preserve the live selection, submitted-meal snapshot, and topping-mask history.
// The player's selection persists, and the history accumulates for repeat detection.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::advanceToNextDeliverySlot() {
	// Guards
	if (_delivererPoolExhausted || _allTrollOrdersMatched)
		return;
	if (_delivererSequenceIdx != -1 && !_nextDelivererRequired)
		return;
	_nextDelivererRequired = false;

	// Move to the next pack sequence entry.
	// Only the solved path in @ref ZoombiniPuzzlePizza::handleOrderLineComplete() unlocks departure.
	_delivererSequenceIdx += 1;

	// Select only pack Zoombinis, not SCRS-pool Snoids that share @ref ZoombiniPage::_snoidMap.
	int16 packSnoidIdx = 0;
	ZmbSnoid *picked = nullptr;
	for (ZmbFeatureList<ZmbSnoid>::const_iterator it = _snoidMap.begin(); it != _snoidMap.end(); it++) {
		ZmbSnoid *s = *it;
		if (!s->isPackSnoid())
			continue;
		if (packSnoidIdx == _delivererSequenceIdx) {
			picked = s;
			break;
		}
		packSnoidIdx += 1;
	}

	if (!picked) {
		_delivererPoolExhausted = true;
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "pizza: exhausted the pack deliverer pool");
		return;
	}

	_postmanSnoid = picked;

	// If the snoid is already at the seat, just take it.
	if (picked->getPointLoc() == kMachineSeatPosition)
		return;

	// Walk toward the DRAW_ON_REG seat at the fast two-frame interval and mark the arrival as pending.
	picked->setFrameInterval(2);
	picked->initWalkToTarget(kMachineSeatPosition);
	_delivererWalkInPending = true;
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: selected deliverer %d for the machine seat", static_cast<int16>(picked->getId()) - 10000);

	// After a lost deliverer, disable Produce submission while the replacement walks in; the arrival poll re-enables it.
	if (_lostDelivererCount)
		_produceSubmissionEnabled = false;
}

// ---------------------------------------------------------------------------
// Advance the introduction sequence.
// Each step that loads a SCRB only increments the counter (1->2, 2->3, 3->4).
// The termination step (->0) fires on the NEXT callback, AFTER the SCRB finishes.
//
// Flow:
//   Step 1: Load SCRB 8032 (Arno), step=2  (always)
//   Step 2: diff==0 -> step=0 | diff>=1 -> Load SCRB 9034 (Willa), step=3
//   Step 3: diff==1 -> step=0 | diff>=2 -> Load SCRB 10038 (Shyler), step=4
//   Step 4: step=0
//
// The zero step triggers @ref ZoombiniPuzzlePizza::triggerOrderFeatureAmbientAnim().
// That method loads an ambient SCRB on the last active troll.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::advanceIntroSequence() {
	switch (_introSequenceStep) {
	case 1:
		// Step 1: Load intro SCRB 8032 on the base order runner (Arno)
		loadScrbOntoFeature(_arnoFeature, kResScrb8032_ArnoOrderIntro);
		_arnoPhase = kPhaseIntro;
		_introSequenceStep = 2;
		break;
	case 2:
		if (kPuzzleLevel2 <= _difficultyLevel) {
			// Step 2 (L2+): Load intro SCRB 9034 on order 1 runner (Willa)
			if (_willaFeature) {
				loadScrbOntoFeature(_willaFeature, kResScrb9034_WillaOrderIntro);
				_willaPhase = kPhaseIntro;
			}
			_introSequenceStep = 3;
		} else {
			// Step 2 (L1): Arno's SCRB finished, intro done
			_introSequenceStep = 0;
		}
		break;
	case 3:
		if (kPuzzleLevel3 <= _difficultyLevel) {
			// Step 3 (L3+): Load intro SCRB 10038 on order 2 runner (Shyler)
			if (_shylerFeature) {
				loadScrbOntoFeature(_shylerFeature, kResScrb10038_ShylerOrderIntro);
				_shylerPhase = kPhaseIntro;
			}
			_introSequenceStep = 4;
		} else {
			// Step 3 (L2): Willa's SCRB finished, intro done
			_introSequenceStep = 0;
		}
		break;
	case 4:
		// Step 4 (L3+): Shyler's SCRB finished, intro done
		_introSequenceStep = 0;
		break;
	default:
		break;
	}

	// When the step reaches zero, mark the introduction complete.
	// Load an ambient SCRB on the last active troll to reactivate its idle animation.
	if (_introSequenceStep == 0 && !_introComplete) {
		_introComplete = true;
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "pizza: completed introduction sequence");
		triggerOrderFeatureAmbientAnim();
	}
}

// ---------------------------------------------------------------------------
// Trigger an ambient order-feature animation.
// Load an ambient idle SCRB on the last active troll after the introduction completes.
// Loading the script reactivates rendering so the troll plays a short idle animation.
// Level 1 uses SCRB 8014 on Arno.
// Level 2 randomly selects SCRB 9019-9020 on Willa.
// Levels 3-4 randomly select SCRB 10001-10008 on Shyler.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::triggerOrderFeatureAmbientAnim() {
	// Set a post-intro phase and load an ambient SCRB on the last troll.
	// Its completion spawns the next answer Zoombini;
	// the subsequent draw-on-registration completion clears the delivery-in-progress state.
	if (_difficultyLevel == kPuzzleLevel1) {
		loadScrbOntoFeature(_arnoFeature, kResScrb8014_ArnoReject);
		_arnoPhase = kPhasePostIntroAmbient;
	} else if (_difficultyLevel == kPuzzleLevel2) {
		int16 variant = _vm->_rnd->getRandomNumber(1); // 0 or 1
		loadScrbOntoFeature(_willaFeature, kResScrb9019_WillaIdleBase + variant);
		_willaPhase = kPhasePostIntroAmbient;
	} else {
		int16 variant = _vm->_rnd->getRandomNumber(7); // 0-7
		loadScrbOntoFeature(_shylerFeature, kResScrb10001_ShylerRejectBase + variant);
		_shylerPhase = kPhasePostIntroAmbient;
	}
}

void ZoombiniPuzzlePizza::runOrderFeatureAmbientIdleDriver() {
	if (_arnoPhase != kPhaseNone || _willaPhase != kPhaseNone || _shylerPhase != kPhaseNone)
		return;

	if (_difficultyLevel == kPuzzleLevel1) {
		if (_trollOrderStates[0] != TrollOrderState::kAccepted03)
			loadScrbOntoFeature(_arnoFeature, kResScrb8034_ArnoIdleBase + _vm->_rnd->getRandomNumber(1));
		return;
	}

	if (_difficultyLevel == kPuzzleLevel2) {
		if (_vm->_rnd->getRandomNumber(1000) < 500 && _trollOrderStates[0] == TrollOrderState::kActive01) {
			loadScrbOntoFeature(_arnoFeature, kResScrb8034_ArnoIdleBase + _vm->_rnd->getRandomNumber(1));
		} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
			loadScrbOntoFeature(_willaFeature, kResScrb9019_WillaIdleBase + _vm->_rnd->getRandomNumber(1));
		}
		return;
	}

	const int16 orderSelector = _vm->_rnd->getRandomNumber(1000);
	if (orderSelector < 300 && _trollOrderStates[0] == TrollOrderState::kActive01) {
		loadScrbOntoFeature(_arnoFeature, kResScrb8034_ArnoIdleBase + _vm->_rnd->getRandomNumber(1));
	} else if (orderSelector < 600 && _trollOrderStates[1] == TrollOrderState::kActive01) {
		loadScrbOntoFeature(_willaFeature, kResScrb9019_WillaIdleBase + _vm->_rnd->getRandomNumber(1));
	} else if (_trollOrderStates[2] == TrollOrderState::kActive01) {
		const int16 variant = _vm->_rnd->getRandomNumber(3);
		const int16 scrbId = static_cast<int16>(kResScrb10000_ShylerOrderBase + (variant ? variant + 5 : 1));
		loadScrbOntoFeature(_shylerFeature, scrbId);
	}
}

// ---------------------------------------------------------------------------
// Pack the topping-selection bitmask.
// Pack the submitted-meal snapshot into a single byte.
// ---------------------------------------------------------------------------
byte ZoombiniPuzzlePizza::packToppingBitmask() const {
	byte mask = 0;
	for (int16 i = 0; i < 8; i++) {
		if (_submittedMealToppings[i])
			mask |= (1 << i);
	}
	return mask;
}

// ---------------------------------------------------------------------------
// Return whether the submitted meal bitmask was already tried.
// ---------------------------------------------------------------------------
bool ZoombiniPuzzlePizza::checkToppingMaskMatch() const {
	byte currentMask = packToppingBitmask();
	for (int16 i = 0; i <= _lastToppingMaskHistoryIdx; i++) {
		if (_toppingMaskHistory[i] == currentMask)
			return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Handle order submission.
// Called when the player clicks the authored Produce or machine-mouth area.
// Start the delivery cycle.
// The phases are Produce button, machine callback, overlay, classification, service, evaluation, delivery callback,
// and advance.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::handleSubmit() {
	_produceSubmissionEnabled = false;

	// Snapshot the live machine selection for this delivery chain.
	for (int16 i = 0; i < 8; i++) {
		_submittedMealToppings[i] = _selectedMealToppings[i];
	}

	// Ignore submissions after every delivery has finished.
	if (_delivererPoolExhausted)
		return;

	// Load the pressed Produce-button SCRB (7057 at level 1, 7058 at level 2+).
	int16 answerScrbId;
	if (_difficultyLevel == kPuzzleLevel1)
		answerScrbId = kResScrb7057_AnswerLevel1;
	else
		answerScrbId = kResScrb7058_AnswerLevel2Base;
	loadScrbOntoFeature(_produceButtonFeature, answerScrbId);

	// Load SCRB 7066 on the machine feature to start the production callback chain.
	loadScrbOntoFeature(_machineFeature, kResScrb7066_QuestionExit);
	_machinePhase = kPhaseExitCallback;
	debugC(2, MohawkEngine_Zoombini::kDebugPage02, "pizza: started delivery cycle");
	// Keep the SCRB 7066 machine feature behind the Produce button while the exit animation is active.
	// Otherwise its machine-front frames cover the pressed generate button.
	manualLinkBefore(_machineFeature, _produceButtonFeature);
}

// ---------------------------------------------------------------------------
// Handle Pizza exit callbacks.
// Handle animation events from SCRB 7066 on the machine feature and SCRB 9026 on Willa.
// Event 32: Initial overlay setup
// Event 60: Snoid trait reveal
// Event 99: Restore Willa behind active Arno
// Event -1: Delivery overlay and classify
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::handleZmbExitEvent(ZmbFeature *feature, int16 eventCode) {
	switch (eventCode) {
	case kQuestionEventCode032_LoadToppingOverlay: {
		// The machine feature reached the produce marker.
		// A newly registered overlay starts at the render-list tail, which is the visual front.
		// Move the serving stone after it before rebuilding the active delivery chain.
		// The final overlay-after-stone link leaves every older pizza behind both runners.
		if (!_toppingOverlayFeature) {
			ZmbFeature::EventHooks overlayHooks;
			overlayHooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzlePizza::toppingRunner_preRenderShape));
			_toppingOverlayFeature = loadScrbFeature(
				ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Topping), kResScrb12000_ToppingOverlay, 6,
				ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE,
				overlayHooks);
			manualLinkAfter(_servingStoneFeature, _toppingOverlayFeature);
		} else {
			loadScrbOntoFeature(_toppingOverlayFeature, kResScrb12000_ToppingOverlay);
		}
		if (_toppingOverlayFeature) {
			_overlayPhase = kPhaseToppingOverlay;
			linkToppingRunners();
		}
		// The generate button pops back out (unpressed) as soon as the machine starts producing.
		registerProduceButton();
		break;
	}
	case kQuestionEventCode060_StartTraitReveal: {
		// The machine feature reached the trait-reveal marker.
		// Start the Postman SCRS before the delivery overlay is classified.
		// Play snoid SCRS for trait reveal
		if (_postmanSnoid) {
			const int16 traitIdx = getTraitIndexForOrder(0);
			const int16 scrsId = static_cast<int16>(kResScrs13000_CarryBase + traitIdx);
			startSnoidScrs(_postmanSnoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle);
		}
		break;
	}
	case kAnimEventM1_End: {
		// The machine feature's production animation ended.
		// Select the active order, start its delivery SCRS, and reload the matching overlay.
		// Determine active order line for this delivery
		_machinePhase = kPhaseNone;

		int16 traitIdx = getTraitIndexForOrder(0);

		// Select the first active order line.
		int16 scrsId = 0;
		int16 overlayScrbId = 0;
		if (_trollOrderStates[0] == TrollOrderState::kActive01) {
			scrsId = kResScrs13005_DeliveryBase + traitIdx;
			overlayScrbId = kResScrb12001_ToppingOverlayBase + traitIdx;
		} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
			scrsId = kResScrs13010_DeliveryBase + traitIdx;
			overlayScrbId = kResScrb12006_ToppingOverlayBase + traitIdx;
		} else {
			scrsId = kResScrs13015_DeliveryBase + traitIdx;
			overlayScrbId = kResScrb12011_ToppingOverlayBase + traitIdx;
		}

		// Play exit SCRS on the Postman.
		if (_postmanSnoid)
			startSnoidScrs(_postmanSnoid, ZmbResource(ZmbResource::kPage, scrsId), ZmbScrsCompletionMode::kReturnToIdle);

		// Load delivery overlay SCRB
		if (_toppingOverlayFeature) {
			// Queue only the coupled delivery overlay so its SFX can replace the movement loop.
			_toppingOverlayFeature->setScriptSoundPolicy(ZmbFeature::ScriptSoundPolicy::kPriorityQueue);
			loadScrbOntoFeature(_toppingOverlayFeature, overlayScrbId);
			_overlayPhase = kPhaseToppingDelivery;
			linkToppingRunners();
		}

		// Refresh the preview.
		registerProduceButton();

		break;
	}
	case kWillaResultEventCode099_RelinkBehindArno:
		// The terminal Willa frame omits its authored Arno occlusion layer.
		// Restore the real active Arno as the foreground owner for that overlap.
		if (feature == _willaFeature && _trollOrderStates[0] == TrollOrderState::kActive01)
			manualLinkBefore(_willaFeature, _arnoFeature);
		break;
	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// Handle a Postman delivery event.
// Handle events during the delivery evaluation animation.
// Event 61: play SCRS on snoid + SFX 8040
// Event -1: if the allowance was exhausted, lose this deliverer;
//           otherwise return the same deliverer to the machine seat.
// Event 0: toggle facing, handle pending body arrangement
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::handleZmbDeliveryEvent(ZmbFeature *feature, int16 eventCode) {
	(void)feature;

	if (eventCode == kDeliveryEventCode061_StartSnoidScript) {
		// The delivery evaluation reached its Postman marker.
		// Start the survival-specific SCRS and play the delivery confirmation SFX.
		if (_skipDeliveryScriptEvent) {
			_skipDeliveryScriptEvent = false;
			return;
		}

		// Determine SCRS ID and initial position based on active order
		int16 scrsId = 0;
		Common::Point initPos(180, 327);
		if (_trollOrderStates[0] == TrollOrderState::kActive01) {
			scrsId = kResScrs14000_DeliveryBase + _delivererSurvivedAttempt;
			if (!_delivererSurvivedAttempt)
				initPos = Common::Point(34, 59);
		} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
			scrsId = kResScrs14002_DeliveryBase + _delivererSurvivedAttempt;
			if (!_delivererSurvivedAttempt)
				initPos = Common::Point(46, 46);
		} else {
			scrsId = kResScrs14004_DeliveryBase + _delivererSurvivedAttempt;
			if (!_delivererSurvivedAttempt)
				initPos = Common::Point(95, 27);
		}

		// Play SCRS on the Postman.
		// The registered group determines the script state; the page helper also
		// dirties both the interrupted pose and the synchronously materialized frame.
		ZmbScrsCompletionMode completionMode;
		if (_delivererSurvivedAttempt)
			completionMode = ZmbScrsCompletionMode::kReturnToIdle;
		else
			completionMode = ZmbScrsCompletionMode::kHide;
		startSnoidScrs(_postmanSnoid, ZmbResource(ZmbResource::kPage, scrsId), completionMode, &initPos);

		// Play delivery SFX
		_vm->_sound->playSound(ZmbResource(ZmbResource::kPage, kResSound8040_DeliverySfx), Audio::Mixer::kSFXSoundType);

	} else if (eventCode == kAnimEventM1_End) {
		// Complete delivery evaluation, then load the result SCRB.
		// Use @ref ZoombiniPuzzlePizza::kPhaseDeliveryResult
		// instead of @ref ZoombiniPuzzlePizza::kPhaseDeliveryEval to avoid re-entering evaluation
		// when the result SCRB completes.
		if (!_delivererSurvivedAttempt) {
			// The rejecting SCRS already flung the deliverer away and hid it.
			// Clear the runner and request the next pick.
			_lostDelivererCount += 1;
			_nextDelivererRequired = true;
			if (_postmanSnoid) {
				_postmanSnoid->_packIsOccupied = false;
				_postmanSnoid = nullptr;
			}
			_delivererReturnPending = false;
			debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: lost a deliverer; lost count is %d", _lostDelivererCount);
		} else {
			// The same deliverer walks back to the answer seat.
			_nextDelivererRequired = false;
			if (_postmanSnoid)
				_delivererReturnPending = true;
		}

		// Track result SCRB 8020, 9026, or 10030 as @ref ZoombiniPuzzlePizza::kPhaseDeliveryResult.
		loadDeliveryResultScrb();

	} else if (eventCode == kDeliveryEventCode000_ToggleFacing) {
		// Event 0 is consumed by the normal Snoid animation dispatcher.
		// Let the animation system toggle frame visibility.
	}
}

// ---------------------------------------------------------------------------
// Handle completion of an order line.
// Called when an order line's reaction animation finishes (event -1).
// Handle matched-to-accepted transitions, serve chaining, and the evaluation trigger.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::handleOrderLineComplete(int16 orderLine) {
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: completed reaction for order %d in state %d", orderLine, static_cast<int>(_trollOrderStates[orderLine]));

	// An already accepted order spawns its next Postman.
	// When every order is satisfied, the completion chain starts the solved-order troll reactions, enables the Go button,
	// and arms the narrator voice-over.
	// @ref ZoombiniPuzzlePizza::spawnPostmanSnoid() arms the ready-flash completion in slot 40.
	// That completion unlocks delivery.
	if (_trollOrderStates[orderLine] == TrollOrderState::kAccepted03) {
		// Rebuild the persistent accepted-pizza stack after the troll's accept animation.
		linkSettledAcceptedToppingStack(orderLine);

		spawnPostmanSnoid();
		if (_allTrollOrdersMatched) {
			setupQuestionRunners();
			// The final active troll reaction has completed; retain the Go latch in the common button state.
			setGoButtonsEnabled(true);
			return;
		}
		// The acceptance SCRB has finished.
		// Either serve the same meal to the next active troll or clear the pending-order counter
		// so the next pizza can be classified for the remaining orders.
		// Without the reset, those orders remain gated and the puzzle cannot progress.
		if (_submittedMealAlreadyTried) {
			_exactOrderMatchCount = 0;
		} else if (orderLine == 0) {
			if (_trollOrderStates[1] != TrollOrderState::kActive01 || _acceptedOrdersThisMeal) {
				if (_trollOrderStates[2] != TrollOrderState::kActive01 || _acceptedOrdersThisMeal)
					_acceptedOrdersThisMeal = 0;
				else
					serveNextTopping(TrollOrderLine::kShyler02);
			} else {
				serveNextTopping(TrollOrderLine::kWilla01);
			}
		} else if (orderLine == 1) {
			if (_trollOrderStates[2] != TrollOrderState::kActive01 || _acceptedOrdersThisMeal)
				_acceptedOrdersThisMeal = 0;
			else
				serveNextTopping(TrollOrderLine::kShyler02);
		} else {
			_acceptedOrdersThisMeal = 0;
		}
		// @ref ZoombiniPuzzlePizza::spawnPostmanSnoid() can return through its guards without loading the ready flash.
		// For example, a troll reaction runner may still be active; re-enable Produce submission in that case.
		if (_produceButtonPhase != kPhaseSpawnAnswer) {
			_produceButtonPhase = kPhaseNone;
			_produceSubmissionEnabled = true;
		}
		return;
	}

	// A newly matched order begins its acceptance transition.
	if (_trollOrderStates[orderLine] == TrollOrderState::kMatched02) {
		// The troll takes the pizza off the serving stone, so hide the delivery overlay now.
		hideToppingOverlay();
		_toppingOverlayCompletionPending = false;

		ZmbFeature *orderFeature;
		int16 acceptScrbId;
		FeaturePhase *acceptPhase;
		if (orderLine == 0) {
			orderFeature = _arnoFeature;
			acceptScrbId = kResScrb8021_ArnoDeliveryAccept;
			acceptPhase = &_arnoPhase;
		} else if (orderLine == 1) {
			orderFeature = _willaFeature;
			acceptScrbId = kResScrb9027_WillaDeliveryAccept;
			acceptPhase = &_willaPhase;
		} else {
			orderFeature = _shylerFeature;
			acceptScrbId = kResScrb10031_ShylerDeliveryAccept;
			acceptPhase = &_shylerPhase;
		}

		// Load the accept-and-eat SCRB onto the idle troll's order feature.
		loadScrbOntoFeature(orderFeature, acceptScrbId);

		_trollOrderStates[orderLine] = TrollOrderState::kAccepted03;
		if (3 < getPackSnoidCount() - _lostDelivererCount)
			_pendingCelebrationBatchCount += 1;

		// Render only this troll's requested combination on the accepted pizza.
		attachOrderFilter(orderFeature);

		// Draw the accepting troll in front of the serving stone.
		// Overlay creation placed every older pizza behind the stone for this delivery.
		manualLinkAfter(orderFeature, _servingStoneFeature);
		// Arno and Willa draw the deliverer in front of the accepting troll.
		// Shyler preserves the deliverer's existing chain position.
		// On those two paths, the next delivery overlay links to the Postman
		// and remains in front of the already-accepted troll.
		if (_postmanSnoid && orderLine < 2)
			manualLinkAfter(_postmanSnoid, orderFeature);

		// The acceptance loads while this runner is already in the current pre-render pass.
		// Materialize frame zero immediately so the serving-stone pizza transfers
		// to the acceptance runner without a blank frame.
		prepareFeatureVisualCoverage(orderFeature);
		markPreparedFeatureVisualCoverageDirty(orderFeature, true);

		// Track the 8021/9027/10031 completion on the order runner.
		// The corresponding handler re-enters with the accepted state.
		// It then starts the ready flash that unlocks the produce button.
		*acceptPhase = kPhaseServeReaction;

		// Mark the accepted troll so the return setup preserves the link created above.
		_preserveAcceptedTrollLinkCount += 1;
		// The Postman turns around at the serving spot in @ref kSnoidAnimState001_TurnLeft,
		// and the Produce-button runner is reloaded; when the turn completes the Snoid walks back to the machine seat.
		animatePostman();
		// Record the pending order and restart delivery-slot selection.
		_acceptedOrdersThisMeal += 1;
		_pendingResultOrderSlot = 0;
		debugC(2, MohawkEngine_Zoombini::kDebugPage02, "pizza: accepted order %d with SCRB %d", orderLine, acceptScrbId);

		return;
	}

	// Check what to do next
	if (_allTrollOrdersMatched) {
		setupQuestionRunners();
		// The final active troll reaction has completed; retain the Go latch in the common button state.
		setGoButtonsEnabled(true);
		return;
	}

	if (_submittedMealAlreadyTried) {
		_exactOrderMatchCount = 0;
		// The repeat-combo placement stores a pending delivery slot,
		// so the frame poll still calls @ref ZoombiniPuzzlePizza::evaluateDelivery() on the next frame.
		if (0 < _pendingResultOrderSlot)
			evaluateDelivery();
		return;
	}

	// Chain to next active, non-accepted order line
	bool foundNext = false;
	if (orderLine == 0) {
		if (_trollOrderStates[1] == TrollOrderState::kActive01 && !_acceptedOrdersThisMeal) {
			serveNextTopping(TrollOrderLine::kWilla01);
			foundNext = true;
		} else if (_trollOrderStates[2] == TrollOrderState::kActive01 && !_acceptedOrdersThisMeal) {
			serveNextTopping(TrollOrderLine::kShyler02);
			foundNext = true;
		}
	} else if (orderLine == 1) {
		if (_trollOrderStates[2] == TrollOrderState::kActive01 && !_acceptedOrdersThisMeal) {
			serveNextTopping(TrollOrderLine::kShyler02);
			foundNext = true;
		}
	}

	if (!foundNext) {
		// All lines have been served, so trigger @ref ZoombiniPuzzlePizza::evaluateDelivery().
		// @ref ZoombiniPuzzlePizza::serveNextTopping() sets this count, and frame updates check it.
		_acceptedOrdersThisMeal = 0;
		if (0 < _pendingResultOrderSlot) {
			evaluateDelivery();
		}
	}

	_exactOrderMatchCount = 0;
}

// ---------------------------------------------------------------------------
// Handle completion of a topping delivery.
// This callback runs when the delivery overlay animation completes.
// Record the topping bitmask and check for duplicate combinations.
// Call @ref ZoombiniPuzzlePizza::placeTopping() for repeated combinations.
// Call @ref ZoombiniPuzzlePizza::serveNextTopping() for new combinations.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::onToppingDelivered() {

	_toppingOwnerOrderSlot = 0;

	_submittedMealAlreadyTried = checkToppingMaskMatch();
	debugC(3, MohawkEngine_Zoombini::kDebugPage02, "pizza: completed delivery overlay; repeated meal=%d", _submittedMealAlreadyTried ? 1 : 0);

	if (_submittedMealAlreadyTried) {
		// Check active orders in priority order.
		// Place the topping on the first partial subset match and stop,
		// so one delivery cannot be assigned to multiple orders.
		bool placed = false;
		if (_trollOrderStates[0] == TrollOrderState::kActive01 &&
			classifySubmittedMeal(TrollOrderLine::kArno00) == SubmittedMealClassification::kMissingRequestedToppings01) {
			placeTopping(1, 0);
			placed = true;
		}
		if (!placed && _trollOrderStates[1] == TrollOrderState::kActive01 &&
			classifySubmittedMeal(TrollOrderLine::kWilla01) == SubmittedMealClassification::kMissingRequestedToppings01) {
			placeTopping(1, 1);
			placed = true;
		}
		if (!placed && _trollOrderStates[2] == TrollOrderState::kActive01 &&
			classifySubmittedMeal(TrollOrderLine::kShyler02) == SubmittedMealClassification::kMissingRequestedToppings01) {
			placeTopping(1, 2);
			placed = true;
		}
		if (!placed) {
			// Non-match for order 2.
			placeTopping(0, 2);
		}
	} else {
		// Serve to first active order line
		if (_trollOrderStates[0] == TrollOrderState::kActive01) {
			serveNextTopping(TrollOrderLine::kArno00);
		} else if (_trollOrderStates[1] == TrollOrderState::kActive01 && !_acceptedOrdersThisMeal) {
			serveNextTopping(TrollOrderLine::kWilla01);
		} else if (_trollOrderStates[2] == TrollOrderState::kActive01 && !_acceptedOrdersThisMeal) {
			serveNextTopping(TrollOrderLine::kShyler02);
		}
	}
}

// ---------------------------------------------------------------------------
// Register or reload the Produce button.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::registerProduceButton() {
	int16 scrbId = static_cast<int16>(kResScrb7001_AnswerLevel1 + (_difficultyLevel - 1));
	if (_produceButtonFeature) {
		loadScrbOntoFeature(_produceButtonFeature, scrbId);
	} else {
		ZmbFeature::EventHooks hooks;
		hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzlePizza::produceButton_preRenderShape));
		_produceButtonFeature = loadScrbFeature(
			ZmbResource(ZmbResource::kPage, kResBitmapShape7000_Order), scrbId, 6,
			ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE,
			hooks);
	}
}

// ---------------------------------------------------------------------------
// Prepare the Produce-button meal shapes before rendering.
//
// Pre-render shape callback for the Produce-button feature.
// Filters out hotspot entries whose shapes correspond to unselected toppings.
// Shape IDs 57-61 map to selected toppings 0-4; 67-69 map to 5-7.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::produceButton_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	for (int hotspotIdx = static_cast<int>(hotspots.size()) - 1; 0 <= hotspotIdx; hotspotIdx--) {
		int16 shapeIdx = hotspots[hotspotIdx]._shapeIdx;
		int flagIdx = -1;
		if (57 <= shapeIdx && shapeIdx <= 61)
			flagIdx = shapeIdx - 57; // maps 57-61 -> flags 0-4
		else if (67 <= shapeIdx && shapeIdx <= 69)
			flagIdx = shapeIdx - 62; // maps 67-69 -> flags 5-7

		if (0 <= flagIdx && !_selectedMealToppings[flagIdx])
			hotspots.remove_at(hotspotIdx);
	}
}

// ---------------------------------------------------------------------------
// Start the Produce-button ready flash for the next Postman action.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::spawnPostmanSnoid() {
	// Skip if the machine is active, a ready flash is pending, or all orders already matched.
	if (_machinePhase != kPhaseNone || _produceButtonPhase == kPhaseSpawnAnswer ||
		_allTrollOrdersMatched || _delivererPoolExhausted)
		return;

	// Load the Produce-button ready SCRB for the next delivery.
	if (!_delivererPoolExhausted) {
		int16 scrbId = (_difficultyLevel == kPuzzleLevel1) ? kResScrb7067_AnswerReadyLevel1 : kResScrb7068_AnswerReadyLevel2;
		loadScrbOntoFeature(_produceButtonFeature, scrbId);
		// Track its completion before unlocking the delivery button.
		_produceButtonPhase = kPhaseSpawnAnswer;
	} else {
		_delivererPoolExhausted = true;
	}
}

// ---------------------------------------------------------------------------
// Animate the Postman.
// Find the Postman, set speed=6, start walk right animation (anim 1),
// link to the first active (not yet accepted) order feature.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::animatePostman() {
	if (!_postmanSnoid)
		return;

	// Use the normal interval and enter @ref kSnoidAnimState001_TurnLeft,
	// which briefly turns around and then becomes idle.
	// The Snoid is standing at the SERVING spot (where the carry SCRS left it);
	// the completion handler walks it back to the answer seat.
	_postmanSnoid->setFrameInterval(6);
	_postmanSnoid->setAnimState(kSnoidAnimState001_TurnLeft);
	_delivererReturnPending = true;

	// A completed answer already owns its Z-order; clear the marker without relinking it.
	if (_preserveAcceptedTrollLinkCount) {
		_preserveAcceptedTrollLinkCount = 0;
	} else if (_trollOrderStates[0] == TrollOrderState::kActive01) {
		// Draw the Postman in front of active Arno.
		manualLinkAfter(_postmanSnoid, _arnoFeature);
	} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
		// Draw the Postman in front of active Willa.
		manualLinkAfter(_postmanSnoid, _willaFeature);
	} else if (_trollOrderStates[2] == TrollOrderState::kActive01) {
		// Draw the Postman in front of active Shyler.
		manualLinkAfter(_postmanSnoid, _shylerFeature);
	}

	// Restore the normal Produce-button feature for the current Postman.
	registerProduceButton();
}

// ---------------------------------------------------------------------------
// Set up solved-order troll reaction runners.
// Select reaction SCRBs from the active troll combination.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::setupQuestionRunners() {
	const bool has1 = _trollOrderStates[1] != TrollOrderState::kInactive00;
	const bool has2 = _trollOrderStates[2] != TrollOrderState::kInactive00;

	// Arno always owns the base completion runner. Optional troll state zero means that runner is absent.
	if (has1 && has2) {
		// All three active
		const int16 variant = _vm->_rnd->getRandomNumber(0, 1);
		loadScrbOntoFeature(_arnoFeature, kResScrb8030_ArnoReactionBase + variant);
		loadScrbOntoFeature(_willaFeature, kResScrb9032_WillaReactionBase + variant);
		loadScrbOntoFeature(_shylerFeature, kResScrb10036_ShylerReactionBase + variant);
		attachOrderFilter(_arnoFeature);
		attachOrderFilter(_willaFeature);
		attachOrderFilter(_shylerFeature);
	} else if (has1) {
		const int16 variant = _vm->_rnd->getRandomNumber(0, 1);
		loadScrbOntoFeature(_arnoFeature, kResScrb8026_ArnoRejectBase + variant);
		loadScrbOntoFeature(_willaFeature, kResScrb9030_WillaRejectBase + variant);
		attachOrderFilter(_arnoFeature);
		attachOrderFilter(_willaFeature);
	} else if (has2) {
		const int16 variant = _vm->_rnd->getRandomNumber(0, 1);
		loadScrbOntoFeature(_arnoFeature, kResScrb8028_ArnoRejectBase + variant);
		loadScrbOntoFeature(_shylerFeature, kResScrb10035_ShylerReject + variant);
		attachOrderFilter(_arnoFeature);
		attachOrderFilter(_shylerFeature);
	} else {
		loadScrbOntoFeature(_arnoFeature, kResScrb8024_ArnoRejectBase + _vm->_rnd->getRandomNumber(0, 1));
		attachOrderFilter(_arnoFeature);
	}

	// Register the reaction SCRB group in slot 25 on the base order runner.
	// Its completion plays the narrator voice-over through @ref ZoombiniPuzzlePizza::kPhaseQuestionSetup.
	_arnoPhase = kPhaseQuestionSetup;
	_willaPhase = kPhaseNone;
	_shylerPhase = kPhaseNone;
	debugC(2, MohawkEngine_Zoombini::kDebugPage02, "pizza: configured solved-order reactions");
}

// ---------------------------------------------------------------------------
// Place a topping.
// For repeat topping combinations (mask match), decide which order feature
// gets the topping placement animation.
// @param mode  1 = allWrong/partial (use hintSlot directly),
//              0 or 4 = auto-select among active orders
// @param hintSlot Target order slot (0-2) when mode==1
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::placeTopping(int16 mode, int16 hintSlot) {
	_exactOrderMatchCount = 0;

	int16 targetSlot = hintSlot;
	if (!mode || mode == 4) {
		// Select among the active order lines.
		if (_trollOrderStates[0] == TrollOrderState::kActive01) {
			if (_trollOrderStates[1] != TrollOrderState::kActive01 && _trollOrderStates[2] != TrollOrderState::kActive01) {
				targetSlot = 0;
			} else if (_trollOrderStates[1] == TrollOrderState::kActive01 && _trollOrderStates[2] != TrollOrderState::kActive01) {
				targetSlot = _vm->_rnd->getRandomNumber(0, 1);
			} else if (_trollOrderStates[1] != TrollOrderState::kActive01 && _trollOrderStates[2] == TrollOrderState::kActive01) {
				targetSlot = 2 * _vm->_rnd->getRandomNumber(0, 1);
			} else {
				// All three active -- picks 0 or 1
				targetSlot = _vm->_rnd->getRandomNumber(0, 1);
			}
		} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
			if (_trollOrderStates[2] == TrollOrderState::kActive01)
				targetSlot = _vm->_rnd->getRandomNumber(0, 1) + 1;
			else
				targetSlot = 1;
		} else {
			targetSlot = 2;
		}
	}

	ZmbFeature *orderFeature = nullptr;
	int16 scrbId = 0;
	FeaturePhase *phase = nullptr;

	switch (targetSlot) {
	case 0:
		orderFeature = _arnoFeature;
		phase = &_arnoPhase;
		_pendingResultOrderSlot = 1;
		if (mode == 1) {
			scrbId = kResScrb8000_ArnoOrderBase + _arnoMissingToppingAnimStep;
			_toppingOwnerOrderSlot = 1;
			_toppingRunnerCategory = 5;
		} else {
			scrbId = kResScrb8015_ArnoRejectBase + _vm->_rnd->getRandomNumber(0, 1);
			_toppingRunnerCategory = 4;
		}
		break;
	case 1:
		orderFeature = _willaFeature;
		phase = &_willaPhase;
		_pendingResultOrderSlot = 2;
		if (mode == 1) {
			scrbId = kResScrb9021_WillaOneCorrectBase + _willaMissingToppingAnimStep;
			_toppingOwnerOrderSlot = 2;
			_toppingRunnerCategory = 6;
		} else {
			scrbId = kResScrb9017_WillaRejectBase + _vm->_rnd->getRandomNumber(0, 1);
			_toppingRunnerCategory = 4;
		}
		break;
	case 2:
		orderFeature = _shylerFeature;
		phase = &_shylerPhase;
		_pendingResultOrderSlot = 3;
		if (mode == 1) {
			scrbId = kResScrb10009_ShylerMultipleCorrectBase + _shylerMissingToppingAnimStep;
			_toppingOwnerOrderSlot = 3;
			_toppingRunnerCategory = 7;
		} else {
			scrbId = kResScrb10027_ShylerRejectBase + _vm->_rnd->getRandomNumber(0, 1);
			_toppingRunnerCategory = 4;
		}
		break;
	default:
		return;
	}

	if (orderFeature && scrbId) {
		loadScrbOntoFeature(orderFeature, scrbId);
		linkToppingRunners();
		if (phase)
			*phase = kPhaseServeReaction;
		debugC(4, MohawkEngine_Zoombini::kDebugPage02, "pizza: placed topping on order slot %d with SCRB %d in mode %d", targetSlot, scrbId, mode);
	}
}

// ---------------------------------------------------------------------------
// Play the loading chant.
// The pizza-troll singing heard between the xfer map and the puzzle page.
// Variant selection from the SFX-selection group:
//   Group 1 (Puzzle Level 1)                     -> variant 0
//   Practice at Puzzle Level 1                   -> variant 0
//   otherwise: rand(0..100): >75->3, >50->2, >25->1, else 0
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::playOpeningChant(ZmbSfxGroupFlags sfxGroupFlags) {
	_loadChantSkipPending = false;
	int16 variant;
	if (sfxGroupFlags == ZmbSfxGroupFlags::kEasy_01) {
		variant = 0;
	} else if (sfxGroupFlags == ZmbSfxGroupFlags::kPractice_05 && _difficultyLevel == kPuzzleLevel1) {
		variant = 0;
	} else {
		int16 r = _vm->_rnd->getRandomNumber(0, 100);
		if (75 < r)
			variant = 3;
		else if (50 < r)
			variant = 2;
		else if (25 < r)
			variant = 1;
		else
			variant = 0;
	}
	playArnoOpeningChant(variant);
}

// ---------------------------------------------------------------------------
// Play Arno's pizza song synchronously during puzzle loading.
// Each sound blocks until it finishes while input and the wait cursor continue to update.
// The background has not yet been presented, so the display remains black.
// Explicit waits of roughly 17 ms per tick pace some chant segments.
//
//   case 0: SND 15005, wait 60 ticks, SND 15006
//   case 1: SND 15000, then SND 15001
//   case 2: SND 15002
//   case 3: SND 15003, then SND 15004
//   case 4: SND 15003, SND 15004, wait 20 ticks, SND 15005, wait 60, SND 15006
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::playArnoOpeningChant(int16 chantVariant) {
	if (4 < chantVariant)
		return;

	switch (chantVariant) {
	case 0:
		if (!playChantSoundSync(kResSound15005_ArnoChant)) {
			pumpLoadWait(60 * 17, false);
			playChantSoundSync(kResSound15006_ArnoChant);
		}
		break;
	case 1:
		if (!playChantSoundSync(kResSound15000_ArnoChant))
			playChantSoundSync(kResSound15001_ArnoChant);
		break;
	case 2:
		playChantSoundSync(kResSound15002_ArnoChant);
		break;
	case 3:
		if (!playChantSoundSync(kResSound15003_ArnoChant))
			playChantSoundSync(kResSound15004_ArnoChant);
		break;
	case 4:
		if (!playChantSoundSync(kResSound15003_ArnoChant) && !playChantSoundSync(kResSound15004_ArnoChant)) {
			pumpLoadWait(20 * 17, false);
			if (!playChantSoundSync(kResSound15005_ArnoChant)) {
				pumpLoadWait(60 * 17, false);
				playChantSoundSync(kResSound15006_ArnoChant);
			}
		}
		break;
	default:
		break;
	}

	_loadChantSkipPending = false;
}

// ---------------------------------------------------------------------------
// Play one chant SND synchronously while updating the screen and animated eye cursor.
// ---------------------------------------------------------------------------
bool ZoombiniPuzzlePizza::playChantSoundSync(int16 sndId) {
	Audio::SoundHandle *handle = _vm->_sound->playSound(ZmbResource(ZmbResource::kPage, sndId), Audio::Mixer::kSpeechSoundType);
	if (!handle)
		return false;

	if (_loadChantSkipPending) {
		_loadChantSkipPending = false;
		_vm->_mixer->stopHandle(*handle);
		return true;
	}

	while (_vm->_mixer->isSoundHandleActive(*handle) && !_vm->shouldQuit()) {
		if (pumpLoadWait(0, true)) {
			_vm->_mixer->stopHandle(*handle);
			return true;
		}
	}

	return false;
}

// ---------------------------------------------------------------------------
// Pump the minimum frames needed for the synchronous loading chant.
// Drain the OS event queue, animate the eye cursor, and refresh the visible screen.
// A zero duration performs one pump iteration.
// ---------------------------------------------------------------------------
bool ZoombiniPuzzlePizza::pumpLoadWait(uint32 durationMs, bool allowSkip) {
	if (allowSkip && _loadChantSkipPending) {
		_loadChantSkipPending = false;
		return true;
	}

	const uint32 endTime = _vm->_system->getMillis() + durationMs;
	do {
		bool skipped = false;
		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if (isLoadChantSkipEvent(event)) {
				if (allowSkip)
					skipped = true;
				else
					_loadChantSkipPending = true;
			}
		}

		uint32 now = _vm->_system->getMillis();
		uint32 currentFrame = _vm->getAnimationFrameCounter(now);
		if (!_vm->_gfx->isMouseCursorEyeAnimationActive())
			_vm->_gfx->startMouseCursorEyeAnimation();
		_vm->_gfx->runMouseCursorEyeAnimationFrame(currentFrame);

		_vm->_gfx->flushScreens();
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(10);

		if (skipped)
			return true;
	} while (_vm->_system->getMillis() < endTime && !_vm->shouldQuit());

	return false;
}

// ---------------------------------------------------------------------------
// Link the topping runners.
// For the active delivery path, link the Postman behind the serving stone and the topping overlay in front of it.
// This keeps the carried pizza in front of the Postman.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::linkToppingRunners() {
	// Rebuild the pizza Z-order chain whenever a delivery or topping runner changes.
	// The delivery overlay stays in front of the serving stone.
	// The Postman and active trolls form the ordered chain behind the stone.
	// The event-created overlay leaves persistent pizzas behind the stone while preserving their relative order.
	if (_toppingOverlayFeature) {
		// Draw the delivery overlay in front of the serving stone.
		manualLinkAfter(_toppingOverlayFeature, _servingStoneFeature);
		// Once Arno has accepted an order, place him behind the accepted-pizza stack
		// so a new delivery remains visible over the seated troll.
		// His acceptance animation has already completed before this overlay exists.
		if (_trollOrderStates[0] == TrollOrderState::kAccepted03) {
			ZmbFeature *parent;
			if (_lastToppingRunnerSlotIdx < 0)
				parent = _toppingOverlayFeature;
			else
				parent = _toppingRunnerSlots[0].feature;
			manualLinkBefore(_arnoFeature, parent);
		}
	}
	if (_postmanSnoid) {
		// Keep the Postman behind the serving stone by default.
		manualLinkBefore(_postmanSnoid, _servingStoneFeature);
		if (_trollOrderStates[0] == TrollOrderState::kActive01) {
			manualLinkBefore(_arnoFeature, _postmanSnoid);
			if (_trollOrderStates[2] == TrollOrderState::kActive01) {
				manualLinkBefore(_shylerFeature, _arnoFeature);
				if (_trollOrderStates[1] != TrollOrderState::kActive01)
					return;
				manualLinkBefore(_willaFeature, _shylerFeature);
			} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
				manualLinkBefore(_willaFeature, _arnoFeature);
			}
		} else if (_trollOrderStates[2] == TrollOrderState::kActive01) {
			manualLinkBefore(_shylerFeature, _postmanSnoid);
			if (_trollOrderStates[1] == TrollOrderState::kActive01)
				manualLinkBefore(_willaFeature, _shylerFeature);
		} else if (_trollOrderStates[1] == TrollOrderState::kActive01) {
			manualLinkBefore(_willaFeature, _postmanSnoid);
		}
	}
}

// ---------------------------------------------------------------------------
// Restore one seated troll's persistent accepted-pizza stack.
// Arno and Shyler move behind their first current pizza.
// Willa also restores the authored order of all three current pizza slots.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::linkSettledAcceptedToppingStack(int16 orderLine) {
	ZmbFeature *orderFeature = nullptr;
	ZmbFeature **slots = nullptr;
	if (orderLine == 0) {
		orderFeature = _arnoFeature;
		slots = _arnoToppingRunnerSlots;
	} else if (orderLine == 1) {
		orderFeature = _willaFeature;
		slots = _willaToppingRunnerSlots;
	} else if (orderLine == 2) {
		orderFeature = _shylerFeature;
		slots = _shylerToppingRunnerSlots;
	}

	if (!orderFeature || !slots || !slots[0])
		return;

	manualLinkBefore(orderFeature, slots[0]);
	if (orderLine != 1)
		return;

	ZmbFeature *previous = slots[0];
	if (slots[1]) {
		manualLinkAfter(slots[1], previous);
		previous = slots[1];
	}
	if (slots[2])
		manualLinkAfter(slots[2], previous);
}

// ---------------------------------------------------------------------------
// Finish the tail of topping-runner registration.
// Rebuild the main chain, then apply the final Z-order adjustment for the runner category.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::finishToppingRunnerRegistration(ZmbFeature *newFeature, int16 runnerCategory) {
	// Rebuild the shared chain before applying the category-specific adjustment.
	linkToppingRunners();

	if (runnerCategory == 4) {
		// Generic overlays render in front of the serving stone.
		// Reusing an existing runner leaves its current position unchanged.
		manualLinkAfter(newFeature, _servingStoneFeature);
	} else if (runnerCategory == 6) {
		// For Willa's category, place the first accepted-pizza slot behind the second and place both behind Willa.
		if (_willaToppingRunnerSlots[1]) {
			manualLinkBefore(_willaToppingRunnerSlots[1], _willaFeature);
			manualLinkBefore(_willaToppingRunnerSlots[0], _willaToppingRunnerSlots[1]);
		}
	}
	// @ref ZmbFeature::FLAG_00001000_TOPMOST keeps the Map, Save, and Send overlay above the runners.
}

// ---------------------------------------------------------------------------
// Register a topping runner.
// Create a new SCRB overlay feature showing the accepted topping state on a pizza troll.
// Called after an order line's serve reaction completes.
//
// @ref ZoombiniPuzzlePizza::_toppingRunnerCategory selects the resource family:
//   Type 4: generic overlay, SCRB 12025+counter (wraps at 16, skip 13)
//   Type 5: order 0, SCRB 12016+counter (wraps at 3)
//   Type 6: order 1, SCRB 12019+counter (wraps at 3)
//   Type 7: order 2, SCRB 12022+counter (wraps at 3)
//
// @ref ZmbFeature::setPreRenderShapeFunc() installs the active-ingredient filter.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::registerToppingRunner() {
	if (27 <= _lastToppingRunnerSlotIdx)
		return; // Safety: don't overflow the 28-slot array

	_lastToppingRunnerSlotIdx += 1;

	ToppingRunnerSlot &slot = _toppingRunnerSlots[_lastToppingRunnerSlotIdx];
	slot.mask = (0 <= _lastToppingMaskHistoryIdx) ? _toppingMaskHistory[_lastToppingMaskHistoryIdx] : 0;
	// This runner's completion triggers
	slot.spawnOnComplete = true;

	int16 scrbId = 0;
	ZmbFeature *newFeature = nullptr;
	ZmbFeature *prevSlotRunner = nullptr;

	switch (_toppingRunnerCategory) {
	case 4: {
		// SCRB 12025+counter, wraps at 16, skip 13
		_genericToppingRunnerCycle += 1;
		if (16 <= _genericToppingRunnerCycle) {
			_genericToppingRunnerCycle = 0;
			_genericToppingRunnersWrapped = true;
		}
		if (_genericToppingRunnerCycle == 13)
			_genericToppingRunnerCycle = 14;
		scrbId = kResScrb12025_GenericToppingBase + _genericToppingRunnerCycle;

		if (_genericToppingRunnersWrapped) {
			// Find the matching SCRB slot, reload it, and keep the runner at its current chain position.
			// Store the feature so @ref ZoombiniPuzzlePizza::getToppingRunnerMask() returns the fresh submission mask
			// for this reused runner.
			for (int16 i = 0; i < 28; i++) {
				if (_toppingRunnerSlots[i].scrbId == scrbId && _toppingRunnerSlots[i].feature) {
					loadScrbOntoFeature(_toppingRunnerSlots[i].feature, scrbId);
					slot.feature = _toppingRunnerSlots[i].feature;
					slot.scrbId = scrbId;
					// Reusing a feature leaves no new feature to relink.
					// Pass nullptr to @ref ZoombiniPuzzlePizza::finishToppingRunnerRegistration().
					// This makes the tail relink a no-op.
					finishToppingRunnerRegistration(nullptr, 4);
					return;
				}
			}
		}
		newFeature = createToppingRunnerFeature(scrbId, 6);
		break;
	}
	case 5: {
		// SCRB 12016+counter (wraps at 3)
		_arnoToppingRunnerCycle += 1;
		if (2 < _arnoToppingRunnerCycle)
			_arnoToppingRunnerCycle = 0;
		prevSlotRunner = _arnoToppingRunnerSlots[_arnoToppingRunnerCycle];
		scrbId = kResScrb12016_Order0ToppingBase + _arnoToppingRunnerCycle;
		newFeature = createToppingRunnerFeature(scrbId, 6);
		// Place the new runner in front of the previous occupant of this cyclic slot.
		if (newFeature && prevSlotRunner)
			manualLinkAfter(newFeature, prevSlotRunner);
		_arnoToppingRunnerSlots[_arnoToppingRunnerCycle] = newFeature;
		break;
	}
	case 6: {
		// SCRB 12019+counter (wraps at 3)
		_willaToppingRunnerCycle += 1;
		if (2 < _willaToppingRunnerCycle)
			_willaToppingRunnerCycle = 0;
		prevSlotRunner = _willaToppingRunnerSlots[_willaToppingRunnerCycle];
		scrbId = kResScrb12019_Order1ToppingBase + _willaToppingRunnerCycle;
		newFeature = createToppingRunnerFeature(scrbId, 6);
		if (newFeature && prevSlotRunner)
			manualLinkAfter(newFeature, prevSlotRunner);
		_willaToppingRunnerSlots[_willaToppingRunnerCycle] = newFeature;
		break;
	}
	case 7: {
		// SCRB 12022+counter (wraps at 3)
		_shylerToppingRunnerCycle += 1;
		if (2 < _shylerToppingRunnerCycle)
			_shylerToppingRunnerCycle = 0;
		prevSlotRunner = _shylerToppingRunnerSlots[_shylerToppingRunnerCycle];
		scrbId = kResScrb12022_Order2ToppingBase + _shylerToppingRunnerCycle;
		newFeature = createToppingRunnerFeature(scrbId, 6);
		if (newFeature && prevSlotRunner)
			manualLinkAfter(newFeature, prevSlotRunner);
		_shylerToppingRunnerSlots[_shylerToppingRunnerCycle] = newFeature;
		break;
	}
	default:
		return;
	}

	if (newFeature) {
		slot.feature = newFeature;
		slot.scrbId = scrbId;
	}

	finishToppingRunnerRegistration(newFeature, _toppingRunnerCategory);
}

ZmbFeature *ZoombiniPuzzlePizza::createToppingRunnerFeature(int16 scrbId, uint32 frameInterval) {
	// Create a new overlay runner with a unique identity.
	uint16 featureId = _nextDynamicFeatureId;
	_nextDynamicFeatureId += 1;
	ZmbFeature::EventHooks hooks;
	hooks.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzlePizza::toppingRunner_preRenderShape));

	ZmbFeature *newFeature = loadVirtualFeature(
		ZmbResource(ZmbResource::kPage, kResBitmapShape12000_Topping), featureId, frameInterval,
		ZmbFeature::FLAG_00008000_LOOP_ANIM | ZmbFeature::FLAG_00100000_PLAY_ONCE | ZmbFeature::FLAG_04000000_OVERLAY,
		hooks);

	if (newFeature) {
		// Load the SCRB selected for this runner category.
		loadScrbOntoFeature(newFeature, scrbId);
	}

	return newFeature;
}

byte ZoombiniPuzzlePizza::getToppingRunnerMask(const ZmbFeature *feature) const {
	for (int16 i = _lastToppingRunnerSlotIdx; 0 <= i; i--) {
		if (_toppingRunnerSlots[i].feature == feature)
			return _toppingRunnerSlots[i].mask;
	}

	return packToppingBitmask();
}

// ---------------------------------------------------------------------------
// Prepare topping-runner shapes before rendering.
// Filter each four-shape authored group in @ref ZoombiniPuzzlePizza::MealShape
// by the topping mask and difficulty recorded when this runner was created.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::toppingRunner_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	byte activeMask = getToppingRunnerMask(feature);

	for (int hotspotIdx = static_cast<int>(hotspots.size()) - 1; 0 <= hotspotIdx; hotspotIdx--) {
		int16 shapeIdx = hotspots[hotspotIdx]._shapeIdx;
		bool keep = true;

		if (kMealShapeCheese05_First <= shapeIdx && shapeIdx <= kMealShapeCheese08_Last) {
			keep = (activeMask & (1 << kMealIngredientCheese04)) != 0;
		} else if (kMealShapeMushrooms09_First <= shapeIdx && shapeIdx <= kMealShapeMushrooms12_Last) {
			keep = (activeMask & (1 << kMealIngredientMushrooms03)) != 0;
		} else if (kMealShapePepperoni13_First <= shapeIdx && shapeIdx <= kMealShapePepperoni16_Last) {
			keep = (activeMask & (1 << kMealIngredientPepperoni02)) != 0;
		} else if (kMealShapePeppers17_First <= shapeIdx && shapeIdx <= kMealShapePeppers20_Last) {
			keep = (activeMask & (1 << kMealIngredientPeppers01)) != 0;
		} else if (kMealShapeOlives21_First <= shapeIdx && shapeIdx <= kMealShapeOlives24_Last) {
			keep = (activeMask & (1 << kMealIngredientOlives00)) != 0;
		} else if (kMealShapePlainSundae25_First <= shapeIdx && shapeIdx <= kMealShapePlainSundae28_Last) {
			keep = kPuzzleLevel2 <= _difficultyLevel;
		} else if (kMealShapeChocolateSauce29_First <= shapeIdx && shapeIdx <= kMealShapeChocolateSauce32_Last) {
			keep = (activeMask & (1 << kMealIngredientChocolateSauce07)) != 0 && _difficultyLevel == kPuzzleLevel4;
		} else if (kMealShapeWhippedCream33_First <= shapeIdx && shapeIdx <= kMealShapeWhippedCream36_Last) {
			keep = (activeMask & (1 << kMealIngredientWhippedCream06)) != 0 && kPuzzleLevel2 <= _difficultyLevel;
		} else if (kMealShapeCherry37_First <= shapeIdx && shapeIdx <= kMealShapeCherry40_Last) {
			keep = (activeMask & (1 << kMealIngredientCherry05)) != 0 && kPuzzleLevel2 <= _difficultyLevel;
		}

		if (!keep)
			hotspots.remove_at(hotspotIdx);
	}
}

// ---------------------------------------------------------------------------
// Tear down the topping overlay.
// Remove the serving-stone pizza runner when a troll picks up or accepts the pizza.
// The next produce event registers a fresh runner at the render-list tail, which is the visual front.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::hideToppingOverlay() {
	if (!_toppingOverlayFeature)
		return;
	if (!_toppingOverlayFeature->getZSortRect().isEmpty()) {
		// Accept callbacks run during pre-render, after external damage was merged.
		// Restore the serving stone in this pass as well as preserving the damage for the next pass.
		addDirtyRect(_toppingOverlayFeature->getZSortRect());
		addExternalDirtyRect(_toppingOverlayFeature->getZSortRect());
	}
	unloadScrbFeature(_toppingOverlayFeature);
	_toppingOverlayFeature = nullptr;
	_overlayPhase = kPhaseNone;
}

// ---------------------------------------------------------------------------
// Install the meal-filter callbacks.
// @ref ZoombiniPuzzlePizza::attachMealFilter() installs the meal-filter callback.
// @ref ZoombiniPuzzlePizza::mealFilter_preRenderShape() handles every order runner in the serving and disposal paths.
// The callback filters the topping ranges in @ref ZoombiniPuzzlePizza::OrderFeatureShape with @ref ZoombiniPuzzlePizza::_submittedMealToppings.
// The troll's inspect, carry, and throw animations therefore show exactly the submitted toppings.
// @ref ZoombiniPuzzlePizza::OrderFeatureShape::kArnoAndStoneOcclusionPatch212 is hidden after Arno accepts his order.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::attachMealFilter(ZmbFeature *feature) {
	if (feature)
		feature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzlePizza::mealFilter_preRenderShape));
}

void ZoombiniPuzzlePizza::mealFilter_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)feature;
	(void)hsGroup;

	for (int hotspotIdx = static_cast<int>(hotspots.size()) - 1; 0 <= hotspotIdx; hotspotIdx--) {
		const OrderFeatureShape shapeIdx = static_cast<OrderFeatureShape>(hotspots[hotspotIdx]._shapeIdx);
		bool keep = true;

		if (OrderFeatureShape::kCheese156_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kCheese159_Last) {
			keep = _submittedMealToppings[kMealIngredientCheese04];
		} else if (OrderFeatureShape::kMushrooms160_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kMushrooms163_Last) {
			keep = _submittedMealToppings[kMealIngredientMushrooms03];
		} else if (OrderFeatureShape::kPepperoni164_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kPepperoni167_Last) {
			keep = _submittedMealToppings[kMealIngredientPepperoni02];
		} else if (OrderFeatureShape::kPeppers168_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kPeppers171_Last) {
			keep = _submittedMealToppings[kMealIngredientPeppers01];
		} else if (OrderFeatureShape::kOlives172_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kOlives175_Last) {
			keep = _submittedMealToppings[kMealIngredientOlives00];
		} else if (OrderFeatureShape::kPlainSundae176_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kPlainSundae179_Last) {
			// Removed only at the lowest difficulty
			keep = kPuzzleLevel2 <= _difficultyLevel;
		} else if (OrderFeatureShape::kChocolateSauce180_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kChocolateSauce183_Last) {
			keep = _submittedMealToppings[kMealIngredientChocolateSauce07] && _difficultyLevel == kPuzzleLevel4;
		} else if (OrderFeatureShape::kWhippedCream184_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kWhippedCream187_Last) {
			keep = _submittedMealToppings[kMealIngredientWhippedCream06] && kPuzzleLevel2 <= _difficultyLevel;
		} else if (OrderFeatureShape::kCherry188_First <= shapeIdx && shapeIdx <= OrderFeatureShape::kCherry191_Last) {
			keep = _submittedMealToppings[kMealIngredientCherry05] && kPuzzleLevel2 <= _difficultyLevel;
		} else if (shapeIdx == OrderFeatureShape::kArnoAndStoneOcclusionPatch212) {
			// Omit Willa's Arno-and-stone occlusion patch after Arno accepts.
			keep = _trollOrderStates[0] != TrollOrderState::kAccepted03;
		}

		if (!keep)
			hotspots.remove_at(hotspotIdx);
	}
}

// ---------------------------------------------------------------------------
// Attach the topping filter to a troll order feature.
// Call this after loading a held-pizza or question-display SCRB onto the feature.
// @ref ZoombiniPuzzlePizza::orderFeature_preRenderShape() selects the combination by feature identity.
// One callback therefore serves all three order features.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::attachOrderFilter(ZmbFeature *feature) {
	if (feature)
		feature->setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniPuzzlePizza::orderFeature_preRenderShape));
}

// ---------------------------------------------------------------------------
// Prepare order-feature shapes before rendering.
// Pre-render callback for the troll-held pizza order features.
// Filter the topping ranges in @ref ZoombiniPuzzlePizza::OrderFeatureShape so only the requested combination is drawn for each troll.
// The combo array is selected by feature identity:
// @ref ZoombiniPuzzlePizza::_arnoFeature uses @ref ZoombiniPuzzlePizza::_arnoToppings for Arno.
//   @ref ZoombiniPuzzlePizza::_willaFeature uses @ref ZoombiniPuzzlePizza::_willaToppings for Willa.
//   @ref ZoombiniPuzzlePizza::_shylerFeature uses @ref ZoombiniPuzzlePizza::_shylerToppings for Shyler.
// Each four-frame @ref ZoombiniPuzzlePizza::OrderFeatureShape range maps to the corresponding @ref ZoombiniPuzzlePizza::MealIngredient entry.
// The plain-sundae range and sundae toppings require level 2 or higher, with chocolate sauce additionally restricted to level 4 by order generation.
// @ref ZoombiniPuzzlePizza::OrderFeatureShape::kArnoAndStoneOcclusionPatch212 is hidden after Arno accepts his order.
// ---------------------------------------------------------------------------
void ZoombiniPuzzlePizza::orderFeature_preRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	const bool *combo;
	if (feature == _willaFeature)
		combo = _willaToppings;
	else if (feature == _shylerFeature)
		combo = _shylerToppings;
	else
		combo = _arnoToppings;

	const bool diffEnabled = kPuzzleLevel2 <= _difficultyLevel;

	for (int hotspotIdx = static_cast<int>(hotspots.size()) - 1; 0 <= hotspotIdx; hotspotIdx--) {
		const OrderFeatureShape shapeIdx = static_cast<OrderFeatureShape>(hotspots[hotspotIdx]._shapeIdx);
		if (feature == _willaFeature && shapeIdx == OrderFeatureShape::kArnoAndStoneOcclusionPatch212 && _trollOrderStates[0] == TrollOrderState::kAccepted03) {
			// Omit Willa's Arno-and-stone occlusion patch after Arno accepts.
			hotspots.remove_at(hotspotIdx);
			continue;
		}
		if (shapeIdx < OrderFeatureShape::kCheese156_First || OrderFeatureShape::kCherry191_Last < shapeIdx)
			continue;

		bool keep = true;
		if (shapeIdx <= OrderFeatureShape::kCheese159_Last) {
			keep = combo[kMealIngredientCheese04] != 0;
		} else if (shapeIdx <= OrderFeatureShape::kMushrooms163_Last) {
			keep = combo[kMealIngredientMushrooms03] != 0;
		} else if (shapeIdx <= OrderFeatureShape::kPepperoni167_Last) {
			keep = combo[kMealIngredientPepperoni02] != 0;
		} else if (shapeIdx <= OrderFeatureShape::kPeppers171_Last) {
			keep = combo[kMealIngredientPeppers01] != 0;
		} else if (shapeIdx <= OrderFeatureShape::kOlives175_Last) {
			keep = combo[kMealIngredientOlives00] != 0;
		} else if (shapeIdx <= OrderFeatureShape::kPlainSundae179_Last) {
			keep = diffEnabled;
		} else if (shapeIdx <= OrderFeatureShape::kChocolateSauce183_Last) {
			keep = combo[kMealIngredientChocolateSauce07] != 0 && diffEnabled;
		} else if (shapeIdx <= OrderFeatureShape::kWhippedCream187_Last) {
			keep = combo[kMealIngredientWhippedCream06] != 0 && diffEnabled;
		} else {
			keep = combo[kMealIngredientCherry05] != 0 && diffEnabled;
		}

		if (!keep)
			hotspots.remove_at(hotspotIdx);
	}
}

void ZoombiniPuzzlePizza::reloadScrbAnimation(ZmbFeature *feature, int16 scrbId) {
	if (feature) {
		loadScrbOntoFeature(feature, scrbId);
	}
}

// ---------------------------------------------------------------------------
// Get the trait-based index for SCRS and overlay selection.
// Return a value from 0 through 4 based on the Postman's traits.
// ---------------------------------------------------------------------------
int16 ZoombiniPuzzlePizza::getTraitIndexForOrder(int16 orderSlot) const {
	if (!_postmanSnoid)
		return 0;

	(void)orderSlot;

	// Use the same zero-based feet variant as the other Pizza Snoid SCRS IDs.
	return CLIP<int16>(static_cast<int16>(_postmanSnoid->_trait._feet), 1, 5) - 1;
}

} // End of namespace Mohawk
