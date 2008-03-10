#ifndef __RPCKBD_H__
#define __RPCKBD_H__

#include "backends/platform/ps2/iop/rpckbd/include/ps2kbd.h"

typedef kbd_rawkey PS2KbdRawKey;
typedef kbd_keymap PS2KbdKeyMap;

#ifdef __cplusplus
extern "C" {
#endif
	int PS2KbdInit(void);
	int PS2KbdRead(char *key);
	int PS2KbdReadRaw(PS2KbdRawKey *key);
	int PS2KbdSetReadmode(u32 readmode);
	int PS2KbdSetLeds(u8 leds);
	int PS2KbdSetKeymap(PS2KbdKeyMap *keymaps);
	int PS2KbdSetCtrlmap(u8 *ctrlmap);
	int PS2KbdSetAltmap(u8 *altmap);
	int PS2KbdSetSpecialmap(u8 *special);
	int PS2KbdFlushBuffer(void);
	int PS2KbdResetKeymap(void);
#ifdef __cplusplus
}
#endif

#endif

