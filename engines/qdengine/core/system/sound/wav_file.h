#ifndef __WAV_FILE_H__
#define __WAV_FILE_H__

// STUB FIXME
#define LRESULT long
#define PASCAL
#define LPSTR char *
#define HMMIO void *
#define WAVEFORMATEX int
#define MMCKINFO int

namespace QDEngine {

bool wav_file_load(const char *fname, class wavSound *snd);

} // namespace QDEngine

#endif /* __WAV_FILE_H__ */
