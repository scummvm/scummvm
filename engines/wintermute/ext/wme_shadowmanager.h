/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WINTERMUTE_SHADOWMANAGER_H
#define WINTERMUTE_SHADOWMANAGER_H

#include "common/str.h"

#include "engines/wintermute/base/base_scriptable.h"

namespace Wintermute {

class AdActor3DX;
class Light3D;

class SXShadowManager : public BaseScriptable {
public:
	DECLARE_PERSISTENT(SXShadowManager, BaseScriptable)
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	SXShadowManager(BaseGame *inGame, ScStack *stack);
	~SXShadowManager() override;
	
private:
	static void callback(void *eventData1, void *eventData2);
	void update();
	void run();
	void stop();
	bool addActor(AdActor3DX *actorObj);
	bool removeAllActors();
	bool enableLight(const char *lightName);
	bool disableLight(const char *lightName);

	Common::List<Common::Pair<AdActor3DX *, uint32>> _actors;
	Common::List<Common::Pair<Light3D *, bool>> _lights;
	uint32 _lastTime{};
	DXVector3 _defaultLightPos;
	float _minShadow;
	float _maxShadow;
	bool _useSmartShadows;
	uint32 _shadowColor;
};

} // End of namespace Wintermute

#endif
