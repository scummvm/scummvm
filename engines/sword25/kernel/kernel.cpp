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
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/system.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/fmv/movieplayer.h"
#include "sword25/input/inputengine.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/persistenceservice.h"
#include "sword25/kernel/service_ids.h"
#include "sword25/package/packagemanager.h"
#include "sword25/script/script.h"
#include "sword25/sfx/soundengine.h"

namespace Sword25 {

#define BS_LOG_PREFIX "KERNEL"

Kernel *Kernel::_instance = 0;

Kernel::Kernel() :
	_pWindow(NULL),
	_running(false),
	_pResourceManager(NULL),
	_initSuccess(false) {

	// Log that the kernel is beign created
	BS_LOGLN("created.");

	// Read the BS_SERVICE_TABLE and prepare kernel structures
	for (uint i = 0; i < ARRAYSIZE(BS_SERVICE_TABLE); i++) {
		// Is the superclass already registered?
		Superclass *pCurSuperclass = NULL;
		Common::Array<Superclass *>::iterator iter;
		for (iter = _superclasses.begin(); iter != _superclasses.end(); ++iter)
			if ((*iter)->getIdentifier() == BS_SERVICE_TABLE[i].superclassId) {
				pCurSuperclass = *iter;
				break;
			}

		// If the superclass isn't already registered, then add it in
		if (!pCurSuperclass)
			_superclasses.push_back(new Superclass(this, BS_SERVICE_TABLE[i].superclassId));
	}

	// Create window object
	_pWindow = Window::createBSWindow(0, 0, 0, 0, false);
	if (!_pWindow) {
		BS_LOG_ERRORLN("Failed to create the window.");
	} else
		BS_LOGLN("Window created.");

	// Create the resource manager
	_pResourceManager = new ResourceManager(this);

	// Initialise the script engine
	ScriptEngine *pScript = static_cast<ScriptEngine *>(newService("script", "lua"));
	if (!pScript || !pScript->init()) {
		_initSuccess = false;
		return;
	}

	// Register kernel script bindings
	if (!registerScriptBindings()) {
		BS_LOG_ERRORLN("Script bindings could not be registered.");
		_initSuccess = false;
		return;
	}
	BS_LOGLN("Script bindings registered.");

	_initSuccess = true;
}

Kernel::~Kernel() {
	// Services are de-registered in reverse order of creation
	while (!_serviceCreationOrder.empty()) {
		Superclass *superclass = getSuperclassByIdentifier(_serviceCreationOrder.top());
		if (superclass)
			superclass->disconnectService();
		_serviceCreationOrder.pop();
	}

	// Empty the Superclass list
	while (_superclasses.size()) {
		delete _superclasses.back();
		_superclasses.pop_back();
	}

	// Release the window object
	delete _pWindow;
	BS_LOGLN("Window destroyed.");

	// Resource-Manager freigeben
	delete _pResourceManager;

	BS_LOGLN("destroyed.");
}

Kernel::Superclass::Superclass(Kernel *pKernel, const Common::String &identifier) :
	_pKernel(pKernel),
	_identifier(identifier),
	_serviceCount(0),
	_activeService(NULL) {
	for (uint i = 0; i < ARRAYSIZE(BS_SERVICE_TABLE); i++)
		if (BS_SERVICE_TABLE[i].superclassId == _identifier)
			_serviceCount++;
}

Kernel::Superclass::~Superclass() {
	disconnectService();
}

/**
 * Gets the identifier of a service with a given superclass.
 * The number of services in a superclass can be learned with GetServiceCount().
 * @param superclassId      The name of the superclass
 *         e.g.: "sfx", "gfx", "package" ...
 * @param Number die Nummer des Services, dessen Bezeichner man erfahren will.<br>
 *         Hierbei ist zu beachten, dass der erste Service die Nummer 0 erhält. Number muss also eine Zahl zwischen
 *         0 und GetServiceCount() - 1 sein.
 */
Common::String Kernel::Superclass::getServiceIdentifier(uint number) {
	if (number > _serviceCount)
		return NULL;

	uint curServiceOrd = 0;
	for (uint i = 0; i < ARRAYSIZE(BS_SERVICE_TABLE); i++) {
		if (BS_SERVICE_TABLE[i].superclassId == _identifier) {
			if (number == curServiceOrd)
				return BS_SERVICE_TABLE[i].serviceId;
			else
				curServiceOrd++;
		}
	}

	return Common::String();
}

/**
 * Creates a new service with the given identifier. Returns a pointer to the service, or null if the
 * service could not be created
 * Note: All services must be registered in service_ids.h, otherwise they cannot be created here
 * @param superclassId      The name of the superclass of the service
 *         e.g.: "sfx", "gfx", "package" ...
 * @param serviceId         The name of the service
 *         For the superclass "sfx" an example could be "Fmod" or "directsound"
 */
Service *Kernel::Superclass::newService(const Common::String &serviceId) {
	for (uint i = 0; i < ARRAYSIZE(BS_SERVICE_TABLE); i++)
		if (BS_SERVICE_TABLE[i].superclassId == _identifier &&
		        BS_SERVICE_TABLE[i].serviceId == serviceId) {
			Service *newService_ = BS_SERVICE_TABLE[i].create(_pKernel);

			if (newService_) {
				disconnectService();
				BS_LOGLN("Service '%s' created from superclass '%s'.", serviceId.c_str(), _identifier.c_str());
				_activeService = newService_;
				_activeServiceName = BS_SERVICE_TABLE[i].serviceId;
				return _activeService;
			} else {
				BS_LOG_ERRORLN("Failed to create service '%s' from superclass '%s'.", serviceId.c_str(), _identifier.c_str());
				return NULL;
			}
		}

	BS_LOG_ERRORLN("Service '%s' is not avaliable from superclass '%s'.", serviceId.c_str(), _identifier.c_str());
	return NULL;
}

/**
 * Ends the current service of a superclass. Returns true on success, and false if the superclass
 * does not exist or if not service was active
 * @param superclassId       The name of the superclass which is to be disconnected
 *         e.g.: "sfx", "gfx", "package" ...
 */
bool Kernel::Superclass::disconnectService() {
	if (_activeService) {
		delete _activeService;
		_activeService = 0;
		BS_LOGLN("Active service '%s' disconnected from superclass '%s'.", _activeServiceName.c_str(), _identifier.c_str());
		return true;
	}

	return false;
}

Kernel::Superclass *Kernel::getSuperclassByIdentifier(const Common::String &identifier) const {
	Common::Array<Superclass *>::const_iterator iter;
	for (iter = _superclasses.begin(); iter != _superclasses.end(); ++iter) {
		if ((*iter)->getIdentifier() == identifier)
			return *iter;
	}

	// BS_LOG_ERRORLN("Superclass '%s' does not exist.", identifier.c_str());
	return NULL;
}

/**
 * Returns the number of register superclasses
 */
uint Kernel::getSuperclassCount() const {
	return _superclasses.size();
}

/**
 * Returns the name of a superclass with the specified index.
 * Note: The number of superclasses can be retrieved using GetSuperclassCount
 * @param Number        The number of the superclass to return the identifier for.
 * It should be noted that the number should be between 0 und GetSuperclassCount() - 1.
 */
Common::String Kernel::getSuperclassIdentifier(uint number) const {
	if (number > _superclasses.size())
		return NULL;

	uint curSuperclassOrd = 0;
	Common::Array<Superclass *>::const_iterator iter;
	for (iter = _superclasses.begin(); iter != _superclasses.end(); ++iter) {
		if (curSuperclassOrd == number)
			return ((*iter)->getIdentifier());

		curSuperclassOrd++;
	}

	return Common::String();
}

/**
 * Returns the number of services registered with a given superclass
 * @param superclassId      The name of the superclass
 *         e.g.: "sfx", "gfx", "package" ...
 */
uint Kernel::getServiceCount(const Common::String &superclassId) const {
	Superclass *pSuperclass = getSuperclassByIdentifier(superclassId);
	if (!pSuperclass)
		return 0;

	return pSuperclass->getServiceCount();

}

/**
 * Gets the identifier of a service with a given superclass.
 * The number of services in a superclass can be learned with GetServiceCount().
 * @param superclassId      The name of the superclass
 *         e.g.: "sfx", "gfx", "package" ...
 * @param Number die Nummer des Services, dessen Bezeichner man erfahren will.<br>
 *         Hierbei ist zu beachten, dass der erste Service die Nummer 0 erhält. Number muss also eine Zahl zwischen
 *         0 und GetServiceCount() - 1 sein.
 */
Common::String Kernel::getServiceIdentifier(const Common::String &superclassId, uint number) const {
	Superclass *pSuperclass = getSuperclassByIdentifier(superclassId);
	if (!pSuperclass)
		return NULL;

	return (pSuperclass->getServiceIdentifier(number));
}

/**
 * Creates a new service with the given identifier. Returns a pointer to the service, or null if the
 * service could not be created
 * Note: All services must be registered in service_ids.h, otherwise they cannot be created here
 * @param superclassId      The name of the superclass of the service
 *         e.g.: "sfx", "gfx", "package" ...
 * @param serviceId         The name of the service
 *         For the superclass "sfx" an example could be "Fmod" or "directsound"
 */
Service *Kernel::newService(const Common::String &superclassId, const Common::String &serviceId) {
	Superclass *pSuperclass = getSuperclassByIdentifier(superclassId);
	if (!pSuperclass)
		return NULL;

	// Die Reihenfolge merken, in der Services erstellt werden, damit sie später in umgekehrter Reihenfolge entladen werden können.
	_serviceCreationOrder.push(superclassId);

	return pSuperclass->newService(serviceId);
}

/**
 * Ends the current service of a superclass. 
 * @param superclassId       The name of the superclass which is to be disconnected
 *         e.g.: "sfx", "gfx", "package" ...
 * @return true on success, and false if the superclass does not exist or if not service was active.
 */
bool Kernel::disconnectService(const Common::String &superclassId) {
	Superclass *pSuperclass = getSuperclassByIdentifier(superclassId);
	if (!pSuperclass)
		return false;

	return pSuperclass->disconnectService();
}

/**
 * Returns a pointer to the currently active service object of a superclass.
 * @param superclassId       The name of the superclass
 *         e.g.: "sfx", "gfx", "package" ...
 */
Service *Kernel::getService(const Common::String &superclassId) {
	Superclass *pSuperclass = getSuperclassByIdentifier(superclassId);
	if (!pSuperclass)
		return NULL;

	return (pSuperclass->getActiveService());
}

/**
 * Returns the name of the currently active service object of a superclass.
 * If an error occurs, then an empty string is returned
 * @param superclassId       The name of the superclass
 *         e.g.: "sfx", "gfx", "package" ...
 */
Common::String Kernel::getActiveServiceIdentifier(const Common::String &superclassId) {
	Superclass *pSuperclass = getSuperclassByIdentifier(superclassId);
	if (!pSuperclass)
		return Common::String();

	return pSuperclass->getActiveServiceName();
}

/**
 * Returns a random number
 * @param Min       The minimum allowed value
 * @param Max       The maximum allowed value
 */
int Kernel::getRandomNumber(int min, int max) {
	BS_ASSERT(min <= max);

	return min + _rnd.getRandomNumber(max - min + 1);
}

/**
 * Returns the elapsed time since startup in milliseconds
 */
uint Kernel::getMilliTicks() {
	return g_system->getMillis();
}

/**
 * Returns how much memory is being used
 */
size_t Kernel::getUsedMemory() {
	return 0;
}

/**
 * Returns a pointer to the active Gfx Service, or NULL if no Gfx service is active.
 */
GraphicEngine *Kernel::getGfx() {
	return static_cast<GraphicEngine *>(getService("gfx"));
}

/**
 * Returns a pointer to the active Sfx Service, or NULL if no Sfx service is active.
 */
SoundEngine *Kernel::getSfx() {
	return static_cast<SoundEngine *>(getService("sfx"));
}

/**
 * Returns a pointer to the active input service, or NULL if no input service is active.
 */
InputEngine *Kernel::getInput() {
	return static_cast<InputEngine *>(getService("input"));
}

/**
 * Returns a pointer to the active package manager, or NULL if no manager is active.
 */
PackageManager *Kernel::getPackage() {
	return static_cast<PackageManager *>(getService("package"));
}

/**
 * Returns a pointer to the script engine, or NULL if it is not active.
 */
ScriptEngine *Kernel::getScript() {
	return static_cast<ScriptEngine *>(getService("script"));
}

/**
 * Returns a pointer to the movie player, or NULL if it is not active.
 */
MoviePlayer *Kernel::getFMV() {
	return static_cast<MoviePlayer *>(getService("fmv"));
}

void Kernel::sleep(uint msecs) const {
	g_system->delayMillis(msecs);
}

} // End of namespace Sword25
