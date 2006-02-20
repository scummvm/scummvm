// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#include "debug.h"
#include "colormap.h"
#include "material.h"
#include "driver_ronin.h"

#include <ronin/ronin.h>
#include <ronin/report.h>
#include <ronin/sincos_rroot.h>

void DriverRonin::toggleFullscreenMode()
{
	//FIXME
	reportf("%s\n", __func__);
}

bool DriverRonin::isHardwareAccelerated()
{
	reportf("%s\n", __func__);
	return true;
}

void DriverRonin::setupCamera(float fov, float nclip, float fclip, float roll)
{
	//FIXME
	//reportf("%s\n", __func__);

	clearMatrixStack();

	float cot_fov = 1 / std::tan(fov * (M_PI / 180 / 2));
	float frustum[4][4] = {
		{ (-640/2) * cot_fov, 0, 0, 0 },
		{ 0, (480/2 / 0.75) * cot_fov, 0, 0 },
		{ 640/2, 480/2, (nclip + fclip) / (nclip - fclip), 1 },
		{ 0, 0, 2*(nclip*fclip) / (nclip - fclip), 0 }
	};
        loadMatrix(&frustum);
	rotateZ( roll );
}

void DriverRonin::positionCamera(Vector3d pos, Vector3d interest)
{
	//FIXME
	//reportf("%s\n", __func__);
	float fx = interest.x() - pos.x(),
		fy = interest.y() - pos.y(),
		fz = interest.z() - pos.z();
	float n;
	RROOT(fx*fx + fy*fy + fz*fz, n);
	fx *= n;
	fy *= n;
	fz *= n;
	float sx = 0, sy = 0, sz = 0;
	if (fx == 0.0 && fy == 0.0) {
		sx = -fz;
		sz = fx;
	} else {
		sx = fy;
		sy = -fx;
	}
	float m[4][4] = {
		{ sx, sy*fz-sz*fy, -fx, 0 },
		{ sy, sz*fx-sx*fz, -fy, 0 },
		{ sz, sx*fy-sy*fx, -fz, 0 },
		{ 0, 0, 0, 1 }
	};
	applyMatrix(&m);
	translate(-pos.x(), -pos.y(), -pos.z());
}

void DriverRonin::clearScreen()
{
	struct polygon_list mypoly;
	struct packed_colour_vertex_list myvertex;

	if(!_polyCount)
		ta_begin_frame();

	mypoly.cmd =
		TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
		TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR;
	mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS/*|TA_POLYMODE1_NO_Z_UPDATE*/;
	mypoly.mode2 =
		TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED;
	mypoly.texture = 0;
	
	mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
	
	ta_commit_list(&mypoly);
	
	myvertex.cmd = TA_CMD_VERTEX;
	myvertex.ocolour = 0;
	myvertex.colour = 0xff00ff;
	myvertex.z = 0.01;
	myvertex.u = 0.0;
	myvertex.v = 0.0;

	myvertex.x = 0;
	myvertex.y = 0;
	ta_commit_list(&myvertex);
	
	myvertex.x = 640;
	ta_commit_list(&myvertex);
	
	myvertex.x = 0;
	myvertex.y = 480;
	ta_commit_list(&myvertex);
	
	myvertex.x = 640;
	myvertex.cmd |= TA_CMD_VERTEX_EOS;
	ta_commit_list(&myvertex);
	
	_polyCount++;

	//FIXME
	// reportf("%s\n", __func__);
}

void commit_dummy_transpoly()
{
	struct polygon_list mypoly;

	mypoly.cmd =
		TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
		TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR;
	mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
	mypoly.mode2 =
		TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
		TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA;
	mypoly.texture = 0;
	mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
	ta_commit_list(&mypoly);
}

void DriverRonin::flipBuffer()
{
	if(_polyCount) {
		ta_commit_end();
		commit_dummy_transpoly();
		ta_commit_frame();
		_polyCount = 0;
	}
}

void DriverRonin::startActorDraw(Vector3d pos, float yaw, float pitch, float roll)
{
	//FIXME
	//reportf("%s\n", __func__);
	translateViewpoint(pos, pitch, yaw, roll);
}

void DriverRonin::finishActorDraw()
{
	//FIXME
	//reportf("%s\n", __func__);
	translateViewpoint();
}
	
