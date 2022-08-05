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

#include "dgStdafx.h"
#include "dgMemory.h"
#include "dgGoogol.h"
#include "dgIntersections.h"

#define USE_FLOAT_VERSION



#define DG_RAY_TOL_ERROR (dgFloat32 (-1.0e-3f))
#define DG_RAY_TOL_ADAPTIVE_ERROR (dgFloat32 (1.0e-1f))


dgFastRayTest::dgFastRayTest(const dgVector& l0, const dgVector& l1) :
    m_p0(l0), m_p1(l1), m_diff(l1 - l0), m_minT(dgFloat32(0.0f),
        dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f)), m_maxT(
        dgFloat32(1.0f), dgFloat32(1.0f), dgFloat32(1.0f), dgFloat32(1.0f)),
	m_zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f))
{
  m_isParallel[0] =
      (dgAbsf(m_diff.m_x) > dgFloat32(1.0e-8f)) ? 0 : dgInt32(0xffffffff);
  m_isParallel[1] =
      (dgAbsf(m_diff.m_y) > dgFloat32(1.0e-8f)) ? 0 : dgInt32(0xffffffff);
  m_isParallel[2] =
      (dgAbsf(m_diff.m_z) > dgFloat32(1.0e-8f)) ? 0 : dgInt32(0xffffffff);
  m_isParallel[3] = 0;

  m_dpInv.m_x =
      (!m_isParallel[0]) ? (dgFloat32(1.0f) / m_diff.m_x) : dgFloat32(1.0e20f);
  m_dpInv.m_y =
      (!m_isParallel[1]) ? (dgFloat32(1.0f) / m_diff.m_y) : dgFloat32(1.0e20f);
  m_dpInv.m_z =
      (!m_isParallel[2]) ? (dgFloat32(1.0f) / m_diff.m_z) : dgFloat32(1.0e20f);
  m_dpInv.m_w = dgFloat32(0.0f);
  m_dpBaseInv = m_dpInv;

//	m_ray_xxx = dgVector (m_diff.m_x, m_diff.m_x, m_diff.m_x, dgFloat32 (0.0f));
//	m_ray_yyy = dgVector (m_diff.m_y, m_diff.m_y, m_diff.m_y, dgFloat32 (0.0f));
//	m_ray_zzz = dgVector (m_diff.m_z, m_diff.m_z, m_diff.m_z, dgFloat32 (0.0f));

  m_ray_xxxx = dgVector(m_diff.m_x, m_diff.m_x, m_diff.m_x, m_diff.m_x);
  m_ray_yyyy = dgVector(m_diff.m_y, m_diff.m_y, m_diff.m_y, m_diff.m_y);
  m_ray_zzzz = dgVector(m_diff.m_z, m_diff.m_z, m_diff.m_z, m_diff.m_z);

  dgFloat32 mag = dgSqrt (m_diff % m_diff);
  m_dirError = -dgFloat32(0.0175f) * mag;
  m_magRayTest = GetMax (mag, dgFloat32 (1.0f));
}

