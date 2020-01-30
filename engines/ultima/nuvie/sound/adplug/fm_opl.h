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

#ifndef NUVIE_SOUND_ADPLUG_FM_OPL_H
#define NUVIE_SOUND_ADPLUG_FM_OPL_H

#include "common/scummsys.h"

namespace Ultima {
namespace Nuvie {

#define HAS_YM3812  1

/* --- select emulation chips --- */
#define BUILD_YM3812 (HAS_YM3812)
#define BUILD_YM3526 (HAS_YM3526)
#define BUILD_Y8950  (HAS_Y8950)

/* select output bits size of output : 8 or 16 */
#define OPL_SAMPLE_BITS 16

#if (OPL_SAMPLE_BITS==16)
typedef int16 OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef int8 OPLSAMPLE;
#endif


typedef void (*OPL_TIMERHANDLER)(int channel, double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param, int irq);
typedef void (*OPL_UPDATEHANDLER)(int param, int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(int param, unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(int param);


#if BUILD_YM3812

int  YM3812Init(int num, int clock, int rate);
void YM3812Shutdown(void);
void YM3812ResetChip(int which);
int  YM3812Write(int which, int a, int v);
unsigned char YM3812Read(int which, int a);
int  YM3812TimerOver(int which, int c);
void YM3812UpdateOne(int which, int16 *buffer, int length);

void YM3812SetTimerHandler(int which, OPL_TIMERHANDLER TimerHandler, int channelOffset);
void YM3812SetIRQHandler(int which, OPL_IRQHANDLER IRQHandler, int param);
void YM3812SetUpdateHandler(int which, OPL_UPDATEHANDLER UpdateHandler, int param);

#endif


#if BUILD_YM3526

/*
** Initialize YM3526 emulator(s).
**
** 'num' is the number of virtual YM3526's to allocate
** 'clock' is the chip clock in Hz
** 'rate' is sampling rate
*/
int  YM3526Init(int num, int clock, int rate);
/* shutdown the YM3526 emulators*/
void YM3526Shutdown(void);
void YM3526ResetChip(int which);
int  YM3526Write(int which, int a, int v);
unsigned char YM3526Read(int which, int a);
int  YM3526TimerOver(int which, int c);
/*
** Generate samples for one of the YM3526's
**
** 'which' is the virtual YM3526 number
** '*buffer' is the output buffer pointer
** 'length' is the number of samples that should be generated
*/
void YM3526UpdateOne(int which, int16 *buffer, int length);

void YM3526SetTimerHandler(int which, OPL_TIMERHANDLER TimerHandler, int channelOffset);
void YM3526SetIRQHandler(int which, OPL_IRQHANDLER IRQHandler, int param);
void YM3526SetUpdateHandler(int which, OPL_UPDATEHANDLER UpdateHandler, int param);

#endif

} // End of namespace Nuvie
} // End of namespace Ultima

#if BUILD_Y8950

#include "ymdeltat.h"

namespace Ultima {
namespace Nuvie {

/* Y8950 port handlers */
void Y8950SetPortHandler(int which, OPL_PORTHANDLER_W PortHandler_w, OPL_PORTHANDLER_R PortHandler_r, int param);
void Y8950SetKeyboardHandler(int which, OPL_PORTHANDLER_W KeyboardHandler_w, OPL_PORTHANDLER_R KeyboardHandler_r, int param);
void Y8950SetDeltaTMemory(int which, void *deltat_rom, int deltat_rom_size);

int  Y8950Init(int num, int clock, int rate);
void Y8950Shutdown(void);
void Y8950ResetChip(int which);
int  Y8950Write(int which, int a, int v);
unsigned char Y8950Read(int which, int a);
int  Y8950TimerOver(int which, int c);
void Y8950UpdateOne(int which, int16 *buffer, int length);

void Y8950SetTimerHandler(int which, OPL_TIMERHANDLER TimerHandler, int channelOffset);
void Y8950SetIRQHandler(int which, OPL_IRQHANDLER IRQHandler, int param);
void Y8950SetUpdateHandler(int which, OPL_UPDATEHANDLER UpdateHandler, int param);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif

#endif
