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

#ifndef WATCHMAKER_T3D_H
#define WATCHMAKER_T3D_H

#include "common/array.h"
#include "common/stream.h"
#include "watchmaker/render.h"
#include "watchmaker/types.h"
#include "watchmaker/3d/types3d.h"

namespace Watchmaker {

//************************************************************************************************************************
//	defines
//************************************************************************************************************************

#define T3D_PI                              3.14159265358979323846f
#define T3D_2PI                             (3.14159265358979323846f*2.0f)
#define DEGREE_TO_RADIANS(x)                ((x)*T3D_PI/180.0f)
#define RADIANS_TO_DEGREE(x)                ((x)*180.0f/T3D_PI)

#define T3D_NAMELEN                         32          // default name len
#define T3D_MAX_LEVELS                      5           // max bounds level per room
#define T3D_MAX_PATHNODES                   1000        // max path nodes per walk
#define T3D_MAX_WALKSTEPS                   4000        // max walk steps per walk
#define T3D_MAX_BLOCK_MESHES                3           // max block meshes per room
#define T3D_MAX_CHARACTERS                  26          // max characters
#define T3D_MAX_SHADOWBOX_PER_CHAR          10          // max shadow box per character

// Normal Array Define; used in flag field
#define T3D_NORMAL_INVISIBLE                (1<<0)      // invisible
#define T3D_NORMAL_VISIBLE                  (1<<1)      // visible

//Matrices flags
#define T3D_MATRIX_IDENTITY                 (1<<0)      // is identical

//Faces flags - additional to T3D_MATERIAL_FLAGS
#define T3D_FACE_VISIBLE                    (1<<13)     // is visiuble
#define T3D_FACE_CASTSHADOWS                (1<<14)     // cast shadows
#define T3D_FACE_RECEIVESHADOWS             (1<<15)     // receive shadows

//Mesh flags
#define T3D_MESH_CASTSHADOWS                (1<<0)      // Mesh cast shadow
#define T3D_MESH_RECEIVESHADOWS             (1<<1)      // Mesh receive shadows
#define T3D_MESH_NOLIGHTMAP                 (1<<2)      // Mesh without lightmap
#define T3D_MESH_VISIBLE                    (1<<3)      // Mesh visible
#define T3D_MESH_NOBOUNDBOX                 (1<<4)      // Disable BoundBox check
#define T3D_MESH_NOPORTALCHECK              (1<<5)      // Disable Portal check
#define T3D_MESH_HIDDEN                     (1<<6)      // Mesh Hidden
#define T3D_MESH_DEFAULTANIM                (1<<7)      // if takes animartions from the default set
#define T3D_MESH_CHARACTER                  (1<<8)      // set if mesh is part of a character
#define T3D_MESH_PORTAL                     (1<<9)      // mesh is a portal
#define T3D_MESH_MIRROR                     (1<<10)     // mesh is a mirror
#define T3D_MESH_INVISIBLEFROMSECT          (1<<11)     // mesh is VISIBLE form the current sector
#define T3D_MESH_RAYBAN                     (1<<12)     // mesh reject ray detection
#define T3D_MESH_ABS_ANIM                   (1<<13)     // se animazione e' assoluta o relativa
#define T3D_MESH_LAST_DEFAULTANIM           (1<<14)     // if last frame was DEFAULTANIM
#define T3D_MESH_RECEIVERIPPLES             (1<<15)     // Mesh accepts ripples on it's material texture
#define T3D_MESH_WAVESTEXTURE               (1<<16)     // Waves textures on all the material textures
#define T3D_MESH_POOLWATER                  (1<<17)     // Mesh accepts swimming pool water fx
#define T3D_MESH_PREPROCESSPORTAL           (1<<18)     // Preprocess portal for mesh visibilities
#define T3D_MESH_CASTREALTIMESHADOWS        (1<<19)     // Mesh calc realtime shadows using shadows-volumes
#define T3D_MESH_UPDATEVB                   (1<<20)     // needs to update materail VB
#define T3D_MESH_ONLYINMIRRORS              (1<<21)     // appears only in mirrors
#define T3D_MESH_VIEWONLYPORTAL             (1<<22)     // if it's a View Only portal
#define T3D_MESH_PORTALPROCESSED            (1<<23)     // if it was just processed by another portal
#define T3D_MESH_ALWAYSVISIBLE              (1<<24)     // if this mesh is always visible
#define T3D_MESH_SOLARVARIATION             (1<<25)     // if change when sun moves

//Light flags
#define T3D_LIGHT_SPOTLIGHT                 (1<<0)      // is a spot
#define T3D_LIGHT_LIGHTON                   (1<<1)      // light is on
#define T3D_LIGHT_ATTENUATION               (1<<2)      // is attenuated
#define T3D_LIGHT_CASTSHADOWS               (1<<3)      // cast shadows
#define T3D_LIGHT_PULSE                     (1<<4)      // is flicking
#define T3D_LIGHT_ALLLIGHTSOFF              (1<<5)      // turn off all the lights in the room
#define T3D_LIGHT_FLARE                     (1<<6)      // is a flare
#define T3D_LIGHT_CANDLESMOKE               (1<<7)      // light from a moving candle
#define T3D_LIGHT_REALTIME                  (1<<8)      // affects realtime objects
#define T3D_LIGHT_SUN                       (1<<9)      // if it's the sunlight position
#define T3D_LIGHT_SOLARVARIATION            (1<<10)     // if change when sun moves
#define T3D_LIGHT_OFF_MORNING               (1<<11)     // disable during the morning
#define T3D_LIGHT_OFF_AFTERNOON             (1<<12)     // disable during the afternoon
#define T3D_LIGHT_OFF_EVENING               (1<<13)     // disable during the evening
#define T3D_LIGHT_OFF_NIGHT                 (1<<14)     // disable during the night

// Character flags
#define T3D_CHARACTER_HIDE                  (1<<0)      // is hidden
#define T3D_CHARACTER_ENABLEDINMIRROR       (1<<1)      // is in first person but enabled in the mirror
#define T3D_CHARACTER_CASTREALTIMESHADOWS   (1<<2)      // cast real time shadows
#define T3D_CHARACTER_REALTIMELIGHTING      (1<<3)      // needs realtime lighting
#define T3D_CHARACTER_VOLUMETRICLIGHTING    (1<<4)      // needs realtime volumetric lighting
#define T3D_CHARACTER_BNDHIDE               (1<<5)      // don't use char BND
#define T3D_CHARACTER_DIARYDISABLE          (1<<6)      // diable diary

struct t3dM3X3F {
	t3dF32  M[9] = {};                           // Matrix 3 x 3
	uint8   Flags = 0;                          // flags: if identity
	constexpr t3dM3X3F() = default;
};

struct t3dBONE {
	t3dM3X3F    *Matrix = nullptr;            // matrix list
	t3dV3F      *Trasl = nullptr;             // traslation list
	Common::Array<int32> ModVertices;       // modified vertices list
};

struct t3dBONEANIM {
	t3dBONE     *BoneTable = nullptr;           // bones list
	t3dF32      *Dist = nullptr;                // distanca form start frame (for walk and default)
	uint16      NumBones = 0;                   // num bones
	uint16      NumFrames = 0;                  // num frames