void DriverRonin::set3DMode()
{
	//FIXME
	//reportf("%s\n", __func__);
}

void DriverRonin::translateViewpoint(Vector3d pos, float pitch, float yaw, float roll)
{
	//FIXME
	//reportf("%s\n", __func__);
	pushMatrix();
	translate(pos.x(), pos.y(), pos.z());
	rotateZ( -yaw );
	rotateX( -pitch );
	rotateY( -roll );
}

void DriverRonin::translateViewpoint()
{
	//FIXME
	//reportf("%s\n", __func__);
	popMatrix();
}

void DriverRonin::drawHierachyNode(const Model::HierNode *node)
{
	//FIXME
	//reportf("%s\n", __func__);
	translateViewpoint(node->_animPos / node->_totalWeight, node->_animPitch / node->_totalWeight, node->_animYaw / node->_totalWeight, node->_animRoll / node->_totalWeight);
	if (node->_hierVisible) {
		if (node->_mesh != NULL && node->_meshVisible) {
			pushMatrix();
			translate(node->_pivot.x(), node->_pivot.y(), node->_pivot.z());
			node->_mesh->draw();
			popMatrix();
		}

		if (node->_child != NULL) {
			node->_child->draw();
		}
	}
	translateViewpoint();

	if (node->_sibling != NULL)
		node->_sibling->draw();
}

void DriverRonin::drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts)
{
	//FIXME
	//reportf("%s\n", __func__);

	struct packed_colour_vertex_list myvertex;

	myvertex.cmd = TA_CMD_VERTEX;
	myvertex.ocolour = 0;
	myvertex.colour = 0x00ff00;
	myvertex.u = 0.0;
	myvertex.v = 0.0;

	for (int i = 0; i < face->_numVertices; ) {
		int j = i&1? i>>1 : face->_numVertices-1-(i>>1);
		float *v = vertices + 3*face->_vertices[j];

		if(face->_texVertices != NULL) {
			float *t = textureVerts + 2*face->_texVertices[j];
			myvertex.u = t[0] * _u_scale;
			myvertex.v = t[1] * _v_scale;
			//printf("%f %f\n", myvertex.u, myvertex.v);
		}

		if(++i == face->_numVertices)
			myvertex.cmd |= TA_CMD_VERTEX_EOS;

		ta_commit_vertex(&myvertex, v[0], v[1], v[2]);
	}
}

void DriverRonin::disableLights()
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::setupLight(Scene::Light *light, int lightId)
{
	//FIXME
	//reportf("%s\n", __func__);
}

void DriverRonin::createMaterial(Material *material, const char *data, const CMap *cmap)
{
	//FIXME
	reportf("%s\n", __func__);
	reportf("%d x %d x %d\n", material->_width, material->_height, material->_numImages);
	if(data != NULL && material->_numImages > 0) {
		Texture *tex = allocateTexture(material->_width, material->_height, material->_numImages);
		material->_textures = tex;
		for(int i=0; i<material->_numImages; i++) {
			tex->setTexture(i, (const uint8 *)data, cmap);
			data += material->_width * material->_height + 24;
		}
	} else {
		material->_textures = 0;
	}
}

void DriverRonin::selectMaterial(const Material *material)
{
	//FIXME
	//reportf("%s\n", __func__);
	struct polygon_list mypoly;

	if(!_polyCount)
		ta_begin_frame();

	if(material->_textures) {
		mypoly.cmd =
			TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
			TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|
			TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_GOURAUD_SHADING;
		mypoly.mode1 = TA_POLYMODE1_Z_GREATEREQUAL; //TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
		((Texture *)material->_textures)->setup(material->_currImage, mypoly, _u_scale, _v_scale);
	} else {

		// reportf("%d x %d x %d\n", material->_width, material->_height, material->_numImages);

		mypoly.cmd =
			TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
			TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|
			TA_CMD_POLYGON_GOURAUD_SHADING;
		mypoly.mode1 = TA_POLYMODE1_Z_GREATEREQUAL; //TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
		mypoly.mode2 =
			TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED;
		mypoly.texture = 0;
	}
		
	mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
	
	ta_commit_list(&mypoly);

	_polyCount++;
}

void DriverRonin::destroyMaterial(Material *material)
{
	//FIXME
	reportf("%s\n", __func__);
	if(material->_textures)
		freeTexture((Texture *)material->_textures);
}

