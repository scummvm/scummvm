
#ifndef __QD_FILE_MANAGER_H__
#define __QD_FILE_MANAGER_H__

#include "zip_container.h"
#include "qd_file_owner.h"

//! Контейнер с файлами.
class qdFilePackage
{
public:
	qdFilePackage();
	~qdFilePackage();

	enum {
		LOCAL_DRIVE_ID = -1
	};

	//! Возвращает имя файла контейнера.
	const char* file_name() const;

	void set_drive_ID(int drive_id){ drive_ID_ = drive_id; }
	void set_container_index(int idx){ container_index_ = idx; }

	bool is_open(){
#ifndef _NO_ZIP_
		return container_->isOpen();
#else
		return true;
#endif
	}

	void init();
	bool open(){ 
#ifndef _NO_ZIP_
		return container_->open(file_name());
#else
		return false;
#endif
	}
	void close(){
#ifndef _NO_ZIP_
		container_->close();
#endif
	}

	//! Проверяет наличие файла контейнера.
	/**
	Если файл отсутствует - закрывает контейнер и возвращает false.
	*/
	bool check_container();

	bool open_file(const char* file_name){
#ifndef _NO_ZIP_
		// IMPORTANT(pabdulin): is this a valid hack?
		XZipStream xzs = XZipStream(file_name);
		return container_->fileOpen(xzs, file_name);
#else
		return false;
#endif
	}

private:
	//! Номер диска, на котором расположен контейнер.
	/**
	если равен LOCAL_DRIVE_ID - контейнер скопирован к игре в директорию Resources
	иначе лежит на диске A + drive_ID_
	*/
	int drive_ID_;

	//! Номер контейнера.
	/**
	имя файла контейнера - resourceX.pak
	где X - container_index_
	*/
	int container_index_;

#ifndef _NO_ZIP_
	XZipArchive* container_;
#endif
};

//! Менеджер файлов.
class qdFileManager
{
public:
	~qdFileManager();

	bool init(int cd_count);

	void enable_packages(){ enable_packages_ = true; }

	bool open_file(XZipStream& fh, const char* file_name, bool err_message = true);

	int last_CD_id() const { return last_CD_id_; }
	void set_last_CD_id(int cd_id){ last_CD_id_ = cd_id; }

	int CD_count() const { return drive_IDs_.size(); }
	bool is_CD_available(int cd_id) const { assert(cd_id >= 0 && cd_id < drive_IDs_.size()); return drive_IDs_[cd_id] != NA_DRIVE_ID; }
	bool is_package_available(int idx){ assert(idx >= 0 && idx < packages_.size()); return packages_[idx].is_open(); }
	bool is_package_available(const qdFileOwner& file_owner);

	const char* CD_path(const qdFileOwner& file_owner) const;

	bool scan_drives(const qdFileOwner* file_owner = NULL);
	bool scan_drives(int cd_id);
	void update_packages();

	void toggle_silent_update_mode(bool mode){ silent_update_mode_ = mode; }
	
	bool check_drives(const char* cdkey = NULL);
	bool check_drive(int drive_letter);
 
	static qdFileManager& instance();

	typedef bool (*request_CD_handler_t)(int cd_id);
	request_CD_handler_t set_request_CD_handler(request_CD_handler_t new_handler){
		request_CD_handler_t old_handler = request_CD_handler_;
		request_CD_handler_ = new_handler;
		return old_handler;
	}

private:
	
	qdFileManager();

	enum {
		NA_DRIVE_ID = -1,
		CDKEY_LENGTH = 32
	};

	bool enable_packages_;
	bool silent_update_mode_;

	int last_CD_id_;

	std::vector<int> drive_IDs_;

	typedef std::vector<qdFilePackage> packages_container_t;
	packages_container_t packages_;

	//! Функция, которая просит вставить CD в привод.
	request_CD_handler_t request_CD_handler_;

	char cd_key_[CDKEY_LENGTH + 1];
};

#endif // __QD_FILE_MANAGER_H__