dgInt32 dgFastRayTest::BoxTestSimd(const dgVector& minBox,
    const dgVector& maxBox) const
{
#ifdef DG_BUILD_SIMD_CODE
//	dgInt32 isParallel;

//	simd_type t0;
//	simd_type t1;
//	simd_type tt0;
//	simd_type tt1;
//	simd_type test;
//	simd_type paralletTest;

  simd_type tt0 =
      simd_and_v (simd_or_v (simd_cmple_v((simd_type&)m_p0, (simd_type&)minBox), simd_cmpge_v((simd_type&)m_p0, (simd_type&)maxBox)), (simd_type&)m_isParallel);
  tt0 = simd_or_v (tt0, simd_move_hl_v (tt0, tt0));

//	dgFloatSign isParallel;
//	simd_store_s(simd_or_v (tt0, simd_permut_v (tt0, tt0, PURMUT_MASK(3, 2, 1, 1))), &isParallel.m_fVal);
//	if (isParallel.m_integer.m_iVal) {
  if (simd_store_is(simd_or_v (tt0, simd_permut_v (tt0, tt0, PURMUT_MASK(3, 2, 1, 1)))))
  {
    return 0;
  }

  tt0 =
      simd_mul_v (simd_sub_v ((simd_type&)minBox, (simd_type&)m_p0), (simd_type&)m_dpInv);
  simd_type tt1 =
      simd_mul_v (simd_sub_v ((simd_type&)maxBox, (simd_type&)m_p0), (simd_type&)m_dpInv);

  simd_type t0 = simd_max_v(simd_min_v(tt0, tt1), (simd_type&)m_minT);
  simd_type t1 = simd_min_v(simd_max_v(tt0, tt1), (simd_type&)m_maxT);

  t0 = simd_max_v(t0, simd_permut_v (t0, t0, PURMUT_MASK(3, 2, 1, 2)));
  t1 = simd_min_v(t1, simd_permut_v (t1, t1, PURMUT_MASK(3, 2, 1, 2)));

  t0 = simd_max_s(t0, simd_permut_v (t0, t0, PURMUT_MASK(3, 2, 1, 1)));
  t1 = simd_min_s(t1, simd_permut_v (t1, t1, PURMUT_MASK(3, 2, 1, 1)));

//	simd_store_s(simd_cmple_s(t0, t1), &isParallel.m_fVal);
//	return isParallel.m_integer.m_iVal;
  return simd_store_is(simd_cmple_s(t0, t1));
#else
  return 0;
#endif
}

dgInt32 dgFastRayTest::BoxTest(const dgVector& minBox,
    const dgVector& maxBox) const
{
  dgFloat32 tmin = 0.0f;
  dgFloat32 tmax = 1.0f;

  for (dgInt32 i = 0; i < 3; i++)
  {
    if (m_isParallel[i])
    {
      if (m_p0[i] <= minBox[i] || m_p0[i] >= maxBox[i])
      {
        return 0;
      }
    }
    else
    {
      dgFloat32 t1 = (minBox[i] - m_p0[i]) * m_dpInv[i];
      dgFloat32 t2 = (maxBox[i] - m_p0[i]) * m_dpInv[i];

      if (t1 > t2)
      {
        Swap(t1, t2);
      }
      if (t1 > tmin)
      {
        tmin = t1;
      }
      if (t2 < tmax)
      {
        tmax = t2;
      }
      if (tmin > tmax)
      {
        return 0;
      }
    }
  }
  return 0xffffff;
}

