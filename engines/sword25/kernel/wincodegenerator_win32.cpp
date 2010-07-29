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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <LMCons.h>

#include <vector>
using namespace std;

#include "md5.h"
#include "wincodegenerator.h"

// -----------------------------------------------------------------------------
// Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	void AddedFixedDrivesEntropy(BS_MD5 & md5)
	{
		if (DWORD LogicalDrivesMask = ::GetLogicalDrives())
		{
			// Über alle Laufwerke iterieren.
			char CurrentDriveLetter[] = "A:\\";
			while (LogicalDrivesMask && CurrentDriveLetter[0] <= 'Z')
			{
				if (LogicalDrivesMask & 1)
				{
					// Nur feste Laufwerke werden betrachtet, ansonsten würde sich die System-ID ändern, wenn jemand einen USB-Stick ansteckt oder
					// eine CD einlegt.
					if (::GetDriveTypeA(CurrentDriveLetter) == DRIVE_FIXED)
					{
						// Laufwerksinformationen auslesen.
						CHAR VolumeNameBuffer[MAX_PATH + 1];
						DWORD SerialNumber;
						DWORD MaximumComponentLength;
						DWORD FileSystemFlags;
						CHAR FileSystemNameBuffer[MAX_PATH + 1];
						if (::GetVolumeInformationA(CurrentDriveLetter,
							VolumeNameBuffer, sizeof(VolumeNameBuffer),
							&SerialNumber,
							&MaximumComponentLength,
							&FileSystemFlags,
							FileSystemNameBuffer, sizeof(FileSystemNameBuffer)))
						{
							// Als Entropie werden genutzt: Laufwerksbuchstabe, Laufwerksbezeichnung, Seriennummer und Dateisystemname.
							md5.Update(reinterpret_cast<const unsigned char *>(CurrentDriveLetter), strlen(CurrentDriveLetter));
							md5.Update(reinterpret_cast<const unsigned char *>(VolumeNameBuffer), strlen(VolumeNameBuffer));
							md5.Update(reinterpret_cast<const unsigned char *>(&SerialNumber), sizeof(SerialNumber));
							md5.Update(reinterpret_cast<const unsigned char *>(FileSystemNameBuffer), strlen(FileSystemNameBuffer));
						}
					}
				}

				LogicalDrivesMask >>= 1;
				++CurrentDriveLetter[0];
			}
		}
	}

	// -------------------------------------------------------------------------

	void AddUserNameEntropy(BS_MD5 & md5)
	{
		// Benutzernamen auslesen und als Entropie nutzen.
		DWORD UserNameLength = UNLEN + 1;
		CHAR UserName[UNLEN + 1];
		if (::GetUserNameA(UserName, &UserNameLength))
		{
			md5.Update(reinterpret_cast<const unsigned char *>(&UserName[0]), strlen(UserName));
		}		
	}

	// -------------------------------------------------------------------------

	void AddOSVersionEntropy(BS_MD5 & md5)
	{
		// Windows-Version auslesen und in die Einzelkomponenten MajorVersion, MinorVersion und Build aufspalten.
		DWORD VersionInfo = ::GetVersion();

		DWORD MajorVersion = (DWORD)(LOBYTE(LOWORD(VersionInfo)));
		DWORD MinorVersion = (DWORD)(HIBYTE(LOWORD(VersionInfo)));
		DWORD Build = 0;
		if (VersionInfo < 0x80000000) Build = (DWORD)(HIWORD(VersionInfo));

		// Diese drei Informationen als Entropie nutzen.
		md5.Update(reinterpret_cast<const unsigned char *>(&MajorVersion), sizeof(DWORD));
		md5.Update(reinterpret_cast<const unsigned char *>(&MinorVersion), sizeof(DWORD));
		md5.Update(reinterpret_cast<const unsigned char *>(&Build), sizeof(DWORD));
	}
}

// -----------------------------------------------------------------------------

unsigned int BS_WinCodeGenerator::GetSystemID()
{
	BS_MD5 md5;

	AddedFixedDrivesEntropy(md5);
	AddUserNameEntropy(md5);
	AddOSVersionEntropy(md5);

	unsigned char Digest[16];
	md5.GetDigest(Digest);

	return (Digest[3] << 24) + (Digest[2] << 16) + (Digest[1] << 8) + Digest[0];
}
