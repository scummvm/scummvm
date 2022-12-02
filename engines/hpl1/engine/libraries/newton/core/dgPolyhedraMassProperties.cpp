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
#include "dgVector.h"
#include "dgPolyhedraMassProperties.h"

#if 0
class dgPolyhedraMassProperties {
#define X 0
#define Y 1
#define Z 2
#define SQR(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

public:
	dgPolyhedraMassProperties() {
		memset(this, 0, sizeof(dgPolyhedraMassProperties));
	}

	void ProjectionIntegrals(
	    dgInt32 indexCount,
	    const dgVector *faceVertex) {
		dgInt32 i0;
		dgInt32 i1;
		dgFloat32 a0, a1, da;
		dgFloat32 b0, b1, db;

		dgFloat32 C1;
		dgFloat32 Ca;
		dgFloat32 Cb;
		dgFloat32 Caa;
		dgFloat32 Cbb;
		dgFloat32 Cab;
		dgFloat32 Kab;
		dgFloat32 a0_2;
		dgFloat32 a0_3;
		dgFloat32 a1_2;
		dgFloat32 b0_2;
		dgFloat32 b0_3;

		m_P1 = dgFloat32(0.0f);
		m_Pa = dgFloat32(0.0f);
		m_Pb = dgFloat32(0.0f);
		m_Paa = dgFloat32(0.0f);
		m_Pbb = dgFloat32(0.0f);
		m_Pab = dgFloat32(0.0f);

		i0 = indexCount - 1;
		for (i1 = 0; i1 < indexCount; i1 ++) {
			a0 = faceVertex[i0][m_A];
			b0 = faceVertex[i0][m_B];

			a1 = faceVertex[i1][m_A];
			b1 = faceVertex[i1][m_B];
			i0 = i1;

			da = a1 - a0;
			db = b1 - b0;

			a0_2 = a0 * a0;
			a0_3 = a0_2 * a0;

			b0_2 = b0 * b0;
			b0_3 = b0_2 * b0;

			a1_2 = a1 * a1;

			C1 = a1 + a0;
			Ca = a1 * C1 + a0_2;
			Caa = a1 * Ca + a0_3;

			Cb = b1 * (b1 + b0) + b0_2;
			Cbb = b1 * Cb + b0_3;

			Cab = dgFloat32(3.0f) * a1_2 + dgFloat32(2.0f) * a1 * a0 + a0_2;
			Kab = a1_2 + dgFloat32(2.0f) * a1 * a0 + dgFloat32(3.0f) * a0_2;

			m_P1 += db * C1;
			m_Pa += db * Ca;
			m_Paa += db * Caa;

			m_Pb += da * Cb;
			m_Pbb += da * Cbb;
			m_Pab += db * (b1 * Cab + b0 * Kab);
		}

		m_P1 *= dgFloat32(0.5f);
		m_Pa *= dgFloat32(1.0f / 6.0f);
		m_Pb *= dgFloat32(-1.0f / 6.0f);
		m_Paa *= dgFloat32(1.0f / 12.0f);
		m_Pbb *= dgFloat32(-1.0f / 12.0f);
		m_Pab *= dgFloat32(1.0f / 24.0f);
	}

	void FaceIntegrals(
	    dgInt32 count,
	    const dgPlane &plane,
	    const dgVector *faceVertex) {

		dgFloat32 k1, k2, k3, k4;

		ProjectionIntegrals(count, faceVertex);

		k1 = dgFloat32(1.0f) / plane[m_C];
		k2 = k1 * k1;
		k3 = k2 * k1;
		k4 = k3 * k1;

		m_Fa = k1 * m_Pa;
		m_Fb = k1 * m_Pb;
		m_Fc = -k2 * (plane[m_A] * m_Pa + plane[m_B] * m_Pb + plane[3] * m_P1);

		m_Faa = k1 * m_Paa;
		m_Fbb = k1 * m_Pbb;
		m_Fcc = k3 * (SQR(plane[m_A]) * m_Paa + dgFloat32(2.0f) * plane[m_A] * plane[m_B] * m_Pab +
		              SQR(plane[m_B]) * m_Pbb + plane[3] * (dgFloat32(2.0f) * (plane[m_A] * m_Pa + plane[m_B] * m_Pb) + plane[3] * m_P1));
	}

