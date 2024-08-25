/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SURFACE_MANAGER
#define ICB_SURFACE_MANAGER

#include "engines/icb/p4_generic.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/common/px_types.h"

#include "graphics/surface.h"

namespace ICB {

#define SURFACE_MANAGER_LOG "surface_manager_log.txt"

// If this is passed in as an id to the Blit_surface_to_surface() the blit is performed to the backbuffer.
#define SURFACE_MANAGER_USE_BACKBUFFER 0xffffffff

#define EITHER 0x00000002 // Surface can be in vram or system
#define SYSTEM 0x00000000 // Surface is in system
#define VIDEO 0x00000001  // Surface must be in vram

extern uint32 effect_time; // Time spent doing postprocessing effects (fades etc)
extern uint32 working_buffer_id;
extern uint32 bg_buffer_id;

// Define this here so we can get rid of the <ddraw.h> include
#define DDBLT_KEYSRC 0x00008000l

class _surface {
public:
	Graphics::Surface *m_dds;
	pxString m_name; // The surface name
	uint32 m_width;  // The surface width
	uint32 m_height; // The surface height
	bool8 m_locked;  // Is the surface locked ?
	uint32 m_colorKey;

	_surface();
	~_surface();
};

class _surface_manager {

private:
	Graphics::Surface *screenSurface;
	LRECT m_screen_rect; // The screen rectangle
public:
	rcAutoPtrArray<_surface> m_Surfaces; // List of client surface
private:
	bool8 m_hasMMX; // True if the processor has MMX capabilities

public:
	_surface_manager();
	~_surface_manager();

	uint32 Init_direct_draw();
	void Flip();
	void Reset_Effects();

	/* Public Surface Functions */
	uint32 Create_new_surface(const char *name, uint32 width, uint32 height, uint32 type);
	void Kill_surface(uint32 s_id);

	uint8 *Lock_surface(uint32 s_id);
	void Unlock_surface(uint32 s_id);

	void PrintDebugLabel(const char *mess, uint32 c);
	void PrintTimer(char label, uint32 time, uint32 limit);

	int32 Get_pitch(uint32 s_id);
	inline uint32 Get_width(uint32 nSurfaceID) const;
	inline uint32 Get_height(uint32 nSurfaceID) const;
	inline uint32 Get_BytesPP(uint32 nSurfaceID) const;
	void Clear_surface(uint32 s_id) { Fill_surface(s_id, 0x00000000); }
	void Fill_surface(uint32 s_id, uint32 rgb_value);
	void Blit_fillfx(uint32 surface_id, LRECT *rect, uint32 col);
	void Blit_surface_to_surface(uint32 from_id, uint32 to_id, LRECT *rect_from, LRECT *rect_to, uint32 nFlags = 0);
	void Set_transparent_colour_key(uint32 nSurfaceID, uint32 nKey);
	bool8 &HasMMX() { return m_hasMMX; }
	void RecordFrame(const char *path);
	void Unlock_all_surfaces();

public:
	void DrawEffects(uint32 surface_id);

	LRECT &BorderRect() {
		return m_borders; // Access to the border rect
	}
	uint8 &BorderRed() {
		return m_borderRed; // Access to the border colour
	}
	uint8 &BorderGreen() {
		return m_borderGreen; // Access to the border colour
	}
	uint8 &BorderBlue() {
		return m_borderBlue; // Access to the border colour
	}
	uint8 &BorderAlpha() {
		return m_borderAlpha; // Access to the border colour
	}
	uint32 &BorderMode() { return m_borderMode; }

	void LoadGFXInfo(Common::SeekableReadStream *stream);
	void SaveGFXInfo(Common::WriteStream *stream);

private:
	LRECT m_borders;     // The border to add to the screen
	uint8 m_borderRed;   // The colour of the border
	uint8 m_borderGreen; // The colour of the border
	uint8 m_borderBlue;  // The colour of the border
	uint8 m_borderAlpha; // The colour of the border
	uint32 m_borderMode;

public: /* FADE */
	uint32 &FadeMode() {
		return m_fadeMode; // Access to the fade mode
	}
	uint8 &FadeToRed() {
		return m_fadeToRed; // Access to the red fade component
	}
	uint8 &FadeToGreen() {
		return m_fadeToGreen; // Access to the red fade component
	}
	uint8 &FadeToBlue() {
		return m_fadeToBlue; // Access to the red fade component
	}
	uint8 &FadeFromRed() {
		return m_fadeFromRed; // Access to the red fade component
	}
	uint8 &FadeFromGreen() {
		return m_fadeFromGreen; // Access to the red fade component
	}
	uint8 &FadeFromBlue() {
		return m_fadeFromBlue; // Access to the red fade component
	}
	uint8 &FadeAlpha() {
		return m_fadeAlpha; // Access to the red fade component
	}

private:
	uint32 m_fadeMode;     // The type of fade (additive / subtractive / alpha )
	uint8 m_fadeToRed;     // The red component to fade to
	uint8 m_fadeToGreen;   // The green component to fade to
	uint8 m_fadeToBlue;    // The blue component to fade to
	uint8 m_fadeFromRed;   // The red component to fade from
	uint8 m_fadeFromGreen; // The green component to fade from
	uint8 m_fadeFromBlue;  // The blue component to fade from
	uint8 m_fadeAlpha;     // The alpha component of the fade

private: /* Private Helper Functions */
	/* Let the MouseDraw function get at the back buffer */
	friend int32 DrawMouse();

private:
	_surface_manager(const _surface_manager &) { ; }
	void operator=(const _surface_manager &) { ; }
};

inline uint32 _surface_manager::Get_width(uint32 nSurfaceID) const {
	assert(nSurfaceID < m_Surfaces.GetNoItems());

	return (m_Surfaces[nSurfaceID]->m_width);
}

inline uint32 _surface_manager::Get_height(uint32 nSurfaceID) const {
	assert(nSurfaceID < m_Surfaces.GetNoItems());

	return (m_Surfaces[nSurfaceID]->m_height);
}

inline uint32 _surface_manager::Get_BytesPP(uint32 nSurfaceID) const {
	assert(nSurfaceID < m_Surfaces.GetNoItems());

	return ((m_Surfaces[nSurfaceID]->m_dds->format.bytesPerPixel));
}

} // End of namespace ICB

#endif