dgFloat32 dgFastRayTest::PolygonIntersectSimd(const dgVector& normal,
    const dgFloat32* const polygon, dgInt32 strideInBytes,
    const dgInt32* const indexArray, dgInt32 indexCount) const
{
#ifdef DG_BUILD_SIMD_CODE

  /*
   dgFloatSign test;

   _ASSERTE (m_p0.m_w == m_p1.m_w);

   simd_type dist = simd_mul_v ((simd_type&)normal, (simd_type&)m_diff);
   dist = simd_add_s (dist, simd_permut_v(dist, dist, PURMUT_MASK(3, 2, 1, 2)));
   dist = simd_add_s (dist, simd_permut_v(dist, dist, PURMUT_MASK(3, 2, 1, 1)));
   //	simd_store_s (simd_cmple_s (dist, simd_set1(dgFloat32 (0.0f))), &test.m_fVal);
   simd_store_s (simd_cmple_s (dist, simd_set1(m_dirError)), &test.m_fVal);

   //	if (dist < dgFloat32 (0.0f)) {
   if (test.m_integer.m_iVal) {
   dgInt32 i1;

   dgInt32 stride = strideInBytes / sizeof (dgFloat32);

   dgInt32 i0 = indexArray[0] * stride;
   simd_type v0 = simd_loadu_v (polygon[i0]);
   simd_type p0v0 = simd_sub_v (v0, (simd_type&)m_p0);

   simd_type num = simd_mul_v ((simd_type&)normal, p0v0);
   num = simd_add_s (num, simd_permut_v(num, num, PURMUT_MASK(3, 2, 1, 2)));
   num = simd_add_s (num, simd_permut_v(num, num, PURMUT_MASK(3, 2, 1, 1)));
   //		if ((tOut < dgFloat32 (0.0f)) && (tOut > dist)) {
   simd_store_s (simd_and_v (simd_cmplt_s (num, (simd_type&) m_zero), simd_cmpgt_s (num, (simd_type&) dist)), (dgFloat32*) &i1);
   if (i1) {
   i1 = indexArray[1] * stride;
   simd_type v1 = simd_loadu_v (polygon[i1]);
   simd_type p0v1 = simd_sub_v (v1, (simd_type&)m_p0);

   for (dgInt32 i = 2; i < indexCount; i ++) {
   dgFloatSign test;

   i1 = indexArray[i] * stride;
   //			dgVector v2 (&polygon[i2]);
   simd_type v2 = simd_loadu_v (polygon[i1]);
   //			dgVector p0v2 (v2 - ray_p0);
   simd_type p0v2 = simd_sub_v (v2, (simd_type&)m_p0);

   simd_type p0v_y = simd_pack_lo_v (p0v0, p0v1);
   simd_type p0v_x = simd_move_lh_v (p0v_y, p0v2);
   p0v_y = simd_permut_v (p0v_y, p0v2, PURMUT_MASK (3, 1, 3, 2));
   simd_type p0v_z = simd_permut_v (simd_pack_hi_v (p0v0, p0v1), p0v2, PURMUT_MASK (3, 2, 1, 0));

   simd_type tmp = simd_sub_v (simd_mul_v ((simd_type&)m_ray_yyyy, p0v_z), simd_mul_v ((simd_type&)m_ray_zzzz, p0v_y));
   simd_type alpha = simd_mul_v (simd_permut_v (tmp, tmp, PURMUT_MASK (3, 0, 2, 1)), p0v_x);

   tmp = simd_sub_v (simd_mul_v ((simd_type&)m_ray_zzzz, p0v_x), simd_mul_v ((simd_type&)m_ray_xxxx, p0v_z));
   alpha = simd_mul_add_v (alpha, simd_permut_v (tmp, tmp, PURMUT_MASK (3, 0, 2, 1)), p0v_y);

   tmp = simd_sub_v (simd_mul_v ((simd_type&)m_ray_xxxx, p0v_y), simd_mul_v ((simd_type&)m_ray_yyyy, p0v_x));
   alpha = simd_mul_add_v (alpha, simd_permut_v (tmp, tmp, PURMUT_MASK (3, 0, 2, 1)), p0v_z);

   tmp = simd_cmpgt_v (alpha, (simd_type&) m_tolerance);
   tmp = simd_and_v (tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3, 2, 1, 2)));

   simd_store_s (simd_and_v (tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3, 2, 1, 1))), &test.m_fVal);
   if (test.m_integer.m_iVal) {
   dgFloat32 tOut;
   simd_store_s (simd_div_s(num, dist), &tOut);
   _ASSERTE (tOut >= dgFloat32 (0.0f));
   _ASSERTE (tOut <= dgFloat32 (1.0f));
   return tOut;
   }
   p0v1 = p0v2;
   }
   }
   }
   return 1.2f;
   */

  _ASSERTE(m_p0.m_w == m_p1.m_w);

  dgFloat32 dist = normal % m_diff;
  if (dist < m_dirError)
  {
    dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat32));

    dgVector v0(&polygon[indexArray[indexCount - 1] * stride]);
    dgVector p0v0(v0 - m_p0);
    dgFloat32 tOut = normal % p0v0;
    // this only work for convex polygons and for single side faces
    // walk the polygon around the edges and calculate the volume

    if ((tOut < dgFloat32(0.0f)) && (tOut > dist))
    {
      dgInt32 i3 = indexCount - 1;
      dgInt32 i2 = indexCount - 2;
      dgInt32 i1 = indexCount - 3;
      dgInt32 i0 = (indexCount > 3) ? indexCount - 4 : 2;

	  simd_type tolerance = simd_set1 (m_magRayTest * DG_RAY_TOL_ADAPTIVE_ERROR);
      for (dgInt32 i4 = 0; i4 < indexCount; i4 += 4)
      {
//				dgVector v1 (&polygon[i2]);
//				dgVector p0v1 (v1 - m_p0);

        simd_type v0 = simd_loadu_v (polygon[indexArray[i0] * stride]);
        simd_type v1 = simd_loadu_v (polygon[indexArray[i1] * stride]);
        simd_type v2 = simd_loadu_v (polygon[indexArray[i2] * stride]);
        simd_type v3 = simd_loadu_v (polygon[indexArray[i3] * stride]);
        simd_type v4 = simd_loadu_v (polygon[indexArray[i4] * stride]);

        simd_type p0v0 = simd_sub_v (v0, (simd_type&)m_p0);
        simd_type p0v1 = simd_sub_v (v1, (simd_type&)m_p0);
        simd_type p0v2 = simd_sub_v (v2, (simd_type&)m_p0);
        simd_type p0v3 = simd_sub_v (v3, (simd_type&)m_p0);
        simd_type p0v4 = simd_sub_v (v4, (simd_type&)m_p0);

        // transpose the data into a structure of arrays
        simd_type tmp0 = simd_pack_lo_v(p0v0, p0v1);
        simd_type tmp1 = simd_pack_lo_v(p0v2, p0v3);
        simd_type p0v0_x = simd_move_lh_v (tmp0, tmp1);
        simd_type p0v0_y = simd_move_hl_v (tmp1, tmp0);
        tmp0 = simd_pack_hi_v(p0v0, p0v1);
        tmp1 = simd_pack_hi_v(p0v2, p0v3);
        simd_type p0v0_z = simd_move_lh_v (tmp0, tmp1);

        tmp0 = simd_pack_lo_v(p0v1, p0v2);
        tmp1 = simd_pack_lo_v(p0v3, p0v4);
        simd_type p0v1_x = simd_move_lh_v (tmp0, tmp1);
        simd_type p0v1_y = simd_move_hl_v (tmp1, tmp0);
        tmp0 = simd_pack_hi_v(p0v1, p0v2);
        tmp1 = simd_pack_hi_v(p0v3, p0v4);
        simd_type p0v1_z = simd_move_lh_v (tmp0, tmp1);

        //dgFloat32 alpha = (m_diff * p0v1) % p0v0;
        simd_type cross =
            simd_mul_add_v (simd_mul_add_v (simd_mul_v(p0v0_x, simd_mul_sub_v (simd_mul_v ((simd_type&)m_ray_yyyy, p0v1_z), (simd_type&)m_ray_zzzz, p0v1_y)),
                    p0v0_y, simd_mul_sub_v (simd_mul_v ((simd_type&)m_ray_zzzz, p0v1_x), (simd_type&)m_ray_xxxx, p0v1_z)),
                p0v0_z, simd_mul_sub_v (simd_mul_v ((simd_type&)m_ray_xxxx, p0v1_y), (simd_type&)m_ray_yyyy, p0v1_x));

        // if a least one volume is negative it mean the line cross the polygon outside this edge and do not hit the face
        //if (alpha < DG_RAY_TOL_ERROR) {
        //	return 1.2f;
        //}
        tmp0 = simd_cmpgt_v (cross, tolerance);
        tmp0 = simd_and_v (tmp0, simd_move_hl_v (tmp0, tmp0));
        tmp0 =
            simd_and_v (tmp0, simd_permut_v (tmp0, tmp0, PURMUT_MASK (0, 0, 0, 1)));

//				dgFloatSign test;
//				simd_store_s (tmp0, &test.m_fVal);
//				if (!test.m_integer.m_iVal) {
        if (!simd_store_is (tmp0))
        {
          return 1.2f;
        }

        // calculate the volume formed by the line and the edge of the polygon
//				p0v0 = p0v1;

        i3 = i4 + 3;
        i2 = i4 + 2;
        i1 = i4 + 1;
        i0 = i4 + 0;
      }

      //the line is to the left of all the polygon edges,
      //then the intersection is the point we the line intersect the plane of the polygon
      tOut = tOut / dist;
      _ASSERTE(tOut >= dgFloat32 (0.0f));
      _ASSERTE(tOut <= dgFloat32 (1.0f));
      return tOut;
    }
  }
  return dgFloat32(1.2f);
