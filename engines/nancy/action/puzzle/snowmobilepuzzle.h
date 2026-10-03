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

#ifndef NANCY_ACTION_SNOWMOBILEPUZZLE_H
#define NANCY_ACTION_SNOWMOBILEPUZZLE_H

#include "math/vector2d.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 snowmobile rides. The snowmobile is seen from above, at the center of a view
// that scrolls over an endless snow field. The field has no map: it is made of zones,
// a few fixed ones (the start and finish lines, their scenery) and sets of obstacles
// that get scattered at random around the snowmobile as it moves. Hitting obstacles
// damages the snowmobile, and enough damage ends the ride.
//   kDriving - ride to the zone that leads to the next scene
//   kChase   - keep close to another snowmobile that follows a recorded path
class SnowmobilePuzzle : public PuzzleRecord {
public:
	enum Variant {
		kDriving,
		kChase
	};

	SnowmobilePuzzle(Variant variant) : PuzzleRecord(7), _variant(variant) {}
	virtual ~SnowmobilePuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

	// The crash scene has no art of its own, and plays its explosion over the field
	bool survivesSceneChange(bool nextSceneIsNoArt) const override { return nextSceneIsNoArt; }

protected:
	Common::String getRecordTypeName() const override {
		return _variant == kChase ? "SnowmobileChasePuzzle" : "SnowmobilePuzzle";
	}

	enum ZoneType {
		kZoneChangeScene	= 0x00,
		kZoneSound			= 0x05,	// only plays its sound
		kZoneFlag			= 0x0b,
		kZoneOverlay		= 0x0d,
		kZoneBoundary		= 0x14,
		kZoneDamage			= 0x15,
		kZoneTiledOverlay	= 0x17,	// repeated across the whole width of the view
		kZonePointer		= 0x18,	// arrow at a fixed place in the view, pointing at the zone
		kZoneNoRandomZones	= 0x19
	};

	struct Zone {
		byte type = 0;
		Common::Rect32 rect;
		double minSpeed = 0;		// only applies while the snowmobile's speed is within these
		double maxSpeed = 0;
		int16 conditionFlag = -1;
		byte conditionValue = 0;
		RandomSoundBlock sound;		// played on entering

		uint16 sceneID = kNoScene;	// kZoneChangeScene

		int16 flagLabel = -1;		// kZoneFlag
		byte flagValue = 0;

		int32 minDamage = 0;		// kZoneDamage
		int32 maxDamage = 0;

		// Overlays
		uint imageID = 0;
		Common::Rect32 src;
		Common::Rect32 dest;				// on the field; in the view for kZonePointer
		int32 layer = 0;			// 0 is below the snowmobiles, 1 above
		bool hasImplicitRect = false;

		bool isActive = false;
		Math::Vector2d target;				// kZonePointer
		int32 cellX = 0;			// for zones from a random set
		int32 cellY = 0;

		// Moves the zone so its rect is centered on the point
		void centerOn(const Math::Vector2d &pos);
	};

	struct ZoneSet {
		int32 maxRoll = 0;			// running total of the sets' percentages
		Common::Array<Zone> zones;
	};

	struct Snowmobile {
		Math::Vector2d pos;
		double heading = 0;			// radians, counterclockwise from east
		double speed = 0;			// pixels per second
		double maxSpeed = 0;
		double minSpeed = 0;
		double friction = 0;		// speed lost per tick
		int32 size = 0;				// side of the square that collides with boundaries
		int32 damage = 0;

		Common::Rect32 getBounds() const;
	};

	// The chase is won by getting within reach of the other snowmobile and
	// staying there for a while, several times over
	struct ChaseStage {
		int16 distance = 0;
		uint32 duration = 0;		// ms
		int16 reachedFlag = -1;		// set the first time
		int16 inReachFlag = -1;		// follows the state
		bool isInReach = false;
		uint32 reachedTime = 0;
	};

	struct Waypoint {
		uint32 time = 0;
		Math::Vector2d pos;
		double heading = 0;
	};

