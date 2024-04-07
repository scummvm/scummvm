#include "qd_precomp.h"

#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "gr_dispatcher.h"

#include "qd_setup.h"
#include "qd_camera.h"
#include "qd_game_object_animated.h"
#include "qd_game_dispatcher.h"

struct sPlane4f {
	float A,B,C,D;
	sPlane4f()												{ }
	sPlane4f(float a,float b,float c,float d)				{ A=a,B=b,C=c,D=d; }
	sPlane4f(const Vect3f &a,const Vect3f &b,const Vect3f &c)
	{ // инициализация плоскости по трем точкам
		A=(b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z);
		B=(b.z-a.z)*(c.x-a.x)-(c.z-a.z)*(b.x-a.x);
		C=(b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y);
		GetNormal().normalize();
		D=-A*a.x-B*a.y-C*a.z;
	}
	__forceinline void Set(const Vect3f &a,const Vect3f &b,const Vect3f &c)
	{ // инициализация плоскости по трем точкам
		A=(b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z);
		B=(b.z-a.z)*(c.x-a.x)-(c.z-a.z)*(b.x-a.x);
		C=(b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y);
		GetNormal().normalize();
		D=-A*a.x-B*a.y-C*a.z;
	}
	__forceinline float GetDistance(const Vect3f &a)
	{ // расстояние от точки до плоскости
		float t=A*a.x+B*a.y+C*a.z+D;
		return t;
	}
	__forceinline float GetCross(const Vect3f &a,const Vect3f &b)
	{ // поиск пересечения данной плоскости с прямой заданной двумя точками a и b
		Vect3f v=a-b;
		float t=A*v.x+B*v.y+C*v.z;
		if(t==0) return 0;			// прямая и плоскость параллельны
		t=(A*a.x+B*a.y+C*a.z+D)/t;
		return t;
	}
	__forceinline Vect3f& GetNormal()							{ return *(Vect3f*)&A; }
	__forceinline void xform(MatXf &mat)
	{
		Vect3f p;
		mat.xformPoint(GetNormal()*D,p);
		GetNormal()=mat.xformVect(GetNormal());
		D=-A*p.x-B*p.y-C*p.z;
	}
	__forceinline void GetReflectionMatrix(const MatXf &in,MatXf &out)
	{ // out - ReflectionMatrix
		MatXf RefSurface(	Mat3f(	1-2*A*A,	-2*A*B,		-2*A*C,
									-2*B*A,		1-2*B*B,	-2*B*C,
									-2*C*A,		-2*C*B,		1-2*C*C),
							Vect3f(	-2*D*A,		-2*D*B,		-2*D*C));
		out=in*RefSurface;
	}
	__forceinline void GetReflectionVector(const Vect3f &in,Vect3f &out)
	{ // out - поиск отражение вектора от плоскости
		out=in-2*dot(GetNormal(),in)*GetNormal();
	}
};

const DWORD IMPASSIBLE_CELL_CLR		= RGB(255, 0, 0);
const DWORD SELECTED_CELL_CLR		= RGB(0, 0, 255);
const DWORD NORMAL_CELL_CLR		= RGB(255, 255, 255);

const int DASH_LEN = 2;

qdCamera* qdCamera::current_camera_ = NULL;
const Vect3f qdCamera::world_UP = Vect3f(0, 1, 0);
const Vect3f qdCamera::atPoint = Vect3f(0, 0, 0);
const float qdCamera::NEAR_PLANE = 1;
const float qdCamera::FAR_PLANE = 10000;

//qdCameraMode qdCamera::default_mode_;

qdCamera::qdCamera() : m_fR(300.0f), xAngle(45), yAngle(0), zAngle(0),
	GSX(0), GSY(0), Grid(NULL),
	cellSX(32), cellSY(32), focus(1000.0f),
	gridCenter(0,0,0),
	redraw_mode(QDCAM_GRID_ZBUFFER),
	scrOffset(0,0),
	current_mode_work_time_(0.0f),
	current_mode_switch_(false),
	current_object_(NULL),
	default_object_(NULL),
	scale_pow_(1.0f),
	scale_z_offset_(0.0f)
{
	set_grid_size(50,50);
	set_scr_size(640,480);
	set_scr_center(320,240);
	set_scr_center_initial(Vect2i(320,240));

	rotate_and_scale(xAngle,yAngle,zAngle,1,1,1);

	cycle_x_ = cycle_y_ = false;
}

qdCamera::~qdCamera()
{
	if(GSX){
		delete [] Grid;
	}
}

void qdCamera::set_grid_size(int xs, int ys)
{
	if(GSX == xs && GSY == ys) return;

	if(GSX)
		delete [] Grid;

	Grid = new sGridCell[xs * ys];

	GSX = xs; 
	GSY = ys;
}

void qdCamera::clear_grid()
{
	int cnt=0;
	for(int i=0; i<GSY; i++){
		for(int j=0; j<GSX; j++){
			Grid[cnt++].clear();
		}
	}
}
float qdCamera::get_scale(const Vect3f& glCoord) const
{
	if ((focus < 5000.0f) || (fabs(scale_pow_ - 1) > 0.001)) {
		Vect3f cameraCoord=global2camera_coord(glCoord);
        float buf = cameraCoord.z + scale_z_offset_;
		// Если координата отрицательна, то масштабирование происходит по линейному
		// закону. Иначе по общему (степенному) закону.
		if (buf > 0)
			buf = exp(scale_pow_*log(buf));

		float scale = (focus/(buf + focus));
		if(scale < 0)
			return 0;
		return scale;
	}
	else 
		return 1.0f;
}

const Vect2s qdCamera::scr2rscr(const Vect2s& v) const
{		 
	Vect2s res;
#ifdef _QUEST_EDITOR
	res.x = v.x - scrCenter.x ;
	res.y = scrCenter.y  - v.y;
#else
	res.x = v.x - (scrCenter.x - scrOffset.x);
	res.y = (scrCenter.y - scrOffset.y) - v.y;
#endif
	return res;
}

const Vect2s qdCamera::rscr2scr(const Vect2s& v) const
{
	Vect2s res;
#ifdef _QUEST_EDITOR
	res.x=scrCenter.x + v.x;
	res.y=scrCenter.y - v.y;
#else
	res.x=scrCenter.x + v.x - scrOffset.x;
	res.y=scrCenter.y - v.y - scrOffset.y;
#endif
	return res;
}

const Vect3f qdCamera::camera_coord2global(const Vect3f &v) const{
	return TransformVector(v, MatrixInverse(m_cam));
}

const Vect3f qdCamera::scr2global(const Vect2s &vScrPoint, float zInCameraCoord) const
{
	return rscr2global(scr2rscr(vScrPoint), zInCameraCoord);
}

