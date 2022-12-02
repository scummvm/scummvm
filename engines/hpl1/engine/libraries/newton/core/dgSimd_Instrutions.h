/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef AFX_SIMD_INTRUCTION_4563GFJK9R__INCLUDED_
#define AFX_SIMD_INTRUCTION_4563GFJK9R__INCLUDED_


#include "dgStdafx.h"
#include "dgTypes.h"

#ifdef DG_BUILD_SIMD_CODE
#ifdef __ppc__

#include <vecLib/veclib.h>

#define simd_type                   vFloat
#define simd_char                   vUInt8
#define simd_env                    vUInt16

union vFloatTuple {
	simd_type v;
	dgFloat32 f[4];
};


#define PURMUT_MASK(w, z, y, x)   (vUInt8)       \
	(x * 4 + 0, x * 4 + 1, x * 4 + 2, x * 4 + 3, \
	 y * 4 + 0, y * 4 + 1, y * 4 + 2, y * 4 + 3, \
	 z * 4 + 0, z * 4 + 1, z * 4 + 2, z * 4 + 3, \
	 w * 4 + 0, w * 4 + 1, w * 4 + 2, w * 4 + 3)


#define simd_get_ctrl()             vec_mfvscr ()
#define simd_set_ctrl(env)          vec_mtvscr (env)
#define simd_set_FZ_mode()          simd_set_ctrl (vec_or ((simd_env) (0, 0, 0, 0, 0, 0, 1, 0), simd_get_ctrl()))
#define simd_set1(a)                (vFloat) ((dgFloat32)a, (dgFloat32)a, (dgFloat32)a, (dgFloat32)a)
#define simd_load_s(a)              (vFloat) ((dgFloat32)a, (dgFloat32)a, (dgFloat32)a, (dgFloat32)a)
#define simd_load1_v(a)             (vFloat) ((dgFloat32)a, (dgFloat32)a, (dgFloat32)a, (dgFloat32)a)

#define simd_permut_v(a,b,mask)     vec_perm (a, b, mask)

#define simd_or_v(a,b)              vec_or (a, b)
#define simd_and_v(a,b)             vec_and (a, b)
#define simd_add_v(a,b)             vec_add (a, b)
#define simd_sub_v(a,b)             vec_sub (a, b)
#define simd_min_v(a,b)             vec_min (a, b)
#define simd_max_v(a,b)             vec_max (a, b)
#define simd_mul_v(a,b)             vec_madd (a, b, (simd_type) (0.0f, 0.0f, 0.0f, 0.0f))
#define simd_mul_add_v(a,b,c)       vec_madd (b, c, a)
#define simd_mul_sub_v(a,b,c)       vec_nmsub (b, c, a)
#define simd_cmpgt_v(a,b)           xxxxxx_mm_cmpgt_ps (a, b)
#define simd_rsqrt_v(a)             xxxx(a)

#define simd_add_s(a,b)             simd_add_v (a, b)
#define simd_sub_s(a,b)             simd_sub_v (a, b)
#define simd_mul_s(a,b)             simd_mul_v (a, b)
#define simd_min_s(a,b)             simd_min_v (a, b)
#define simd_max_s(a,b)             simd_max_v (a, b)
#define simd_mul_add_s(a,b,c)       simd_mul_add_v (a, b, c)
#define simd_mul_sub_s(a,b,c)       simd_mul_sub_v (a, b, c)
#define simd_cmpgt_s(a,b)           (simd_type) vec_cmpgt(a, b)
#define simd_store_s(a,x)           {vFloatTuple __tmp; __tmp.v = x; a = __tmp.f[0];}

#else

#define simd_type                   __m128
#define simd_env                    dgUnsigned32

#define simd_get_ctrl()             _mm_getcsr ()
#define simd_set_ctrl(a)            _mm_setcsr (a)
#define simd_set_FZ_mode()          _MM_SET_FLUSH_ZERO_MODE (_MM_FLUSH_ZERO_ON)

