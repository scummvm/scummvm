// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef MODEL_H
#define MODEL_H

#include "vector3d.h"
#include "matrix4.h"
#include "resource.h"
#include <cstring>

class CMap;
class Material;
class TextSplitter;

class Model : public Resource {
public:
  // Construct a 3D model from the given data.
  Model(const char *filename, const char *data, int len, const CMap &cmap);
  void loadBinary(const char *data, const CMap &cmap);
  void loadText(TextSplitter &ts, const CMap &cmap);

  void draw() const;

  ~Model();

  struct Geoset;
  struct Mesh;
  struct HierNode {
    void loadBinary(const char *&data, HierNode *hierNodes, const Geoset &g);
    void draw() const;
    void addChild(HierNode *child);
    void removeChild(HierNode *child);
	void setMatrix(Matrix4 matrix);
	void update();

    char name_[64];
    Mesh *mesh_;
    int flags_, type_;
    int depth_, numChildren_;
    HierNode *parent_, *child_, *sibling_;
    Vector3d pos_, pivot_;
    float pitch_, yaw_, roll_;
    Vector3d animPos_;
    float animPitch_, animYaw_, animRoll_;
    bool meshVisible_, hierVisible_;
    int priority_, totalWeight_;
	Matrix4 matrix_;
	Matrix4 localMatrix_;
	Matrix4 pivotMatrix;
  };

  HierNode *copyHierarchy();
  int numNodes() const { return numHierNodes_; }

private:
  struct Face {
    void loadBinary(const char *&data, ResPtr<Material> *materials);
    void draw(float *vertices, float *vertNormals, float *textureVerts) const;
    ~Face();

    Material *material_;
    int type_, geo_, light_, tex_;
    float extraLight_;
    int numVertices_;
    int *vertices_, *texVertices_;
    Vector3d normal_;
  };

  struct Mesh {
    void loadBinary(const char *&data, ResPtr<Material> *materials);
    void loadText(TextSplitter &ts, ResPtr<Material> *materials);
    void draw() const;
	void update();
    ~Mesh();

    char name_[32];
    float radius_;
    int shadow_, geometryMode_, lightingMode_, textureMode_;

    int numVertices_;
    float *vertices_;		// sets of 3
    float *verticesI_;
    float *vertNormals_;	// sets of 3

    int numTextureVerts_;
    float *textureVerts_;	// sets of 2

    int numFaces_;
    Face *faces_;
	Matrix4 matrix_;
  };

  struct Geoset {
    void loadBinary(const char *&data, ResPtr<Material> *materials);
    void loadText(TextSplitter &ts, ResPtr<Material> *materials);
    ~Geoset();

    int numMeshes_;
    Mesh *meshes_;
  };

  int numMaterials_;
  ResPtr<Material> *materials_;
  Vector3d insertOffset_;
  int numGeosets_;
  Geoset *geosets_;
  float radius_;
  int numHierNodes_;
  HierNode *rootHierNode_;
};

#endif
