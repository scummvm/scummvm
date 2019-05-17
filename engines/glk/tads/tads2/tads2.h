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

#ifndef GLK_TADS_TADS2
#define GLK_TADS_TADS2

#include "glk/tads/tads.h"
#include "glk/tads/tads2/error_handling.h"
#include "glk/tads/tads2/appctx.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/*
 *   Run-time version number 
 */
#define  TADS_RUNTIME_VERSION   "2.5.17"

# define TRD_HEAPSIZ  4096
# define TRD_STKSIZ   200
# define TRD_UNDOSIZ  (16 * 1024)
# define TDD_HEAPSIZ  4096
# define TDD_STKSIZ   200
# define TDD_UNDOSIZ  (16 * 1024)
# define TDD_POOLSIZ  (2 * 1024)
# define TDD_LCLSIZ   0

# define ERR_TRUS_OS_FIRST    100
# define ERR_TRUS_OS_LAST      99

/**
 * TADS 2 game interpreter
 */
class TADS2 : public TADS {
private:
	// STUBS
	void os_printz(const Common::String &s) {}
	void tio_set_html_expansion(unsigned int html_char_val,
		const char *expansion, size_t expansion_len) {}
private:
	/**
	 * \defgroup trd
	 * @{
	 */

	void trdmain1(errcxdef *errctx);

	/**
	 * printf-style formatting
	 */
	void trdptf(const char *fmt, ...);

	/**@}*/
public:
	/**
	 * Constructor
	 */
	TADS2(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Execute the game
	 */
	virtual void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_TADS2; }
};

//typedef TADS2 appctxdef;

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
