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


#ifndef WATCHMAKER_T3D_BODY_H
#define WATCHMAKER_T3D_BODY_H

#include "watchmaker/types.h"
#include "watchmaker/t3d.h"
#include "watchmaker/3d/t3d_mesh.h"

namespace Watchmaker {

struct t3dBODY {
	Common::String				 name; // room name
	uint32                       NumMeshes() { return MeshTable.size(); };                    // num meshes
	uint32                       NumCameras() const { return CameraTable.size(); }            // num cameras
	uint16                       NumPaths() const { return CameraPath.size(); }               // num camera paths
	uint32                       NumLights() const { return LightTable.size(); }              // num lights
	uint16                       NumPanels[T3D_MAX_LEVELS] = {};  // num panels per level
	uint16                       NumNormals = 0;          // num face normals
	uint16                       NumVerticesNormals = 0;  // num vertex normals
	uint16                       NumPosLights() const { return PosLightTable.size(); };           // num positional lights
	uint16                       NumLevels = 0;         // num panel levels
	uint16                       CurLevel = 0;          // current level
	uint32                       NumTotVerts = 0;       // total number of verts in room
	t3dV3F                       AmbientLight;          // room ambient color
	Common::Array<t3dMESH>         MeshTable;			    // meshes list
	MaterialTable		         MatTable;              // materials list
	uint32                       NumMaterials() const { return MatTable.size(); }           // num materials
	MaterialTable		         LightmapTable;         // lightmap material list
	uint32                       NumLightmaps = 0;      // num lightmap materials
	MaterialTable		         MirrorMatTable;        // material list (for mirrors)
	uint32                       NumMirrorMaterials() const { return MirrorMatTable.size(); };     // num materials (for mirror)
private:
	Common::Array<Common::SharedPtr<VertexBuffer>>    VBTable; // metrial vertex buffers list
public:
	Common::SharedPtr<VertexBuffer> addVertexBuffer() {
		VBTable.push_back(Common::SharedPtr<VertexBuffer>(new VertexBuffer()));
		return VBTable.back();
	}
	void clearVBTable() {
		for (int i = 0; i < VBTable.size(); i++) {
			rDeleteVertexBuffer(*VBTable[i]);
		}
		VBTable.clear();
	}
	uint32                       NumVB() { return VBTable.size(); };             // num vertex buffer
public:
	Common::Array<t3dCAMERA>       CameraTable;           // camera list
	Common::Array<t3dLIGHT>        LightTable;            // light list
	Common::Array<t3dPLIGHT>       PosLightTable;         // positional light list
	NormalList				     NList;                 // normal list
	t3dCAMERAGRID                CameraGrid;             // camera grid
	Common::Array<t3dCAMERAPATH>   CameraPath;            // camer paths list
	t3dPAN          			 *Panel[T3D_MAX_LEVELS] = {}; // room panels for level
	t3dF32          			 PanelHeight[T3D_MAX_LEVELS] = {};        // panel height for levels
	Common::SharedPtr<t3dVolLights> VolumetricLights;                  // volumetric lights
	t3dMESH         			 *BlockMeshes[T3D_MAX_BLOCK_MESHES] = {}; // block mesh (for external rooms)
	t3dV3F          			 MinPos;                             // min room position
private:
	void allocateNormals();
	void initNormals(Common::SeekableReadStream &stream);
public:
	t3dCAMERA *PickCamera(uint8 in);
	Common::Array<t3dPLIGHT> getPositionalLight(uint8 pos);

	t3dBODY *loadFromStream(WGame &game, const Common::String &pname, Common::SeekableReadStream &stream, uint32 LoaderFlags);
	void populatePortalLists();
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_T3D_BODY_H
