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

#ifndef SCI_DETECTION_H
#define SCI_DETECTION_H

namespace Sci {

// GUI-options, primarily used by detection_tables.h
#define GAMEOPTION_PREFER_DIGITAL_SFX       GUIO_GAMEOPTIONS1
#define GAMEOPTION_ORIGINAL_SAVELOAD        GUIO_GAMEOPTIONS2
#define GAMEOPTION_MIDI_MODE                GUIO_GAMEOPTIONS3
#define GAMEOPTION_JONES_CDAUDIO            GUIO_GAMEOPTIONS4
#define GAMEOPTION_WINDOWS_CURSORS          GUIO_GAMEOPTIONS5
#define GAMEOPTION_SQ4_SILVER_CURSORS       GUIO_GAMEOPTIONS6
#define GAMEOPTION_EGA_UNDITHER             GUIO_GAMEOPTIONS7
// HIGH_RESOLUTION_GRAPHICS availability is checked for in SciEngine::run()
#define GAMEOPTION_HIGH_RESOLUTION_GRAPHICS GUIO_GAMEOPTIONS8
#define GAMEOPTION_ENABLE_BLACK_LINED_VIDEO GUIO_GAMEOPTIONS9
#define GAMEOPTION_HQ_VIDEO                 GUIO_GAMEOPTIONS10
#define GAMEOPTION_ENABLE_CENSORING         GUIO_GAMEOPTIONS11
#define GAMEOPTION_LARRYSCALE               GUIO_GAMEOPTIONS12
#define GAMEOPTION_UPSCALE_VIDEOS           GUIO_GAMEOPTIONS13
#define GAMEOPTION_RGB_RENDERING            GUIO_GAMEOPTIONS14
#define GAMEOPTION_PALETTE_MODS             GUIO_GAMEOPTIONS15
#define GAMEOPTION_SQ1_BEARDED_MUSICIANS    GUIO_GAMEOPTIONS16
#define GAMEOPTION_TTS                      GUIO_GAMEOPTIONS17
#define GAMEOPTION_ENABLE_GMM_SAVE          GUIO_GAMEOPTIONS18

enum SciGameId {
	GID_ALL,
	GID_ASTROCHICKEN,
	GID_CAMELOT,
	GID_CASTLEBRAIN,
	GID_CHEST,
	GID_CHRISTMAS1988,
	GID_CHRISTMAS1990,
	GID_CHRISTMAS1992,
	GID_CNICK_KQ,
	GID_CNICK_LAURABOW,
	GID_CNICK_LONGBOW,
	GID_CNICK_LSL,
	GID_CNICK_SQ,
	GID_ECOQUEST,
	GID_ECOQUEST2,
	GID_FAIRYTALES,
	GID_FREDDYPHARKAS,
	GID_FUNSEEKER,
	GID_GK1DEMO,	// We have a separate ID for GK1 demo, because it's actually a completely different game (SCI1.1 vs SCI2/SCI2.1)
	GID_GK1,
	GID_GK2,
	GID_HOYLE1,
	GID_HOYLE2,
	GID_HOYLE3,
	GID_HOYLE4,
	GID_HOYLE5,
	GID_ICEMAN,
	GID_INNDEMO,
	GID_ISLANDBRAIN,
	GID_JONES,
	GID_KQ1,
	GID_KQ4,
	GID_KQ5,
	GID_KQ6,
	GID_KQ7,
	GID_KQUESTIONS,
	GID_LAURABOW,
	GID_LAURABOW2,
	GID_LIGHTHOUSE,
	GID_LONGBOW,
	GID_LSL1,
	GID_LSL2,
	GID_LSL3,
	GID_LSL5,
	GID_LSL6,
	GID_LSL6HIRES, // We have a separate ID for LSL6 SCI32, because it's actually a completely different game
	GID_LSL7,
	GID_MOTHERGOOSE, // this one is the SCI0 version
	GID_MOTHERGOOSE256, // this one handles SCI1 and SCI1.1 variants, at least those 2 share a bit in common
	GID_MOTHERGOOSEHIRES, // this one is the SCI2.1 hires version, completely different from the other ones
	GID_MSASTROCHICKEN,
	GID_PEPPER,
	GID_PHANTASMAGORIA,
	GID_PHANTASMAGORIA2,
	GID_PQ1,
	GID_PQ2,
	GID_PQ3,
	GID_PQ4,
	GID_PQ4DEMO,	// We have a separate ID for PQ4 demo, because it's actually a completely different game (SCI1.1 vs SCI2/SCI2.1)
	GID_PQSWAT,
	GID_QFG1,
	GID_QFG1VGA,
	GID_QFG2,
	GID_QFG3,
	GID_QFG4,
	GID_QFG4DEMO,	// We have a separate ID for QFG4 demo, because it's actually a completely different game (SCI1.1 vs SCI2/SCI2.1)
	GID_RAMA,
	GID_REALM,
	GID_SHIVERS,
	//GID_SHIVERS2,	// Not SCI
	GID_SLATER,
	GID_SQ1,
	GID_SQ3,
	GID_SQ4,
	GID_SQ5,
	GID_SQ6,
	GID_TORIN,
	GID_FANMADE
};

/**
 * SCI versions
 * For more information, check here:
 * https://wiki.scummvm.org/index.php/Sierra_Game_Versions#SCI_Games
 */
enum SciVersion {
	SCI_VERSION_NONE,
	SCI_VERSION_0_EARLY, // KQ4 early, LSL2 early, XMAS card 1988
	SCI_VERSION_0_LATE, // KQ4, LSL2, LSL3, SQ3 etc
	SCI_VERSION_01, // KQ1 and multilingual games (S.old.*)
	SCI_VERSION_1_EGA_ONLY, // SCI 1 EGA with parser (i.e. QFG2 only)
	SCI_VERSION_1_EARLY, // KQ5 floppy, SQ4 floppy, XMAS card 1990, Fairy tales, Jones floppy
	SCI_VERSION_1_MIDDLE, // LSL1, Jones CD, LSL3 & SQ3 multilingual Amiga
	SCI_VERSION_1_LATE, // Dr. Brain 1, EcoQuest 1, Longbow, PQ3, SQ1, LSL5, KQ5 CD
	SCI_VERSION_1_1, // Dr. Brain 2, EcoQuest 1 CD, EcoQuest 2, KQ6, QFG3, SQ4CD, XMAS 1992 and many more
	SCI_VERSION_2, // GK1, PQ4 floppy, QFG4 floppy
	SCI_VERSION_2_1_EARLY, // GK2 demo, KQ7 1.4/1.51, LSL6 hires, PQ4CD, QFG4CD, SQ6 early demos
	SCI_VERSION_2_1_MIDDLE, // GK2, Hoyle 5, KQ7 2.00b, MUMG Deluxe, Phantasmagoria 1, PQ:SWAT, Shivers 1, SQ6, Torin
	SCI_VERSION_2_1_LATE, // Demos and Mac versions of LSL7, Lighthouse, RAMA
	SCI_VERSION_3 // LSL7, Lighthouse, RAMA, Phantasmagoria 2, interactive Lighthouse demos
};

/** MIDI devices */
enum kMidiMode {
	kMidiModeStandard,
	kMidiModeFB01,
	kMidiModeD110,
	kMidiModeMT540,
	kMidiModeCT460
};

} // End of namespace Sci

#endif // SCI_DETECTION_H
