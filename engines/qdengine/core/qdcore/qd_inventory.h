#ifndef __QD_INVENTORY_H__
#define __QD_INVENTORY_H__

#include "mouse_input.h"
#include "qd_named_object.h"
#include "qd_inventory_cell.h"

//! Инвентори.
class qdInventory : public qdNamedObject
{
public:
	qdInventory();
	~qdInventory();

	//! Флаги.
	enum {
		//! если установлен, объект берется сначала на мышь
		INV_TAKE_TO_MOUSE		= 0x01,
		//! если установлен, то после взятия объекта инвентори не открывается 
		INV_DONT_OPEN_AFTER_TAKE	= 0x02,
		//! если установлен, то после клика объектом он возвращается в инветори
		INV_PUT_OBJECT_AFTER_CLICK	= 0x04,
		//! если установлен, объект возвращается в инветори по правому клику мыши
		INV_ENABLE_RIGHT_CLICK		= 0x08,
		//! отрисовывается даже если неактивно в данный момент
		INV_VISIBLE_WHEN_INACTIVE	= 0x10
	};

	enum {
		INV_DEFAULT_SHADOW_COLOR = 0,
		INV_DEFAULT_SHADOW_ALPHA = 128
	};

	int named_object_type() const { return QD_NAMED_OBJECT_INVENTORY; }

	void add_cell_set(const qdInventoryCellSet& set){ cell_sets_.push_back(set); }
	void remove_cell_set(int idx);

#ifdef _QUEST_EDITOR
	qdInventoryCellSet* add_cell_set(const Vect2s& scr_pos, const Vect2s& sz, const qdInventoryCellType& t){ 
		typedef qdInventoryCellSetVector::iterator i_t;
		i_t res = cell_sets_.insert(cell_sets_.end(),qdInventoryCellSet()); 
		if (res == cell_sets_.end())
			return NULL;

		res->set_size(sz);
		res->set_cell_type(t);
		res->set_screen_pos(scr_pos);

		return &*res;
	}
	const qdInventoryCellSetVector& cell_sets() const{ return cell_sets_; }

	//возвращает номер сельсета, если попали иначе -1
	int hit(int x, int y);

	int selected_cellset_num() const{
		return m_iActiveCellset;
	}
	void select_cellset(int inum) const{
		assert(-1 <= inum && inum < (int)cell_sets_.size());
		m_iActiveCellset = inum;
	}
	const qdInventoryCellSet* selected_cellset() const{
		if (m_iActiveCellset == -1)
			return NULL;
		return &get_cell_set(m_iActiveCellset);
	}
#endif // _QUEST_EDITOR
	
	const qdInventoryCellSet& get_cell_set(int id = 0) const { return cell_sets_[id]; }

	void set_cells_pos(int set_id,const Vect2s& pos){
		if(set_id >= 0 && set_id < cell_sets_.size())
			cell_sets_[set_id].set_screen_pos(pos);
	}
	void set_cells_type(int set_id,const qdInventoryCellType& tp){
		if(set_id >= 0 && set_id < cell_sets_.size())
			cell_sets_[set_id].set_cell_type(tp);
	}
	void set_cells_size(int set_id,const Vect2s& sz){
		if(set_id >= 0 && set_id < cell_sets_.size())
			cell_sets_[set_id].set_size(sz);
	}

	int cell_index(const qdGameObjectAnimated* obj) const;
	Vect2s cell_position(int cell_idx) const;

	void redraw(int offs_x = 0,int offs_y = 0,bool inactive_mode = false) const;
	void toggle_redraw(bool state){ need_redraw_ = state; }
	void pre_redraw() const;
	void post_redraw();

	bool put_object(qdGameObjectAnimated* p);
	bool put_object(qdGameObjectAnimated* p,const Vect2s& pos);
	bool remove_object(qdGameObjectAnimated* p);
	qdGameObjectAnimated* get_object(const Vect2s& pos) const;
	bool is_object_in_list(const qdGameObjectAnimated* p) const;

	unsigned shadow_color() const { return shadow_color_; }
	void set_shadow_color(unsigned color){ shadow_color_ = color; }

	int shadow_alpha() const { return shadow_alpha_; }
	void set_shadow_alpha(int alpha){ shadow_alpha_ = alpha; }

	bool load_script(const xml::tag* p);
	bool save_script(class XStream& fh,int indent = 0) const;

	bool load_resources();
	bool free_resources();

	//! Загрузка данных из сэйва.
	bool load_data(qdSaveStream& fh,int save_version);
	//! Запись данных в сэйв.
	bool save_data(qdSaveStream& fh) const;

	bool init(const qdInventoryCellTypeVector& tp);

	bool mouse_handler(int x,int y,mouseDispatcher::mouseEvent ev);

	void objects_quant(float dt);

	Vect2s additional_cells() const { return additional_cells_; }
	void set_additional_cells(Vect2s val) { additional_cells_ = val; }

	//! Скроллинг
	void scroll_left();
	void scroll_right();
	void scroll_up();
	void scroll_down();

	void debug_log() const;

private:

	qdInventoryCellSetVector cell_sets_;

	bool need_redraw_;

	unsigned shadow_color_;
	int shadow_alpha_;

	//! Дополнительные ячейки для всех наборов инвентори
	Vect2s additional_cells_;

#ifdef _QUEST_EDITOR
	mutable int m_iActiveCellset;
#endif
};

#endif /* __QD_INVENTORY_H__ */
