#ifndef __QD_CAMERA_H__
#define __QD_CAMERA_H__

#include "qd_d3dutils.h"
#include "qd_camera_mode.h"

class sGridCell {
public:
	//! Атрибуты
	enum cell_attribute_t {
		//! ячейка выделена
		CELL_SELECTED		= 0x01,
		//! ячейка непроходима
		CELL_IMPASSABLE		= 0x02,
		//! ячейка занята объектом
		CELL_OCCUPIED		= 0x04,
		//! ячейка занята персонажем
		CELL_PERSONAGE_OCCUPIED	= 0x08,
		//! по ячейке проходит путь персонажа
		CELL_PERSONAGE_PATH	= 0x10
	};

	sGridCell(){ attributes_ = CELL_IMPASSABLE; }
	sGridCell(unsigned short atr, short h){ attributes_ = atr; }
	~sGridCell(){ }

	bool is_walkable() const { return check_attribute(CELL_IMPASSABLE); }
	void make_impassable(){ set_attribute(CELL_IMPASSABLE); }
	void make_walkable(){ drop_attribute(CELL_IMPASSABLE); }

	void select(){ set_attribute(CELL_SELECTED); }
	void deselect(){ drop_attribute(CELL_SELECTED); }
	void toggle_select(){ toggle_attribute(CELL_SELECTED); }
	bool is_selected() const { return check_attribute(CELL_SELECTED); }
	
	short height() const { return 0; }
	void set_height(short h){ }

	void set_attribute(unsigned int attr){ attributes_ |= attr; }
	void toggle_attribute(unsigned int attr){ attributes_ ^= attr; }
	void drop_attribute(unsigned int attr){ attributes_ &= ~attr; }
	bool check_attribute(unsigned int attr) const { if(attributes_ & attr) return true; else return false; }

	unsigned attributes() const { return attributes_; }
	void set_attributes(unsigned attr){ attributes_ = attr; }

	bool clear(){ attributes_ = 0; return true; }
		
private:
	unsigned char attributes_;
};

enum qdCameraRedrawMode
{
	QDCAM_GRID_ABOVE,
	QDCAM_GRID_ZBUFFER,
	QDCAM_GRID_NONE
};

class qdCamera 
{
public:
	qdCamera();
	~qdCamera();

	// Устанавливет параметры клетки с координатами cell_pos
	// по параметрам клетки cell
	bool set_grid_cell(const Vect2s& cell_pos,const sGridCell& cell);
	bool set_grid_cell_attributes(const Vect2s& cell_pos,int attr);

	//! Устанавливает атрибуты для клеток из прямоугольника на сетке с ценром center_pos и размерами size.
	bool set_grid_attributes(const Vect2s& center_pos,const Vect2s& size,int attr);
	//! Очищает атрибуты для клеток из прямоугольника на сетке с ценром center_pos и размерами size.
	bool drop_grid_attributes(const Vect2s& center_pos,const Vect2s& size,int attr);
	//! Возвращает true, если в прямоугольнике на сетке ЕСТЬ ХОТЯ БЫ ОДНА ячейка с атрибутами attr.
	bool check_grid_attributes(const Vect2s& center_pos,const Vect2s& size,int attr) const;
	//! Возвращает количество ячеек в заданной области, имеющих именно аттрибуты attr
	int cells_num_with_exact_attributes(const Vect2s& center_pos,const Vect2s& size,int attr) const;

	//! Устанавливает атрибуты для линии клеток.
	bool set_grid_line_attributes(const Vect2s& start_pos,const Vect2s& end_pos,const Vect2s& size,int attr);
	//! Очищает атрибуты для линии клеток.
	bool drop_grid_line_attributes(const Vect2s& start_pos,const Vect2s& end_pos,const Vect2s& size,int attr);
	//! Возвращает true, если в линии на сетке есть хотя бы одна ячейка с атрибутами attr.
	bool check_grid_line_attributes(const Vect2s& start_pos,const Vect2s& end_pos,const Vect2s& size,int attr) const;

	bool is_walkable(const Vect2s& center_pos,const Vect2s& size,bool ignore_personages = false) const;
	
	//! Устанавливает атрибуты attr для всех клеток сетки.
	bool set_grid_attributes(int attr);
	//! Очищает атрибуты attr для всех клеток сетки.
	bool drop_grid_attributes(int attr);
	
	sGridCell* get_cell(const Vect2s& cell_pos);
	const sGridCell* get_cell(const Vect2s& cell_pos) const;

	// Восстанавливает параметры клетки (сейчас - делает ее непроходимой
	// с нулевой высотой)
	bool restore_grid_cell(const Vect2s cell_pos);

	void cycle_coords(int& x,int& y) const;