const Vect3f qdCamera::rscr2global(const Vect2s rScrPoint, const float zInCameraCoord) const{
	//Преобразование экран - координаты в системе камеры
	Vect3f _t = rscr2camera_coord(rScrPoint, zInCameraCoord);
	//Преобразование координаты в системе камеры - в координаты глобальные(в системе основной плоскости)
	return camera_coord2global(_t);
};

const Vect3f qdCamera::global2camera_coord(const Vect3f& glCoord) const
{
	return TransformVector(glCoord, m_cam);
};

const Vect3f qdCamera::rscr2camera_coord(const Vect2s& rScrPoint, float z) const{
	float x=((float)rScrPoint.x * (z+focus)) /focus;
	float y=((float)rScrPoint.y * (z+focus)) /focus;
	return Vect3f(x,y,z);
};

const Vect2s qdCamera::camera_coord2rscr(const Vect3f& coord) const{
	short sx=round(coord.x*focus/(coord.z+focus));
	short sy=round(coord.y*focus/(coord.z+focus));
	return Vect2s(sx, sy);
};

const Vect2s qdCamera::camera_coord2scr(const Vect3f& coord) const{
	return rscr2scr(camera_coord2rscr(coord));
};

const Vect2s qdCamera::global2scr(const Vect3f& glCoord) const{
	return camera_coord2scr(global2camera_coord(glCoord));
};

const Vect2s qdCamera::global2rscr(const Vect3f& glCoord) const{
	return camera_coord2rscr(global2camera_coord(glCoord));
};

void qdCamera::set_R(const float r)
{
	m_fR = r;
	rotate_and_scale(xAngle, yAngle, zAngle, 1,1,1);
}

bool qdCamera::line_cutting(Vect3f& b, Vect3f& e) const
{
	//положение по Z плоскости отсечения
	const float D = -focus*.9f;
	if(b.z < D)//первая лежит позади
	{
		if(e.z < D)//обе точки лежат позади
			return false;
		float k = (D - b.z)/(e.z - b.z);
		b.z = D; b.y = k*(e.y - b.y) + b.y; 	b.x = k*(e.x - b.x) + b.x;
	}
	else if(e.z < D)
	{
		float k = (D - e.z)/(b.z - e.z);
		e.z = D; e.y = k*(b.y - e.y) + e.y; 	e.x = k*(b.x - e.x) + e.x;
	}
	return true;
}

void qdCamera::rotate_and_scale(float XA, float YA, float ZA, float kX, float kY, float kZ)
{
	xAngle=XA;
	yAngle=YA;
	zAngle=ZA;
	MATRIX3D rot = RotateXMatrix(XA*(M_PI/180.f));
	rot = MatrixMult(RotateYMatrix(-YA*(M_PI/180.f)), rot);
	rot = MatrixMult(RotateZMatrix(-ZA*(M_PI/180.f)), rot);
	//точка, из которой мы сомотрим
	const Vect3f camPos(0, 0, m_fR);
	//новая позиция камеры после поворота
	Vect3f pos = TransformVector(camPos, rot);

	//вычисляем, как измениться нормальный вектор камеры после поворота
	Vect3f new_up = TransformVector(world_UP, rot);

	m_cam = ViewMatrix(pos, atPoint,world_UP, new_up);
}

const Vect3f qdCamera::rscr2plane_camera_coord(const Vect2s& scrPoint) const
{
	const int XSP=cellSX*GSX;
	const int YSP=cellSY*GSY;
	const float XSP05=XSP*0.5f;
	const float YSP05=YSP*0.5f;
	Vect3f p0 = global2camera_coord(Vect3f(-XSP05, -YSP05, 0) + gridCenter);
	Vect3f p1 = global2camera_coord(Vect3f(-XSP05, +YSP05, 0) + gridCenter);
	Vect3f p2 = global2camera_coord(Vect3f(+XSP05, +YSP05, 0) + gridCenter);
	
	sPlane4f plnT(p0, p1, p2);
	Vect3f tlV((float)scrPoint.x, (float)scrPoint.y, focus);
	Vect3f tlP(0, 0, -focus);
	float t=-(plnT.A*tlP.x + plnT.B*tlP.y + plnT.C*tlP.z + plnT.D)/
		(plnT.A*tlV.x + plnT.B*tlV.y + plnT.C*tlV.z);

	float x=tlP.x + tlV.x*t; 
	float y=tlP.y + tlV.y*t; 
	float z=tlP.z + tlV.z*t;
	return Vect3f(x,y,z);
}

const Vect3f qdCamera::scr2plane_camera_coord(const Vect2s& scrPoint) const
{
	return rscr2plane_camera_coord(scr2rscr(scrPoint));
}

const Vect3f qdCamera::scr2plane(const Vect2s& scrPoint) const
{
	return camera_coord2global(scr2plane_camera_coord(scrPoint));
}

const Vect3f qdCamera::rscr2plane(const Vect2s& rscrPoint) const
{
	return camera_coord2global(rscr2plane_camera_coord(rscrPoint));
}

const Vect2s qdCamera::plane2scr(const Vect3f& plnPoint) const
{
	return rscr2scr(plane2rscr(plnPoint));
}

const Vect2s qdCamera::plane2rscr(const Vect3f &plnPoint) const
{
	const float SMALL_VALUE=0.0001f;
	const int XSP=cellSX*GSX;
	const int YSP=cellSY*GSY;
	const float XSP05=XSP/2.f;
	const float YSP05=YSP/2.f;
	
	Vect3f res = global2camera_coord(plnPoint);
	
	if(res.z<(SMALL_VALUE-focus)) return Vect2s(0, 0);
	
	int sx0=round(res.x*focus/(res.z+focus));
	int sy0=round(res.y*focus/(res.z+focus));
	
	return Vect2s(sx0, sy0);
}

const sGridCell* qdCamera::get_cell(float _x, float _y) const
{
	int x=round(_x - gridCenter.x);
	int y=round(_y - gridCenter.y);

	const int XSP=cellSX*GSX;
	const int YSP=cellSY*GSY;
	const int XSP05=XSP/2;
	const int YSP05=YSP/2;

	x+=XSP05; y+=YSP05;
	if(x<0 || x>=XSP || y<0 || y>=YSP) return 0;
	x=x/cellSX; y=y/cellSY;
	return &Grid[y*GSX+x];
}

