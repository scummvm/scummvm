/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_GLOBAL_OBJECTS_PSX_H
#define ICB_GLOBAL_OBJECTS_PSX_H

#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/surface_manager.h"
#include "engines/icb/gfx/psx_light.h"
#include "engines/icb/gfx/psx_tman.h"

namespace ICB {

#define DECOMPRESSION_BUFFER_SIZE (64 * 64)
// A piece of memory for decompressing sprites into
// extern uint8 decompBuffer[DECOMPRESSION_BUFFER_SIZE];
extern uint8 *decompBuffer;

// A piece of memory for pre-building the save game onto !
#define MEM_BUF_SIZE (8 * 1024)
// extern uint8 eightkb[MEM_BUF_SIZE];
extern uint8 *eightkb;

// For reusing characters already loaded into VRAM
// -32 because characters start from ' '
#define N_FONT_CHARS (256 - ' ')
extern uint16 cPos[N_FONT_CHARS];
extern uint8 cPWidth[N_FONT_CHARS];
extern uint8 cHeight[N_FONT_CHARS];
extern uint8 cWidth[N_FONT_CHARS];
extern uint16 fontClutLoaded;
extern uint16 fontFreePos;
extern uint8 fontNcolours;

// Global instances of common strings, as GCC seems to make
// multiple copies wasting all of the rdata section !

extern const char *global_null_path;
extern const char *global_sys_font;
extern const char *global_set_path;
extern const char *global_session_test_path;
extern const char *global_session_cluster_path;
extern const char *global_snddata_test_path;
extern const char *global_session_xa_path;
extern const char *global_global_xa_path;
extern const char *global_chr_path;
extern const char *global_fir_cluster_path;
extern const char *global_image_cluster_path;
extern const char *global_cluster_path;
extern const char *global_var_path;
extern const char *gamescript_path;
extern const char *global_votext_name;
extern const char *global_icon_labels_filename;

extern const char *global_cover_page;

// speech
extern const char *global_speech_cluster;
extern uint32 global_speech_cluster_hash;

extern const char *session_speech_cluster_path;

extern const char *global_music_cluster;
extern uint32 global_music_cluster_hash;

// The texture manager for the actor textures
extern TextureManager *tman;

// This does nothing but makes PC & PSX code more compatible the surface manager
extern _surface_manager *surface_manager;

// The 3 directional lights and the ambient light
extern psxLight Lights[4];

// Colour of ambient to add when targetting a mega and you are armed
extern int32 mega_select_armed_r;
extern int32 mega_select_armed_g;
extern int32 mega_select_armed_b;

// Colour of ambient to add when targetting a mega and you are unarmed
extern int32 mega_select_unarmed_r;
extern int32 mega_select_unarmed_g;
extern int32 mega_select_unarmed_b;

// Colour of the actors bounding box
extern int32 bboxRed;
extern int32 bboxGreen;
extern int32 bboxBlue;

// Colour of the actors screen outline
extern int32 slineRed;
extern int32 slineGreen;
extern int32 slineBlue;

// Colour of the z-fragments outlines
extern int32 zfragRed;
extern int32 zfragGreen;
extern int32 zfragBlue;

// Colour of the z-layers outlines
extern int32 zoverRed;
extern int32 zoverGreen;
extern int32 zoverBlue;

// Colour of the wire-frame man
extern int32 wfrmRed;
extern int32 wfrmGreen;
extern int32 wfrmBlue;

// Drawing options
extern int32 _normalScale;
extern int32 _useWindingOrder;
extern int32 _removeBackface;
extern int32 _drawWfrm;
extern int32 _drawTxture;
extern int32 _drawLit;
extern int32 _drawGouraud;
extern int32 _drawPolys;
extern int32 _drawNormals;

extern int32 _drawActor;
extern int32 _drawBbox;
extern int32 _drawShadowBbox;
extern int32 _drawSolidBbox;
extern int32 _drawSline;
extern int32 _drawShadowSline;
extern int32 _drawZfragEdge;
extern int32 _drawZLayerEdge;
extern int32 _drawZfragImg;
extern int32 _drawBG;
extern int32 _drawProps;
extern int32 _drawSemiTrans;

extern int32 _drawText;
extern int32 _drawCompass;

extern int32 _drawDebugMesh;

// For choosing different fonts
#define MAX_FONTS (4)
extern const char *fonts[MAX_FONTS];
extern int32 selFont;

#define MAX_MOVIES (64)
extern int32 selMovie;
extern int32 selSession;
extern int32 selGame;

// For choosing actor names for voxel_viewer
#define MAX_ACTORS 10
extern int32 selActor;
extern char *actor_names[MAX_ACTORS];

// For choosing actor outfits for voxel_viewer
#define MAX_OUTFITS 8
extern int32 selOutfit;
extern char *actor_outfits[MAX_OUTFITS];

// For choosing actor weapons for voxel_viewer
#define MAX_WEAPONS 3
extern int32 selWeapon;
extern char *actor_weapons[MAX_WEAPONS];

// For choosing actor anims : these come from the master list in
// animation_mega_set.{cpp,h}
extern int32 selAnim;

} // End of namespace ICB

#endif // #ifndef   _GLOBAL_OBJECTS_PSX_H
