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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/savefile.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/persistenceservice.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/sfx/soundengine.h"
#include "sword25/input/inputengine.h"
#include "sword25/math/regionregistry.h"
#include "sword25/script/script.h"
#include <zlib.h>

using namespace std;

#define BS_LOG_PREFIX "PERSISTENCESERVICE"

// -----------------------------------------------------------------------------
// Constants and utility functions
// -----------------------------------------------------------------------------

namespace Sword25 {
const char         *SAVEGAME_EXTENSION = ".b25s";
const char         *SAVEGAME_DIRECTORY = "saves";
const char         *FILE_MARKER = "BS25SAVEGAME";
const unsigned int  SLOT_COUNT = 18;
const unsigned int  FILE_COPY_BUFFER_SIZE = 1024 * 10;
const char *VERSIONID = "1";

// -------------------------------------------------------------------------

Common::String GenerateSavegameFilename(unsigned int SlotID) {
	Common::String oss;
	oss += SlotID;
	oss += SAVEGAME_EXTENSION;
	return oss;
}

// -------------------------------------------------------------------------

Common::String GenerateSavegamePath(unsigned int SlotID) {
	Common::String oss;
	oss = BS_PersistenceService::GetSavegameDirectory();
	oss += BS_FileSystemUtil::GetInstance().GetPathSeparator();
	oss += GenerateSavegameFilename(SlotID);
	return oss;
}

// -------------------------------------------------------------------------

Common::String FormatTimestamp(TimeDate Time) {
	// In the original BS2.5 engine, this used a local object to show the date/time as as a string.
	// For now in ScummVM it's being hardcoded to 'dd-MON-yyyy hh:mm:ss'
	Common::String monthList[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	char buffer[100];
	snprintf(buffer, 100, "%.2d-%s-%.4d %.2d:%.2d:%.2d",
	         Time.tm_mday, monthList[Time.tm_mon].c_str(), Time.tm_year,
	         Time.tm_hour, Time.tm_min, Time.tm_sec
	        );

	return Common::String(buffer);
}

// -------------------------------------------------------------------------

Common::String LoadString(Common::InSaveFile *In, uint MaxSize = 999) {
	Common::String Result;
	char ch;
	while ((ch = (char)In->readByte()) != '\0') {
		Result += ch;
		if (Result.size() >= MaxSize) break;
	}

	return Result;
}
}

namespace Sword25 {

// -----------------------------------------------------------------------------
// Private Implementation
// -----------------------------------------------------------------------------

struct SavegameInformation {
	bool            IsOccupied;
	bool            IsCompatible;
	Common::String  Description;
	Common::String  Filename;
	unsigned int    GamedataLength;
	unsigned int    GamedataOffset;
	unsigned int    GamedataUncompressedLength;

	SavegameInformation() {
		Clear();
	}

	void Clear() {
		IsOccupied = false;
		IsCompatible = false;
		Description = "";
		Filename = "";
		GamedataLength = 0;
		GamedataOffset = 0;
		GamedataUncompressedLength = 0;
	}
};

struct BS_PersistenceService::Impl {
	SavegameInformation m_SavegameInformations[SLOT_COUNT];

	// -----------------------------------------------------------------------------

	Impl() {
		ReloadSlots();
	}

	// -----------------------------------------------------------------------------

	void ReloadSlots() {
		// Über alle Spielstanddateien iterieren und deren Infos einlesen.
		for (unsigned int i = 0; i < SLOT_COUNT; ++i) {
			ReadSlotSavegameInformation(i);
		}
	}

