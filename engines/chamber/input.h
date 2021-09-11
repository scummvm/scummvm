#ifndef _INPUT_H_
#define _INPUT_H_

extern unsigned char buttons;
extern unsigned char right_button;

extern unsigned char have_mouse;

extern volatile unsigned char key_direction;
extern volatile unsigned char key_code;
extern unsigned char key_held;

unsigned char ReadKeyboardChar(void);
void ClearKeyboard(void);

unsigned char PollMouse(void);
unsigned char PollKeyboard(void);
void SetInputButtons(unsigned char keys);

void PollInput(void);
void ProcessInput(void);

void InitInput(void);
void UninitInput(void);

#endif
