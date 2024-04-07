/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <commctrl.h>

#include "app_core.h"
#include "app_error_handler.h"
#include "qd_game_dispatcher.h"
#include "qd_game_scene.h"
#include "qd_game_object_static.h"
#include "qd_video.h"
#include "qd_music_track.h"

#include "qd_resource_packager.h"

#include "xml_parser.h"
#include "qdscr_parser.h"

#include "..\resource.h"

#include "malloc.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

namespace dlg {
	BOOL APIENTRY dlg_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam);
	void show_progress(int percents_done,const char* file_name);
	void set_dlg_title(const char* str);

	bool copy_file(const char* target,const char* source);
};

/* --------------------------- DEFINITION SECTION --------------------------- */

namespace dlg {
	HWND hDlg = NULL;
	int cmdMode = SW_SHOWNORMAL;

	char* script_name = NULL;
	char initial_path[MAX_PATH];
	std::string output_path;
};

using namespace dlg;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine, int nCmdShow)
{
	const char* QD_COPY_TEMP_DIR = "Resource\\ExtCopy\\";
	const char* QD_PACK_TEMP_DIR = "Output\\ExtFiles\\";

	InitCommonControls();

	GetCurrentDirectory(MAX_PATH,initial_path);

	if(__argc < 2)
		return 1;

	script_name = __argv[1];

	if(__argc >= 3){
		output_path = __argv[2];
		if(output_path[output_path.size() - 1] != '\\')
			output_path += "\\";
	}

	output_path += "Output\\";

	cmdMode = nCmdShow;
	hDlg = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_PROGRESS_DLG),NULL,dlg_proc);
	if(!hDlg) return 1;
	set_dlg_title("Сборка ресурсов");
	show_progress(0,"");

	app_io::set_current_directory(script_name);

	// Проверка на допустимость работы с временными дирректориями
	if (true == app_io::is_directory_exist(QD_COPY_TEMP_DIR))
	{
		std::string mess_str = "Директория \"";
		mess_str += QD_COPY_TEMP_DIR;
		mess_str += "\" необходима для работы паковщика и по окончании сборки проекта будет удалена. Продолжить?";
		if (IDCANCEL == MessageBox(NULL,  mess_str.c_str(), "Внимание!", MB_OKCANCEL))
			return 0;
	}

	app_io::remove_directory(output_path.c_str());

	app_io::create_directory(output_path.c_str());
	
	xml::parser& pr = qdscr_XML_Parser();
	pr.parse_file(app_io::strip_path(script_name));

	qdGameDispatcher* p = new qdGameDispatcher;
	if(const xml::tag* tg = pr.root_tag().search_subtag(QDSCR_ROOT))
		p -> load_script(tg);

	int packages_count = 2;
	bool one_cd_mode = true;
	for(int i = 1; i < 32; i++){
		if(p -> is_on_CD(i)){
			one_cd_mode = false;
			packages_count = i + 2;
		}
	}

	qdResourcePackager packager;
	packager.set_packages_number(packages_count);
	packager.set_output_path(output_path.c_str());
	packager.set_compression(p->resource_compression());

	std::vector<qdFileNameList> files_to_copy;
	files_to_copy.resize(packages_count);

	std::vector<qdFileNameList> files_to_pack;
	files_to_pack.resize(packages_count);

	dlg::show_progress(0, "Глобальные файлы: исправление внешних путей");
	p -> adjust_global_object_files_paths(QD_COPY_TEMP_DIR, QD_PACK_TEMP_DIR);
	p -> get_files_list(files_to_copy[1],files_to_pack[0]);
	for(int i = 1; i < packages_count - 1; i++){
		if(p -> is_on_CD(i)){
			files_to_copy[i + 1].insert(files_to_copy[i + 1].end(),files_to_copy[1].begin(),files_to_copy[1].end());
//			files_to_pack[i + 1].insert(files_to_pack[i + 1].end(),files_to_pack[0].begin(),files_to_pack[0].end());
		}
	}

	dlg::show_progress(0, "Видео: исправление внешних путей");
	qdVideoList video = p->video_list();
	for(qdVideoList::iterator it = video.begin(); it != video.end(); ++it){
		(*it) -> adjust_files_paths(QD_COPY_TEMP_DIR, QD_PACK_TEMP_DIR);
		if(!one_cd_mode){
			for(int i = 0; i < packages_count; i++){
				if((*it) -> is_on_CD(i))
					(*it) -> get_files_list(files_to_copy[i + 1],files_to_pack[i + 1]);
			}
		}
		else
			(*it) -> get_files_list(files_to_copy[1],files_to_pack[1]);
	}

	qdGameSceneList scenes = p -> scene_list();
	for(qdGameSceneList::iterator it = scenes.begin(); it != scenes.end(); ++it){
		XBuffer msg;
		msg < "Сцена \"" < (*it)->name() < "\"" < ": исправление внешних путей";
		dlg::show_progress(0, msg);
		(*it) -> adjust_files_paths(QD_COPY_TEMP_DIR, QD_PACK_TEMP_DIR);

		if(!one_cd_mode){
			for(int i = 0; i < packages_count; i++){
				if((*it) -> is_on_CD(i))
					(*it) -> get_files_list(files_to_copy[i + 1],files_to_pack[i + 1]);
			}
		}
		else
			(*it) -> get_files_list(files_to_copy[1],files_to_pack[1]);
	}

	// В qdGameDispatcher все пути поправлены. Сохраняем правильный бинарник.
	// --> Формируем обычный путь
	std::string path_str = output_path;
	if(packages_count > 2) path_str += "CD1\\";
	app_io::create_directory(path_str.c_str());
	path_str += "qd_game.qml"; 
	// --> Процесс сохранения бинарного скрипта через сохранение и загрузку обычн. скрипта
	p->save_script(path_str.c_str());
	pr.clear(); // Очищаем парсер от старой информации
	pr.parse_file(path_str.c_str());
	pr.write_binary_script(path_str.c_str());

	for(int i = 0; i < packages_count; i++)
		packager.add_resources(files_to_pack[i],i);

	packager.set_progress_callback(show_progress);

	packager.make_packages();
	set_dlg_title("Копирование файлов");

	int cur_file = 0;
	int num_files = 0;
	for(int i = 0; i < packages_count; i++)
		num_files += files_to_copy[i].size();

	for(int i = 1; i < packages_count - 1; i++){
		if(p -> is_on_CD(i))
			num_files++;
	}

	XBuffer target_file(MAX_PATH);

	for(int i = 1; i < packages_count; i++){
		for(qdFileNameList::const_iterator it = files_to_copy[i].begin(); it != files_to_copy[i].end(); ++it){
			target_file.init();
			target_file < output_path.c_str();

			if(packages_count > 2)
				target_file < "CD" <= i < "\\";

			target_file < it -> c_str();

			int percents_done = cur_file * 100 / num_files;
			show_progress(percents_done,it -> c_str());

			copy_file(target_file.c_str(),it -> c_str());

			cur_file++;
		}
	}


	XBuffer source_file(MAX_PATH);
	for(int i = 1; i < packages_count - 1; i++){
		if(p -> is_on_CD(i)){
			source_file.init();
			source_file < packager.cd_path(0) < "\\Resource\\resource0.pak";

			target_file.init();
			target_file < packager.cd_path(i) < "\\Resource\\resource0.pak";

			int percents_done = cur_file * 100 / num_files;
			show_progress(percents_done,source_file.c_str());
			copy_file(target_file.c_str(),source_file.c_str());

			cur_file++;
		}
	}

	source_file.init();
	source_file < app_io::strip_file_name(_pgmptr) < "qd_game_final.exe";
	if(!app_io::is_file_exist(source_file.c_str())){
		source_file.init();
		source_file < app_io::strip_file_name(_pgmptr) < "qd_game.exe";
	}

	path_str = output_path;
	if(packages_count > 2) path_str += "CD1\\";
	path_str += "qd_game.exe";

	if(app_io::is_file_exist(source_file.c_str()))
		CopyFile(source_file.c_str(),path_str.c_str(),FALSE);

	for(int i = 1; i < packages_count; i++){
		target_file.init();
		target_file < output_path.c_str();

		if(packages_count > 2)
			target_file < "CD" <= i < "\\";

		target_file < "qd_cd.id";

		XStream fh(target_file.c_str(),XS_OUT);
		fh <= i < "\r\n";
		fh.close();
	}

	// Удаляем промежуточные дирректории со всеми файлами.
	app_io::full_remove_directory(QD_PACK_TEMP_DIR);
	app_io::full_remove_directory(QD_COPY_TEMP_DIR);

	return 0;
}

namespace dlg {

BOOL APIENTRY dlg_proc(HWND hdlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

void show_progress(int percents_done,const char* file_name)
{
	SendDlgItemMessage(hDlg,IDC_PROGRESS1,PBM_SETPOS,(WPARAM)percents_done,(LPARAM)0);
	SendDlgItemMessage(hDlg,IDC_FILE_NAME,WM_SETTEXT,(WPARAM)0,(LPARAM)file_name);

	ShowWindow(hDlg,cmdMode); 
	UpdateWindow(hDlg);

	MSG msg;
	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		DispatchMessage(&msg); 
}

void set_dlg_title(const char* str)
{
	SendMessage(hDlg,WM_SETTEXT,(WPARAM)0,(LPARAM)str);
}

bool copy_file(const char* target,const char* source)
{
	app_io::create_directory(app_io::strip_file_name(target));
	return CopyFile(source,target,0);
}

}; // namespace dlg