	t3dBONEANIM() = default;
	t3dBONEANIM(const t3dBONEANIM &other) = default;
	t3dBONEANIM& operator=(const t3dBONEANIM &rhs) = default;
	t3dBONEANIM(t3dBONEANIM &&old) :
		BoneTable(old.BoneTable),
		Dist(old.Dist),
		NumBones(old.NumBones),
		NumFrames(old.NumFrames) {

		old.BoneTable = nullptr;
		old.Dist = nullptr;
		old.NumBones = 0;
		old.NumFrames = 0;
	}
};

struct t3dMODVERTS {
	uint16  NumVert = 0;                    // vertex indices in mesh
	uint16  NumBone = 0;                    // modified by bone number
	t3dMODVERTS(Common::SeekableReadStream &stream) {
		NumVert = stream.readSint32LE();
		NumBone = stream.readSint32LE();
	}
};

struct t3dMORPH {
	struct t3dMORPHVERT {
		uint32 _index = 0;                      // vertex indices in mesh
		t3dV3F _v;                              // new vertex coords
		t3dMORPHVERT(Common::SeekableReadStream &stream) {
			_index = (uint32)stream.readSint16LE();  // Legge indice vertice
			_v = t3dV3F(stream);          // legge nuova posizione
		}
	};

	Common::Array<t3dMORPHVERT>    _morphModVertices;  // morph verteices list
	t3dMORPH(Common::SeekableReadStream &stream) {
		int morphNumModVertices = stream.readSint16LE();                    // Legge e alloca numero vertici che si muovono
		_morphModVertices.reserve(morphNumModVertices);
		if (morphNumModVertices) {
			for (int j = 0; j < morphNumModVertices; j++) {
				_morphModVertices.push_back(t3dMORPHVERT(stream));
			}
		}
	}
};

struct t3dVERTEX {
	t3dV3F  p;                              // Coordinate vertice in Obj-space  12
	uint8   r = 0;                          // Preilluminazione R vertice       1
	uint8   g = 0;                          // Preilluminazione G vertice       1
	uint8   b = 0;                          // Preilluminazione B vertice       1   15
};

struct t3dPathCamera {
	uint8   PathIndex = 0;                  // path number in room
	uint8   NumCamera = 0;                  // num destination camera
	int8   Direction = 0;                  // path direction: source->dest or dest->source
public:
	explicit t3dPathCamera(Common::SeekableReadStream &stream);
};

struct t3dCAMERAGRID {
	t3dV3F  TopLeft, BottomRight;           // world coords of camera grid
	uint16  Row = 0, Col = 0;               // number of cells
	t3dV3F  CellDim;                        // cell dimensiom
	Common::Array<uint8>   Grid;              // grid
};

struct t3dCAMERAPATH {
	uint16  NumPoints() {
		return PList.size();    // num points in path
	}
	Common::Array<t3dV3F>  PList;                    // points list
	uint32  CarrelloDist = 0;                      // if carrello: max distance from the target
public:
	explicit t3dCAMERAPATH(Common::SeekableReadStream &stream);
};

struct t3dCAMERA {
	t3dV3F      Source;                       // camera eye
	t3dV3F      Target;                       // camera target
	t3dV3F      MaxTarget;                    // original 3dsmax target
	t3dV3F      NormalizedDir;                // cam direction
	t3dF32      Fov;                          // cam field of view
	t3dV2F      Center;                       // cam center
	t3dM3X3F    Matrix;                       // cam view matrix
	t3dF32      NearClipPlane, FarClipPlane;  // camera planes
	uint8       Index;                        // cam index 9in room
	Common::Array<t3dPathCamera> CameraPaths;

