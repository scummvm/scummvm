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

#ifndef TETRAEDGE_TE_TE_LAYOUT_H
#define TETRAEDGE_TE_TE_LAYOUT_H

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_i_layout.h"
#include "tetraedge/te/te_i_3d_object2.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_vector2s32.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeLayout : public TeILayout, public Te3DObject2 {
public:
	TeLayout();

	virtual ~TeLayout();

	void addChild(Te3DObject2 *child) override;
	void addChildBefore(Te3DObject2 *newchild, const Te3DObject2 *ref) override;
	const TeVector3f32 &anchor();
	void disableAutoZ();
	void enableAutoZ();
	bool isAutoZEnabled();

	void draw() override;

	virtual bool isMouseIn(const TeVector2s32 &mouseloc);

	DrawMode mode();

	bool onChildSizeChanged();
	bool onParentSizeChanged();
	bool onParentWorldTransformationMatrixChanged();

	TeVector3f32 position() override;
	CoordinatesType positionType() const;
	float ratio() const;
	RatioMode ratioMode() const;
	void removeChild(Te3DObject2 *child) override;
	float safeAreaRatio() const;
	void setAnchor(const TeVector3f32 &anchor);
	virtual void setEditionColor(TeColor col) {};
	void setMode(DrawMode mode);
	void setParent(Te3DObject2 *parent) override;
	void setPosition(const TeVector3f32 &pos) override;
	void setPositionType(CoordinatesType newtype);
	void setRatio(float val);
	void setRatioMode(RatioMode mode);
	void setRotation(const TeQuaternion &rot) override;
	void setSafeAreaRatio(float ratio);
	void setScale(const TeVector3f32 &scale) override;
	void setSize(const TeVector3f32 &size) override;
	void setSizeType(CoordinatesType coordtype);
	void setZPosition(float zpos) override;

	virtual TeVector3f32 size() override;
	CoordinatesType sizeType() const;
	TeVector3f32 transformMousePosition(const TeVector2s32 &mousepos);

	virtual void updateMesh() {}; // override available for TeSpriteLayout
	void updatePosition();
	virtual void updateSize();
	void updateWorldMatrix();
	void updateZ() override;
	void updateZSize();

	TeVector3f32 userPosition() const;
	TeVector3f32 userSize();
	TeVector3f32 worldPosition() override;
	TeMatrix4x4 worldTransformationMatrix() override;
	bool worldVisible() override;
	float xSize() override;
	float ySize() override;
	float zSize() override;

protected:
	bool _sizeChanged;

private:
	TeVector3f32 _anchor;
	CoordinatesType _positionType;
	TeVector3f32 _userPosition;
	CoordinatesType _sizeType;
	TeVector3f32 _userSize;
	TeMatrix4x4 _worldMatrixCache;

	DrawMode _drawMode;
	bool _autoz;
	bool _positionChanged;
	bool _worldMatrixChanged;
	bool _needZSizeUpdate;
	bool _needZUpdate;
	bool _updatingZ;
	bool _updatingZSize;
	bool _updatingSize;
	bool _updatingPosition;
	bool _updatingWorldMatrix;
	float _ratio;
	RatioMode _ratioMode;
	float _safeAreaRatio;

	TeICallback0ParamPtr _onChildSizeChangedCallback;
	TeICallback0ParamPtr _onParentSizeChangedCallback;
	TeICallback0ParamPtr _onParentWorldTransformationMatrixChangedCallback;
	TeICallback0ParamPtr _onMainWindowChangedCallback;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LAYOUT_H
