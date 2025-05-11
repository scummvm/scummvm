// For colony class
#define LIFE            1 // '+'
#define NO_LIFE         0 // '-'
#define BOARD_START_ROW	58
#define	BOARD_START_COL	63
//#define BOARD_SPACING 1.5
#define	BOARD_SPACING_TIMES_TWO	3
#define ROWS			15
#define COLS			20


class colony {
	int grid[ROWS][COLS];
	int rows, cols;
public:

	//creator
	colony(int r, int c);
	//destroy construct
	~colony(void);
	//copy operator
	colony& operator=(const colony& c);
	//equality operators: assuming equal dims
	operator==(const colony& c);
	operator!=(const colony& c);

	//accessors
	int row() { return rows; };
	int col() { return cols; };
	int islife(int r, int c) { return grid[r][c]; };
	void evolve(colony c);
	void flip(int r, int c);
	//void print(void);
public:
	int m_nColony_count;
};
