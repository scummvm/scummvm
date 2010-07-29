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

/*
	BS_Kernel
	---------
	Dies ist die Hauptklasse der Engine.
	Diese Klasse erzeugt und verwaltet alle anderen Enginelemente, wie Soundengine, Graphikengine...
	Es ist nicht notwendig alle Enginenelemente einzeln freizugeben, dieses wird von der Kernelklasse übernommen.

	Autor: Malte Thiesen
*/

#ifndef _BS_KERNEL_H
#define _BS_KERNEL_H

// Includes
#include "memlog_off.h"
#include <vector>
#include <stack>
#include <string>
#include "memlog_on.h"

#include "common.h"
#include "bs_stdint.h"
#include "window.h"
#include "resmanager.h"


// Klassendefinition
class BS_Service;
class BS_GraphicEngine;
class BS_ScriptEngine;
class BS_SoundEngine;
class BS_InputEngine;
class BS_PackageManager;
class BS_MoviePlayer;

/**
	@brief Dies ist die Hauptklasse der Engine.
	
	Diese Klasse erzeugt und verwaltet alle anderen Engineelemente, wie Soundengine, Graphikengine...<br>
	Es ist nicht notwendig alle Enginenelemente einzeln freizugeben, dieses wird von der Kernelklasse übernommen.
*/
class BS_Kernel
{
public:
	// Fenster Methoden
	// ----------------
	/**
		@brief Gibt einen Pointer auf das Fensterobjekt zurück.
		@return Gibt einen Pointer auf das Fensterobjekt zurück.
	*/
	BS_Window* GetWindow() {return _pWindow; }

	// Service Methoden
	// ----------------

	/**
		@brief Erzeugt einen neuen Service der angegebenen Superclass mit dem übergebenen Identifier.
		@param SuperclassIdentifier der Name der Superclass des Services<br>
			   z.B: "sfx", "gfx", "package" ...
		@param ServiceIdentifier der Name des Services<br>
			   Für die Superclass "sfx" könnten das z.B. "fmod" oder "directsound" sein.
		@return Gibt einen Pointer auf den Service zurück, oder NULL wenn der Service nicht erstellt werden konnte.
		@remark Alle Services müssen in service_ids.h eingetragen sein, sonst können sie hier nicht erstellt werden.
	*/
	BS_Service* NewService(const std::string & SuperclassIdentifier, const std::string & ServiceIdentifier);
	/**
		@brief Beendet den aktuellen Service einer Superclass.
		@param SuperclassIdentfier der Name der Superclass dessen aktiver Service beendet werden soll<br>
			   z.B: "sfx", "gfx", "package" ...
		@return Gibt bei Erfolg true zurück und false wenn die Superclass nicht existiert oder wenn kein Service aktiv war.
	*/
	bool DisconnectService(const std::string & SuperclassIdentifier);

	/**
		@brief Gibt einen Pointer auf das momentan aktive Serviceobjekt einer Superclass zurück.
		@param SuperclassIdentfier der Name der Superclass<br>
			   z.B: "sfx", "gfx", "package" ...
		@return Gibt einen Pointer auf den Service zurück, oder NULL wenn kein Service aktiv war.
	*/
	BS_Service* GetService(const std::string& SuperclassIdentifier);

	/**
		@brief Gibt den Namen des aktuell aktiven Serviceobjektes einer Superclass zurück.
		@param SuperclassIdentfier der Name der Superclass<br>
			   z.B: "sfx", "gfx", "package" ...
		@return Gibt den Namen des Serviceobjektes zurück, oder einen leeren String, wenn ein Fehler aufgetreten ist.
	*/
	std::string GetActiveServiceIdentifier(const std::string& SuperclassIdentifier);

	/**
		@brief Gibt die Anzahl der Registrierten Superclasses zurück.
		@return Gibt die Anzahl der Registrierten Superclasses zurück.
	*/
	unsigned int GetSuperclassCount();

	// Gibt den Identifier der mit Number bezeichneten Superclass zurück
	/**
		@brief Gibt den Identifier einer Superclass zurück.
		@param Number die Nummer der Superclass, dessen Bezeichner man erfahren möchte<br>
		       Hierbei ist zu beachten, dass die erste Superclass die Nummer 0 erhält. Number muss also eine Zahl zwischen
			   0 und GetSuperclassCount() - 1 sein.
		@return Gibt den Identifier der Superclass zurück.
		@remark Die Anzahl der Superclasses kann man mit GetSuperclassCount() erfahren.
	*/
	std::string GetSuperclassIdentifier(unsigned int Number);

