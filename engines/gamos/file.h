#ifndef GAMOS_FILE_H
#define GAMOS_FILE_H

#include "common/file.h"

namespace Gamos {

typedef Common::Array<byte> RawData;

struct ArchiveDir {
    uint32 offset;
    byte id;
};

class Archive : public Common::File {
public:
	Archive();
	~Archive() override;
	bool open(const Common::Path &name) override;

    uint16 getDirCount() const {
        return _dirCount;
    }

    int16 findDirByID(uint id) const {
        for (uint i = 0; i < _directories.size(); ++i) {
            if (_directories[i].id == id)
                return i;
        }

        return -1;
    }

    bool seekDir(uint id);

    int32 readPackedInt();

    RawData *readCompressedData();
    bool readCompressedData(RawData *out);

    static void decompress(RawData const *in, RawData *out);

public:

uint32 _lastReadSize = 0;
uint32 _lastReadDecompressedSize = 0;
uint32 _lastReadDataOffset = 0;


private:
    int32 _dirOffset;

    byte _dirCount;
    uint32 _dataOffset;

    Common::Array<ArchiveDir> _directories;


    bool _error;
};



}; // namespace Gamos

#endif // GAMOS_FILE_H