const Vect2s qdCamera::get_cell_index(float _x, float _y, bool grid_crop) const
{
	int x=round(_x - gridCenter.x);
	int y=round(_y - gridCenter.y);

	const int XSP=cellSX*GSX;
	const int YSP=cellSY*GSY;
	const int XSP05=XSP>>1;
	const int YSP05=YSP>>1;
	x+=XSP05; y+=YSP05;

	if(grid_crop && (x<0 || x>=XSP || y<0 || y>=YSP))
		return Vect2s(-1,-1);

	return Vect2s(x/cellSX
#ifdef _QUEST_EDITOR
		- static_cast<int>(x < 0)
#endif // _QUEST_EDITOR
		,y/cellSY
#ifdef _QUEST_EDITOR
		- static_cast<int>(y < 0)
#endif // _QUEST_EDITOR
		);
}

const Vect2s qdCamera::get_cell_index(const Vect3f& v, bool grid_crop) const
{
	return get_cell_index(v.x, v.y, grid_crop);
}

const Vect3f qdCamera::get_cell_coords(int _x_idx,int _y_idx) const
{
	//float xx = (_x_idx - (GSX>>1)) * cellSX + (cellSX>>1) + gridCenter.x;
	//float yy = (_y_idx - (GSY>>1)) * cellSY + (cellSY>>1) + gridCenter.y;

	float xx = (_x_idx - static_cast<float>(GSX)/2 + 0.5)*cellSX + gridCenter.x;
	float yy = (_y_idx - static_cast<float>(GSY)/2 + 0.5)*cellSY + gridCenter.y;

	return Vect3f(xx,yy,gridCenter.z);
}

const Vect3f qdCamera::get_cell_coords(const Vect2s& idxs) const
{
	return get_cell_coords(idxs.x, idxs.y);
}

void qdCamera::reset_all_select()
{
	int cnt=0;
	for(int i=0; i<GSY; i++){
		for(int j=0; j<GSX; j++){
			Grid[cnt++].deselect();
		}
	}
}

bool qdCamera::select_cell(int x, int y)
{
	const int XSP=cellSX*GSX;
	const int YSP=cellSY*GSY;
	const int XSP05=XSP>>1;
	const int YSP05=YSP>>1;
	
	x += XSP05 - gridCenter.x;
	y += YSP05 - gridCenter.y;
	
	if(x<0 || x>=XSP || y<0 || y>=YSP) return false;
	x=x/cellSX; y=y/cellSY;
	Grid[y*GSX+x].select();
	return true;
}

bool qdCamera::deselect_cell(int x, int y)
{
	const int XSP=cellSX*GSX;
	const int YSP=cellSY*GSY;
	const int XSP05=XSP>>1;
	const int YSP05=YSP>>1;
	
	x += XSP05 - gridCenter.x;
	y += YSP05 - gridCenter.y;
	
	if(x<0 || x>=XSP || y<0 || y>=YSP) return false;
	x=x/cellSX; y=y/cellSY;
	Grid[y*GSX+x].deselect();
	return true;
}

void qdCamera::load_script(const xml::tag* p)
{
	int x,y;
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		xml::tag_buffer buf(*it);

		switch(it -> ID()){
		case QDSCR_CAMERA_GRID_SIZE:
			buf > x > y;
			set_grid_size(x,y);
			break;
		case QDSCR_CAMERA_CELL_SIZE:
			buf > cellSX > cellSY;
			break;
		case QDSCR_CAMERA_SCREEN_SIZE:
			buf > x > y;
			set_scr_size(x,y);
			set_scr_center(x/2,y/2);
			set_scr_center_initial(Vect2i(x/2,y/2));
			break;
		case QDSCR_CAMERA_SCREEN_OFFSET:
			buf > x > y;
			set_scr_offset(Vect2i(x,y));
			break;
		case QDSCR_CAMERA_SCREEN_CENTER:
			buf > x > y;
			set_scr_center_initial(Vect2i(x,y));
			set_scr_center(x,y);
			break;
		case QDSCR_CAMERA_FOCUS:
			buf > focus;
			break;
		case QDSCR_CAMERA_ANGLES:
			buf > xAngle > yAngle > zAngle;
			rotate_and_scale(xAngle,yAngle,zAngle,1,1,1);
			break;
		case QDSCR_CAMERA_GRID_CENTER: {
				Vect3f v;
				buf > v.x > v.y > v.z;
				set_grid_center(v);
			}
			break;
		case QDSCR_POS3D:
			buf.get_float();
			buf.get_float();
			set_R(buf.get_float());
			break;
		case QDSCR_CAMERA_SCALE_POW:
			buf > scale_pow_;
			break;
		case QDSCR_CAMERA_SCALE_Z_OFFSET:
			buf > scale_z_offset_;
			break;
		}
	}

	rotate_and_scale(xAngle,yAngle,zAngle,1,1,1);
}

bool qdCamera::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<camera";

	fh < " camera_grid_size=\"" <= GSX < " " <= GSY < "\"";
	
	fh < ">\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_cell_size>" <= cellSX < " " <= cellSY < "</camera_cell_size>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<pos_3d>" <= 0L < " " <= 0L < " " <= get_R() < "</pos_3d>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_focus>" <= focus < "</camera_focus>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_angles>" <= xAngle < " " <= yAngle < " " <= zAngle < "</camera_angles>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_screen_size>" <= scrSize.x < " " <= scrSize.y < "</camera_screen_size>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_screen_offset>" <= scrOffset.x < " " <= scrOffset.y < "</camera_screen_offset>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_screen_center>" <= scrCenterInitial.x < " " <= scrCenterInitial.y < "</camera_screen_center>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_grid_center>" <= gridCenter.x < " " <= gridCenter.y < " " <= gridCenter.z < "</camera_grid_center>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_scale_pow>" <= scale_pow() < "</camera_scale_pow>\r\n";

	for(int i = 0; i <= indent; i ++) fh < "\t";
	fh < "<camera_scale_z_offset>" <= scale_z_offset() < "</camera_scale_z_offset>\r\n";

	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "</camera>\r\n";

	return true;
}

const Vect2i qdCamera::screen_center_limit_x() const
{
	int x0,x1;
	if(scrSize.x < qdGameConfig::get_config().screen_sx()){
		x0 = x1 = qdGameConfig::get_config().screen_sx()/2;
	}
	else {
		x0 = -scrSize.x/2 + qdGameConfig::get_config().screen_sx();
		x1 = scrSize.x/2;
	}

	if(cycle_x_){
		x0 -= scrSize.x;
		x1 += scrSize.x;
	}

	return Vect2i(x0,x1);
}

const Vect2i qdCamera::screen_center_limit_y() const
{
	int y0,y1;
	if(scrSize.y < qdGameConfig::get_config().screen_sy()){
		y0 = y1 = qdGameConfig::get_config().screen_sy()/2;
	}
	else {
		y0 = -scrSize.y/2 + qdGameConfig::get_config().screen_sy();
		y1 = scrSize.y/2;
	}

	if(cycle_y_){
		y0 -= scrSize.y;
		y1 += scrSize.y;
	}

	return Vect2i(y0,y1);
}

