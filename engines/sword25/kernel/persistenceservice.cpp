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

#include "kernel.h"
#include "persistenceservice.h"
#include "inputpersistenceblock.h"
#include "outputpersistenceblock.h"
#include "filesystemutil.h"
#include "gfx/graphicengine.h"
#include "sfx/soundengine.h"
#include "input/inputengine.h"
#include "math/regionregistry.h"
#include "script/script.h"
#include "debug/debugtools.h"
#include "util/zlib/zlib.h"

#include "kernel/memlog_off.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <locale>
#include "kernel/memlog_on.h"

using namespace std;

#define BS_LOG_PREFIX "PERSISTENCESERVICE"

// -----------------------------------------------------------------------------
// Konstanten und Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	const char *		SAVEGAME_EXTENSION = ".b25s";
	const char *		SAVEGAME_DIRECTORY = "saves";
	const char *		FILE_MARKER = "BS25SAVEGAME";
	const unsigned int	SLOT_COUNT = 18;
	const unsigned int	FILE_COPY_BUFFER_SIZE = 1024 * 10;

	// -------------------------------------------------------------------------

	string GenerateSavegameFilename(unsigned int SlotID)
	{
		ostringstream oss;
		oss << SlotID << SAVEGAME_EXTENSION;
		return oss.str();
	}

	// -------------------------------------------------------------------------

	string GenerateSavegamePath(unsigned int SlotID)
	{
		ostringstream oss;
		oss << BS_PersistenceService::GetSavegameDirectory() << BS_FileSystemUtil::GetInstance().GetPathSeparator() << GenerateSavegameFilename(SlotID);
		return oss.str();
	}

	// -------------------------------------------------------------------------

	string FormatTimestamp(time_t Time)
	{
		// Zeitstempel in Einzelkomponenten auflösen.
		tm * Timeinfo = localtime(&Time);

		// Zeitangabe im lokalen Format in einen String-Stream schreiben.
		locale Locale("");
		ostringstream StringBuilder;
		StringBuilder.imbue(Locale);
		char * Pattern = "%x %X";
		use_facet<time_put<char> >(Locale).put(StringBuilder,
			StringBuilder, StringBuilder.fill(), Timeinfo, Pattern, Pattern + strlen(Pattern));

		// Formatierten String zurückgeben.
		return StringBuilder.str();
	}
}

// -----------------------------------------------------------------------------
// Private Implementation (Pimpl-Pattern)
// -----------------------------------------------------------------------------

struct SavegameInformation
{
	bool			IsOccupied;
	bool			IsCompatible;
	string			Description;
	string			Filename;
	unsigned int	GamedataLength;
	unsigned int	GamedataOffset;
	unsigned int	GamedataUncompressedLength;

	SavegameInformation() { Clear(); }

	void Clear()
	{
		IsOccupied = false;
		IsCompatible = false;
		Description = "";
		Filename = "";
		GamedataLength = 0;
		GamedataOffset = 0;
		GamedataUncompressedLength = 0;
	}
};

struct BS_PersistenceService::Impl
{
	SavegameInformation m_SavegameInformations[SLOT_COUNT];

	// -----------------------------------------------------------------------------

	Impl()
	{
		ReloadSlots();
	}

	// -----------------------------------------------------------------------------

	void ReloadSlots()
	{
		// Über alle Spielstanddateien iterieren und deren Infos einlesen.
		for (unsigned int i = 0; i < SLOT_COUNT; ++i)
		{
			ReadSlotSavegameInformation(i);
		}
	}

