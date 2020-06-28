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

#ifndef WINTERMUTE_AD_ATTACH_3DX_H
#define WINTERMUTE_AD_ATTACH_3DX_H

#include "engines/wintermute/ad/ad_object_3d.h"

namespace Wintermute {

class AdAttach3DX : public AdObject3D {
public:
	DECLARE_PERSISTENT(AdAttach3DX, AdObject3D)

	AdAttach3DX(BaseGame *inGame, BaseObject *owner);
	virtual ~AdAttach3DX();

	bool init(const char *modelFile, const char *name, const char *parentBone);
	bool update() override;
	bool displayAttachable(const Math::Matrix4 &viewMat, bool registerObjects);
	bool displayShadowVol(const Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth, bool update);
	bool invalidateDeviceObjects() override;
	bool restoreDeviceObjects() override;

	Common::String getParentBone();

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;

private:
	BaseObject *_owner;
	Common::String _parentBone;
};

} // namespace Wintermute

#endif
