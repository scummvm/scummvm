// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef SWORD25_SCRIPT_H
#define SWORD25_SCRIPT_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/persistable.h"

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include <string>
#include "sword25/kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

class BS_Kernel;
class BS_OutputPersistenceBlock;
class BS_InputPersistenceBlock;

// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class BS_ScriptEngine : public BS_Service, public BS_Persistable
{
public:
	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// -----------------------------------------------------------------------------
	
	BS_ScriptEngine(BS_Kernel * KernelPtr) : BS_Service(KernelPtr) {};
	virtual ~BS_ScriptEngine() {};

	// -----------------------------------------------------------------------------
	// DIESE METHODEN MÜSSEN VON DER SCRIPTENGINE IMPLEMENTIERT WERDEN
	// -----------------------------------------------------------------------------
	
	/**
		@brief Initialisiert die Scriptengine.
		@return Gibt true bei Erfolg zurück, ansonsten false.
	*/
	virtual bool Init() = 0;

	/**
		@brief Lädt eine Skriptdatei und führt diese aus.
		@param FileName der Dateiname der Skriptdatei
		@return Gibt true bei Erfolg zurück, ansonsten false.
	*/
	virtual bool ExecuteFile(const std::string & FileName) = 0;

	/**
		@brief Führt einen String mit Skriptcode aus.
		@param Code ein String der Skriptcode enthält.
		@return Gibt true bei Erfolg zurück, ansonsten false.
	*/
	virtual bool ExecuteString(const std::string & Code) = 0;

	/**
		@brief Gibt einen Pointer auf das Hauptobjekt der Skriptsprache zurück.
		@remark Durch die Benutzung dieser Methode wird die Kapselung der Sprache vom Rest der Engine aufgehoben.
	*/
	virtual void * GetScriptObject() = 0;

	/**
		@brief Macht die Kommandozeilen-Parameter für die Skriptumgebung zugänglich.
		@param CommandLineParameters ein string vector der alle Kommandozeilenparameter enthält.
		@remark Auf welche Weise die Kommandozeilen-Parameter durch Skripte zugegriffen werden können hängt von der jeweiligen Implementierung ab.
	*/
	virtual void SetCommandLine(const std::vector<std::string> & CommandLineParameters) = 0;

	virtual bool Persist(BS_OutputPersistenceBlock & Writer) = 0;
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader) = 0;
};

#endif
