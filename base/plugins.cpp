/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "base/gameDetector.h"
#include "base/plugins.h"
#include "base/engine.h"


// Factory functions => no need to include the specific classes
// in this header. This serves two purposes:
// 1) Clean seperation from the game modules (scumm, simon) and the generic code
// 2) Faster (compiler doesn't have to parse lengthy header files)
#ifndef DISABLE_SCUMM
extern const TargetSettings *Engine_SCUMM_targetList();
extern Engine *Engine_SCUMM_create(GameDetector *detector, OSystem *syst);
#endif

#ifndef DISABLE_SIMON
extern Engine *Engine_SIMON_create(GameDetector *detector, OSystem *syst);
extern const TargetSettings *Engine_SIMON_targetList();
#endif

#ifndef DISABLE_SKY
extern const TargetSettings *Engine_SKY_targetList();
extern Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst);
#endif

#ifndef DISABLE_SWORD2
extern const TargetSettings *Engine_SWORD2_targetList();
extern Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst);
#endif


#pragma mark -


PluginManager	*g_pluginManager = 0;


#pragma mark -


int Plugin::countTargets() const {
	const TargetSettings *target = getTargets();
	int count;
	for (count = 0; target->targetName; target++, count++)
		;
	return count;
}

const TargetSettings *Plugin::findTarget(const char *targetName) const {
	// Find the TargetSettings for this target
	const TargetSettings *target = getTargets();
	assert(targetName);
	while (target->targetName) {
		if (!scumm_stricmp(target->targetName, targetName)) {
			return target;
		}
		target++;
	}
	return 0;
}


#pragma mark -


class StaticPlugin : public Plugin {
	const char *_name;
	const TargetSettings *_targets;
	int _targetCount;
	EngineFactory _ef;
public:
	StaticPlugin(const char *name, const TargetSettings *targets, EngineFactory ef)
		: _name(name), _targets(targets), _ef(ef) {
		_targetCount = Plugin::countTargets();
	}

	const char *getName() const					{ return _name; }
	int getVersion() const						{ return 0; }

	int countTargets() const					{ return _targetCount; }
	const TargetSettings *getTargets() const	{ return _targets; }

	Engine *createInstance(GameDetector *detector, OSystem *syst) const {
		return (*_ef)(detector, syst);
	}
};


#pragma mark -


PluginManager::PluginManager() {
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadPlugins();
}

void PluginManager::loadPlugins() {
#ifndef DISABLE_SCUMM
	_plugins.push_back(new StaticPlugin("scumm", Engine_SCUMM_targetList(), Engine_SCUMM_create));
#endif

#ifndef DISABLE_SIMON
	_plugins.push_back(new StaticPlugin("simon", Engine_SIMON_targetList(), Engine_SIMON_create));
#endif

#ifndef DISABLE_SKY
	_plugins.push_back(new StaticPlugin("sky", Engine_SKY_targetList(), Engine_SKY_create));
#endif

#ifndef DISABLE_SWORD2
	_plugins.push_back(new StaticPlugin("sword2", Engine_SWORD2_targetList(), Engine_SWORD2_create));
#endif
}

void PluginManager::unloadPlugins() {
	int i;
	for (i = 0; i < _plugins.size(); i++) {
		_plugins[i]->unloadPlugin();
		delete _plugins[i];
	}
	_plugins.clear();
}
