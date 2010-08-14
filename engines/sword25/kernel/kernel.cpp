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

BS_Kernel *BS_Kernel::_Instance = 0;

BS_Kernel::BS_Kernel() :
	_pWindow(NULL),
	_Running(false),
	_pResourceManager(NULL),
	_InitSuccess(false) {

	// Log that the kernel is beign created
	BS_LOGLN("created.");

	// Read the BS_SERVICE_TABLE and prepare kernel structures
	for (uint i = 0; i < BS_SERVICE_COUNT; i++) {
		// Is the superclass already registered?
		Superclass *pCurSuperclass = NULL;
		Common::Array<Superclass *>::iterator Iter;
		for (Iter = _SuperclassList.begin(); Iter != _SuperclassList.end(); ++Iter)
			if ((*Iter)->GetIdentifier() == BS_SERVICE_TABLE[i].SuperclassIdentifier) {
				pCurSuperclass = *Iter;
				break;
			}

		// If the superclass isn't already registered, then add it in
		if (!pCurSuperclass)
			_SuperclassList.push_back(new Superclass(this, BS_SERVICE_TABLE[i].SuperclassIdentifier));
	}

	// Create window object
	_pWindow = BS_Window::CreateBSWindow(0, 0, 0, 0, false);
	if (!_pWindow) {
		BS_LOG_ERRORLN("Failed to create the window.");
	} else
		BS_LOGLN("Window created.");

	// Create the resource manager
	_pResourceManager = new BS_ResourceManager(this);

	// Initialise the script engine
	BS_ScriptEngine *pScript = static_cast<BS_ScriptEngine *>(NewService("script", "lua"));
	if (!pScript || !pScript->Init()) {
		_InitSuccess = false;
		return;
	}

	// Register kernel script bindings
	if (!_RegisterScriptBindings()) {
		BS_LOG_ERRORLN("Script bindings could not be registered.");
		_InitSuccess = false;
		return;
	}
	BS_LOGLN("Script bindings registered.");

	_InitSuccess = true;
}

BS_Kernel::~BS_Kernel() {
	// Services are de-registered in reverse order of creation
	while (!_ServiceCreationOrder.empty()) {
		Superclass *superclass = GetSuperclassByIdentifier(_ServiceCreationOrder.top());
		if (superclass) superclass->DisconnectService();
		_ServiceCreationOrder.pop();
	}

	// Empty the Superclass list
	while (_SuperclassList.size()) {
		delete _SuperclassList.back();
		_SuperclassList.pop_back();
	}

	// Release the window object
	delete _pWindow;
	BS_LOGLN("Window destroyed.");

	// Resource-Manager freigeben
	delete _pResourceManager;

	BS_LOGLN("destroyed.");
}

// Service Methoden
// ----------------

BS_Kernel::Superclass::Superclass(BS_Kernel *pKernel, const Common::String &Identifier) :
	_pKernel(pKernel),
	_Identifier(Identifier),
	_ServiceCount(0),
	_ActiveService(NULL) {
	for (unsigned int i = 0; i < BS_SERVICE_COUNT; i++)
		if (BS_SERVICE_TABLE[i].SuperclassIdentifier == _Identifier)
			_ServiceCount++;
}

BS_Kernel::Superclass::~Superclass() {
	DisconnectService();
}

/**
 * Gets the identifier of a service with a given superclass.
 * The number of services in a superclass can be learned with GetServiceCount().
 * @param SuperclassIdentifier      The name of the superclass
 *         z.B: "sfx", "gfx", "package" ...
 * @param Number die Nummer des Services, dessen Bezeichner man erfahren will.<br>
 *         Hierbei ist zu beachten, dass der erste Service die Nummer 0 erhält. Number muss also eine Zahl zwischen
 *         0 und GetServiceCount() - 1 sein.
 */
Common::String BS_Kernel::Superclass::GetServiceIdentifier(unsigned int Number) {
	if (Number > _ServiceCount) return NULL;

	unsigned int CurServiceOrd = 0;
	for (unsigned int i = 0; i < BS_SERVICE_COUNT; i++) {
		if (BS_SERVICE_TABLE[i].SuperclassIdentifier == _Identifier)
			if (Number == CurServiceOrd)
				return BS_SERVICE_TABLE[i].ServiceIdentifier;
			else
				CurServiceOrd++;
	}

	return Common::String("");
}

/**
 * Creates a new service with the given identifier. Returns a pointer to the service, or null if the
 * service could not be created
 * Note: All services must be registered in service_ids.h, otherwise they cannot be created here
 * @param SuperclassIdentifier      The name of the superclass of the service
 *         z.B: "sfx", "gfx", "package" ...
 * @param ServiceIdentifier         The name of the service
 *         For the superclass "sfx" an example could be "Fmod" or "directsound"
 */