	void VolumeIntegrals(
	    dgInt32 indexCount,
	    const dgPlane &plane,
	    const dgVector *faceVertex) {
		dgFloat32 mag2;

		mag2 = plane % plane;
		if (mag2 > dgFloat32(1.0e-8f)) {
			if ((dgAbsf(plane.m_x) > dgAbsf(plane.m_y)) && (dgAbsf(plane.m_x) > dgAbsf(plane.m_z))) {
				m_C = X;
			} else {
				m_C = (dgAbsf(plane.m_y) > dgAbsf(plane.m_z)) ? Y : Z;
			}
			m_A = (m_C + 1) % 3;
			m_B = (m_A + 1) % 3;
			FaceIntegrals(indexCount, plane, faceVertex);

			m_T0 += plane[X] * ((m_A == X) ? m_Fa : ((m_B == X) ? m_Fb : m_Fc));

			m_T1[m_A] += plane[m_A] * m_Faa;
			m_T1[m_B] += plane[m_B] * m_Fbb;
			m_T1[m_C] += plane[m_C] * m_Fcc;
			//dgTrace (("(%f %f %f) (%f %f %f) (%f %f %f)\n", m_T1[m_A], m_T1[m_B], m_T1[m_C], plane[m_A], plane[m_B], plane[m_C], m_Faa, m_Fbb, m_Fcc))
		}
	}

