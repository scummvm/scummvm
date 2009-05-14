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

#ifndef SCI_H
#define SCI_H

#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "sci/scicore/resource.h"

namespace Sci {

class Console;

// our engine debug levels
enum kDebugLevels {
	kDebugLevelError      = 1 << 0,
	kDebugLevelNodes      = 1 << 1,
	kDebugLevelGraphics   = 1 << 2,
	kDebugLevelStrings    = 1 << 3,
	kDebugLevelMem        = 1 << 4,
	kDebugLevelFuncCheck  = 1 << 5,
	kDebugLevelBresen     = 1 << 6,
	kDebugLevelSound      = 1 << 7,
	kDebugLevelGfxDriver  = 1 << 8,
	kDebugLevelBaseSetter = 1 << 9,
	kDebugLevelParser     = 1 << 10,
	kDebugLevelMenu       = 1 << 11,
	kDebugLevelSaid       = 1 << 12,
	kDebugLevelFile       = 1 << 13,
	kDebugLevelTime       = 1 << 14,
	kDebugLevelRoom       = 1 << 15,
	kDebugLevelAvoidPath  = 1 << 16,
	kDebugLevelDclInflate = 1 << 17
};

struct SciGameDescription {
	ADGameDescription desc;
	uint32 flags;
	int res_version;
	int version;
};

enum SciGameVersions {
	SCI_VERSION_AUTODETECT = 0,
	SCI_VERSION_0 = 1,
	SCI_VERSION_01 = 2,
	SCI_VERSION_01_VGA = 3,
	SCI_VERSION_01_VGA_ODD = 4,
	SCI_VERSION_1_EARLY = 5,
	SCI_VERSION_1_LATE = 6,
	SCI_VERSION_1_1 = 7,
	SCI_VERSION_32 = 8
};

enum SciGameFlags {
	/*
	** SCI0 flags
	*/

	/* Applies to all versions before 0.000.395
	** Old SCI versions used two word header for script blocks (first word equal
	** to 0x82, meaning of the second one unknown). New SCI versions used one
	** word header.
	*/
	GF_OLDSCRIPTHEADER	= (1 << 0),

	/* Applies to all versions before 0.000.395
	** Earlier versions assign 120 degrees to left & right , and 60 to up and down.
	** Later versions use an even 90 degree distribution.
	*/
	GF_OLDANGLES		= (1 << 1),

	/* Applies to all versions before 0.000.490 (PQ2-new)
	** When a new song is initialized, we store its state and
    ** resume it when the new one finishes.  Older versions completely
    ** clobbered the old songs.
	*/
	GF_OLDRESUMESONG	= (1 << 2),

	/* Applies to all versions before 0.000.502
	** Old SCI versions used to interpret the third DrawPic() parameter inversely,
	** with the opposite default value (obviously).
	** Also, they used 15 priority zones from 42 to 200 instead of 14 priority
	** zones from 42 to 190.
	*/
	GF_OLDGFXFUNCTIONS	= (1 << 3),

	/* Applies to all versions before 0.000.629
	** Older SCI versions had simpler code for GetTime()
	*/
	GF_OLDGETTIME		= (1 << 4),

	// ----------------------------------------------------------------------------

	/*
	** SCI1 flags
	*/
	
	/* Applies to all versions from 1.000.200 onwards
    ** In later SCI1 versions, the argument of lofs[as]
	** instructions is absolute rather than relative.
	*/
	GF_LOFSABSOLUTE		= (1 << 5),

	/* Applies to all versions from 1.000.510 onwards
	** Also in kDisplay(), if the text would not fit on the screen, it
    ** is moved to the left and upwards until it fits.
	** Finally, kDoSound() is different than in earlier SCI1 versions.
	*/
	GF_LATESCI1			= (1 << 6)
};

class SciEngine : public Engine {
public:
	SciEngine(OSystem *syst, const SciGameDescription *desc);
	~SciEngine();

	// Engine APIs
	virtual Common::Error run();
	virtual GUI::Debugger *getDebugger();

	const char* getGameID() const;
	int getResourceVersion() const;
	int getVersion() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint32 getFlags() const;
	ResourceManager *getResMgr() { return _resmgr; }

	Common::String getSavegameName(int nr) const;
	Common::String getSavegamePattern() const;

	/** Prepend 'TARGET-' to the given filename. */
	Common::String wrapFilename(const Common::String &name) const;

	/** Remove the 'TARGET-' prefix of the given filename, if present. */
	Common::String unwrapFilename(const Common::String &name) const;

	Console *_console;

private:
	const SciGameDescription *_gameDescription;
	ResourceManager *_resmgr;
};

} // End of namespace Sci

#endif // SCI_H
