//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include <stdlib.h>
#include <string.h>
#include "script/cc_error.h"
#include "script/cc_script.h"
#include "script/script_common.h"
#include "util/stream.h"
#include "util/string_compat.h"

using AGS::Common::Stream;

// currently executed line
int currentline;
// script file format signature
const char scfilesig[5] = "SCOM";

// [IKM] I reckon this function is almost identical to fgetstring in string_utils
void freadstring(char **strptr, Stream *in)
{
    static char ibuffer[300];
    int idxx = 0;

    while ((ibuffer[idxx] = in->ReadInt8()) != 0)
        idxx++;

    if (ibuffer[0] == 0) {
        strptr[0] = nullptr;
        return;
    }

    strptr[0] = (char *)malloc(strlen(ibuffer) + 1);
    strcpy(strptr[0], ibuffer);
}

void fwritestring(const char *strptr, Stream *out)
{
    if(strptr == nullptr){
        out->WriteByte(0);
    } else {
        out->Write(strptr, strlen(strptr) + 1);
    }
}

ccScript *ccScript::CreateFromStream(Stream *in)
{
    ccScript *scri = new ccScript();
    if (!scri->Read(in))
    {
        delete scri;
        return nullptr;
    }
    return scri;
}

ccScript::ccScript()
{
    globaldata          = nullptr;
    globaldatasize      = 0;
    code                = nullptr;
    codesize            = 0;
    strings             = nullptr;
    stringssize         = 0;
    fixuptypes          = nullptr;
    fixups              = nullptr;
    numfixups           = 0;
    importsCapacity     = 0;
    imports             = nullptr;
    numimports          = 0;
    exportsCapacity     = 0;
    exports             = nullptr;
    export_addr         = nullptr;
    numexports          = 0;
    instances           = 0;
    sectionNames        = nullptr;
    sectionOffsets      = nullptr;
    numSections         = 0;
    capacitySections    = 0;
}

ccScript::ccScript(const ccScript &src)
{
    globaldatasize = src.globaldatasize;
    if (globaldatasize > 0)
    {
        globaldata = (char*)malloc(globaldatasize);
        memcpy(globaldata, src.globaldata, globaldatasize);
    }
    else
    {
        globaldata = nullptr;
    }

    codesize = src.codesize;
    if (codesize > 0)
    {
        code = (int32_t*)malloc(codesize * sizeof(int32_t));
        memcpy(code, src.code, sizeof(int32_t) * codesize);
    }
    else
    {
        code = nullptr;
    }

    stringssize = src.stringssize;
    if (stringssize > 0)
    {
        strings = (char*)malloc(stringssize);
        memcpy(strings, src.strings, stringssize);
    }
    else
    {
        strings = nullptr;
    }

    numfixups = src.numfixups;
    if (numfixups > 0)
    {
        fixuptypes = (char*)malloc(numfixups);
        fixups = (int32_t*)malloc(numfixups * sizeof(int32_t));
        memcpy(fixuptypes, src.fixuptypes, numfixups);
        memcpy(fixups, src.fixups, numfixups * sizeof(int32_t));
    }
    else
    {
        fixups = nullptr;
        fixuptypes = nullptr;
    }

    importsCapacity = src.numimports;
    numimports = src.numimports;
    if (numimports > 0)
    {
        imports = (char**)malloc(sizeof(char*) * numimports);
        for (int i = 0; i < numimports; ++i)
            imports[i] = ags_strdup(src.imports[i]);
    }
    else
    {
        imports = nullptr;
    }

    exportsCapacity = src.numexports;
    numexports = src.numexports;
    if (numexports > 0)
    {
        exports = (char**)malloc(sizeof(char*) * numexports);
        export_addr = (int32_t*)malloc(sizeof(int32_t) * numexports);
        for (int i = 0; i < numexports; ++i)
        {
            exports[i] = ags_strdup(src.exports[i]);
            export_addr[i] = src.export_addr[i];
        }
    }
    else
    {
        exports = nullptr;
        export_addr = nullptr;
    }

    capacitySections = src.numSections;
    numSections = src.numSections;
    if (numSections > 0)
    {
        sectionNames = (char**)malloc(numSections * sizeof(char*));
        sectionOffsets = (int32_t*)malloc(numSections * sizeof(int32_t));
        for (int i = 0; i < numSections; ++i)
        {
            sectionNames[i] = ags_strdup(src.sectionNames[i]);
            sectionOffsets[i] = src.sectionOffsets[i];
        }
    }
    else
    {
        numSections = 0;
        sectionNames = nullptr;
        sectionOffsets = nullptr;
    }

    instances = 0;
}

ccScript::~ccScript()
{
    Free();
}

