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

// Die von der Engine ausgegebenen Soundhandles werden intern auf FMOD Ex Handles gemapped.
// Diese Handles sind nur solange gültig, wie der Sound spielt. Falls danach versucht wird manipulierend auf den Sound zuzugreifen,
// schlägt dieses ohne Fehlermeldung fehl.

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "FMODEXSOUND"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/package/packagemanager.h"
#include "sword25/sfx/fmodexsound.h"

// -----------------------------------------------------------------------------
// Konstanten und lokale Funktionen
// -----------------------------------------------------------------------------

namespace Sword25 {
const float         DEFAULT_MUSIC_VOLUME = 1.0f;
const float         DEFAULT_SPEECH_VOLUME = 1.0f;
const float         DEFAULT_SFX_VOLUME = 1.0f;
const unsigned int  SOUNDTYPE_COUNT = 3;
const unsigned int  INVALID_SOUND_HANDLE = 0xffffffff;

// -------------------------------------------------------------------------

#if 0
inline float NormalizePanning(float Panning) {
	bool Corrected = false;
	float Result = Panning;
	if (Result > 1.0f) {
		Result = 1.0f;
		Corrected = true;
	}
	if (Result < -1.0f) {
		Result = -1.0f;
		Corrected = true;
	}

	if (Corrected) BS_LOG_WARNINGLN("Tried to set an invalid panning value of %.2f. It was corrected to %.2f", Panning, Result);

	return Result;
}

// -------------------------------------------------------------------------

inline float NormalizeVolume(float Volume) {
	bool Corrected = false;
	float Result = Volume;
	if (Result > 1.0f) {
		Result = 1.0f;
		Corrected = true;
	}
	if (Result < 0.0f) {
		Result = 0.0f;
		Corrected = true;
	}

	if (Corrected) BS_LOG_WARNINGLN("Tried to set an invalid volume value of %.2f. It was corrected to %.2f", Volume, Result);

	return Result;
}

// -------------------------------------------------------------------------

inline FMOD_SOUND_FORMAT BitsPerSampleToFMODExSoundFormat(unsigned int BitsPerSample) {
	switch (BitsPerSample) {
	case 8:
		return FMOD_SOUND_FORMAT_PCM8;
	case 16:
		return FMOD_SOUND_FORMAT_PCM16;
	case 24:
		return FMOD_SOUND_FORMAT_PCM24;
	case 32:
		return FMOD_SOUND_FORMAT_PCM32;
	default:
		return FMOD_SOUND_FORMAT_NONE;
	}
}

#endif

BS_FMODExSound::BS_FMODExSound(BS_Kernel *pKernel) :
	BS_SoundEngine(pKernel)
	/*	m_FMOD(0), 
		m_NextHandle(1) */ {
	// Lautstärkeneinstellungen auf die Standardwerte setzen
#if 0
	m_Volumes[MUSIC] = DEFAULT_MUSIC_VOLUME;
	m_Volumes[SPEECH] = DEFAULT_SPEECH_VOLUME;
	m_Volumes[SFX] = DEFAULT_SFX_VOLUME;
#endif
}

// -----------------------------------------------------------------------------

BS_FMODExSound::~BS_FMODExSound() {
#if 0
	// Alle noch spielenden Sounds stoppen und die Ressourcen freigeben
	for (PSM_ITER it = m_PlayingSoundsMap.begin(); it != m_PlayingSoundsMap.end(); ++it) {
		if (it->second.ChannelPtr) delete it->second.ChannelPtr;
		if (it->second.ResourcePtr) it->second.ResourcePtr->Release();
	}

	// FMOD Ex deinitialisieren
	if (m_FMOD) FMOD_System_Release(m_FMOD);
#endif
}

// -----------------------------------------------------------------------------

BS_Service *BS_FMODExSound_CreateObject(BS_Kernel *pKernel) {
	return new BS_FMODExSound(pKernel);
}

// -----------------------------------------------------------------------------

bool BS_FMODExSound::Init(unsigned int SampleRate, unsigned int Channels) {
#if 0
	// Eine Warnung ausgeben, wenn dieser Service schon initialisiert wurde.
	// Allerdings wird trotzdem true zurückgegeben, weil kein Fehler aufgetreten ist, der Service ist noch benutzbar.
	if (m_FMOD) {
		BS_LOG_WARNINGLN("Tried to initialize again. Call ignored.");
		return true;
	} else {
		try {
			// Die FMOD Ex mit den übergebenen Werte initialisieren
			FMOD_RESULT Result = FMOD_System_Create(&m_FMOD);
			if (Result != FMOD_OK) throw(BS_FMODExException("FMOD_System_Create()", Result));

			Result = FMOD_System_SetSoftwareFormat(m_FMOD, SampleRate, FMOD_SOUND_FORMAT_PCM16, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
			if (Result != FMOD_OK) throw(BS_FMODExException("FMOD_System_SetSoftwareFormat()", Result));

			Result = FMOD_System_Init(m_FMOD, Channels, FMOD_INIT_NORMAL, 0);
			if (Result != FMOD_OK) throw(BS_FMODExException("FMOD_System_Init()", Result));
		}

		catch (BS_FMODExException Ex) {
			Ex.Log();
			BS_LOG_ERRORLN("FMOD Ex could not be initialized.");

			if (m_FMOD) {
				FMOD_System_Release(m_FMOD);
				m_FMOD = 0;
			}

			return false;
		}

		BS_LOGLN("FMOD Ex initialized. Sample rate: %d / Channels: %d", SampleRate, Channels);
		return true;
	}
#else
	return true;
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::Update() {
#if 0
	BS_ASSERT(m_FMOD);

	FMOD_RESULT Result = FMOD_System_Update(m_FMOD);
	if (Result != FMOD_OK) BS_FMODExException("FMOD_System_Update()", Result).Log();

	RemoveInactiveSounds();
#endif
}

// -----------------------------------------------------------------------------
// Sounds abspielen
// -----------------------------------------------------------------------------

bool BS_FMODExSound::PlaySound(const Common::String &FileName,
                               SOUND_TYPES Type,
                               float Volume,
                               float Pan,
                               bool Loop,
                               int LoopStart, int LoopEnd,
                               unsigned int Layer) {
#if 0
	return PlaySoundInternal(FileName, Type, Volume, Pan, Loop, LoopStart, LoopEnd, Layer, 0, 0) != 0;
#else
	return true;
#endif
}

// -----------------------------------------------------------------------------

unsigned int BS_FMODExSound::PlaySoundEx(const Common::String &FileName,
        SOUND_TYPES Type,
        float Volume,
        float Pan,
        bool Loop,
        int LoopStart, int LoopEnd,
        unsigned int Layer) {
#if 0
	return PlaySoundInternal(FileName, Type, Volume, Pan, Loop, LoopStart, LoopEnd, Layer, 0, 0);
#else
	return true;
#endif
}

// -------------------------------------------------------------------------

#if 0
FMOD_RESULT F_CALLBACK BS_FMODExSound::FMODExDynamicSoundSetPosCallback(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype) {
	// In dynamischen Sounds wird nicht gesprungen, daher tut dieses Funktion nichts.
	return FMOD_OK;
}

// -------------------------------------------------------------------------

FMOD_RESULT F_CALLBACK BS_FMODExSound::FMODExDynamicSoundReadCallback(FMOD_SOUND *sound, void *data, unsigned int datalen) {
	// Handle auf das aktuelle Soundsystem holen, dies ist wohl dieses hier.
	BS_FMODExSound *t = reinterpret_cast<BS_FMODExSound *>(BS_Kernel::GetInstance()->GetSfx());

	// Handle auf den richtigen Sound holen, wurde als FMOD Ex Benutzerdaten gesetzt.
	unsigned int Handle;
	FMOD_RESULT Result = FMOD_Sound_GetUserData(sound, reinterpret_cast<void **>(&Handle));
	if (Result != FMOD_OK) {
		BS_FMODExException("FMOD_Sound_GetUserData()", Result).Log();
		return FMOD_OK;
	}

	// Sounddaten holen und Callbackfunktion aufrufen.
	PlayingSoundData *PSD = t->GetPlayingSoundDataByHandle(Handle);
	if (PSD) PSD->ReadCallback(PSD->UserData, data, datalen);

	return FMOD_OK;
}
#endif
// -----------------------------------------------------------------------------

unsigned int BS_FMODExSound::PlayDynamicSoundEx(DynamicSoundReadCallback ReadCallback,
        void *UserData,
        SOUND_TYPES Type,
        unsigned int SampleRate,
        unsigned int BitsPerSample,
        unsigned int Channels,
        float Volume,
        float Pan,
        unsigned int Layer) {
#if 0
	// Parameter überprüfen
	if (BitsPerSampleToFMODExSoundFormat(BitsPerSample) == FMOD_SOUND_FORMAT_NONE) {
		BS_LOG_ERRORLN("Cannot create a dynamic sound with %d bits per sample.", BitsPerSample);
		return 0;
	}
	if (Channels == 0 || Channels > 2) {
		BS_LOG_ERRORLN("Cannot create a dynamic sound with %d channels.", Channels);
		return 0;
	}

	// Zu vergebendes Handle bestimmen
	unsigned int Handle = m_NextHandle++;

	// Sound in die Sound-Map eintragen mit all den Informationen, die wir bisher haben.
	// Dies muss für dynamische Sounds so früh geschehen, da sofort nach dem Aufruf von FMOD_System_CreateSound der Callback aufgerufen wird um
	// den Decode-Buffer zu füllen. Unser Callback liest den BS-Callback aus der Sound-Map. Wenn wir den Sound später hinzufügen würden, würde der
	// BS-Callback zu diesem Sound nicht in der Map stehen und nicht aufgerufen werden können.
	// Den fehlenden ChannelPtr tragen wir später in dieser Funktion ein.
	m_PlayingSoundsMap[Handle] = PlayingSoundData(0, 0, Type, Layer, Volume, ReadCallback, UserData);

	// Dynamischen FMOD Ex Sound erstellen
	FMOD_CREATESOUNDEXINFO CreateSoundExInfo;
	memset(&CreateSoundExInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	CreateSoundExInfo.cbsize            = sizeof(FMOD_CREATESOUNDEXINFO);
	CreateSoundExInfo.length            = 0xffffffff;
	CreateSoundExInfo.numchannels       = Channels;
	CreateSoundExInfo.defaultfrequency  = SampleRate;
	CreateSoundExInfo.format            = BitsPerSampleToFMODExSoundFormat(BitsPerSample);
	CreateSoundExInfo.pcmreadcallback   = FMODExDynamicSoundReadCallback;
	CreateSoundExInfo.pcmsetposcallback = FMODExDynamicSoundSetPosCallback;
	CreateSoundExInfo.userdata          = reinterpret_cast<void *>(Handle);

	FMOD_SOUND *FMODExSoundPtr;
	FMOD_RESULT Result = FMOD_System_CreateSound(m_FMOD,
	                     0,
	                     FMOD_2D | FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_HARDWARE | FMOD_CREATESTREAM,
	                     &CreateSoundExInfo,
	                     &FMODExSoundPtr);
	if (Result != FMOD_OK) {
		BS_FMODExException("FMOD_System_CreateSound() from PlayDynamicSoundEx()", Result).Log();
		return 0;
	}

	// Neu erstellten Sound einem Kanal zuweisen
	FMOD_CHANNEL *FMODExChannelPtr;
	Result = FMOD_System_PlaySound(m_FMOD, FMOD_CHANNEL_FREE, FMODExSoundPtr, 1, &FMODExChannelPtr);
	if (Result != FMOD_OK) {
		BS_FMODExException("FMOD_System_PlaySound() from PlayDynamicSoundEx()", Result).Log();
		return 0;
	}

	// FMOD Ex Kanal an einen BS_FMODExChannel binden und abspielen
	BS_FMODExChannel *ChannelPtr = new BS_FMODExChannel(FMODExChannelPtr, FMODExSoundPtr);
	ChannelPtr->SetPaused(false);

	// ChannelPtr in die PlayingSoundData-Struktur eintragen
	PlayingSoundData *PSD = GetPlayingSoundDataByHandle(Handle);
	if (PSD) PSD->ChannelPtr = ChannelPtr;

	return Handle;
#else
	return 0;
#endif
}

// -----------------------------------------------------------------------------

#if 0
unsigned int BS_FMODExSound::PlaySoundInternal(const Common::String &FileName,
        SOUND_TYPES Type,
        float Volume,
        float Pan,
        bool Loop,
        int LoopStart, int LoopEnd,
        unsigned int Layer,
        unsigned int Position,
        unsigned int Handle) {
	BS_ASSERT(m_FMOD);
	BS_ASSERT(Type < SOUNDTYPE_COUNT);

	// Resource anfordern
	BS_Resource *ResourcePtr = BS_Kernel::GetInstance()->GetResourceManager()->RequestResource(FileName);
	if (!ResourcePtr) {
		BS_LOG_ERRORLN("Could not request resource \"%s\".", FileName.c_str());
		return 0;
	}
	if (ResourcePtr->GetType() != BS_Resource::TYPE_SOUND) {
		BS_LOG_ERRORLN("Requested resource \"%s\" is not a sound.", FileName.c_str());
		return 0;
	}
	BS_FMODExResource *SoundResourcePtr = static_cast<BS_FMODExResource *>(ResourcePtr);

	// Sound im Pause-Modus starten
	BS_FMODExChannel *ChannelPtr = SoundResourcePtr->StartSound(m_FMOD);

	if (ChannelPtr) {
		try {
			// Falls der Sound gelooped wird, Loop-Points setzen
			if (Loop) {
				// Bestimmen, welche Loop-Points benutzt werden. Falls ein Loop-Point als Parameter nicht spezifiziert wurde (Wert -1),
				// wird der Loop-Point von FMOD Ex benutzt.
				unsigned int RealLoopStart = (LoopStart > 0) ? LoopStart : ChannelPtr->GetLoopStart();
				unsigned int RealLoopEnd = (LoopEnd > 0) ? LoopEnd : ChannelPtr->GetLoopEnd();

				// Loop-Points auf Gültigkeit überprüfen
				if (RealLoopStart > RealLoopEnd) {
					BS_LOG_ERRORLN("Loop start (%d) was placed after loop end (%d) for sound \"%s\".",
					               RealLoopStart, RealLoopEnd,
					               SoundResourcePtr->GetFileName().c_str());
					throw(0);
				}
				if (RealLoopStart > ChannelPtr->GetLoopEnd()) {
					BS_LOG_ERRORLN("Loop start (%d) was placed after end (%d) of sound \"%s\".",
					               RealLoopStart,
					               ChannelPtr->GetLoopEnd(),
					               SoundResourcePtr->GetFileName().c_str());
					throw(0);
				}
				if (RealLoopEnd > ChannelPtr->GetLoopEnd()) {
					BS_LOG_ERRORLN("Loop end (%d) was placed after end (%d) of sound \"%s\".",
					               RealLoopEnd,
					               ChannelPtr->GetLoopEnd(),
					               SoundResourcePtr->GetFileName().c_str());
					throw(0);
				}

				// Loop-Points setzen
				if (!ChannelPtr->SetLoopPoints(RealLoopStart, RealLoopEnd)) throw(0);
			}

			// Sound-Parameter gemäß der Übergabeparameter setzen
			if (!ChannelPtr->SetVolume(NormalizeVolume(Volume) * m_Volumes[Type])) throw(0);
			if (!ChannelPtr->SetPanning(NormalizePanning(Pan))) throw(0);
			if (!ChannelPtr->SetLoop(Loop)) throw(0);
			if (!ChannelPtr->SetPosition(Position)) throw(0);
		} catch (...) {
			delete ChannelPtr;
			SoundResourcePtr->Release();
			return 0;
		}

		unsigned int MyLoopStart = ChannelPtr->GetLoopStart();
		unsigned int MyLoopEnd = ChannelPtr->GetLoopEnd();
		ChannelPtr->SetLoopPoints(MyLoopStart, MyLoopEnd);

		// Sound abspielen
		ChannelPtr->SetPaused(false);

		// Sound in die Sound-Map eintragen
		unsigned int NewHandle = (Handle != 0) ? Handle : m_NextHandle++;
		m_PlayingSoundsMap[NewHandle] = PlayingSoundData(SoundResourcePtr, ChannelPtr, Type, Layer, Volume);

		return NewHandle;
	} else {
		SoundResourcePtr->Release();
		return 0;
	}
}
#endif
// -----------------------------------------------------------------------------
// Sonstige Methoden
// -----------------------------------------------------------------------------

void BS_FMODExSound::SetVolume(float Volume, SOUND_TYPES Type) {
#if 0
	BS_ASSERT(m_FMOD);
	BS_ASSERT(Type < SOUNDTYPE_COUNT);
	m_Volumes[Type] = NormalizeVolume(Volume);

	// Alle Volumen der Sounds der Kategorie aktualisieren
	PSM_CONST_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		const PlayingSoundData &PSD = it->second;
		if (PSD.ChannelPtr && PSD.Type == Type) PSD.ChannelPtr->SetVolume(Volume * PSD.Volume);

		++it;
	}
#endif
}

// -----------------------------------------------------------------------------

float BS_FMODExSound::GetVolume(SOUND_TYPES Type) {
#if 0
	BS_ASSERT(m_FMOD);
	BS_ASSERT(Type < SOUNDTYPE_COUNT);
	return m_Volumes[Type];
#else
	return 0;
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::PauseAll() {
#if 0
	BS_ASSERT(m_FMOD);

	// Alle Sounds durchgehen und alle pausieren.
	// Diese werden dann markiert, damit ResumeAll() feststellen kann, welche Sounds mit PauseAll() pausiert wurden.
	// ResumeAll() setzt dann nur diejenigen fort, die nur über PauseAll() pausiert wurden.
	PSM_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		PlayingSoundData &PSD = it->second;

		if (PSD.ChannelPtr) PSD.ChannelPtr->SetPaused(true);
		PSD.PausedGlobal = true;

		++it;
	}
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::ResumeAll() {
#if 0
	BS_ASSERT(m_FMOD);

	// Alle Sounds durchgehen, die gloable Pause aufheben und diejenigen fortsetzen,
	// die keine Pause mehr haben (weder explizit, über den Layer oder global).
	PSM_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		PlayingSoundData &PSD = it->second;

		if (PSD.PausedGlobal) {
			PSD.PausedGlobal = false;
			if (PSD.ChannelPtr && !PSD.PausedLayer && !PSD.Paused) PSD.ChannelPtr->SetPaused(false);
		}

		++it;
	}
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::PauseLayer(unsigned int Layer) {
#if 0
	BS_ASSERT(m_FMOD);

	// Alle Sounds durchgehen und alle pausieren, die sich auf den angegebenen Layer befinden.
	// Diese werden dann markiert, damit ResumeLayer() feststellen kann, welche Sounds mit PauseLayer() pausiert wurden.
	// ResumeLayer() setzt dann nur diejenigen fort, die nur über PauseLayer() mit der entsprechenden Layer-Nummer pausiert wurden.
	PSM_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		PlayingSoundData &PSD = it->second;

		if (PSD.Layer == Layer) {
			if (PSD.ChannelPtr) PSD.ChannelPtr->SetPaused(true);
			PSD.PausedLayer = true;
		}

		++it;
	}
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::ResumeLayer(unsigned int Layer) {
#if 0
	BS_ASSERT(m_FMOD);

	// Alle Sounds durchgehen, die Layer-Pause aufheben und diejenigen fortsetzen,
	// die keine Pause mehr haben (weder explizit, über den Layer oder global).
	PSM_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		PlayingSoundData &PSD = it->second;

		if (PSD.PausedLayer && PSD.Layer == Layer) {
			PSD.PausedLayer = false;
			if (PSD.ChannelPtr && !PSD.PausedGlobal && !PSD.Paused) PSD.ChannelPtr->SetPaused(false);
		}

		++it;
	}
#endif
}

// -----------------------------------------------------------------------------
// Sound Setter
// -----------------------------------------------------------------------------

void BS_FMODExSound::SetSoundVolume(unsigned int Handle, float Volume) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr) if (PSDPtr->ChannelPtr && PSDPtr->ChannelPtr->SetVolume(NormalizeVolume(Volume) * m_Volumes[PSDPtr->Type])) PSDPtr->Volume = Volume;
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::SetSoundPanning(unsigned int Handle, float Pan) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr && PSDPtr->ChannelPtr) PSDPtr->ChannelPtr->SetPanning(NormalizePanning(Pan));
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::PauseSound(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr) {
		PSDPtr->Paused = true;
		if (PSDPtr->ChannelPtr) PSDPtr->ChannelPtr->SetPaused(true);
	}
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::ResumeSound(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr) {
		PSDPtr->Paused = false;
		if (PSDPtr->ChannelPtr && !PSDPtr->PausedGlobal && !PSDPtr->PausedLayer) PSDPtr->ChannelPtr->SetPaused(false);
	}
#endif
}

