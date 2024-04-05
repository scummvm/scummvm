#ifndef __XML_TAG_H__
#define __XML_TAG_H__

//! Парсер XML на базе expat.
namespace xml {

//! XML тег.
class tag
{
public:
	typedef std::list<tag> subtags_t;
	typedef subtags_t::const_iterator subtag_iterator;
	
	//! Формат данных тега.
	enum tag_data_format {
		//! данные отсутствуют
		TAG_DATA_VOID,
		//! данные типа short int
		TAG_DATA_SHORT,
		//! данные типа unsigned short int
		TAG_DATA_UNSIGNED_SHORT,
		//! данные типа int
		TAG_DATA_INT,
		//! данные типа unsigned int
		TAG_DATA_UNSIGNED_INT,
		//! данные типа float
		TAG_DATA_FLOAT,
		//! строковые данные
		TAG_DATA_STRING
	};

	tag(int id = 0,tag_data_format data_fmt = TAG_DATA_VOID,int data_sz = 0,int data_offs = 0) : ID_(id), data_format_(data_fmt), data_size_(data_sz), data_offset_(data_offs), data_(NULL) { }
	tag(const tag& tg) : ID_(tg.ID_), data_format_(tg.data_format_), data_size_(tg.data_size_), data_offset_(tg.data_offset_), data_(tg.data_), subtags_(tg.subtags_) { }
	~tag(){ }

	tag& operator = (const tag& tg){
		if(this == &tg) return *this;

		ID_ = tg.ID_;
		data_format_ = tg.data_format_;
		data_size_ = tg.data_size_;
		data_offset_ = tg.data_offset_;

		subtags_ = tg.subtags_;

		return *this;
	}

	//! Возвращает идентификатор тега.
	int ID() const { return ID_; }

	//! Возвращает формат данных тега.
	tag_data_format data_format() const { return data_format_; }

	//! Возвращает количество элеметов данных тега.
	/**
	Чтобы получить размер данных в байтах, надо это число
	умножить на размер элемента данных в байтах - data_elemet_size().
	*/
	int data_size() const { return data_size_; }
	//! Возвращает размер элемента данных тега в байтах.
	int data_element_size() const {
		switch(data_format_){
		case TAG_DATA_VOID:
			return 0;
		case TAG_DATA_SHORT:
			return sizeof(short);
		case TAG_DATA_INT:
			return sizeof(int);
		case TAG_DATA_UNSIGNED_INT:
			return sizeof(unsigned int);
		case TAG_DATA_FLOAT:
			return sizeof(float);
		case TAG_DATA_STRING:
			return sizeof(char);
		}
		return 0;
	}

	//! Устанавливает количество элементов данных тега.
	void set_data_size(int sz){ data_size_ = sz; }

	//! Возвращает смещение до данных тега в данных парсера.
	int data_offset() const { return data_offset_; }
	//! Устанавливает смещение до данных тега в данных парсера.
	void set_data_offset(int off){ data_offset_ = off; }

	//! Возвращает указатель на данные тега.
	const char* data() const { return &*(data_ -> begin() + data_offset_); }

	//! Устанавливает указатель на общие данные.
	void set_data(const std::vector<char>* p){ 
		data_ = p; 

		for(subtags_t::iterator it = subtags_.begin(); it != subtags_.end(); ++it)
			it -> set_data(p);
	}

	//! Очистка вложенных тегов.
	void clear(){ subtags_.clear(); }
	//! Добавляет вложенный тег.
	/**
	Возвращает ссылку на последний вложенный тег.
	*/
	tag& add_subtag(const tag& tg){ subtags_.push_back(tg); return subtags_.back(); }
	//! Возвращает true, если список вложенных тегов не пустой .
	bool has_subtags() const { return !subtags_.empty(); }
	//! Возвращает количество вложенных тэгов.
	int num_subtags() const { return subtags_.size(); }
	//! Возвращает итератор начала списка вложенных тегов.
	subtag_iterator subtags_begin() const { return subtags_.begin(); }
	//! Возвращает итератор конца списка вложенных тегов.
	subtag_iterator subtags_end() const { return subtags_.end(); }
	//! Поиск вложенного тега по его идентификатору.
	const tag* search_subtag(int subtag_id) const {
		for(subtag_iterator it = subtags_begin(); it != subtags_end(); ++it)
			if(it -> ID() == subtag_id) return &*it;
			
		return NULL;
	}

private:

	//! Идентификатор (тип) тега.
	int ID_;
	//! Формат данных тега.
	tag_data_format data_format_;
	//! Количество элементов данных тега.
	int data_size_;
	//! Смещение до данных тега в общих данных.
	int data_offset_;
	//! Указатель на данные.
	const std::vector<char>* data_;
	
	//! Список вложенных тегов.
	subtags_t subtags_;
};

}; /* namespace xml */

#endif /* __XML_TAG_H__ */