#define simd_set1(a)                _mm_set_ps1 (a)
#define simd_set(x,y,z,w)           _mm_set_ps(w, z, y, x)
#define simd_load_s(a)              _mm_load_ss (&a)
//#define simd_load_is(a,i)         _mm_cvtsi32_ss (a, i)
#define simd_load1_v(a)             _mm_load1_ps (&a)
#define simd_loadu_v(a)             _mm_loadu_ps (&a)


#define PURMUT_MASK(w, z, y, x)     _MM_SHUFFLE (w, z, y, x)
#define simd_permut_v(a,b,mask)     _mm_shuffle_ps (a,b,mask)


#define simd_or_v(a,b)              _mm_or_ps (a, b)
#define simd_and_v(a,b)             _mm_and_ps (a, b)
#define simd_xor_v(a,b)             _mm_xor_ps (a, b)
#define simd_andnot_v(a,b)          _mm_andnot_ps(b, a)
#define simd_add_v(a,b)             _mm_add_ps (a, b)
#define simd_sub_v(a,b)             _mm_sub_ps (a, b)
#define simd_min_v(a,b)             _mm_min_ps (a, b)
#define simd_max_v(a,b)             _mm_max_ps (a, b)
#define simd_mul_v(a,b)             _mm_mul_ps (a, b)
#define simd_mul_add_v(a,b,c)       _mm_add_ps (a, _mm_mul_ps (b, c))
#define simd_mul_sub_v(a,b,c)       _mm_sub_ps (a, _mm_mul_ps (b, c))
#define simd_cmpgt_v(a,b)           _mm_cmpgt_ps (a, b)
#define simd_cmpge_v(a,b)           _mm_cmpge_ps(a, b)
#define simd_cmplt_v(a,b)           _mm_cmplt_ps (a, b)
#define simd_cmple_v(a,b)           _mm_cmple_ps (a, b)
#define simd_div_v(a,b)             _mm_div_ps(a,b)
#define simd_rsqrt_v(a)             _mm_rsqrt_ps(a)
#define simd_store_v(a,ptr)         _mm_store_ps (ptr, a)

//  #define simd_mask_v(a)              _mm_movemask_ps(a)
#define simd_pack_lo_v(a,b)         _mm_unpacklo_ps(a,b)
#define simd_pack_hi_v(a,b)         _mm_unpackhi_ps(a,b)
#define simd_move_lh_v(a,b)         _mm_movelh_ps(a,b)
#define simd_move_hl_v(a,b)         _mm_movehl_ps(a,b)




#define simd_add_s(a,b)             _mm_add_ss (a, b)
#define simd_sub_s(a,b)             _mm_sub_ss (a, b)
#define simd_mul_s(a,b)             _mm_mul_ss (a, b)
#define simd_min_s(a,b)             _mm_min_ss (a, b)
#define simd_max_s(a,b)             _mm_max_ss (a, b)
#define simd_mul_add_s(a,b,c)       _mm_add_ss (a, _mm_mul_ss (b, c))
#define simd_mul_sub_s(a,b,c)       _mm_sub_ss (a, _mm_mul_ss (b, c))
#define simd_store_s(a,ptr)         _mm_store_ss (ptr, a)
#define simd_store_is(a)            _mm_cvtss_si32 (a)
//#define simd_store_d(a)               _mm_cvtps_pi32 (a)



#define simd_cmpgt_s(a,b)           _mm_cmpgt_ss(a, b)
#define simd_cmpge_s(a,b)           _mm_cmpge_ss(a, b)
#define simd_cmplt_s(a,b)           _mm_cmplt_ss (a, b)
#define simd_cmple_s(a,b)           _mm_cmple_ss (a, b)

#define simd_div_s(a,b)             _mm_div_ss(a,b)
#define simd_rcp_s(a)               _mm_rcp_ss(a)
#define simd_rsqrt_s(a)             _mm_rsqrt_ss(a)

#endif

#endif

#endif