#define QACR0 (*(volatile unsigned int *)(void *)0xff000038)
#define QACR1 (*(volatile unsigned int *)(void *)0xff00003c)
#define SQ_WAIT_STORE_QUEUES() do { unsigned int *d = (unsigned int *)0xe0000000; d[0] = d[8] = 0; } while(0)
static void texture_memcpy64(void *dest, void *src, int cnt)
{
	unsigned int *s = (unsigned int *)src;
	unsigned int *d = (unsigned int *)(void *)
		(0xe0000000 | (((unsigned long)dest) & 0x03ffffc0));
	unsigned int old_qacr0 = QACR0;
	unsigned int old_qacr1 = QACR1;
	SQ_WAIT_STORE_QUEUES();
	QACR0 = ((0xa4000000>>26)<<2)&0x1c;
	QACR1 = ((0xa4000000>>26)<<2)&0x1c;
	while(cnt--) {
		d[0] = *s++;
		d[1] = *s++;
		d[2] = *s++;
		d[3] = *s++;
		asm("pref @%0" : : "r" (s+16));
		d[4] = *s++;
		d[5] = *s++;
		d[6] = *s++;
		d[7] = *s++;
		asm("pref @%0" : : "r" (d));
		d += 8;
		d[0] = *s++;
		d[1] = *s++;
		d[2] = *s++;
		d[3] = *s++;
		asm("pref @%0" : : "r" (s+16));
		d[4] = *s++;
		d[5] = *s++;
		d[6] = *s++;
		d[7] = *s++;
		asm("pref @%0" : : "r" (d));
		d += 8;
	}
	SQ_WAIT_STORE_QUEUES();
	QACR0 = old_qacr0;
	QACR1 = old_qacr1;
}

void DriverRonin::createBitmap(Bitmap *bitmap)
{
	static int x = 0;

	//FIXME
	reportf("%s %p\n", __func__, bitmap);

	printf("format = %d, numImages = %d, w x h = %d x %d\n",
	       bitmap->_format, bitmap->_numImages,
	       bitmap->_width, bitmap->_height);
	bitmap->_texIds = NULL;
	if(bitmap->_format == 1 && ++x <= 3) {
	  bitmap->_texIds = ta_txalloc(bitmap->_width * bitmap->_height * 2);
	  texture_memcpy64(bitmap->_texIds, bitmap->_data[0],
			   (bitmap->_height*bitmap->_width)>>5);
	}
	if(true) {
	  for(int i=0; i<bitmap->_numImages; i++) {
	    delete[] bitmap->_data[i];
	  }
	  delete[] bitmap->_data;
	  bitmap->_data = NULL;
	}
}

void DriverRonin::drawBitmap(const Bitmap *bitmap)
{
	struct polygon_list mypoly;
	struct packed_colour_vertex_list myvertex;

	if(bitmap->_format != 1 || !bitmap->_texIds)
		return;

	if(!_polyCount)
		ta_begin_frame();

	mypoly.cmd =
		TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
		TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
	mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS/*|TA_POLYMODE1_NO_Z_UPDATE*/;
	mypoly.mode2 =
		TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_REPLACE|
		TA_POLYMODE2_U_SIZE_1024|TA_POLYMODE2_V_SIZE_1024;
	mypoly.texture = TA_TEXTUREMODE_RGB565|TA_TEXTUREMODE_NON_TWIDDLED|
		TA_TEXTUREMODE_STRIDE|TA_TEXTUREMODE_ADDRESS(bitmap->_texIds);
	
	mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
	
	ta_commit_list(&mypoly);
	
	myvertex.cmd = TA_CMD_VERTEX;
	myvertex.ocolour = 0;
	myvertex.colour = 0;
	myvertex.z = 0.01;
	myvertex.u = 0.0;
	myvertex.v = 0.0;

	myvertex.x = bitmap->_x;
	myvertex.y = bitmap->_y;
	ta_commit_list(&myvertex);
	
	myvertex.x += bitmap->_width;
	myvertex.u = bitmap->_width*(1/1024.0);
	ta_commit_list(&myvertex);
	
	myvertex.x = bitmap->_x;
	myvertex.y += bitmap->_height;
	myvertex.u = 0.0;
	myvertex.v = bitmap->_height*(1/1024.0);
	ta_commit_list(&myvertex);
	
	myvertex.x += bitmap->_width;
	myvertex.u = bitmap->_width*(1/1024.0);
	myvertex.cmd |= TA_CMD_VERTEX_EOS;
	ta_commit_list(&myvertex);
	
	_polyCount++;

	//FIXME
	//reportf("%s %p\n", __func__, bitmap);
}