	void ReadSlotSavegameInformation(unsigned int SlotID)
	{
		// Aktuelle Slotinformationen in den Ausgangszustand versetzen, er wird im Folgenden neu gefüllt.
		SavegameInformation & CurSavegameInfo = m_SavegameInformations[SlotID];
		CurSavegameInfo.Clear();

		// Den Dateinamen für den Spielstand des Slots generieren.
		string Filename = GenerateSavegamePath(SlotID);

		// Feststellen, ob eine Spielstanddatei dieses Namens existiert.
		if (BS_FileSystemUtil::GetInstance().FileExists(Filename))
		{
			// Die Spielstanddatei öffnen.
			ifstream File(Filename.c_str(), ifstream::binary);
			if (File.good() && File.is_open())
			{
				// Die Headerdaten einlesen.
				string StoredMarker, StoredVersionID;
				File >> StoredMarker >> StoredVersionID >> CurSavegameInfo.GamedataLength >> CurSavegameInfo.GamedataUncompressedLength;

				// Falls die Headerdaten gelesen werden konnten und der Marker stimmt, nehmen wir an eine gültige Spielstanddatei zu haben.
				if (File.good() && StoredMarker == FILE_MARKER)
				{
					// Der Slot wird als belegt markiert.
					CurSavegameInfo.IsOccupied = true;
					// Speichern, ob der Spielstand kompatibel mit der aktuellen Engine-Version ist.
					CurSavegameInfo.IsCompatible = (StoredVersionID == BS_Debugtools::GetVersionID());
					// Dateinamen des Spielstandes speichern.
					CurSavegameInfo.Filename = GenerateSavegameFilename(SlotID);
					// Die Beschreibung des Spielstandes besteht aus einer textuellen Darstellung des Änderungsdatums der Spielstanddatei.
					CurSavegameInfo.Description = FormatTimestamp(BS_FileSystemUtil::GetInstance().GetFileTime(Filename));
					// Den Offset zu den gespeicherten Spieldaten innerhalb der Datei speichern.
					// Dieses entspricht der aktuellen Position + 1, da nach der letzten Headerinformation noch ein Leerzeichen als trenner folgt.
					CurSavegameInfo.GamedataOffset = static_cast<unsigned int>(File.tellg()) + 1;
				}

			}
		}
	}
};

// -----------------------------------------------------------------------------
// Construction / Destruction
// -----------------------------------------------------------------------------

BS_PersistenceService & BS_PersistenceService::GetInstance()
{
	static BS_PersistenceService Instance;
	return Instance;
}

// -----------------------------------------------------------------------------

BS_PersistenceService::BS_PersistenceService() : m_impl(new Impl)
{
}

// -----------------------------------------------------------------------------

BS_PersistenceService::~BS_PersistenceService()
{
	delete m_impl;
}

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

void BS_PersistenceService::ReloadSlots()
{
	m_impl->ReloadSlots();
}

// -----------------------------------------------------------------------------

unsigned int BS_PersistenceService::GetSlotCount()
{
	return SLOT_COUNT;
}

// -----------------------------------------------------------------------------

std::string BS_PersistenceService::GetSavegameDirectory()
{
	return BS_FileSystemUtil::GetInstance().GetUserdataDirectory() + BS_FileSystemUtil::GetInstance().GetPathSeparator() + SAVEGAME_DIRECTORY;
}

// -----------------------------------------------------------------------------