	void ReadSlotSavegameInformation(unsigned int SlotID) {
		// Aktuelle Slotinformationen in den Ausgangszustand versetzen, er wird im Folgenden neu gefüllt.
		SavegameInformation &CurSavegameInfo = m_SavegameInformations[SlotID];
		CurSavegameInfo.Clear();

		// Den Dateinamen für den Spielstand des Slots generieren.
		Common::String Filename = GenerateSavegamePath(SlotID);

		// Feststellen, ob eine Spielstanddatei dieses Namens existiert.
		if (BS_FileSystemUtil::GetInstance().FileExists(Filename)) {
			// Read in the game
			Common::SaveFileManager *sfm = g_system->getSavefileManager();
			Common::InSaveFile *File = sfm->openForLoading(Filename);

			if (File) {
				// Read in the header
				Common::String StoredMarker = LoadString(File);
				Common::String StoredVersionID = LoadString(File);
				CurSavegameInfo.GamedataLength = File->readUint32LE();
				CurSavegameInfo.GamedataUncompressedLength = File->readUint32LE();

				// If the header can be read in and is detected to be valid, we will have a valid file
				if (StoredMarker == FILE_MARKER) {
					// Der Slot wird als belegt markiert.
					CurSavegameInfo.IsOccupied = true;
					// Speichern, ob der Spielstand kompatibel mit der aktuellen Engine-Version ist.
					CurSavegameInfo.IsCompatible = (StoredVersionID == Common::String(VERSIONID));
					// Dateinamen des Spielstandes speichern.
					CurSavegameInfo.Filename = GenerateSavegameFilename(SlotID);
					// Die Beschreibung des Spielstandes besteht aus einer textuellen Darstellung des Änderungsdatums der Spielstanddatei.
					CurSavegameInfo.Description = FormatTimestamp(BS_FileSystemUtil::GetInstance().GetFileTime(Filename));
					// Den Offset zu den gespeicherten Spieldaten innerhalb der Datei speichern.
					// Dieses entspricht der aktuellen Position + 1, da nach der letzten Headerinformation noch ein Leerzeichen als trenner folgt.
					CurSavegameInfo.GamedataOffset = static_cast<unsigned int>(File->pos()) + 1;
				}

				delete File;
			}
		}
	}
};

// -----------------------------------------------------------------------------
// Construction / Destruction
// -----------------------------------------------------------------------------

BS_PersistenceService &BS_PersistenceService::GetInstance() {
	static BS_PersistenceService Instance;
	return Instance;
}

// -----------------------------------------------------------------------------

BS_PersistenceService::BS_PersistenceService() : m_impl(new Impl) {
}

// -----------------------------------------------------------------------------

BS_PersistenceService::~BS_PersistenceService() {
	delete m_impl;
}

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

void BS_PersistenceService::ReloadSlots() {
	m_impl->ReloadSlots();
}

// -----------------------------------------------------------------------------

unsigned int BS_PersistenceService::GetSlotCount() {
	return SLOT_COUNT;
}

// -----------------------------------------------------------------------------

Common::String BS_PersistenceService::GetSavegameDirectory() {
	return BS_FileSystemUtil::GetInstance().GetUserdataDirectory() + BS_FileSystemUtil::GetInstance().GetPathSeparator() + SAVEGAME_DIRECTORY;
}

// -----------------------------------------------------------------------------

namespace {
bool CheckSlotID(unsigned int SlotID) {
	// Überprüfen, ob die Slot-ID zulässig ist.
	if (SlotID >= SLOT_COUNT) {
		BS_LOG_ERRORLN("Tried to access an invalid slot (%d). Only slot ids from 0 to %d are allowed.", SlotID, SLOT_COUNT - 1);
		return false;
	} else {
		return true;
	}
}
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::IsSlotOccupied(unsigned int SlotID) {
	if (!CheckSlotID(SlotID)) return false;
	return m_impl->m_SavegameInformations[SlotID].IsOccupied;
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::IsSavegameCompatible(unsigned int SlotID) {
	if (!CheckSlotID(SlotID)) return false;
	return m_impl->m_SavegameInformations[SlotID].IsCompatible;
}

// -----------------------------------------------------------------------------

Common::String &BS_PersistenceService::GetSavegameDescription(unsigned int SlotID) {
	static Common::String EmptyString;
	if (!CheckSlotID(SlotID)) return EmptyString;
	return m_impl->m_SavegameInformations[SlotID].Description;
}

// -----------------------------------------------------------------------------

Common::String &BS_PersistenceService::GetSavegameFilename(unsigned int SlotID) {
	static Common::String EmptyString;
	if (!CheckSlotID(SlotID)) return EmptyString;
	return m_impl->m_SavegameInformations[SlotID].Filename;
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::SaveGame(unsigned int SlotID, const Common::String &ScreenshotFilename) {
	// Überprüfen, ob die Slot-ID zulässig ist.
	if (SlotID >= SLOT_COUNT) {
		BS_LOG_ERRORLN("Tried to save to an invalid slot (%d). Only slot ids form 0 to %d are allowed.", SlotID, SLOT_COUNT - 1);
		return false;
	}

	// Dateinamen erzeugen.
	Common::String Filename = GenerateSavegamePath(SlotID).c_str();

	// Sicherstellen, dass das Verzeichnis für die Spielstanddateien existiert.
	BS_FileSystemUtil::GetInstance().CreateDirectory(GetSavegameDirectory());

	// Spielstanddatei öffnen und die Headerdaten schreiben.
	Common::SaveFileManager *sfm = g_system->getSavefileManager();
	Common::OutSaveFile *File = sfm->openForSaving(Filename);

	File->writeString(FILE_MARKER);
	File->writeByte(' ');
	File->writeString(VERSIONID);
	File->writeByte(' ');

	if (File->err()) {
		error("Unable to write header data to savegame file \"%s\".", Filename.c_str());
	}

	// Alle notwendigen Module persistieren.
	BS_OutputPersistenceBlock Writer;
	bool Success = true;
	Success &= BS_Kernel::GetInstance()->GetScript()->Persist(Writer);
	Success &= BS_RegionRegistry::GetInstance().Persist(Writer);
	Success &= BS_Kernel::GetInstance()->GetGfx()->Persist(Writer);
	Success &= BS_Kernel::GetInstance()->GetSfx()->Persist(Writer);
	Success &= BS_Kernel::GetInstance()->GetInput()->Persist(Writer);
	if (!Success) {
		error("Unable to persist modules for savegame file \"%s\".", Filename.c_str());
	}

	// Daten komprimieren.
	uLongf CompressedLength = Writer.GetDataSize() + (Writer.GetDataSize() + 500) / 1000 + 12;
	Bytef *CompressionBuffer = new Bytef[CompressedLength];

	if (compress2(&CompressionBuffer[0], &CompressedLength, reinterpret_cast<const Bytef *>(Writer.GetData()), Writer.GetDataSize(), 6) != Z_OK) {
		error("Unable to compress savegame data in savegame file \"%s\".", Filename.c_str());
	}

	// Länge der komprimierten Daten und der unkomprimierten Daten in die Datei schreiben.
	char sBuffer[10];
	snprintf(sBuffer, 10, "%ld", CompressedLength);
	File->writeString(sBuffer);
	File->writeByte(' ');
	snprintf(sBuffer, 10, "%uld", Writer.GetDataSize());
	File->writeString(sBuffer);
	File->writeByte(' ');

	// Komprimierte Daten in die Datei schreiben.
	File->write(reinterpret_cast<char *>(&CompressionBuffer[0]), CompressedLength);
	if (File->err()) {
		error("Unable to write game data to savegame file \"%s\".", Filename.c_str());
	}

	// Screenshotdatei an die Datei anfügen.
	if (BS_FileSystemUtil::GetInstance().FileExists(ScreenshotFilename)) {
		Common::File ScreenshotFile;
		if (!ScreenshotFile.open(ScreenshotFilename.c_str()))
			error("Unable to load screenshot file");

		byte *Buffer = new Byte[FILE_COPY_BUFFER_SIZE];
		while (!ScreenshotFile.eos()) {
			int bytesRead = ScreenshotFile.read(&Buffer[0], FILE_COPY_BUFFER_SIZE);
			File->write(&Buffer[0], bytesRead);
		}
	} else {
		BS_LOG_WARNINGLN("The screenshot file \"%s\" does not exist. Savegame is written without a screenshot.", Filename.c_str());
	}

	// Savegameinformationen für diesen Slot aktualisieren.
	m_impl->ReadSlotSavegameInformation(SlotID);

	delete[] CompressionBuffer;

	// Erfolg signalisieren.
	return true;
}

// -----------------------------------------------------------------------------

bool BS_PersistenceService::LoadGame(unsigned int SlotID) {
	Common::SaveFileManager *sfm = g_system->getSavefileManager();
	Common::InSaveFile *File;

	// Überprüfen, ob die Slot-ID zulässig ist.
	if (SlotID >= SLOT_COUNT) {
		BS_LOG_ERRORLN("Tried to load from an invalid slot (%d). Only slot ids form 0 to %d are allowed.", SlotID, SLOT_COUNT - 1);
		return false;
	}

	SavegameInformation &CurSavegameInfo = m_impl->m_SavegameInformations[SlotID];

	// Überprüfen, ob der Slot belegt ist.
	if (!CurSavegameInfo.IsOccupied) {
		BS_LOG_ERRORLN("Tried to load from an empty slot (%d).", SlotID);
		return false;
	}

	// Überprüfen, ob der Spielstand im angegebenen Slot mit der aktuellen Engine-Version kompatibel ist.
	// Im Debug-Modus wird dieser Test übersprungen. Für das Testen ist es hinderlich auf die Einhaltung dieser strengen Bedingung zu bestehen,
	// da sich die Versions-ID bei jeder Codeänderung mitändert.
#ifndef DEBUG
	if (!CurSavegameInfo.IsCompatible) {
		BS_LOG_ERRORLN("Tried to load a savegame (%d) that is not compatible with this engine version.", SlotID);
		return false;
	}
#endif

	byte *CompressedDataBuffer = new byte[CurSavegameInfo.GamedataLength];
	byte *UncompressedDataBuffer = new Bytef[CurSavegameInfo.GamedataUncompressedLength];

	File = sfm->openForLoading(GenerateSavegamePath(SlotID));

	File->seek(CurSavegameInfo.GamedataOffset);
	File->read(reinterpret_cast<char *>(&CompressedDataBuffer[0]), CurSavegameInfo.GamedataLength);
	if (File->err()) {
		BS_LOG_ERRORLN("Unable to load the gamedata from the savegame file \"%s\".", CurSavegameInfo.Filename.c_str());
		delete[] CompressedDataBuffer;
		delete[] UncompressedDataBuffer;
		return false;
	}

	// Spieldaten dekomprimieren.
	uLongf UncompressedBufferSize = CurSavegameInfo.GamedataUncompressedLength;
	if (uncompress(reinterpret_cast<Bytef *>(&UncompressedDataBuffer[0]), &UncompressedBufferSize,
	               reinterpret_cast<Bytef *>(&CompressedDataBuffer[0]), CurSavegameInfo.GamedataLength) != Z_OK) {
		BS_LOG_ERRORLN("Unable to decompress the gamedata from savegame file \"%s\".", CurSavegameInfo.Filename.c_str());
		delete[] UncompressedDataBuffer;
		delete[] CompressedDataBuffer;
		delete File;
		return false;
	}

	BS_InputPersistenceBlock Reader(&UncompressedDataBuffer[0], CurSavegameInfo.GamedataUncompressedLength);

	// Einzelne Engine-Module depersistieren.
	bool Success = true;
	Success &= BS_Kernel::GetInstance()->GetScript()->Unpersist(Reader);
	// Muss unbedingt nach Script passieren. Da sonst die bereits wiederhergestellten Regions per Garbage-Collection gekillt werden.
	Success &= BS_RegionRegistry::GetInstance().Unpersist(Reader);
	Success &= BS_Kernel::GetInstance()->GetGfx()->Unpersist(Reader);
	Success &= BS_Kernel::GetInstance()->GetSfx()->Unpersist(Reader);
	Success &= BS_Kernel::GetInstance()->GetInput()->Unpersist(Reader);

	delete[] CompressedDataBuffer;
	delete[] UncompressedDataBuffer;
	delete File;

	if (!Success) {
		BS_LOG_ERRORLN("Unable to unpersist the gamedata from savegame file \"%s\".", CurSavegameInfo.Filename.c_str());
		return false;
	}

	return true;
}

} // End of namespace Sword25
