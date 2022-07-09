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

#include "dgMeshEffectSolidTree.h"
#include "dgMeshEffect.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


#define DG_CLIPPER_TOL dgFloat64(1.0e-12)

dgMeshTreeCSGFace::dgMeshTreeCSGFace(const dgMeshEffect &mesh,
									 dgEdge *const face) : dgList<dgHugeVector>(mesh.GetAllocator()), m_side(dgMeshEffectSolidTree::m_divider) {
	const dgEdge *ptr = face;
	const dgMeshEffect::dgVertexAtribute *const attib = mesh.m_attib;
	do {
		Append(attib[ptr->m_userData].m_vertex);
		ptr = ptr->m_next;
	} while (ptr != face);
}

dgMeshTreeCSGFace::dgMeshTreeCSGFace(dgMemoryAllocator *const allocator,
									 dgInt32 count, const dgHugeVector *const points) : dgList<dgHugeVector>(allocator), m_side(dgMeshEffectSolidTree::m_divider) {
	for (dgInt32 i = 0; i < count; i++) {
		Append(points[i]);
	}
}

bool dgMeshTreeCSGFace::IsPointOnEdge(const dgHugeVector &p0,
									  const dgHugeVector &p1, const dgHugeVector &q0) const {
	dgHugeVector p1p0(p1 - p0);
	dgGoogol den(p1p0 % p1p0);

	dgHugeVector q0p0(q0 - p0);
	dgGoogol num(q0p0 % p1p0);

	dgFloat64 numf = num.GetAproximateValue();
	if (numf > (DG_CLIPPER_TOL * dgFloat64(1.0e3))) {
		if (numf < den.GetAproximateValue() * dgFloat64(1.0 - (DG_CLIPPER_TOL * dgFloat64(1.0e3)))) {
			dgGoogol t = num / den;
			_ASSERTE(t.GetAproximateValue() > dgFloat64(0.0f));
			_ASSERTE(t.GetAproximateValue() < dgFloat64(1.0f));
			dgHugeVector q1(p0 + p1p0.Scale(t));
			dgHugeVector dist(q1 - q0);
			dgGoogol dist2 = dist % dist;
			if (dist2.GetAproximateValue() < DG_CLIPPER_TOL) {
				return true;
			}
		}
	}
	return false;
}

bool dgMeshTreeCSGFace::CheckConvex(const dgHugeVector &normal) const {
	dgHugeVector p1(GetLast()->GetInfo());
	dgHugeVector p0(GetLast()->GetPrev()->GetInfo());
	dgHugeVector e0(p0 - p1);
	for (dgListNode *node = GetFirst(); node; node = node->GetNext()) {
		dgHugeVector p2(node->GetInfo());
		dgHugeVector e1(p2 - p1);

		dgHugeVector n(e1 * e0);
		dgGoogol convex = n % normal;
		if (convex.GetAproximateValue() < dgFloat64(-1.0e10f)) {
			return false;
		}
		p1 = p2;
		e0 = e1.Scale(dgGoogol(-1.0f));
	}
	return true;
};

void dgMeshTreeCSGFace::MergeMissingVertex(const dgMeshTreeCSGFace *const face) {
	for (dgMeshTreeCSGFace::dgListNode *outNode = GetFirst();
		 outNode != GetLast(); outNode = outNode->GetNext()) {
		dgHugeVector p0(outNode->GetInfo());
		for (dgMeshTreeCSGFace::dgListNode *node = face->GetFirst(); node; node =
																			   node->GetNext()) {
			if (IsPointOnEdge(p0, outNode->GetNext()->GetInfo(), node->GetInfo())) {
				dgMeshTreeCSGFace::dgListNode *const insertNode = Append(
					node->GetInfo());
				InsertAfter(outNode, insertNode);
			}
		}
	}

	RotateToBegin(GetLast());
	dgListNode *const last = GetFirst()->GetNext();
	for (dgMeshTreeCSGFace::dgListNode *outNode = GetFirst(); outNode != last;
		 outNode = outNode->GetNext()) {
		dgHugeVector p0(outNode->GetInfo());
		dgHugeVector p1(outNode->GetNext()->GetInfo());

		for (dgMeshTreeCSGFace::dgListNode *node = face->GetFirst(); node; node =
																			   node->GetNext()) {
			if (IsPointOnEdge(p0, outNode->GetNext()->GetInfo(), node->GetInfo())) {
				dgMeshTreeCSGFace::dgListNode *const insertNode = Append(
					node->GetInfo());
				InsertAfter(outNode, insertNode);
			}
		}
	}
}