void qdCamera::move_scr_center(int dxc, int dyc)
{
	scrCenter.x += dxc;
	scrCenter.y += dyc;

#ifndef _QUEST_EDITOR
	clip_center_coords(scrCenter.x,scrCenter.y);
#endif
}

float qdCamera::scrolling_phase_x() const
{
	if(scrSize.x <= qdGameConfig::get_config().screen_sx())
		return 0.0f;
	else 
		return float(scrCenter.x*2 + scrSize.x - qdGameConfig::get_config().screen_sx()*2) / float(scrSize.x - qdGameConfig::get_config().screen_sx()) - 1.0f;
}

float qdCamera::scrolling_phase_y() const
{
	if(scrSize.y <= qdGameConfig::get_config().screen_sy())
		return 0.0f;
	else 
		return float(scrCenter.y*2 + scrSize.y - qdGameConfig::get_config().screen_sy()*2) / float(scrSize.x - qdGameConfig::get_config().screen_sy()) - 1.0f;
}

bool qdCamera::draw_grid() const
{
	if(redraw_mode == QDCAM_GRID_NONE) return true;

	int i,j;
	int cnt=0;

	const int XSP=cellSX*GSX;
	const int YSP=cellSY*GSY;
	const float XSP05=XSP/2.f;
	const float YSP05=YSP/2.f;

	for(cnt = 0, i=0; i<GSY; ++i){
		for(int j=0; j<GSX; ++j){
			if(!Grid[cnt].is_walkable()) 
				draw_cell( j, i, 0, 1,IMPASSIBLE_CELL_CLR);

			if(Grid[cnt].is_selected() || Grid[cnt].check_attribute(sGridCell::CELL_OCCUPIED | sGridCell::CELL_PERSONAGE_OCCUPIED))
				draw_cell( j, i, 0, 1,SELECTED_CELL_CLR);
			++cnt;
		}
	}
	
	if(redraw_mode == QDCAM_GRID_ZBUFFER){
		for(i=0; i<=GSX; i++){
			for(j=0; j<GSY; j++){
				Vect3f begPoint(-XSP05+i*cellSX, -YSP05+j*cellSY, 0);
				Vect3f endPoint(-XSP05+i*cellSX, -YSP05+(j+1)*cellSY, 0);
				begPoint = global2camera_coord(begPoint + gridCenter);
				endPoint = global2camera_coord(endPoint + gridCenter);
				if(line_cutting(begPoint, endPoint))
				{
					Vect2s b = camera_coord2scr(begPoint);
					Vect2s e = camera_coord2scr(endPoint);

					#ifdef _GR_ENABLE_ZBUFFER
					grDispatcher::instance() -> Line_z(b.x, b.y, begPoint.z, e.x, e.y, endPoint.z, NORMAL_CELL_CLR, DASH_LEN);
					#else
					grDispatcher::instance() -> Line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
					#endif
				}
			}
		}
		for(i=0; i<=GSY; i++){
			for(j=0; j<GSX; j++){
				Vect3f begPoint(-XSP05+j*cellSX, -YSP05+i*cellSY, 0);
				Vect3f endPoint(-XSP05+(j+1)*cellSX, -YSP05+i*cellSY, 0);
				begPoint = global2camera_coord(begPoint + gridCenter);
				endPoint = global2camera_coord(endPoint + gridCenter);
				if(line_cutting(begPoint, endPoint))
				{
					Vect2s b = camera_coord2scr(begPoint);
					Vect2s e = camera_coord2scr(endPoint);

					#ifdef _GR_ENABLE_ZBUFFER
					grDispatcher::instance() -> Line_z(b.x, b.y, begPoint.z, e.x, e.y, endPoint.z, NORMAL_CELL_CLR, DASH_LEN);
					#else
					grDispatcher::instance() -> Line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
					#endif
				}
			}
		}
	}
	else {
		for(i=0; i<=GSX; i++){
			Vect3f begPoint(-XSP05+i*cellSX, -YSP05, 0);
			Vect3f endPoint(-XSP05+i*cellSX, +YSP05, 0);
			begPoint = global2camera_coord(begPoint + gridCenter);
			endPoint = global2camera_coord(endPoint + gridCenter);
			if(line_cutting(begPoint, endPoint))
			{
				Vect2s b = camera_coord2scr(begPoint);
				Vect2s e = camera_coord2scr(endPoint);

				grDispatcher::instance() -> Line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
			}
		}
		
		for(i=0; i<=GSY; i++){
			Vect3f begPoint(-XSP05, -YSP05+i*cellSY, 0);
			Vect3f endPoint(+XSP05, -YSP05+i*cellSY, 0);
			begPoint = global2camera_coord(begPoint + gridCenter);
			endPoint = global2camera_coord(endPoint + gridCenter);
			if(line_cutting(begPoint, endPoint))
			{
				Vect2s b = camera_coord2scr(begPoint);
				Vect2s e = camera_coord2scr(endPoint);
				
				grDispatcher::instance()->Line(b.x, b.y, e.x, e.y, NORMAL_CELL_CLR, DASH_LEN);
			}
		}
	}

	cnt=0;
	for(i=0; i<GSY; i++){
		for(int j=0; j<GSX; j++, cnt++){
			if(Grid[cnt].height()) {
				draw_cell(j, i, Grid[cnt].height(), 1,RGB(255,255,255));
			}
		}
	}

	return true;
}

