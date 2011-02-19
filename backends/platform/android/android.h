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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#if defined(__ANDROID__)

#include <android/log.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

// toggles start
//#define ANDROID_DEBUG_ENTER
//#define ANDROID_DEBUG_GL
// toggles end

extern const char *android_log_tag;

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, android_log_tag, fmt, ## args)
#define LOGD(fmt, args...) _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define LOGI(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define LOGW(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define LOGE(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

#ifdef ANDROID_DEBUG_ENTER
#define ENTER(fmt, args...) LOGD("%s(" fmt ")", __FUNCTION__, ##args)
#else
#define ENTER(fmt, args...) /**/
#endif

#ifdef ANDROID_DEBUG_GL
extern void checkGlError(const char *expr, const char *file, int line);

#define GLCALL(x) \
	do { \
		(x); \
		checkGlError(#x, __FILE__, __LINE__); \
	} while (false)

#else
#define GLCALL(x) do { (x); } while (false)
#endif

// Fix JNIEXPORT declaration to actually do something useful
#undef JNIEXPORT
#define JNIEXPORT __attribute__ ((visibility("default")))

#endif

