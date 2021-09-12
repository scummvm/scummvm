#ifndef _PRINT_H_
#define _PRINT_H_

extern unsigned char *cur_str_end;

extern unsigned char draw_x;
extern unsigned char draw_y;

extern unsigned char chars_color_bonw[];
extern unsigned char chars_color_bonc[];
extern unsigned char chars_color_wonb[];
extern unsigned char chars_color_wonc[];

void PrintStringCentered(unsigned char *str, unsigned char *target);
unsigned char *PrintStringPadded(unsigned char *str, unsigned char *target);

void DrawMessage(unsigned char *msg, unsigned char *target);

void CGA_DrawTextBox(unsigned char *msg, unsigned char *target);

#endif
