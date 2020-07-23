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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCUMMVM_IPC_INCLUDE
#define SCUMMVM_IPC_INCLUDE

//////////////////////////////////////////////////////////////////////

#include <nds/ndstypes.h>
#include <nds/ipc.h>

//////////////////////////////////////////////////////////////////////


typedef struct {
	const void *data;
	u32 len;
	u32 rate;
	u8 vol;
	u8 pan;
	u8 format;
	u8 PADDING;
} TransferSoundData;




//---------------------------------------------------------------------------------
typedef struct {
	TransferSoundData data[16];
	u8 count;
	u8 PADDING[3];
} TransferSound;



typedef struct {
	u8 *buffer[8];
	bool filled[8];
	u8 *arm7Buffer[8];
	bool arm7Dirty[8];
	bool semaphore;
} adpcmBuffer;

//////////////////////////////////////////////////////////////////////

typedef struct scummvmTransferRegion {
  TransferSound *soundData;

  adpcmBuffer adpcm;

  // These are used for ScummVMs sound output
  bool fillNeeded[4];
  int playingSection;

  // Streaming sound
  bool streamFillNeeded[4];
  int streamPlayingSection;
} scummTransferRegion;

//////////////////////////////////////////////////////////////////////

#undef IPC
#define IPC ((scummTransferRegion volatile *)(0x027FF000))


#endif