	void AddInertia(int indexCount, const dgFloat32 *faceVertex) {
		dgInt32 i0;
		dgInt32 i1;
		dgFloat32 a0, a1, da;
		dgFloat32 b0, b1, db;
		dgFloat32 C1;
		dgFloat32 Ca;
		dgFloat32 Cb;
		dgFloat32 Caa;
		dgFloat32 Cbb;
		dgFloat32 Cab;
		dgFloat32 Kab;
		dgFloat32 Caaa;
		dgFloat32 Cbbb;
		dgFloat32 Cabb;
		dgFloat32 Caab;
		dgFloat32 Kabb;
		dgFloat32 Kaab;
		dgFloat32 a0_2;
		dgFloat32 a0_3;
		dgFloat32 a0_4;
		dgFloat32 a1_2;
		dgFloat32 a1_3;
		dgFloat32 b0_2;
		dgFloat32 b0_3;
		dgFloat32 b0_4;
		dgFloat32 b1_2;
		dgFloat32 b1_3;
		dgFloat32 mag2;

		dgVector p0(&faceVertex[0]);
		dgVector p1(&faceVertex[3]);
		dgVector p2(&faceVertex[6]);
		dgPlane plane(p0, p1, p2);

		mag2 = plane % plane;
		if (mag2 > dgFloat32(1.0e-8f)) {
			plane = plane.Scale(dgRsqrt((plane % plane)));
			if ((dgAbsf(plane.m_x) > dgAbsf(plane.m_y)) && (dgAbsf(plane.m_x) > dgAbsf(plane.m_z))) {
				m_C = X;
			} else {
				m_C = (dgAbsf(plane.m_y) > dgAbsf(plane.m_z)) ? Y : Z;
			}
			m_A = (m_C + 1) % 3;
			m_B = (m_A + 1) % 3;

			//    FaceIntegrals (indexCount, plane, faceVertex);
			dgFloat32 k1, k2, k3, k4;
			{
				{
					//ProjectionIntegrals (count, faceVertex);
					m_P1 = dgFloat32(0.0f);
					m_Pa = dgFloat32(0.0f);
					m_Pb = dgFloat32(0.0f);
					m_Paa = dgFloat32(0.0f);
					m_Pbb = dgFloat32(0.0f);
					m_Pab = dgFloat32(0.0f);

					m_Paaa = dgFloat32(0.0f);
					m_Pbbb = dgFloat32(0.0f);
					m_Paab = dgFloat32(0.0f);
					m_Pabb = dgFloat32(0.0f);

					i0 = indexCount - 1;
					for (i1 = 0; i1 < indexCount; i1 ++) {
						a0 = faceVertex[i0 * 3 + m_A];
						b0 = faceVertex[i0 * 3 + m_B];

						a1 = faceVertex[i1 * 3 + m_A];
						b1 = faceVertex[i1 * 3 + m_B];

						i0 = i1;

						da = a1 - a0;
						db = b1 - b0;

						a0_2 = a0 * a0;
						a0_3 = a0_2 * a0;
						a0_4 = a0_3 * a0;

						b0_2 = b0 * b0;
						b0_3 = b0_2 * b0;
						b0_4 = b0_3 * b0;

						a1_2 = a1 * a1;
						a1_3 = a1_2 * a1;

						b1_2 = b1 * b1;
						b1_3 = b1_2 * b1;

						C1 = a1 + a0;

						Ca = a1 * C1 + a0_2;
						Caa = a1 * Ca + a0_3;
						Caaa = a1 * Caa + a0_4;

						Cb = b1 * (b1 + b0) + b0_2;
						Cbb = b1 * Cb + b0_3;
						Cbbb = b1 * Cbb + b0_4;

						Cab = dgFloat32(3.0f) * a1_2 + dgFloat32(2.0f) * a1 * a0 + a0_2;
						Kab = a1_2 + dgFloat32(2.0f) * a1 * a0 + dgFloat32(3.0f) * a0_2;

						Caab = a0 * Cab + dgFloat32(4.0f) * a1_3;
						Kaab = a1 * Kab + dgFloat32(4.0f) * a0_3;
						Cabb = dgFloat32(4.0f) * b1_3 + dgFloat32(3.0f) * b1_2 * b0 + dgFloat32(2.0f) * b1 * b0_2 + b0_3;
						Kabb = b1_3 + dgFloat32(2.0f) * b1_2 * b0 + dgFloat32(3.0f) * b1 * b0_2 + dgFloat32(4.0f) * b0_3;

						m_P1 += (db * C1);
						m_Pa += (db * Ca);
						m_Paa += (db * Caa);

						m_Pb += (da * Cb);
						m_Pbb += (da * Cbb);
						m_Pab += (db * (b1 * Cab + b0 * Kab));

						m_Paaa += (db * Caaa);
						m_Pbbb += (da * Cbbb);
						m_Paab += (db * (b1 * Caab + b0 * Kaab));
						m_Pabb += (da * (a1 * Cabb + a0 * Kabb));
					}

					m_P1 *= dgFloat32(0.5f);
					m_Pa *= dgFloat32(1.0f / 6.0f);
					m_Pb *= dgFloat32(-1.0f / 6.0f);
					m_Paa *= dgFloat32(1.0f / 12.0f);
					m_Pbb *= dgFloat32(-1.0f / 12.0f);
					m_Pab *= dgFloat32(1.0f / 24.0f);

					m_Paaa *= dgFloat32(1.0f / 20.0);
					m_Pbbb *= dgFloat32(-1.0f / 20.0);
					m_Paab *= dgFloat32(1.0f / 60.0);
					m_Pabb *= dgFloat32(-1.0f / 60.0);
				}

				k1 = dgFloat32(1.0f) / plane[m_C];
				k2 = k1 * k1;
				k3 = k2 * k1;
				k4 = k3 * k1;

				m_Fa = k1 * m_Pa;
				m_Fb = k1 * m_Pb;
				m_Fc = -k2 * (plane[m_A] * m_Pa + plane[m_B] * m_Pb + plane[3] * m_P1);

				m_Faa = k1 * m_Paa;
				m_Fbb = k1 * m_Pbb;
				m_Fcc = k3 * (SQR(plane[m_A]) * m_Paa + dgFloat32(2.0f) * plane[m_A] * plane[m_B] * m_Pab +
				              SQR(plane[m_B]) * m_Pbb + plane[3] * (dgFloat32(2.0f) * (plane[m_A] * m_Pa + plane[m_B] * m_Pb) + plane[3] * m_P1));

				m_Faaa = k1 * m_Paaa;
				m_Fbbb = k1 * m_Pbbb;
				m_Fccc = -k4 * (CUBE(plane[m_A]) * m_Paaa +
				                dgFloat32(3.0f) * SQR(plane[m_A]) * plane[m_B] * m_Paab +
				                dgFloat32(3.0f) * plane[m_A] * SQR(plane[m_B]) * m_Pabb + CUBE(plane[m_B]) * m_Pbbb +
				                dgFloat32(3.0f) * plane[3] * (SQR(plane[m_A]) * m_Paa + dgFloat32(2.0f) * plane[m_A] * plane[m_B] * m_Pab + SQR(plane[m_B]) * m_Pbb) +
				                SQR(plane[3]) * (dgFloat32(3.0f) * (plane[m_A] * m_Pa + plane[m_B] * m_Pb) + plane[3] * m_P1));

				m_Faab = k1 * m_Paab;
				m_Fbbc = -k2 * (plane[m_A] * m_Pabb + plane[m_B] * m_Pbbb + plane[3] * m_Pbb);
				m_Fcca = k3 * (SQR(plane[m_A]) * m_Paaa + dgFloat32(2.0f) * plane[m_A] * plane[m_B] * m_Paab + SQR(plane[m_B]) * m_Pabb +
				               plane[3] * (dgFloat32(2.0f) * (plane[m_A] * m_Paa + plane[m_B] * m_Pab) + plane[3] * m_Pa));
			}

			m_T0 += (plane[X] * ((m_A == X) ? m_Fa : ((m_B == X) ? m_Fb : m_Fc)));

			m_T1[m_A] += (plane[m_A] * m_Faa);
			m_T1[m_B] += (plane[m_B] * m_Fbb);
			m_T1[m_C] += (plane[m_C] * m_Fcc);

			m_T2[m_A] += (plane[m_A] * m_Faaa);
			m_T2[m_B] += (plane[m_B] * m_Fbbb);
			m_T2[m_C] += (plane[m_C] * m_Fccc);

			m_TP[m_A] += (plane[m_A] * m_Faab);
			m_TP[m_B] += (plane[m_B] * m_Fbbc);
			m_TP[m_C] += (plane[m_C] * m_Fcca);
		}
	}

