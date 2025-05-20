/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_METAGAME_GTL_MGM_H
#define BAGEL_METAGAME_GTL_MGM_H

#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/gtl/resource.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"
#include "bagel/hodjnpodj/metagame/bgen/bgenut.h"
#include "bagel/hodjnpodj/metagame/bgen/btimeut.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

// the following are redefined at end of this header file
#undef PUBLIC
#undef PRIVATE
#undef PROTECTED
#undef PDFT
#define PUBLIC public:
#define PRIVATE private:
#define PROTECTED protected:
#define PDFT(value) = value

// DATADIR is directory for *.BMP and other data files
// #define DATADIR "..\\"
#define DATADIR ""

// the following are redefined from beginning of header file
#undef PUBLIC
#undef PRIVATE
#undef PROTECTED
#undef PDFT
#define PUBLIC
#define PRIVATE
#define PROTECTED
#define PDFT(value)

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
