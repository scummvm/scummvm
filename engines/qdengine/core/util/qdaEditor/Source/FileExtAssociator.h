#ifndef __FILE_EXT_ASSOCIATOR_H__
#define __FILE_EXT_ASSOCIATOR_H__

/**
	Класс прописывает в реестре какие расширения принадлежат нашему приложению.
	Порядок действий был найден в какой-то статье из MSDN

	необходимо в таблице строк определить строки с такими идентификаторами
	IDS_APPS_SUB_KEY - имя exe-файла приложения 
	IDS_PROG_ID_REG_KEY	- а это загадочная строка. для своего приложения QuestEditor.exe
	я определил её как "qdaEditor.1"
	IDS_PROG_ID_KEY_VALUE - это значения для предыдущего ключа. Здесь находится описание
							того, что мы будем открывать
*/
class FileExtAssociator  
{
public:
	FileExtAssociator();
	~FileExtAssociator();

	bool associate(const char* ext_string);

private:

	bool setShellSubKeys(HKEY key);
};

#endif // __FILE_EXT_ASSOCIATOR_H__
