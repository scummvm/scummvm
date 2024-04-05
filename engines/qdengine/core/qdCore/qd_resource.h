#ifndef __QD_RESOURCE_H__
#define __QD_RESOURCE_H__

//! Базовый класс для игровых ресурсов.
/**
Анимации, звуки и т.д.
*/
class qdResource
{
public:
	//! Форматы файлов.
	enum file_format_t {
		//! анимация - .qda
		RES_ANIMATION,
		//! спрайт - .tga
		RES_SPRITE,
		//! звук - .wav
		RES_SOUND,
		//! неопознанный формат
		RES_UNKNOWN
	};

	qdResource();
	qdResource(const qdResource& res);
	virtual ~qdResource();

	qdResource& operator = (const qdResource& res);
	
	//! Загружает в память данные ресурса.
	virtual bool load_resource() = 0;
	//! Выгружает из памяти данные ресурса.
	virtual bool free_resource() = 0;

	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	virtual void set_resource_file(const char* file_name) = 0;
	//! Возвращает имя файла, в котором хранятся данные ресурса.
	/**
	Если оно не задано, должна возвращаеть NULL.
	*/
	virtual const char* resource_file() const = 0;

	//! Возвращает true, если данные ресурса загружены в память.
	bool is_resource_loaded() const { return is_loaded_; }

	static file_format_t file_format(const char* file_name);

#ifdef __QD_DEBUG_ENABLE__
	virtual unsigned resource_data_size() const = 0;
#endif

protected:

	//! Устанавливает или скидывает флаг, показывающий что данные ресурса загружены в память.
	void toggle_resource_status(bool st = true){ is_loaded_ = st; }

private:

	//! Равно true, если данные ресурса загружены в память.
	bool is_loaded_;
};

#ifdef __QD_DEBUG_ENABLE__
class qdResourceInfo
{
public:
	qdResourceInfo(const qdResource* res = NULL,const qdNamedObject* owner = NULL);
	qdResourceInfo(const qdResourceInfo& inf) : resource_(inf.resource_), data_size_(inf.data_size_), resource_owner_(inf.resource_owner_) { }
	~qdResourceInfo();

	qdResourceInfo& operator = (const qdResourceInfo& inf){
		if(this == &inf) return *this;

		resource_ = inf.resource_;
		resource_owner_ = inf.resource_owner_;
		data_size_ = inf.data_size_;

		return *this;
	}

	bool operator < (const qdResourceInfo& inf) const { return data_size_ < inf.data_size_; }

	unsigned data_size() const { return data_size_; }

	qdResource::file_format_t file_format() const;

	bool write(XStream& fh,int line_class_id = -1) const;

private:
	unsigned data_size_;
	const qdResource* resource_;
	const qdNamedObject* resource_owner_;
};

typedef std::vector<qdResourceInfo> qdResourceInfoContainer;

#endif

#endif /* __QD_RESOURCE_H__ */

