#ifndef __DEBUG_UTIL_H__
#define __DEBUG_UTIL_H__

#include "crc.h"
#include "XMath\Colors.h"
#include "Render\Inc\RenderMT.h"
#include "MTSection.h"
#include "Serialization\Range.h"

class Camera;

/////////////////////////////////////////////////////////////////////////////////
//		Отладочный вывод 3D с кэшированием
/////////////////////////////////////////////////////////////////////////////////
class ShowDispatcher
{
	class Shape {
		enum Type { Point, Text, Text2D, Circle, Delta, Line, Triangle, Quadrangle, ConvexArray };
		Type type;
		Color4c color;
		union {
			struct { Vect3f point; float radius; };
			struct { Vect3f point1, point2; };
		};
		Vect3f pointX; string text;
		int n_points; vector<Vect3f> points;

	public:	
		void validate(const Vect3f& v){const float inf=20000; xassert(v.x>=-inf && v.x<inf && v.y>=-inf && v.y<inf && v.z>=-inf && v.z<inf);}

		Shape(const Vect3f& v, Color4c color_) { validate(v);type = Point; point = v; color = color_; }
		Shape(const Vect3f& v, const char* text_, Color4c color_) { validate(v); type = Text; pointX = v; text = text_; color = color_; }
		Shape(const Vect2f& v, const char* text_, Color4c color_) { type = Text2D; pointX = Vect3f(v.x, v.y, 0.f); text = text_; color = color_; }
		Shape(const Vect3f& v, float radius_, Color4c color_) { validate(v); type = Circle; point = v; radius = radius_; color = color_; }
		Shape(const Vect3f& v0, const Vect3f& v1, Color4c color_, int line) { validate(v0);validate(v1);type = line ? Line : Delta; point1 = v0; point2 = v1; color = color_; }
		Shape(const Vect3f& v0, const Vect3f& v1, const Vect3f& v2, Color4c color_) {
			validate(v0);validate(v1);validate(v2);
			type = Triangle;
			points.push_back(v0);
			points.push_back(v1);
			points.push_back(v2);
			color = color_;
			n_points = points.size();
		}
		Shape(const Vect3f& v0, const Vect3f& v1, const Vect3f& v2, const Vect3f& v3, Color4c color_) {
			validate(v0);validate(v1);validate(v2);validate(v3);
			type = Quadrangle;
			points.push_back(v0);
			points.push_back(v1);
			points.push_back(v2);
			points.push_back(v3);
			color = color_;
			n_points = points.size();
		}
		Shape(int n_points_, const Vect3f* points_, Color4c color_) {
			type = ConvexArray;
			for(int i = 0; i < n_points_; ++i){
				points.push_back(points_[i]);
			}
			color = color_;
			n_points = points.size();
		}
		void show(Camera* camera);
		void showConvex();
		};

	typedef vector<Shape> List;
	List shapes;
	List shapesGraphics_;
	bool need_font;
	MTSection lock_;

public:
	void draw(Camera* camera);
	void clear();

	void point(const Vect3f& v, Color4c color)
			{ MTL(); shapes.push_back(Shape(v, color)); }
	void text(const Vect3f& v, const char* text, Color4c color)
			{ MTL(); shapes.push_back(Shape(v, text, color)); need_font = true; }
	void text2d(const Vect2f& v, const char* text, Color4c color)
			{ MTL(); shapes.push_back(Shape(v, text, color)); need_font = true; }
	void circle(const Vect3f& v, float radius, Color4c color)
			{ MTL(); shapes.push_back(Shape(v, radius, color)); }
	void line(const Vect3f &v0, const Vect3f &v1, Color4c color)
			{ MTL(); shapes.push_back(Shape(v0, v1, color, 1)); }
	void delta(const Vect3f& v, const Vect3f& dv, Color4c color)
			{ MTL(); shapes.push_back(Shape(v, dv, color, 0)); }
	void triangle(const Vect3f &v0, const Vect3f &v1, const Vect3f &v2, Color4c color)
			{ MTL(); shapes.push_back(Shape(v0, v1, v2, color)); }
	void quadrangle(const Vect3f &v0, const Vect3f &v1, const Vect3f &v2, const Vect3f &v3, Color4c color)
			{ MTL(); shapes.push_back(Shape(v0, v1, v2, v3, color)); }
	void convex(int n_points, const Vect3f* points, Color4c color)
			{ MTL(); shapes.push_back(Shape(n_points, points, color));  }
};

extern ShowDispatcher show_dispatcher;

inline void show_vector(const Vect3f& vg, Color4c color){ show_dispatcher.point(vg, color); }
inline void show_vector(const Vect3f& vg, float radius, Color4c color){ show_dispatcher.circle(vg, radius, color); }
inline void show_vector(const Vect3f& vg, const Vect3f& delta, Color4c color){ show_dispatcher.delta(vg, delta, color); }
inline void show_vector(const Vect3f &vg0, const Vect3f &vg1, const Vect3f &vg2, Color4c color){ show_dispatcher.triangle(vg0, vg1, vg2, color); }
inline void show_vector(const Vect3f &vg0, const Vect3f &vg1, const Vect3f &vg2, const Vect3f &vg3, Color4c color){ show_dispatcher.quadrangle(vg0, vg1, vg2, vg3, color); }
inline void show_convex(int n_points, const Vect3f* points, Color4c color){ show_dispatcher.convex(n_points, points, color); }
inline void show_line(const Vect3f &vg0, const Vect3f &vg1, Color4c color){ show_dispatcher.line(vg0, vg1, color); }
void show_terrain_line(const Vect2f& p1, const Vect2f& p2, Color4c color); // Медленная очень.
inline void show_text(const Vect3f& vg, const char* text, Color4c color){ show_dispatcher.text(vg, text, color); }
inline void show_text2d(const Vect2f& vg, const char* text, Color4c color){ show_dispatcher.text2d(vg, text, color); }

