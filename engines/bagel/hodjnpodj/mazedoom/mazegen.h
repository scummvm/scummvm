//	Defines for Maze generation routines:

#define MAX_MAZE_SIZE_X	12
#define MAX_MAZE_SIZE_Y	9

#define MOVE_LIST_SIZE  (MAX_MAZE_SIZE_X * MAX_MAZE_SIZE_Y)
  
#define WALL_TOP	0x8000
#define WALL_RIGHT	0x4000
#define WALL_BOTTOM	0x2000
#define WALL_LEFT	0x1000
  
#define DOOR_IN_TOP		0x800
#define DOOR_IN_RIGHT	0x400
#define DOOR_IN_BOTTOM	0x200
#define DOOR_IN_LEFT	0x100
#define DOOR_IN_ANY		0xF00
  
#define DOOR_OUT_TOP	0x80
#define DOOR_OUT_RIGHT	0x40
#define DOOR_OUT_BOTTOM	0x20
#define DOOR_OUT_LEFT	0x10
  
#define START_SQUARE	0x2
#define END_SQUARE		0x1
  

static unsigned short _maze[MAX_MAZE_SIZE_X][MAX_MAZE_SIZE_Y];

static struct {
  unsigned char x;
  unsigned char y;
  unsigned char dir;
} _moveList[MOVE_LIST_SIZE];

static int _mazeSizeX, _mazeSizeY;
static int _sqNum, _curSqX, _curSqY;
static int _startX, _startY, _endX, _endY;
