#include	<wind.h>
#include	<dos.h>

#define		Ctrl_Break	0x1B
#define		Ctrl_c		0x23

void interrupt	(*OldCtrlBreak)		();
void interrupt	(*OldCtrl_c)		();





void RestoreBreak (void)
{
  setvect(Ctrl_Break, OldCtrlBreak);
  setvect(Ctrl_c, OldCtrl_c);
}




void DisableBreak (void)
{
  OldCtrlBreak = getvect(Ctrl_Break);
  OldCtrl_c = getvect(Ctrl_c);
  setvect(Ctrl_Break, IretCode);
  setvect(Ctrl_c, IretCode);
}
