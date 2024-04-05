
#ifndef __QDSCR_DEFINE_H__
#define __QDSCR_DEFINE_H__

class qdscrDefineData
{
public:
	enum { // flags
		TEXT_FLAG = 0x01
	};

	qdscrDefineData(void);
	~qdscrDefineData(void);

	bool init(char* ptr,int sz);

	int beg_index(void) const { return beg_index_; }
	void set_beg_index(int idx){ beg_index_ = idx; }

	int end_index(void) const { return end_index_; }
	void set_end_index(int idx){ end_index_ = idx; }

	int src_length(void) const { return src_length_; }
	const char* src_data(void) const { return src_data_; }

	int dest_length(void) const { return dest_length_; }
	const char* dest_data(void) const { return dest_data_; }

	void set_flag(int fl){ flags_ |= fl; }

private:
	int flags_;

	int beg_index_;
	int end_index_;

	int src_length_;
	char* src_data_;

	int dest_length_;
	char* dest_data_;
};

typedef std::list<qdscrDefineData*> qdscrDefineDataList;

#endif /* __QDSCR_DEFINE_H__ */