	uint8 NumAvailablePaths() const {
		return CameraPaths.size();
	}

public:
	t3dCAMERA() {
		reset();
	}

	explicit t3dCAMERA(Common::SeekableReadStream &stream);

	void reset() {
		Source = t3dV3F();
		Target = t3dV3F();
		MaxTarget = t3dV3F();
		NormalizedDir = t3dV3F();
		Fov = 0.0f;
		Center = t3dV2F();
		Matrix = t3dM3X3F();
		NearClipPlane = 0.0f, FarClipPlane = 0.0f;
		Index = 0;
		CameraPaths.clear();
	}

	void normalizedSight();
};

struct t3dVolLights {
	t3dF32  CellsSize = 0.0f;                   // cell cube dimension
	uint32  xcells = 0;                         // number of cells for x dir
	uint32  ycells = 0;                         // number of cells for y dir
	uint32  zcells = 0;                         // number of cells for z dir
	Common::Array<uint8>   VolMap;                // volumetric map
};

struct t3dBODY;

struct t3dParticle {
	uint16  ParticleIndex;
	uint16  Num;
	t3dF32  Lung;
	t3dF32  Size;
	t3dF32  Seg1;
	t3dF32  Seg2;
	t3dF32  Dim1;
	t3dF32  Dim2;
	t3dF32  Speed;
	t3dF32  Speed1;
	t3dF32  Speed2;
	t3dF32  Caos;
	t3dF32  Caos1;
	t3dF32  Caos2;
	uint32  Delay;
	uint8   OR1, Type;
	t3dF32  R1, G1, B1;
	t3dF32  R2, G2, B2;
	t3dF32  R3, G3, B3;
public:
	explicit t3dParticle(Common::SeekableReadStream &stream);
};

struct t3dAnimLight {
	Common::Array<gVertex *> VisVerts;        // pointer to visible vertices from lights
	uint16      NumVisVerts() const {
		return VisVerts.size();    // num visible vertices from lights
	}
	//t3dU32      *SavedLightColor;           // pointer to original vartex illumination
	int8       LastRandomizer;             // randomizer for flicker effects
};

struct t3dLIGHT {
	uint32      Type;                       // light type
	t3dV3F      Source;                     // source
	t3dV3F      Target;                     // target
	t3dF32      Multiplier;                 // light multiplier
	t3dF32      NearRange, FarRange;        // attenuation ranges
	t3dF32      HotSpot, FallOff;           // spot ranges
	t3dV3F      Color;                      // orig color
	uint8       Flicker;                    // flicker activated
	uint8       FlickerDelay = 0;           // flicker delay
	int8       LightRandomizer = 0;         // flicker randomizer
	Common::SharedPtr<t3dParticle> Particle;  // paticles attached
	t3dF32      FlareSize = 0;              // flare size
	gMaterial   Material[2];                // flare material
	t3dAnimLight AnimLight;                 // animation attached
	t3dV3F      SolarColor[4];              // Ambient color value for solar movement
	t3dV3F      SolarPos[4];                // Position movements due to solar movement
public:
	t3dLIGHT(WGame &game, t3dBODY *b, WorkDirs &workDirs, Common::SeekableReadStream &stream);
private:
	void setupVisibleVerticesFromLight(t3dBODY *b);
	void SetVisibleFromLight(gVertex *v);
};
typedef t3dLIGHT *LightPtr;

struct t3dPLIGHT {
	uint8   Num = 0;                        // index positional light in room
	t3dV3F  Pos;                            // position (world)
	t3dV3F  Dir;                            // destination (world)
public:
	explicit t3dPLIGHT(Common::SeekableReadStream &stream);
};

struct PointXZ {
	float x = 0.0f, z = 0.0f;
	static PointXZ readFromStream(Common::SeekableReadStream &stream) {
		PointXZ result;
		result.x = stream.readFloatLE();
		result.z = stream.readFloatLE();
		return result;
	}
};

// Struct for returning from "partial" functions mathematically
struct PointResult {
	PointXZ result;
	bool isValid = false;
};

struct t3dPAN {
	PointXZ a, b;                           // point A / B position
	PointXZ backA, backB;                   // back to point A / B
	uint16 near1 = 0;                       // panel near to A
	uint16 near2 = 0;                       // panel near to B
};

struct t3dPATHNODE {
	PointXZ pos;                                // path node position
	float dist = 0.0f;                          // distance from start
	short oldp = 0;                             // last panel hit
	short curp = 0;                             // cur panel hit
};

struct t3dSTEPS {
	t3dV3F  Pos;                            // position
	t3dF32  Angle = 0.0f;                   // angle
	int16  curp = 0;                       // cur panel
	int16  Act = 0, Frame = 0;             // cur action and frame

