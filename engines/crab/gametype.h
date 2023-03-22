#pragma once

namespace Crab {

#define STATNAME_HEALTH "health"
#define STATNAME_ATTACK "attack"
#define STATNAME_DEFENSE "defense"
#define STATNAME_SPEED "speed"
// #define STATNAME_CHARISMA "charisma"
// #define STATNAME_INTELLIGENCE "intelligence"

namespace pyrodactyl {

namespace stat {

enum StatType {
	STAT_HEALTH,
	STAT_ATTACK,
	STAT_DEFENSE,
	STAT_SPEED,
	/*STAT_CHARISMA,
	STAT_INTELLIGENCE,*/
	STAT_TOTAL
};

} // End of namespace stat

} // End of namespace pyrodactyl

enum Align { ALIGN_LEFT,
			 ALIGN_CENTER,
			 ALIGN_RIGHT };

enum Direction {
	// An invalid direction, used for collisions
	DIRECTION_NONE = -1,

	// South
	DIRECTION_DOWN,

	// North
	DIRECTION_UP,

	// West
	DIRECTION_LEFT,

	// East
	DIRECTION_RIGHT,

	// Also an invalid direction, used for animations
	DIRECTION_TOTAL
};

enum TextureFlipType {
	// Draw texture normally
	FLIP_NONE,

	// Flipped horizontally
	FLIP_X,

	// Flipped vertically
	FLIP_Y,

	// Flipped horizontally and vertically
	FLIP_XY,

	// Flipped anti-diagonally, where anti-diagonal is from top right corner to bottom left
	FLIP_D,

	// Flipped both anti-diagonally and horizontally
	FLIP_DX,

	// Flipped both anti-diagonally and vertically
	FLIP_DY,

	// Flipped anti-diagonally and horizontally and vertically - MAXIMUM FLIP
	FLIP_XYD
};

} // End of namespace Crab