	dgInt32 m_A; // alpha
	dgInt32 m_B;// beta
	dgInt32 m_C;// gamma

	dgFloat32 m_T0;
	dgFloat32 m_T1[3];
	dgFloat32 m_T2[3];
	dgFloat32 m_TP[3];

	dgFloat32 m_P1;
	dgFloat32 m_Pa;
	dgFloat32 m_Pb;

	dgFloat32 m_Paa;
	dgFloat32 m_Pbb;
	dgFloat32 m_Pab;

	dgFloat32 m_Paaa;
	dgFloat32 m_Pbbb;
	dgFloat32 m_Paab;
	dgFloat32 m_Pabb;

	dgFloat32 m_Fa;
	dgFloat32 m_Fb;
	dgFloat32 m_Fc;

	dgFloat32 m_Faa;
	dgFloat32 m_Fbb;
	dgFloat32 m_Fcc;

	dgFloat32 m_Faaa;
	dgFloat32 m_Fbbb;
	dgFloat32 m_Fccc;

	dgFloat32 m_Faab;
	dgFloat32 m_Fbbc;
	dgFloat32 m_Fcca;
};
#endif

dgPolyhedraMassProperties::dgPolyhedraMassProperties() {
	memset(this, 0, sizeof(dgPolyhedraMassProperties));
}

