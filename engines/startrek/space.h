// Pseudo-3D structs

struct Point3 {
	int32 x;
	int32 y;
	int32 z;

	Point3 operator+(const Point3 &p) const {
		Point3 p2;
		p2.x = x + p.x;
		p2.y = y + p.y;
		p2.z = z + p.z;
		return p2;
	}
	Point3 operator-(const Point3 &p) const {
		Point3 p2;
		p2.x = x - p.x;
		p2.y = y - p.y;
		p2.z = z - p.z;
		return p2;
	}
};

struct Point3W {
	int16 x;
	int16 y;
	int16 z;
};

struct Star {
	bool active;
	Point3 pos;
};

// Struct for objects in space.
// TODO: what does this stand for? Maybe rename it.
struct R3 {
	Point3 pos; // 0x0
	int16 field1e; // 0x1e
};

// Maximum number of stars visible at once in the starfields
#define NUM_STARS 16

// Maximum number of R3 objects in space at once
#define NUM_SPACE_OBJECTS 0x30
