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

// The code in this file is based in part on code from the OggTheora Software
// codec source released under the following terms:
//
// Copyright (C) 2002-2007 Xiph.org Foundation
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// - Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// - Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// - Neither the name of the Xiph.org Foundation nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "OGGTHEORA"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/memlog_off.h"
#include <algorithm>
#include "sword25/kernel/memlog_on.h"
#include <float.h>

#include "sword25/package/packagemanager.h"
#include "sword25/sfx/soundengine.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/panel.h"
#include "sword25/fmv/oggtheora/oggtheora.h"
#include "sword25/fmv/oggtheora/yuvtorgba.h"

using namespace std;

// -----------------------------------------------------------------------------
// Konstanten und Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	// --------------------------------------------------------------------------

	const int MAX_FRAMES_PER_TICK = 10;
	const int READ_BLOCK_SIZE = 1024 * 40;
	const float MAX_AUDIO_BUFFER_LENGTH = 1.0f;

	// --------------------------------------------------------------------------

	template<typename T>
	inline T Clamp(T x, T low, T high)
	{
		return ((x > high) ? high : (( x < low) ? low : x));
	}
};

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_OggTheora::BS_OggTheora(BS_Kernel * pKernel) : BS_MoviePlayer(pKernel), m_SoundHandle(0)
{
	UnloadMovie();
}

// -----------------------------------------------------------------------------

BS_OggTheora::~BS_OggTheora()
{
	UnloadMovie();
}

// -----------------------------------------------------------------------------

BS_Service * BS_OggTheora_CreateObject(BS_Kernel* pKernel) { return new BS_OggTheora(pKernel); }

// -----------------------------------------------------------------------------
// BS_MoviePlayer Interface
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// LoadMovie() mit Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	inline bool VerifyRequiredServiceAvailability()
	{
		char * RequiredServices[] = { "gfx", "sfx", "package" };
		for (size_t i = 0; i < sizeof(RequiredServices) / sizeof(RequiredServices[0]); ++i)
		{
			if (!BS_Kernel::GetInstance()->GetService(RequiredServices[i]))
			{
				BS_LOG_ERRORLN("Required service \"%s\" is not active.", RequiredServices[i]);
				return false;
			}
		}

		return true;
	}

	// -------------------------------------------------------------------------

	bool ParseStreamHeaders(auto_ptr<BS_MovieFile> & File,
							auto_ptr<BS_OggState> & OggState,
							auto_ptr<BS_OggStreamState> & TheoraStreamState,
							auto_ptr<BS_OggStreamState> & VorbisStreamState,
							auto_ptr<BS_TheoraState> & TheoraState,
							auto_ptr<BS_VorbisState> & VorbisState,
							bool & TheoraPresent,
							bool & VorbisPresent,
							const std::string & Filename)
	{
		TheoraPresent = false;
		VorbisPresent = false;

		// Ogg file open; parse the headers
		// Only interested in Vorbis/Theora streams
		bool FinishedHeaderParsing = false;
		while (!FinishedHeaderParsing)
		{
			if (File->BufferData(*OggState.get()) == 0) return false;

			ogg_page Page;
			while(OggState->SyncPageout(&Page) > 0)
			{
				// is this a mandated initial header? If not, stop parsing
				if(!ogg_page_bos(&Page))
				{
					// don't leak the page; get it into the appropriate stream
					if (TheoraPresent) TheoraStreamState->PageIn(&Page);
					if (VorbisPresent) VorbisStreamState->PageIn(&Page);

					FinishedHeaderParsing = true;
					break;
				}

				auto_ptr<BS_OggStreamState> streamState(new BS_OggStreamState(ogg_page_serialno(&Page)));

				streamState->PageIn(&Page);

				ogg_packet Packet;
				streamState->PacketOut(&Packet);

				// identify the codec: try theora
				if(!TheoraPresent && TheoraState->DecodeHeader(&Packet) >= 0)
				{
					// it is theora
					TheoraStreamState = streamState;
					TheoraPresent = true;
				}
				else if(!VorbisPresent && VorbisState->SynthesisHeaderIn(&Packet) >=0)
				{
					// it is vorbis
					VorbisStreamState = streamState;
					VorbisPresent = true;
				}
			}
			// fall through to non-bos page parsing
		}

		// we're expecting more header packets.
		unsigned int TheoraPacketsRead = TheoraPresent ? 1 : 0;
		unsigned int VorbisPacketsRead = VorbisPresent ? 1 : 0;
		while((TheoraPresent && TheoraPacketsRead < 3) || (VorbisPresent && VorbisPacketsRead < 3))
		{
			int ret;
			ogg_packet Packet;

			// look for further theora headers
			while(TheoraPresent && (TheoraPacketsRead < 3) && (ret = TheoraStreamState->PacketOut(&Packet)))
			{
				if(ret < 0 || TheoraState->DecodeHeader(&Packet))
				{
					BS_LOG_ERRORLN("Error parsing Theora stream headers. Stream is possibly corrupt. (%s)", Filename.c_str());
					return false;
				}

				++TheoraPacketsRead;
				if (TheoraPacketsRead == 3) break;
			}

			// look for more vorbis header packets
			while(VorbisPresent && (VorbisPacketsRead < 3) && (ret = VorbisStreamState->PacketOut(&Packet)))
			{
				if(ret < 0 || VorbisState->SynthesisHeaderIn(&Packet))
				{
					BS_LOG_ERRORLN("Error parsing Vorbis stream headers. Stream is possibly corrupt. (%s)", Filename.c_str());
					return false;
				}

				++VorbisPacketsRead;
				if (VorbisPacketsRead == 3) break;
			}

			// The header pages/packets will arrive before anything else we care about, or the stream is not obeying spec
			ogg_page Page;
			if(OggState->SyncPageout(&Page) > 0)
			{
				// demux into the appropriate stream
				if(TheoraPresent) TheoraStreamState->PageIn(&Page);
				if(VorbisPresent) VorbisStreamState->PageIn(&Page);
			}
			else
			{
				if(File->BufferData(*OggState.get()) == 0)
				{
					BS_LOG_ERRORLN("End of file while searching for codec headers. (%s)", Filename.c_str());
					return false;
				}
			}
		}

		return true;
	}
}

