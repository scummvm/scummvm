#ifndef __WAV_SOUND_H__
#define __WAV_SOUND_H__

//! Звук из WAV файла.
class wavSound
{
public:
	wavSound();
	~wavSound();

	const char* data() const { return data_; }
	int data_length() const { return data_length_; }
	int bits_per_sample() const { return bits_per_sample_; }
	int channels() const { return channels_; }
	int samples_per_sec() const { return samples_per_sec_; }

	bool init(int data_len,int bits,int chn,int samples);
	void free_data();

	//! Возвращает true, если звук валиден (т.е. параметры допустимые).
	bool is_valid() const {
		if(bits_per_sample_ != 8 && bits_per_sample_ != 16) return false;
		if(channels_ != 1 && channels_ != 2) return false;
		if(!samples_per_sec_) return false;

		return true;
	}

	//! Возвращает длительность звука в секундах.
	float length() const {
		if(!is_valid()) return 0.0f;
		return float(data_length_ / channels_ / (bits_per_sample_ >> 3)) / float(samples_per_sec_);
	}

private:

	//! Данные.
	char* data_;
	//! Длина данных.
	int data_length_;
	//! Количество бит на сэмпл (8/16).
	int bits_per_sample_;
	//! Количество каналов (1/2 - моно/стерео).
	int channels_;
	//! Частота дискретизации - количество сэмплов в секунду.
	/**
	Значения: 8.0, 11.025, 22.05, 44.1 x1000 Hz.
	 */
	int samples_per_sec_;

	friend bool wav_file_load(const char* fname,class wavSound* snd);

};

#endif /* __WAV_SOUND_H__ */