// -----------------------------------------------------------------------------

void BS_FMODExSound::StopSound(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr && PSDPtr->ChannelPtr) PSDPtr->ChannelPtr->Stop();
#endif
}

// -----------------------------------------------------------------------------
// Sound Getter
// -----------------------------------------------------------------------------

bool BS_FMODExSound::IsSoundPaused(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr && PSDPtr->ChannelPtr) return PSDPtr->ChannelPtr->IsPaused();
#endif
	return false;
}

// -----------------------------------------------------------------------------

bool BS_FMODExSound::IsSoundPlaying(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr && PSDPtr->ChannelPtr) return PSDPtr->ChannelPtr->IsPlaying();
#endif
	return false;
}

// -----------------------------------------------------------------------------

float BS_FMODExSound::GetSoundVolume(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr) return PSDPtr->Volume;
#endif
	return 0;
}

// -----------------------------------------------------------------------------

float BS_FMODExSound::GetSoundPanning(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr && PSDPtr->ChannelPtr) return PSDPtr->ChannelPtr->GetPanning();
#endif
	return 0;
}

// -----------------------------------------------------------------------------

float BS_FMODExSound::GetSoundTime(unsigned int Handle) {
#if 0
	BS_ASSERT(m_FMOD);
	PlayingSoundData *PSDPtr = GetPlayingSoundDataByHandle(Handle);
	if (PSDPtr && PSDPtr->ChannelPtr) return static_cast<float>(PSDPtr->ChannelPtr->GetTime()) / 1000.0f;
#endif
	return 0;
}

