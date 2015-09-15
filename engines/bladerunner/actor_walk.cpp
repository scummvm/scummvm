#include "bladerunner/actor_walk.h"
#include "bladerunner/actor.h"
#include "bladerunner/scene_objects.h"

namespace BladeRunner
{

	ActorWalk::ActorWalk(BladeRunnerEngine *vm) {
	_vm = vm;
}

ActorWalk::~ActorWalk() {
}

bool ActorWalk::isWalking() {
	return _walking;
}

void ActorWalk::stop(int actorId, bool unknown, int animationMode, int notused) {
	_vm->_sceneObjects->setMoving(actorId, 0); 
	_vm->_actors[actorId]->setMoving(0);

	if(_vm->_actors[actorId]->inCombat()) {
		_vm->_actors[actorId]->changeAnimationMode(animationMode, 0);
	} else {
		_vm->_actors[actorId]->changeAnimationMode(notused, 0);
	}
	
	if(unknown) {
		_walking = 0;
		_running = 0;
		_status = 0;
	}else {
		_walking = 1;
		_running = 0;
		_status = 5;
	}
}

}