	void set_redraw_mode(int md) const{ redraw_mode = md; }
	int get_redraw_mode() const { return redraw_mode; }

	void set_grid_size(int xs, int ys);

	void scale_grid(int sx,int sy,int csx,int csy);
	void resize_grid(int sx,int sy);

	int get_grid_sx() const { return GSX; }
	int get_grid_sy() const { return GSY; }

	const sGridCell* get_grid() const { return Grid; }

	int get_cell_sx() const { return cellSX; }
	int get_cell_sy() const { return cellSY; }

	void set_cell_size(int csx, int csy){
		cellSX = csx;
		cellSY = csy;
	}
		
	void clear_grid();

	// rotateAndScaling
	void rotate_and_scale(float XA, float YA, float ZA, float kX, float kY, float kZ);

	float get_focus() const { return focus; }
	void set_focus(float _focus){ focus=_focus; }

	void set_R(const float r);
	float get_R() const {return m_fR;}

	float get_x_angle() const {return xAngle;}
	float get_y_angle() const {return yAngle;}
	float get_z_angle() const {return zAngle;}

	inline void set_scr_size(int xs, int ys){
		scrSize.x = xs;
		scrSize.y = ys;
	}

	const Vect2i& get_scr_size() const {
		return scrSize;
	}

	// getScrSizeX
	int get_scr_sx() const { return scrSize.x; }
	// getScrSizeY
	int get_scr_sy() const { return scrSize.y;	}

	void set_scr_center(int xc, int yc){
		scrCenter.x = xc;
		scrCenter.y = yc;
	}

	const Vect2i& get_scr_center() const { return scrCenter; }

	int get_scr_center_x() const { return scrCenter.x; }
	int get_scr_center_y() const { return scrCenter.y; }

	const Vect2i screen_center_limit_x() const;
	const Vect2i screen_center_limit_y() const;

	const Vect2i& get_scr_offset() const { return scrOffset; }
	void set_scr_offset(const Vect2i& offs){ scrOffset = offs; }

	const Vect2i& get_scr_center_initial() const { return scrCenterInitial; }
	void set_scr_center_initial(const Vect2i& v){ scrCenterInitial = v; }
	
	void move_scr_center(int dxc, int dyc);

	float scrolling_phase_x() const;
	float scrolling_phase_y() const;

	const Vect3f scr2plane(const Vect2s& scrPoint) const;
	const Vect3f rscr2plane(const Vect2s& rscrPoint) const;

	const Vect3f scr2plane_camera_coord(const Vect2s& scrPoint) const;
	const Vect3f rscr2plane_camera_coord(const Vect2s& scrPoint) const;
	const Vect2s plane2scr(const Vect3f& plnPoint) const;
	const Vect2s plane2rscr(const Vect3f& plnPoint) const;

	const Vect3f rscr2camera_coord(const Vect2s& rScrPoint, float z) const;
	const Vect2s camera_coord2rscr(const Vect3f& coord) const;

	const Vect2s rscr2scr(const Vect2s& v) const;
	const Vect2s scr2rscr(const Vect2s& v) const;

	const Vect2s camera_coord2scr(const Vect3f& coord) const;

	const Vect3f scr2global(const Vect2s &vScrPoint, float zInCameraCoord) const;
	const Vect3f rscr2global(const Vect2s rScrPoint, const float zInCameraCoord) const;
	const Vect3f global2camera_coord(const Vect3f& glCoord) const;
	const Vect3f camera_coord2global(const Vect3f& v) const;

	const Vect2s global2scr(const Vect3f& glCoord) const;
	const Vect2s global2rscr(const Vect3f& glCoord) const;

	//обрезание отрезка по плоскости отсечения камеры
	//возвращает false, если отрезок вообеще невидим
	bool line_cutting(Vect3f& b, Vect3f& e) const;
	//! Обрезка линиии, так чтобы она за сетку не выходила.
	/**
	Все координаты - сеточные, т.е. индексы ячеек сетки.
	*/
	bool clip_grid_line(Vect2s& v0, Vect2s& v1) const;

	// getKScale
	float get_scale(const Vect3f& glCoord) const;

	const Vect3f& get_grid_center() const { return gridCenter; }
	void set_grid_center(const Vect3f& v){ gridCenter = v; }

	const sGridCell* get_cell(float _x, float _y) const;
	const Vect2s get_cell_index(float _x, float _y, bool grid_crop = true) const;
	const Vect2s get_cell_index(const Vect3f& v, bool grid_crop = true) const;
	const Vect3f get_cell_coords(int _x_idx,int _y_idx) const;
	const Vect3f get_cell_coords(const Vect2s& idxs) const;

	void reset_all_select();
	//принимает глобальные координаты
	bool select_cell(int x, int y);

