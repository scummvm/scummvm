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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <math.h>
#include <algorithm>

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/timer.h"
#include "sword25/kernel/service_ids.h"

#include "sword25/gfx/graphicengine.h"
#include "sword25/sfx/soundengine.h"
#include "sword25/input/inputengine.h"
#include "sword25/package/packagemanager.h"
#include "sword25/script/script.h"
#include "sword25/fmv/movieplayer.h"
#include "sword25/kernel/persistenceservice.h"

#define BS_LOG_PREFIX "KERNEL"

BS_Kernel * BS_Kernel::_Instance = 0;

// Konstruktion / Destruktion
// --------------------------
BS_Kernel::BS_Kernel() :
	_pWindow(NULL),
	_Running(false),
	_pResourceManager(NULL),
	_InitSuccess(false)
{
	// TODO:
	// Messagebox ausgeben wenn nicht gelogged werden kann -> log.txt schreibgeschützt
	BS_LOGLN("created.");

	// Feststellen, ob der Timer unterstützt wird.
	if (!BS_Timer::IsTimerAvaliable())
	{
		BS_LOG_ERRORLN("This machine doesn't support a performance counter.");
		return;
	}

	// Die BS_SERVICE_TABLE auslesen und kernelinterne Strukturen vorbereiten
	for (unsigned int i = 0; i < BS_SERVICE_COUNT; i++)
	{
		// Ist die Superclass schon registriert?
		Superclass* pCurSuperclass = NULL;
		std::vector<Superclass*>::iterator Iter;
		for (Iter = _SuperclassList.begin(); Iter != _SuperclassList.end(); ++Iter)
			if ((*Iter)->GetIdentifier() == BS_SERVICE_TABLE[i].SuperclassIdentifier)
			{
				pCurSuperclass = *Iter;
				break;
			}

		// Falls die Superclass noch nicht registriert war, wird dies jetzt gemacht
		if (!pCurSuperclass)
			_SuperclassList.push_back(new Superclass(this, BS_SERVICE_TABLE[i].SuperclassIdentifier));
	}

	// Fensterobjekt erstellen
	_pWindow = BS_Window::CreateBSWindow(0,0,0,0,false);
	if (!_pWindow)
	{
		BS_LOG_ERRORLN("Failed to create the window.");
	}
	else
		BS_LOGLN("Window created.");

	// Resource-Manager erstellen
	_pResourceManager = new BS_ResourceManager(this);

	// Random-Number-Generator initialisieren
	srand(GetMilliTicks());

	// Die Skriptengine initialisieren
	// Die Skriptengine muss bereits von Kernel und nicht vom Benutzer gestartet werden, damit der Kernel seine Funktionen bei seiner Erzeugung
	// registrieren kann.
	BS_ScriptEngine * pScript = static_cast<BS_ScriptEngine *>(NewService("script", "lua"));
	if (!pScript || !pScript->Init())
	{
		_InitSuccess = false;
		return;
	}

	// Scriptbindings des Kernels registrieren
	if (!_RegisterScriptBindings())
	{
		BS_LOG_ERRORLN("Script bindings could not be registered.");
		_InitSuccess = false;
		return;
	}
	BS_LOGLN("Script bindings registered.");

	_InitSuccess = true;
}

BS_Kernel::~BS_Kernel()
{
	// Services in umgekehrter Reihenfolge der Erstellung endladen.
	while (!_ServiceCreationOrder.empty())
	{
		Superclass * superclass = GetSuperclassByIdentifier(_ServiceCreationOrder.top());
		if (superclass) superclass->DisconnectService();
		_ServiceCreationOrder.pop();
	}

	// Superclasslist leeren
	while (_SuperclassList.size())
	{
		delete _SuperclassList.back();
		_SuperclassList.pop_back();
	}
	
	// Fensterobjekt freigeben
	delete _pWindow;
	BS_LOGLN("Window destroyed.");

	// Resource-Manager freigeben
	delete _pResourceManager;

	BS_LOGLN("destroyed.");
}