void ccScript::Write(Stream *out) {
    int n;
    out->Write(scfilesig,4);
    out->WriteInt32(SCOM_VERSION);
    out->WriteInt32(globaldatasize);
    out->WriteInt32(codesize);
    out->WriteInt32(stringssize);
    if (globaldatasize > 0)
        out->WriteArray(globaldata,globaldatasize,1);
    if (codesize > 0)
        out->WriteArrayOfInt32(code,codesize);
    if (stringssize > 0)
        out->WriteArray(strings,stringssize,1);
    out->WriteInt32(numfixups);
    if (numfixups > 0) {
        out->WriteArray(fixuptypes,numfixups,1);
        out->WriteArrayOfInt32(fixups,numfixups);
    }
    out->WriteInt32(numimports);
    for (n=0;n<numimports;n++)
        fwritestring(imports[n], out);
    out->WriteInt32(numexports);
    for (n=0;n<numexports;n++) {
        fwritestring(exports[n], out);
        out->WriteInt32(export_addr[n]);
    }
    out->WriteInt32(numSections);
    for (n = 0; n < numSections; n++) {
        fwritestring(sectionNames[n], out);
        out->WriteInt32(sectionOffsets[n]);
    }
    out->WriteInt32(ENDFILESIG);
}

bool ccScript::Read(Stream *in)
{
  instances = 0;
  int n;
  char gotsig[5];
  currentline = -1;
  // MACPORT FIX: swap 'size' and 'nmemb'
  in->Read(gotsig, 4);
  gotsig[4] = 0;

  int fileVer = in->ReadInt32();

  if ((strcmp(gotsig, scfilesig) != 0) || (fileVer > SCOM_VERSION)) {
    cc_error("file was not written by ccScript::Write or seek position is incorrect");
    return false;
  }

  globaldatasize = in->ReadInt32();
  codesize = in->ReadInt32();
  stringssize = in->ReadInt32();

  if (globaldatasize > 0) {
    globaldata = (char *)malloc(globaldatasize);
    // MACPORT FIX: swap
    in->Read(globaldata, globaldatasize);
  }
  else
    globaldata = nullptr;

  if (codesize > 0) {
    code = (int32_t *)malloc(codesize * sizeof(int32_t));
    // MACPORT FIX: swap

    // 64 bit: Read code into 8 byte array, necessary for being able to perform
    // relocations on the references.
    in->ReadArrayOfInt32(code, codesize);
  }
  else
    code = nullptr;

  if (stringssize > 0) {
    strings = (char *)malloc(stringssize);
    // MACPORT FIX: swap
    in->Read(strings, stringssize);
  } 
  else
    strings = nullptr;

  numfixups = in->ReadInt32();
  if (numfixups > 0) {
    fixuptypes = (char *)malloc(numfixups);
    fixups = (int32_t *)malloc(numfixups * sizeof(int32_t));
    // MACPORT FIX: swap 'size' and 'nmemb'
    in->Read(fixuptypes, numfixups);
    in->ReadArrayOfInt32(fixups, numfixups);
  }
  else {
    fixups = nullptr;
    fixuptypes = nullptr;
  }

  numimports = in->ReadInt32();

  imports = (char**)malloc(sizeof(char*) * numimports);
  for (n = 0; n < numimports; n++)
    freadstring(&imports[n], in);

  numexports = in->ReadInt32();
  exports = (char**)malloc(sizeof(char*) * numexports);
  export_addr = (int32_t*)malloc(sizeof(int32_t) * numexports);
  for (n = 0; n < numexports; n++) {
    freadstring(&exports[n], in);
    export_addr[n] = in->ReadInt32();
  }

  if (fileVer >= 83) {
    // read in the Sections
    numSections = in->ReadInt32();
    sectionNames = (char**)malloc(numSections * sizeof(char*));
    sectionOffsets = (int32_t*)malloc(numSections * sizeof(int32_t));
    for (n = 0; n < numSections; n++) {
      freadstring(&sectionNames[n], in);
      sectionOffsets[n] = in->ReadInt32();
    }
  }
  else
  {
    numSections = 0;
    sectionNames = nullptr;
    sectionOffsets = nullptr;
  }

  if (in->ReadInt32() != ENDFILESIG) {
    cc_error("internal error rebuilding script");
    return false;
  }
  return true;
}

void ccScript::Free()
{
    if (globaldata != nullptr)
        free(globaldata);

    if (code != nullptr)
        free(code);

    if (strings != nullptr)
        free(strings);

    if (fixups != nullptr && numfixups > 0)
        free(fixups);

    if (fixuptypes != nullptr && numfixups > 0)
        free(fixuptypes);

    globaldata = nullptr;
    code = nullptr;
    strings = nullptr;
    fixups = nullptr;
    fixuptypes = nullptr;

    int aa;
    for (aa = 0; aa < numimports; aa++) {
        if (imports[aa] != nullptr)
            free(imports[aa]);
    }

    for (aa = 0; aa < numexports; aa++)
        free(exports[aa]);

    for (aa = 0; aa < numSections; aa++)
        free(sectionNames[aa]);

    if (sectionNames != nullptr)
    {
        free(sectionNames);
        free(sectionOffsets);
        sectionNames = nullptr;
        sectionOffsets = nullptr;
    }

    if (imports != nullptr)
    {
        free(imports);
        free(exports);
        free(export_addr);
        imports = nullptr;
        exports = nullptr;
        export_addr = nullptr;
    }
    numimports = 0;
    numexports = 0;
    numSections = 0;
}

const char* ccScript::GetSectionName(int32_t offs) {

    int i;
    for (i = 0; i < numSections; i++) {
        if (sectionOffsets[i] < offs)
            continue;
        break;
    }

    // if no sections in script, return unknown
    if (i == 0)
        return "(unknown section)";

    return sectionNames[i - 1];
}
