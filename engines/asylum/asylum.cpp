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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"

#include "asylum/asylum.h"
#include "asylum/screen.h"
#include "asylum/resource.h"
#include "asylum/graphics.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, Common::Language language)
	: Engine(system) {

	Common::File::addDefaultDirectory(_gameDataDir.getChild("Data"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("Vids"));	

	_eventMan->registerRandomSource(_rnd, "asylum");
}

AsylumEngine::~AsylumEngine() {
	//Common::clearAllDebugChannels();	
	delete _screen;
}

Common::Error AsylumEngine::run() {
	Common::Error err;
	err = init();
	if (err != Common::kNoError)
		return err;
	return go();
}

Common::Error AsylumEngine::init() {
	_screen = new Screen(_system); 

	return Common::kNoError;
}

Common::Error AsylumEngine::go() {
	Resource* res = new Resource;
	
	res->load("res.001");
	res->dump();
    
	GraphicResource *gres = new GraphicResource( res->getResource(0) );
	gres->dump();
	
	
	delete res;	
	delete gres;
	
	return Common::kNoError;
}

} // namespace Asylum
