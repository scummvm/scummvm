/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_error_handler.h"
#include "xml_tag_buffer.h"
#include "gr_dispatcher.h"
#include "mouse_input.h"
#include "keyboard_input.h"
#include "qdscr_parser.h"

#include "qd_game_dispatcher.h"
#include "qd_minigame.h"
#include "qd_setup.h"
#include "qd_minigame_interface.h"
#include "qd_engine_interface.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdMiniGame::qdMiniGame() : dll_handle_(NULL), 
	interface_(NULL)
{
}

qdMiniGame::qdMiniGame(const qdMiniGame& mg) : qdNamedObject(mg),
	dll_name_(mg.dll_name_),
	dll_handle_(mg.dll_handle_),
	interface_(mg.interface_),
	config_(mg.config_)
{
}

qdMiniGame::~qdMiniGame()
{
}

bool qdMiniGame::start()
{
	if(load_interface())
		return interface_ -> init(&qdmg::qdEngineInterfaceImpl::instance());

	return false;
}

bool qdMiniGame::is_started() const
{
	return interface_ != 0;
}

bool qdMiniGame::quant(float dt)
{
	if(interface_)
		interface_ -> quant(dt);
	return true;
}

bool qdMiniGame::end()
{
	if(interface_)
		interface_->finit();

	return release_interface();
}

int qdMiniGame::save_game(char* buffer, int buffer_size, qdGameScene* scene)
{
	bool need_release = false;
	if(!is_started()){
		if(!load_interface()) return 0;
		need_release = true;
	}

	qdMinigameSceneInterface* scene_int = qdmg::qdEngineInterfaceImpl::instance().scene_interface(scene);
	int size = interface_->save_game(&qdmg::qdEngineInterfaceImpl::instance(), scene_int, buffer, buffer_size);
	qdmg::qdEngineInterfaceImpl::instance().release_scene_interface(scene_int);

	if(need_release)
		release_interface();

	return size;
}

int qdMiniGame::load_game(const char* buffer, int buffer_size, qdGameScene* scene)
{
	bool need_release = false;
	if(!is_started()){
		if(!load_interface()) return 0;
		need_release = true;
	}

	qdMinigameSceneInterface* scene_int = qdmg::qdEngineInterfaceImpl::instance().scene_interface(scene);
	int size = interface_->load_game(&qdmg::qdEngineInterfaceImpl::instance(), scene_int, buffer, buffer_size);
	qdmg::qdEngineInterfaceImpl::instance().release_scene_interface(scene_int);

	if(need_release)
		release_interface();

	return size;
}

bool qdMiniGame::load_script(const xml::tag* p)
{
#ifndef _QUEST_EDITOR
	int config_size = 0;
	for(xml::tag::subtag_iterator it1 = p -> subtags_begin(); it1 != p -> subtags_end(); ++it1){
		if(it1 -> ID() == QDSCR_MINIGAME_CONFIG_PARAMETER)
			config_size++;
	}
#endif

	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_FLAG:
				set_flag(xml::tag_buffer(*it).get_int());
				break;
			case QDSCR_NAME:
				set_name(it -> data());
				break;
			case QDSCR_MINIGAME_DLL_NAME:
				set_dll_name(it -> data());
				break;
			case QDSCR_MINIGAME_GAME_NAME:
				set_game_name(it -> data());
				break;
			case QDSCR_MINIGAME_CONFIG_FILE:
				set_config_file_name(it -> data());
				load_config();
#ifndef _QUEST_EDITOR
				config_.reserve(config_.size() + config_size);
#endif
				break;
			case QDSCR_MINIGAME_CONFIG_PARAMETER: {
				if(!qdGameConfig::get_config().minigame_read_only_ini()){
					qdMinigameConfigParameter prm;
					prm.load_script(&*it);
					config_container_t::iterator cfg_it = std::find(config_.begin(),config_.end(),prm);
					if(cfg_it != config_.end()){
						cfg_it -> set_data_string(prm.data_string());
						cfg_it -> set_data_count(prm.data_count());
					}
				}
				}
				break;
		}
	}

	return true;
}

bool qdMiniGame::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<minigame";

	if(name()) fh < " name=\"" < qdscr_XML_string(name()) < "\"";
	if(flags()) fh < " flags=\"" <= flags() < "\"";

	if(!config_file_name_.empty()) fh < " config_file=\"" < qdscr_XML_string(config_file_name()) < "\"";
	if(!dll_name_.empty()) fh < " dll_name=\"" < qdscr_XML_string(dll_name()) < "\"";
	if(!game_name_.empty()) fh < " game_name=\"" < qdscr_XML_string(game_name()) < "\"";

	if(!config_.empty()){
		fh < ">\r\n";

		for(config_container_t::const_iterator it = config_.begin(); it != config_.end(); ++it)
			it -> save_script(fh,indent + 1);

		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "</minigame>\r\n";
	}
	else
		fh < "/>\r\n";
	
	return true;
}

