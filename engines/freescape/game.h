//
//  Game.h
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Game__
#define __Phantasma__Game__

#include "common/hashmap.h"
#include "common/array.h"
#include "engines/util.h"

#include "freescape/area.h"
#include "freescape.h"

//#include "Matrix.h"
//#include "BatchDrawer.h"

class Game
{
	public:

		Game(Binary binary);
		virtual ~Game();

		//void setAspectRatio(float aspectRatio);
		void draw();
		//void advanceToTime(uint32 millisecondsSinceArbitraryMoment);

		//void setupOpenGL();

		//void rotateView(float x, float y, float z);
		//void setMovementVelocity(float x, float y, float z);
		
	private:
		int _screenW, _screenH;
		Common::Array<uint8> *_border;
		uint32 timeOfLastTick;
		bool hasReceivedTime;

		AreaMap *areasByAreaID;

		float rotation[3], velocity[3], position[3];
		//Matrix rotationMatrix;
		//Matrix translationMatrix;

		//BatchDrawer batchDrawer;*/
};

#endif /* defined(__Phantasma__Game__) */