// -----------------------------------------------------------------------------

bool BS_OggTheora::LoadMovie(const std::string & Filename, unsigned int Z)
{
	if (!VerifyRequiredServiceAvailability()) return false;

	UnloadMovie();

	// Alle Objekte die in dieser Funktion erzeugt werden, werden in zunächst lokalen Auto-Pointern gehalten.
	// Bei erfolgreicher Beendigung dieser Funktion werden sie den objektlokalen Auto-Pointern zugewiesen.
	// So wird sichergestellt, dass sie korrekt deinitialisiert werden, wenn in dieser Funktion ein Fehler auftritt, denn beim Zerstören der
	// lokalen Auto-Pointer werden die entsprechenden Destruktoren aufgerufen.

	// Film laden
	// Für Filmdateien wird das Cachingsystem nicht benutzt, da Filme in der Regel nur ein Mal abgespielt werden.
	bool Success;
	auto_ptr<BS_MovieFile> File(new BS_MovieFile(Filename, READ_BLOCK_SIZE, Success));
	if (!Success) return false;

	// States erzeugen für Ogg, Vorbis und Theora, sowie die Ogg und Theora Streams
	auto_ptr<BS_OggState> OggState(new BS_OggState());
	auto_ptr<BS_VorbisState> VorbisState(new BS_VorbisState());
	auto_ptr<BS_TheoraState> TheoraState(new BS_TheoraState());

	auto_ptr<BS_OggStreamState> TheoraStreamState;
	auto_ptr<BS_OggStreamState> VorbisStreamState;

	if (!ParseStreamHeaders(File, OggState, TheoraStreamState, VorbisStreamState, TheoraState, VorbisState, m_TheoraPresent, m_VorbisPresent, Filename)) return false;

	// Theora-Decoder Initialisieren
	if(m_TheoraPresent)
	{
		TheoraState->DecodeInit();

		const theora_info & TheoraInfo = TheoraState->GetInfo();

		if (TheoraInfo.pixelformat != OC_PF_444 &&
			TheoraInfo.pixelformat != OC_PF_422 &&
			TheoraInfo.pixelformat != OC_PF_420)
		{
			BS_LOG_ERRORLN("Unknown chroma sampling. (%s)", Filename.c_str());
			return false;
		}

		// Ausgabebitmap erstellen
		BS_GraphicEngine * pGfx = BS_Kernel::GetInstance()->GetGfx();
		m_OutputBitmap = pGfx->GetMainPanel()->AddDynamicBitmap(TheoraInfo.frame_width, TheoraInfo.frame_height);
		if (!m_OutputBitmap.IsValid())
		{
			BS_LOG_ERRORLN("Output bitmap for movie playback could not be created.");
			return false;
		}

		// Skalierung des Ausgabebitmaps berechnen, so dass es möglichst viel Bildschirmfläche einnimmt.
		float ScreenToVideoWidth = (float) pGfx->GetDisplayWidth() / (float) m_OutputBitmap->GetWidth();
		float ScreenToVideoHeight = (float) pGfx->GetDisplayHeight() / (float) m_OutputBitmap->GetHeight();
		float ScaleFactor = std::min(ScreenToVideoWidth, ScreenToVideoHeight);
		if (abs(ScaleFactor - 1.0f) < FLT_EPSILON) ScaleFactor = 1.0f;
		m_OutputBitmap->SetScaleFactor(ScaleFactor);

		// Z-Wert setzen
		m_OutputBitmap->SetZ(Z);

		// Ausgabebitmap auf dem Bildschirm zentrieren
		m_OutputBitmap->SetX((pGfx->GetDisplayWidth() - m_OutputBitmap->GetWidth()) / 2);
		m_OutputBitmap->SetY((pGfx->GetDisplayHeight() - m_OutputBitmap->GetHeight()) / 2);

		// Buffer für die Pixeldaten erstellen
		m_Pixels.resize(TheoraInfo.width * TheoraInfo.height * 4);

		m_VideoEnded = false;
	}

	// Vorbis-Decoder initialisieren
	if(m_VorbisPresent)
	{
		VorbisState->SynthesisInit();
		VorbisState->BlockInit();
		m_AudioBuffer.reset(new BS_AudioBuffer());

		m_AudioEnded = false;
	}

	// Keine Kopie, überträgt Besitz der erzeugten Objekte von der Funktion auf das Objekt.
	m_File = File;
	m_OggState = OggState;
	m_TheoraState = TheoraState;
	m_TheoraStreamState = TheoraStreamState;
	m_VorbisState = VorbisState;
	m_VorbisStreamState = VorbisStreamState;
	m_MovieLoaded = true;
	m_Timer = 0;

	return true;
}

