/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_AD_OBJECT_3D_H
#define WINTERMUTE_AD_OBJECT_3D_H

#include "engines/wintermute/ad/ad_object.h"
#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class ShadowVolume;

class AdObject3D : public AdObject {
public:
	bool setupLights();
	bool convert3DTo2D(Math::Matrix4 *worldMat, int32 *posX, int32 *posY);
	bool skipTo(int x, int y, bool tolerant = false);
	char *_tempSkelAnim;
	Math::Vector3d _lastPosVector;
	DECLARE_PERSISTENT(AdObject3D, AdObject)
	bool _dropToFloor;
	bool display() override;
	bool update() override;
	AdObject3D(BaseGame *inGame);
	virtual ~AdObject3D();
	bool getBonePosition2D(const char *boneName, int32 *x, int32 *y);
	bool getBonePosition3D(const char *boneName, Math::Vector3d *pos, Math::Vector3d *offset = nullptr);

	float _velocity;
	float _angVelocity;

	uint32 _ambientLightColor;
	bool _hasAmbientLightColor;

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;

	ShadowVolume *getShadowVolume();

private:
	ShadowVolume *_shadowVolume;
	BaseArray<char *> _ignoredLights;
	void clearIgnoredLights();
	bool addIgnoredLight(char *lightName);
	bool removeIgnoredLight(char *lightName);
};

} // namespace Wintermute

#endif
