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

namespace Sci {

class Console;
struct EngineState;
class Kernel;
class Vocabulary;
class ResourceManager;

// our engine debug levels
enum kDebugLevels {
	kDebugLevelError      = 1 << 0,
	kDebugLevelNodes      = 1 << 1,
	kDebugLevelGraphics   = 1 << 2,
	kDebugLevelStrings    = 1 << 3,
	kDebugLevelMemory     = 1 << 4,
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
	kDebugLevelDclInflate = 1 << 17,
	kDebugLevelVM         = 1 << 18,
	kDebugLevelScripts    = 1 << 19,
	kDebugLevelGC         = 1 << 20,
	kDebugLevelSci0Pic    = 1 << 21
};

struct SciGameDescription {
	ADGameDescription desc;
	uint32 flags;
};

extern const char *versionNames[];

enum SciGameFlags {
	// SCI0 flags

	/* Applies to all versions before 0.000.629
	 * Older SCI versions had simpler code for GetTime()
	 */
	GF_SCI0_OLDGETTIME		= (1 << 0)
};

/** SCI versions */
enum SciVersion {
	SCI_VERSION_AUTODETECT,
	SCI_VERSION_0_EARLY, // Early KQ4, 1988 xmas card
	SCI_VERSION_0_LATE, // KQ4, LSL2, LSL3, SQ3 etc
	SCI_VERSION_01, // KQ1 and multilingual games (S.old.*)
	SCI_VERSION_1_EGA, // EGA with parser, QFG2
	SCI_VERSION_1_EARLY, // KQ5. (EGA/VGA)
	SCI_VERSION_1_MIDDLE, // LSL1, JONESCD. (EGA?/VGA)
	SCI_VERSION_1_LATE, // ECO1, LSL5. (EGA/VGA)
	SCI_VERSION_1_1, // KQ6, ECO2
	SCI_VERSION_2, // GK1, PQ4 (Floppy), QFG4 (Floppy)
	SCI_VERSION_2_1, // GK2, KQ7, SQ6, Torin
	SCI_VERSION_3 // LSL7, RAMA, Lighthouse
};

class SciEngine : public Engine {
	friend class Console;
public:
	SciEngine(OSystem *syst, const SciGameDescription *desc);
	~SciEngine();

	// Engine APIs
	virtual Common::Error run();
	void pauseEngineIntern(bool pause);
	virtual GUI::Debugger *getDebugger();
	Console *getSciDebugger();

	const char* getGameID() const;
	int getResourceVersion() const;
	SciVersion getVersion() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint32 getFlags() const;
	ResourceManager *getResourceManager() const { return _resMan; }
	Kernel *getKernel() const { return _kernel; }
	Vocabulary *getVocabulary() const { return _vocabulary; }

	Common::String getSavegameName(int nr) const;
	Common::String getSavegamePattern() const;

	/** Prepend 'TARGET-' to the given filename. */
	Common::String wrapFilename(const Common::String &name) const;

	/** Remove the 'TARGET-' prefix of the given filename, if present. */
	Common::String unwrapFilename(const Common::String &name) const;

	Common::String getSciVersionDesc(SciVersion version) const;

private:
	const SciGameDescription *_gameDescription;
	ResourceManager *_resMan;
	EngineState *_gamestate;
	Kernel *_kernel;
	Vocabulary *_vocabulary;
	Console *_console;
};

/**
 * Convenience function to obtain the active SCI version.
 */
inline static SciVersion getSciVersion() {
	return ((SciEngine*)g_engine)->getVersion();
}


} // End of namespace Sci

#endif // SCI_H
