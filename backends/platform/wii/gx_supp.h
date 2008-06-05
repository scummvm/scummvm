/****************************************************************************
*   Generic GX Scaler 
*   softdev 2007
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program; if not, write to the Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* NGC GX Video Functions
*
* These are pretty standard functions to setup and use GX scaling.
****************************************************************************/
#ifndef _WII_GX_SUPP_H_
#define _WII_GX_SUPP_H_

#ifdef __cplusplus
extern "C" {
#endif

void GX_InitVideo();

void GX_Start(u16 width, u16 height, s16 haspect, s16 vaspect);
void GX_Render(u16 width, u16 height, u8 *buffer, u16 pitch);

#ifdef __cplusplus
}
#endif

#endif