	void reset() {
		Pos = t3dV3F();
		Angle = 0.0f;
		curp = 0;
		Act = 0;
		Frame = 0;
	}
};

struct t3dWALK {
	PointXZ Look;                                  // Point on the bounds
	PointXZ Cur;                                   // Point perpendicular
	t3dPAN  *Panel = nullptr;                      // pointer to cur panel struct
	t3dPATHNODE PathNode[T3D_MAX_PATHNODES] = {};  // path nodes list
	t3dSTEPS    WalkSteps[T3D_MAX_WALKSTEPS] = {}; // walk steps list
	int32  CurrentStep = 0;                // current step
	int32  PanelNum = 0;                   // num panels
	int32  NumPathNodes = 0;               // num path nodes
	int32  NumSteps = 0;                   // num steps
	int16  CurPanel = 0, OldPanel = 0;     // current panel
	int16  CurAction = 0, CurFrame = 0;    // cur action and frame
	int32  Check = 0;                      // intersection check
};

struct t3dMESH;

struct t3dCHARACTER {
	char        Name[T3D_NAMELEN] = {};     // character name
	t3dBODY     *Body = nullptr;            // caracter body
	t3dBODY     *Shadow = nullptr;          // shadow struct
	t3dMESH     *Mesh = nullptr;            // shortcut to first mesh
	t3dBODY     *CurRoom = nullptr;         // character cur room
	t3dV3F      Pos;                        // char pos (world)
	t3dV3F      Dir;                        // char dir
	t3dWALK     Walk;                       // walk struct
	uint8       Flags = 0;                  // char flags
	SHADOWBOX   *ShadowBox[T3D_MAX_SHADOWBOX_PER_CHAR] = {}; // active shadows
	t3dF32      Radius = 0.0f, Height = 0.0f;  // char dimension (for shadow)
	int32      CurExpressionSet = 0;          // current expression face set
};

struct t3dCHARSTOPSTATUS {
	uint8   BlendPercent = 0;
	int32  NumPathNodes = 0;
	int32  CurrentStep = 0;
	int32  NumSteps = 0;
	int16  CurAction = 0, CurFrame = 0;
	int16  mCurFrame = 0;
	t3dF32  posy = 0.0f, mposy = 0.0f;
	t3dM3X3F    Matrix;
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_T3D_H