/////////////////////////////////////////////////////////////////////////////////
//		Determinacy Log
//
//  Log usage:
//
//		log_var(var);  
//		
//	or:	
//
//		#ifndef _FINAL_VERSION_
//		if(log_mode)
//			log_buffer <= var;
//		#endif
//
//	Command line arguments:
//
//	save_log [time_to_exit:seconds]
//	verify_log [append_log] [time_to_exit:seconds]
//
//	_FORCE_NET_LOG_ - to force network log under _FINAL_VERSION_
/////////////////////////////////////////////////////////////////////////////////
#define _FORCE_NET_LOG_
#if (!defined(_FINAL_VERSION_) || defined(_FORCE_NET_LOG_))
#define _DO_LOG_
#endif

XBuffer& watch_buffer();
template<class T> inline void watchBuffer(const T& t) { watch_buffer().init(); watch_buffer() <= t; watch_buffer() < char(0); }
inline void watchBuffer(const char* t) { watch_buffer().init(); watch_buffer() < t; watch_buffer() < char(0); }
inline void watchBuffer(const string& t) { watch_buffer().init(); watch_buffer() < t.c_str(); watch_buffer() < char(0); }

extern bool net_log_mode;
extern XBuffer net_log_buffer;

#ifdef _DO_LOG_
#define log_var_aux(var, file, line) { if(net_log_mode) { watchBuffer(var); net_log_buffer < file < ", " <= line < "] " < #var < ": " < watch_buffer() < "\n"; } }
#define log_var(var) log_var_aux(var, __FILE__, __LINE__)
#define log_var_crc(address, size) { log_var(crc32((const unsigned char*)address, size, startCRC32)) }
#else
#define log_var_aux(var, file, line)
#define log_var(var)
#define log_var_crc(address, size)	
#endif


/////////////////////////////////////////////////////////////////////////////////
//		Smart Log
/////////////////////////////////////////////////////////////////////////////////
#ifdef _FINAL_VERSION_
	class VoidStream
	{
	public:
		VoidStream(const char* name = 0, unsigned flags = 0){}
		void open(const char* name, unsigned flags){}
		bool isOpen() const { return true; }
		void close(){}
		template<class T> 
		VoidStream& operator< (const T&) { return *this; }
		template<class T> 
		VoidStream& operator<= (const T&) { return *this; }
		template<class T> 
		VoidStream& operator<< (const T&) { return *this; }
	};

	typedef VoidStream LogStream;
#else
	typedef XStream LogStream;
#endif

extern LogStream fout;


/////////////////////////////////////////////////////////////////////////////////
//		Utils
/////////////////////////////////////////////////////////////////////////////////
Vect3f To3D(const Vect2f& pos);
Vect2f clampWorldPosition(const Vect2f& pos, float radius);
Vect3f clampWorldPosition(const Vect3f& pos, float radius);

//--------------------------------------
extern RandomGenerator effectRND;//В графике используется graphRnd.
__declspec(selectany) RandomGenerator logicRnd;

#ifndef _FINAL_VERSION_

inline int logicRNDi(int x, const char* file, int line)
{
	MTL();
	log_var_aux(logicRnd.get(), file, line);
	return logicRnd(x);
}
inline int logicRNDii(int min, int max, const char* file, int line)
{
	MTL();
	log_var_aux(logicRnd.get(), file, line);
	return logicRnd(min, max);
}
inline float logicRNDf(float val, const char* file, int line)
{
	MTL();
	log_var_aux(logicRnd.get(), file, line);
	return logicRnd.frnd(val);
}
inline float logicRNDff(float min, float max, const char* file, int line)
{
	MTL();
	log_var_aux(logicRnd.get(), file, line);
	return logicRnd.fabsRnd(min, max);
}
inline float logicRNDfa(const char* file, int line, float val = 1.f)
{
	MTL();
	log_var_aux(logicRnd.get(), file, line);
	return logicRnd.fabsRnd(val);
}

#define logicRND(x) logicRNDi(x, __FILE__, __LINE__)
#define logicRNDinterval(min, max) logicRNDii(min, max, __FILE__, __LINE__)
//-1..+1
#define logicRNDfrnd(val) logicRNDf(val, __FILE__, __LINE__)
//0..+1
#define logicRNDfrand() logicRNDfa(__FILE__, __LINE__)
#define logicRNDfabsRnd(val) logicRNDfa(__FILE__, __LINE__, val)
#define logicRNDfabsRndInterval(min, max) logicRNDff((min), (max), __FILE__, __LINE__)

#else // _FINAL_VERSION_

#define logicRND(x) logicRnd(x)
#define logicRNDinterval(min, max) logicRnd(min, max)
//-1..+1
#define logicRNDfrnd(val) logicRnd.frnd(val)
//0..+1
#define logicRNDfrand() logicRnd.fabsRnd(1.f)
#define logicRNDfabsRnd(val) logicRnd.fabsRnd(val)
#define logicRNDfabsRndInterval(min, max) logicRnd.fabsRnd(min, max)

#endif // _FINAL_VERSION_

//--------------------------------------

bool isUnderEditor();

#endif // __DEBUG_UTIL_H__