	// Gibt die Anzahl der für die mit SuperclassIdentifier bezeichneten Superclass vorhandenen
	// Services zurück
	/**
		@brief Gibt die Anzahl an Services zurück, die in einer Superclass registriert sind.
		@param SuperclassIdentifier der Name der Superclass<br>
			   z.B: "sfx", "gfx", "package" ...
		@return Gibt die Anzahl an Services zurück, die in der Superclass registriert sind.
	*/
	unsigned int GetServiceCount(const std::string & SuperclassIdentifier);

	/**
		@brief Gibt den Identifier eines Services in einer Superclass zurück.
		@param SuperclassIdentifier der Name der Superclass<br>
			   z.B: "sfx", "gfx", "package" ...
		@param Number die Nummer des Services, dessen Bezeichner man erfahren will.<br>
			   Hierbei ist zu beachten, dass der erste Service die Nummer 0 erhält. Number muss also eine Zahl zwischen
		       0 und GetServiceCount() - 1 sein.
		@return Gibt den Identifier des Services zurück
		@remark Die Anzahl der Services in einer Superclass kann man mit GetServiceCount() erfahren.
	*/
	std::string GetServiceIdentifier(const std::string & SuperclassIdentifier, unsigned int Number);
	/**
	 	@brief Gibt die vergangene Zeit seit dem Systemstart in Millisekunden zurück.
	 */
	unsigned int GetMilliTicks();
	/**
		@brief Gibt die vergangene Zeit seit dem Systemstart in Microsekunden zurück.
		@remark Diese Methode sollte nur verwendet werden, falls GetMilliTick() für den gewünschten Einsatz zu ungenau ist.
	*/
	uint64_t GetMicroTicks();
	/**
	 	@brief Gibt an, ob die Konstruktion erfolgreich war.
		@return Gibt true zurück, wenn die Konstruktion erfolgreich war.
	 */
	bool GetInitSuccess() { return _InitSuccess; }
	/**
		@brief Gibt einen Pointer auf den BS_ResourceManager zurück.
	*/
	BS_ResourceManager* GetResourceManager() { return _pResourceManager; }
	/**
		@brief Gibt zurück wie viel Speicher von diesem Prozess belegt ist.
	*/
	size_t GetUsedMemory();
	/**
		@brief Gibt eine Zufallszahl zurück.
		@param Min der minimale Wert, den die Zufallszahl haben darf
		@param Max der maximale Wert, den die Zufallszahl haben darf
		@return Gibt eine Zufallszahl zurück, die zwischen Min und Max liegt.
	*/
	int GetRandomNumber(int Min, int Max);
	/**
		@brief Gibt einen Pointer auf den aktiven Gfx-Service zurück oder NULL wenn kein Gfx-Service aktiv.
	*/
	BS_GraphicEngine * GetGfx();
	/**
		@brief Gibt einen Pointer auf den aktiven Sfx-Service zurück oder NULL wenn kein Sfx-Service aktiv.
	*/
	BS_SoundEngine * GetSfx();
	/**
		@brief Gibt einen Pointer auf den aktiven Input-Service zurück oder NULL wenn kein Input-Service aktiv.
	*/
	BS_InputEngine * GetInput();
	/**
		@brief Gibt einen Pointer auf den aktiven Package-Service zurück oder NULL wenn kein Package-Service aktiv.
	*/
	BS_PackageManager * GetPackage();
	/**
		@brief Gibt einen Pointer auf den aktiven Script-Service zurück oder NULL wenn kein Script-Service aktiv.
	*/
	BS_ScriptEngine * GetScript();
	/**
		@brief Gibt einen Pointer auf den aktiven FMV-Service zurück oder NULL wenn kein FMV-Service aktiv.
	*/
	BS_MoviePlayer * GetFMV();

	/**
	    @brief Stoppt den Prozess für eine gewisse Zeit.
		@param Msecs Zeit in Millisekunden, die der Prozess gestoppt werden soll.
		@remark Es wird nicht garantiert, dass der Prozess genau nach der angegebenen Zeit wieder aktiv wird.
	*/
	void Sleep(unsigned int Msecs) const;