// Service Methoden
// ----------------
BS_Kernel::Superclass::Superclass (BS_Kernel* pKernel, const std::string& Identifier) :
	_pKernel(pKernel),
	_Identifier(Identifier), 
	_ServiceCount(0), 
	_ActiveService(NULL)
{	
	for (unsigned int i = 0; i < BS_SERVICE_COUNT; i++)
		if (BS_SERVICE_TABLE[i].SuperclassIdentifier == _Identifier)
			_ServiceCount++;
}

BS_Kernel::Superclass::~Superclass()
{
	DisconnectService();
}

std::string BS_Kernel::Superclass::GetServiceIdentifier(unsigned int Number)
{
	if (Number > _ServiceCount) return NULL;
		
	unsigned int CurServiceOrd = 0;
	for (unsigned int i = 0; i < BS_SERVICE_COUNT; i++)
	{
		if (BS_SERVICE_TABLE[i].SuperclassIdentifier == _Identifier)
			if (Number == CurServiceOrd)
				return BS_SERVICE_TABLE[i].ServiceIdentifier;
			else
				CurServiceOrd++;
	}

	return std::string("");
}

BS_Service* BS_Kernel::Superclass::NewService(const std::string& ServiceIdentifier)
{
	for (unsigned int i = 0; i < BS_SERVICE_COUNT; i++)
		if (BS_SERVICE_TABLE[i].SuperclassIdentifier == _Identifier &&
			BS_SERVICE_TABLE[i].ServiceIdentifier == ServiceIdentifier)
		{
			BS_Service* NewService = BS_SERVICE_TABLE[i].CreateMethod(_pKernel);
			
			if (NewService)
			{
				DisconnectService();
				BS_LOGLN("Service '%s' created from superclass '%s'.", ServiceIdentifier.c_str(), _Identifier.c_str());
				_ActiveService = NewService;
				_ActiveServiceName = BS_SERVICE_TABLE[i].ServiceIdentifier;
				return _ActiveService;
			}
			else
			{
				BS_LOG_ERRORLN("Failed to create service '%s' from superclass '%s'.", ServiceIdentifier.c_str(), _Identifier.c_str());
				return NULL;
			}
		}

	BS_LOG_ERRORLN("Service '%s' is not avaliable from superclass '%s'.", ServiceIdentifier.c_str(), _Identifier.c_str());
	return NULL;
}

bool BS_Kernel::Superclass::DisconnectService()
{
	if (_ActiveService)
	{
		delete _ActiveService;
		_ActiveService = 0;
		BS_LOGLN("Active service '%s' disconnected from superclass '%s'.", _ActiveServiceName.c_str(), _Identifier.c_str());
		return true;
	}

	return false;
}

BS_Kernel::Superclass* BS_Kernel::GetSuperclassByIdentifier(const std::string & Identifier)
{
	std::vector<Superclass*>::iterator Iter;
	for (Iter = _SuperclassList.begin(); Iter != _SuperclassList.end(); ++Iter)
	{
		if ((*Iter)->GetIdentifier() == Identifier)
			return *Iter;
	}

	// BS_LOG_ERRORLN("Superclass '%s' does not exist.", Identifier.c_str());
	return NULL;
}

unsigned int BS_Kernel::GetSuperclassCount()
{
	return _SuperclassList.size();
}

std::string BS_Kernel::GetSuperclassIdentifier(unsigned int Number)
{
	if (Number > _SuperclassList.size()) return NULL;
	
	unsigned int CurSuperclassOrd = 0;
	std::vector<Superclass*>::iterator Iter;
	for (Iter = _SuperclassList.begin(); Iter != _SuperclassList.end(); ++Iter)
	{
		if (CurSuperclassOrd == Number)
			return ((*Iter)->GetIdentifier());
		
		CurSuperclassOrd++;
	}
	
	return std::string("");
}

