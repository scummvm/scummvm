struct Sprite {
	uint16 updateCallback;
	uint16 w2;
	uint16 w4;
	uint16 w6;
	uint16 w8;
	uint8  x;
	uint8  y;
	uint16 w12;
	uint8  b14;
	uint8  b15;
	uint16 w16;
	uint8  delay;
	uint8  frame;
	uint16 obj_data;
	uint8  b22;
	uint8  priority;
	uint16 w24;
	uint16 w26;
	uint16 w28;
	uint8  type;
	uint8  hidden;
};

struct ObjData {
	uint8 b0;
	uint8 b1;
	uint8 b2;
	uint8 b3;
	uint8 b4;
	uint8 b5;
	uint8 b6;
	uint8 delay;
	uint8 type;
	uint8 b9;
	uint8 b10;
	uint8 b11;
	uint8 b12;
	uint8 b13;
	uint8 b14;
	uint8 b15;
	uint8 b16;
	uint8 b17;
	uint8 b18[256]; // NB: Don't know the size yet
};


