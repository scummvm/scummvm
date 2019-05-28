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

#ifndef BLADERUNNER_SET_EFFECTS_H
#define BLADERUNNER_SET_EFFECTS_H

#include "bladerunner/bladerunner.h"
#include "bladerunner/color.h"
#include "bladerunner/fog.h"

#include "common/stream.h"

namespace BladeRunner {

class SetEffects {
	friend class Debugger;

	BladeRunnerEngine *_vm;

	Color _distanceColor;
	float _distanceCoeficient;
	Color _fadeColor;
	float _fadeDensity;
	int   _fogCount;
	Fog  *_fogs;

public:
	SetEffects(BladeRunnerEngine *vm);
	~SetEffects();

	void read(Common::ReadStream *stream, int frameCount);

	void reset();

	void setupFrame(int frame);

	void setFadeColor(float r, float g, float b);
	void setFadeDensity(float density);
	void setFogColor(const Common::String &fogName, float r, float g, float b);
	void setFogDensity(const Common::String &fogName, float density);

	void calculateColor(Vector3 viewPosition, Vector3 position, float *outCoeficient, Color *outColor) const;

private:
	Fog *findFog(const Common::String &fogName) const;
};

} // End of namespace BladeRunner

#endif
