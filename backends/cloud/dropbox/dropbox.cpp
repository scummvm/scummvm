/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/cloud/dropbox/dropbox.h"
#include "curl/curl.h"


namespace Cloud {

  Dropbox::Dropbox() {
    token = Common::String("");
  }

  Common::String Dropbox::getSavePath() {

    Common::String dir;

    dir = ConfMan.get("savepath");
    if (dir == "None") {
      ConfMan.removeKey("savepath",
          ConfMan.getActiveDomainName());
      ConfMan.flushToDisk();
      dir = ConfMan.get("savepath");
    }

#ifdef _WIN32_WCE
    if (dir.empty())
      dir = ConfMan.get("path");
#endif

    return dir;
  }

  void Dropbox::Auth(Common::String code) {
    getTokenFromCode(code);
  }

  size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    //Discard response
    return size*nmemb;
  }
  bool Dropbox::checkAuth() {
    if (ConfMan.get("token") == Common::String(""))
      return false;
    CURL* curl = curl_easy_init();
    struct curl_slist *list = NULL;
    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/2/users/get_current_account");
      curl_easy_setopt(curl, CURLOPT_POST, 1L);

      list = curl_slist_append(list, 
          (Common::String("Authorization: Bearer ")+getToken()).c_str());
      list = curl_slist_append(list, "Content-Type: application/json");
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

      Common::String data = Common::String("null");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

      curl_easy_perform(curl);

      long response;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);

      curl_easy_cleanup(curl);
      curl_slist_free_all(list);
      curl = NULL;
      if (response != 200) {
        return false;
      }
      return true;
    }
    return false;
  }

  Common::String Dropbox::getToken() {
    if (token != Common::String(""))
      return token;
    return token = ConfMan.get(Common::String("dropbox_token"));
  }

  size_t parseToken(char *ptr, size_t size, size_t nmemb, void *userdata) {
    int id = 0;
    char *pch = strtok (ptr,":{[}\", ");
    while (pch != NULL)
    {
      if (id == 0 && Common::String(pch) != Common::String("access_token")) {
        warning("Authorization failed. Please get a new token by revisiting the URL");
        return 0;
      } else if (id == 1) {
        ConfMan.set(Common::String("dropbox_token"),pch);
        ConfMan.flushToDisk();
      }
      pch = strtok (NULL, ":{[}\", ");
      id += 1;
    }
    return size*nmemb;

  }

  Common::String Dropbox::getTokenFromCode(Common::String code) {
    CURL* curl = curl_easy_init();
    struct curl_slist *list = NULL;
    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;

    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/1/oauth2/token");
      curl_easy_setopt(curl, CURLOPT_POST, 1L);

      /* Fill in the grant_type field field */ 
      curl_formadd(&formpost,
          &lastptr,
          CURLFORM_COPYNAME, "grant_type",
          CURLFORM_COPYCONTENTS, "authorization_code",
          CURLFORM_END);

      /* Fill in the code field */ 
      curl_formadd(&formpost,
          &lastptr,
          CURLFORM_COPYNAME, "code",
          CURLFORM_COPYCONTENTS, code.c_str(),
          CURLFORM_END);

      curl_easy_setopt(curl, CURLOPT_USERNAME, "bgnmkfnaxarvq5z");
      /*TODO: Devise a method to not disclose the password here in code*/
      curl_easy_setopt(curl, CURLOPT_PASSWORD, "4le56sk0rr5v98t");
      curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parseToken);

      curl_easy_perform(curl);

      curl_easy_cleanup(curl);
      curl_slist_free_all(list);
      curl = NULL;
      return token = ConfMan.get(Common::String("dropbox_token"));
    }
    return token = Common::String("");
  }


  int Dropbox::Copy(const Common::String &srcFilename, const Common::String &destFilename) {

    CURL* curl = curl_easy_init();
    struct curl_slist *list = NULL;

    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/2/files/copy");
      curl_easy_setopt(curl, CURLOPT_POST, 1L);

      list = curl_slist_append(list,
          Common::String("Authorization: Bearer "+getToken()).c_str());
      list = curl_slist_append(list, "Content-Type: application/json");

      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

      Common::String data = Common::String("{\"from_path\":\"/"+srcFilename+"\",\"to_path\":\""+destFilename+"\"}");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

      curl_easy_perform(curl);

      curl_easy_cleanup(curl);
      curl_slist_free_all(list);
      curl = NULL;
      return 0;
    }
    return 1;

  }

  int Dropbox::Remove(const Common::String &filename) {
    CURL* curl = curl_easy_init();
    struct curl_slist *list = NULL;

    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/2/files/delete");
      curl_easy_setopt(curl, CURLOPT_POST, 1L);

      list = curl_slist_append(list, (Common::String("Authorization: Bearer ")+getToken()).c_str());
      list = curl_slist_append(list, "Content-Type: application/json");

      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

      Common::String data = Common::String("{\"path\":\"/"+filename+"\"}");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

      curl_easy_perform(curl);

      curl_easy_cleanup(curl);
      curl_slist_free_all(list);
      curl = NULL;
      return 0;
    }
    return 1;
  }

  static Common::StringArray CloudGames;
  size_t CloudSearch(char *ptr, size_t size, size_t nmemb, void *userdata) {
    CloudGames = Common::StringArray();

    int id = 0;
    char *pch = strtok (ptr,"{[}\", ");
    while (pch != NULL)
    {
      if (id%34 == 14) {
        CloudGames.push_back(Common::String(pch));
      }
      pch = strtok (NULL, "{[}\", ");
      id += 1;
    }
    return size*nmemb;
  }

  int Dropbox::Sync(const Common::String &pattern) { 

    Common::FSDirectory Dir(getSavePath());

    CURL* curl = curl_easy_init();
    struct curl_slist *list = NULL;

    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://api.dropboxapi.com/2/files/search");
      curl_easy_setopt(curl, CURLOPT_POST, 1L);

      list = curl_slist_append(list, (Common::String("Authorization: Bearer ")+getToken()).c_str());
      list = curl_slist_append(list, "Content-Type: application/json");

      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

      Common::String data = Common::String("{\"path\":\"\",\"query\":\""+pattern+"\"}");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CloudSearch);

      curl_easy_perform(curl);

      curl_easy_cleanup(curl);
      curl_slist_free_all(list);
      curl = NULL;

      for (size_t i = 0; i < CloudGames.size(); i++) {
        if (!Dir.hasFile(CloudGames[i])) {
          Download(CloudGames[i]);
        }
      }
      Common::String search(pattern);
      Common::ArchiveMemberList savefiles;
      if (Dir.listMatchingMembers(savefiles, search) > 0) {
        for (Common::ArchiveMemberList::const_iterator file = savefiles.begin(); file != savefiles.end(); ++file) {
          bool cloud_has = 0;
          for (size_t i = 0; i < CloudGames.size(); i++) {
            if (CloudGames[i] == (*file)->getName()) {
              cloud_has = 1;
              break;
            }
          }
          if (!cloud_has) {
            Upload((*file)->getName());
          }
        }
      } 

      return 0;
    }
    return 1;
  }

  size_t downloadCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    return ((Common::WriteStream *)userdata)->write(ptr, size*nmemb);
  }
  int Dropbox::Download(const Common::String &filename) {  

    Common::String dir = getSavePath();
    CURL* curl = curl_easy_init();
    struct curl_slist *list = NULL;
    Common::FSNode node(getSavePath());
    node = node.getChild(filename);
    Common::WriteStream *fd = node.createWriteStream();

    if (fd==0) {
      warning("Could not open file for writing\n");
      return 1;
    }

    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");

      list = curl_slist_append(list, (Common::String("Authorization: Bearer ")+getToken()).c_str());

      list = curl_slist_append(list,
          Common::String("Dropbox-API-Arg: {\"path\":\"/"+filename+"\"}").c_str());
      curl_easy_setopt(curl, CURLOPT_HEADER, 0); 

      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd); 
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadCallback);

      curl_easy_perform(curl);

      curl_easy_cleanup(curl);
      curl_slist_free_all(list);
      curl = NULL;
      fd->flush();
      return 0;
    }
    return 1;
  }

  size_t uploadCallback(char *buffer, size_t size, size_t nitems, void *instream) {
    return ((Common::SeekableReadStream *)instream)->read(buffer, size*nitems);
  }

  int Dropbox::Upload(const Common::String &filename) {
    CURL* curl = curl_easy_init();
    struct curl_slist *list = NULL;
    Common::FSNode node(getSavePath());
    node = node.getChild(filename);
    Common::SeekableReadStream *fd = node.createReadStream();
    if (fd==0) {
      warning("couldn't open savefile");
      return 1;
    }

    int filesize = fd->size();

    if (curl) {
      curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload");
      curl_easy_setopt(curl, CURLOPT_POST, 1L);

      list = curl_slist_append(list, (Common::String("Authorization: Bearer ")+getToken()).c_str());
      list = curl_slist_append(list, "Content-Type: application/octet-stream");
      Common::String new1 = Common::String("Dropbox-API-Arg: {\"path\":\"/"+filename+"\",\"mode\":{\".tag\":\"overwrite\"}}");

      list = curl_slist_append(list, new1.c_str());

      curl_easy_setopt(curl, CURLOPT_READDATA, fd);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
          filesize);

      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, uploadCallback); 

      curl_easy_perform(curl);

      curl_easy_cleanup(curl);
      curl_slist_free_all(list);
      curl = NULL;
      return 0;
    }
    return 1;
  }
}
