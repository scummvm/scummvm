/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    Sound resource management module - public header

 Notes: 
*/

#ifndef SAGA_SNDRES_MOD_H_
#define SAGA_SNDRES_MOD_H_

namespace Saga {

int SND_Init(void);

int SND_LoadSound(ulong sound_rn);

int SND_PlayVoice(ulong voice_rn);

int SND_GetVoiceLength(ulong voice_rn);

int
SND_ITEVOC_Resample(long src_freq,
    long dst_freq,
    uchar * src_buf,
    size_t src_buf_len, uchar ** dst_buf, size_t * dst_buf_len);

} // End of namespace Saga

#endif				/* SAGA_SNDRES_MOD_H_ */
