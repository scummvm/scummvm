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
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "FMODEXRESOURCE"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <memory>
#include "fmod.h"
#include "fmodexexception.h"
#include "fmodexchannel.h"
#include "package/packagemanager.h"
#include "fmodexresource.h"

// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

namespace
{
	const unsigned int MAX_SAMPLE_SIZE = 100 * 1024; // Die Dateigröße in Byte ab der ein Sound als Stream abgespielt wird
}

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------


BS_FMODExResource::BS_FMODExResource(const std::string& FileName, FMOD_SYSTEM * FMOD, bool & Success) :
	m_SoundPtr(0),
	m_SoundDataPtr(0),
	BS_Resource(FileName, BS_Resource::TYPE_SOUND)
{
	BS_ASSERT(FMOD);

	// Von Misserfolg ausgehen
	Success = false;

	// Pointer auf den Package-Manager bekommen
	BS_PackageManager * PackagePtr = BS_Kernel::GetInstance()->GetPackage();
	if (!PackagePtr)
	{
		BS_LOG_ERRORLN("Package manager not found.");
		return;
	}

	// Datei laden
	 unsigned int FileSize;
	char * FileDataPtr = (char*) PackagePtr->GetFile(GetFileName(), &FileSize);
	if (!FileDataPtr)
	{
		BS_LOG_ERRORLN("File \"%s\" could not be loaded.", GetFileName().c_str());
		return;
	}

	// Ob die Sounddatei als Sample oder als Stream behandelt wird, ist abhängig von der Dateigröße.
	// Samples werden sofort intialisiert.
	// Für Streams wird hingegen bei jedem Abspielen ein neuer Sound erstellt. Dieses Vorgehen ist notwendig, da FMOD Ex Samples beliebig oft
	// gleichzeitig abspielen kann, Streams jedoch nur ein mal.
	if (FileSize <= MAX_SAMPLE_SIZE)
	{
		FMOD_CREATESOUNDEXINFO ExInfo;
		memset(&ExInfo, 0, sizeof(ExInfo));
		ExInfo.cbsize = sizeof(ExInfo);
		ExInfo.length = FileSize;

		FMOD_RESULT Result = FMOD_System_CreateSound(FMOD, FileDataPtr,
													 FMOD_CREATESAMPLE | FMOD_OPENMEMORY | FMOD_SOFTWARE | FMOD_2D | FMOD_LOOP_NORMAL,
													 &ExInfo,
													 &m_SoundPtr);
		if (Result != FMOD_OK) BS_FMODExException("FMOD_System_CreateSound()", Result).Log();

		Success = Result == FMOD_OK;

		delete FileDataPtr;
	}
	else
	{
		m_SoundDataPtr = FileDataPtr;
		m_SoundDataSize = FileSize;

		Success = true;
	}
}

// -----------------------------------------------------------------------------

BS_FMODExResource::~BS_FMODExResource()
{
	// Sound freigeben, solange des Soundsystem noch läuft.
	// Sollte das Soundsystem beendet worden sein müssen und können Sounds nicht mehr freigegeben werden.
	if (m_SoundPtr && BS_Kernel::GetInstance()->GetService("sfx")) FMOD_Sound_Release(m_SoundPtr);
	if (m_SoundDataPtr) delete [] m_SoundDataPtr;
}

// -----------------------------------------------------------------------------
// Abspielen
// -----------------------------------------------------------------------------

BS_FMODExChannel * BS_FMODExResource::StartSound(FMOD_SYSTEM * FMOD)
{
	BS_ASSERT(FMOD);

	FMOD_CHANNEL * NewChannelPtr;
	FMOD_SOUND *   NewSoundPtr = 0;

	// Sample können sofort abgespielt werden.
	if (m_SoundPtr)
	{
		FMOD_RESULT Result = FMOD_System_PlaySound(FMOD, FMOD_CHANNEL_FREE, m_SoundPtr, 1, &NewChannelPtr);
		if (Result != FMOD_OK)
		{
			BS_FMODExException("FMOD_System_PlaySound()", Result).Log();
			return 0;
		}
	}
	// Für Streams muss ein neuer Sound erstellt werden.
	else
	{
		FMOD_CREATESOUNDEXINFO ExInfo;
		memset(&ExInfo, 0, sizeof(ExInfo));
		ExInfo.cbsize = sizeof(ExInfo);
		ExInfo.length = m_SoundDataSize;

		FMOD_RESULT Result;
		Result = FMOD_System_CreateSound(FMOD,
										 m_SoundDataPtr,
										 FMOD_CREATESTREAM | FMOD_OPENMEMORY_POINT | FMOD_SOFTWARE | FMOD_2D | FMOD_LOOP_NORMAL,
										 &ExInfo,
										 &NewSoundPtr);
		if (Result != FMOD_OK)
		{
			BS_FMODExException("FMOD_System_CreateSound()", Result).Log();
			return 0;
		}

		Result = FMOD_System_PlaySound(FMOD, FMOD_CHANNEL_FREE, NewSoundPtr, 1, &NewChannelPtr);
		if (Result != FMOD_OK)
		{
			BS_FMODExException("FMOD_System_PlaySound()", Result).Log();
			return 0;
		}
	}

	// Der Channel und der Sound (bei Streams) werden an ein BS_FMODExChannel-Objekt übergeben.
	// Dieses Sorgt auch dafür, dass Channel und Sound korrekt zerstört werden.
	return new BS_FMODExChannel(NewChannelPtr, NewSoundPtr);
}
