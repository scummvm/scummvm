#pragma once

typedef unsigned char byte;
typedef short int16;
typedef unsigned short uint16;

struct icon_t {
	int16	sx;
	int16	sy;
	int16	ex;
	int16	ey;
	uint16	cursor_id;          // & 0x8000 - inactive/hidden
	unsigned int	action_id;
	unsigned int	object_id;
};
#define END_ICONS {-1, -1, -1, -1, 0, 0, 0}

struct room_t {
	byte   ff_0;
	byte   exits[4];
	byte   flags;
	uint16  bank;
	uint16  party;
	byte   level;
	byte   video;
	byte   location;
	byte   background;
};
#define END_ROOMS {0xFF, {0xFF, 0xFF, 0xFF, 0xFF}, 0xFF, 0xFFFF, 0xFFFF, 0xFF, 0xFF, 0xFF, 0xFF}
