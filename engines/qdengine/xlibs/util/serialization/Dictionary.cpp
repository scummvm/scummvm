#include "StdAfx.h"
#include <string.h>
#include "Serialization\Dictionary.h"
#include "Serialization\XPrmArchive.h"
#include "FileUtils\FileUtils.h"

class TranslationManagerImpl{
public:
    static TranslationManager& instance();
	const char* translate(const char* key);

	void setTranslationsDir(const char* dir);

	void setDefaultLanguage(const char* languageName);
    void setLanguage(const char* languageName);
	const char* language() const;
	const char* languageList() const{ return languageList_.c_str(); }
    TranslationManagerImpl();

	struct StrICmpLess{
		operator()(const std::string& str1, const std::string& str2){
			return stricmp(str1.c_str(), str2.c_str()) < 0;
		}
	};

private:
    void readLanguagesList();
    Dictionary* loadLanguage(const char* languageName);

	typedef StaticMap< std::string, ShareHandle<Dictionary>, StrICmpLess > Languages;
    Languages languages_;
    std::string languageList_;

    Dictionary* currentLanguage_;
    Dictionary* defaultLanguage_;

	std::string translationsDir_;
};

TranslationManagerImpl::TranslationManagerImpl()
: currentLanguage_(0)
, defaultLanguage_(0)
{
    languages_[""] = currentLanguage_ = defaultLanguage_ = new Dictionary();
}

void TranslationManagerImpl::readLanguagesList()
{
	DirIterator it((translationsDir_ + "\\*.*").c_str());
    DirIterator end;


    while(it != end){
        const char* filename = it.c_str();
		if(it.isFile()){
			languageList_ += "|";
			languageList_ += filename;
		}		
		++it;
    }
}

void TranslationManagerImpl::setTranslationsDir(const char* dir)
{
	translationsDir_ = dir;
	readLanguagesList();
}

Dictionary* TranslationManagerImpl::loadLanguage(const char* languageName)
{
    std::string filename = translationsDir_;
    filename += "\\";
    filename += languageName;

    Dictionary* dictionary = new Dictionary();
    XPrmIArchive ia;
    if(ia.open(filename.c_str())){
        ia.serialize(*dictionary, "Dictionary", 0);
    }
    else{
        xassert(0);
    }
    return dictionary;
}

const char* TranslationManagerImpl::translate(const char* key)
{
	xassert(currentLanguage_);
    const char* result = currentLanguage_->translate(key);
	if(result != key)
		return result;
	else
		return currentLanguage_->useFallback() ?  defaultLanguage_->translate(key) : key;
}

const char* TranslationManagerImpl::language() const
{
	Languages::const_iterator it;
	FOR_EACH(languages_, it){
		if(it->second == currentLanguage_)
			return it->first.c_str();
	}
	xassert(0);
	return "";
}

void TranslationManagerImpl::setLanguage(const char* languageName)
{
    ShareHandle<Dictionary>& language = languages_[languageName];
    if(!language)
        language = loadLanguage(languageName);
    if(language)
        currentLanguage_ = language;
    else
        currentLanguage_ = languages_[""];
}

void TranslationManagerImpl::setDefaultLanguage(const char* languageName)
{
    ShareHandle<Dictionary>& language = languages_[languageName];
    if(!language)
        language = loadLanguage(languageName);
    if(language)
        defaultLanguage_ = language;
    else
        defaultLanguage_ = languages_[""];
}


Dictionary::Dictionary()
{
	useFallback_ = true;
    codePage_ = 1251;
	isOn_ = false;
}

const char* Dictionary::translate(const char* name) const
{
	Map::const_iterator i = map_.find(name);
	if(i == map_.end())
		return name;
	else{
		const char* result = i->second->c_str();
		xassert(strlen(result) > 0);
		return result;
	}
}

bool needTranslation(const std::pair<std::string, std::string*>& p)
{
	if(!p.second->empty())
		return false;

	int i = 0;
	for(i = 0; i < p.first.size(); ++i){
		if(reinterpret_cast<const unsigned char&>(p.first[i]) >= (unsigned char)('А')) // русская 'А'
			return true;
	}
	return false;
}

void Dictionary::serialize(Archive& ar) 
{
	StaticMap<std::string, std::string> translated;
	StaticMap<std::string, std::string> untranslated;
	if(ar.isOutput()){
		Map::const_iterator i;
		FOR_EACH(map_, i){
			if(needTranslation(*i))
				untranslated[i->first] = "";
			else
				translated[i->first] = *i->second;
		}
	}
    ar.serialize(codePage_, "codePage", "Кодовая страница");
	ar.serialize(useFallback_, "useFallback", 0);
	untranslated.serialize(ar, "untranslated", "Непереведенные");
	translated.serialize(ar, "translated", "Словарь");
	if(ar.isInput()){
		strings_.clear();
		StaticMap<std::string, std::string>::const_iterator it;
		FOR_EACH(translated, it){
			if(!it->first.empty() && !it->second.empty() && it->first[0] != '[' && it->second != "&"){
				std::string new_string = it->second;
				if(!it->second.empty() &&  it->first[0] == '&' && it->second[0] != '&')
                    new_string = string("&") + new_string;
				strings_.push_back(new_string);
				map_[it->first] = &strings_.back();
			}
		}
		FOR_EACH(untranslated, it){
			if(!it->first.empty() && !it->second.empty() && it->first[0] != '[' && it->second != "&"){
				std::string new_string = it->second;
				if(!it->second.empty() && it->first[0] == '&' && it->second[0] != '&')
                    new_string = string("&") + new_string;
				strings_.push_back(new_string);
				map_[it->first] = &strings_.back();
			}
		}
	}
}


std::string translateComboList(const char* comboList)
{
	string in = comboList;
	string out;
	while(true){
		string name = cutTokenFromComboList(in);
		if(name.empty())
			break;
		else{
			if(!out.empty())
				out += "|";
			out += TRANSLATE(name.c_str());
		}
	}
	return out;
}

// ---------------------------------------------------------------------------

TranslationManager::TranslationManager()
: impl_(new TranslationManagerImpl)
{

}

TranslationManager::~TranslationManager()
{
    delete impl_;
}

TranslationManager& TranslationManager::instance()
{
    static TranslationManager theOnlyOne;
    return theOnlyOne;
}

const char* TranslationManager::translate(const char* key) const
{
    return impl_->translate(key);
}

void TranslationManager::setTranslationsDir(const char* dir)
{
    impl_->setTranslationsDir(dir);
}


void TranslationManager::setLanguage(const char* languageName)
{
    impl_->setLanguage(languageName);
}

void TranslationManager::setDefaultLanguage(const char* languageName)
{
    impl_->setDefaultLanguage(languageName);
}

const char* TranslationManager::language() const
{
    return impl_->language();
}

const char* TranslationManager::languageList() const
{
    return impl_->languageList();
}
