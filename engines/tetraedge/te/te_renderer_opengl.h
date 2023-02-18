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

#ifndef TETRAEDGE_TE_TE_RENDERER_OPENGL_H
#define TETRAEDGE_TE_TE_RENDERER_OPENGL_H

#if defined(USE_OPENGL_GAME)

#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

class TeRendererOpenGL : public TeRenderer {
public:
	TeRendererOpenGL();
	void clearBuffer(TeRenderer::Buffer buf) override;
	void colorMask(bool r, bool g, bool b, bool a) override;
	void disableAllLights() override;
	void disableTexture() override;
	void disableWireFrame() override;
	void disableZBuffer() override;
	void drawLine(const TeVector3f32 &from, const TeVector3f32 &to) override;
	void enableAllLights() override;
	void enableTexture() override;
	void enableWireFrame() override;
	void enableZBuffer() override;
	void init(uint width, uint height) override;
	void loadProjectionMatrix(const TeMatrix4x4 &matrix) override;
	Common::String renderer() override;
	void renderTransparentMeshes() override;
	void reset() override;
	void setClearColor(const TeColor &col) override;
	void setCurrentColor(const TeColor &col) override;
	void setMatrixMode(enum MatrixMode mode) override;
	void setViewport(int x, int y, int w, int h) override;
	void shadowMode(enum ShadowMode mode) override;
	Common::String vendor() override;
	void applyMaterial(const TeMaterial &m) override;
	void updateGlobalLight() override;
	void updateScreen() override;

protected:

	void loadMatrixToGL(const TeMatrix4x4 &matrix) override;
};

} // end namespace Tetraedge

#endif // USE_OPENGL

#endif // TETRAEDGE_TE_TE_RENDERER_OPENGL_H