void dgPolyhedraMassProperties::AddCGFace(dgInt32 indexCount,
        const dgVector *faceVertex) {
	dgVector f1;
	dgVector f2;
	dgFloat32 temp0;

#define CDSubexpressions(w0,w1,w2,f1,f2) \
	{                   \
		temp0 = w0 + w1; \
		f1 = temp0 + w2; \
		f2 = w0 * w0 + w1 * temp0 + w2 * f1; \
	}

	const dgVector &p0 = faceVertex[0];
	dgVector p1(faceVertex[1]);

	for (dgInt32 i = 2; i < indexCount; i++) {
		const dgVector &p2 = faceVertex[i];

		dgVector e01(p1 - p0);
		dgVector e02(p2 - p0);
		dgVector d(e01 * e02);

		CDSubexpressions(p0.m_x, p1.m_x, p2.m_x, f1.m_x, f2.m_x);
		CDSubexpressions(p0.m_y, p1.m_y, p2.m_y, f1.m_y, f2.m_y);
		CDSubexpressions(p0.m_z, p1.m_z, p2.m_z, f1.m_z, f2.m_z);

		// update integrals
		intg[0] += d[0] * f1.m_x;

		intg[1] += d[0] * f2.m_x;
		intg[2] += d[1] * f2.m_y;
		intg[3] += d[2] * f2.m_z;

		p1 = p2;
	}
}

void dgPolyhedraMassProperties::AddInertiaFace(dgInt32 indexCount,
        const dgFloat32 *faceVertex) {
	dgInt32 i;
	dgFloat32 temp0;
	dgFloat32 temp1;
	dgFloat32 temp2;
	dgVector f1;
	dgVector f2;
	dgVector f3;

#define InertiaSubexpression(w0,w1,w2,f1,f2,f3) \
	{                    \
		temp0 = w0 + w1; \
		temp1 = w0 * w0; \
		temp2 = temp1 + w1 * temp0; \
		f1 = temp0 + w2; \
		f2 = temp2 + w2 * f1;  \
		f3 = w0 * temp1 + w1 * temp2 + w2 * f2; \
	}

	dgVector p0(faceVertex[0], faceVertex[1], faceVertex[2], 0.0f);
	dgVector p1(faceVertex[3], faceVertex[4], faceVertex[5], 0.0f);

	for (i = 2; i < indexCount; i++) {
		dgVector p2(faceVertex[i * 3], faceVertex[i * 3 + 1], faceVertex[i * 3 + 2],
		            0.0f);

		dgVector e01(p1 - p0);
		dgVector e02(p2 - p0);
		dgVector d(e01 * e02);

		InertiaSubexpression(p0.m_x, p1.m_x, p2.m_x, f1.m_x, f2.m_x, f3.m_x);
		InertiaSubexpression(p0.m_y, p1.m_y, p2.m_y, f1.m_y, f2.m_y, f3.m_y);
		InertiaSubexpression(p0.m_z, p1.m_z, p2.m_z, f1.m_z, f2.m_z, f3.m_z);

		// update integrals
		intg[0] += d[0] * f1.m_x;

		intg[1] += d[0] * f2.m_x;
		intg[2] += d[1] * f2.m_y;
		intg[3] += d[2] * f2.m_z;

		intg[4] += d[0] * f3.m_x;
		intg[5] += d[1] * f3.m_y;
		intg[6] += d[2] * f3.m_z;

		p1 = p2;
	}
}