dgInt32 dgMeshTreeCSGFace::RemoveDuplicates(dgInt32 count,
											dgHugeVector *const points) const {
	dgInt32 index[256];
	for (dgInt32 i = 0; i < count; i++) {
		index[i] = i + 1;
	}
	index[count - 1] = 0;

	dgInt32 originalCount = count;
	dgInt32 start = index[0];
	for (dgInt32 count1 = 0; (count1 != count) && (count >= 3);) {
		count1 = 0;
		for (dgInt32 i = 0; i < count; i++) {
			dgInt32 next = index[start];
			dgHugeVector err(points[start] - points[next]);
			dgGoogol dist2 = err % err;
			dgFloat64 val = dist2.GetAproximateValue();
			if (val < dgFloat64(1.0e-12f)) {
				index[start] = index[index[start]];
				count1 = 0;
				count--;
				break;
			}

			count1++;
			start = index[start];
		}
	}

	if ((count != originalCount) && (count >= 3)) {
		dgHugeVector tmp[256];
		for (dgInt32 i = 0; i < count; i++) {
			tmp[i] = points[start];
			start = index[start];
		}

		for (dgInt32 i = 0; i < count; i++) {
			points[i] = tmp[i];
		}
	}

	return count;
}

#ifdef _DEBUG
dgMatrix dgMeshTreeCSGFace::DebugMatrix() const {
	dgMatrix matrix(dgGetIdentityMatrix());
	dgHugeVector e0(GetFirst()->GetNext()->GetInfo() - GetFirst()->GetInfo());
	dgHugeVector e1(
		GetFirst()->GetNext()->GetNext()->GetInfo() - GetFirst()->GetInfo());
	dgHugeVector aaa(e0 * e1);
	dgVector aaaa(dgFloat32(aaa.m_x.GetAproximateValue()),
				  dgFloat32(aaa.m_y.GetAproximateValue()),
				  dgFloat32(aaa.m_z.GetAproximateValue()), 0.0f);
	aaaa = aaaa.Scale(dgFloat32(1.0) / dgSqrt(aaaa % aaaa));

	dgVector bbbb(dgFloat32(e0.m_x.GetAproximateValue()),
				  dgFloat32(e0.m_y.GetAproximateValue()),
				  dgFloat32(e0.m_z.GetAproximateValue()), 0.0f);
	bbbb = bbbb.Scale(dgFloat32(1.0) / dgSqrt(bbbb % bbbb));

	matrix.m_up = bbbb;
	matrix.m_right = aaaa;
	matrix.m_front = bbbb * aaaa;

	return matrix;
}

void dgMeshTreeCSGFace::Trace(const dgMatrix &matrix) const {
	for (dgMeshTreeCSGFace::dgListNode *node = GetFirst(); node;
		 node = node->GetNext()) {
		const dgHugeVector &bbb = node->GetInfo();
		dgVector bbbb(dgFloat32(bbb.m_x.GetAproximateValue()),
					  dgFloat32(bbb.m_y.GetAproximateValue()),
					  dgFloat32(bbb.m_z.GetAproximateValue()), 0.0f);
		bbbb = matrix.UnrotateVector(bbbb);
		bbbb.Trace();
	}
	dgTrace(("\n"));
}

#endif

dgHugeVector dgMeshTreeCSGFace::FaceNormal() const {
	dgHugeVector area(0.0f, 0.0f, 0.0f, 0.0f);
	dgHugeVector e0(GetFirst()->GetNext()->GetInfo() - GetFirst()->GetInfo());
	for (dgListNode *node = GetFirst()->GetNext()->GetNext(); node;
		 node = node->GetNext()) {
		dgHugeVector e1(node->GetInfo() - GetFirst()->GetInfo());
		area += e0 * e1;
		e0 = e1;
	}
	return area;
}

