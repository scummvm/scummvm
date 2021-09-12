#include "common.h"
#include "invent.h"
#include "script.h"
#include "cga.h"
#include "cursor.h"
#include "input.h"
#include "sound.h"

/*inventory box cells*/
#define INVENTORY_SPOTS_MAX (4 * 4)

struct {
	unsigned char   sx;
	unsigned char   ex;
	unsigned char   sy;
	unsigned char   ey;
	unsigned char   name;
	unsigned char   unkn5;
	unsigned short  command;
	unsigned char   itemidx;
	unsigned char   unkn9;
} inventory_spots[] = {
	{58, 62,  56,  72, 0, 0, 0, 0, 0},
	{62, 66,  56,  72, 0, 0, 0, 0, 0},
	{66, 70,  56,  72, 0, 0, 0, 0, 0},
	{70, 74,  56,  72, 0, 0, 0, 0, 0},
	{58, 62,  72,  88, 0, 0, 0, 0, 0},
	{62, 66,  72,  88, 0, 0, 0, 0, 0},
	{66, 70,  72,  88, 0, 0, 0, 0, 0},
	{70, 74,  72,  88, 0, 0, 0, 0, 0},
	{58, 62,  88, 104, 0, 0, 0, 0, 0},
	{62, 66,  88, 104, 0, 0, 0, 0, 0},
	{66, 70,  88, 104, 0, 0, 0, 0, 0},
	{70, 74,  88, 104, 0, 0, 0, 0, 0},
	{58, 62, 104, 120, 0, 0, 0, 0, 0},
	{62, 66, 104, 120, 0, 0, 0, 0, 0},
	{66, 70, 104, 120, 0, 0, 0, 0, 0},
	{70, 74, 104, 120, 0, 0, 0, 0, 0}
};


unsigned char inv_count = 0;    /*TODO: pass this as param?*/
unsigned char inv_bgcolor = 0;  /*TODO: pass this as param?*/

/*Filter items and put them inventory box, then draw it if non-empty*/
void DrawInventoryBox(unsigned short filtermask, unsigned short filtervalue) {
	int i;
	unsigned char count = 0;
	for (i = 0; i < MAX_INV_ITEMS; i++) {
		unsigned short flags = (inventory_items[i].flags2 << 8) | inventory_items[i].flags;
		if ((flags & filtermask) != filtervalue)
			continue;
		if (count == 0) {
			/*draw the box*/
			CGA_FillAndWait(inv_bgcolor, 64 / 4, 64, CGA_SCREENBUFFER, CGA_CalcXY_p(232 / 4, 56));
			PlaySound(20);
		}
		inventory_spots[count].name = inventory_items[i].name;
		inventory_spots[count].command = inventory_items[i].command;
		inventory_spots[count].itemidx = i + 1;
		DrawSpriteN(inventory_items[i].sprite, inventory_spots[count].sx, inventory_spots[count].sy, CGA_SCREENBUFFER);
		count++;
	}
	inv_count = count;
}

void CheckInventoryItemHover(unsigned char count) {
	int i;
	for (i = 0; i < count; i++) {
		if (IsCursorInRect((rect_t *)&inventory_spots[i])) {
			the_command = inventory_spots[i].command;
			command_hint = inventory_spots[i].name;
			cursor_color = 0xAA;
			script_byte_vars.inv_item_index = inventory_spots[i].itemidx;
			script_vars[ScrPool3_CurrentItem] = &inventory_items[script_byte_vars.inv_item_index - 1];
			return;
		}
	}
	/*nothing found*/
	command_hint = 100;
	cursor_color = 0xFF;
	the_command = 0;
}

void OpenInventory(unsigned short filtermask, unsigned short filtervalue) {
	the_command = 0;
	CGA_BackupImageReal(CGA_CalcXY_p(232 / 4, 56), 64 / 4, 64);
	DrawInventoryBox(filtermask, filtervalue);
	if (inv_count != 0) {
		SelectCursor(CURSOR_0);
		ProcessInput();
		do {
			PollInput();
			CheckInventoryItemHover(inv_count);
			if (command_hint != last_command_hint)
				DrawCommandHint();
			DrawHintsAndCursor(frontbuffer);
		} while (buttons == 0);
		UndrawCursor(frontbuffer);
	}
	CGA_RestoreImage(scratch_mem2, frontbuffer);
	PlaySound(20);
	switch (((item_t *)script_vars[ScrPool3_CurrentItem])->name) {
	case 108:
	case 115:
	case 117:
		script_byte_vars.byte_17A19 = 1;
		break;
	default:
		script_byte_vars.byte_17A19 = 0;
	}
}
