
extern void initSound();
extern void checkSound(Scumm *s);
extern void handleInput(struct mapledev *pad, int16 &mouse_x, int16 &mouse_y,
			byte &leftBtnPressed, byte &rightBtnPressed,
			int &keyPressed);
extern bool selectGame(Scumm *s, char *&, char *&, class Icon &);

