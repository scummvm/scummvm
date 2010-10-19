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

// Alle Callbackfunktionen die von Objekten gerufen werden, die persistiert werden können, müssen hier registriert werden.
// Beim Speichern wird statt des Pointers der Bezeichner gespeichert. Beim Laden wird der Bezeichner wieder in einen Pointer umgewandelt.
// Diese Klasse führt also so etwas ähnliches wie eine Importtabelle für Callback-Funktionen.
//
// Dieses Vorgehen hat mehrere Vorteile:
// 1. Die Speicherstände sind plattformunabhängig. Es werden keine Pointer auf Funktionen gespeichert, sondern nur Namen von Callbackfunktionen.
//	  Diese können beim Laden über diese Klasse in systemabhängige Pointer umgewandelt werden.
// 2. Speicherstände können auch nach einem Engineupdate weiterhin benutzt werden. Beim Erstellen einer neun Binary verschieben sich häufig die
//	  Funktionen. Eine Callbackfunktion könnte sich also nach einem Update an einer anderen Stelle befinden als davor. Wenn im Spielstand der
//	  Pointer gespeichert war, stürtzt das Programm beim Äufrufen dieser Callbackfunktion ab. Durch das Auflösungverfahren wird beim Laden der
//	  Callbackbezeichner in den neuen Funktionspointer umgewandelt und der Aufruf kann erfolgen.

#define BS_LOG_PREFIX "CALLBACKREGISTRY"

#include "sword25/kernel/callbackregistry.h"

namespace Sword25 {

bool CallbackRegistry::registerCallbackFunction(const Common::String &name, CallbackPtr ptr) {
	if (name == "") {
		BS_LOG_ERRORLN("The empty string is not allowed as a callback function name.");
		return false;
	}

	if (findPtrByName(name) != 0) {
		BS_LOG_ERRORLN("There is already a callback function with the name \"%s\".", name.c_str());
		return false;
	}
	if (findNameByPtr(ptr) != "") {
		BS_LOG_ERRORLN("There is already a callback function with the pointer 0x%x.", ptr);
		return false;
	}

	storeCallbackFunction(name, ptr);

	return true;
}

CallbackPtr CallbackRegistry::resolveCallbackFunction(const Common::String &name) const {
	CallbackPtr result = findPtrByName(name);

	if (!result) {
		BS_LOG_ERRORLN("There is no callback function with the name \"%s\".", name.c_str());
	}

	return result;
}

Common::String CallbackRegistry::resolveCallbackPointer(CallbackPtr ptr) const {
	const Common::String &result = findNameByPtr(ptr);

	if (result == "") {
		BS_LOG_ERRORLN("There is no callback function with the pointer 0x%x.", ptr);
	}

	return result;
}

CallbackPtr CallbackRegistry::findPtrByName(const Common::String &name) const {
	// Eintrag in der Map finden und den Pointer zurückgeben.
	NameToPtrMap::const_iterator it = _nameToPtrMap.find(name);
	return it == _nameToPtrMap.end() ? 0 : it->_value;
}

Common::String CallbackRegistry::findNameByPtr(CallbackPtr ptr) const {
	// Eintrag in der Map finden und den Namen zurückgeben.
	PtrToNameMap::const_iterator it = _ptrToNameMap.find(ptr);
	return it == _ptrToNameMap.end() ? "" : it->_value;
}

void CallbackRegistry::storeCallbackFunction(const Common::String &name, CallbackPtr ptr) {
	// Callback-Funktion in beide Maps eintragen.
	_nameToPtrMap[name] = ptr;
	_ptrToNameMap[ptr] = name;
}

} // End of namespace Sword25
