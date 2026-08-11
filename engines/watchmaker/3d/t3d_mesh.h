/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef WATCHMAKER_T3D_MESH_H
#define WATCHMAKER_T3D_MESH_H

#include "watchmaker/3d/t3d_face.h"
#include "watchmaker/t3d.h"

namespace Watchmaker {

struct t3dMESH {
	Common::String        name;          // mesh name
	Common::String        portalName;    // dest room name (if portal)
	uint16                NumFaces() {
		return FList.size();
	};                   // faces number
	uint16                NumVerts = 0;                   // verts number
	uint16                NumNormals = 0;                 // face normals number
	uint16                NumVerticesNormals = 0;         // verts normals number
	gVertex              *SavedVertexBuffer = nullptr;         // saved vertices pos (original)
	gVertex              *VertexBuffer = nullptr;              // cur vertices pos
	gVertex              *OldVertexBuffer = nullptr;           // last vertices pos
	t3dV3F               *VertsInterpolants = nullptr;         // verts interpolants (for blending)
	gVertex              *VBptr = nullptr;                     // temp pointer to vertexbuffer
	Common::Array<t3dMORPH> MorphFrames;               // morph frames list
	Common::Array<t3dFACE>  FList;                          // faces list
	NormalList            NList;                          // normals list
	t3dV3F                Pos;                            // mesh center
	t3dV3F                Trasl;                          // mesh traslation (world)
	t3dF32                Radius = 0.0f;                  // radius for Bounding Sphere culling
	t3dVERTEX             BBox[8] = {};                   // Bounding box
	t3dNORMAL             BBoxNormal[6] = {};             // bound box normals
	t3dF32                BBoxAverageZ = 0.0f;            // average distance from eye
	t3dV3F                Intersection;                   // intersecton form eye
	t3dM3X3F              Matrix;                         // transformation matrix (useful only for 1st mesh in body)
	uint16                LightmapDim = 0;                // lightmap texture dimensions
	Common::Array<t3dMODVERTS> ModVertices;         // mod vertices list
	t3dBONEANIM           DefaultAnim;                    // Default Animations
	t3dBONEANIM           Anim;                           // Animations
	int16                CurFrame = 0;                   // current animation frames (0 no anim)
	int16                LastFrame = 0;                  // last animation frames
	uint8                 BlendPercent = 0;               // blend animation percentage
	uint8                 LastBlendPercent = 0;           // last blend animation percentage
	uint32                ExpressionFrame = 0;            // current expression frames
	uint32                LastExpressionFrame = 0;        // last expression frames
	int32				  *WaterBuffer1 = nullptr;	  // pointers to ripple buffer
	int32 				  *WaterBuffer2 = nullptr;	  // pointers to ripple buffer
	t3dF32                WavesSpeed;                 // waves speed
	t3dF32                YSpeed;                     // waves y speed
	t3dF32                XInc, YInc;                 // waves movements

	t3dV3F                SolarRGBVar[4];             // Override ambient color variation for solar movement

	uint32                Flags;                      // Flags
	t3dBODY              *PortalList = nullptr;       // Pointer to portal connected
	Common::Array<t3dMESH *> RejectedMeshes;      // rejected mesh from portal

	void saveVertexBuffer() { // Scorre le mesh
		this->VBptr = this->VertexBuffer;
		if (this->OldVertexBuffer)
			memcpy(this->OldVertexBuffer, this->VBptr, sizeof(gVertex)*this->NumVerts);
		if (this->SavedVertexBuffer)
			memcpy(this->SavedVertexBuffer, this->VBptr, sizeof(gVertex)*this->NumVerts);
		this->VBptr = nullptr;
	}