BS_Service *BS_Kernel::Superclass::NewService(const Common::String &ServiceIdentifier) {
	for (unsigned int i = 0; i < BS_SERVICE_COUNT; i++)
		if (BS_SERVICE_TABLE[i].SuperclassIdentifier == _Identifier &&
		        BS_SERVICE_TABLE[i].ServiceIdentifier == ServiceIdentifier) {
			BS_Service *NewService_ = BS_SERVICE_TABLE[i].CreateMethod(_pKernel);

			if (NewService_) {
				DisconnectService();
				BS_LOGLN("Service '%s' created from superclass '%s'.", ServiceIdentifier.c_str(), _Identifier.c_str());
				_ActiveService = NewService_;
				_ActiveServiceName = BS_SERVICE_TABLE[i].ServiceIdentifier;
				return _ActiveService;
			} else {
				BS_LOG_ERRORLN("Failed to create service '%s' from superclass '%s'.", ServiceIdentifier.c_str(), _Identifier.c_str());
				return NULL;
			}
		}

	BS_LOG_ERRORLN("Service '%s' is not avaliable from superclass '%s'.", ServiceIdentifier.c_str(), _Identifier.c_str());
	return NULL;
}

/**
 * Ends the current service of a superclass. Returns true on success, and false if the superclass
 * does not exist or if not service was active
 * @param SuperclassIdentfier       The name of the superclass which is to be disconnected
 *         z.B: "sfx", "gfx", "package" ...
 */
bool BS_Kernel::Superclass::DisconnectService() {
	if (_ActiveService) {
		delete _ActiveService;
		_ActiveService = 0;
		BS_LOGLN("Active service '%s' disconnected from superclass '%s'.", _ActiveServiceName.c_str(), _Identifier.c_str());
		return true;
	}

	return false;
}

BS_Kernel::Superclass *BS_Kernel::GetSuperclassByIdentifier(const Common::String &Identifier) {
	Common::Array<Superclass *>::iterator Iter;
	for (Iter = _SuperclassList.begin(); Iter != _SuperclassList.end(); ++Iter) {
		if ((*Iter)->GetIdentifier() == Identifier)
			return *Iter;
	}

	// BS_LOG_ERRORLN("Superclass '%s' does not exist.", Identifier.c_str());
	return NULL;
}

/**
 * Returns the number of register superclasses
 */
unsigned int BS_Kernel::GetSuperclassCount() {
	return _SuperclassList.size();
}

/**
 * Returns the name of a superclass with the specified index.
 * Note: The number of superclasses can be retrieved using GetSuperclassCount
 * @param Number        The number of the superclass to return the identifier for.
 * It should be noted that the number should be between 0 und GetSuperclassCount() - 1.
 */
Common::String BS_Kernel::GetSuperclassIdentifier(unsigned int Number) {
	if (Number > _SuperclassList.size()) return NULL;

	unsigned int CurSuperclassOrd = 0;
	Common::Array<Superclass *>::iterator Iter;
	for (Iter = _SuperclassList.begin(); Iter != _SuperclassList.end(); ++Iter) {
		if (CurSuperclassOrd == Number)
			return ((*Iter)->GetIdentifier());

		CurSuperclassOrd++;
	}

	return Common::String("");
}

/**
 * Returns the number of services registered with a given superclass
 * @param SuperclassIdentifier      The name of the superclass
 *         z.B: "sfx", "gfx", "package" ...
 */
unsigned int BS_Kernel::GetServiceCount(const Common::String &SuperclassIdentifier) {
	Superclass *pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier)))
		return 0;

	return pSuperclass->GetServiceCount();

}

/**
 * Gets the identifier of a service with a given superclass.
 * The number of services in a superclass can be learned with GetServiceCount().
 * @param SuperclassIdentifier      The name of the superclass
 *         z.B: "sfx", "gfx", "package" ...
 * @param Number die Nummer des Services, dessen Bezeichner man erfahren will.<br>
 *         Hierbei ist zu beachten, dass der erste Service die Nummer 0 erhält. Number muss also eine Zahl zwischen
 *         0 und GetServiceCount() - 1 sein.
 */
Common::String BS_Kernel::GetServiceIdentifier(const Common::String &SuperclassIdentifier, unsigned int Number) {
	Superclass *pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return NULL;

	return (pSuperclass->GetServiceIdentifier(Number));
}

/**
 * Creates a new service with the given identifier. Returns a pointer to the service, or null if the
 * service could not be created
 * Note: All services must be registered in service_ids.h, otherwise they cannot be created here
 * @param SuperclassIdentifier      The name of the superclass of the service
 *         z.B: "sfx", "gfx", "package" ...
 * @param ServiceIdentifier         The name of the service
 *         For the superclass "sfx" an example could be "Fmod" or "directsound"
 */
