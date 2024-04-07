/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_resource_packager.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdResourcePackager::qdResourcePackager() : progress_callback_(NULL)
{
	compression_ = 0;
}

qdResourcePackager::~qdResourcePackager()
{
}

bool qdResourcePackager::clear()
{
	packages_.clear();
	return true;
}

bool qdResourcePackager::set_packages_number(int sz)
{
	packages_.resize(sz);
	return true;
}

bool qdResourcePackager::add_resource(const char* file_name,int package_idx)
{
	assert(package_idx >= 0 && package_idx < packages_.size());

	resource_container_t::iterator it = std::find(packages_[package_idx].begin(),packages_[package_idx].end(),file_name);
	if(it != packages_[package_idx].end())
		return false;

	packages_[package_idx].push_back(file_name);
	return true;
}

bool qdResourcePackager::add_resources(const qdFileNameList& file_list,int package_idx)
{
	for(qdFileNameList::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
		add_resource(it -> c_str(),package_idx);

	return true;
}

bool qdResourcePackager::make_packages() const
{
	bool ret = true;

	int num_files = 0;
	for(int i = 0; i < packages_.size(); i++)
		num_files += packages_[i].size();

	XBuffer pak_name(MAX_PATH);

	const int buf_size = 256 * 1024;
	char buf[buf_size];

	int cur_file = 0;
	for(int i = 0; i < packages_.size(); i++){
		pak_name.init();
		pak_name < cd_path((i) ? i - 1 : 0) < "\\Resource\\";
		app_io::create_directory(pak_name.c_str());

		pak_name < "resource" <= i < ".pak";

		XZipArchiveManager::instance().openArchive(pak_name.c_str(), XZS_OUT);

		for(resource_container_t::const_iterator it = packages_[i].begin(); it != packages_[i].end(); ++it){
			if(progress_callback_){
				int percents_done = cur_file * 100 / num_files;
				(*progress_callback_)(percents_done,it -> c_str());
			}

			int compression_level = isGraphResource(it -> c_str()) ? compression_ : 0;

			XStream fin(false);
			if(fin.open(it->c_str(), XS_IN)){
				XZipStream fout(it->c_str(), XZS_OUT, false, compression_level);

				int sz = fin.size();
				while(sz > 0){
					int delta = sz > buf_size ? buf_size : sz;
					fin.read(buf, delta);
					fout.write(buf, delta);
					sz -= delta;
				}

				fin.close();
				fout.close();
			}

			cur_file++;
		}

		XZipArchiveManager::instance().closeArchive(pak_name.c_str());
	}

	return ret;
}

qdResourcePackager::progress_callback_t qdResourcePackager::set_progress_callback(progress_callback_t fnc)
{
	progress_callback_t old_fnc = progress_callback_;
	progress_callback_ = fnc;
	return old_fnc;
}

void qdResourcePackager::set_output_path(const char* path)
{
	output_path_ = path;

	if(!output_path_.empty()){
		if(*output_path_.rbegin() == '\\')
			output_path_.pop_back();
	}
}

const char* qdResourcePackager::cd_path(int cd_id) const
{
	static XBuffer path_buf(MAX_PATH);
	path_buf.init();

	if(!output_path_.empty())
		path_buf < output_path_.c_str();

	if(packages_.size() > 2){
		if(!output_path_.empty())
			path_buf < "\\";

		path_buf < "CD" <= cd_id + 1;
	}

	return path_buf.c_str();
}

bool qdResourcePackager::isGraphResource(const char* file_name)
{
	const char* ext = app_io::get_ext(file_name);
	return (!stricmp(ext, ".tga") || !stricmp(ext, ".qda"));
}
