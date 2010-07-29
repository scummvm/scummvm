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

#ifndef SWORD25_OGGTHEORA_H
#define SWORD25_OGGTHEORA_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include <memory>
#include <queue>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/bs_stdint.h"
#include "sword25/gfx/bitmap.h"
#include "sword25/gfx/renderobjectptr.h"
#include "sword25/fmv/movieplayer.h"
#include "sword25/fmv/oggtheora/vorbisstate.h"
#include "sword25/fmv/oggtheora/theorastate.h"
#include "sword25/fmv/oggtheora/oggstate.h"
#include "sword25/fmv/oggtheora/oggstreamstate.h"
#include "sword25/fmv/oggtheora/moviefile.h"
#include "sword25/fmv/oggtheora/audiobuffer.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_OggTheora : public BS_MoviePlayer
{
public:
	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// -----------------------------------------------------------------------------

	BS_OggTheora(BS_Kernel * pKernel);
	virtual ~BS_OggTheora();

	// -----------------------------------------------------------------------------
	// BS_MoviePlayer Interface
	// -----------------------------------------------------------------------------

	virtual bool LoadMovie(const std::string & Filename, unsigned int Z);
	virtual bool UnloadMovie();
	virtual bool Play();
	virtual bool Pause();
	virtual void Update();
	virtual bool IsMovieLoaded();
	virtual bool IsPaused();
	virtual float GetScaleFactor();
	virtual void SetScaleFactor(float ScaleFactor);
	virtual double GetTime();

private:
	bool	DecodeTheora();
	void	DecodeVorbis();
	void	RefillOggBuffer();
	void	ReadData();
	static	void DynamicSoundCallBack(void * UserData, void * Data, unsigned int DataLength);

	bool								m_MovieLoaded;
	bool								m_Paused;

	std::auto_ptr<BS_OggStreamState>	m_VorbisStreamState;
	bool								m_VorbisPresent;
	std::auto_ptr<BS_VorbisState>		m_VorbisState;
	unsigned int						m_SoundHandle;
	bool								m_AudioEnded;
	std::auto_ptr<BS_AudioBuffer>		m_AudioBuffer;

	std::auto_ptr<BS_OggStreamState>	m_TheoraStreamState;
	bool								m_TheoraPresent;
	std::auto_ptr<BS_TheoraState>		m_TheoraState;
	bool								m_VideoEnded;

	std::auto_ptr<BS_OggState>			m_OggState;

	std::auto_ptr<BS_MovieFile>			m_File;

	uint64_t							m_StartTime;
	double								m_LastFrameTime;

	float								m_Timer;

	std::vector<unsigned char>			m_Pixels;
	BS_RenderObjectPtr<BS_Bitmap>		m_OutputBitmap;
};

#endif
