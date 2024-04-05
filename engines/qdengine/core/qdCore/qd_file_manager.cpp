/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_file_manager.h"
#include "qd_game_dispatcher.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

#ifndef _DEBUG
#ifdef _FINAL_VERSION
//#define _CDCHECK_
#endif
#endif

extern "C" int WINAPI OpenCD(BYTE drive_letter);
extern "C" int WINAPI FindCD();
extern "C" int WINAPI SetSpeed(BYTE speed);
extern "C" int WINAPI ReadCD();
extern "C" int WINAPI ReadCD2();
extern "C" int WINAPI ReadCheck(int mode);
extern "C" int WINAPI RunCheck(char *key);
extern "C" int WINAPI CloseCD();

/* --------------------------- DEFINITION SECTION --------------------------- */

qdFilePackage::qdFilePackage() : container_index_(0),
#ifndef _NO_ZIP_
		container_(0),
#endif
	drive_ID_(LOCAL_DRIVE_ID)
{
}

qdFilePackage::~qdFilePackage()
{
}

bool qdFilePackage::check_container()
{
	if(!is_open()) return false;

	if(!app_io::is_file_exist(file_name())){
		close();
		return false;
	}

	close();
	return open();
}

const char* qdFilePackage::file_name() const
{
	static XBuffer fname(MAX_PATH);
	fname.init();

	if(drive_ID_ != LOCAL_DRIVE_ID)
		fname < char('A' + drive_ID_) < ":\\";

	fname < "Resource\\resource" <= container_index_ < ".pak";

	return fname.c_str();
}

void qdFilePackage::init()
{
#ifndef _NO_ZIP_
	container_ = XZipArchiveManager::instance().createArchive();
	XZipArchiveManager::instance().registerArchive(container_);
	container_->setHandleErrors(false);
#endif
}

qdFileManager::qdFileManager() : request_CD_handler_(NULL)
{
	last_CD_id_ = 0;
	enable_packages_ = false;
	cd_key_[0] = cd_key_[CDKEY_LENGTH] = 0;
	silent_update_mode_ = false;
}

qdFileManager::~qdFileManager()
{
}

bool qdFileManager::init(int cd_count)
{
	if(!enable_packages_) return false;

	int packages_count = cd_count + 1;

	drive_IDs_.resize(cd_count);
	packages_.resize(packages_count);

	for(int i = 0; i < packages_count; i++){
		packages_[i].init();
		packages_[i].set_container_index(i);
		packages_[i].open();
	}

	scan_drives();
	update_packages();

	return true;
}

qdFileManager& qdFileManager::instance()
{
	static qdFileManager mgr;
	return mgr;
}

bool qdFileManager::open_file(XZipStream& fh, const char* file_name, bool err_message)
{
	bool err = fh.handleErrors();
	fh.setHandleErrors(false);
	if(!fh.open(file_name)){
		if(err_message){
			if(enable_packages_ && request_CD_handler_){
				if((*request_CD_handler_)(last_CD_id_)){
					if(fh.open(file_name)){
						fh.setHandleErrors(err);
						return true;
					}
				}
			}

			if(!silent_update_mode_){
				while(1){
					switch(app_errH.message_box(file_name,appErrorHandler::ERR_FILE_NOT_FOUND,appErrorHandler::ERR_MB_ABORTRETRYIGNORE)){
					case appErrorHandler::ERR_RETRY:
						scan_drives();
						update_packages();

						if(fh.open(file_name)){
							fh.setHandleErrors(err);
							return true;
						}
						break;
					case appErrorHandler::ERR_IGNORE:
						fh.setHandleErrors(err);
						return false;
					case appErrorHandler::ERR_ABORT:
						exit(1);
					}
				}
			}
			else {
				Sleep(500);

				scan_drives();
				update_packages();

				if(fh.open(file_name)){
					fh.setHandleErrors(err);
					return true;
				}
			}
		}
		else {
			fh.setHandleErrors(err);
			return false;
		}
	}
	else {
		fh.setHandleErrors(err);
		return true;
	}

	return false;
}

bool qdFileManager::scan_drives(const qdFileOwner* file_owner)
{
	bool ret = false;
	DWORD drives = GetLogicalDrives();

	static XBuffer buf(16);

	std::fill(drive_IDs_.begin(),drive_IDs_.end(),NA_DRIVE_ID);

	for(int i = 0; i < 32; i++){
		if(drives & (1 << i)){
			buf.init();
			buf < (char)('A' + i) < ":\\";
			if(GetDriveType(buf.c_str()) == DRIVE_CDROM){
				XStream fh(0);
				buf < "qd_cd.id";
				if(fh.open(buf.c_str(),XS_IN)){
					int id;
					fh >= id;

					assert(id >= 1 && id <= CD_count());

					if(id >= 1 && id <= CD_count()){
						drive_IDs_[id - 1] = i;
						if(!file_owner || file_owner -> is_on_CD(id - 1))
							ret = true;
					}
				}
			}
		}
	}

	return ret;
}