	void readZone(Common::SeekableReadStream &stream, Zone &zone);
	void readZones(Common::SeekableReadStream &stream, Common::Array<Zone> &zones);
	uint loadSheet(const Common::String &name);
	void finalizeZone(Zone &zone) const;

	// Random zones
	void getCurrentCell(int32 &cellX, int32 &cellY) const;
	void populateField();
	void updateField();
	bool rollForCell(int32 cellX, int32 cellY) const;
	void spawnSet(int32 cellX, int32 cellY);
	bool isCellExcluded(int32 cellX, int32 cellY) const;

	// Physics
	static double wrapAngle(double angle);
	static double angleOf(const Math::Vector2d &v);
	void setSpeed(double speed);
	void steer(int32 ticks);
	void applyFriction(double ticks);
	void move(double ticks);
	void tryMove(const Math::Vector2d &pos);
	bool isFree() const;
	void update(int32 ticks);

	// Zones
	bool isInZone(const Zone &zone) const;
	void seedZones();
	void updateZones();
	void enterZone(Zone &zone);

	// The path repeats forever, each lap starting where the last one ended
	uint findWaypoint(uint32 time) const;
	Math::Vector2d getPathPos(uint32 time) const;
	bool getPathRange(int32 left, int32 right, int32 &top, int32 &bottom) const;
	void updateChase(int32 ticks);
	bool updateStage(ChaseStage &stage, double distance);

	void changeScene(uint16 sceneID);

	Common::Rect32 getView() const;
	Common::Rect getCarDest(const Common::Rect32 &frame) const;
	void blendPixel(int x, int y, const Graphics::ManagedSurface &image, uint32 color);
	void drawSprite(const Graphics::ManagedSurface &image, const Common::Rect32 &src, int32 destX, int32 destY);
	void drawRotated(const Graphics::ManagedSurface &image, const Common::Rect32 &src, const Common::Rect &dest, double angle);
	void drawZones(int32 layer);
	void redraw();

	Variant _variant;

	// -- File data --
	// The snowmobile's image is _image
	int32 _startX = 0;
	int32 _startY = 0;
	int32 _startAngle = 0;			// degrees
	int32 _forwardSpeed = 0;
	int32 _reverseSpeed = 0;
	int16 _resourceID = 0;			// the zones stop working once this UI resource runs out
	uint16 _crashScene = kNoScene;
	RandomSoundBlock _crashSound;
	RandomSoundBlock _engineSound;
	Common::Rect32 _carSrc;

	Common::Array<ChaseStage> _stages;
	uint16 _winScene = kNoScene;
	int16 _winFlag = -1;
	Common::String _chaserImageName;
	Common::Rect32 _chaserSrc;
	Common::Array<Waypoint> _path;

	int32 _cellSize = 0;
	int32 _density = 0;				// percent chance of a cell getting a set
	Common::Array<ZoneSet> _sets;

	// -- Runtime state --
	Common::Array<Common::String> _sheetNames;
	Common::Array<Graphics::ManagedSurface *> _sheets;
	uint _chaserImageID = 0;

	// The fixed zones, followed by the ones spawned from the sets
	Common::Array<Zone> _zones;
	uint _numFixedZones = 0;
	int32 _cellRadiusX = 0;
	int32 _cellRadiusY = 0;
	int32 _lastCellX = 0;
	int32 _lastCellY = 0;

	SoundDescription _engineSoundDesc;

	Snowmobile _car;
	bool _hasCrashed = false;
	bool _zonesEnabled = true;

	Math::Vector2d _chaserPos;
	double _chaserHeading = 0;
	uint32 _pathTime = 0;
	uint32 _pathDuration = 1;
	Math::Vector2d _pathLap;
	uint _curStage = 0;
	int _pointerZone = -1;

	uint32 _lastTickTime = 0;
	uint16 _inputFlags = 0;
	bool _isMouseSteering = true;
	Common::Point _lastMousePos;
	uint16 _nextScene = kNoScene;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SNOWMOBILEPUZZLE_H
