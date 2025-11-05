
namespace Pelrock {

const int kRoomStructSize = 104;
const int kNumRooms = 56;

struct Anim {
	int x;
	int y;
	int w;
	int h;
	int nframes;
	int curFrame;
	byte *animData;
};

struct Exit {
	uint16 x;
	uint16 y;
	byte w;
	byte h;
	uint16 targetRoom;
	uint16 targetX;
	uint16 targetY;
	uint16 targetDir;
	byte dir;
	byte flags;
};

struct AnimSet {
	int x;
	int y;
	int w;
	int h;
	int speed;
	int numAnims;
	Anim *animData;
};

struct HotSpot {
	int x;
	int y;
	int id;
	int w;
	int h;
	int extra;
};

struct WalkBox {
	int16 x;
	int16 y;
	int16 w;
	int16 h;
	byte flags;
};

enum GameState {
	GAME = 100,
	MENU = 101,
	CREDITS = 102,
	SAVELOAD = 103,
	SETTINGS = 104,
	EXTRA_SCREEN = 105,
	INTRO = 106,
	PROMOTE = 107,
};

// struct rectCam
// {
//     Common::List<int> vecinos;
//     bool marked;
//     int index;
//     int x;
//     int y;
//     int w;
//     int h;
// };

// struct defCam
// {
//     rectCam cams[];
// };

} // End of namespace Pelrock