qdMiniGame& qdMiniGame::operator = (const qdMiniGame& mg)
{
	if(this == &mg) return *this;

	*static_cast<qdNamedObject*>(this) = mg;

	dll_name_ = mg.dll_name_;
	dll_handle_ = mg.dll_handle_;
	interface_ = mg.interface_;

	config_file_name_ = mg.config_file_name_;
	config_ = mg.config_;

	return *this;
}

bool qdMiniGame::init()
{
#ifndef _QUEST_EDITOR
	if(load_interface()){
		interface_->new_game(&qdmg::qdEngineInterfaceImpl::instance());
		release_interface();
		return true;
	}
#endif
	return false;
}

bool qdMiniGame::load_config()
{
	if(config_file_name_.empty())
		return false;

	config_.clear();
	std::list<std::string> section_list;
	enumerateIniSections(config_file_name(),section_list);

#ifndef _QUEST_EDITOR
	config_.reserve(section_list.size());
#endif

	for(std::list<std::string>::const_iterator it = section_list.begin(); it != section_list.end(); ++it){
		qdMinigameConfigParameter prm;
		prm.load_ini(config_file_name(),it -> c_str());
		config_.push_back(prm);
	}

	return true;
}

bool qdMiniGame::adjust_files_paths(const char* copy_dir, const char* pack_dir, bool can_overwrite)
{
	std::string copy_corr_dir = copy_dir;
	app_io::adjust_dir_end_slash(copy_corr_dir);

	bool all_ok = true;
	if(has_dll())
		QD_ADJUST_TO_REL_FILE_MEMBER(copy_corr_dir, dll_name, set_dll_name, can_overwrite, all_ok); 

	if(has_config_file())
		QD_ADJUST_TO_REL_FILE_MEMBER(copy_corr_dir, config_file_name, 
		  set_config_file_name, can_overwrite, all_ok); 

	for(config_container_t::iterator it = config_.begin(); it != config_.end(); ++it)
	{
		if(it -> data_type() == qdMinigameConfigParameter::PRM_DATA_FILE)
			QD_ADJUST_TO_REL_FILE_MEMBER(copy_corr_dir, it->data_string, 
			  it->set_data_string, can_overwrite, all_ok);
	}

	return all_ok;
}

bool qdMiniGame::get_files_list(qdFileNameList& files_to_copy,qdFileNameList& files_to_pack) const
{
	if(has_dll())
		files_to_copy.push_back(dll_name());

	if(has_config_file())
		files_to_copy.push_back(config_file_name());

	for(config_container_t::const_iterator it = config_.begin(); it != config_.end(); ++it){
		if(it -> data_type() == qdMinigameConfigParameter::PRM_DATA_FILE)
			files_to_copy.push_back(it -> data_string());
	}

	return true;
}

const char* qdMiniGame::config_parameter_value(const char* cfg_param_name) const
{
#ifndef _QUEST_EDITOR
	config_container_t::const_iterator it = std::find(config_.begin(),config_.end(),cfg_param_name);
	if(it != config_.end())
		return it -> data_string();
#endif
	return NULL;
}

bool qdMiniGame::load_interface()
{
	if(!dll_name_.empty()){
		dll_handle_ = LoadLibrary(dll_name_.c_str());
		if(!dll_handle_){
			app_errH.message_box(dll_name_.c_str(),appErrorHandler::ERR_FILE_NOT_FOUND);
			return false;
		}

		qdMiniGameInterface::interface_open_proc ip = (qdMiniGameInterface::interface_open_proc)(GetProcAddress(HINSTANCE(dll_handle_),"open_game_interface"));
		if(!ip){
			app_errH.message_box(dll_name_.c_str(),appErrorHandler::ERR_BAD_FILE_FORMAT);
			FreeLibrary(static_cast<HMODULE>(dll_handle_));
			return false;
		}

		interface_ = (*ip)(game_name());

		if(!interface_){
			app_errH.message_box(dll_name_.c_str(),appErrorHandler::ERR_BAD_FILE_FORMAT);
			FreeLibrary(static_cast<HMODULE>(dll_handle_));
			return false;
		}

		if(interface_->version() != qdMiniGameInterface::INTERFACE_VERSION){
			XBuffer text;
			text <= qdMiniGameInterface::INTERFACE_VERSION < " <> "< dll_name_.c_str() < ":" <= interface_->version();
			app_errH.show_error(text, appErrorHandler::ERR_MINIGAME_VERSION);
		}

		return true;
	}

	return false;
}

bool qdMiniGame::release_interface()
{
	if(dll_handle_){
		if(interface_){
			qdMiniGameInterface::interface_close_proc ip = (qdMiniGameInterface::interface_close_proc)(GetProcAddress(static_cast<HMODULE>(dll_handle_),"close_game_interface"));
			if(ip) (*ip)(interface_);

			interface_ = NULL;
		}

		FreeLibrary(static_cast<HMODULE>(dll_handle_));
		return true;
	}

	return false;
}
