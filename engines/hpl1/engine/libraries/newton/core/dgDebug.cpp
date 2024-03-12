/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "dgStdafx.h"
#include "dgDebug.h"
#include "hpl1/debug.h"

#ifdef _DEBUG

void dgApi dgExpandTraceMessage(const char *fmt, ...) {
	va_list v_args;
	char text[4096];

	text[0] = 0;
	va_start(v_args, fmt);
	vsnprintf(text, 4096, fmt, v_args);
	va_end(v_args);
	debug("%s\n", text);

}
#endif