bool qdFileManager::scan_drives(int cd_id)
{
	bool ret = false;
	DWORD drives = GetLogicalDrives();

	static XBuffer buf(16);

	std::fill(drive_IDs_.begin(),drive_IDs_.end(),NA_DRIVE_ID);

	for(int i = 0; i < 32; i++){
		if(drives & (1 << i)){
			buf.init();
			buf < (char)('A' + i) < ":\\";
			if(GetDriveType(buf.c_str()) == DRIVE_CDROM){
				XStream fh(0);
				buf < "qd_cd.id";
				if(fh.open(buf.c_str(),XS_IN)){
					int id;
					fh >= id;

					assert(id >= 1 && id <= CD_count());

					if(id >= 1 && id <= CD_count()){
						drive_IDs_[id - 1] = i;
						if(id == cd_id + 1)
							ret = true;
					}
				}
			}
		}
	}

	return ret;
}

void qdFileManager::update_packages()
{
	if(!enable_packages_) return;

	if(!packages_[0].check_container()){
		for(int i = 0; i < drive_IDs_.size(); i++){
			if(drive_IDs_[i] != NA_DRIVE_ID){
				packages_[0].set_drive_ID(drive_IDs_[i]);
				if(packages_[0].open())
					break;
			}
		}
	}

	for(int i = 1; i < packages_.size(); i++){
		if(!packages_[i].check_container()){
			if(drive_IDs_[i - 1] != NA_DRIVE_ID){
				packages_[i].set_drive_ID(drive_IDs_[i - 1]);
				packages_[i].open();
			}
		}
	}
}

bool qdFileManager::is_package_available(const qdFileOwner& file_owner)
{
	for(int i = 1; i < packages_.size(); i++){
		if(file_owner.is_on_CD(i - 1) && packages_[i].is_open())
			return true;
	}

	return false;
}

const char* qdFileManager::CD_path(const qdFileOwner& file_owner) const
{
	static XBuffer path(MAX_PATH);
	path.init();

	for(int i = 0; i < CD_count(); i++){
		if(file_owner.is_on_CD(i) && drive_IDs_[i] != NA_DRIVE_ID){
			path < (char)('A' + drive_IDs_[i]) < ":\\";
			return path.c_str();
		}
	}

	return path.c_str();
}

bool qdFileManager::check_drives(const char* cdkey)
{
#ifdef _CDCHECK_
	for(int i = 0; i < drive_IDs_.size(); i++){
		if(drive_IDs_[i] != NA_DRIVE_ID){
			if(check_drive('A' + drive_IDs_[i]) && (!cdkey || !strcmp(cdkey,cd_key_)))
				return true;
		}
	}

	return false;
#else
	return true;
#endif
}

bool qdFileManager::check_drive(int drive_letter)
{
	bool failed = false;

#ifdef _CDCHECK_
	int retcode;
	retcode = OpenCD(drive_letter);
	appLog::default_log() << "OpenCD() = " << retcode << "\r\n";
//	if(retcode != 14) return false;

	retcode = FindCD();
	appLog::default_log() << "FindCD() = " << retcode << "\r\n";
//	if(retcode != 14) return false;

	retcode = ReadCD();
	appLog::default_log() << "ReadCD() = " << retcode << "\r\n";
	if(retcode < 0) return false;

	int test_speed = 0xFF;
	do {
		SetSpeed(test_speed);
		int retcode = ReadCD2();
		appLog::default_log() << "ReadCD2() = " << retcode << " speed: " << test_speed << "\r\n";
		if(retcode == -1)
			failed = true;
		else {
			if(retcode == 1){
				failed = false;
				break;
			}
			else 
				if(retcode != 0)
					failed = true;
		}
		test_speed /= 2;

	} while(test_speed > 0);

	if(failed){ 
		CloseCD();
		return false;
	}

	failed = true;
	test_speed = 0xFF;
	do {
		SetSpeed(test_speed);

		SetSpeed(0xFF);
		int retcode = ReadCheck(1);
		appLog::default_log() << "ReadCheck(1) = " << retcode << " speed: " << test_speed << "\r\n";

		cd_key_[0] = cd_key_[CDKEY_LENGTH] = 0;
		retcode = RunCheck(cd_key_);
		appLog::default_log() << "RunCheck() = " << retcode << " cdkey: " << cd_key_ << "\r\n";

		if(retcode == 1000){
			failed = false;
			break;
		}

	} while(test_speed > 0);

	CloseCD();
#endif
	
	return !failed;
}

