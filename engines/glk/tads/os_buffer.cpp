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

#include "glk/tads/os_buffer.h"
#include "glk/tads/os_parse.h"
#include "glk/tads/tads.h"

namespace Glk {
namespace TADS {

extern winid_t mainwin;

#ifndef GLK_MODULE_UNICODE

void os_put_buffer(const char *buf, size_t len) {
    g_vm->glk_put_buffer(buf, len);
}

void os_get_buffer (char *buf, size_t len, size_t init)
{
	g_vm->glk_request_line_event(mainwin, buf, len - 1, init);
}

char *os_fill_buffer(char *buf, size_t len)
{
    buf[len] = '\0';
    return buf;
}

#else

static uint32 *input = 0;
static uint max = 0;

void os_put_buffer(const char *buf, size_t len) {
    uint32 *out;
    uint outlen;

    if (!len)
        return;

    out = new uint32[len + 1];
    if (!out)
        return;
    outlen = len;

    outlen = os_parse_chars((const unsigned char *)buf, len, out, outlen);

    if (outlen)
        g_vm->glk_put_buffer_uni(out, outlen);
    else
		g_vm->glk_put_buffer(buf, len);

	delete[] out;
}

void os_get_buffer(char *buf, size_t len, size_t init) {
    input = (uint32 *)malloc(sizeof(uint) * (len + 1));
    max = len;

    if (init)
        os_parse_chars((const unsigned char *)buf, init + 1, input, len);

    g_vm->glk_request_line_event_uni(mainwin, input, len - 1, init);
}

char *os_fill_buffer(char *buf, size_t len) {
    uint res = os_prepare_chars(input, len, (unsigned char *)buf, max);
    buf[res] = '\0';

    free(input);
    input = nullptr;
    max = 0;

    return buf;
}

#endif /* GLK_MODULE_UNICODE */

} // End of namespace TADS
} // End of namespace Glk
