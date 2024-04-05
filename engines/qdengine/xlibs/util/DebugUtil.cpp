#include "StdAfxTr.h"
#include "DebugUtil.h"			 
#include "Render\Inc\IRenderDevice.h"
#include "Render\Src\cCamera.h"
#include "Terra\vmap.h"

//LogStream fout("lst",XS_OUT);

bool net_log_mode=0;
XBuffer net_log_buffer(8192, 1);

////////////////////////////////////////////////////////////////////////////////////////////
//		Converter
////////////////////////////////////////////////////////////////////////////////////////////
Vect3f To3D(const Vect2f& pos)
{
	Vect3f p;
	p.x=pos.x;
	p.y=pos.y;

	int x = round(pos.x), y = round(pos.y);
	if(x >= 0 && x < vMap.H_SIZE && y >= 0 && y < vMap.V_SIZE)
		p.z=vMap.gVBuf[vMap.offsetGBuf(x>>kmGrid,y>>kmGrid)];
	else
		p.z=0;
	return p;
}

Vect2f clampWorldPosition(const Vect2f& pos, float radius)
{
	return Vect2f(clamp(pos.x, radius, vMap.H_SIZE - 1 - radius), clamp(pos.y, radius, vMap.V_SIZE - 1 - radius));
}

Vect3f clampWorldPosition(const Vect3f& pos, float radius)
{
	return Vect3f(clampWorldPosition(Vect2f(pos), radius), pos.z);
}


////////////////////////////////////////////////////////////////////////////////////////////
//			Show System
////////////////////////////////////////////////////////////////////////////////////////////
ShowDispatcher show_dispatcher;

void clip_line_3D(const Vect3f &v1,const Vect3f &v2,Color4c color)
{
	gb_RenderDevice->DrawLine(v1, v2,color);
}

void clip_pixel(int x1,int y1,Color4c color, int size)
{
	gb_RenderDevice -> DrawPixel(x1,y1,color);
	if(size){
		gb_RenderDevice -> DrawPixel(x1 + 1,y1,color);
		gb_RenderDevice -> DrawPixel(x1,y1 + 1,color);
		gb_RenderDevice -> DrawPixel(x1 + 1,y1 + 1,color);

		if(size == 2){
			gb_RenderDevice -> DrawPixel(x1 - 1,y1,color);
			gb_RenderDevice -> DrawPixel(x1,y1 - 1,color);
			gb_RenderDevice -> DrawPixel(x1 - 1,y1 - 1,color);
			gb_RenderDevice -> DrawPixel(x1 - 1,y1 + 1,color);
			gb_RenderDevice -> DrawPixel(x1 + 1,y1 - 1,color);
			}
		}
}

void clip_circle_3D(const Vect3f& vc, float radius, Color4c color)
{
	float segment_length = 3;
	int N = round(2*M_PI*radius/segment_length);
	if(N < 10)
		N = 10;
	float dphi = 2*M_PI/N;
	Vect3f v0 = vc + Vect3f(radius,0,0);
	for(float phi = dphi;phi < 2*M_PI + dphi/2; phi += dphi)
	{
		Vect3f v1 = vc + Vect3f(cos(phi), sin(phi),0)*radius;
		gb_RenderDevice->DrawLine(v0, v1,color);
		v0 = v1;
	}
}

Vect3f G2S(const Vect3f &vg, Camera* camera)
{
	Vect3f pv,pe;
	camera->ConvertorWorldToViewPort(&vg,&pv,&pe);
	return Vect3f(pe.x, pe.y, pv.z);
}

