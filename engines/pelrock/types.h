
namespace Pelrock {

    const int kRoomStructSize = 104;
    const int kNumRooms = 56;


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
