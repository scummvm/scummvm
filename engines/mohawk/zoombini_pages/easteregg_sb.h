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
#ifndef MOHAWK_ZOOMBINI_EASTEREGG_SB_H
#define MOHAWK_ZOOMBINI_EASTEREGG_SB_H

#include "mohawk/zoombini_pages/interactive_base.h"

namespace Mohawk {

/** Runs the hidden SB page using the original Picker resources. */
class ZoombiniEasterEggSB : public ZoombiniInteractive {
public:
	/**
	 * Construct an independent SB session.
	 * @param vm The owning Zoombini engine.
	 */
	explicit ZoombiniEasterEggSB(MohawkEngine_Zoombini *vm);

	/** Open the Picker archive. */
	void open() override;
	/** Start the SB music. */
	void setBackgroundMusic() override;
	/** Install the palette and fill the playfield. */
	void setBackgroundBitmap() override;
	/** Install the ship and score display. */
	void loadFeatures() override;
	/**
	 * Return to the map.
	 * @param absPos Click position in page coordinates.
	 * @param relPos Click movement relative to the previous position.
	 * @return The consumed event result.
	 */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/**
	 * Apply rotation, thrust, fire and restart controls before shared shortcuts.
	 * @param kbd Translated key and modifiers.
	 * @param kbdRepeat Whether the key is repeating.
	 * @return The shared key dispatch result.
	 */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;
	/**
	 * Record completion of an obstacle's explosion.
	 * @param feature The runner dispatching the event.
	 * @param eventCode The script event number.
	 */
	void onFeatureAnimEvent(ZmbFeature *feature, int16 eventCode) override;

protected:
	/** Apply projectile and obstacle changes after rendering. */
	void onPostRenderFrame() override;
	/** Leave the minigame and return to RodMap. */
	void onMapButtonActivated() override;
	/** Schedule effects through the original script sound queue. */
	bool usesOriginalScriptSoundQueue() const override { return true; }
	/** Return the sound range used by SB's shot and impact effects. */
	const ScriptSoundPriorityRanges &getScriptSoundPriorityRanges() const override;

private:
	/** Picker resources used by SB. */
	enum PageResourceId : int16 {
		kResBackground2000 = 2000,
		kResBitmapShape1000_Game = 1000,
		kResScrb1000_MediumObstacleBase = 1000,
		kResScrb1005_SmallObstacleBase = 1005,
		kResScrb1010_Ship = 1010,
		kResScrb1011_Projectile = 1011,
		kResScrb1012_ScorePanel = 1012,
		kResScrb1013_GameOver = 1013,
		kResScrb1014_ScoreDigits = 1014,
		kResScrb1015_Explosion = 1015,
		kResScrb1016_LargeObstacleBase = 1016,
		kResScrb1021_LeftObstacleBase = 1021,
		kResScrb1026_RightObstacleBase = 1026,
		kResSound3000_ObstacleHit = 3000,
		kResSound3001_ShipHit = 3001,
		kResSound3002_Fire = 3002
	};

	/** The eight indices into @ref ZoombiniEasterEggSB::kDirections, clockwise from north. */
	enum Direction : uint16 {
		kDirection00_North = 0,
		kDirection01_Northeast = 1,
		kDirection02_East = 2,
		kDirection03_Southeast = 3,
		kDirection04_South = 4,
		kDirection05_Southwest = 5,
		kDirection06_West = 6,
		kDirection07_Northwest = 7
	};

	/** Bit mask for wrapping direction indices. */
	enum DirectionMask : uint16 {
		/** Keep the low three bits to wrap a direction index to 0-7. */
		kDirectionMask07 = 0x07
	};

	/** Two-coordinate step and sprite placement for one moving runner. */
	struct Motion {
		/** The page-owned SCRB runner. */
		ZmbFeature *feature = nullptr;
		/** Current sprite anchor in game pixels. */
		Common::Point position;
		/** Signed displacement per update in game pixels. */
		Common::Point velocity;
		/** The first animation tick eligible for the next update. */
		uint32 nextStep = 0;
		/** Current one-based sprite shape, or zero when hidden. */
		int16 shape = 0;
	};

	/** Player ship movement and gameplay state. */
	struct Ship {
		/** Shared movement and sprite state. */
		Motion motion;
		/** Facing direction used for thrust and firing. */
		Direction direction = kDirection00_North;
		/** Death animation frame, zero while alive and 5 after the explosion. */
		uint16 deathFrame = 0;
	};

	/** One of six possible active obstacles. */
	struct Obstacle {
		/** Shared movement and sprite state. */
		Motion motion;
		/** Travel direction used to derive fragment headings on impact. */
		Direction direction = kDirection00_North;
	};

	/** One of three possible active projectiles. */
	struct Projectile {
		/** Shared movement and sprite state. */
		Motion motion;
		/** Age counted in three-tick projectile updates. */
		int16 age = 0;
	};

