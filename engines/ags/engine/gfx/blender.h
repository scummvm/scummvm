//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// AGS specific color blending routines for transparency and tinting effects
//
//=============================================================================

#ifndef __AC_BLENDER_H
#define __AC_BLENDER_H

//
// Allegro's standard alpha blenders result in:
// - src and dst RGB are combined proportionally to src alpha
//   (src.rgb * src.alpha + dst.rgb * (1 - dst.alpha));
// - final alpha is zero.
// This blender is suggested for use with opaque destinations
// (ones without alpha channel).
//
/* Declared in Allegro's color.h:
void set_alpha_blender();
*/

unsigned long _myblender_color15(unsigned long x, unsigned long y, unsigned long n);
unsigned long _myblender_color16(unsigned long x, unsigned long y, unsigned long n);
unsigned long _myblender_color32(unsigned long x, unsigned long y, unsigned long n);
unsigned long _myblender_color15_light(unsigned long x, unsigned long y, unsigned long n);
unsigned long _myblender_color16_light(unsigned long x, unsigned long y, unsigned long n);
unsigned long _myblender_color32_light(unsigned long x, unsigned long y, unsigned long n);
// Customizable alpha blender that uses the supplied alpha value as src alpha,
// and preserves destination's alpha channel (if there was one);
void set_my_trans_blender(int r, int g, int b, int a);
// Argb2argb alpha blender combines RGBs proportionally to src alpha, but also
// applies dst alpha factor to the dst RGB used in the merge;
// The final alpha is calculated by multiplying two translucences (1 - .alpha).
// Custom alpha parameter, when not zero, is treated as fraction of source
// alpha that has to be used in color blending.
unsigned long _argb2argb_blender(unsigned long src_col, unsigned long dst_col, unsigned long src_alpha);
// Argb2rgb blender combines RGBs proportionally to src alpha, but discards alpha in the end.
// It is almost a clone of Allegro's _blender_alpha32, except it also applies optional overall alpha.
unsigned long _argb2rgb_blender(unsigned long src_col, unsigned long dst_col, unsigned long src_alpha);
// Rgb2argb blender treats all src pixels as if having opaque alpha.
unsigned long _rgb2argb_blender(unsigned long src_col, unsigned long dst_col, unsigned long src_alpha);
// Sets the alpha channel to opaque. Used when drawing a non-alpha sprite onto an alpha-sprite.
unsigned long _opaque_alpha_blender(unsigned long src_col, unsigned long dst_col, unsigned long src_alpha);

// Additive alpha blender plain copies src over, applying a summ of src and
// dst alpha values.
void set_additive_alpha_blender();
// Opaque alpha blender plain copies src over, applying opaque alpha value.
void set_opaque_alpha_blender();
// Sets argb2argb for 32-bit mode, and provides appropriate funcs for blending 32-bit onto 15/16/24-bit destination
void set_argb2any_blender();

#endif // __AC_BLENDER_H
