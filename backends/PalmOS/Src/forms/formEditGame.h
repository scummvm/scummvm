#ifndef __FORMEDITGAME_H__
#define __FORMEDITGAME_H__

// edit game mode
enum {
	edtModeAdd,
	edtModeEdit,
	edtModeParams
};

extern UInt8 gFormEditMode;
void EditGameFormDelete(Boolean direct);

#endif