	/**
	 * Rotate around the eight directions, wrapping at north.
	 * @param direction The starting direction.
	 * @param steps The number of 45-degree steps. Positive values turn clockwise.
	 * @return The resulting direction.
	 */
	static Direction rotateDirection(Direction direction, int16 steps);
	/** Place a fresh ship at the center, facing north with no velocity. */
	void resetShip();
	/** Reset the score and lives after game over, then replace the ship. */
	void restartGame();
	/** Create a projectile one step ahead of the ship. */
	void fireProjectile();
	/**
	 * Register a sprite with the motion callbacks and Picker shape offsets.
	 * @param script The Picker SCRB ID.
	 * @param interval Animation ticks per position update.
	 * @return The page-owned moving feature.
	 */
	ZmbFeature *loadMovingFeature(int16 script, uint32 interval);
	/**
	 * Create an obstacle from a script bank or split one at a given position.
	 * @param baseScript The first of five authored variant scripts.
	 * @param splitPosition The parent obstacle's position when splitting, or nullptr for a new wave.
	 * @param splitDirection Direction for a split obstacle.
	 * @return The new obstacle feature, or nullptr when slots are full.
	 */
	ZmbFeature *spawnObstacle(int16 baseScript, const Common::Point *splitPosition = nullptr, Direction splitDirection = kDirection00_North);
	/** Start the queued explosion and spawn fragments before releasing the hit slot. */
	void splitHitObstacle();
	/**
	 * Advance one motion if its cadence is due.
	 * @param object The moving runner state.
	 * @return Whether an update is due.
	 */
	bool beginMotionStep(Motion &object) const;
	/**
	 * Advance one position and wrap it at the authored playfield boundaries.
	 * @param object The moving runner state.
	 */
	static void advanceMotion(Motion &object);
	/**
	 * Return the first obstacle intersecting a runner's existing click rectangle.
	 * @param feature The moving runner to check.
	 * @return Obstacle slot index, or -1 when there is no collision.
	 */
	int16 findObstacleHit(const ZmbFeature *feature) const;
	/**
	 * Advance ship, obstacle and projectile state in runner registration order.
	 * @param feature The feature about to render.
	 * @return True to run standard SCRB processing after the update.
	 */
	bool tickMovingFeature(ZmbFeature *feature);
	/** Advance ship motion and handle obstacle impacts and death animation. */
	void tickShip();
	/**
	 * Advance projectile motion, lifetime and obstacle hits.
	 * @param projectile The current projectile.
	 */
	void tickProjectile(Projectile &projectile);
	/**
	 * Apply the moving sprite and update its unclipped collision bounds.
	 * @param feature The feature being drawn.
	 * @param group The active hotspot group.
	 * @param hotspots The mutable records for the current draw.
	 */
	void positionMovingShape(ZmbFeature *feature, ZmbHotspotGroup *group, Common::Array<ZmbHotspot> &hotspots);
	/**
	 * Return the motion record associated with a rendered feature.
	 * @param feature The moving feature.
	 * @return Motion state, or nullptr for a static or exploded feature.
	 */
	const Motion *getMotion(const ZmbFeature *feature) const;
	/** Refresh current score, high score and remaining lives. */
	void refreshScore();
	/**
	 * Replace the authored score and life digit placeholders.
	 * @param feature The score runner.
	 * @param group The active hotspot group.
	 * @param hotspots The mutable records for the current draw.
	 */
	void updateScoreDigits(ZmbFeature *feature, ZmbHotspotGroup *group, Common::Array<ZmbHotspot> &hotspots);

	/** Signed unit vectors starting north and rotating clockwise. */
	static const Common::Point kDirections[8];

	/** The player-controlled ship. */
	Ship _ship;
	/** Obstacle slots in collision-test order. */
	Obstacle _obstacles[6];
	/** Number of slots occupied by active obstacles. */
	int16 _obstacleCount = 0;
	/** Projectile slots used until each projectile's final frame ends. */
	Projectile _projectiles[3];
	/** Projectile whose removal is due after rendering this tick. */
	Projectile *_expiredProjectile = nullptr;
	/** Index of the obstacle just hit by a projectile, or -1. */
	int16 _hitObstacleSlot = -1;
	/** Single active obstacle explosion. */
	ZmbFeature *_explosionFeature = nullptr;
	/** The explosion SCRB reached its final callback. */
	bool _explosionFinished = false;
	/** The leftward or rightward obstacle used by an alternating wave. */
	ZmbFeature *_specialObstacleFeature = nullptr;
	/** Whether the last cleared wave scheduled a directional obstacle. */
	bool _alternateWave = false;
	/** Score digit runner. */
	ZmbFeature *_scoreDigitsFeature = nullptr;
	/** Game-over banner, created after the last life is spent. */
	ZmbFeature *_gameOverFeature = nullptr;
	/** Score for this round. */
	uint16 _score = 0;
	/** Points required to award another life. */
	uint16 _nextBonusScore = 100;
	/** Lives remaining in this round. */
	uint16 _lives = 3;
};

} // End of namespace Mohawk

#endif // MOHAWK_ZOOMBINI_EASTEREGG_SB_H