void DriverRonin::destroyBitmap(Bitmap *bitmap)
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::drawDepthBitmap(int x, int y, int w, int h, char *data)
{
	//FIXME
	reportf("%s\n", __func__);
}

Bitmap *DriverRonin::getScreenshot(int w, int h)
{
	//FIXME
	reportf("%s\n", __func__);
	return NULL;
}

void DriverRonin::storeDisplay()
{
	//FIXME
	//reportf("%s\n", __func__);
}

void DriverRonin::copyStoredToDisplay()
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::dimScreen()
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::dimRegion(int x, int y, int w, int h, float level)
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::drawEmergString(int x, int y, const char *text, const Color &fgColor)
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::loadEmergFont()
{
	//FIXME
	reportf("%s\n", __func__);
}

Driver::TextObjectHandle *DriverRonin::createTextBitmap(uint8 *bitmap, int width, int height, const Color &fgColor)
{
	//FIXME
	reportf("%s\n", __func__);
	return new TextObjectHandle;
}

void DriverRonin::drawTextBitmap(int x, int y, TextObjectHandle *handle)
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::destroyTextBitmap(TextObjectHandle *handle)
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::drawRectangle(PrimitiveObject *primitive)
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::drawLine(PrimitiveObject *primitive)
{
	//FIXME
	reportf("%s\n", __func__);
}

void DriverRonin::prepareSmushFrame(int width, int height, byte *bitmap)
{
	//FIXME
  //reportf("%s(%d,%d,%p)\n", __func__, width, height, bitmap);
	assert(width == 640 && height == 480);
	texture_memcpy64(_smushTex, bitmap, (640*480)>>5);
}

void DriverRonin::drawSmushFrame(int offsetX, int offsetY)
{
	//FIXME
	//	reportf("%s(%d,%d)\n", __func__, offsetX, offsetY);

	struct polygon_list mypoly;
	struct packed_colour_vertex_list myvertex;

	if(!_polyCount)
		ta_begin_frame();
	
	mypoly.cmd =
		TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
		TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_TEXTURED;
	mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
	mypoly.mode2 =
		TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_TEXTURE_REPLACE|
		TA_POLYMODE2_U_SIZE_1024|TA_POLYMODE2_V_SIZE_1024;
	mypoly.texture = TA_TEXTUREMODE_RGB565|TA_TEXTUREMODE_NON_TWIDDLED|
		TA_TEXTUREMODE_STRIDE|TA_TEXTUREMODE_ADDRESS(_smushTex);
	
	mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
	
	ta_commit_list(&mypoly);
	
	myvertex.cmd = TA_CMD_VERTEX;
	myvertex.ocolour = 0;
	myvertex.colour = 0;
	myvertex.z = 0.01;
	myvertex.u = 0.0;
	myvertex.v = 0.0;
	
	myvertex.x = offsetX;
	myvertex.y = offsetY;
	ta_commit_list(&myvertex);
	
	myvertex.x += 640;
	myvertex.u = 640*(1/1024.0);
	ta_commit_list(&myvertex);
	
	myvertex.x = offsetX;
	myvertex.y += 480;
	myvertex.u = 0.0;
	myvertex.v = 480*(1/1024.0);
	ta_commit_list(&myvertex);
	
	myvertex.x += 640;
	myvertex.u = 640*(1/1024.0);
	myvertex.cmd |= TA_CMD_VERTEX_EOS;
	ta_commit_list(&myvertex);
	
	_polyCount++;
}

char *DriverRonin::getVideoDeviceName()
{
	return "Dreamcast PowerVR Video Device";
}

void DriverRonin::initGfx()
{
	_polyCount = 0;
	dc_setup_ta();
	initTextures();
	*(volatile unsigned int *)(0xa05f80e4) = 640/32; //stride

	_smushTex = ta_txalloc(640 * 480 * 2);
}
