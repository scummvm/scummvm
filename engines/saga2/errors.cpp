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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

/* ===================================================================== *
   Includes
 * ===================================================================== */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/errlist.h"
#include "saga2/errtype.h"

namespace Saga2 {

// ------------------------------------------------------------------
// FTA2 error text

char *FTA2ProgramErrors[] = {
	"No Error",
	"Unknown command line argument",


	"Minimum CPU not found",
	"Get a real machine",
	"Get a real machine",
	"Not in a DOS Box",
	"This program requires 8 Megabytes of RAM to run properly",
	"You do not have sufficient memory available",
	"This program requires more virtual memory to run properly",
	"You do not have sufficient virtual memory available",
	"No DPMI support",   // Minimum memory


	"You should run SETSOUND before running this program for the first time",
	"You should run SETSOUND and set up a digital sound driver",
	"You should run SETSOUND and set up a music driver",
	"Unaccelerated video card detected",
	"Get a real machine",
	"This game requires a mouse. No mouse driver was detected.",


	"Unable to allocate fault handler",
	"Direct X does not recognize your display. You may have inappropriate drivers.",
	"Could not initialize the audio",
	"Could not initialize the game clock",


	"A severe internal error has occurred",


	"A program file cannot be opened",
	"A necessary file can't be found. You may need to insert the game CD",
	"The game CD is required to play a video.",
	"Sound driver not detected. Continue anyway?",
	"Music driver not detected. Continue anyway?",

	"A program file is in use and cannot be opened",
	"A program file may be corrupt or wrong size",
	"A program file could not be read",
	"A program file could not be written",
	"A program file could not be closed",

	"A saved game file could not be read",
	"A saved game file could not be written",


	"The display is in use by another program",


	"An internal program error has been detected",


	"Are you sure you want to exit",
	"Direct X does not recognize your display. You may have inappropriate drivers. Emulation will allow you to continue, but perfomance will be severly degraded. Do you want to continue?",
	"Game heap overflowed continue?",

	""
};

// ------------------------------------------------------------------
// FTA2 error list

SequentialErrorList
programErrors(etiFTA2ErrorList, FTA2ProgramErrors);

#ifdef _WIN32

// ------------------------------------------------------------------
// WIN32 exceptions

ErrorRec win32Exceptions[] = {
	{ EXCEPTION_ACCESS_VIOLATION,  "General Access Violation" },
	{ EXCEPTION_BREAKPOINT,  "A debugger breakpoint" },
	{ EXCEPTION_DATATYPE_MISALIGNMENT,  "Data Alignment error" },
	{ EXCEPTION_SINGLE_STEP,  "Debugger single-step" },
	{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED,  "Array index out of bounds" },
	{ EXCEPTION_FLT_DENORMAL_OPERAND,  "FP: Denormal operand" },
	{ EXCEPTION_FLT_DIVIDE_BY_ZERO,  "FP: Division by zero" },
	{ EXCEPTION_FLT_INEXACT_RESULT,  "FP: inexact result" },
	{ EXCEPTION_FLT_INVALID_OPERATION,  "FP: invalid operand" },
	{ EXCEPTION_FLT_OVERFLOW,  "FP: overflow" },
	{ EXCEPTION_FLT_STACK_CHECK,  "FP: stack overflow" },
	{ EXCEPTION_FLT_UNDERFLOW,  "FP: underflow" },
	{ EXCEPTION_INT_DIVIDE_BY_ZERO,  "Int: division by zero" },
	{ EXCEPTION_INT_OVERFLOW,  "Int: overflow" },
	{ EXCEPTION_PRIV_INSTRUCTION,  "Priviledged instruction" },
	{ EXCEPTION_NONCONTINUABLE_EXCEPTION,  "Attempt to continue after last exception was unsucessful" }
};

// ------------------------------------------------------------------
// WIN32 exception error list

SparseErrorList
win32ExceptionList(etiWin32ExceptionList, 16, win32Exceptions);


#include "ddraw.h"

#ifndef __WATCOMC__
#pragma warning(disable : 4245)
#endif

ErrorRec directDrawErrors[] = {
	{ DDERR_ALREADYINITIALIZED, "This object is already initialized " },
	{ DDERR_BLTFASTCANTCLIP, "Return if a clipper object is attached to the source surface passed into a BltFast call." },
	{ DDERR_CANNOTATTACHSURFACE, "This surface can not be attached to the requested surface." },
	{ DDERR_CANNOTDETACHSURFACE, "This surface can not be detached from the requested surface." },
	{ DDERR_CANTCREATEDC, "Windows can not create any more DCs" },
	{ DDERR_CANTDUPLICATE, "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created." },
	{ DDERR_CLIPPERISUSINGHWND, "An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd." },
	{ DDERR_COLORKEYNOTSET, "No src color key specified for this operation." },
	{ DDERR_CURRENTLYNOTAVAIL, "Support is currently not available." },
	{ DDERR_DIRECTDRAWALREADYCREATED, "A DirectDraw object representing this driver has already been created for this process." },
	{ DDERR_EXCEPTION, "An exception was encountered while performing the requested operation." },
	{ DDERR_EXCLUSIVEMODEALREADYSET, "An attempt was made to set the cooperative level when it was already set to exclusive." },
	{ DDERR_GENERIC, "Generic failure." },
	{ DDERR_HEIGHTALIGN, "Height of rectangle provided is not a multiple of reqd alignment." },
	{ DDERR_HWNDALREADYSET, "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created." },
	{ DDERR_HWNDSUBCLASSED, "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state." },
	{ DDERR_IMPLICITLYCREATED, "This surface can not be restored because it is an implicitly created surface." },
	{ DDERR_INCOMPATIBLEPRIMARY, "Unable to match primary surface creation request with existing primary surface." },
	{ DDERR_INVALIDCAPS, "One or more of the caps bits passed to the callback are incorrect." },
	{ DDERR_INVALIDCLIPLIST, "DirectDraw does not support the provided cliplist." },
	{ DDERR_INVALIDDIRECTDRAWGUID, "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier." },
	{ DDERR_INVALIDMODE, "DirectDraw does not support the requested mode." },
	{ DDERR_INVALIDOBJECT, "DirectDraw received a pointer that was an invalid DIRECTDRAW object." },
	{ DDERR_INVALIDPARAMS, "One or more of the parameters passed to the function are incorrect." },
	{ DDERR_INVALIDPIXELFORMAT, "The pixel format was invalid as specified." },
	{ DDERR_INVALIDPOSITION, "Returned when the position of the overlay on the destination is no longer legal for that destination." },
	{ DDERR_INVALIDRECT, "Rectangle provided was invalid." },
	{ DDERR_LOCKEDSURFACES, "Operation could not be carried out because one or more surfaces are locked." },
	{ DDERR_NO3D, "There is no 3D present." },
	{ DDERR_NOALPHAHW, "Operation could not be carried out because there is no alpha accleration hardware present or available." },
	{ DDERR_NOBLTHW, "No blter hardware present." },
	{ DDERR_NOCLIPLIST, "No cliplist available." },
	{ DDERR_NOCLIPPERATTACHED, "No clipper object attached to surface object." },
	{ DDERR_NOCOLORCONVHW, "Operation could not be carried out because there is no color conversion hardware present or available." },
	{ DDERR_NOCOLORKEY, "Surface doesn't currently have a color key" },
	{ DDERR_NOCOLORKEYHW, "Operation could not be carried out because there is no hardware support of the destination color key." },
	{ DDERR_NOCOOPERATIVELEVELSET, "Create function called without DirectDraw object method SetCooperativeLevel being called." },
	{ DDERR_NODC, "No DC was ever created for this surface." },
	{ DDERR_NODDROPSHW, "No DirectDraw ROP hardware." },
	{ DDERR_NODIRECTDRAWHW, "A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware." },
	{ DDERR_NOEMULATION, "Software emulation not available." },
	{ DDERR_NOEXCLUSIVEMODE, "Operation requires the application to have exclusive mode but the application does not have exclusive mode." },
	{ DDERR_NOFLIPHW, "Flipping visible surfaces is not supported." },
	{ DDERR_NOGDI, "There is no GDI present." },
	{ DDERR_NOHWND, "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND." },
	{ DDERR_NOMIRRORHW, "Operation could not be carried out because there is no hardware present or available." },
	{ DDERR_NOOVERLAYDEST, "Returned when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination." },
	{ DDERR_NOOVERLAYHW, "Operation could not be carried out because there is no overlay hardware present or available." },
	{ DDERR_NOPALETTEATTACHED, "No palette object attached to this surface." },
	{ DDERR_NOPALETTEHW, "No hardware support for 16 or 256 color palettes." },
	{ DDERR_NORASTEROPHW, "Operation could not be carried out because there is no appropriate raster op hardware present or available." },
	{ DDERR_NOROTATIONHW, "Operation could not be carried out because there is no rotation hardware present or available." },
	{ DDERR_NOSTRETCHHW, "Operation could not be carried out because there is no hardware support for stretching." },
	{ DDERR_NOT4BITCOLOR, "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette." },
	{ DDERR_NOT4BITCOLORINDEX, "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette." },
	{ DDERR_NOT8BITCOLOR, "DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color" },
	{ DDERR_NOTAOVERLAYSURFACE, "Returned when an overlay member is called for a non-overlay surface." },
	{ DDERR_NOTEXTUREHW, "Operation could not be carried out because there is no texture mapping hardware present or available." },
	{ DDERR_NOTFLIPPABLE, "An attempt has been made to flip a surface that is not flippable." },
	{ DDERR_NOTFOUND, "Requested item was not found." },
	{ DDERR_NOTLOCKED, "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted." },
	{ DDERR_NOTPALETTIZED, "The surface being used is not a palette-based surface." },
	{ DDERR_NOVSYNCHW, "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations." },
	{ DDERR_NOZBUFFERHW, "Operation could not be carried out because there is no hardware support for zbuffer blting." },
	{ DDERR_NOZOVERLAYHW, "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays." },
	{ DDERR_OUTOFCAPS, "The hardware needed for the requested operation has already been allocated." },
	{ DDERR_OUTOFMEMORY, "DirectDraw does not have enough memory to perform the operation." },
	{ DDERR_OUTOFVIDEOMEMORY, "DirectDraw does not have enough memory to perform the operation." },
	{ DDERR_OVERLAYCANTCLIP, "The hardware does not support clipped overlays." },
	{ DDERR_OVERLAYCOLORKEYONLYONEACTIVE, "Can only have ony color key active at one time for overlays." },
	{ DDERR_OVERLAYNOTVISIBLE, "Returned when GetOverlayPosition is called on a hidden overlay." },
	{ DDERR_PALETTEBUSY, "Access to this palette is being refused because the palette is already locked by another thread." },
	{ DDERR_PRIMARYSURFACEALREADYEXISTS, "This process already has created a primary surface." },
	{ DDERR_REGIONTOOSMALL, "Region passed to Clipper::GetClipList is too small." },
	{ DDERR_SURFACEALREADYATTACHED, "This surface is already attached to the surface it is being attached to." },
	{ DDERR_SURFACEALREADYDEPENDENT, "This surface is already a dependency of the surface it is being made a dependency of." },
	{ DDERR_SURFACEBUSY, "Access to this surface is being refused because the surface is already locked by another thread." },
	{ DDERR_SURFACEISOBSCURED, "Access to surface refused because the surface is obscured." },
	{ DDERR_SURFACELOST, "Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it." },
	{ DDERR_SURFACENOTATTACHED, "The requested surface is not attached." },
	{ DDERR_TOOBIGHEIGHT, "Height requested by DirectDraw is too large." },
	{ DDERR_TOOBIGSIZE, "Size requested by DirectDraw is too large --	the individual height and width are OK." },
	{ DDERR_TOOBIGWIDTH, "Width requested by DirectDraw is too large." },
	{ DDERR_UNSUPPORTED, "Action not supported." },
	{ DDERR_UNSUPPORTEDFORMAT, "FOURCC format requested is unsupported by DirectDraw." },
	{ DDERR_UNSUPPORTEDMASK, "Bitmask in the pixel format requested is unsupported by DirectDraw." },
	{ DDERR_VERTICALBLANKINPROGRESS, "Vertical blank is in progress." },
	{ DDERR_WASSTILLDRAWING, "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete." },
	{ DDERR_WRONGMODE, "This surface can not be restored because it was created in a different mode." },
	{ DDERR_XALIGN, "Rectangle provided was not horizontally aligned on required boundary." }
};

SparseErrorList
directDrawErrorList(etiDDrawErrorList, 90, directDrawErrors);


#endif

}