	void preCalcLights(const t3dV3F &ambientLight) {
#ifndef WMGEN
		this->VBptr = this->VertexBuffer;
#endif
		for (uint16 j = 0; j < this->NumFaces(); j++) {                                          // Scorre le facce
			t3dFACE &Face = this->FList[j];
			MaterialPtr Material = Face.getMaterial();
			uint32      alphaval = 2;
			uint32      rr = 0, gg = 0, bb = 0;

			rr = Material->r;                                                                   // Prende Ambient da materiale
			gg = Material->g;
			bb = Material->b;

			if (Material->hasFlag(T3D_MATERIAL_CLIPMAP)) {                                       // Se il materiale e' clipmap
				alphaval = 0xfe;
				Face.flags |= T3D_MATERIAL_CLIPMAP;                                            // lo setta sulla faccia
				/*              Face->flags&=~T3D_MATERIAL_OPACITY;
				                Face->flags&=~T3D_MATERIAL_GLASS;
				                Material->Flags&=~T3D_MATERIAL_OPACITY;
				                Material->Flags&=~T3D_MATERIAL_GLASS;*/
//				r=g=b=0;
			}
			if (Material->hasFlag(T3D_MATERIAL_OPACITY)) {                                       // Se il materiale e' opacity
				Face.flags |= T3D_MATERIAL_OPACITY;                                            // lo setta sulla faccia
				alphaval = 0x88;
				rr = gg = bb = 0;
			}
			if (Material->hasFlag(T3D_MATERIAL_GLASS)) {                                         // Se e' un glass
				Face.flags |= T3D_MATERIAL_GLASS;                                              // lo setta sulla faccia
				alphaval = 0xfe;
				rr = gg = bb = 255;
			}
			if (Material->hasFlag(T3D_MATERIAL_BOTTLE)) {                                        // Se e' un bottle
				Face.flags |= T3D_MATERIAL_BOTTLE;                                             // sulla faccia
				alphaval = 0x88;
				rr = gg = bb = 255;
			}
			if (Material->hasFlag(T3D_MATERIAL_ADDITIVE)) {                                      // Se e' un additivo
				Face.flags |= T3D_MATERIAL_ADDITIVE;                                           // sulla faccia
				alphaval = 0x88;
				rr = gg = bb = 255;
			}
			if (rr < ambientLight.x) rr = (uint8)ambientLight.x;
			if (gg < ambientLight.y) gg = (uint8)ambientLight.y;
			if (bb < ambientLight.z) bb = (uint8)ambientLight.z;
			this->VBptr[Face.VertexIndex[0]].diffuse = RGBA_MAKE(rr, gg, bb, alphaval);        // Cambia diffse dei vertici della mesh
			this->VBptr[Face.VertexIndex[1]].diffuse = RGBA_MAKE(rr, gg, bb, alphaval);
			this->VBptr[Face.VertexIndex[2]].diffuse = RGBA_MAKE(rr, gg, bb, alphaval);
		}
#ifndef WMGEN
		this->VBptr = nullptr;
#endif
//		rOptimizeVertexArray(Mesh->VertexBuffer);
	}

	t3dMESH() = default;
	t3dMESH(t3dBODY *b, Common::SeekableReadStream &stream, t3dMESH *&ReceiveRipples, uint8 &Mirror);
	t3dMESH(const t3dMESH &other);
	t3dMESH(t3dMESH &&old);
	t3dMESH& operator=(t3dMESH rhs);
	~t3dMESH();
	void loadFaces(t3dBODY *b, Common::SeekableReadStream &stream, int numFaces);
	void release();
	void releaseAnim(uint8 flag);

	bool hasFaceMaterial() const {
		return !this->FList.empty() && (bool)(this->FList[0].getMaterial());
	}

	void setMovieFrame(uint32 dwCurrFrame) {
		// TODO: Could just inline rSetMovieFrame?
		if (!this->FList.empty() && this->FList[0].getMaterial()) {
			rSetMovieFrame(this->FList[0].getMaterial(), dwCurrFrame);
		}
	}
	uint32 getMovieFrame() {
		assert(!this->FList.empty() && this->FList[0].getMaterial());
		return rGetMovieFrame(this->FList[0].getMaterial());
	}
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_T3D_MESH_H
