#include "bladerunner/lights.h"

namespace BladeRunner {

Lights::Lights(BladeRunnerEngine *vm) {
	_vm = vm;

	_ambientLightColor.r = 1.0;
	_ambientLightColor.g = 0.0;
	_ambientLightColor.b = 0.0;
	
	_lightsCount = 0;
	_lights.clear();
	_frame = 0;
}

Lights::~Lights() {
	reset();
}

void Lights::read(Common::ReadStream *stream, int framesCount) {
	_ambientLightColor.r = stream->readFloatLE();
	_ambientLightColor.g = stream->readFloatLE();
	_ambientLightColor.b = stream->readFloatLE();

	_lightsCount = stream->readUint32LE();
	int i;
	for (i = 0; i < _lightsCount; i++) {
		Light *light;
		int type = stream->readUint32LE();
		switch (type) {
		case 1:
			light = new Light1();
			break;
		case 2:
			light = new Light2();
			break;
		case 3:
			light = new Light3();
			break;
		case 4:
			light = new Light4();
			break;
		case 5:
			light = new LightAmbient();
			break;
		default:
			light = new Light();
		}

		light->read(stream, framesCount, _frame, 0);
		_lights.push_back(light);
	}
}

void Lights::removeAnimated() {
	for (int i = (int)(_lights.size() - 1); i > 0; i--) {
		if (_lights[i]->_animated) {
			delete _lights.remove_at(i);
		}
	}
}

void Lights::readVqa(Common::ReadStream *stream) {
	removeAnimated();
	if (stream->eos())
		return;

	int framesCount = stream->readUint32LE();
	int count = stream->readUint32LE();
	for (int i = 0; i < count; i++) {
		int lightType = stream->readUint32LE();
		Light *light;
		switch (lightType) {
		case 5:
			light = new LightAmbient();
			break;
		case 4:
			light = new Light4();
			break;
		case 3:
			light = new Light3();
			break;
		case 2:
			light = new Light2();
			break;
		case 1:
			light = new Light1();
			break;
		default:
			light = new Light();
		}		
		light->readVqa(stream, framesCount, _frame, 1);
		_lights.push_back(light);
	}
}

void Lights::setupFrame(int frame) {
	if (frame == _frame) {
		return;
	}

	for (uint i = 0; i < _lights.size(); i++) {
		_lights[i]->setupFrame(frame);
	}
}

void Lights::reset() {
	for (int i = (int)(_lights.size() - 1); i > 0; i--) {
		delete _lights.remove_at(i);
	}
	_lights.clear();
}

} // End of namespace BladeRunner
