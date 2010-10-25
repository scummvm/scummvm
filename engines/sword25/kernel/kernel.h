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

/*
 * BS_Kernel
 * ---------
 * This is the main class of the engine.
 * This class creates and manages all other Engine elements: the sound engine, graphics engine ...
 * It is not necessary to release all the items individually, this is performed by the Kernel class.
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_KERNEL_H
#define SWORD25_KERNEL_H

#include "common/scummsys.h"
#include "common/random.h"
#include "common/stack.h"
#include "common/util.h"
#include "engines/engine.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/resmanager.h"

namespace Sword25 {

// Class definitions
class Service;
class GraphicEngine;
class ScriptEngine;
class SoundEngine;
class InputEngine;
class PackageManager;
class MoviePlayer;

/**
 * This is the main engine class
 *
 * This class creates and manages all other engine components such as sound engine, graphics engine ...
 * It is not necessary to release all the items individually, this is performed by the Kernel class.
*/
class Kernel {
public:

	// Service Methods
	// ---------------

	/**
	 * Creates a new service with the given identifier. Returns a pointer to the service, or null if the
	 * service could not be created
	 * Note: All services must be registered in service_ids.h, otherwise they cannot be created here
	 * @param SuperclassIdentifier      The name of the superclass of the service
	 *         z.B: "sfx", "gfx", "package" ...
	 * @param ServiceIdentifier         The name of the service
	 *         For the superclass "sfx" an example could be "Fmod" or "directsound"
	 */
	Service *newService(const Common::String &superclassIdentifier, const Common::String &serviceIdentifier);

	/**
	 * Ends the current service of a superclass. Returns true on success, and false if the superclass
	 * does not exist or if not service was active
	 * @param SuperclassIdentfier       The name of the superclass which is to be disconnected
	 *         z.B: "sfx", "gfx", "package" ...
	 */
	bool disconnectService(const Common::String &superclassIdentifier);

	/**
	 * Returns a pointer to the currently active service object of a superclass
	 * @param SuperclassIdentfier       The name of the superclass
	 *         z.B: "sfx", "gfx", "package" ...
	 */
	Service *getService(const Common::String &superclassIdentifier);

	/**
	 * Returns the name of the currentl active service object of a superclass.
	 * If an error occurs, then an empty string is returned
	 * @param SuperclassIdentfier       The name of the superclass
	 *         z.B: "sfx", "gfx", "package" ...
	 */
	Common::String getActiveServiceIdentifier(const Common::String &superclassIdentifier);

	/**
	 * Returns the number of register superclasses
	 */
	uint getSuperclassCount() const;

	/**
	 * Returns the name of a superclass with the specified index.
	 * Note: The number of superclasses can be retrieved using GetSuperclassCount
	 * @param Number        The number of the superclass to return the identifier for.
	 * It should be noted that the number should be between 0 und GetSuperclassCount() - 1.
	 */
	Common::String getSuperclassIdentifier(uint number) const;

	/**
	 * Returns the number of services registered with a given superclass
	 * @param SuperclassIdentifier      The name of the superclass
	 *         z.B: "sfx", "gfx", "package" ...
	 */
	uint getServiceCount(const Common::String &superclassIdentifier) const;

	/**
	 * Gets the identifier of a service with a given superclass.
	 * The number of services in a superclass can be learned with GetServiceCount().
	 * @param SuperclassIdentifier      The name of the superclass
	 *         z.B: "sfx", "gfx", "package" ...
	 * @param Number die Nummer des Services, dessen Bezeichner man erfahren will.<br>
	 *         Hierbei ist zu beachten, dass der erste Service die Nummer 0 erhält. Number muss also eine Zahl zwischen
	 *         0 und GetServiceCount() - 1 sein.
	 */
	Common::String getServiceIdentifier(const Common::String &superclassIdentifier, uint number) const;

	/**
	 * Returns the elapsed time since startup in milliseconds
	 */
	uint getMilliTicks();

	/**
	 * Specifies whether the kernel was successfully initialised
	 */
	bool getInitSuccess() const {
		return _initSuccess;
	}
	/**
	 * Returns a pointer to the BS_ResourceManager
	 */
	ResourceManager *getResourceManager() {
		return _pResourceManager;
	}
	/**
	 * Returns how much memory is being used
	 */
	size_t getUsedMemory();
	/**
	 * Returns a random number
	 * @param Min       The minimum allowed value
	 * @param Max       The maximum allowed value
	 */
	int getRandomNumber(int min, int max);
	/**
	 * Returns a pointer to the active Gfx Service, or NULL if no Gfx service is active
	 */
	GraphicEngine *getGfx();
	/**
	 * Returns a pointer to the active Sfx Service, or NULL if no Sfx service is active
	 */
	SoundEngine *getSfx();
	/**
	 * Returns a pointer to the active input service, or NULL if no input service is active
	 */
	InputEngine *getInput();
	/**
	 * Returns a pointer to the active package manager, or NULL if no manager is active
	 */
	PackageManager *getPackage();
	/**
	 * Returns a pointer to the script engine, or NULL if it is not active
	 */
	ScriptEngine *getScript();

	/**
	 * Returns a pointer to the movie player, or NULL if it is not active
	 */
	MoviePlayer *getFMV();

	/**
	 * Pauses for the specified amount of time
	 * @param Msecs     The amount of time in milliseconds
	 */
	void sleep(uint msecs) const;

	/**
	 * Returns the singleton instance for the kernel
	 */
	static Kernel *getInstance() {
		if (!_instance)
			_instance = new Kernel();
		return _instance;
	}

	/**
	 * Destroys the kernel instance
	 * This method should only be called when the game is ended. No subsequent calls to any kernel
	 * methods should be done after calling this method.
	 */
	static void deleteInstance() {
		if (_instance) {
			delete _instance;
			_instance = NULL;
		}
	}

	/**
	 * Raises an error. This method is used in crashing testing.
	 */
	void crash() const {
		error("Kernel::Crash");
	}

private:
	// -----------------------------------------------------------------------------
	// Constructor / destructor
	// Private singleton methods
	// -----------------------------------------------------------------------------

	Kernel();
	virtual ~Kernel();

	// -----------------------------------------------------------------------------
	// Singleton instance
	// -----------------------------------------------------------------------------
	static Kernel *_instance;

	// Superclass class
	// ----------------
	class Superclass {
	private:
		Kernel *_pKernel;
		uint _serviceCount;
		Common::String _identifier;
		Service *_activeService;
		Common::String _activeServiceName;

	public:
		Superclass(Kernel *pKernel, const Common::String &identifier);
		~Superclass();

		uint getServiceCount() const {
			return _serviceCount;
		}
		Common::String getIdentifier() const {
			return _identifier;
		}
		Service *getActiveService() const {
			return _activeService;
		}
		Common::String getActiveServiceName() const {
			return _activeServiceName;
		}
		Common::String getServiceIdentifier(uint number);
		Service *newService(const Common::String &serviceIdentifier);
		bool disconnectService();
	};

	Common::Array<Superclass *>  _superclasses;
	Common::Stack<Common::String> _serviceCreationOrder;
	Superclass *getSuperclassByIdentifier(const Common::String &identifier) const;

	bool _initSuccess; // Specifies whether the engine was set up correctly
	bool _running;  // Specifies whether the application should keep running on the next main loop iteration

	// Random number generator
	// -----------------------
	Common::RandomSource _rnd;

	// Resourcemanager
	// ---------------
	ResourceManager *_pResourceManager;

	bool registerScriptBindings();
};

} // End of namespace Sword25

#endif