bool qdCamera::draw_cell(int x,int y,int z,int penWidth,unsigned color) const
{
	const float SMALL_VALUE=0.0001f;
	const int XSP=get_cell_sx()*get_grid_sx();
	const int YSP=get_cell_sy()*get_grid_sy();
	const float XSP05=XSP*0.5f;
	const float YSP05=YSP*0.5f;
	const int offset = 2;
	
	Vect3f point0((float)(x*get_cell_sx() - XSP05 + offset), (float)(y*get_cell_sy() - YSP05 + offset), (float)z);
	Vect3f point1((float)((x+1)*get_cell_sx() - XSP05 - offset), (float)(y*get_cell_sy() - YSP05 + offset), (float)z);
	Vect3f point2((float)((x+1)*get_cell_sx() - XSP05 - offset), (float)((y+1)*get_cell_sy() - YSP05 - offset), (float)z);
	Vect3f point3((float)(x*get_cell_sx() - XSP05 + offset), (float)((y+1)*get_cell_sy() - YSP05 - offset), (float)z);
	
	point0 = global2camera_coord(point0 + gridCenter);
	point1 = global2camera_coord(point1 + gridCenter);
	point2 = global2camera_coord(point2 + gridCenter);
	point3 = global2camera_coord(point3 + gridCenter);
	
	if(redraw_mode == QDCAM_GRID_ZBUFFER){
		if(line_cutting(point0, point1))
		{
			Vect2s p0=camera_coord2scr(point0);
			Vect2s p1=camera_coord2scr(point1);

			#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point0.z, p1.x, p1.y, point1.z, color/*, DASH_LEN*/); 
			#else
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
			#endif
		}
		
		if(line_cutting(point1, point2))
		{
			Vect2s p0=camera_coord2scr(point1);
			Vect2s p1=camera_coord2scr(point2);

			#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point1.z, p1.x, p1.y, point2.z, color/*, DASH_LEN*/); 
			#else
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
			#endif
		}
		
		if(line_cutting(point2, point3))
		{
			Vect2s p0=camera_coord2scr(point2);
			Vect2s p1=camera_coord2scr(point3);

			#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point2.z, p1.x, p1.y, point3.z, color/*, DASH_LEN*/); 
			#else
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
			#endif
		}
		
		if(line_cutting(point3, point0))
		{
			Vect2s p0=camera_coord2scr(point3);
			Vect2s p1=camera_coord2scr(point0);

			#ifdef _GR_ENABLE_ZBUFFER
			grDispatcher::instance()->Line_z(p0.x, p0.y, point3.z, p1.x, p1.y, point0.z, color/*, DASH_LEN*/); 
			#else
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
			#endif
		}
	}
	else {
		if(line_cutting(point0, point1))
		{
			Vect2s p0=camera_coord2scr(point0);
			Vect2s p1=camera_coord2scr(point1);
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
		}
		
		if(line_cutting(point1, point2))
		{
			Vect2s p0=camera_coord2scr(point1);
			Vect2s p1=camera_coord2scr(point2);
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
		}
		
		if(line_cutting(point2, point3))
		{
			Vect2s p0=camera_coord2scr(point2);
			Vect2s p1=camera_coord2scr(point3);
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
		}
		
		if(line_cutting(point3, point0))
		{
			Vect2s p0=camera_coord2scr(point3);
			Vect2s p1=camera_coord2scr(point0);
			grDispatcher::instance()->Line(p0.x, p0.y, p1.x, p1.y, color/*, DASH_LEN*/); 
		}
	}
	return true;
}

void qdCamera::scale_grid(int sx,int sy,int csx,int csy)
{
	if(GSX == sx && GSY == sy) return;

	sGridCell* new_grid = new sGridCell[sx * sy];

	if(GSX){
		if(GSX >= sx && GSY >= sy){
			int dx = GSX/sx;
			int dy = GSY/sy;

			sGridCell* new_p = new_grid;
			sGridCell* old_p = Grid;

			for(int i = 0; i < sy; i ++){
				for(int j = 0; j < sx; j ++){
					int attr_count = 0;
					int height_sum = 0;

					for(int y = 0; y < dy; y ++){
						for(int x = 0; x < dx; x ++){
							if(!old_p[j * dx + x + y * GSX].is_walkable())
								attr_count ++;

							height_sum += old_p[j * dx + x + y * GSX].height();
						}
					}
					if(attr_count >= dx * dy / 2)
						new_p -> make_impassable();

					new_p -> set_height(height_sum / (dx * dy));

					new_p ++;
				}
				old_p += GSX * dy;
			}
		}
		if(GSX <= sx && GSY <= sy){
			int dx = sx/GSX;
			int dy = sy/GSY;

			sGridCell* new_p = new_grid;
			sGridCell* old_p = Grid;

			for(int i = 0; i < GSY; i ++){
				for(int j = 0; j < GSX; j ++){
					for(int y = 0; y < dy; y ++){
						for(int x = 0; x < dx; x ++)
							new_p[j * dx + x + y * sx] = *old_p;
					}
					old_p ++;
				}
				new_p += sx * dy;
			}
		}

		delete [] Grid;
	}

	Grid = new_grid;

	GSX = sx; 
	GSY = sy;

	cellSX = csx;
	cellSY = csy;
}

void qdCamera::resize_grid(int sx,int sy)
{
	if(GSX == sx && GSY == sy) return;

	sGridCell* new_grid = new sGridCell[sx * sy];

	if(GSX){
		int x0 = (sx - GSX)/2;
		int y0 = (sy - GSY)/2;

		for(int y = 0; y < GSY; y ++){
			for(int x = 0; x < GSX; x ++){
				if(x + x0 >= 0 && x + x0 < sx && y + y0 >= 0 && y + y0 < sy)
					new_grid[x + x0 + (y + y0) * sx] = Grid[x + y * GSX];
			}
		}
		
		delete [] Grid;
	}

	Grid = new_grid;

	GSX = sx; 
	GSY = sy;
}

sGridCell* qdCamera::backup(sGridCell* ptrBuff)
{
	memcpy(ptrBuff, Grid, sizeof(sGridCell)*GSX*GSY);
	return ptrBuff;
}

bool qdCamera::restore(sGridCell* grid, int sx, int sy, int csx, int csy)
{
	if(Grid)
		delete [] Grid;
	Grid  = new sGridCell[sx*sy];
	if(!Grid)
		return false;
	memcpy(Grid, grid, sizeof(sGridCell)*sx*sy);
	
	GSX = sx;
	GSY = sy;
	cellSX = csx;
	cellSY = csy;
	
	return true;
}

bool qdCamera::set_grid_cell(const Vect2s& cell_pos,const sGridCell& cell)
{
	if(cell_pos.x >= 0 && cell_pos.x < GSX && cell_pos.y >= 0 && cell_pos.y < GSY){
		Grid[cell_pos.x + cell_pos.y * GSX] = cell;
		return true;
	}
	
	return false;
}

bool qdCamera::set_grid_cell_attributes(const Vect2s& cell_pos,int attr)
{
	if(cell_pos.x >= 0 && cell_pos.x < GSX && cell_pos.y >= 0 && cell_pos.y < GSY){
		Grid[cell_pos.x + cell_pos.y * GSX].set_attributes(attr);
		return true;
	}
	
	return false;
}

bool qdCamera::restore_grid_cell(const Vect2s cell_pos)
{
	if(cell_pos.x >= 0 && cell_pos.x < GSX && cell_pos.y >= 0 && cell_pos.y < GSY){
		sGridCell cl;
		cl.make_impassable();
		Grid[cell_pos.x + cell_pos.y * GSX] = cl;
		return true;
	}
	
	return false;
}