namespace
{
	bool CheckSlotID(unsigned int SlotID)
	{
		// Überprüfen, ob die Slot-ID zulässig ist.
		if (SlotID >= SLOT_COUNT)
		{
			BS_LOG_ERRORLN("Tried to access an invalid slot (%d). Only slot ids from 0 to %d are allowed.", SlotID, SLOT_COUNT - 1);
			return false;
		}
		else
		{
			return true;
		}
	}
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::IsSlotOccupied(unsigned int SlotID)
{
	if (!CheckSlotID(SlotID)) return false;
	return m_impl->m_SavegameInformations[SlotID].IsOccupied;
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::IsSavegameCompatible(unsigned int SlotID)
{
	if (!CheckSlotID(SlotID)) return false;
	return m_impl->m_SavegameInformations[SlotID].IsCompatible;
}

// -----------------------------------------------------------------------------

string & BS_PersistenceService::GetSavegameDescription(unsigned int SlotID)
{
	static string EmptyString;
	if (!CheckSlotID(SlotID)) return EmptyString;
	return m_impl->m_SavegameInformations[SlotID].Description;
}

// -----------------------------------------------------------------------------

string & BS_PersistenceService::GetSavegameFilename(unsigned int SlotID)
{
	static string EmptyString;
	if (!CheckSlotID(SlotID)) return EmptyString;
	return m_impl->m_SavegameInformations[SlotID].Filename;
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::SaveGame(unsigned int SlotID, const std::string & ScreenshotFilename)
{
	// Überprüfen, ob die Slot-ID zulässig ist.
	if (SlotID >= SLOT_COUNT)
	{
		BS_LOG_ERRORLN("Tried to save to an invalid slot (%d). Only slot ids form 0 to %d are allowed.", SlotID, SLOT_COUNT - 1);
		return false;
	}

	// Dateinamen erzeugen.
	string Filename = GenerateSavegamePath(SlotID).c_str();

	try
	{
		// Sicherstellen, dass das Verzeichnis für die Spielstanddateien existiert.
		BS_FileSystemUtil::GetInstance().CreateDirectory(GetSavegameDirectory());

		// Spielstanddatei öffnen und die Headerdaten schreiben.
		ofstream File(Filename.c_str(), ofstream::binary);
		File << string(FILE_MARKER) << " " << string(BS_Debugtools::GetVersionID()) << " ";
		if (!File.good())
		{
			BS_LOG_ERRORLN("Unable to write header data to savegame file \"%s\".", Filename.c_str());
			throw 0;
		}

		// Alle notwendigen Module persistieren.
		BS_OutputPersistenceBlock Writer;
		bool Success = true;
		Success &= BS_Kernel::GetInstance()->GetScript()->Persist(Writer);
		Success &= BS_RegionRegistry::GetInstance().Persist(Writer);
		Success &= BS_Kernel::GetInstance()->GetGfx()->Persist(Writer);
		Success &= BS_Kernel::GetInstance()->GetSfx()->Persist(Writer);
		Success &= BS_Kernel::GetInstance()->GetInput()->Persist(Writer);
		if (!Success)
		{
			BS_LOG_ERRORLN("Unable to persist modules for savegame file \"%s\".", Filename.c_str());
			throw 0;
		}

		// Daten komprimieren.
		vector<unsigned char> CompressionBuffer(Writer.GetDataSize() + (Writer.GetDataSize() + 500) / 1000 + 12);
		uLongf CompressedLength = CompressionBuffer.size();
		if (compress2(&CompressionBuffer[0], &CompressedLength, reinterpret_cast<const Bytef *>(Writer.GetData()), Writer.GetDataSize(), 6) != Z_OK)
		{
			BS_LOG_ERRORLN("Unable to compress savegame data in savegame file \"%s\".", Filename.c_str());
			throw 0;
		}

		// Länge der komprimierten Daten und der unkomprimierten Daten in die Datei schreiben.
		File << CompressedLength << " " << Writer.GetDataSize() << " ";

		// Komprimierte Daten in die Datei schreiben.
		File.write(reinterpret_cast<char *>(&CompressionBuffer[0]), CompressedLength);
		if (!File.good())
		{
			BS_LOG_ERRORLN("Unable to write game data to savegame file \"%s\".", Filename.c_str());
			throw 0;
		}

		// Screenshotdatei an die Datei anfügen.
		if (BS_FileSystemUtil::GetInstance().FileExists(ScreenshotFilename))
		{
			ifstream ScreenshotFile(ScreenshotFilename.c_str(), ifstream::binary);

			vector<char> Buffer(FILE_COPY_BUFFER_SIZE);
			while (ScreenshotFile.good())
			{
				ScreenshotFile.read(&Buffer[0], Buffer.size());
				File.write(&Buffer[0], ScreenshotFile.gcount());
			}
		}
		else
		{
			BS_LOG_WARNINGLN("The screenshot file \"%s\" does not exist. Savegame is written without a screenshot.", Filename.c_str());
		}

		// Savegameinformationen für diesen Slot aktualisieren.
		m_impl->ReadSlotSavegameInformation(SlotID);
	}
	catch(...)
	{
		BS_LOG_ERRORLN("An error occured while create savegame file \"%s\".", Filename.c_str());

		// Es ist ein Fehler aufgetreten, die Spielstanddatei wird gelöscht, da sie keinen konsistenten Zustand besitzt.
		::remove(Filename.c_str());

		// Misserfolg signalisieren.
		return false;
	}

	// Erfolg signalisieren.
	return true;
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::LoadGame(unsigned int SlotID)
{
	// Überprüfen, ob die Slot-ID zulässig ist.
	if (SlotID >= SLOT_COUNT)
	{
		BS_LOG_ERRORLN("Tried to load from an invalid slot (%d). Only slot ids form 0 to %d are allowed.", SlotID, SLOT_COUNT - 1);
		return false;
	}

	SavegameInformation & CurSavegameInfo = m_impl->m_SavegameInformations[SlotID];

	// Überprüfen, ob der Slot belegt ist.
	if (!CurSavegameInfo.IsOccupied)
	{
		BS_LOG_ERRORLN("Tried to load from an empty slot (%d).", SlotID);
		return false;
	}

	// Überprüfen, ob der Spielstand im angegebenen Slot mit der aktuellen Engine-Version kompatibel ist.
	// Im Debug-Modus wird dieser Test übersprungen. Für das Testen ist es hinderlich auf die Einhaltung dieser strengen Bedingung zu bestehen,
	// da sich die Versions-ID bei jeder Codeänderung mitändert.
#ifndef DEBUG
	if (!CurSavegameInfo.IsCompatible)
	{
		BS_LOG_ERRORLN("Tried to load a savegame (%d) that is not compatible with this engine version.", SlotID);
		return false;
	}
#endif

	vector<unsigned char> UncompressedDataBuffer(CurSavegameInfo.GamedataUncompressedLength);
	{
		// Komprimierte gespeicherte Spieldaten laden.
		vector<unsigned char> CompressedDataBuffer(CurSavegameInfo.GamedataLength);
		{
			ifstream File(GenerateSavegamePath(SlotID).c_str(), ifstream::binary);
			File.seekg(CurSavegameInfo.GamedataOffset);
			File.read(reinterpret_cast<char *>(&CompressedDataBuffer[0]), CurSavegameInfo.GamedataLength);
			if (!File.good())
			{
				BS_LOG_ERRORLN("Unable to load the gamedata from the savegame file \"%s\".", CurSavegameInfo.Filename.c_str());
				return false;
			}
		}

		// Spieldaten dekomprimieren.
		uLongf UncompressedBufferSize = UncompressedDataBuffer.size();
		if (uncompress(reinterpret_cast<Bytef *>(&UncompressedDataBuffer[0]), &UncompressedBufferSize,
			reinterpret_cast<Bytef *>(&CompressedDataBuffer[0]), CompressedDataBuffer.size()) != Z_OK)
		{
			BS_LOG_ERRORLN("Unable to decompress the gamedata from savegame file \"%s\".", CurSavegameInfo.Filename.c_str());
			return false;
		}
	}

	BS_InputPersistenceBlock Reader(&UncompressedDataBuffer[0], UncompressedDataBuffer.size());

	// Einzelne Engine-Module depersistieren.
	bool Success = true;
	Success &= BS_Kernel::GetInstance()->GetScript()->Unpersist(Reader);
	// Muss unbedingt nach Script passieren. Da sonst die bereits wiederhergestellten Regions per Garbage-Collection gekillt werden.
	Success &= BS_RegionRegistry::GetInstance().Unpersist(Reader);
	Success &= BS_Kernel::GetInstance()->GetGfx()->Unpersist(Reader);
	Success &= BS_Kernel::GetInstance()->GetSfx()->Unpersist(Reader);
	Success &= BS_Kernel::GetInstance()->GetInput()->Unpersist(Reader);

	if (!Success)
	{
		BS_LOG_ERRORLN("Unable to unpersist the gamedata from savegame file \"%s\".", CurSavegameInfo.Filename.c_str());
		return false;
	}

	return true;
}
