enum {
	GUI_NONE = 0,
	GUI_TEXT = 1,
	GUI_IMAGE = 2,
	GUI_STAT = 3,
};

enum {
	GWF_BORDER = 1,
	GWF_CLEARBG = 2,
	GWF_PARENT = 4,
	GWF_DELAY = 8,
	GWF_DEFAULT = GWF_BORDER|GWF_CLEARBG,
	GWF_BUTTON = GWF_BORDER|GWF_CLEARBG|GWF_DELAY,

};

struct GuiWidget {
	byte _type;
	byte _page;
	byte _flags;
	int16 _x,_y;
	uint16 _w,_h;
	uint16 _id;
	byte _string_number;
};

struct Gui {
	Scumm *_s;
	const GuiWidget *_widgets[4];
	int _curX, _curY;
	VirtScreen *_vs;
	byte _color,_shadowcolor;
	byte _bgcolor;
	byte _textcolor;
	byte _textcolorhi;
	int _parentX, _parentY;
	byte _active;
	byte _clickTimer;
	byte _cur_page;
	int _clickWidget;
	char *_queryMess;

	/* savedialog specifics */	
	int _slotIndex;
	int _editString;
	int _editLen;
	bool valid_games[9];
	char game_names[9][32];
	void loop();
	void init(Scumm *s);
	void draw(int start, int end);
	void draw(int item) { draw(item,-1); }
	void drawWidget(const GuiWidget *w);
	void moveto(int x, int y);
	void lineto(int x, int y);
	void hline(int x, int y, int x2);
	void vline(int x, int y, int y2);
	void drawString(const char *str, int x, int y, int w, byte color, bool center);
	void widgetClear(const GuiWidget *w);
	void widgetBorder(const GuiWidget *w);
	byte *getBasePtr(int x, int y);
	const GuiWidget *widgetFromPos(int x, int y);
	void leftMouseClick(int x, int y);
	void handleCommand(int cmd);
	void close();
	const char *queryString(int string, int id);
	void getSavegameNames(int start);
	void editString(int index);
	void showCaret(bool show);
	void addLetter(char letter);
	void saveLoadDialog();
	void queryMessage(const char *msg, const char *alts);
	byte getDefaultColor(int color);
};
