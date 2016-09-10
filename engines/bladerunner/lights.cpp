#include "bladerunner/lights.h"

namespace BladeRunner {

Lights::Lights(BladeRunnerEngine *vm)
{
	_vm = vm;

	_ambientLightColor.r = 1.0;
	_ambientLightColor.g = 0.0;
	_ambientLightColor.b = 0.0;

	_lights = NULL;
	_frame = 0;
}

Lights::~Lights()
{
	reset();
}

void Lights::read(Common::ReadStream* stream, int framesCount)
{
	_ambientLightColor.r = stream->readFloatLE();
	_ambientLightColor.g = stream->readFloatLE();
	_ambientLightColor.b = stream->readFloatLE();

	_lightsCount = stream->readUint32LE();
	int i;
	for (i = 0; i < _lightsCount; i++)
	{
		Light *light;
		int type = stream->readUint32LE();
		switch (type)
		{
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
			light = new Light5();
			break;
		default:
			light = new Light();
		}

		light->read(stream, framesCount, _frame, 0);
		light->_next = _lights;
		_lights = light;
	}
}

void Lights::readVqa(Common::ReadStream* stream)
{
	reset();
	//int framesCount = stream->readUint32LE();
	//int count = stream->readUint32LE();
}

void Lights::setupFrame(int frame)
{
	Light *light;

	if (frame == _frame)
		return;

	if (!_lights)
		return;

	for (light = _lights; light; light = light->_next)
	{
		light->setupFrame(frame);
	}
}

void Lights::reset()
{
	Light *light;
	Light *nextLight;

	if (!_lights)
		return;

	do
	{
		light = _lights;
		nextLight = light->_next;
		delete light;
		_lights = nextLight;
	} while (nextLight);
}

} // End of namespace BladeRunner
