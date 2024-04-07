#ifndef __QD_RESOURCE_PACKAGER_H__
#define __QD_RESOURCE_PACKAGER_H__

//! —борщик ресурсов игры - складывает анимации/картинки/звуки в zip-контейнеры.
class qdResourcePackager
{
public:
	qdResourcePackager();
	~qdResourcePackager();

	bool clear();
	bool set_packages_number(int sz);
	bool add_resource(const char* file_name,int package_idx = 0);
	bool add_resources(const qdFileNameList& file_list,int package_idx = 0);

	void set_compression(int compression){ compression_ = compression; }

	bool make_packages() const;

	typedef void (*progress_callback_t)(int percents_done,const char* current_file);
	progress_callback_t set_progress_callback(progress_callback_t fnc);

	void set_output_path(const char* path);
	
	const char* cd_path(int cd_id) const;

private:

	typedef qdFileNameList resource_container_t;
	typedef std::vector<resource_container_t> package_container_t;

	package_container_t packages_;
	progress_callback_t progress_callback_;

	int compression_;

	std::string output_path_;

	static bool isGraphResource(const char* file_name);
};

#endif // __QD_RESOURCE_PACKAGER_H__