unsigned int BS_Kernel::GetServiceCount(const std::string & SuperclassIdentifier)
{
	Superclass* pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return 0;

	return pSuperclass->GetServiceCount();

}

std::string BS_Kernel::GetServiceIdentifier(const std::string & SuperclassIdentifier, unsigned int Number)
{
	Superclass* pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return NULL;

	return (pSuperclass->GetServiceIdentifier(Number));	
}

BS_Service* BS_Kernel::NewService(const std::string& SuperclassIdentifier, const std::string& ServiceIdentifier)
{
	Superclass* pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return NULL;

	// Die Reihenfolge merken, in der Services erstellt werden, damit sie später in umgekehrter Reihenfolge entladen werden können.
	_ServiceCreationOrder.push(SuperclassIdentifier);

	return pSuperclass->NewService(ServiceIdentifier);
}

bool BS_Kernel::DisconnectService(const std::string& SuperclassIdentifier)
{
	Superclass* pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return false;

	return pSuperclass->DisconnectService();
}

BS_Service* BS_Kernel::GetService(const std::string& SuperclassIdentifier)
{
	Superclass* pSuperclass;
	if (!(pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier))) return NULL;

	return (pSuperclass->GetActiveService());
}

std::string BS_Kernel::GetActiveServiceIdentifier(const std::string& SuperclassIdentifier)
{
	Superclass * pSuperclass = GetSuperclassByIdentifier(SuperclassIdentifier);
	if (!pSuperclass) return std::string("");

	return (pSuperclass->GetActiveServiceName());
}

// -----------------------------------------------------------------------------

int BS_Kernel::GetRandomNumber(int Min, int Max)
{
	BS_ASSERT(Min <= Max);
	unsigned int MaxInternal = (Min - Max + 1) < 0 ? - (Min - Max + 1) : (Min - Max + 1);
	return (rand() % MaxInternal) + Min;
}

// Timer Methoden
// --------------
unsigned int BS_Kernel::GetMilliTicks()
{
	return BS_Timer::GetMilliTicks();
}

uint64_t BS_Kernel::GetMicroTicks()
{
	return BS_Timer::GetMicroTicks();
}

// Sonstige Methoden
// -----------------

size_t BS_Kernel::GetUsedMemory()
{
	PROCESS_MEMORY_COUNTERS pmc;
	pmc.cb = sizeof(pmc);
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		return pmc.WorkingSetSize;
	}
	else
	{
		BS_LOG_ERRORLN("Call to GetProcessMemoryInfo() failed. Error code: %d", GetLastError());
		return 0;
	}
}

// -----------------------------------------------------------------------------

BS_GraphicEngine * BS_Kernel::GetGfx()
{
	return static_cast<BS_GraphicEngine *>(GetService("gfx"));
}

// -----------------------------------------------------------------------------

BS_SoundEngine * BS_Kernel::GetSfx()
{
	return static_cast<BS_SoundEngine *>(GetService("sfx"));
}

// -----------------------------------------------------------------------------

BS_InputEngine * BS_Kernel::GetInput()
{
	return static_cast<BS_InputEngine *>(GetService("input"));
}

// -----------------------------------------------------------------------------

BS_PackageManager * BS_Kernel::GetPackage()
{
	return static_cast<BS_PackageManager *>(GetService("package"));
}

// -----------------------------------------------------------------------------

BS_ScriptEngine * BS_Kernel::GetScript()
{
	return static_cast<BS_ScriptEngine *>(GetService("script"));
}

// -----------------------------------------------------------------------------

BS_MoviePlayer * BS_Kernel::GetFMV()
{
	return static_cast<BS_MoviePlayer *>(GetService("fmv"));
}

// -----------------------------------------------------------------------------

void BS_Kernel::Sleep(unsigned int Msecs) const
{
	::Sleep(Msecs);
}