sGridCell* qdCamera::get_cell(const Vect2s& cell_pos)
{
	if(cell_pos.x >= 0 && cell_pos.x < GSX && cell_pos.y >= 0 && cell_pos.y < GSY){
		return &Grid[cell_pos.x + cell_pos.y * GSX];
	}
	return NULL;
}

const sGridCell* qdCamera::get_cell(const Vect2s& cell_pos) const
{
	if(cell_pos.x >= 0 && cell_pos.x < GSX && cell_pos.y >= 0 && cell_pos.y < GSY){
		return &Grid[cell_pos.x + cell_pos.y * GSX];
	}
	return NULL;
}

bool qdCamera::load_data(qdSaveStream& fh,int save_version)
{
	int x,y;
	char flag;
	fh > scrCenter.x > scrCenter.y > x > y > current_mode_work_time_ > flag;
	current_mode_switch_ = bool(flag);

	if(x != GSX || y != GSY) return false;

	if(!current_mode_.load_data(fh,save_version)) return false;
	if(!default_mode_.load_data(fh,save_version)) return false;

	fh > flag;
	if(flag){
		qdNamedObjectReference ref;
		if(!ref.load_data(fh,save_version)) return false;
		current_object_ = dynamic_cast<qdGameObjectAnimated*>(qdGameDispatcher::get_dispatcher() -> get_named_object(&ref));
	}

	fh > flag;
	if(flag){
		qdNamedObjectReference ref;
		if(!ref.load_data(fh,save_version)) return false;
		default_object_ = dynamic_cast<qdGameObjectAnimated*>(qdGameDispatcher::get_dispatcher() -> get_named_object(&ref));
	}

	return true;
}

bool qdCamera::save_data(qdSaveStream& fh) const
{
	fh < scrCenter.x < scrCenter.y < GSX < GSY < current_mode_work_time_ < char(current_mode_switch_);

	current_mode_.save_data(fh);
	default_mode_.save_data(fh);

	if(current_object_){
		fh < char(1);
		qdNamedObjectReference ref(current_object_);
		ref.save_data(fh);
	}
	else
		fh < char(0);

	if(default_object_){
		fh < char(1);
		qdNamedObjectReference ref(default_object_);
		ref.save_data(fh);
	}
	else
		fh < char(0);

	return true;
}

bool qdCamera::set_mode(const qdCameraMode& mode,qdGameObjectAnimated* object)
{
	current_mode_ = mode;
	current_object_ = object;

	current_mode_work_time_ = 0.0f;
	current_mode_switch_ = current_mode_.smooth_switch();

	return true;
}

bool qdCamera::quant(float dt)
{
	Vect2i last_pos = scrCenter;

#ifndef _QUEST_EDITOR
	qdGameObjectAnimated* p = current_object_;
	if(!p) p = default_object_;

	if(p)
		p -> qdGameObject::update_screen_pos();

	switch(current_mode_.camera_mode()){
	case qdCameraMode::MODE_CENTER_OBJECT:
		if(p){
			Vect2i r = p -> screen_pos() + current_mode_.center_offset();

			int cx = scrCenter.x + qdGameConfig::get_config().screen_sx()/2 - r.x;
			int cy = scrCenter.y + qdGameConfig::get_config().screen_sy()/2 - r.y;

			clip_center_coords(cx,cy);

			int dx = cx - scrCenter.x;
			int dy = cy - scrCenter.y;

			if(current_mode_switch_){
				Vect2f dr(dx,dy);

				float dr0 = current_mode_.scrolling_speed() * dt;

				if(dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);
				else
					current_mode_switch_ = false;
				
				move_scr_center(dr.xi(),dr.yi());
			}
			else
				move_scr_center(dx,dy);
		}
		break;
	case qdCameraMode::MODE_OBJECT_ON_SCREEN:
		if(p){
			Vect2s r = p -> screen_pos();
			float sz = p -> radius();
			
			int dx = 0;
			int dy = 0;
			if(r.x + sz + current_mode_.scrolling_distance() >= qdGameConfig::get_config().screen_sx()){
				dx = qdGameConfig::get_config().screen_sx() - (r.x + sz + current_mode_.scrolling_distance());
			}
			else {
				if(r.x - sz - current_mode_.scrolling_distance() < 0)
					dx = -r.x + sz + current_mode_.scrolling_distance();
			}
			
			if(r.y + sz + current_mode_.scrolling_distance() >= qdGameConfig::get_config().screen_sy()){
				dy = qdGameConfig::get_config().screen_sy() - (r.y + sz + current_mode_.scrolling_distance());
			}
			else {
				if(r.y - sz - current_mode_.scrolling_distance() < 0)
					dy = -r.y + sz + current_mode_.scrolling_distance();
			}

			if(current_mode_switch_){
				int cx = scrCenter.x + dx;
				int cy = scrCenter.y + dy;

				clip_center_coords(cx,cy);

				dx = cx - scrCenter.x;
				dy = cy - scrCenter.y;

				Vect2f dr(dx,dy);

				float dr0 = current_mode_.scrolling_speed() * dt;

				if(dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);
				else
					current_mode_switch_ = false;
				
				move_scr_center(dr.xi(),dr.yi());
			}
			else
				move_scr_center(dx,dy);
		}
		break;
	case qdCameraMode::MODE_FOLLOW_OBJECT:
		if(p){
			Vect2s r = p -> screen_pos() + current_mode_.center_offset();
			int dx = -r.x + qdGameConfig::get_config().screen_sx()/2;
			int dy = -r.y + qdGameConfig::get_config().screen_sy()/2;
			
			if(dx || dy){
				Vect2f dr(dx,dy);

				float dr0 = current_mode_.scrolling_speed() * dt;

				if(dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);
				
				move_scr_center(dr.xi(),dr.yi());
			}
			current_mode_switch_ = false;
		}
		break;
	case qdCameraMode::MODE_CENTER_OBJECT_WHEN_LEAVING:
		if(p){
			Vect2s r = p -> screen_pos() + current_mode_.center_offset();
			float sz = p -> radius();
			
			int dx = 0;
			int dy = 0;
			if(r.x + sz + current_mode_.scrolling_distance() >= qdGameConfig::get_config().screen_sx()){
				dx = qdGameConfig::get_config().screen_sx() - (r.x + sz + current_mode_.scrolling_distance());
			}
			else {
				if(r.x - sz - current_mode_.scrolling_distance() < 0)
					dx = -r.x + sz + current_mode_.scrolling_distance();
			}
			
			if(r.y + sz + current_mode_.scrolling_distance() >= qdGameConfig::get_config().screen_sy()){
				dy = qdGameConfig::get_config().screen_sy() - (r.y + sz + current_mode_.scrolling_distance());
			}
			else {
				if(r.y - sz - current_mode_.scrolling_distance() < 0)
					dy = -r.y + sz + current_mode_.scrolling_distance();
			}

			if(dx || dy){
				int dx1 = -r.x + qdGameConfig::get_config().screen_sx()/2;
				int dy1 = -r.y + qdGameConfig::get_config().screen_sy()/2;

				Vect2f dr(dx1,dy1);

				float dr0 = current_mode_.scrolling_speed() * dt;

				if(dr.norm2() > dr0 * dr0)
					dr.normalize(dr0);

				move_scr_center(dr.xi(),dr.yi());
			}

			current_mode_switch_ = false;
		}
		break;
	}

	if(p)
		p -> update_screen_pos();

	clip_center_coords(scrCenter.x,scrCenter.y);

	if(!current_mode_switch_)
		current_mode_work_time_ += dt;

	if(current_mode_.has_work_time() && current_mode_work_time_ > current_mode_.work_time())
		set_mode(default_mode_,default_object_);

#endif

	if(last_pos.x != scrCenter.x || last_pos.y != scrCenter.y)
		return true;

	return false;
}