// -----------------------------------------------------------------------------

bool BS_OggTheora::UnloadMovie()
{
	m_MovieLoaded = false;
	m_Paused = true;

	m_VorbisStreamState.reset();
	m_VorbisPresent = false;
	m_VorbisState.reset();
	if (m_SoundHandle)
	{
		BS_Kernel::GetInstance()->GetSfx()->StopSound(m_SoundHandle);
		m_SoundHandle = 0;
	}
	m_AudioEnded = true;
	m_AudioBuffer.reset();

	m_TheoraStreamState.reset();
	m_TheoraPresent = false;
	m_TheoraState.reset();
	m_VideoEnded = true;

	m_OggState.reset();

	m_File.reset();

	m_StartTime = 0;
	m_LastFrameTime = 0;
	m_Timer = 0.0f;

	vector<unsigned char>().swap(m_Pixels);
	m_OutputBitmap.Erase();

	return true;
}

// -----------------------------------------------------------------------------

bool BS_OggTheora::Play()
{
	if (m_MovieLoaded)
	{
		if (m_Paused)
		{
			if (m_SoundHandle)
			{
				BS_SoundEngine * SfxPtr = BS_Kernel::GetInstance()->GetSfx();
				SfxPtr->ResumeSound(m_SoundHandle);
			}
			m_Paused = false;
		}

		return true;
	}
	else
	{
		BS_LOG_WARNINGLN("Cannot play movie, when no movie is loaded.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool BS_OggTheora::Pause()
{
	if (m_MovieLoaded)
	{
		if (m_SoundHandle)
		{
			BS_SoundEngine * SfxPtr = BS_Kernel::GetInstance()->GetSfx();
			SfxPtr->PauseSound(m_SoundHandle);
		}

		m_Paused = true;
		return true;
	}
	else
	{
		BS_LOG_WARNINGLN("Cannot pause movie, when no movie is loaded.");
		return false;
	}
}

// -----------------------------------------------------------------------------

void BS_OggTheora::ReadData()
{
	if (!m_File->IsEOF()) m_File->BufferData(*m_OggState.get());

	ogg_page Page;
	while(m_OggState->SyncPageout(&Page) > 0)
	{
		if(m_TheoraPresent) m_TheoraStreamState->BufferPage(&Page);
		if(m_VorbisPresent) m_VorbisStreamState->BufferPage(&Page);
	}
}

void BS_OggTheora::Update()
{
	if (m_AudioEnded && m_VideoEnded)
	{
		m_Paused = true;

		// Falls der Sound noch läuft, muss er jetzt beendet werden.
		if (m_SoundHandle)
		{
			BS_SoundEngine * SfxPtr = BS_Kernel::GetInstance()->GetSfx();
			SfxPtr->StopSound(m_SoundHandle);
			m_SoundHandle = 0;
		}
	}

	if (m_Paused) return;

	// Timer aktualisieren.
	// Wird nur genutzt, wenn keine Audiodaten vorhanden sind.
	m_Timer += BS_Kernel::GetInstance()->GetGfx()->GetSecondaryFrameDuration();

	// Audiodaten dekodieren
	if (m_VorbisPresent && !m_AudioEnded) DecodeVorbis();


	// Videodaten dekodieren
	if (m_TheoraPresent && !m_VideoEnded)
	{
		bool Result = DecodeTheora();

		if (Result)
		{
			// YUV Framebuffer holen
			yuv_buffer YUVBuffer;
			m_TheoraState->DecodeYUVOut(&YUVBuffer);

			// YUV Bilddaten nach RGBA konvertieren
			BS_YUVtoRGBA::YUVtoRGBA(YUVBuffer, m_TheoraState->GetInfo(), m_Pixels);

			// RGBA Bilddaten auf das Ausgabebild kopieren, dabei die Postion des Theoraframes innerhalb des dekodierten Frames beachten.
			const theora_info & TheoraInfo = m_TheoraState->GetInfo();
			m_OutputBitmap->SetContent(m_Pixels, (TheoraInfo.offset_x + TheoraInfo.width * TheoraInfo.offset_y) * 4, (TheoraInfo.width - TheoraInfo.frame_width) * 4);
		}
	}
}

// -----------------------------------------------------------------------------

bool BS_OggTheora::DecodeTheora()
{
	double MovieTime = GetTime();

	// Check if this frame time has not passed yet. If the frame is late we need
	// to decode additonal ones and keep looping, since theora at this stage
	// needs to decode all frames (due to keyframing)
	// Getting the current time once at the beginning of the function rather than
	// every time at the beginning of the loop produces the smoothest framerate

	unsigned int FramesDecoded = 0;
	bool FrameReady = false;
	while (m_TheoraState->GranuleTime() < MovieTime)
	{
		// theora is one in, one out...
		ogg_packet Packet;
		if(m_TheoraStreamState->PacketOut(&Packet) > 0)
		{
			if (FramesDecoded < MAX_FRAMES_PER_TICK || Packet.granulepos >= MovieTime)
			{
				m_TheoraState->DecodePacketIn(&Packet);
				FrameReady = true;
				++FramesDecoded;
			}
		}
		else
		{
			if (m_TheoraStreamState->GetPageBufferSize() > 0)
				m_TheoraStreamState->PageInBufferedPage();
			else
			{
				if(m_File->IsEOF())
				{
					m_VideoEnded = true;
					break;
				}
				else
					ReadData();
			}
		}
	}
	m_LastFrameTime = MovieTime;

	return FrameReady;
}

// -----------------------------------------------------------------------------

void BS_OggTheora::DynamicSoundCallBack(void * UserData, void * Data, unsigned int DataLength)
{
	BS_OggTheora & t = *reinterpret_cast<BS_OggTheora *>(UserData);

	signed short * Buffer = reinterpret_cast<signed short *>(Data);

	// Audiodaten in den Soundbuffer schreiben.
	DataLength -= t.m_AudioBuffer->Pop(Buffer, DataLength / 2) * 2;

	// Falls nicht genug Audiodaten vorhanden waren, wird der Rest mit Stille überschrieben.
	if (DataLength)
	{
		char * ByteBuffer = reinterpret_cast<char *>(Buffer);
		while (DataLength--)
		{
			*ByteBuffer++ = 0;
		}
	}
}

// -----------------------------------------------------------------------------

void BS_OggTheora::DecodeVorbis()
{
	// Vorbis-Stream Infos holen.
	const vorbis_info & Info = m_VorbisState->GetInfo();

	// Maximalgröße des Audiobuffers berechnen.
	size_t MaxAudioBufferSamples = static_cast<size_t>(Info.channels * Info.rate * MAX_AUDIO_BUFFER_LENGTH);

	// Zwischenspeicher für die Samples.
	vector<signed short> Samples;

	// Audiobuffer bis zur Maximalgröße füllen, wenn möglich.
	while (m_AudioBuffer->Size() < MaxAudioBufferSamples)
	{
		// Vorhandene Audiodaten auslesen
		float ** PCM;
		int SampleCount = m_VorbisState->SynthesisPCMout(&PCM);

		// Wenn Audiodaten gelesen werden konnten, werden diese in 16-Bit Sample umgewandelt und in den Audiobuffer geschrieben.
		if(SampleCount > 0)
		{
			// Im Samplezwischenspeicher leeren und genügend Platz für die kommenden Samples reservieren.
			Samples.reserve(SampleCount);
			Samples.clear();

			// Samples konvertieren und in die Samplezwischenspeicher schreiben.
			for (int i = 0; i < SampleCount; ++i)
			{
				for(int j = 0; j < Info.channels; ++j)
				{
					int SampleValue = static_cast<int>(PCM[j][i] * 32767.0f);
					Samples.push_back(Clamp(SampleValue, -32700, 32700));
				}
			}

			// Daten aus dem Samplezwischenspeicher in den Audiobuffer schreiben.
			m_AudioBuffer->Push(&Samples[0], Samples.size());

			// Vorbis mitteilen, dass wir alle Samples gelesen haben.
			m_VorbisState->SynthesisRead(SampleCount);
		}
		else
		{        
			// Wir konnten, keine Audiodaten auslesen, versuchen ein neues Paket zu dekodieren.
			ogg_packet Packet;
			if(m_VorbisStreamState->PacketOut(&Packet) > 0)
			{
				if (m_VorbisState->Synthesis(&Packet) == 0) m_VorbisState->SynthesisBlockIn();
			}
			else
			{
				// Gepufferte Daten in den Stream einfügen.
				if (m_VorbisStreamState->GetPageBufferSize() > 0)
					m_VorbisStreamState->PageInBufferedPage();
				else
				{
					// Nicht genug Daten vorhanden. Wenn die Datei leer ist und bereits alle Audiodaten gelesen wurden, ist der Audiostream am Ende.
					// Ansonsten Daten nachladen.
					if(m_File->IsEOF())
					{
						if (m_AudioBuffer->Size() == 0)
						{
							m_AudioEnded = true;
						}

						break;
					}
					else
						ReadData();
				}
			}
		}
	}

	// Soundkanal abspielen, wenn er noch nicht spielt und Audiodaten vorhanden sind.
	if (m_SoundHandle == 0 && m_AudioBuffer->Size())
	{
		BS_SoundEngine * SfxPtr = BS_Kernel::GetInstance()->GetSfx();
		m_SoundHandle = SfxPtr->PlayDynamicSoundEx(&DynamicSoundCallBack, this, BS_SoundEngine::SFX, Info.rate, 16, Info.channels);
	}
}

// -----------------------------------------------------------------------------

double BS_OggTheora::GetTime()
{
	if(m_VorbisPresent)
	{
		if (m_SoundHandle)
		{
			float time = BS_Kernel::GetInstance()->GetSfx()->GetSoundTime(m_SoundHandle);
			return time;
		}
		else
			return 0.0f;
	}
	else
		return m_Timer;
}

// -----------------------------------------------------------------------------

bool BS_OggTheora::IsMovieLoaded()
{
	return m_MovieLoaded;
}

// -----------------------------------------------------------------------------

bool BS_OggTheora::IsPaused()
{
	return m_Paused;
}

// -----------------------------------------------------------------------------

float BS_OggTheora::GetScaleFactor()
{
	if (m_MovieLoaded)
		return m_OutputBitmap->GetScaleFactorX();
	else
		return 0;
}

// -----------------------------------------------------------------------------

void BS_OggTheora::SetScaleFactor(float ScaleFactor)
{
	if (m_MovieLoaded)
	{
		m_OutputBitmap->SetScaleFactor(ScaleFactor);

		// Ausgabebitmap auf dem Bildschirm zentrieren
		BS_GraphicEngine * GfxPtr = BS_Kernel::GetInstance()->GetGfx();
		m_OutputBitmap->SetX((GfxPtr->GetDisplayWidth() - m_OutputBitmap->GetWidth()) / 2);
		m_OutputBitmap->SetY((GfxPtr->GetDisplayHeight() - m_OutputBitmap->GetHeight()) / 2);
	}
}
