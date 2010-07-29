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

#define BS_LOG_PREFIX "CPUINFO"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "cpuinfo.h"

// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

// Standard CPU-Features
static const unsigned int MMX_BITMASK = 1 << 23;
static const unsigned int SSE_BITMASK = 1 << 25;
static const unsigned int SSE2_BITMASK = 1 << 26;

// Erweiterte CPU-Features
static const unsigned int _3DNOW_BITMASK = 1 << 30;
static const unsigned int _3DNOWEXT_BITMASK = 1 << 31;

// -----------------------------------------------------------------------------
// Konstruktion
// -----------------------------------------------------------------------------

BS_CPUInfo::BS_CPUInfo() :
	_VendorID(V_UNKNOWN),
	_VendorString("unknown"),
	_CPUName("unknown"),
	_MMXSupported(false),
	_SSESupported(false),
	_SSE2Supported(false),
	_3DNowSupported(false),
	_3DNowExtSupported(false)
{
	if (!_IsCPUIDSupported())
	{
		BS_LOG_ERRORLN("CPUID instruction ist not supported. Could not gather processor information.");
		return;
	}

	if (!_ReadVendor())
	{
		BS_LOG_WARNINGLN("Unrecognized CPU vendor.");
	}

	if (!_ReadCPUName())
	{
		BS_LOG_WARNINGLN("Could not determine CPU name.");
	}

	if (!_ReadCPUFeatures())
	{
		BS_LOG_WARNINGLN("Could not determine CPU-features.");
	}
}

// -----------------------------------------------------------------------------

bool BS_CPUInfo::_IsCPUIDSupported() const
{
	__try
	{
		__asm
		{
			mov		eax, 0
			cpuid
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return true;
}

// -----------------------------------------------------------------------------

bool BS_CPUInfo::_ReadVendor()
{
	static struct 
	{
		char*					VendorString;
		BS_CPUInfo::VENDORID	ID;
	} VENDOR_TABLE[] =
	{
		"GenuineIntel", V_INTEL,
		"AuthenticAMD", V_AMD,
		"CyrixInstead", V_CYRIX,
		"CentaurHauls", V_CENTAUR,
		"NexGenDriven", V_NEXGEN,
		"GenuineTMx86", V_TRANSMETA,
		"RiseRiseRise", V_RISE,
		"UMC UMC UMC", V_UMC,
		"SiS SiS SiS", V_SIS,
		"Geode by NSC", V_NSC,
		0, V_UNKNOWN,
	};

	// Vendor-String bestimmen
	char Buffer[13];
	__asm
	{
		xor eax, eax
		cpuid
		mov dword ptr [Buffer], ebx
		mov dword ptr [Buffer + 4], edx
		mov dword ptr [Buffer + 8], ecx
		mov byte ptr [Buffer + 12], 0
	}
	_VendorString = Buffer;

	// Vendor-ID bestimmen
	int i;
	for (i = 0; VENDOR_TABLE[i].VendorString; i++) if (_VendorString == VENDOR_TABLE[i].VendorString) break;
	_VendorID = VENDOR_TABLE[i].ID;

	return _VendorID != V_UNKNOWN;
}

// -----------------------------------------------------------------------------

bool BS_CPUInfo::_ReadCPUName()
{
	// Feststellen, ob das CPU-Name Feature vorhanden ist.
	unsigned int Result;
	__asm
	{
		mov eax, 0x80000000
		cpuid
		mov Result, eax
	}
	if (Result < 0x80000004) return false;

	// CPU-Namen einlesen
	char Buffer[49];
	__asm
	{
		mov     eax,0x80000002
		cpuid
		mov     dword ptr [Buffer + 0], eax
		mov     dword ptr [Buffer + 4], ebx
		mov     dword ptr [Buffer + 8], ecx
		mov     dword ptr [Buffer + 12], edx
		mov     eax,0x80000003
		cpuid
		mov     dword ptr [Buffer + 16], eax
		mov     dword ptr [Buffer + 20], ebx
		mov     dword ptr [Buffer + 24], ecx
		mov     dword ptr [Buffer + 28], edx
		mov     eax,0x80000004
		cpuid
		mov     dword ptr [Buffer + 32], eax
		mov     dword ptr [Buffer + 36], ebx
		mov     dword ptr [Buffer + 40], ecx
		mov     dword ptr [Buffer + 44], edx
		mov		byte ptr [Buffer + 48], 0
	}
	std::string TempCPUName = Buffer;
	if (TempCPUName.size() != 0)
	{
		// Führende und nachfolgende Leerzeichen entfernen
		std::string::const_iterator StringBegin = TempCPUName.begin();
		for (; StringBegin != TempCPUName.end() && *StringBegin == ' '; StringBegin++);
		std::string::const_iterator StringEnd = TempCPUName.end() - 1;
		for(; StringEnd >= TempCPUName.begin() && *StringEnd == ' '; StringEnd--);

		if (StringBegin != TempCPUName.end() && StringEnd >= TempCPUName.begin())
		{
			_CPUName = std::string(StringBegin, StringEnd + 1);
			return true;
		}
	}

	return false;
}

// -----------------------------------------------------------------------------

bool BS_CPUInfo::_ReadCPUFeatures()
{
	{
		// Feststellen ob die Standard-Features abgefragt werden können
		unsigned int Result;
		__asm
		{
			xor eax, eax
			cpuid
			mov Result, eax
		}

		// Nicht einmal die Standard-Features können abgefragt werden, also muss abgebrochen werden
		if (Result < 1) return false;

		// Standard-Features abfragen
		unsigned int Features;
		__asm
		{
			mov eax, 1
			cpuid
			mov Features, edx
		}

		_MMXSupported = (Features & MMX_BITMASK) != 0;
		_SSESupported = (Features & SSE_BITMASK) != 0;
		_SSE2Supported = (Features & SSE2_BITMASK) != 0;
	}


	// Feststellen ob erweiterte CPU-Features abgefragt werden können
	{
		unsigned int Result;
		__asm
		{
			mov eax, 0x80000000
			cpuid
			mov Result, eax
		}

		// Die erweiterten Features können nicht abgefragt werden, aber die Standard-Features wurden schon
		// abgefragt, daher wird true zurückgegeben.
		if (Result < 0x80000001) return true;

		// Erweiterte Features abfragen
		unsigned int Features;
		__asm
		{
			mov eax, 0x80000001
			cpuid
			mov Features, edx
		}

		_3DNowSupported = (Features & _3DNOW_BITMASK) != 0;
		_3DNowExtSupported = (Features & _3DNOWEXT_BITMASK) != 0;
	}

	return true;
}