bool dgMeshTreeCSGFace::CheckFaceArea(dgInt32 count,
									  const dgHugeVector *const points) const {
	dgHugeVector area(dgFloat64(0.0), dgFloat64(0.0), dgFloat64(0.0),
					  dgFloat64(0.0));
	dgHugeVector e0(points[1] - points[0]);
	for (dgInt32 i = 2; i < count; i++) {
		dgHugeVector e1(points[i] - points[0]);
		area += e0 * e1;
		e0 = e1;
	}

	dgFloat64 val = (area % area).GetAproximateValue();
	return (val > dgFloat64(1.0e-12f)) ? true : false;
}

/*
 //dgGoogol xxx1(dgGoogol v)
 dgGoogol xxx1(dgInt32 x)
 {
 dgGoogol old (0);
 dgGoogol ans (1);
 while (x > 1) {
 dgGoogol n (ans + old);
 old = ans;
 ans = n;
 x = x - 1;
 }
 return ans;
 }

 dgGoogol xxx2(dgInt32 x)
 {
 dgGoogol one (1);
 dgGoogol n (x);
 dgGoogol a (1);
 for (int i = x; i >= 1; i --) {
 a = a * n;
 n -= one;
 }
 return a;
 }

 void xxxx()
 {
 char text[256];
 //	dgGoogol a (xxx1(100));
 //	dgGoogol a (1.05);
 dgGoogol a (xxx2 (60));
 a.ToString(text);
 //	dgFloat64 xxxxx = a.GetAproximateValue();
 }
 */

void dgMeshTreeCSGFace::Clip(const dgHugeVector &plane,
							 dgMeshTreeCSGFace **leftOut, dgMeshTreeCSGFace **rightOut) {
	// xxxx ();

	dgInt8 pointSide[256];

	dgInt32 count = 0;
	dgInt32 rightCount = 0;
	dgInt32 leftCount = 0;
	for (dgMeshTreeCSGFace::dgListNode *ptr = GetFirst(); ptr; ptr =
																   ptr->GetNext()) {
		const dgHugeVector &p = ptr->GetInfo();

		dgGoogol test = plane.EvaluePlane(p);
		dgFloat64 val = test.GetAproximateValue();
		if (fabs(val) < DG_CLIPPER_TOL) {
			val = dgFloat64(0.0f);
		}

		if (val > dgFloat64(0.0f)) {
			pointSide[count] = 1;
			rightCount++;
		} else if (val < dgFloat64(0.0f)) {
			pointSide[count] = -1;
			leftCount++;
		} else {
			pointSide[count] = 0;
		}
		count++;
	}

	*leftOut = NULL;
	*rightOut = NULL;
	if ((leftCount && !rightCount) || (!leftCount && rightCount)) {
		if (leftCount) {
			_ASSERTE(!rightCount);
			AddRef();
			*leftOut = this;
		} else {
			_ASSERTE(!leftCount);
			*rightOut = this;
			AddRef();
		}
	} else if (!(leftCount || rightCount)) {
		_ASSERTE(!leftCount);
		_ASSERTE(!rightCount);
		AddRef();
		// AddRef();
	} else {
		dgInt32 leftCount = 0;
		dgInt32 rightCount = 0;
		dgHugeVector leftFace[256];
		dgHugeVector rightFace[256];

		dgInt32 i1 = 0;
		dgInt32 i0 = count - 1;
		dgHugeVector p0(GetLast()->GetInfo());
		for (dgMeshTreeCSGFace::dgListNode *ptr = GetFirst(); ptr;
			 ptr = ptr->GetNext()) {
			const dgHugeVector &p1(ptr->GetInfo());

			dgHugeVector inter;
			if (((pointSide[i0] == -1) && (pointSide[i1] == 1)) || ((pointSide[i0] == 1) && (pointSide[i1] == -1))) {
				// inter = Interpolate (plane, p0, p1);
				dgHugeVector dp(p1 - p0);
				dgGoogol den(plane % dp);
				dgGoogol num = plane.EvaluePlane(p0);
				_ASSERTE(fabs(num.GetAproximateValue()) > dgFloat64(0.0f));
				dgGoogol ti(num / den);
				inter = p0 - dp.Scale(num / den);
			}

			if (pointSide[i1] == -1) {
				if (pointSide[i0] == 1) {
					rightFace[rightCount] = inter;
					rightCount++;
				}
			} else {
				if ((pointSide[i1] == 1) && (pointSide[i0] == -1)) {
					rightFace[rightCount] = inter;
					rightCount++;
				}
				rightFace[rightCount] = p1;
				rightCount++;
			}

			if (pointSide[i1] == 1) {
				if (pointSide[i0] == -1) {
					leftFace[leftCount] = inter;
					leftCount++;
				}
			} else {
				if ((pointSide[i1] == -1) && (pointSide[i0] == 1)) {
					leftFace[leftCount] = inter;
					leftCount++;
				}
				leftFace[leftCount] = p1;
				leftCount++;
			}
			_ASSERTE(leftCount < dgInt32((sizeof(leftFace) / sizeof(leftFace[0])) - 1));
			_ASSERTE(rightCount < dgInt32((sizeof(rightFace) / sizeof(rightFace[0])) - 1));

			i0 = i1;
			i1++;
			p0 = p1;
		}

		leftCount = RemoveDuplicates(leftCount, leftFace);
		rightCount = RemoveDuplicates(rightCount, rightFace);
		if ((leftCount >= 3) && CheckFaceArea(leftCount, leftFace)) {
			*leftOut = new (GetAllocator()) dgMeshTreeCSGFace(GetAllocator(),
															  leftCount, leftFace);
		}

		if ((rightCount >= 3) && CheckFaceArea(rightCount, rightFace)) {
			*rightOut = new (GetAllocator()) dgMeshTreeCSGFace(GetAllocator(),
															   rightCount, rightFace);
		}
		_ASSERTE(*leftOut || *rightOut);
	}
}

