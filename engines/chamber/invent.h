#ifndef _INVENT_H_
#define _INVENT_H_

extern unsigned char inv_count;
extern unsigned char inv_bgcolor;

void DrawInventoryBox(unsigned short filtermask, unsigned short filtervalue);

void OpenInventory(unsigned short filtermask, unsigned short filtervalue);

#endif
