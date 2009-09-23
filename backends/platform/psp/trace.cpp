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


#include "./trace.h"


void PSPDebugTrace (const char *format, ...) {
#ifdef __PSP_DEBUG__
	va_list	opt;
	char		buff[2048];
	int			bufsz, fd;

	va_start(opt, format);
	bufsz = vsnprintf( buff, (size_t) sizeof(buff), format, opt);
	va_end(opt);

	fd = sceIoOpen("MS0:/DTRACE.TXT", PSP_O_RDWR | PSP_O_CREAT | PSP_O_APPEND, 0777);

	if (fd <= 0)
		return;

	sceIoWrite(fd, (const void*)buff, bufsz);
	sceIoClose(fd);
#endif /* __PSP_DEBUG__ */
}

void PSPDebugTrace (const char * filename, const char *format, ...) {
#ifdef __PSP_DEBUG__
	va_list	opt;
	char		buff[2048];
	int			bufsz, fd;

	va_start(opt, format);
	bufsz = vsnprintf( buff, (size_t) sizeof(buff), format, opt);
	va_end(opt);

	fd = sceIoOpen(filename, PSP_O_RDWR | PSP_O_CREAT | PSP_O_APPEND, 0777);

	if (fd <= 0)
		return;

	sceIoWrite(fd, (const void*)buff, bufsz);
	sceIoClose(fd);
#endif /* __PSP_DEBUG__ */
}
