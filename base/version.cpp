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

#include "common/scummsys.h"
#include "base/version.h"

#define INCLUDED_FROM_BASE_VERSION_CPP
#include "base/internal_version.h"

/*
 * Version string and build date string. These can be used by anything that
 * wants to display this information to the user (e.g. about dialog).
 *
 * Note: it would be very nice if we could instead of (or in addition to) the
 * build date present a date which corresponds to the date our source files
 * were last changed. To understand the difference, imagine that a user
 * makes a checkout on January 1, then after a week compiles it
 * (e.g. after doing a 'make clean'). The build date then will say January 8
 * even though the files were last changed on January 1.
 *
 * Another problem is that __DATE__/__TIME__ depend on the local time zone.
 *
 * It's clear that such a "last changed" date would be much more useful to us
 * for feedback purposes. After all, when somebody files a bug report, we
 * don't care about the build date, we want to know which date their checkout
 * was made.
 *
 * So, how could we implement this? At least on unix systems, a special script
 * could do it. Basically, that script could parse the output of "svn info" or
 * "svnversion" to determine the revision of the checkout, and insert that
 * information somehow into the build process (e.g. by generating a tiny
 * header file, analog to internal_version.h, maybe called svn_rev.h or so.)
 *
 * Drawback: This only works on systems which can run suitable scripts as part
 * of the build process (so I guess Visual C++ would be out of the game here?
 * I don't know VC enough to be sure). And of course it must be robust enough
 * to properly work in exports (i.e. release tar balls etc.).
 */
const char gScummVMVersion[] = SCUMMVM_VERSION SCUMMVM_REVISION;
#if defined(__amigaos4__) || defined(__MORPHOS__)
static const char *version_cookie __attribute__((used)) = "$VER: ScummVM " SCUMMVM_VERSION SCUMMVM_REVISION " (" AMIGA_DATE ")";
#endif
const char gScummVMBuildDate[] = __DATE__ " " __TIME__;
const char gScummVMVersionDate[] = SCUMMVM_VERSION SCUMMVM_REVISION " (" __DATE__ " " __TIME__ ")";
const char gScummVMCompiler[] = ""
#define STR_HELPER(x)	#x
#define STR(x)		STR_HELPER(x)
#if defined(_MSC_VER)
	"MSVC " STR(_MSC_FULL_VER)
#elif defined(__INTEL_COMPILER)
	"ICC " STR(__INTEL_COMPILER) "." STR(__INTEL_COMPILER_UPDATE)
#elif defined(__clang__)
	"Clang " STR(__clang_major__) "." STR(__clang_minor__) "." STR(__clang_patchlevel__)
#elif defined(__GNUC__)
	"GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#else
	"unknown compiler"
#endif
#undef STR
#undef STR_HELPER
	;
const char gScummVMFullVersion[] = "ScummVM " SCUMMVM_VERSION SCUMMVM_REVISION " (" __DATE__ " " __TIME__ ")";
const char gScummVMFeatures[] = ""
#ifdef TAINTED_BUILD
	// TAINTED means the build contains engines/subengines not enabled by default
	"TAINTED "
#endif

#ifdef USE_TREMOR
#  ifdef USE_TREMOLO
	// libTremolo is used on WinCE for better ogg performance
	"Tremolo "
#  else
	"Tremor "
#  endif
#elif defined(USE_VORBIS)
	"Vorbis "
#endif

#ifdef USE_FLAC
	"FLAC "
#endif

#ifdef USE_MAD
	"MP3 "
#endif

#ifdef USE_ALSA
	"ALSA "
#endif

#ifdef USE_SEQ_MIDI
	"SEQ "
#endif

#ifdef USE_SNDIO
	"sndio "
#endif

#ifdef USE_TIMIDITY
	"TiMidity "
#endif

#ifdef USE_RGB_COLOR
	"RGB "
#endif

#ifdef USE_ZLIB
	"zLib "
#endif

#ifdef USE_MPEG2
	"MPEG2 "
#endif

#ifdef USE_FLUIDLITE
	"FluidLite "
#elif defined(USE_FLUIDSYNTH)
	"FluidSynth "
#endif

#ifdef USE_MIKMOD
	"MikMod "
#endif

#ifdef USE_THEORADEC
	"Theora "
#endif

#ifdef USE_VPX
	"VPX "
#endif

#ifdef USE_FAAD
	"AAC "
#endif

#ifdef USE_A52
	"A/52 "
#endif

#ifdef USE_FREETYPE2
	"FreeType2 "
#endif

#ifdef USE_FRIBIDI
	"FriBiDi "
#endif

#ifdef USE_JPEG
	"JPEG "
#endif

#ifdef USE_PNG
	"PNG "
#endif

#ifdef USE_GIF
	"GIF "
#endif

#ifdef ENABLE_VKEYBD
	"VirtualKeyboard "
#endif

#ifdef ENABLE_EVENTRECORDER
	"EventRecorder "
#endif

#ifdef USE_TASKBAR
	"taskbar "
#endif

#ifdef USE_TTS
	"TTS "
#endif

#ifdef USE_CLOUD
	"cloud ("
#  ifdef USE_LIBCURL
	"servers"
#    ifdef USE_SDL_NET
	", local) "
#    else
	") "
#    endif
#  endif
#else
#  ifdef USE_LIBCURL
	"libcurl "
#  endif
#  ifdef USE_SDL_NET
	"SDL_net "
#  endif
#endif

#ifdef USE_ENET
	"ENet "
#endif

#ifdef SDL_BACKEND
#  ifdef USE_SDL2
	"SDL2 "
#  else
	"SDL1.2 "
#  endif
#endif

#ifdef USE_TINYGL
	"TinyGL "
#endif

#ifdef USE_OPENGL
	"OpenGL "
#  ifdef USE_OPENGL_SHADERS
	"(with shaders) "
#  endif
#endif

#ifdef USE_GLES_MODE
#  if USE_GLES_MODE == 0
	"OpenGL desktop only "
#  elif USE_GLES_MODE == 1
	"OpenGL ES 1 only "
#  elif USE_GLES_MODE == 2
	"OpenGL ES 2 only "
#  endif
#endif

#ifdef USE_RETROWAVE
	"RetroWave "
#endif
	;
