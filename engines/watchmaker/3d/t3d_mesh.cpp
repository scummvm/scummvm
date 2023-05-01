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

#include "watchmaker/3d/t3d_mesh.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/3d/geometry.h"

namespace Watchmaker {

void t3dMESH::loadFaces(t3dBODY *b, Common::SeekableReadStream &stream, int numFaces) {
	//Mesh[mesh].FList = new t3dFACE[Mesh[mesh].NumFaces]{};                  // Alloca facce

	this->FList.reserve(numFaces); // Legge numero facce mesh
	for (uint16 face = 0; face < numFaces; face++) {
		FList.push_back(t3dFACE(b, stream));
	}//__for_face
}

t3dMESH::t3dMESH(t3dBODY *b, Common::SeekableReadStream &stream, t3dMESH *&ReceiveRipples, uint8 &Mirror) {
	t3dMatIdentity(&Matrix);                                                     // Setta matrice identica

	this->DefaultAnim.NumBones = this->DefaultAnim.NumFrames = 0;
	this->Anim.NumBones = this->Anim.NumFrames = 0;

	char stringBuffer[T3D_NAMELEN + 1] = {};
	stream.read(stringBuffer, T3D_NAMELEN); // Legge nome mesh
	this->name = stringBuffer;
	stream.read(stringBuffer, T3D_NAMELEN); // Legge nome portale
	this->portalName = stringBuffer;

	int numFaces = stream.readSint16LE(); // Legge numero facce mesh

	t3dVectFill(&this->Trasl, 0.0f);
	this->Pos = t3dV3F(stream) * SCALEFACTOR;
	this->Radius = stream.readFloatLE() * SCALEFACTOR;                                        // Legge raggio boundsphere

	//this->LightmapDim=(t3dU16)t3dRead8();
	uint8 SaveMipStatus = stream.readByte();                                                             // Legge dimensione lightmap
	this->LightmapDim = SaveMipStatus;
	if (this->LightmapDim == 255)
		this->LightmapDim = 256;
	if (this->LightmapDim == 0) {                                                      // Se non e' specificata
		if ((this->name[0] == 'o') || (this->name[0] == 'O'))                  // decide in base al nome
			this->LightmapDim = 8;
		else if ((this->name[0] == 'p') || (this->name[0] == 'P'))
			this->LightmapDim = 16;
	}
	if (this->LightmapDim > 256) {                                                     // Se > 256, errore
		this->LightmapDim = 256;
		warning("ATTENTION: Lightmap dim >256 on mesh %s!!!", this->name.c_str());
		warning("Check and verify .t3d");
	}

	this->Flags = stream.readSint32LE();                                                         // Legge flags
//		if ( !strcasecmp( "pxt-musoleoBUCO", this->Name ) );
	if (this->Flags & T3D_MESH_MIRROR)                                                 // Incrementa numero mirror
		Mirror++;
//		if( this->Flags&T3D_MESH_PORTAL)
//			this->Flags|=T3D_MESH_PORTAL;
	if ((this->Flags & T3D_MESH_RECEIVERIPPLES) ||                                      // Aggiunge buffer per le onde
	        (this->Flags & T3D_MESH_POOLWATER))
		ReceiveRipples = this;
	if (this->Flags & T3D_MESH_WAVESTEXTURE) {                                         // Legge informazioni sulle onde
		this->WavesSpeed = (t3dF32)stream.readSint32LE() / 10000.0f;
		this->YSpeed = (t3dF32)stream.readSint32LE() / 100.0f;
	}
	if (this->Flags & T3D_MESH_SOLARVARIATION) {                                           // Legge informazioni sulla variazione solare
		this->SolarRGBVar[0].x = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[0].y = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[0].z = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[1].x = 1.0f;
		this->SolarRGBVar[1].y = 1.0f;
		this->SolarRGBVar[1].z = 1.0f;
		this->SolarRGBVar[2].x = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[2].y = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[2].z = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[3].x = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[3].y = (t3dF32)stream.readSint16LE() / 100.0f;
		this->SolarRGBVar[3].z = (t3dF32)stream.readSint16LE() / 100.0f;
	}
	{
		uint16 n = stream.readSint32LE();
		if (n)
			this->XInc = 1.0f / (t3dF32) n;                                // Legge info su movimenti texture
		else this->XInc = 0.0f;
		n = stream.readSint32LE();
		if (n) this->YInc = 1.0f / (t3dF32) n;
		else this->YInc = 0.0f;
	}
	this->CurFrame = 0;

	this->loadFaces(b, stream, numFaces);

	this->NumVerts = stream.readSint16LE();                                                      // Rilegge numero vertici
	b->NumTotVerts += this->NumVerts;

#ifndef WMGEN
	this->VertexBuffer = new gVertex[this->NumVerts]();  // Crea un VertexBuffer
	this->VBptr = this->VertexBuffer;
#else
	this->VBptr = (gVertex *)t3dMalloc(sizeof(gVertex) * this->NumVerts);
#endif
	for (uint16 i = 0; i < this->NumVerts; i++) {
		this->VBptr[i].x = stream.readFloatLE();                                              // Legge X
		this->VBptr[i].y = stream.readFloatLE();                                              // Legge Y
		this->VBptr[i].z = stream.readFloatLE();                                              // Legge Z
		this->VBptr[i].u1 = stream.readFloatLE();                                             // Legge U
		this->VBptr[i].v1 = stream.readFloatLE();                                             // Legge V
		uint16 n = stream.readSint16LE();                                                          // Legge indice normale
		this->VBptr[i].diffuse = n; //temporary storage for normal index
	}
#ifndef WMGEN
	this->VBptr = nullptr;
#endif
	this->NList = b->NList;                                                            // Lista di normali e' quella globale

	int numMorphFrames = stream.readSint16LE();                                                // Legge frame di espressioni
	if (numMorphFrames) {                                                        // Se frame di espressioni
		this->MorphFrames.reserve(numMorphFrames); // Alloca spazio
		for (int i = 0; i < numMorphFrames; i++) {
			this->MorphFrames.push_back(t3dMORPH(stream));
		}
	}
//	Bounding box vertices
//
//  4       5
//  *-------*
//0*-----1*/|
// | 6    | |
// |/*----+-*7
// *------*/
//2      3
	for (uint16 normal = 0; normal < 8; normal++) {                                                // Legge BoundingBox
		this->BBox[normal].p = t3dV3F(stream) * SCALEFACTOR;
	}//__for_normal
	if ((this->BBox[0].p == this->BBox[4].p) &&                        // Se non ha spessore
	        (this->BBox[1].p == this->BBox[5].p) &&
	        (this->BBox[2].p == this->BBox[6].p) &&
	        (this->BBox[3].p == this->BBox[7].p)) {
		t3dV3F sub;
		sub.x = sub.y = sub.z = 5.0f;
		t3dVectSub(&this->BBox[0].p, &this->BBox[0].p, &sub);                     // Aggiunge 5 di spessore
		t3dVectSub(&this->BBox[1].p, &this->BBox[1].p, &sub);
		t3dVectSub(&this->BBox[2].p, &this->BBox[2].p, &sub);
		t3dVectSub(&this->BBox[3].p, &this->BBox[3].p, &sub);
	}

//		Calcs the BBox normals
	t3dPlaneNormal(&this->BBoxNormal[0], &this->BBox[0].p, &this->BBox[2].p, &this->BBox[1].p); //front
	t3dPlaneNormal(&this->BBoxNormal[1], &this->BBox[4].p, &this->BBox[5].p, &this->BBox[6].p); //back
	t3dPlaneNormal(&this->BBoxNormal[2], &this->BBox[4].p, &this->BBox[0].p, &this->BBox[5].p); //Up
	t3dPlaneNormal(&this->BBoxNormal[3], &this->BBox[6].p, &this->BBox[7].p, &this->BBox[2].p); //Down
	t3dPlaneNormal(&this->BBoxNormal[4], &this->BBox[4].p, &this->BBox[6].p, &this->BBox[0].p); //Left
	t3dPlaneNormal(&this->BBoxNormal[5], &this->BBox[5].p, &this->BBox[1].p, &this->BBox[7].p); //Right
	this->BBoxAverageZ = 0;                                                            // Azzera distanza media
	uint32 numAniVerts = stream.readSint32LE();
	if (numAniVerts > 0) {                                                  // Se ci sono Smoothing Groups
		this->OldVertexBuffer = new gVertex[this->NumVerts];   // Crea un OldVertexBuffer
		this->SavedVertexBuffer = new gVertex[this->NumVerts]; // Crea un SavedVertexBuffer
		this->VertsInterpolants = t3dCalloc<t3dV3F>(this->NumVerts);   // Crea spazio per interpolanti

		this->ModVertices.reserve(numAniVerts);
		for (uint32 i = 0; i < numAniVerts; i++) {
			this->ModVertices.push_back(t3dMODVERTS(stream));
		}
	}
}

/* -----------------10/06/99 15.39-------------------
 *                  t3dReleaseAnim
 * --------------------------------------------------*/
void t3dMESH::releaseAnim(uint8 flag) {
	t3dBONEANIM *ba;
	uint32      i;

	if (flag & T3D_MESH_DEFAULTANIM)
		ba = &this->DefaultAnim;
	else
		ba = &this->Anim;

	if (!ba)
		return ;

	for (i = 0; i < ba->NumBones; i++) {
		if (ba->BoneTable && ba->BoneTable[i].Matrix) {
			t3dFree(ba->BoneTable[i].Matrix);
			ba->BoneTable[i].Matrix = nullptr;
			t3dFree(ba->BoneTable[i].Trasl);
			ba->BoneTable[i].Trasl = nullptr;
			ba->BoneTable[i].ModVertices.clear();
		}
	}

	t3dFree(ba->BoneTable);
	ba->BoneTable = nullptr;
	delete[] ba->Dist;
	ba->Dist = nullptr;
}

void t3dMESH::release() { // Will eventually be a destructor.
	this->FList.clear();

	this->MorphFrames.clear();
	this->NList.clear();

	releaseAnim(0);
	releaseAnim(T3D_MESH_DEFAULTANIM);

	this->RejectedMeshes.clear();
	this->PortalList = nullptr;

	delete[] this->WaterBuffer1;
	this->WaterBuffer1 = nullptr;

	delete[] this->WaterBuffer2;
	this->WaterBuffer2 = nullptr;

	delete[] this->VertexBuffer;
	this->VertexBuffer = nullptr;

	delete[] this->OldVertexBuffer;
	this->OldVertexBuffer = nullptr;

	delete[] this->SavedVertexBuffer;
	this->SavedVertexBuffer = nullptr;

	delete[] this->VertsInterpolants;
	this->VertsInterpolants = nullptr;

//		if(mt->VBptr)
//			t3dFree(mt->VBptr);
	this->VBptr = nullptr;
}

} // End of namespace Watchmaker