bool qdCamera::set_grid_attributes(const Vect2s& center_pos,const Vect2s& size,int attr)
{
	int x0 = center_pos.x - size.x/2;
	int y0 = center_pos.y - size.y/2;

	int x1 = x0 + size.x;
	int y1 = y0 + size.y;

	if(x0 < 0) x0 = 0;
	if(x1 > GSX - 1) x1 = GSX - 1;
	if(y0 < 0) y0 = 0;
	if(y1 > GSY - 1) y1 = GSY - 1;

	sGridCell* cells = Grid + x0 + y0 * GSX;

	for(int y = y0; y < y1; y ++){
		sGridCell* p = cells;
		for(int x = x0; x < x1; x++, p++)
			p -> set_attribute(attr);

		cells += GSX;
	}
	
	return true;
}

bool qdCamera::drop_grid_attributes(const Vect2s& center_pos,const Vect2s& size,int attr)
{
	int x0 = center_pos.x - size.x/2;
	int y0 = center_pos.y - size.y/2;
	
	int x1 = x0 + size.x;
	int y1 = y0 + size.y;
	
	if(x0 < 0) x0 = 0;
	if(x1 > GSX - 1) x1 = GSX - 1;
	if(y0 < 0) y0 = 0;
	if(y1 > GSY - 1) y1 = GSY - 1;
	
	sGridCell* cells = Grid + x0 + y0 * GSX;
	
	for(int y = y0; y < y1; y ++){
		sGridCell* p = cells;
		for(int x = x0; x < x1; x++, p++)
			p -> drop_attribute(attr);
		
		cells += GSX;
	}

	return true;
}

bool qdCamera::set_grid_attributes(int attr)
{
	sGridCell* p = Grid;
	for(int i = 0; i < GSX * GSY; i++, p++)
		p -> set_attribute(attr);

	return true;
}

bool qdCamera::drop_grid_attributes(int attr)
{
	sGridCell* p = Grid;
	for(int i = 0; i < GSX * GSY; i++, p++)
		p -> drop_attribute(attr);

	return true;
}

bool qdCamera::check_grid_attributes(const Vect2s& center_pos,const Vect2s& size,int attr) const
{
	int x0 = center_pos.x - size.x/2;
	int y0 = center_pos.y - size.y/2;
	
	int x1 = x0 + size.x;
	int y1 = y0 + size.y;
	
	if(x0 < 0) x0 = 0;
	if(x1 > GSX - 1) x1 = GSX - 1;
	if(y0 < 0) y0 = 0;
	if(y1 > GSY - 1) y1 = GSY - 1;
	
	const sGridCell* cells = Grid + x0 + y0 * GSX;
	
	for(int y = y0; y < y1; y ++){
		const sGridCell* p = cells;
		for(int x = x0; x < x1; x++, p++){
			if(p -> check_attribute(attr))
				return true;
		}
		
		cells += GSX;
	}
	
	return false;
}

int qdCamera::cells_num_with_exact_attributes(const Vect2s& center_pos,const Vect2s& size,int attr) const
{
	int x0 = center_pos.x - size.x/2;
	int y0 = center_pos.y - size.y/2;
	
	int x1 = x0 + size.x;
	int y1 = y0 + size.y;
	
	if(x0 < 0) x0 = 0;
	if(x1 > GSX - 1) x1 = GSX - 1;
	if(y0 < 0) y0 = 0;
	if(y1 > GSY - 1) y1 = GSY - 1;
	
	const sGridCell* cells = Grid + x0 + y0 * GSX;
	
	int ret = 0;
	for(int y = y0; y < y1; y ++){
		const sGridCell* p = cells;
		for(int x = x0; x < x1; x++, p++){
			if(p -> attributes() == attr)
				ret++;
		}
		
		cells += GSX;
	}
	
	return ret;
}

bool qdCamera::is_walkable(const Vect2s& center_pos,const Vect2s& size,bool ignore_personages) const
{
	int x0 = center_pos.x - size.x/2;
	int y0 = center_pos.y - size.y/2;
	
	int x1 = x0 + size.x;
	int y1 = y0 + size.y;
	
	if(x0 < 0) x0 = 0;
	if(x1 > GSX - 1) x1 = GSX - 1;
	if(y0 < 0) y0 = 0;
	if(y1 > GSY - 1) y1 = GSY - 1;
	
	const sGridCell* cells = Grid + x0 + y0 * GSX;
	int attr = sGridCell::CELL_IMPASSABLE | sGridCell::CELL_OCCUPIED;
	if(!ignore_personages) 
		attr |= sGridCell::CELL_PERSONAGE_OCCUPIED;
	
	for(int y = y0; y < y1; y ++){
		const sGridCell* p = cells;
		for(int x = x0; x < x1; x++, p++){
			if(p -> check_attribute(attr) && !p -> check_attribute(sGridCell::CELL_SELECTED))
				return false;
		}
		
		cells += GSX;
	}
	
	return true;
}

