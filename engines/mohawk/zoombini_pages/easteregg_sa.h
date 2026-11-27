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
#ifndef MOHAWK_ZOOMBINI_EASTEREGG_SA_H
#define MOHAWK_ZOOMBINI_EASTEREGG_SA_H

#include "mohawk/zoombini_pages/interactive_base.h"

namespace Mohawk {

/** Runs the hidden SA page using the original Picker resources. */
class ZoombiniEasterEggSA : public ZoombiniInteractive {
public:
	/**
	 * Construct an independent SA session.
	 * @param vm The owning Zoombini engine.
	 */
	explicit ZoombiniEasterEggSA(MohawkEngine_Zoombini *vm);
	/** Restore the system cursor if the page is destroyed directly. */
	~ZoombiniEasterEggSA() override;

	/** Open the Picker archive. */
	void open() override;
	/** Start the SA music. */
	void setBackgroundMusic() override;
	/** Install the SA background and palette. */
	void setBackgroundBitmap() override;
	/** Install the background features, crosshair, feedback and score display. */
	void loadFeatures() override;
	/** Restore the system cursor before leaving the page. */
	void close() override;
	/**
	 * Place the crosshair at the current mouse position before drawing.
	 * SCRB 1105 has one frame, so a position change must explicitly refresh its drawn geometry.
	 * At zero zaps the last drawn frame stays in place and marks the restart target.
	 */
	void onAnimFrame() override;
	/**
	 * Handle an exit, restart, catch or miss.
	 * @param absPos Click position in page coordinates.
	 * @param relPos Click displacement.
	 * @return The consumed event result.
	 */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/**
	 * Finish click feedback or release the caught Zoombini at its script marker.
	 * @param feature The runner dispatching the event.
	 * @param eventCode The script event number.
	 */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

protected:
	/** Remove finished walkers and spawn new ones after rendering. */
	void onPostRenderFrame() override;
	/** Leave the minigame and return to RodMap. */
	void onMapButtonActivated() override;
	/** Schedule page sounds through the original script sound queue. */
	bool usesOriginalScriptSoundQueue() const override { return true; }
	/** Assign script sound priority to SA's catch and miss effects. */
	const ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;

private:
	/** Original offset from the crosshair bitmap's top-left corner to its aiming point. */
	static constexpr Common::Point kCrosshairHotspot = Common::Point(14, 13);

	/** Picker resources used by SA. */
	enum PageResourceId : int16 {
		kResBackground1001 = 1001,
		kResBitmapShape1100_Scenery = 1100,
		kResBitmapShape1200_Feedback = 1200,
		kResScrb1100_SceneryBase = 1100,
		kResScrb1104_SceneryLast = 1104,
		kResScrb1105_Crosshair = 1105,
		kResScrb1200_Hit = 1200,
		kResScrb1201_Miss = 1201,
		kResScrb1202_ScorePanel = 1202,
		kResScrb1203_ScoreDigits = 1203,
		kResScrb1204_Restart = 1204,
		kResSound1200_Hit = 1200,
		kResSound1201_Miss = 1201
	};

	/** Callback code from the hit effect's 0xFE01 sound and event marker. */
	enum FeedbackEventCode : int16 {
		kFeedbackEventCode000_RemoveCaught = 0
	};

	/** Clear the score and streak and restore the starting shot budget. */
	void resetRound();
	/** Refresh the score overlay and persist any new high score. */
	void refreshScore();
	/**
	 * Replace the authored digit placeholders with high score, score and remaining shots.
	 * @param feature The score runner.
	 * @param group The active hotspot group.
	 * @param hotspots Draw records for this frame.
	 */
	void updateScoreDigits(ZmbFeature *feature, ZmbHotspotGroup *group, Common::Array<ZmbHotspot> &hotspots);
	/**
	 * Find the frontmost walker and apply the small catch region.
	 * @param point Click position in page coordinates.
	 * @return The walker caught by the click, or nullptr for a miss.
	 */
	ZmbSnoid *findCaughtWalker(const Common::Point &point) const;
	/**
	 * Spend one shot and start the appropriate feedback script.
	 * @param walker The caught walker, or nullptr for a miss.
	 * @param point The click position in page coordinates.
	 */
	void playShot(ZmbSnoid *walker, const Common::Point &point);
	/** Return the current maximum number of concurrent walkers. */
	uint16 getWalkerLimit() const;
	/** Create one temporary Snoid and start its direct walk. */
	ZmbSnoid *spawnWalker();

	/** Crosshair rectangle saved for restarting after the last shot. */
	Common::Rect _restartClickRect;
	/** Three spawn slots, checked in order for expired walkers. */
	ZmbSnoid *_walkingSnoids[3] = {};
	/** A successful catch freezes this walker until the hit script removes it. */
	ZmbSnoid *_caughtSnoid = nullptr;
	/** Crosshair runner used while shots remain. */
	ZmbFeature *_crosshairFeature = nullptr;
	/** Page hit feedback runner. */
	ZmbFeature *_hitFeedbackFeature = nullptr;
	/** Page miss feedback runner. */
	ZmbFeature *_missFeedbackFeature = nullptr;
	/** Feedback runner currently blocking further shots. */
	ZmbFeature *_shotFeedbackFeature = nullptr;
	/** Scoreboard's dynamic digit runner. */
	ZmbFeature *_scoreDigitsFeature = nullptr;
	/** Restart prompt displayed after the last shot. */
	ZmbFeature *_restartPromptFeature = nullptr;
	/** Animation tick after which a walker may spawn. */
	uint32 _nextSpawnFrame = 0;
	/** Successful catches in the current round. */
	int16 _score = 0;
	/** Shots remaining in the current round. */
	int16 _shots = 9;
	/** Consecutive successful catches. */
	int16 _streak = 0;
	/** Maximum number of shots that may be added at the next catch. */
	int16 _shotBonusCapacity = 90;
	/** Animation ticks per step for newly spawned walkers. */
	uint32 _walkInterval = 8;
};

} // End of namespace Mohawk

#endif // MOHAWK_ZOOMBINI_EASTEREGG_SA_H
