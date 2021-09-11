#ifndef _MENU_H_
#define _MENU_H_

extern unsigned char act_menu_x;
extern unsigned char act_menu_y;

void ActionsMenu(unsigned char **pinfo);
void MenuLoop(unsigned char spotmask, unsigned char spotvalue);
void ProcessMenu(void);

void CheckMenuCommandHover(void);
void CheckPsiCommandHover(void);

#endif
