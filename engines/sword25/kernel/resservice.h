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

#ifndef SWORD25_RESOURCESERVICE_H
#define SWORD25_RESOURCESERVICE_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resmanager.h"

namespace Sword25 {

class Resource;

class ResourceService : public Service {
public:
	ResourceService(Kernel *pKernel) : Service(pKernel) {
		ResourceManager *pResource = pKernel->GetResourceManager();
		pResource->RegisterResourceService(this);
	}

	virtual ~ResourceService() {}


	/**
	 * Loads a resource
	 * @return      Returns the resource if successful, otherwise NULL
	 */
	virtual Resource *LoadResource(const Common::String &FileName) = 0;

	/**
	 * Checks whether the given name can be loaded by the resource service
	 * @param FileName  Dateiname
	 * @return          Returns true if the resource can be loaded.
	 */
	virtual bool CanLoadResource(const Common::String &FileName) = 0;

protected:
	// Alternative methods for classes BS_ResourceService

	/**
	 * Compares two strings, with the second string allowed to contain '*' and '?' wildcards
	 * @param String    The first comparison string. This must not contain wildcards
	 * @param Pattern   The sceond comaprison string. Wildcards of '*' and '?' are allowed.
	 * @return          Returns true if the string matches the pattern, otherwise false.
	 */
	bool _WildCardStringMatch(const Common::String &String, const Common::String &Pattern) {
		return _WildCardStringMatchRecursion(String.c_str(), Pattern.c_str());
	}

private:
	bool _WildCardStringMatchRecursion(const char *String, const char *Pattern) {
		// Checks:
		// 1. The pattern starts with '*' -> TRUE
		if (*Pattern == '*') {
			// Use a copy of the pattern pointer so as not to destroy the current state
			const char *PatternCopy = Pattern;
			while (*PatternCopy == '*') {
				PatternCopy++;
			}
			if (!*PatternCopy) return true;
		}
		// 2. The string is over, but the patern is not -> FALSE
		if (!*String && *Pattern) return false;
		// 3. The string is over, and the pattern is finished -> TRUE
		if (!*String) return true;

		// Recursive check 1:
		// If the two current characters are the same, or pattern '?', then keep scanning
		if (*String == *Pattern || *Pattern == '?') return _WildCardStringMatchRecursion(String + 1, Pattern + 1);

		// Falls nicht, wird untersucht ob ein '*' vorliegt
		if (*Pattern == '*') {
			// Recursive check 2:
			// First the result of strign and pattern + 1 is examined..
			if (_WildCardStringMatchRecursion(String, Pattern + 1)) return true;
			// If this fails, the result of string + 1 pattern is returned
			else return _WildCardStringMatchRecursion(String + 1, Pattern);
			// The recursion ends, therefore, keep returning to this place until a character
			// in the string which corresponds to the '*' in pattern
		}

		// The match has failed
		return false;
	}
};

} // End of namespace Sword25

#endif
