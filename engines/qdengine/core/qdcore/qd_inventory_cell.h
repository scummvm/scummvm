#ifndef __QD_INVENTORY_CELL_H__
#define __QD_INVENTORY_CELL_H__

#include <vector>
#include <algorithm>

#include "xml_fwd.h"

#include "qd_sprite.h"

class qdGameObjectAnimated;

//! ќписание €чейки инвентори.
class qdInventoryCellType
{
public:
	qdInventoryCellType() : type_(0) { }
	explicit qdInventoryCellType(int tp) : type_(tp) { }
	~qdInventoryCellType(){ sprite_.free(); }

	int type() const { return type_; }
	void set_type(int tp){ type_ = tp; }

	void set_sprite_file(const char* fname){ sprite_.set_file(fname); }
	const char* sprite_file() const { return sprite_.file(); }

	const qdSprite* sprite() const { return &sprite_; }

	bool load_resources() const;
	void free_resources() const;

	bool load_script(const xml::tag* p);
	bool save_script(class XStream& fh,int indent = 0) const;

	const Vect2i& size() const { return sprite_.size(); }
		
private:
	//! “ип €чейки.
	int type_;	
	//! ¬нешний вид €чейки.
	mutable qdSprite sprite_;
};

inline bool operator == (const qdInventoryCellType& f,const qdInventoryCellType& s){
	return ((f.type() == s.type()) && (f.sprite() == s.sprite()));
}

inline bool operator == (const qdInventoryCellType&f, int type){
	return f.type() == type;
}

typedef std::vector<qdInventoryCellType> qdInventoryCellTypeVector;

//! ячейка инвентори.
class qdInventoryCell
{
public:
	qdInventoryCell();
	qdInventoryCell(const qdInventoryCellType& tp);
	qdInventoryCell(const qdInventoryCell& cl);
	~qdInventoryCell(){ }

	qdInventoryCell& operator = (const qdInventoryCell& cl);

	int type() const { return type_; }
	void set_type(int tp){ type_ = tp; }

#ifndef _QUEST_EDITOR
	const qdSprite* sprite() const { return sprite_; }
	void set_sprite(const qdSprite* spr){ sprite_ = spr; }
#else
	const qdSprite* sprite() const;
#endif

	qdGameObjectAnimated* object() const { return object_; }
	void set_object(qdGameObjectAnimated* obj);

	bool is_empty() const { if(!object_) return true; else return false; }

	const Vect2i& size() const {
		if(sprite())
			return sprite() -> size();

		static Vect2i z = Vect2i(0,0);
		return z;
	}

	int size_x() const { if(sprite()) return sprite() -> size_x(); else return 0; }
	int size_y() const { if(sprite()) return sprite() -> size_y(); else return 0; }
	
	void redraw(int x,int y,bool inactive_mode = false) const;

	bool load_resources();
	bool free_resources();

	//! «агрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! «апись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	static void set_shadow(unsigned color,int alpha){ shadow_color_ = color; shadow_alpha_ = alpha; }
	static const Vect2i& screen_offset(){ return screen_offset_; }
	static void set_screen_offset(const Vect2i& offset){ screen_offset_ = offset; }

private:
	//! “ип €чейки.
	int type_;

#ifndef _QUEST_EDITOR
	//! ¬нешний вид €чейки.
	/**
	”казывает на sprite_ из qdInventoryCellType соответствующего типа.
	*/
	const qdSprite* sprite_;
#endif

	//! ќбъект, который лежит в €чейке.
	mutable qdGameObjectAnimated* object_;

	static Vect2i screen_offset_;
	static unsigned shadow_color_;
	static int shadow_alpha_;
};

typedef std::vector<qdInventoryCell> qdInventoryCellVector;

//! √руппа €чеек инвентори.
class qdInventoryCellSet
{
public:
	qdInventoryCellSet();
	qdInventoryCellSet(int x,int y,int sx,int sy,short addit_sx,short addit_sy,const qdInventoryCellType& tp);
	qdInventoryCellSet(const qdInventoryCellSet& set);
	~qdInventoryCellSet();

	qdInventoryCellSet& operator = (const qdInventoryCellSet& set);

	const Vect2s screen_pos() const { return screen_pos_ + qdInventoryCell::screen_offset(); }
	void set_screen_pos(const Vect2s& pos){ screen_pos_ = pos; }

#ifdef _QUEST_EDITOR
	const Vect2i screen_size() const;
	const Vect2i cell_size() const;
#endif // _QUEST_EDITOR

	grScreenRegion screen_region() const;
	const grScreenRegion& last_screen_region() const { return last_screen_region_; }

	int cell_index(const qdGameObjectAnimated* obj) const;
	Vect2s cell_position(int cell_idx) const;

	const Vect2s& size() const { return size_; }
	void set_size(const Vect2s& sz){ 
		assert(sz.x&&sz.y);
		qdInventoryCell __t;
		if (size_.x != 0)//предполагаю, что либо оба равны либо оба неравны 0
			__t = cells_.front();
		size_ = sz;
		cells_.resize((sz.x+additional_cells_.x) * (sz.y+additional_cells_.y));
		std::fill(cells_.begin(), cells_.end(), __t);
	}

	void set_cell_type(const qdInventoryCellType& tp){ std::fill(cells_.begin(),cells_.end(),tp); }

	bool hit(const Vect2s& pos) const;
	void set_mouse_hover_object(qdGameObjectAnimated* obj);

	void pre_redraw() const;
	void redraw(int offs_x = 0,int offs_y = 0,bool inactive_mode = false) const;
	void post_redraw();

	bool put_object(qdGameObjectAnimated* p);
	bool put_object(qdGameObjectAnimated* p,const Vect2s& pos);
	bool remove_object(qdGameObjectAnimated* p);
	qdGameObjectAnimated* get_object(const Vect2s& pos) const;
	bool is_object_in_list(const qdGameObjectAnimated* p) const;

	bool load_script(const xml::tag* p);
	bool save_script(class XStream& fh,int indent = 0) const;

	//! «агрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! «апись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	bool init(const qdInventoryCellTypeVector& tp);
	
	const qdInventoryCellVector& cells() const { return cells_; }
	int num_cells() const { return cells_.size(); }

	bool load_resources();
	bool free_resources();

	void objects_quant(float dt);

	Vect2s additional_cells() const { return additional_cells_; }
	void set_additional_cells(Vect2s val) { 
		additional_cells_ = val;
		// »зменили кол-во доп. €чеек - измен€ем и всех кол-во массива €чеек
		set_size(size());
	}

	//! —кроллинг
	void scroll_left();
	void scroll_right();
	void scroll_up();
	void scroll_down();

	void debug_log() const;

private:
	// »меет ли область полного инвентори сета объекты
	bool has_rect_objects(int left, int top, int right, int bottom) const;

	//! –азмер группы.
	/**
	¬ группе size_.x * size_.y €чеек.
	*/
	Vect2s size_;
	//! ƒополнительне €чейки по x и y
	Vect2s additional_cells_;
	//! —мещение по x и y (с него вывод€тс€ €чекйки в количестве size_)
	Vect2s cells_shift_;
	//! ячейки.
	qdInventoryCellVector cells_;
	//! Ёкранные координаты центра первой €чейки группы.
	Vect2s screen_pos_;

	grScreenRegion last_screen_region_;
};

typedef std::vector<qdInventoryCellSet> qdInventoryCellSetVector;

#endif /* __QD_INVENTORY_CELL_H__ */