dgMeshEffectSolidTree::dgMeshEffectSolidTree(dgPlaneType type) : m_planeType(type), m_back(NULL), m_front(NULL), m_plane(0.0, 0.0, 0.0, 0.0) {
}

dgMeshEffectSolidTree::dgMeshEffectSolidTree(const dgMeshEffect &mesh,
											 dgEdge *const face) : m_planeType(m_divider), m_back(new (mesh.GetAllocator()) dgMeshEffectSolidTree(m_solid)), m_front(new (mesh.GetAllocator()) dgMeshEffectSolidTree(m_empty)), m_plane(BuildPlane(mesh, face)) {
}

dgMeshEffectSolidTree::dgMeshEffectSolidTree(const dgHugeVector &plane,
											 dgMemoryAllocator *const allocator) : m_planeType(m_divider), m_back(new (allocator) dgMeshEffectSolidTree(m_solid)), m_front(new (allocator) dgMeshEffectSolidTree(m_empty)), m_plane(plane) {
}

dgMeshEffectSolidTree::~dgMeshEffectSolidTree() {
	if (m_front) {
		delete m_front;
	}

	if (m_back) {
		delete m_back;
	}
}

dgHugeVector dgMeshEffectSolidTree::BuildPlane(const dgMeshEffect &mesh,
											   dgEdge *const face) const {
	dgEdge *edge = face;
	dgHugeVector plane(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					   dgFloat32(0.0f));

	dgHugeVector p0(mesh.m_points[edge->m_incidentVertex]);

	edge = edge->m_next;
	dgHugeVector p1(mesh.m_points[edge->m_incidentVertex]);

	dgHugeVector e1(p1 - p0);
	for (edge = edge->m_next; edge != face; edge = edge->m_next) {
		dgHugeVector p2(mesh.m_points[edge->m_incidentVertex]);
		dgHugeVector e2(p2 - p0);
		plane += e1 * e2;
		e1 = e2;
	}

	plane.m_w = (p0 % plane) * dgGoogol(-1.0);
	return plane;
}