#else
  return dgFloat32 (0.0f);
#endif
}


dgFloat32 dgFastRayTest::PolygonIntersect (const dgVector& normal, const dgFloat32* const polygon, dgInt32 strideInBytes, const dgInt32* const indexArray, dgInt32 indexCount) const
{
	_ASSERTE (m_p0.m_w == m_p1.m_w);

	dgFloat32 dist = normal % m_diff;
	if (dist < m_dirError) {

		dgInt32 stride = dgInt32 (strideInBytes / sizeof (dgFloat32));

		dgVector v0 (&polygon[indexArray[indexCount - 1] * stride]);
		dgVector p0v0 (v0 - m_p0);
		dgFloat32 tOut = normal % p0v0;
		// this only work for convex polygons and for single side faces 
		// walk the polygon around the edges and calculate the volume 
		if ((tOut < dgFloat32 (0.0f)) && (tOut > dist)) {
			for (dgInt32 i = 0; i < indexCount; i ++) {
				dgInt32 i2 = indexArray[i] * stride;
				dgVector v1 (&polygon[i2]);
				dgVector p0v1 (v1 - m_p0);
				// calculate the volume formed by the line and the edge of the polygon
				dgFloat32 alpha = (m_diff * p0v1) % p0v0;
				// if a least one volume is negative it mean the line cross the polygon outside this edge and do not hit the face
				if (alpha < DG_RAY_TOL_ERROR) {
					return 1.2f;
				}
				p0v0 = p0v1;
			}

			//the line is to the left of all the polygon edges, 
			//then the intersection is the point we the line intersect the plane of the polygon
			tOut = tOut / dist;
			_ASSERTE (tOut >= dgFloat32 (0.0f));
			_ASSERTE (tOut <= dgFloat32 (1.0f));
			return tOut;
		}
	}
	return dgFloat32 (1.2f);
}