#if 0
void BS_FMODExSound::RemoveInactiveSounds() {
	PSM_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		if (!it->second.ChannelPtr || !it->second.ChannelPtr->IsPlaying()) {
			PlayingSoundData &PSD = it->second;

			delete PSD.ChannelPtr;
			if (PSD.ResourcePtr) PSD.ResourcePtr->Release();

			it = m_PlayingSoundsMap.erase(it);
		} else
			++it;
	}

	/*
	static size_t lastActiveChannels = 0;
	if (m_PlayingSoundsMap.size() != lastActiveChannels)
	{
	    BS_LOGLN("Aktive Kanaele: %d", m_PlayingSoundsMap.size());
	    lastActiveChannels = m_PlayingSoundsMap.size();
	}
	*/
}

// -----------------------------------------------------------------------------

BS_FMODExSound::PlayingSoundData *BS_FMODExSound::GetPlayingSoundDataByHandle(unsigned int Handle) {
	// Zum Soundhandle gehörige Daten in der Hash-Map finden
	PSM_ITER it = m_PlayingSoundsMap.find(Handle);
	// Falls die Daten nicht gefunden werden konnten, Fehler zurückgebene, ansonsten ein Pointer auf die Daten.
	if (it == m_PlayingSoundsMap.end()) return 0;
	return &((*it).second);
}

