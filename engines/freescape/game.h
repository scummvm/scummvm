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
#include "freescape/area.h"
//#include "Matrix.h"
//#include "BatchDrawer.h"

typedef Common::HashMap<uint16, Area*> AreaMap;
class Game
{
	public:

		Game(AreaMap *areasByAreaID);
		virtual ~Game();

		/*void setAspectRatio(float aspectRatio);
		void draw();*/
		//void advanceToTime(uint32 millisecondsSinceArbitraryMoment);

		//void setupOpenGL();

		//void rotateView(float x, float y, float z);
		//void setMovementVelocity(float x, float y, float z);
		
	private:
		uint32 timeOfLastTick;
		bool hasReceivedTime;

		AreaMap *areasByAreaID;

		float rotation[3], velocity[3], position[3];
		//Matrix rotationMatrix;
		//Matrix translationMatrix;

		//BatchDrawer batchDrawer;*/
};

#endif /* defined(__Phantasma__Game__) */
