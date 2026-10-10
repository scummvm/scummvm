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

#ifndef MOHAWK_ZOOMBINI_PAGES_PUZZLE_BASE_H
#define MOHAWK_ZOOMBINI_PAGES_PUZZLE_BASE_H

#include "mohawk/zoombini_pages/interactive_base.h"

namespace Mohawk {

enum class ZmbSfxGroupFlags : int16;

/**
 * Puzzle difficulty level (1-based, 1-4).
 * @ref ZoombiniGameState::readPageRouteLevel() returns 0-3; add one to convert.
 */
enum ZmbPuzzleLevel : int16 {
	/** Level1: "Not So Easy" */
	kPuzzleLevel1 = 1,
	/** Level2: "Oh So Hard" */
	kPuzzleLevel2 = 2,
	/** Level3: "Very Hard" */
	kPuzzleLevel3 = 3,
	/** Level4: "Very Very Hard" */
	kPuzzleLevel4 = 4
};

/**
 * Base class for the 12 puzzle pages.
 * Three puzzles form each route, and the player plays them in sequence.
 */
class ZoombiniPuzzle : public ZoombiniInteractive {
protected:
	/** Restore the shared frame-update flag when a page update scope exits. */
	class PuzzleUpdateGuard {
	public:
		explicit PuzzleUpdateGuard(bool &isUpdating) : _isUpdating(isUpdating) { _isUpdating = true; }
		~PuzzleUpdateGuard() { _isUpdating = false; }

	private:
		bool &_isUpdating;
		PuzzleUpdateGuard(const PuzzleUpdateGuard &) = delete;
		PuzzleUpdateGuard &operator=(const PuzzleUpdateGuard &) = delete;
	};

	/** Puzzle difficulty level (1-4). */
	ZmbPuzzleLevel _difficultyLevel = kPuzzleLevel1;
	/** Transient SFX selector computed once for this puzzle-page entry. */
	ZmbSfxGroupFlags _entrySfxGroupFlags;
	/** Whether @ref ZoombiniPuzzle::_entrySfxGroupFlags has been computed for this page entry. */
	bool _entrySfxGroupFlagsInitialized = false;
	/** Guard to prevent reentrant frame processing during updates. */
	bool _isUpdating = false;
	/** Total number of Zoombinis materialized into the puzzle. */
	int16 _pageLoadedZmbCount = 0;
	/** Body arrangement queued by Snoid SCRS events 240-243. */
	int16 _pendingBodyArrangement = 0;
	/** Maximum page visit count that always enables partial-result feedback. */
	static constexpr uint16 kPartialResultFeedbackEarlyVisitCount = 3;
	// Celebration and idle state remains page-owned because candidate selection,
	// trigger conditions, pool semantics, and completion rules differ per page.

	/** Configure the standard Go, Map, and Help control rectangles. */
	void configureStandardPuzzleControlRects();
	/** Configure the standard Go and Map controls with a page-specific Help rectangle. */
	void configureStandardPuzzleControlRects(const Common::Rect &helpRect);
	/** Load the standard Go, Map, and Help control features. */
	void loadStandardPuzzleControlFeatures(int16 bitmapResId);
	/** Create the standard hidden overlay head used by puzzle feature chains. */
	ZmbFeature *createPuzzleMainFeatureHead();
	/** Select one full-completion Narrator resource ID. */
	int16 selectCompletionNarratorSoundId();
	/** Select one partial-success Narrator resource ID. */
	int16 selectPartialSuccessNarratorSoundId();
	/** Queue one previously selected ownerless Narrator resource with the Speech volume category. */
	void queueNarratorSound(int16 soundId);
	/**
	 * Evaluate the common probability gate for incomplete puzzle result feedback.
	 *
	 * Draws one value from 0 through 4 before applying the early-visit override.
	 */
	bool passesPartialResultFeedbackGate();
	/** Select and queue one ownerless full-completion Narrator sound. */
	void queueCompletionNarratorSound();
	/** Select and queue one ownerless partial-success Narrator sound. */
	void queuePartialSuccessNarratorSound();

public:
	/** Create a puzzle page for @p pageType. */
	ZoombiniPuzzle(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType, ZmbSrcPageKind departXferSrcSiPage);
	/** Release shared puzzle runners and interactive resources. */
	~ZoombiniPuzzle() override;
	/** Return the puzzle name used by debug answer descriptions. */
	virtual const char *getPageName() const = 0;
	/** Return the 1-based route number containing this puzzle. */
	virtual int getRouteNumber() const = 0;
	/** Return the 1-based puzzle number within its route. */
	virtual int getRoutePuzzleIdx() const = 0;
	/** Return the standard banner used by puzzle debug answers. */
	Common::String getDebugBanner() const;
	/** Compute and cache the page-entry SFX selector, including its page-flag update, exactly once. */
	void initSfxGroupFlags() override;
	bool canOpenSaveLoadDialog() const override { return true; }

	/** Return a page-specific rule description for debug output. */
	virtual Common::String debugGetAnswer() const { return "(no answer available)"; }

	/**
	 * Puzzles default to the "amorphous" chance type (a puzzle that does not
	 * count chances). Chance-counting puzzles override this.
	 */
	ZmbChanceInfo debugGetChances() const override;

protected:
	/** Confirm a map transition when the puzzle permits leaving. */
	bool confirmMapTransition() override;
	/** Save the puzzle state before the map transition is committed. */
	void saveStateBeforeMapTransition() override;

	/**
	 * Materialize the occupied entries of the active pack as Snoid runners.
	 * Puzzle pages honor the occupied-animation skip flag and consume the active-pack count.
	 */
	int16 loadOccupiedSnoidsFromActivePack(const Common::Point *positions,
										   uint16 positionCount,
										   Common::Array<ZmbSnoid *> *loadedSnoids = nullptr);

	/**
	 * Materialize occupied active-pack entries as registered Snoid runners and
	 * update @ref ZoombiniPuzzle::_pageLoadedZmbCount.
	 *
	 * This is a destructive ownership transfer: the shared loader clears the
	 * serialized active-pack entries after copying their traits and names into
	 * runners. Keep each call at its page-specific position in
	 * @ref ZoombiniPage::loadFeatures() so serialized consumers and runner
	 * registration order remain intact. Do not move it into
	 * @ref ZoombiniPage::initStates() or another pre-feature initialization hook.
	 */
	int16 loadZoombinisFromPack(const Common::Point *positions, uint16 positionCount);

	/** Collect real pack Snoids in registration order, excluding SCRS pools. */
	void collectPackSnoids(Common::Array<ZmbSnoid *> &snoids, bool occupiedOnly = false) const;

	/** Queue or immediately apply a body-arrangement SCRS event. */
	bool handleBodyArrangementScriptEvent(ZmbSnoid &snoid, int16 eventCode);
	/** Apply and clear the body arrangement queued for the next facing event. */
	bool applyPendingBodyArrangement(ZmbSnoid &snoid);
};

} // End of namespace Mohawk

#endif