bool qdCamera::clip_grid_line(Vect2s& v0, Vect2s& v1) const
{
	int x,y;
	bool accept = false,done = false;

	int outcodeOut;
	int outcode0 = clip_out_code(v0);
	int outcode1 = clip_out_code(v1);

	do {
		if(outcode0 == 0 && outcode1 == 0) {
			accept = true; done = true;
		}
		else {
			if((outcode0 & outcode1) != 0)
				done = true;
			else {
				if(outcode0)
					outcodeOut = outcode0;
				else 
					outcodeOut = outcode1;

				if(clTOP & outcodeOut){
					x = v0.x + (v1.x - v0.x) * (GSY - v0.y - 1) / (v1.y - v0.y);
					y = GSY - 1;
				}
				else if(clBOTTOM & outcodeOut){
					x = v0.x + (v1.x - v0.x) * (-v0.y) / (v1.y - v0.y);
					y = 0;
				}
				if(clRIGHT & outcodeOut){
					y = v0.y + (v1.y - v0.y) * (GSX - v0.x - 1) / (v1.x - v0.x);
					x = GSX - 1;
				}
				else if(clLEFT & outcodeOut){
					y = v0.y + (v1.y - v0.y) * (-v0.x) / (v1.x - v0.x);
					x = 0;
				}

				if(outcodeOut = outcode0){
					v0.x = x; 
					v0.y = y; 

					outcode0 = clip_out_code(Vect2s(x,y));
				}
				else {
					v1.x = x; 
					v1.y = y; 

					outcode1 = clip_out_code(Vect2s(x,y));
				}
			}
		}
	}
	while(!done);

	return accept;
}

bool qdCamera::init()
{
	default_object_ = NULL;
	current_object_ = NULL;

	scrCenter = scrCenterInitial;

	set_mode(default_mode_);

	return true;
}

void qdCamera::clip_center_coords(int& x,int& y) const
{
	Vect2i lim = screen_center_limit_x();

	if(x < lim.x)
		x = lim.x;
	else if(x > lim.y)
		x = lim.y;

	lim = screen_center_limit_y();

	if(y < lim.x)
		y = lim.x;
	else if(y > lim.y)
		y = lim.y;
}

bool qdCamera::is_visible(const Vect2i& center_offs) const
{
	int sx = qdGameConfig::get_config().screen_sx()/2;
	int sy = qdGameConfig::get_config().screen_sy()/2;

	Vect2s pos = scr2rscr(Vect2s(sx,sy));

	pos.x -= center_offs.x;
	pos.y += center_offs.y;

	if(pos.x < -scrSize.x/2 - sx || pos.x > scrSize.x/2 + sx || pos.y < -scrSize.y/2 - sy || pos.y > scrSize.y/2 + sy)
		return false;

	return true;
}

void qdCamera::cycle_coords(int& x,int& y) const
{
	Vect2s pos = scr2rscr(Vect2s(x,y));

	if(cycle_x_){
		if(pos.x < -scrSize.x/2 + scrOffset.x)
			pos.x += scrSize.x;
		else if(pos.x > scrSize.x/2 + scrOffset.x)
			pos.x -= scrSize.x;
	}
	if(cycle_y_){
		if(pos.y < -scrSize.y/2 + scrOffset.y)
			pos.y += scrSize.y;
		else if(pos.y > scrSize.y/2 + scrOffset.y)
			pos.y -= scrSize.y;
	}

	pos = rscr2scr(pos);
	
	x = pos.x;
	y = pos.y;
}

bool qdCamera::set_grid_line_attributes(const Vect2s& start_pos,const Vect2s& end_pos,const Vect2s& size,int attr)
{
	if(start_pos.x == end_pos.x && start_pos.y == end_pos.y){
		set_grid_attributes(start_pos,size,attr);
		return true;
	}

	Vect2f r(start_pos.x,start_pos.y);

	int dx = end_pos.x - start_pos.x;
	int dy = end_pos.y - start_pos.y;

	Vect2f dr(dx,dy);
	float d = cellSX/3;
	if(d < 0.5f) d = 0.5f;
	dr.normalize(d);

	if(abs(dx) > abs(dy)){
		int i = round(float(dx)/dr.x);
		do {
			set_grid_attributes(Vect2s(r.xi(),r.yi()),size,attr);
			r += dr;
		} while(--i >= 0);
	}
	else {
		int i = round(float(dy)/dr.y);
		do {
			set_grid_attributes(Vect2s(r.xi(),r.yi()),size,attr);
			r += dr;
		} while(--i >= 0);
	}

	return true;
}

bool qdCamera::drop_grid_line_attributes(const Vect2s& start_pos,const Vect2s& end_pos,const Vect2s& size,int attr)
{
	if(start_pos.x == end_pos.x && start_pos.y == end_pos.y){
		drop_grid_attributes(start_pos,size,attr);
		return true;
	}

	Vect2f r(start_pos.x,start_pos.y);

	int dx = end_pos.x - start_pos.x;
	int dy = end_pos.y - start_pos.y;

	Vect2f dr(dx,dy);
	float d = cellSX/3;
	if(d < 0.5f) d = 0.5f;
	dr.normalize(d);

	if(abs(dx) > abs(dy)){
		int i = round(float(dx)/dr.x);
		do {
			drop_grid_attributes(Vect2s(r.xi(),r.yi()),size,attr);
			r += dr;
		} while(--i >= 0);
	}
	else {
		int i = round(float(dy)/dr.y);
		do {
			drop_grid_attributes(Vect2s(r.xi(),r.yi()),size,attr);
			r += dr;
		} while(--i >= 0);
	}

	return true;
}

bool qdCamera::check_grid_line_attributes(const Vect2s& start_pos,const Vect2s& end_pos,const Vect2s& size,int attr) const
{
	if(start_pos.x == end_pos.x && start_pos.y == end_pos.y)
		return check_grid_attributes(start_pos,size,attr);

	Vect2f r(start_pos.x,start_pos.y);

	int dx = end_pos.x - start_pos.x;
	int dy = end_pos.y - start_pos.y;

	Vect2f dr(dx,dy);
	float d = cellSX/3;
	if(d < 0.5f) d = 0.5f;
	dr.normalize(d);

	if(abs(dx) > abs(dy)){
		int i = round(float(dx)/dr.x);
		do {
			if(check_grid_attributes(Vect2s(r.xi(),r.yi()),size,attr))
				return true;
			r += dr;
		} while(--i >= 0);
	}
	else {
		int i = round(float(dy)/dr.y);
		do {
			if(check_grid_attributes(Vect2s(r.xi(),r.yi()),size,attr))
				return true;
			r += dr;
		} while(--i >= 0);
	}

	return false;
}

void qdCamera::dump_grid(const char* file_name) const
{
#ifdef __QD_DEBUG_ENABLE__
	XStream fh(file_name,XS_OUT);
	for(int i = 0; i < GSY; i++){
		for(int j = 0; j < GSX; j++){
			if(Grid[j + i * GSX].attributes() < 10)
				fh < " ";
			fh <= Grid[j + i * GSX].attributes() < " ";
		}
		fh < "\r\n";
	}
	fh.close();
#endif
}

bool qdCamera::can_change_mode() const
{
	if(current_mode_.has_work_time()) return false;

	return true;
}