void dgMeshEffectSolidTree::AddFace(const dgMeshEffect &mesh,
									dgEdge *const face) {
	dgBigVector normal(
		mesh.FaceNormal(face, &mesh.m_points[0][0], sizeof(dgBigVector)));
	dgFloat64 mag2 = normal % normal;

	if (mag2 > dgFloat32(1.0e-14f)) {
		dgMeshTreeCSGFace *faces[DG_MESH_EFFECT_BOLLEAN_STACK];
		dgMeshEffectSolidTree *pool[DG_MESH_EFFECT_BOLLEAN_STACK];

		dgHugeVector plane(BuildPlane(mesh, face));

		dgInt32 stack = 1;
		pool[0] = this;
		faces[0] = new (mesh.GetAllocator()) dgMeshTreeCSGFace(mesh, face);

		while (stack) {

			stack--;
			dgMeshEffectSolidTree *const root = pool[stack];
			_ASSERTE(root->m_planeType == m_divider);

			dgMeshTreeCSGFace *const curve = faces[stack];
			_ASSERTE(curve->CheckConvex(plane));

			dgMeshTreeCSGFace *backOut;
			dgMeshTreeCSGFace *frontOut;
			curve->Clip(root->m_plane, &backOut, &frontOut);

			if ((backOut == NULL) && (frontOut == NULL)) {
				curve->Release();
			} else {
				if (backOut && frontOut) {
					dgHugeVector backArea(backOut->FaceNormal());
					dgHugeVector frontArea(frontOut->FaceNormal());

					dgFloat64 backMag = (backArea % backArea).GetAproximateValue();
					dgFloat64 frontMag = (frontArea % frontArea).GetAproximateValue();
					if (backMag > frontMag) {
						if (backMag > (frontMag * dgFloat64(1.0e6))) {
							frontOut->Release();
							frontOut = NULL;
						}
					} else {
						if (frontMag > (backMag * dgFloat64(1.0e6))) {
							backOut->Release();
							backOut = NULL;
						}
					}
				}

				if (backOut) {
					if (root->m_back->m_planeType != m_divider) {
						backOut->Release();
						delete root->m_back;
						root->m_back = new (mesh.GetAllocator()) dgMeshEffectSolidTree(
							plane, mesh.GetAllocator());
					} else {
						faces[stack] = backOut;
						pool[stack] = root->m_back;
						stack++;
						_ASSERTE(stack < dgInt32(sizeof(pool) / sizeof(pool[0])));
					}
				}

				if (frontOut) {
					if (root->m_front->m_planeType != m_divider) {
						frontOut->Release();
						delete root->m_front;
						root->m_front = new (mesh.GetAllocator()) dgMeshEffectSolidTree(
							plane, mesh.GetAllocator());
					} else {
						faces[stack] = frontOut;
						pool[stack] = root->m_front;
						stack++;
						_ASSERTE(stack < dgInt32(sizeof(pool) / sizeof(pool[0])));
					}
				}
			}

			curve->Release();
		}
	}
}

dgMeshEffectSolidTree::dgPlaneType dgMeshEffectSolidTree::GetPointSide(
	const dgHugeVector &point) const {
	const dgMeshEffectSolidTree *root = this;

	_ASSERTE(root);
	while (root->m_planeType == dgMeshEffectSolidTree::m_divider) {
		dgGoogol test = root->m_plane.EvaluePlane(point);
		dgFloat64 dist = test.GetAproximateValue();

		if (fabs(dist) < dgFloat64(1.0e-16f)) {
			dgPlaneType isBackSide = root->m_back->GetPointSide(point);
			dgPlaneType isFrontSide = root->m_front->GetPointSide(point);
			return (isBackSide == isFrontSide) ? isFrontSide : m_divider;

		} else if (dist > dgFloat64(0.0f)) {
			root = root->m_front;
		} else {
			_ASSERTE(dist < dgFloat64(0.0f));
			root = root->m_back;
		}
	}
	return root->m_planeType;
}

dgMeshEffectSolidTree::dgPlaneType dgMeshEffectSolidTree::GetFaceSide(
	const dgMeshTreeCSGFace *const face) const {
	dgHugeVector center(0.0, 0.0, 0.0, 0.0);
	for (dgMeshTreeCSGFace::dgListNode *node = face->GetFirst(); node;
		 node = node->GetNext()) {
		const dgHugeVector &point = node->GetInfo();
		center += point;
	}
	center = center.Scale(dgGoogol(1.0) / dgGoogol(face->GetCount()));
	dgPlaneType faceSide(GetPointSide(center));
	return faceSide;
}
