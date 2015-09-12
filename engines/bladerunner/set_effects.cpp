#include "bladerunner/set_effects.h"

namespace BladeRunner
{
	SetEffects::SetEffects(BladeRunnerEngine* vm)
	{
		_vm = vm;

		_distanceColor.r = 1.0f;
		_distanceColor.g = 1.0f;
		_distanceColor.b = 1.0f;
		_distanceCoeficient = 0.1f;

		_fadeColor.r = 0.0f;
		_fadeColor.g = 0.0f;
		_fadeColor.b = 0.0f;
		_fadeDensity = 0.0f;

		_fogsCount = 0;
		_fogs = NULL;
	}

	SetEffects::~SetEffects()
	{
		reset();
	}

	void SetEffects::read(Common::ReadStream* stream, int framesCount)
	{
		_distanceCoeficient = stream->readFloatLE();
		_distanceColor.r = stream->readFloatLE();
		_distanceColor.g = stream->readFloatLE();
		_distanceColor.b = stream->readFloatLE();

		_fogsCount = stream->readUint32LE();
		int i;
		for (i = 0; i < _fogsCount; i++)
		{
			int type = stream->readUint32LE();
			Fog* fog = NULL;
			switch(type)
			{
			case 0: 
				fog = new FogCone();
				break;
			case 1:
				fog = new FogSphere();
				break;
			case 2:
				fog = new FogBox();
				break;
			}
			if(!fog)
			{
				//TODO exception, unknown fog type
			}
			fog->read(stream, framesCount);
			fog->_next = _fogs;
			_fogs = fog;
		}
	}

	void SetEffects::reset()
	{
		Fog* fog, *nextFog;
		
		if (!_fogs)
			return;

		do
		{
			fog = _fogs;
			nextFog = fog->_next;
			delete fog;
			fog = nextFog;
		} while (nextFog);

	}

	void SetEffects::setupFrame(int frame)
	{
	}
}