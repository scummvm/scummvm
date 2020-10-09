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

#include "engines/icb/global_objects_psx.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/common/px_clu_api.h"

namespace ICB {

// For reusing characters already loaded into VRAM
// -32 because characters start from ' '
u_short cPos[N_FONT_CHARS];
uint8 cPWidth[N_FONT_CHARS];
uint8 cHeight[N_FONT_CHARS];
uint8 cWidth[N_FONT_CHARS];
u_short fontClutLoaded = 0;
u_short fontFreePos = 0;
uint8 fontNcolours = 0;

// Global instances of common strings, as GCC seems to make
// multiple copies wasting all of the rdata section !

const char *global_null_path = "";

// The different fonts we have available
const char *fonts[MAX_FONTS] = {"font", "city", "futura", "futura_outline"};
int selFont = 2;

const char *global_sys_font = fonts[selFont];
const char *global_set_path = "\\M\\%s.CAM;1";
const char *global_session_cluster_path = "\\M\\%s.SEX;1";
const char *global_session_test_path = "\\M\\%s\\%s.SEX;1";
const char *global_snddata_test_path = "\\M\\%s.SND;1";
const char *global_session_xa_path = "\\M\\%s.XA\\XA%04d.XA;1";
const char *global_global_xa_path = "\\G\\XA\\XA%04d.XA;1";
const char *global_chr_path = "\\C\\%s\\%s.OFT;1";
const char *global_fir_cluster_path = "\\G\\FIR.CLU;1";
const char *global_image_cluster_path = "\\G\\IMAGES.CLU;1";
const char *global_cluster_path = "\\G\\HAVENOTA.CLU;1";
const char *global_var_path = "globals";
const char *gamescript_path = "gamescript";
const char *global_votext_name = "remora.ctf";
const char *global_icon_labels_filename = "iconlabels.ctf";

const char *global_cover_page = "cover_page.bs";

const char *global_speech_cluster = "\\G\\SPEECH.CLU;1";
uint32 global_speech_cluster_hash = NULL_HASH;

const char *global_music_cluster = "\\G\\MUSIC.CLU;1";
uint32 global_music_cluster_hash = NULL_HASH;

const char *session_speech_cluster_path = "\\M\\%s.SPK;1";

// The texture manager for the actor textures
TextureManager *tman;

// This does nothing but makes PC & PSX code more compatible the surface manager

// The 3 directional lights and the ambient light
psxLight Lights[4];

// Colour of ambient to add when targetting a mega and you are armed
int mega_select_armed_r = 60;
int mega_select_armed_g = 10;
int mega_select_armed_b = 10;

// Colour of ambient to add when targetting a mega and you are unarmed
int mega_select_unarmed_r = 50;
int mega_select_unarmed_g = 50;
int mega_select_unarmed_b = 50;

#if CD_BUILD == 0
// Colour of the z-fragments outlines
int zfragRed = 255;
int zfragGreen = 0;
int zfragBlue = 0;

// Colour of the z-layers outlines
int zoverRed = 255;
int zoverGreen = 0;
int zoverBlue = 255;
#endif // #if CD_BUILD == 0

// Colour of the actors bounding box
int bboxRed = 0;
int bboxGreen = 255;
int bboxBlue = 0;

// Colour of the actors screen outline
int slineRed = 0;
int slineGreen = 0;
int slineBlue = 255;

// Colour of the wire-frame man
int wfrmRed = 100;
int wfrmGreen = 50;
int wfrmBlue = 20;

// Drawing options
int _normalScale = 8;
int _useWindingOrder = 1;
int _removeBackface = 1;
int _drawWfrm = 0;
int _drawTxture = 1;
int _drawLit = 1;
int _drawGouraud = 1;
int _drawPolys = 1;
int _drawNormals = 0;

int _drawActor = 1;
int _drawSline = 0;
int _drawShadowSline = 0;
int _drawBbox = 0;
int _drawShadowBbox = 0;
int _drawSolidBbox = 0;
int _drawZfragEdge = 0;
int _drawZLayerEdge = 0;
int _drawZfragImg = 1;
int _drawText = 0;
int _drawCompass = 0;
int _drawBG = 1;
int _drawProps = 1;
int _drawSemiTrans = 1;

int _drawDebugMesh = 0;

} // End of namespace ICB