void dgPolyhedraMassProperties::AddInertiaAndCrossFace(dgInt32 indexCount,
        const dgFloat32 *faceVertex) {
	dgInt32 i;
	dgFloat32 temp0;
	dgFloat32 temp1;
	dgFloat32 temp2;
	dgVector f1;
	dgVector f2;
	dgVector f3;
	dgVector g0;
	dgVector g1;
	dgVector g2;

#define Subexpressions(w0,w1,w2,f1,f2,f3,g0,g1,g2) \
	{                                                  \
		temp0 = w0 + w1; \
		temp1 = w0 * w0; \
		temp2 = temp1 + w1 * temp0; \
		f1 = temp0 + w2; \
		f2 = temp2 + w2 * f1;  \
		f3 = w0 * temp1 + w1 * temp2 + w2 * f2; \
		g0 = f2 + w0 * (f1 + w0); \
		g1 = f2 + w1 * (f1 + w1); \
		g2 = f2 + w2 * (f1 + w2); \
	}

	dgVector p0(&faceVertex[0]);
	dgVector p1(&faceVertex[3]);

	for (i = 2; i < indexCount; i++) {
		dgVector p2(&faceVertex[i * 3]);

		dgVector e01(p1 - p0);
		dgVector e02(p2 - p0);
		dgVector d(e01 * e02);

		Subexpressions(p0.m_x, p1.m_x, p2.m_x, f1.m_x, f2.m_x, f3.m_x, g0.m_x,
		               g1.m_x, g2.m_x);
		Subexpressions(p0.m_y, p1.m_y, p2.m_y, f1.m_y, f2.m_y, f3.m_y, g0.m_y,
		               g1.m_y, g2.m_y);
		Subexpressions(p0.m_z, p1.m_z, p2.m_z, f1.m_z, f2.m_z, f3.m_z, g0.m_z,
		               g1.m_z, g2.m_z);

		// update integrals
		intg[0] += d[0] * f1.m_x;

		intg[1] += d[0] * f2.m_x;
		intg[2] += d[1] * f2.m_y;
		intg[3] += d[2] * f2.m_z;

		intg[4] += d[0] * f3.m_x;
		intg[5] += d[1] * f3.m_y;
		intg[6] += d[2] * f3.m_z;

		intg[7] += d[0] * (p0.m_y * g0.m_x + p1.m_y * g1.m_x + p2.m_y * g2.m_x);
		intg[8] += d[1] * (p0.m_z * g0.m_y + p1.m_z * g1.m_y + p2.m_z * g2.m_y);
		intg[9] += d[2] * (p0.m_x * g0.m_z + p1.m_x * g1.m_z + p2.m_x * g2.m_z);

		p1 = p2;
	}
}

dgFloat32 dgPolyhedraMassProperties::MassProperties(dgVector &cg,
        dgVector &inertia, dgVector &crossInertia) {
	dgFloat32 volume;

	for (dgInt32 i = 0; i < 10; i++) {
		intg[i] *= mult[i];
	}

	volume = intg[0];

	cg.m_x = intg[1];
	cg.m_y = intg[2];
	cg.m_z = intg[3];
	inertia.m_x = intg[5] + intg[6];
	inertia.m_y = intg[4] + intg[6];
	inertia.m_z = intg[4] + intg[5];
	inertia.m_w = dgFloat32(0.0f);
	crossInertia.m_x = -intg[8];
	crossInertia.m_y = -intg[9];
	crossInertia.m_z = -intg[7];
	crossInertia.m_w = dgFloat32(0.0f);
	return volume;
}

dgFloat32 dgPolyhedraMassProperties::mult[10] = {
	dgFloat32(1.0f / 6.0f), dgFloat32(1.0f / 24.0f), dgFloat32(1.0f / 24.0f),
	dgFloat32(1.0f / 24.0f), dgFloat32(1.0f / 60.0f), dgFloat32(1.0f / 60.0f),
	dgFloat32(1.0f / 60.0f), dgFloat32(1.0f / 120.0f), dgFloat32(1.0f / 120.0f),
	dgFloat32(1.0f / 120.0f)
};

