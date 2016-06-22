#ifndef DM_LOADSAVE_H
#define DM_LOADSAVE_H

#include "dm.h"

namespace DM {

enum LoadgameResponse {
	kLoadgameFailure = -1, // @ CM1_LOAD_GAME_FAILURE
	kLoadgameSuccess = 1// @ C01_LOAD_GAME_SUCCESS
};

class LoadsaveMan {
	DMEngine *_vm;
public:
	explicit LoadsaveMan(DMEngine *vm);

	LoadgameResponse loadgame(); // @ F0435_STARTEND_LoadGame_CPSF
};

}

#endif