	//принимает глобальные координаты
	bool deselect_cell(int x, int y);

	void load_script(const xml::tag* p);
	bool save_script(XStream& fh,int indent = 0) const;

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	//! Инициализация данных, вызывается при старте и перезапуске игры.
	bool init();

	bool draw_grid() const;
	bool draw_cell(int x,int y,int z,int penWidth,unsigned color) const;
	
	sGridCell* backup(sGridCell* ptrBuff);
	bool restore(sGridCell* grid, int sx, int sy, int csx, int csy);

	//! Установка текущего режима камеры.
	/**
	Если объект нулевой, то берется объект по-умолчанию.
	*/
	bool set_mode(const qdCameraMode& mode,qdGameObjectAnimated* object = NULL);
	//! Возвращает true, если в данный момент можно менять режим камеры.
	bool can_change_mode() const;

	//! Установка объекта, за которым камера следит по умолчанию.
	void set_default_object(qdGameObjectAnimated* p){ default_object_ = p; }

	//! Обсчет логики (движения камеры и т.д), параметр - время в секундах.
	bool quant(float dt);

	//! Возвращает false, если вся рабочая область камеры находится за пределами экрана.
	/**
	Параметр - дополнительное смещение центра рабочей области камеры.
	*/
	bool is_visible(const Vect2i& center_offs = Vect2i(0,0)) const;

	void set_cycle(bool cx,bool cy){ cycle_x_ = cx; cycle_y_ = cy; }

	void dump_grid(const char* file_name) const;

	//! Параметры функции масштабирования
	float scale_pow() const { return scale_pow_; }
	void set_scale_pow(float sp) { scale_pow_ = sp; }
	float scale_z_offset() const { return scale_z_offset_; };
	void set_scale_z_offset(float szo) { scale_z_offset_ = szo; };

	bool need_perspective_correction() const { return (fabs(scale_pow_ - 1) > 0.001 || fabs(scale_z_offset_) > 0.001); }

	//! Возвращает режим работы камеры по умолчанию.
	const qdCameraMode& default_mode() const { return default_mode_; }
	//! Установка режима работы камеры по умолчанию.
	void set_default_mode(const qdCameraMode& mode){ default_mode_ = mode; }

	static qdCamera* current_camera(){ return current_camera_; }
	static void set_current_camera(qdCamera* p){ current_camera_ = p; }

	MATRIX3D const& get_view_matrix() const{
		return m_cam;
	}

private:

	MATRIX3D m_cam;
	float m_fR;
	float xAngle, yAngle, zAngle;

	int GSX, GSY;
	sGridCell* Grid;

	bool cycle_x_;
	bool cycle_y_;

	int cellSX, cellSY;
	float focus;//! расстояние до ближней плоскости отсечения
	// расстояние от центра рабочей области до 
	// верхнего левого угла окна
	Vect2i scrCenter;

	// начальное расстояние от центра рабочей области до 
	// верхнего левого угла окна
	Vect2i scrCenterInitial;

	// размер рабочей области
	Vect2i scrSize;
	// смещение рабочей области
	Vect2i scrOffset;

	// Координаты центра сетки
	Vect3f gridCenter;

	mutable int redraw_mode;

	//! Текущий режим работы камеры.
	qdCameraMode current_mode_;

	//! Время в секуднах с установки текущего режима.
	float current_mode_work_time_;
	bool current_mode_switch_;

	//! Объект, за которым следит камера.
	qdGameObjectAnimated* current_object_;

	//! Объект, за которым камера следит по умолчанию.
	qdGameObjectAnimated* default_object_;
	//! Режим работы камеры по умолчанию.
	qdCameraMode default_mode_;

	//! Параметры функции ускоренного масштабирования из get_scale()
	float scale_pow_;
	float scale_z_offset_;

	static qdCamera* current_camera_;

	static const Vect3f world_UP;
	//точка, на которую мы смотрим
	static const Vect3f atPoint;
	static const float NEAR_PLANE; //ближная плоскость отсечения
	static const float FAR_PLANE;  //дальняя

	enum
	{
		clLEFT	 = 1,
		clRIGHT	 = 2,
		clBOTTOM = 4,
		clTOP	 = 8
	};

	inline int clip_out_code(const Vect2s& v) const
	{
		int code = 0;
		if(v.y >= GSY)
			code |= clTOP;
		else if(v.y < 0)
			code |= clBOTTOM;
		if(v.x >= GSX)
			code |= clRIGHT;
		else if(v.x < 0)
			code |= clLEFT;

		return code;
	}

	void clip_center_coords(int& x,int& y) const;
};

inline Vect3f To3D(const Vect2f& v){ return Vect3f(v.x, v.y, 0); }

#endif /* __QD_CAMERA_H__ */
