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

#ifndef BS_CPUINFO_H
#define BS_CPUINFO_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

/**
    @brief Diese Singleton-Klasse stellt Informationen über die CPU zur verfügung.
*/

class BS_CPUInfo
{
public:
	/**
	    @brief Definiert die Vendor-IDs
	*/
	enum VENDORID
	{
		V_UNKNOWN,
		V_INTEL,
		V_AMD,
		V_CYRIX,
		V_CENTAUR,
		V_NEXGEN,
		V_TRANSMETA,
		V_RISE,
		V_UMC,
		V_SIS,
		V_NSC,
	};

	/**
	    @brief Gibt eine Referenz auf die einzige Instanz dieser Klasse zurück.
	*/
	static const BS_CPUInfo & GetInstance()
	{
		static BS_CPUInfo Instance;
		return Instance;
	}

	/**
	    @brief Gibt die Vendor-ID des CPU-Herstellers zurück.
		@remark Gibt BS_CPUInfo::V_UNKNOWN zurück, wenn die Vendor-ID nicht bestimmt werden konnte.
	*/
	VENDORID GetVendorID() const { return _VendorID; }

	/**
	    @brief Gibt den Vendor-String zurück.
		@remark Gibt "unknown" zurück, wenn der Vendor-String nicht bestimmt werden konnte.
	*/
	const std::string & GetVendorString() const { return _VendorString; }

	/**
	    @brief Gibt den CPU-Namen zurück.
		@remark Gibt "unknown" zurück, wenn der CPU-Name nicht bestimmt werden konnte.
	*/
	const std::string & GetCPUName() const { return _CPUName; }

	/**
	    @brief Gibt zurück, ob der Prozessor MMX untersützt.
	*/
	bool IsMMXSupported() const { return _MMXSupported; }

	/**
	    @brief Gibt zurück, ob der Prozessor SSE unterstützt.
	*/
	bool IsSSESupported() const { return _SSESupported; }
	
	/**
		@brief Gibt zurück, ob der Prozessor SSE2 unterstützt.
	*/
	bool IsSSE2Supported() const { return _SSE2Supported; }

	/**
		@brief Gibt zurück, ob der Prozessor 3DNow! unterstützt.
	*/
	bool Is3DNowSupported() const { return _3DNowSupported; }

	/**
		@brief Gibt zurück, ob der Prozessor 3DNow!-Ext. unterstützt.
	*/
	bool Is3DNowExtSupported() const { return _3DNowExtSupported; }

private:
	BS_CPUInfo();

	VENDORID	_VendorID;
	std::string	_VendorString;
	std::string	_CPUName;
	bool		_MMXSupported;
	bool		_SSESupported;
	bool		_SSE2Supported;
	bool		_3DNowSupported;
	bool		_3DNowExtSupported;

	bool _ReadVendor();
	bool _ReadCPUFeatures();
	bool _ReadCPUName();
	bool _IsCPUIDSupported() const;
};

#endif
