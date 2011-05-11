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
 */

#include "tinsel/coroutine.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Tinsel {


CoroContext nullContext = NULL;	// FIXME: Avoid non-const global vars


#if COROUTINE_DEBUG
namespace {
static int s_coroCount = 0;

typedef Common::HashMap<Common::String, int> CoroHashMap;
static CoroHashMap *s_coroFuncs = 0;

static void changeCoroStats(const char *func, int change) {
	if (!s_coroFuncs)
		s_coroFuncs = new CoroHashMap();

	(*s_coroFuncs)[func] += change;
}

static void displayCoroStats() {
	debug("%d active coros", s_coroCount);

	// Loop over s_coroFuncs and print info about active coros
	if (!s_coroFuncs)
		return;
	for (CoroHashMap::const_iterator it = s_coroFuncs->begin();
		it != s_coroFuncs->end(); ++it) {
		if (it->_value != 0)
			debug("  %3d x %s", it->_value, it->_key.c_str());
	}
}

}
#endif

CoroBaseContext::CoroBaseContext(const char *func)
	: _line(0), _sleep(0), _subctx(0) {
#if COROUTINE_DEBUG
	_funcName = func;
	changeCoroStats(_funcName, +1);
	s_coroCount++;
#endif
}

CoroBaseContext::~CoroBaseContext() {
#if COROUTINE_DEBUG
	s_coroCount--;
	changeCoroStats(_funcName, -1);
	debug("Deleting coro in %s at %p (subctx %p)",
		_funcName, (void *)this, (void *)_subctx);
	displayCoroStats();
#endif
	delete _subctx;
}

} // End of namespace Tinsel