	/**
	    @brief Gibt das einzige Exemplar des Kernels zurück (Singleton).
	*/
	
	static BS_Kernel * GetInstance()
	{
		if (!_Instance) _Instance = new BS_Kernel();
		return _Instance;
	}

	/**
	    @brief Zerstört das einzige Exemplar des Kernels.
		@remark Diese Methode darf nur zum Beenden des System aufgerufen werden. Nachfolgende Aufrufe sämtlicher Kernelmethoden liefern
				undefinierte Ergebnisse.
	*/

	static void DeleteInstance()
	{
		if (_Instance)
		{
			delete(_Instance);
			_Instance = 0;
		}
	}

	/**
	    @brief Löst eine Schutzverletzung aus.
		@remark Diese Methode dient zum Testen des Crash-Handlings.
	*/
	void Crash() const
	{
		__asm
		{
			xor eax, eax
			mov [eax], 0
		}
	}
	
private:

	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// Private da Singleton
	// -----------------------------------------------------------------------------
	
	BS_Kernel();
	virtual ~BS_Kernel();

	// -----------------------------------------------------------------------------
	// Singleton-Exemplar
	// -----------------------------------------------------------------------------
	static BS_Kernel * _Instance;

	// Service Daten
	// -------------
	class Superclass
	{
	private:
		BS_Kernel*	_pKernel;
		unsigned int		_ServiceCount;
		std::string	_Identifier;
		BS_Service* _ActiveService;
		std::string	_ActiveServiceName;
		
	public:
		Superclass (BS_Kernel* pKernel, const std::string& Identifier);
		~Superclass();
		
		unsigned int GetServiceCount() const { return _ServiceCount; }
		std::string GetIdentifier() const { return _Identifier; }
		BS_Service* GetActiveService() const { return _ActiveService; }
		std::string GetActiveServiceName() const { return _ActiveServiceName; }
		std::string GetServiceIdentifier(unsigned int Number);
		BS_Service* NewService(const std::string& ServiceIdentifier);
		bool DisconnectService();
	};
	
	std::vector<Superclass*>	_SuperclassList;
	std::stack<std::string>		_ServiceCreationOrder;
	Superclass* GetSuperclassByIdentifier(const std::string& Identifier);
	
	bool _InitSuccess; // Gibt an, ob die Konstruktion erfolgreich war
	bool _Running;	// Gibt an, ob die Applikation im nächsten Durchlauf des Main-Loops noch weiterlaufen soll
	
	// Fenster Variablen
	// -----------------
	BS_Window* _pWindow;

	/*
	// CPU-Feature Variablen und Methoden
	// ----------------------------------
	enum _CPU_FEATURES_BITMASKS
	{
		_MMX_BITMASK		= (1 << 23),
		_SSE_BITMASK		= (1 << 25),
		_SSE2_BITMASK		= (1 << 26),
		_3DNOW_BITMASK		= (1 << 30),
		_3DNOWEXT_BITMASK	= (1 << 31)
	};

	bool		_DetectCPU();

	bool		_MMXPresent;
	bool		_SSEPresent;
	bool		_SSE2Present;
	bool		_3DNowPresent;
	bool		_3DNowExtPresent;
	CPU_TYPES	_CPUType;
	std::string	_CPUVendorID;
	*/

	// Resourcemanager
	// ---------------
	BS_ResourceManager* _pResourceManager;

	bool _RegisterScriptBindings();
};

// Dies ist nur eine kleine Klasse, die die Daten eines Services verwaltet.
// Ist ein wenig unschön, ich weiss, aber mit std::string ließ sich dass nicht mit einer
// einfachen struct bewerkstelligen.
class BS_ServiceInfo
{
public:
	BS_ServiceInfo(const std::string& SuperclassIdentifier, const std::string& ServiceIdentifier, BS_Service* (*CreateMethod)(BS_Kernel*))
	{
		this->SuperclassIdentifier = SuperclassIdentifier;
		this->ServiceIdentifier = ServiceIdentifier;
		this->CreateMethod = CreateMethod;
	};
	
	std::string	SuperclassIdentifier;
	std::string	ServiceIdentifier;
	BS_Service* (*CreateMethod)(BS_Kernel*);
};

#endif
