//
//  Game.cpp
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "common/system.h"
#include "common/debug.h"
#include "graphics/surface.h"
#include "graphics/palette.h"

#include "freescape/game.h"
//#include "GeometricObject.h"
//#include "Matrix.h"
//#include "VertexBuffer.h"

Game::Game(Binary binary) {

}

Game::~Game() {
	delete areasByAreaID;
}
/*
void Game::setAspectRatio(float aspectRatio)
{
	// create a projection matrix; the 16-bit kit permits the range 0-8192 to
	// be used along all three axes and from that comes the far plane distance
	// of 14189.
	Matrix projectionMatrix = Matrix::projectionMatrix(60.0f, aspectRatio, 1.0f, 14189.0f);
	GeometricObject::setProjectionMatrix(projectionMatrix.contents);
}
*/

void Game::draw()
{
	g_system->copyRectToScreen((const void *) _border->data(), _screenW, 0, 0, _screenW, _screenH);
	g_system->updateScreen();
	/*
	// set the clear colour to salmon; we're not catching floor/ceiling
	// colours yet
	glClearColor(1.0f, 0.5f, 0.5f, 1.0f);

	// clear depth and colour
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// establish the view matrix
	GeometricObject::setViewMatrix((rotationMatrix * translationMatrix).contents);

	// 4 is the rocket
	const uint16_t areaNumber = 1;

	// draw once to populate the z buffer without a polygon offset applied
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	(*areasByAreaID)[areaNumber]->draw(false, &batchDrawer);
	batchDrawer.flush();

	// draw with a polygon offset, allowing only reading from the depth buffer —
	// the overall objective here is to allow arbitrary coplanar rendering
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	(*areasByAreaID)[areaNumber]->draw(true, &batchDrawer);
	batchDrawer.flush();
	*/
}

/*
void Game::setupOpenGL()
{
	GeometricObject::setupOpenGL();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glLineWidth(4.0f);

	for(AreaMap::iterator iterator = areasByAreaID->begin(); iterator != areasByAreaID->end(); iterator++)
		iterator->second->setupOpenGL();
}

void Game::advanceToTime(uint32 millisecondsSinceArbitraryMoment)
{
	if(!hasReceivedTime)
	{
		timeOfLastTick = millisecondsSinceArbitraryMoment;
		hasReceivedTime = true;
		return;
	}

	// so how many milliseconds is that since we last paid attention?
	uint32 timeDifference = millisecondsSinceArbitraryMoment - timeOfLastTick;

	// TODO: player movement updates out here
	float velocityMultiplier = (float)timeDifference;
//	position[0] -= velocityMultiplier * (velocity[0] * rotationMatrix.contents[0] + velocity[2] * rotationMatrix.contents[2]);
//	position[1] -= velocityMultiplier * velocity[1];
//	position[2] -= velocityMultiplier * (velocity[0] * rotationMatrix.contents[8] + velocity[2] * rotationMatrix.contents[10]);

	position[0] -= velocityMultiplier * (velocity[0] * rotationMatrix.contents[0] + velocity[1] * rotationMatrix.contents[1] + velocity[2] * rotationMatrix.contents[2]);
	position[1] -= velocityMultiplier * (velocity[0] * rotationMatrix.contents[4] + velocity[1] * rotationMatrix.contents[5] + velocity[2] * rotationMatrix.contents[6]);
	position[2] -= velocityMultiplier * (velocity[0] * rotationMatrix.contents[8] + velocity[1] * rotationMatrix.contents[9] + velocity[2] * rotationMatrix.contents[10]);

	translationMatrix = Matrix::translationMatrix(-position[0], -position[1], -position[2]);

	// we'll advance at 50hz, which makes for some easy integer arithmetic here
	while(timeDifference > 20)
	{
		timeOfLastTick += 20;
		timeDifference -= 20;

		// TODO: in-game timed events here
	}
}*/

/*void Game::rotateView(float x, float y, float z)
{
	rotation[0] -= x;
	rotation[1] -= y;
	rotation[2] -= z;

	Matrix xRotationMatrix = Matrix::rotationMatrix(rotation[0], 1.0f, 0.0f, 0.0f);
	Matrix yRotationMatrix = Matrix::rotationMatrix(rotation[1], 0.0f, 1.0f, 0.0f);
	Matrix zRotationMatrix = Matrix::rotationMatrix(rotation[2], 0.0f, 0.0f, 1.0f);
	rotationMatrix = zRotationMatrix * xRotationMatrix * yRotationMatrix;
}*/

/*void Game::setMovementVelocity(float x, float y, float z)
{
	velocity[0] = x;
	velocity[1] = y;
	velocity[2] = z;
}*/