// -----------------------------------------------------------------------------

unsigned int BS_FMODExSound::CountPlayingDynamicSounds() {
	unsigned int Result = 0;
	for (PSM_CONST_ITER it = m_PlayingSoundsMap.begin(); it != m_PlayingSoundsMap.end(); ++it) if (!it->second.ResourcePtr) ++Result;

	return Result;
}
#endif

// -----------------------------------------------------------------------------
// Ressourcen-Verwaltung
// -----------------------------------------------------------------------------

BS_Resource *BS_FMODExSound::LoadResource(const Common::String &FileName) {
#if 0
	BS_ASSERT(m_FMOD);
	BS_ASSERT(CanLoadResource(FileName));

	bool Success;
	BS_FMODExResource *ResourcePtr = new BS_FMODExResource(FileName, m_FMOD, Success);
	if (Success)
		return ResourcePtr;
	else {
		delete ResourcePtr;
		return 0;
	}
#else
	return 0;
#endif
}
bool BS_FMODExSound::CanLoadResource(const Common::String &FileName) {
#if 0
	if (FileName.size() >= 4) {
		Common::String Extension(FileName.end() - 4, FileName.end());
		BS_String::ToLower(Extension);

		return  Extension == ".wav" ||
		        Extension == ".ogg" ||
		        Extension == ".mp3";
	} else
		return false;
#else
	return true;
#endif
}

