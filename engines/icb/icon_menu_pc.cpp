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

#include "engines/icb/icon_menu.h"
#include "engines/icb/icon_menu_pc.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/res_man.h"
#include "engines/icb/mission.h"
#include "engines/icb/remora.h"

namespace ICB {

// Armed menu positioning  -- I've put these here as they are not need outside this file,
// and I don't like the rebuild over head when the are in the .h file
#define ICON_ARMED_MENU_PIXEL_X (ICON_MENU_PIXEL_X)
#define ICON_ARMED_MENU_PIXEL_Y (10)

LRECT ICON_BITMAP_RECT = {0, 0, ICON_X_SIZE - 1, ICON_Y_SIZE - 1};
LRECT ICON_ADDING_RECT = {ICON_ADDING_X, ICON_ADDING_Y, ICON_ADDING_X + ICON_X_SIZE - 1, ICON_ADDING_Y + ICON_Y_SIZE - 1};

void _icon_menu::Activate(const _icon_list *pIconList, const _icon_menu_duplicates &sDuplicates, bool8 bAllowEscape, uint32 nSelected) {
	uint32 i;
	uint8 *pyIconBitmap;
	uint8 *pyHiLiteBitmap;
	uint32 nIconCount;
	char pcIconName[MAXLEN_ICON_NAME];
	char pcFullIconName[MAXLEN_URL];
	char pcIconPath[MAXLEN_URL];
	uint32 nFullIconNameHash;
	_pxBitmap *psIconBitmap;

	Zdebug("Entered _icon_menu::Activate()");

	PXTRY

	// If we are not in the remora, then we want to scroll the icons onto the screen
	if (g_oRemora->IsActive())
		m_nMenuY = ICON_MENU_PIXEL_Y;
	else
		m_nMenuY = 490;

	// This allows us to ignore extra key presses from instantly closing the menu again.
	m_nKeyLock = TRUE8;

	// Tell the game that the menu is now active.
	m_eIconMenuGameState = ACTIVE;

	// Set some parameters to do with the icons.
	m_nSelectedIcon = nSelected;
	m_bValidSelection = FALSE8;
	m_pIconList = pIconList;
	nIconCount = pIconList->GetIconCount();
	m_sDuplicates = sDuplicates;
	m_bAllowEscape = bAllowEscape;
	m_nLastIconIndex = (uint8)nSelected;
	m_nScrollCycles = 0;
	m_nScrollDirection = ICON_MENU_SCROLL_NONE;

	// The maximum number of icons that can be displayed is reduced by one if there is currently an
	// email waiting because we need room to display the symbol for that.
	m_nMaxIconsDisplayed = (m_bEmailArrived) ? (uint8)(ICON_LIST_MAX_DISPLAYED - 1) : (uint8)ICON_LIST_MAX_DISPLAYED;

	// Check if there are too many icons to display.
	if (nIconCount > m_nMaxIconsDisplayed) {
		// Set this flag so the drawing code knows about it.
		m_bWiderThanScreen = TRUE8;

		// Prepare the off-screen arrows for drawing.
		SetUpOffScreenArrows();
	} else {
		// Icons fit on screen.
		m_bWiderThanScreen = FALSE8;
	}

	// Loop for each icon to be drawn.
	for (i = 0; i < nIconCount; ++i) {
		// Get the full pathname for the icon.
		strcpy(pcIconName, m_pIconList->GetIcon(i));
		sprintf(pcIconPath, ICON_PATH);
		sprintf(pcFullIconName, "%s%s.%s", pcIconPath, pcIconName, PX_BITMAP_PC_EXT);

		// Open the icon resource.
		nFullIconNameHash = NULL_HASH;
		psIconBitmap = (_pxBitmap *)rs_icons->Res_open(pcFullIconName, nFullIconNameHash, m_pcIconCluster, m_nIconClusterHash);

		// Check the schema is correct.
		if (psIconBitmap->schema != PC_BITMAP_SCHEMA)
			Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", pcFullIconName, PC_BITMAP_SCHEMA, psIconBitmap->schema);

		// Create a surface for the icon
		m_pnIconSurfaceIDs[i] = surface_manager->Create_new_surface(pcIconName, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
		surface_manager->Set_transparent_colour_key(m_pnIconSurfaceIDs[i], m_nTransparentKey);
		pyIconBitmap = surface_manager->Lock_surface(m_pnIconSurfaceIDs[i]);
		uint32 nPitch = surface_manager->Get_pitch(m_pnIconSurfaceIDs[i]);
		// Load the icon into the surface
		SpriteXYFrameDraw(pyIconBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 0, FALSE8, NULL, 255);
		// convert it to b/w
		uint32 *icon_ad = (uint32 *)pyIconBitmap;
		for (uint32 y = 0; y < ICON_Y_SIZE; y++) {
			uint32 *rowAd = icon_ad;
			for (int x = 0; x < ICON_X_SIZE; x++) {
				uint32 col = *rowAd;
				if (col != m_nTransparentKey) {
					(*((uint8 *)rowAd + 0)) >>= 1;
					(*((uint8 *)rowAd + 1)) >>= 1;
					(*((uint8 *)rowAd + 2)) >>= 1;
					(*((uint8 *)rowAd + 3)) >>= 1;
				}
				rowAd++;
			}
			icon_ad += nPitch / 4;
		}
		// Unlock the surface
		surface_manager->Unlock_surface(m_pnIconSurfaceIDs[i]);

		// Create a surface for the icons hilite
		sprintf(pcIconName, "%sH", pcIconName);
		m_pnHiLiteSurfaceIDs[i] = surface_manager->Create_new_surface(pcIconName, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
		surface_manager->Set_transparent_colour_key(m_pnHiLiteSurfaceIDs[i], m_nTransparentKey);
		pyHiLiteBitmap = surface_manager->Lock_surface(m_pnHiLiteSurfaceIDs[i]);
		nPitch = surface_manager->Get_pitch(m_pnHiLiteSurfaceIDs[i]);
		// Load the icon hilight
		SpriteXYFrameDraw(pyHiLiteBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 0, FALSE8, NULL, 255);
		// Unlock the surface
		surface_manager->Unlock_surface(m_pnHiLiteSurfaceIDs[i]);
	}

	// Now we are set up so make an initial call to the code for actually drawing the menu on the game screen.
	DrawIconMenu();

	PXCATCH

	Tdebug(EXCEPTION_LOG, "Exception in _icon_menu::Activate()");
	Fatal_error("Exception in _icon_menu::Activate()");

	PXENDCATCH

	Zdebug("Leaving _icon_menu::Activate()");
}

void _icon_menu::ReActivate() {
	int i;

	// Free up all the previous icon surfaces
	for (i = m_pIconList->GetIconCount() - 1; i >= 0; --i) {
		surface_manager->Kill_surface(m_pnIconSurfaceIDs[i]);
		surface_manager->Kill_surface(m_pnHiLiteSurfaceIDs[i]);
	}

	// Dump the off-screen arrows if we were using them.
	if (m_bWiderThanScreen) {
		surface_manager->Kill_surface(m_nLeftArrowID);
		surface_manager->Kill_surface(m_nRightArrowID);
		surface_manager->Kill_surface(m_nLeftArrowHiLiteID);
		surface_manager->Kill_surface(m_nRightArrowHiLiteID);
		SetUpOffScreenArrows();
	}

	// Now recreate and reload all those surfi
	for (i = m_pIconList->GetIconCount() - 1; i >= 0; --i) {
		// Get the full pathname for the icon.
		char pcIconName[MAXLEN_ICON_NAME];
		char pcFullIconName[MAXLEN_URL];
		char pcIconPath[MAXLEN_URL];

		strcpy(pcIconName, m_pIconList->GetIcon(i));
		sprintf(pcIconPath, ICON_PATH);
		sprintf(pcFullIconName, "%s%s.%s", pcIconPath, pcIconName, PX_BITMAP_PC_EXT);

		// Open the icon resource.
		uint32 nFullIconNameHash = NULL_HASH;
		_pxBitmap *psIconBitmap = (_pxBitmap *)rs_icons->Res_open(pcFullIconName, nFullIconNameHash, m_pcIconCluster, m_nIconClusterHash);

		// Check the schema is correct.
		if (psIconBitmap->schema != PC_BITMAP_SCHEMA)
			Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", pcFullIconName, PC_BITMAP_SCHEMA, psIconBitmap->schema);

		// Create a surface for the icon
		m_pnIconSurfaceIDs[i] = surface_manager->Create_new_surface("Icon", ICON_X_SIZE, ICON_Y_SIZE, EITHER);
		uint8 *pyIconBitmap = surface_manager->Lock_surface(m_pnIconSurfaceIDs[i]);
		uint32 nPitch = surface_manager->Get_pitch(m_pnIconSurfaceIDs[i]);

		// Load the icon into the surface
		SpriteXYFrameDraw(pyIconBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 0, FALSE8, NULL, 255);
		// convert it to b/w
		uint32 *icon_ad = (uint32 *)pyIconBitmap;
		for (uint32 y = 0; y < ICON_Y_SIZE; y++) {
			uint32 *rowAd = icon_ad;
			for (int x = 0; x < ICON_X_SIZE; x++) {
				uint32 col = *rowAd;
				if (col != m_nTransparentKey) {
					(*((uint8 *)rowAd + 0)) >>= 1;
					(*((uint8 *)rowAd + 1)) >>= 1;
					(*((uint8 *)rowAd + 2)) >>= 1;
					(*((uint8 *)rowAd + 3)) >>= 1;
				}
				rowAd++;
			}
			icon_ad += nPitch / 4;
		}
		// Unlock the surface
		surface_manager->Unlock_surface(m_pnIconSurfaceIDs[i]);

		// Create a surface for the icons hilite
		sprintf(pcIconName, "%sH", pcIconName);
		m_pnHiLiteSurfaceIDs[i] = surface_manager->Create_new_surface(pcIconName, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
		uint8 *pyHiLiteBitmap = surface_manager->Lock_surface(m_pnHiLiteSurfaceIDs[i]);
		nPitch = surface_manager->Get_pitch(m_pnHiLiteSurfaceIDs[i]);
		// Load the icon hilight
		SpriteXYFrameDraw(pyHiLiteBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 0, FALSE8, NULL, 255);
		// Unlock the surface
		surface_manager->Unlock_surface(m_pnHiLiteSurfaceIDs[i]);
		// Set the transparency keys for the icon
		surface_manager->Set_transparent_colour_key(m_pnIconSurfaceIDs[i], m_nTransparentKey);
		surface_manager->Set_transparent_colour_key(m_pnHiLiteSurfaceIDs[i], m_nTransparentKey);
	}
}

void _icon_menu::DrawIconMenu() {
	uint32 i;
	uint32 nIconIndex;
	uint32 nItemCount;
	int32 nStartX;
	LRECT sToRectangle;
	LRECT sFromRectangle;
	uint32 nIconCount;
	uint32 nMaxDrawableIcons, nIconsToDraw;
	int scrolling = 0;
	char pcDigits[16];
	const char *pcIconLabel;
	char pcIconName[MAXLEN_ICON_NAME];
	uint32 nHashRef;

	Zdebug("Entered _icon_menu::DrawIconMenu()");

	// Check if we are scrolling the icon menu up
	if (m_nMenuY != ICON_MENU_PIXEL_Y)
		m_nMenuY -= 15;
	if (m_nMenuY < ICON_MENU_PIXEL_Y)
		m_nMenuY = ICON_MENU_PIXEL_Y;

	// Get number of icons.
	nIconCount = m_pIconList->GetIconCount();

	// Work out where we start drawing the icons from (based on which one is currently selected).
	nIconIndex = m_nSelectedIcon;
	nStartX = ICON_MENU_PIXEL_X;

	int scrollyX = GetScrollingPosition(nStartX, nIconIndex);

	if ((nStartX != scrollyX) || (nIconIndex != m_nSelectedIcon)) {
		scrolling = 1;
		nStartX = scrollyX;
	}

	// Now modify the start drawing position and how many icons we can display based on whether or not
	// there are too many to fit on the screen.
	if (m_bWiderThanScreen) {
		// Allow space for the off-screen arrows, and start one position in.
		nMaxDrawableIcons = m_nMaxIconsDisplayed - 2;
		nStartX = nStartX + ICON_X_SIZE + ICON_SPACING;

		// Draw the left off-screen arrows.
		sToRectangle.left = ICON_MENU_PIXEL_X;
		sToRectangle.right = sToRectangle.left + ICON_X_SIZE - 1;
		sToRectangle.top = m_nMenuY;
		sToRectangle.bottom = sToRectangle.top + ICON_Y_SIZE - 1;

		sFromRectangle = ICON_BITMAP_RECT;

		if (sToRectangle.left < 0) {
			sFromRectangle.left -= sToRectangle.left;
			sToRectangle.left = 0;
		}

		if (sToRectangle.bottom > SCREEN_DEPTH) {
			sFromRectangle.bottom -= (sToRectangle.bottom - SCREEN_DEPTH);
			sToRectangle.bottom = SCREEN_DEPTH;
		}

		// Draw the flashing highlight if it is visible.
		surface_manager->Blit_surface_to_surface(m_nLeftArrowHiLiteID, working_buffer_id, &sFromRectangle, &sToRectangle, DDBLT_KEYSRC);

		// Draw the right off-screen arrows.
		sToRectangle.left = ICON_MENU_PIXEL_X + ((m_nMaxIconsDisplayed - 1) * (ICON_X_SIZE + ICON_SPACING));
		sToRectangle.right = sToRectangle.left + ICON_X_SIZE - 1;
		sToRectangle.top = ICON_MENU_PIXEL_Y;
		sToRectangle.bottom = ICON_MENU_PIXEL_Y + ICON_Y_SIZE - 1;

		sFromRectangle = ICON_BITMAP_RECT;

		if (sToRectangle.left < 0) {
			sFromRectangle.left -= sToRectangle.left;
			sToRectangle.left = 0;
		}

		if (sToRectangle.bottom > SCREEN_DEPTH) {
			sFromRectangle.bottom -= (sToRectangle.bottom - SCREEN_DEPTH);
			sToRectangle.bottom = SCREEN_DEPTH;
		}

		// Draw the flashing highlight if it is visible.
		surface_manager->Blit_surface_to_surface(m_nRightArrowHiLiteID, working_buffer_id, &sFromRectangle, &sToRectangle, DDBLT_KEYSRC);
	} else {
		nMaxDrawableIcons = m_nMaxIconsDisplayed;
	}

	// The number of icons we draw is the lesser of the maximum we can draw and how many there actually are.
	nIconsToDraw = (nIconCount < nMaxDrawableIcons) ? nIconCount : nMaxDrawableIcons;

	// Where to draw them
	int x = nStartX;

	// Loop for each icon to be drawn.
	for (i = 0; i < nIconsToDraw; ++i) {
		// Work out blit-to rectangle.
		sToRectangle.left = x;
		sToRectangle.right = sToRectangle.left + ICON_X_SIZE - 1;
		sToRectangle.top = m_nMenuY;
		sToRectangle.bottom = sToRectangle.top + ICON_Y_SIZE - 1;

		sFromRectangle = ICON_BITMAP_RECT;

		if (sToRectangle.left < 0) {
			sFromRectangle.left -= sToRectangle.left;
			sToRectangle.left = 0;
		}

		if (sToRectangle.bottom > SCREEN_DEPTH) {
			sFromRectangle.bottom -= (sToRectangle.bottom - SCREEN_DEPTH);
			sToRectangle.bottom = SCREEN_DEPTH;
		}

		// Get the icon name and hash for it.
		strcpy(pcIconName, m_pIconList->GetIcon(nIconIndex));
		nHashRef = HashString(pcIconName);

		// Now blit the icon itself.
		surface_manager->Blit_surface_to_surface(m_pnIconSurfaceIDs[nIconIndex], working_buffer_id, &sFromRectangle, &sToRectangle, DDBLT_KEYSRC);

		// Are we drawing the highlighted icon?
		if ((nIconIndex == m_nSelectedIcon) && (scrolling == 0)) {
			surface_manager->Blit_surface_to_surface(m_pnHiLiteSurfaceIDs[nIconIndex], working_buffer_id, &sFromRectangle, &sToRectangle, DDBLT_KEYSRC);

			// Look for the icon label in the global text file.
			pcIconLabel = (const char *)global_text->Try_fetch_item_by_hash(nHashRef);

			// If we found it, display it.
			if (pcIconLabel && (px.display_mode == THREED)) {
				SetTextColour(255, 255, 255);
				MS->Create_remora_text(sToRectangle.left, sToRectangle.top - 17, pcIconLabel, 2, PIN_AT_TOP_LEFT, 3, 2, 600);
				MS->Render_speech(MS->text_bloc);
				MS->Kill_remora_text();
			}
		}

		// Here, we deal with drawing the counts on the icons.
		nItemCount = m_sDuplicates.s_pnItemCounts[nIconIndex];

		// Only write the number on in 3D mode.
		if (px.display_mode == THREED) {
			// Write the number if greater than 1 or it is the clips or medipacks count.
			if (((nItemCount > 1) || (nHashRef == HashString(ARMS_HEALTH_NAME)) || (nHashRef == HashString(ARMS_AMMO_NAME))) && x > 0) {
				snprintf(pcDigits, 16, "%d", m_sDuplicates.s_pnItemCounts[nIconIndex]);
				if ((nIconIndex != m_nSelectedIcon) || (scrolling != 0))
					SetTextColour(160, 160, 160);
				else
					SetTextColour(255, 255, 255);
				MS->Create_remora_text(x, sToRectangle.top, (const char *)(pcDigits), 2, PIN_AT_TOP_LEFT, 3, 2, ICON_X_SIZE);
				MS->Render_speech(MS->text_bloc);
				MS->Kill_remora_text();
			}
		}

		// Work out index of next icon to be drawn.
		nIconIndex = (nIconIndex + 1) % nIconCount;

		// And where to draw the next one
		x += (ICON_X_SIZE + ICON_SPACING);
	}
}

void _icon_menu::CloseDownIconMenuDisplay() {
	uint32 i;
	uint32 nIconCount;

	Zdebug("Entered _icon_menu::CloseDownIconMenuDisplay()");

	nIconCount = m_pIconList->GetIconCount();

	// Dump the icon store graphic surfaces.
	for (i = 0; i < nIconCount; ++i) {
		surface_manager->Kill_surface(m_pnIconSurfaceIDs[i]);
		surface_manager->Kill_surface(m_pnHiLiteSurfaceIDs[i]);
	}

	// Dump the off-screen arrows if we were using them.
	if (m_bWiderThanScreen) {
		surface_manager->Kill_surface(m_nLeftArrowID);
		surface_manager->Kill_surface(m_nRightArrowID);
		surface_manager->Kill_surface(m_nLeftArrowHiLiteID);
		surface_manager->Kill_surface(m_nRightArrowHiLiteID);
	}

	Zdebug("Leaving _icon_menu::CloseDownIconMenuDisplay()");
}

void _icon_menu::SetTransparencyColourKey() {
	uint32 nFullIconNameHash = NULL_HASH;
	_pxBitmap *psTransparentBitmap;
	uint8 *pnPalette;
	uint32 nIconClusterHash;
	char pcFullIconName[MAXLEN_URL];
	char pcIconCluster[MAXLEN_CLUSTER_URL];

	strcpy(pcIconCluster, ICON_CLUSTER_PATH);
	nIconClusterHash = NULL_HASH;

	// Here we open the bitmap containing the reference colour for transparency and set it.
	sprintf(pcFullIconName, ICON_PATH);
	strcat(pcFullIconName, BITMAP_TRANSPARENCY_REFERENCE);
	strcat(pcFullIconName, ".");
	strcat(pcFullIconName, PX_BITMAP_PC_EXT);

	psTransparentBitmap = (_pxBitmap *)rs_icons->Res_open(pcFullIconName, nFullIconNameHash, pcIconCluster, nIconClusterHash);

	if (psTransparentBitmap->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", pcFullIconName, PC_BITMAP_SCHEMA, psTransparentBitmap->schema);

	pnPalette = psTransparentBitmap->Fetch_palette_pointer();
	m_nTransparentKey = ((uint32 *)pnPalette)[0];
}

void _icon_menu::SetupAdding(const char *pcIconName, uint32 &nSurfaceID) {
	uint32 nPitch;
	char pcFullIconName[MAXLEN_URL];
	char pcIconPath[MAXLEN_URL];
	uint32 nFullIconNameHash;
	_pxBitmap *psIconBitmap;
	uint8 *p8Bitmap;

	// Get the full pathname for the ammo clips icon.
	sprintf(pcIconPath, ICON_PATH);
	sprintf(pcFullIconName, "%s%s.%s", pcIconPath, pcIconName, PX_BITMAP_PC_EXT);

	// Open the icon resource.
	nFullIconNameHash = NULL_HASH;
	psIconBitmap = (_pxBitmap *)rs_icons->Res_open(pcFullIconName, nFullIconNameHash, m_pcIconCluster, m_nIconClusterHash);

	// Check the schema is correct.
	if (psIconBitmap->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", pcFullIconName, PC_BITMAP_SCHEMA, psIconBitmap->schema);

	// Create a surface for the clips icon.
	nSurfaceID = surface_manager->Create_new_surface(pcIconName, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
	surface_manager->Set_transparent_colour_key(nSurfaceID, m_nTransparentKey);
	p8Bitmap = surface_manager->Lock_surface(nSurfaceID);
	nPitch = surface_manager->Get_pitch(nSurfaceID);

	// Draw the icon into the surface.
	SpriteXYFrameDraw(p8Bitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 0, FALSE8, NULL, 255);

	// Finished drawing the icon into the surfaces so we can unlock them.
	surface_manager->Unlock_surface(nSurfaceID);
}

void _icon_menu::DrawAdding() {
	// Check if a symbol currently needs to be drawn and if so which one.
	switch (m_nAddedSymbol) {
	case 1:
		// Draw the medipack flash.
		surface_manager->Blit_surface_to_surface(m_nAddedMedipacksSurface, working_buffer_id, &ICON_BITMAP_RECT, &ICON_ADDING_RECT, DDBLT_KEYSRC);
		break;

	case 2:
		// Draw the ammo clips flash.
		surface_manager->Blit_surface_to_surface(m_nAddedClipsSurface, working_buffer_id, &ICON_BITMAP_RECT, &ICON_ADDING_RECT, DDBLT_KEYSRC);
		break;

	case 3:
		// Draw the ammo clips flash.
		surface_manager->Blit_surface_to_surface(m_nEmailArrivedSurface, working_buffer_id, &ICON_BITMAP_RECT, &ICON_ADDING_RECT, DDBLT_KEYSRC);
		break;

	default:
	    // Draw nothing.
	    ;
	}
}

void _icon_menu::CloseDownAdding() {
	Zdebug("Entered _icon_menu::CloseDownAdding()...");

	surface_manager->Kill_surface(m_nAddedMedipacksSurface);
	surface_manager->Kill_surface(m_nAddedClipsSurface);
	surface_manager->Kill_surface(m_nEmailArrivedSurface);

	Zdebug("Leaving _icon_menu::CloseDownAdding()...");
}

void _icon_menu::SetAddingClipsCount(uint32 nNumClips) {
	// Set the count.
	m_nAddedClips = (uint8)nNumClips;

	// Set the counter that controls the flashing.  Starts at 1 just to tidy up the initial flash.
	m_nAddedFlashCount = 0;

	// Prepare to draw the flashing icon (on PC only).
	SetupAdding(ARMS_AMMO_NAME, m_nAddedClipsSurface);
}

void _icon_menu::SetAddingMedipacksCount(uint32 nNumMedipacks) {
	// Set the count.
	m_nAddedMedipacks = (uint8)nNumMedipacks;

	// Set the counter that controls the flashing.
	m_nAddedFlashCount = 0;

	// Prepare to draw the flashing icon (on PC only).
	SetupAdding(ARMS_HEALTH_NAME, m_nAddedMedipacksSurface);
}

void _icon_menu::SetEmailArrived() {
	// If the icon menu is active and taking up the whole screen then we need to disable it because
	// it needs to adjust its drawing size.
	if ((m_eIconMenuGameState == ACTIVE) && (m_pIconList->GetIconCount() >= ICON_LIST_MAX_DISPLAYED))
		CloseDownIconMenu();

	// Set the email arrived flag.
	m_bEmailArrived = TRUE8;

	// Set the counter that controls the flashing.
	m_nAddedFlashCount = 0;

	// Prepare to draw the flashing icon (on PC only).
	SetupAdding(EMAIL_ARRIVED_NAME, m_nEmailArrivedSurface);
}

void _icon_menu::DrawArmedMenu(const int nBullets, const int maxBullets, const int nClips, const int maxClips) {
	// Ok, let's see just how selfcontained, yet inefficient we can make one function :o)
	uint32 gunSurface = 0;
	uint32 clipSurface = 0;

	// Load the 2 icons... We probably only deleted them last frame but whey !
	SetupAdding(ARMS_GUN_NAME, gunSurface);
	SetupAdding(ARMS_AMMO_NAME, clipSurface);
	// Icon positioning
	LRECT destRect;
	destRect.left = ICON_ARMED_MENU_PIXEL_X + 10;
	destRect.top = ICON_ARMED_MENU_PIXEL_Y;
	destRect.right = destRect.left + ICON_BITMAP_RECT.right - ICON_BITMAP_RECT.left;
	destRect.bottom = destRect.top + ICON_BITMAP_RECT.bottom - ICON_BITMAP_RECT.top;

	// Blit the icon ...
	surface_manager->Blit_surface_to_surface(gunSurface, working_buffer_id, &ICON_BITMAP_RECT, &destRect, DDBLT_KEYSRC);

	// ... and add the counter
	MS->Create_remora_text(destRect.left, destRect.top - 15, pxVString("%d/%d", nBullets, maxBullets), 2, PIN_AT_TOP_LEFT, 3, 2, 300);
	MS->Render_speech(MS->text_bloc);
	MS->Kill_remora_text();

	// Update the screen pos
	destRect.left += (ICON_X_SIZE + ICON_SPACING);
	destRect.right += (ICON_X_SIZE + ICON_SPACING);

	// Blit the icon
	surface_manager->Blit_surface_to_surface(clipSurface, working_buffer_id, &ICON_BITMAP_RECT, &destRect, DDBLT_KEYSRC);
	// and add the counter
	MS->Create_remora_text(destRect.left, destRect.top - 15, pxVString("%d/%d", nClips, maxClips), 2, PIN_AT_TOP_LEFT, 3, 2, 300);
	MS->Render_speech(MS->text_bloc);
	MS->Kill_remora_text();

	// Now delete the surface, so we are ready to recreate them next game cycle.
	surface_manager->Kill_surface(gunSurface);
	surface_manager->Kill_surface(clipSurface);
}

void _icon_menu::SetUpOffScreenArrows() {
	uint8 *pyLeftBitmap;
	uint8 *pyLeftHiLiteBitmap;
	uint8 *pyRightBitmap;
	uint8 *pyRightHiLiteBitmap;
	uint32 nPitch;
	uint32 nFullIconNameHash;
	_pxBitmap *psIconBitmap;
	uint32 nBufferCount;
	char pcArrowIconName[MAXLEN_URL];
	char pcIconPath[MAXLEN_URL];

	// Create surfaces for the left arrow - both highlighted and normal.
	m_nLeftArrowID = surface_manager->Create_new_surface(ICON_MENU_OFF_SCREEN_LEFT, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
	surface_manager->Set_transparent_colour_key(m_nLeftArrowID, m_nTransparentKey);
	pyLeftBitmap = surface_manager->Lock_surface(m_nLeftArrowID);

	m_nLeftArrowHiLiteID = surface_manager->Create_new_surface(ICON_MENU_OFF_SCREEN_LEFT, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
	surface_manager->Set_transparent_colour_key(m_nLeftArrowHiLiteID, m_nTransparentKey);
	pyLeftHiLiteBitmap = surface_manager->Lock_surface(m_nLeftArrowHiLiteID);

	// Get the pitch (assume it's the same for both.
	nPitch = surface_manager->Get_pitch(m_nLeftArrowID);

	// Open the icon (contains both the highlighted and normal frames).
	sprintf(pcIconPath, ICON_PATH);
	nBufferCount = sprintf(pcArrowIconName, "%s%s.%s", pcIconPath, ICON_MENU_OFF_SCREEN_LEFT, PX_BITMAP_PC_EXT);

	nFullIconNameHash = NULL_HASH;

	psIconBitmap = (_pxBitmap *)rs_icons->Res_open(pcArrowIconName, nFullIconNameHash, m_pcIconCluster, m_nIconClusterHash);

	if (psIconBitmap->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", pcArrowIconName, PC_BITMAP_SCHEMA, psIconBitmap->schema);

	// Draw the two frames onto their respective surfaces.
	SpriteXYFrameDraw(pyLeftBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 0, FALSE8, NULL, 255);
	SpriteXYFrameDraw(pyLeftHiLiteBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 1, FALSE8, NULL, 255);

	// Finished drawing the icon into the surfaces so we can unlock them.
	surface_manager->Unlock_surface(m_nLeftArrowID);
	surface_manager->Unlock_surface(m_nLeftArrowHiLiteID);

	// Now we repeat the whole thing for the right arrow.
	m_nRightArrowID = surface_manager->Create_new_surface(ICON_MENU_OFF_SCREEN_RIGHT, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
	surface_manager->Set_transparent_colour_key(m_nRightArrowID, m_nTransparentKey);
	pyRightBitmap = surface_manager->Lock_surface(m_nRightArrowID);

	m_nRightArrowHiLiteID = surface_manager->Create_new_surface(ICON_MENU_OFF_SCREEN_RIGHT, ICON_X_SIZE, ICON_Y_SIZE, EITHER);
	surface_manager->Set_transparent_colour_key(m_nRightArrowHiLiteID, m_nTransparentKey);
	pyRightHiLiteBitmap = surface_manager->Lock_surface(m_nRightArrowHiLiteID);

	// Get the pitch (assume it's the same for both.
	nPitch = surface_manager->Get_pitch(m_nRightArrowID);

	// Open the icon (contains both the highlighted and normal frames).
	nBufferCount = sprintf(pcArrowIconName, "%s%s.%s", pcIconPath, ICON_MENU_OFF_SCREEN_RIGHT, PX_BITMAP_PC_EXT);

	nFullIconNameHash = NULL_HASH;

	psIconBitmap = (_pxBitmap *)rs_icons->Res_open(pcArrowIconName, nFullIconNameHash, m_pcIconCluster, m_nIconClusterHash);

	if (psIconBitmap->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", pcArrowIconName, PC_BITMAP_SCHEMA, psIconBitmap->schema);

	// Draw the two frames onto their respective surfaces.
	SpriteXYFrameDraw(pyRightBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 0, FALSE8, NULL, 255);
	SpriteXYFrameDraw(pyRightHiLiteBitmap, nPitch, ICON_X_SIZE, ICON_Y_SIZE, psIconBitmap, 0, 0, 1, FALSE8, NULL, 255);

	// Finished drawing the icon into the surfaces so we can unlock them.
	surface_manager->Unlock_surface(m_nRightArrowID);
	surface_manager->Unlock_surface(m_nRightArrowHiLiteID);
}

} // End of namespace ICB