bool dgApi dgRayBoxClip(dgVector& p0, dgVector& p1, const dgVector& boxP0,
    const dgVector& boxP1)
{
  for (int i = 0; i < 3; i++)
  {
    dgFloat32 tmp0;
    dgFloat32 tmp1;

    tmp0 = boxP1[i] - p0[i];
    if (tmp0 > dgFloat32(0.0f))
    {
      tmp1 = boxP1[i] - p1[i];
      if (tmp1 < dgFloat32(0.0f))
      {
        p1 = p0 + (p1 - p0).Scale(tmp0 / (p1[i] - p0[i]));
        p1[i] = boxP1[i];
      }
    }
    else
    {
      tmp1 = boxP1[i] - p1[i];
      if (tmp1 > dgFloat32(0.0f))
      {
        p0 += (p1 - p0).Scale(tmp0 / (p1[i] - p0[i]));
        p0[i] = boxP1[i];
      }
      else
      {
        return false;
      }
    }

    tmp0 = boxP0[i] - p0[i];
    if (tmp0 < dgFloat32(0.0f))
    {
      tmp1 = boxP0[i] - p1[i];
      if (tmp1 > dgFloat32(0.0f))
      {
        p1 = p0 + (p1 - p0).Scale(tmp0 / (p1[i] - p0[i]));
        p1[i] = boxP0[i];
      }
    }
    else
    {
      tmp1 = boxP0[i] - p1[i];
      if (tmp1 < dgFloat32(0.0f))
      {
        p0 += (p1 - p0).Scale(tmp0 / (p1[i] - p0[i]));
        p0[i] = boxP0[i];
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

dgVector dgApi dgPointToRayDistance(const dgVector& point,
    const dgVector& ray_p0, const dgVector& ray_p1)
{
  dgFloat32 t;
  dgVector dp(ray_p1 - ray_p0);
  t = ClampValue(((point - ray_p0) % dp) / (dp % dp),
      dgFloat32(dgFloat32(0.0f)), dgFloat32(dgFloat32(1.0f)));
  return ray_p0 + dp.Scale(t);
}

void dgApi dgRayToRayDistance(const dgVector& ray_p0, const dgVector& ray_p1,
    const dgVector& ray_q0, const dgVector& ray_q1, dgVector& pOut,
    dgVector& qOut)
{
  dgFloat32 sN;
  dgFloat32 tN;

  dgVector u(ray_p1 - ray_p0);
  dgVector v(ray_q1 - ray_q0);
  dgVector w(ray_p0 - ray_q0);

  dgFloat32 a = u % u; // always >= 0
  dgFloat32 b = u % v;
  dgFloat32 c = v % v; // always >= 0
  dgFloat32 d = u % w;
  dgFloat32 e = v % w;
  dgFloat32 D = a * c - b * b; // always >= 0
  dgFloat32 sD = D; // sc = sN / sD, default sD = D >= 0
  dgFloat32 tD = D; // tc = tN / tD, default tD = D >= 0

  // compute the line parameters of the two closest points
  if (D < dgFloat32(1.0e-8f))
  { // the lines are almost parallel
    sN = dgFloat32(0.0f); // force using point P0 on segment S1
    sD = dgFloat32(1.0f); // to prevent possible division by 0.0 later
    tN = e;
    tD = c;
  }
  else
  { // get the closest points on the infinite lines
    sN = (b * e - c * d);
    tN = (a * e - b * d);
    if (sN < dgFloat32(0.0f))
    { // sc < 0 => the s=0 edge is visible
      sN = dgFloat32(0.0f);
      tN = e;
      tD = c;
    }
    else if (sN > sD)
    { // sc > 1 => the s=1 edge is visible
      sN = sD;
      tN = e + b;
      tD = c;
    }
  }

  if (tN < dgFloat32(0.0f))
  { // tc < 0 => the t=0 edge is visible
    tN = dgFloat32(0.0f);
    // recompute sc for this edge
    if (-d < dgFloat32(0.0f))
      sN = dgFloat32(0.0f);
    else if (-d > a)
      sN = sD;
    else
    {
      sN = -d;
      sD = a;
    }
  }
  else if (tN > tD)
  { // tc > 1 => the t=1 edge is visible
    tN = tD;
    // recompute sc for this edge
    if ((-d + b) < dgFloat32(0.0f))
      sN = dgFloat32(0.0f);
    else if ((-d + b) > a)
      sN = sD;
    else
    {
      sN = (-d + b);
      sD = a;
    }
  }

  // finally do the division to get sc and tc
  dgFloat32 sc = (dgAbsf(sN) < dgFloat32(1.0e-8f) ? dgFloat32(0.0f) : sN / sD);
  dgFloat32 tc = (dgAbsf(tN) < dgFloat32(1.0e-8f) ? dgFloat32(0.0f) : tN / tD);

  pOut = ray_p0 + u.Scale(sc);
  qOut = ray_q0 + v.Scale(tc);
}

dgVector dgPointToTriangleDistance(const dgVector& point, const dgVector& p0,
    const dgVector& p1, const dgVector& p2)
{
  //	const dgVector p (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
  const dgVector p10(p1 - p0);
  const dgVector p20(p2 - p0);
  const dgVector p_p0(point - p0);

  dgFloat32 alpha1 = p10 % p_p0;
  dgFloat32 alpha2 = p20 % p_p0;
  if ((alpha1 <= dgFloat32(0.0f)) && (alpha2 <= dgFloat32(0.0f)))
  {
    return p0;
  }

  dgVector p_p1(point - p1);
  dgFloat32 alpha3 = p10 % p_p1;
  dgFloat32 alpha4 = p20 % p_p1;
  if ((alpha3 >= dgFloat32(0.0f)) && (alpha4 <= alpha3))
  {
    return p1;
  }

  dgFloat32 vc = alpha1 * alpha4 - alpha3 * alpha2;
  if ((vc <= dgFloat32(0.0f)) && (alpha1 >= dgFloat32(0.0f))
      && (alpha3 <= dgFloat32(0.0f)))
  {
    dgFloat32 t = alpha1 / (alpha1 - alpha3);
    _ASSERTE(t >= dgFloat32 (0.0f));
    _ASSERTE(t <= dgFloat32 (1.0f));
    return p0 + p10.Scale(t);
  }

  dgVector p_p2(point - p2);
  dgFloat32 alpha5 = p10 % p_p2;
  dgFloat32 alpha6 = p20 % p_p2;
  if ((alpha6 >= dgFloat32(0.0f)) && (alpha5 <= alpha6))
  {
    return p2;
  }

  dgFloat32 vb = alpha5 * alpha2 - alpha1 * alpha6;
  if ((vb <= dgFloat32(0.0f)) && (alpha2 >= dgFloat32(0.0f))
      && (alpha6 <= dgFloat32(0.0f)))
  {
    dgFloat32 t = alpha2 / (alpha2 - alpha6);
    _ASSERTE(t >= dgFloat32 (0.0f));
    _ASSERTE(t <= dgFloat32 (1.0f));
    return p0 + p20.Scale(t);
  }

  dgFloat32 va = alpha3 * alpha6 - alpha5 * alpha4;
  if ((va <= dgFloat32(0.0f)) && ((alpha4 - alpha3) >= dgFloat32(0.0f))
      && ((alpha5 - alpha6) >= dgFloat32(0.0f)))
  {
    dgFloat32 t = (alpha4 - alpha3) / ((alpha4 - alpha3) + (alpha5 - alpha6));
    _ASSERTE(t >= dgFloat32 (0.0f));
    _ASSERTE(t <= dgFloat32 (1.0f));
    return p1 + (p2 - p1).Scale(t);
  }

  dgFloat32 den = float(dgFloat32(1.0f)) / (va + vb + vc);
  dgFloat32 t = vb * den;
  dgFloat32 s = vc * den;
  _ASSERTE(t >= dgFloat32 (0.0f));
  _ASSERTE(s >= dgFloat32 (0.0f));
  _ASSERTE(t <= dgFloat32 (1.0f));
  _ASSERTE(s <= dgFloat32 (1.0f));
  return p0 + p10.Scale(t) + p20.Scale(s);
}

dgBigVector dgPointToTriangleDistance(const dgBigVector& point,
    const dgBigVector& p0, const dgBigVector& p1, const dgBigVector& p2)
{
  //	const dgBigVector p (dgFloat64 (0.0f), dgFloat64 (0.0f), dgFloat64 (0.0f));
  const dgBigVector p10(p1 - p0);
  const dgBigVector p20(p2 - p0);
  const dgBigVector p_p0(point - p0);

  dgFloat64 alpha1 = p10 % p_p0;
  dgFloat64 alpha2 = p20 % p_p0;
  if ((alpha1 <= dgFloat64(0.0f)) && (alpha2 <= dgFloat64(0.0f)))
  {
    return p0;
  }

  dgBigVector p_p1(point - p1);
  dgFloat64 alpha3 = p10 % p_p1;
  dgFloat64 alpha4 = p20 % p_p1;
  if ((alpha3 >= dgFloat64(0.0f)) && (alpha4 <= alpha3))
  {
    return p1;
  }

  dgFloat64 vc = alpha1 * alpha4 - alpha3 * alpha2;
  if ((vc <= dgFloat64(0.0f)) && (alpha1 >= dgFloat64(0.0f))
      && (alpha3 <= dgFloat64(0.0f)))
  {
    dgFloat64 t = alpha1 / (alpha1 - alpha3);
    _ASSERTE(t >= dgFloat64 (0.0f));
    _ASSERTE(t <= dgFloat64 (1.0f));
    return p0 + p10.Scale(t);
  }

  dgBigVector p_p2(point - p2);
  dgFloat64 alpha5 = p10 % p_p2;
  dgFloat64 alpha6 = p20 % p_p2;
  if ((alpha6 >= dgFloat64(0.0f)) && (alpha5 <= alpha6))
  {
    return p2;
  }

  dgFloat64 vb = alpha5 * alpha2 - alpha1 * alpha6;
  if ((vb <= dgFloat64(0.0f)) && (alpha2 >= dgFloat64(0.0f))
      && (alpha6 <= dgFloat64(0.0f)))
  {
    dgFloat64 t = alpha2 / (alpha2 - alpha6);
    _ASSERTE(t >= dgFloat64 (0.0f));
    _ASSERTE(t <= dgFloat64 (1.0f));
    return p0 + p20.Scale(t);
  }

  dgFloat64 va = alpha3 * alpha6 - alpha5 * alpha4;
  if ((va <= dgFloat64(0.0f)) && ((alpha4 - alpha3) >= dgFloat64(0.0f))
      && ((alpha5 - alpha6) >= dgFloat64(0.0f)))
  {
    dgFloat64 t = (alpha4 - alpha3) / ((alpha4 - alpha3) + (alpha5 - alpha6));
    _ASSERTE(t >= dgFloat64 (0.0f));
    _ASSERTE(t <= dgFloat64 (1.0f));
    return p1 + (p2 - p1).Scale(t);
  }

  dgFloat64 den = float(dgFloat64(1.0f)) / (va + vb + vc);
  dgFloat64 t = vb * den;
  dgFloat64 s = vc * den;
  _ASSERTE(t >= dgFloat64 (0.0f));
  _ASSERTE(s >= dgFloat64 (0.0f));
  _ASSERTE(t <= dgFloat64 (1.0f));
  _ASSERTE(s <= dgFloat64 (1.0f));
  return p0 + p10.Scale(t) + p20.Scale(s);
}

bool dgApi dgPointToPolygonDistance(const dgVector& p,
    const dgFloat32* const polygon, dgInt32 strideInBytes,
    const dgInt32* const indexArray, dgInt32 indexCount, dgFloat32 bailDistance,
    dgVector& out)
{
//	dgInt32 i;
//	dgInt32 i0;
//	dgInt32 i1;
//	dgInt32 i2;
//	dgInt32 stride;
//	dgFloat32 dist;
//	dgFloat32 minDist;

  _ASSERTE(0);
  dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat32));

  dgInt32 i0 = indexArray[0] * stride;
  dgInt32 i1 = indexArray[1] * stride;

  const dgVector v0(&polygon[i0]);
  dgVector v1(&polygon[i1]);
  dgVector closestPoint(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
      dgFloat32(0.0f));
  dgFloat32 minDist = dgFloat32(1.0e20f);
  for (dgInt32 i = 2; i < indexCount; i++)
  {
    dgInt32 i2 = indexArray[i] * stride;
    const dgVector v2(&polygon[i2]);
    const dgVector q(dgPointToTriangleDistance(p, v0, v1, v2));
    const dgVector error(q - p);
    dgFloat32 dist = error % error;
    if (dist < minDist)
    {
      minDist = dist;
      closestPoint = q;
    }
    v1 = v2;
  }

  if (minDist > (bailDistance * bailDistance))
  {
    return false;
  }

  out = closestPoint;
  return true;
}

dgBigVector LineTriangleIntersection(const dgBigVector& p0,
    const dgBigVector& p1, const dgBigVector& A, const dgBigVector& B,
    const dgBigVector& C)
{
  dgHugeVector ph0(p0);
  dgHugeVector ph1(p1);
  dgHugeVector Ah(A);
  dgHugeVector Bh(B);
  dgHugeVector Ch(C);

  dgHugeVector p1p0(ph1 - ph0);
  dgHugeVector Ap0(Ah - ph0);
  dgHugeVector Bp0(Bh - ph0);
  dgHugeVector Cp0(Ch - ph0);

  dgGoogol t0((Bp0 * Cp0) % p1p0);
  dgFloat64 val0 = t0.GetAproximateValue();
  if (val0 < dgFloat64(0.0f))
  {
    return dgBigVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(-1.0f));
  }

  dgGoogol t1((Cp0 * Ap0) % p1p0);
  dgFloat64 val1 = t1.GetAproximateValue();
  if (val1 < dgFloat64(0.0f))
  {
    return dgBigVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(-1.0f));
  }

  dgGoogol t2((Ap0 * Bp0) % p1p0);
  dgFloat64 val2 = t2.GetAproximateValue();
  if (val2 < dgFloat64(0.0f))
  {
    return dgBigVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(-1.0f));
  }

  dgGoogol sum = t0 + t1 + t2;
  dgFloat64 den = sum.GetAproximateValue();

#ifdef _DEBUG
  dgBigVector testpoint(
      A.Scale(val0 / den) + B.Scale(val1 / den) + C.Scale(val2 / den));
  dgFloat64 volume = ((B - A) * (C - A)) % (testpoint - A);
  _ASSERTE(fabs (volume) < dgFloat64 (1.0e-12f));
#endif

  return dgBigVector(val0 / den, val1 / den, val2 / den, dgFloat32(0.0f));
}
