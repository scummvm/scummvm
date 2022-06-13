/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#if 0//ndef HPL_VIDEO_STREAM_THEORA_H
#define HPL_VIDEO_STREAM_THEORA_H

#include "hpl1/engine/graphics/VideoStream.h"

//#include <theora/theora.h>

namespace hpl {

	//-----------------------------------------
	class  cVideoStreamTheora_Loader;

	class cVideoStreamTheora : public iVideoStream
	{
	public:
		cVideoStreamTheora(tString asName, cVideoStreamTheora_Loader* apLoader);
		~cVideoStreamTheora();

		bool LoadFromFile(tString asFilePath);

		void Update(float afTimeStep);

		void Play();
		void Stop();

		void Pause(bool abX);
		bool IsPaused(){ return mbPaused;}

		void SetLoop(bool abX);
		bool IsLooping(){ return mbLooping;}

		void CopyToTexture(iTexture *apTexture);

	private:
		void DrawFrameToBuffer();
		int BufferData(FILE *pFile ,ogg_sync_state *apOggSynchState);
		void QueuePage(ogg_page *apPage);
		bool GetHeaders();
		bool InitDecoders();
		void ResetStreams();

		cVideoStreamTheora_Loader *mpLoader;

		FILE *mpFile;

		bool mbLooping;
		bool mbPaused;
		bool mbPlaying;

		float mfTime;

		unsigned char *mpFrameBuffer;

		ogg_sync_state   mOggSyncState;
		ogg_stream_state mTheoraStreamState;
		theora_info      mTheoraInfo;
		theora_comment mTheoraComment;
		theora_state	mTheoraState;

		int          mlVideobufReady;
		ogg_int64_t  mlVideobufGranulePos;
		double       mfVideobufTime;

		bool mbVideoLoaded;
		bool mbVideoFrameReady;
		int mlBufferSize;
	};

	//-----------------------------------------

	class cVideoStreamTheora_Loader : public iVideoStreamLoader
	{
	friend class cVideoStreamTheora;
	public:
		cVideoStreamTheora_Loader();
		~cVideoStreamTheora_Loader();

		iVideoStream* Create(const tString& asName){ return hplNew( cVideoStreamTheora, (asName,this) );}

	private:
		unsigned char* mpYuvToR;
		unsigned char* mpYuvToB;

		unsigned short* mpYuv_G_UV;
		unsigned char* mpYuv_G_Y_UV;
	};

	//-----------------------------------------

};
#endif // HPL_VIDEO_STREAM_THEORA_H
