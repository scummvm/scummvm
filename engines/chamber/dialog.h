#ifndef _DIALOG_H_
#define _DIALOG_H_

extern unsigned int cur_str_index;
extern unsigned int cur_dlg_index;

enum DirtyRectKind {
	DirtyRectFree = 0,
	DirtyRectBubble = 1,	/*bubble with spike*/
	DirtyRectSprite = 2,	/*portrait*/
	DirtyRectText = 3		/*text bubble w/o spike*/
};

typedef struct dirty_rect_t {
unsigned char kind;
unsigned int offs;
unsigned char width;
unsigned char height;
unsigned char y;		/*for DirtyRectBubble this is spike offs*/
unsigned char x;
} dirty_rect_t;

extern dirty_rect_t dirty_rects[];

#define SPIKE_MASK     0xE0
#define SPIKE_UPLEFT   0
#define SPIKE_UPRIGHT  0x20
#define SPIKE_DNRIGHT  0x80
#define SPIKE_DNLEFT   0xA0
#define SPIKE_BUBBLES  0x40
#define SPIKE_BUBRIGHT 0xC0
#define SPIKE_BUBLEFT  0xE0

void AddDirtyRect(unsigned char kind, unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned int offs);
void GetDirtyRectAndFree(int index, unsigned char *kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs);
void GetDirtyRectAndSetSprite(int index, unsigned char *kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs);

void PopDirtyRects(unsigned char kind);
void DrawPersonBubble(unsigned char x, unsigned char y, unsigned char flags, unsigned char *msg);
void DesciTextBox(unsigned int x, unsigned int y, unsigned int width, unsigned char *msg);

void PromptWait(void);

unsigned char * SeekToString(unsigned char *bank, unsigned int num);
unsigned char * SeekToStringScr(unsigned char *bank, unsigned int num, unsigned char **ptr);

#endif
