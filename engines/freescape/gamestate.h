//
//  GameState.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__GameState__
#define __Phantasma__GameState__

//#include <iostream>

class CGameState {
public:
	int32_t getVariable(int32_t variableNumber);
	bool getBit(int32_t bitNumber);
};

#endif /* defined(__Phantasma__GameState__) */