// -----------------------------------------------------------------------------
// Persistenz
// -----------------------------------------------------------------------------

bool BS_FMODExSound::Persist(BS_OutputPersistenceBlock &Writer) {
#if 0
	BS_ASSERT(m_FMOD);

	// Alle inaktiven Sounds entfernen, damit kein unnötiger Ballast gespeichert wird
	RemoveInactiveSounds();

	// Warnung ausgeben, wenn dynamische Sounds abgespielt werden
	unsigned int PlayingDynamicSounds = CountPlayingDynamicSounds();
	if (PlayingDynamicSounds) BS_LOG_WARNINGLN("There are currently dynamic sounds playing. These will not be persisted.");

	// Nächstes Handle speichern
	Writer.Write(m_NextHandle);

	// Anzahl spielender (nicht dynamischer) Sounds speichern
	Writer.Write(m_PlayingSoundsMap.size() - PlayingDynamicSounds);

	// Informationen für jeden spielenden (nicht dynamischen) Sound speichern
	PSM_CONST_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		const PlayingSoundData &PSD = it->second;

		if (PSD.ResourcePtr) {
			// Handle speichern
			Writer.Write(it->first);

			// Soundeigenschaften speichern
			Writer.Write(PSD.ResourcePtr->GetFileName());
			Writer.Write(static_cast<unsigned int>(PSD.Type));
			Writer.Write(PSD.Layer);

			Writer.Write(PSD.Volume);
			Writer.Write(PSD.ChannelPtr->GetPanning());
			Writer.Write(PSD.ChannelPtr->IsLooping());
			Writer.Write(PSD.ChannelPtr->GetLoopStart());
			Writer.Write(PSD.ChannelPtr->GetLoopEnd());
			Writer.Write(PSD.ChannelPtr->GetPosition());
			Writer.Write(PSD.Paused);
			Writer.Write(PSD.PausedLayer);
			Writer.Write(PSD.PausedGlobal);
		}

		++it;
	}

