/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/set_effects.h"

namespace BladeRunner {

SetEffects::SetEffects(BladeRunnerEngine *vm) {
	_vm = vm;

	_distanceColor.r = 1.0f;
	_distanceColor.g = 1.0f;
	_distanceColor.b = 1.0f;
	_distanceCoeficient = 0.1f;

	_fadeColor.r = 0.0f;
	_fadeColor.g = 0.0f;
	_fadeColor.b = 0.0f;
	_fadeDensity = 0.0f;

	_fogCount = 0;
	_fogs = nullptr;
}

SetEffects::~SetEffects() {
	reset();
}

void SetEffects::read(Common::ReadStream *stream, int frameCount) {
	_distanceCoeficient = stream->readFloatLE();
	_distanceColor.r = stream->readFloatLE();
	_distanceColor.g = stream->readFloatLE();
	_distanceColor.b = stream->readFloatLE();

	_fogCount = stream->readUint32LE();
	int i;
	for (i = 0; i < _fogCount; ++i) {
		int type = stream->readUint32LE();
		Fog *fog = nullptr;
		switch (type) {
		case 0:
			fog = new FogSphere();
			break;
		case 1:
			fog = new FogCone();
			break;
		case 2:
			fog = new FogBox();
			break;
		default:
			error("Unknown fog type %d", type);
		}
		if (fog != nullptr) {
			fog->read(stream, frameCount);
			fog->_next = _fogs;
			_fogs = fog;
		}
	}
}

void SetEffects::reset() {
	Fog *nextFog;

	if (!_fogs) {
		return;
	}

	do {
		nextFog = _fogs->_next;
		delete _fogs;
		_fogs = nextFog;
	} while (nextFog);
}

void SetEffects::setupFrame(int frame) {
	for (Fog *fog = _fogs; fog != nullptr; fog = fog->_next) {
		fog->setupFrame(frame);
	}
}

void SetEffects::setFadeColor(float r, float g, float b) {
	_fadeColor.r = r;
	_fadeColor.g = g;
	_fadeColor.b = b;
}

void SetEffects::setFadeDensity(float density) {
	_fadeDensity = density;
}

/**
* Set fog color for fog effect named fogName.
* RGB arguments are percentages of red, green and blue
*/
void SetEffects::setFogColor(const Common::String &fogName, float r, float g, float b) {
	Fog *fog = findFog(fogName);
	if (fog == nullptr) {
		return;
	}

	fog->_fogColor.r = r;
	fog->_fogColor.g = g;
	fog->_fogColor.b = b;
}

void SetEffects::setFogDensity(const Common::String &fogName, float density) {
	Fog *fog = findFog(fogName);
	if (fog == nullptr) {
		return;
	}

	fog->_fogDensity = density;
}

void SetEffects::calculateColor(Vector3 viewPosition, Vector3 position, float *outCoeficient, Color *outColor) const {
	float distanceCoeficient = CLIP((position - viewPosition).length() * _distanceCoeficient, 0.0f, 1.0f);

	*outCoeficient = 1.0f - distanceCoeficient;
	outColor->r = _distanceColor.r * distanceCoeficient;
	outColor->g = _distanceColor.g * distanceCoeficient;
	outColor->b = _distanceColor.b * distanceCoeficient;

	for (Fog *fog = _fogs; fog != nullptr; fog = fog->_next) {
		float fogCoeficient = 0.0f;
		fog->calculateCoeficient(position, viewPosition, &fogCoeficient);
		if (fogCoeficient > 0.0f) {
			fogCoeficient = CLIP(fog->_fogDensity * fogCoeficient, 0.0f, 1.0f);

			*outCoeficient = *outCoeficient * (1.0f - fogCoeficient);
			outColor->r = outColor->r * (1.0f - fogCoeficient) + fog->_fogColor.r * fogCoeficient;
			outColor->g = outColor->g * (1.0f - fogCoeficient) + fog->_fogColor.g * fogCoeficient;
			outColor->b = outColor->b * (1.0f - fogCoeficient) + fog->_fogColor.b * fogCoeficient;
		}
	}

	*outCoeficient = *outCoeficient * (1.0f - _fadeDensity);
	outColor->r = outColor->r * (1.0f - _fadeDensity) + _fadeColor.r * _fadeDensity;
	outColor->g = outColor->g * (1.0f - _fadeDensity) + _fadeColor.g * _fadeDensity;
	outColor->b = outColor->b * (1.0f - _fadeDensity) + _fadeColor.b * _fadeDensity;
}

Fog *SetEffects::findFog(const Common::String &fogName) const {
	if (!_fogs) {
		return nullptr;
	}

	Fog *fog = _fogs;

	while (fog != nullptr) {
		if (fogName.compareTo(fog->_name) == 0) {
			break;
		}
		fog = fog->_next;
	}

	return fog;
}

} // End of namespace BladeRunner