void ShowDispatcher::Shape::show(Camera* camera)
{
	switch(type){
		case Point: {
			Vect3f vs = G2S(point, camera);
			if(vs.z > 1)
				clip_pixel(vs.xi(), vs.yi(), color, 1);
			} break;

		case Text: {
			Vect3f vs = G2S(pointX, camera);
			if(vs.z > 1)
				gb_RenderDevice->OutText(vs.xi(), vs.yi(), text.c_str(), Color4f(color));
			} break;

		case Text2D: {
				gb_RenderDevice->OutText(pointX.xi(), pointX.yi(), text.c_str(), Color4f(color));
			} break;

		case Circle: 
			clip_circle_3D(point, radius, color);
			break;

		case Delta: 
			clip_line_3D(point1, point1 + point2, color);
			break;

		case Line: 
			clip_line_3D(point1, point2, color);
			break;

		case Triangle: 
			clip_line_3D(points[0], points[1], color);
			clip_line_3D(points[1], points[2], color);
			clip_line_3D(points[2], points[0], color);
			break;

		case Quadrangle: 
			clip_line_3D(points[0], points[1], color);
			clip_line_3D(points[1], points[2], color);
			clip_line_3D(points[2], points[3], color);
			clip_line_3D(points[3], points[0], color);
			break;

		case ConvexArray:
			showConvex();
			//xassert(0);
			break;
		
		default:
			xassert(0);
		}
}

void ShowDispatcher::Shape::showConvex()
{
	for(int i = 0; i < n_points - 1; i++){
		clip_line_3D(points[i], points[i + 1], color);
		XBuffer buf;
		buf <= i;
		buf.SetDigits(6);
		//show_text(points[i], buf, color);
	}
	clip_line_3D(points[n_points-1], points[0], color);
}

void ShowDispatcher::clear() 
{ 
	MTAuto lock(lock_); 
	shapes.swap(shapesGraphics_);
	shapes.clear(); 
	need_font = false; 
}

void ShowDispatcher::draw(Camera* camera)
{
	MTAuto lock(lock_); 

	List::iterator i;
	FOR_EACH(shapesGraphics_, i)
		i -> show(camera);
}

void show_terrain_line( const Vect2f& p1, const Vect2f& p2, Color4c color )
{
	const float lineStep = 8;
	float lineLen = p1.distance(p2);
	int steps = round(lineLen/lineStep - 0.5);
	Vect2f dir = p2 - p1;
	dir.normalize(lineStep);
	Vect3f prevPoint;
	if(p1.x >= 0 && p1.x < vMap.H_SIZE && p1.y >= 0 && p1.y < vMap.V_SIZE)
		prevPoint = Vect3f(p1.x, p1.y, vMap.getApproxAlt(p1.x, p1.y));
	else
		prevPoint = Vect3f(p1.x, p1.y, 0);
	Vect3f nextPoint;
	for(int i=0;i<(steps-1);i++) {
		Vect2f nextPoint2f = p1 + dir * (i+1);
		if(nextPoint2f.x >= 0 && nextPoint2f.x < vMap.H_SIZE && nextPoint2f.y >=0 && nextPoint2f.y < vMap.V_SIZE)
			nextPoint = Vect3f(nextPoint2f.x, nextPoint2f.y, vMap.getApproxAlt(nextPoint2f.x, nextPoint2f.y));
		else
			nextPoint = Vect3f(nextPoint2f.x, nextPoint2f.y, 0);
		show_line(prevPoint, nextPoint, color);
		prevPoint = nextPoint;
	}
	Vect2f nextPoint2f = p2;
	if(nextPoint2f.x >= 0 && nextPoint2f.x < vMap.H_SIZE && nextPoint2f.y >=0 && nextPoint2f.y < vMap.V_SIZE)
		nextPoint = Vect3f(nextPoint2f.x, nextPoint2f.y, vMap.getApproxAlt(nextPoint2f.x, nextPoint2f.y));
	else
		nextPoint = Vect3f(nextPoint2f.x, nextPoint2f.y, 0);
	show_line(prevPoint, nextPoint, color);
}

////////////////////////////////////////////////////////////////////////////////////////////
//			Watch System
////////////////////////////////////////////////////////////////////////////////////////////

XBuffer& watch_buffer()
{
	static XBuffer buffer(1000, 1);
	return buffer;
}