#endif
	return true;
}

// -----------------------------------------------------------------------------

bool BS_FMODExSound::Unpersist(BS_InputPersistenceBlock &Reader) {
#if 0
	BS_ASSERT(m_FMOD);

	// Alle Sounds stoppen
	PSM_ITER it = m_PlayingSoundsMap.begin();
	while (it != m_PlayingSoundsMap.end()) {
		const PlayingSoundData &PSD = it->second;
		if (PSD.ChannelPtr) delete PSD.ChannelPtr;
		if (PSD.ResourcePtr) PSD.ResourcePtr->Release();
		++it;
	}

	// Sound-Map leeren
	m_PlayingSoundsMap.clear();

	// Nächstes Handle laden
	Reader.Read(m_NextHandle);

	// Soundanzahl einlesen
	unsigned int SoundCount = 0;
	Reader.Read(SoundCount);

	// Informationen über jeden spielenden Sound einlesen und ihn mit den Parametern abspielen
	for (unsigned int i = 0; i < SoundCount; ++i) {
		unsigned int    Handle;
		Common::String     FileName;
		unsigned int    Type;
		unsigned int    Layer;

		float           Volume;
		float           Pan;
		bool            Loop;
		unsigned int    LoopStart;
		unsigned int    LoopEnd;
		unsigned int    Position;
		bool            Paused;
		bool            PausedLayer;
		bool            PausedGlobal;

		Reader.Read(Handle);
		Reader.Read(FileName);
		Reader.Read(Type);
		Reader.Read(Layer);

		Reader.Read(Volume);
		Reader.Read(Pan);
		Reader.Read(Loop);
		Reader.Read(LoopStart);
		Reader.Read(LoopEnd);
		Reader.Read(Position);
		Reader.Read(Paused);
		Reader.Read(PausedLayer);
		Reader.Read(PausedGlobal);

		if (Reader.IsGood()) {
			PlaySoundInternal(FileName, (SOUND_TYPES) Type, Volume, Pan, Loop, LoopStart, LoopEnd, Layer, Position, Handle);
		} else {
			return false;
		}
	}

	return Reader.IsGood();
#else
	return true;
#endif
}

} // End of namespace Sword25