BS_Service *BS_Kernel::NewService(const Common::String &SuperclassIdentifier, const Common::String &ServiceIdentifier) {
	Superclass *pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return NULL;

	// Die Reihenfolge merken, in der Services erstellt werden, damit sie später in umgekehrter Reihenfolge entladen werden können.
	_ServiceCreationOrder.push(SuperclassIdentifier);

	return pSuperclass->NewService(ServiceIdentifier);
}

/**
 * Ends the current service of a superclass. Returns true on success, and false if the superclass
 * does not exist or if not service was active
 * @param SuperclassIdentfier       The name of the superclass which is to be disconnected
 *         z.B: "sfx", "gfx", "package" ...
 */
bool BS_Kernel::DisconnectService(const Common::String &SuperclassIdentifier) {
	Superclass *pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return false;

	return pSuperclass->DisconnectService();
}

/**
 * Returns a pointer to the currently active service object of a superclass
 * @param SuperclassIdentfier       The name of the superclass
 *         z.B: "sfx", "gfx", "package" ...
 */
BS_Service *BS_Kernel::GetService(const Common::String &SuperclassIdentifier) {
	Superclass *pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return NULL;

	return (pSuperclass->GetActiveService());
}

/**
 * Returns the name of the currentl active service object of a superclass.
 * If an error occurs, then an empty string is returned
 * @param SuperclassIdentfier       The name of the superclass
 *         z.B: "sfx", "gfx", "package" ...
 */
Common::String BS_Kernel::GetActiveServiceIdentifier(const Common::String &SuperclassIdentifier) {
	Superclass *pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier);
	if (!pSuperclass) return Common::String("");

	return (pSuperclass->GetActiveServiceName());
}

// -----------------------------------------------------------------------------

/**
 * Returns a random number
 * @param Min       The minimum allowed value
 * @param Max       The maximum allowed value
 */
int BS_Kernel::GetRandomNumber(int Min, int Max) {
	BS_ASSERT(Min <= Max);

	return Min + _rnd.getRandomNumber(Max - Min + 1);
}

/**
 * Returns the elapsed time since startup in milliseconds
 */
unsigned int BS_Kernel::GetMilliTicks() {
	return g_system->getMillis();
}

/**
 * Returns the elapsed time since the system start in microseconds.
 * This method should be used only if GetMilliTick() for the desired application is inaccurate.
 */
uint64 BS_Kernel::GetMicroTicks() {
	return g_system->getMillis() * 1000;
}

// Other methods
// -----------------

/**
 * Returns how much memory is being used
 */
size_t BS_Kernel::GetUsedMemory() {
	return 0;

#ifdef SCUMMVM_DISABLED_CODE
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb = sizeof(pmc);
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		return pmc.WorkingSetSize;
	} else {
		BS_LOG_ERRORLN("Call to GetProcessMemoryInfo() failed. Error code: %d", GetLastError());
		return 0;
	}
#endif
}

// -----------------------------------------------------------------------------

/**
 * Returns a pointer to the active Gfx Service, or NULL if no Gfx service is active
 */
BS_GraphicEngine *BS_Kernel::GetGfx() {
	return static_cast<BS_GraphicEngine *>(GetService("gfx"));
}

// -----------------------------------------------------------------------------

/**
 * Returns a pointer to the active Sfx Service, or NULL if no Sfx service is active
 */
BS_SoundEngine *BS_Kernel::GetSfx() {
	return static_cast<BS_SoundEngine *>(GetService("sfx"));
}

// -----------------------------------------------------------------------------

/**
 * Returns a pointer to the active input service, or NULL if no input service is active
 */
BS_InputEngine *BS_Kernel::GetInput() {
	return static_cast<BS_InputEngine *>(GetService("input"));
}

// -----------------------------------------------------------------------------

/**
 * Returns a pointer to the active package manager, or NULL if no manager is active
 */
BS_PackageManager *BS_Kernel::GetPackage() {
	return static_cast<BS_PackageManager *>(GetService("package"));
}

// -----------------------------------------------------------------------------

/**
 * Returns a pointer to the script engine, or NULL if it is not active
 */
BS_ScriptEngine *BS_Kernel::GetScript() {
	return static_cast<BS_ScriptEngine *>(GetService("script"));
}

// -----------------------------------------------------------------------------

/**
 * Returns a pointer to the movie player, or NULL if it is not active
 */
BS_MoviePlayer *BS_Kernel::GetFMV() {
	return static_cast<BS_MoviePlayer *>(GetService("fmv"));
}

// -----------------------------------------------------------------------------

void BS_Kernel::Sleep(unsigned int Msecs) const {
	g_system->delayMillis(Msecs);
}

} // End of namespace Sword25
