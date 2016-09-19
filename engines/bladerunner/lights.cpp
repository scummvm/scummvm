#include "bladerunner/lights.h"

namespace BladeRunner {

Lights::Lights(BladeRunnerEngine *vm)
{
	_vm = vm;

	_ambientLightColor.r = 1.0;
	_ambientLightColor.g = 0.0;
	_ambientLightColor.b = 0.0;

	_lights = nullptr;
	_frame = 0;
}

Lights::~Lights()
{
	reset();
}

void Lights::read(Common::ReadStream *stream, int framesCount)
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

void Lights::removeAnimated()
{
	Light **nextLight;
	Light *light; 

	nextLight = &this->_lights;
	light = this->_lights;
	if (light)
	{
		do
		{
			if (light->_animated)
			{
				*nextLight = light->_next;
				delete light;
			}
			else
			{
				nextLight = &light->_next;
			}
			light = *nextLight;
		} while (*nextLight);
	}
}

void Lights::readVqa(Common::ReadStream *stream)
{
	removeAnimated();
	if (stream->eos())
		return;

	int framesCount = stream->readUint32LE();
	int count = stream->readUint32LE();
	for (int i = 0; i < count; i++) {
		int lightType = stream->readUint32LE();
		Light* light;
		switch(lightType)
		{
			case 5:
				light = new Light5();
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
		light->_next = _lights;
		_lights = light;
	}
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
