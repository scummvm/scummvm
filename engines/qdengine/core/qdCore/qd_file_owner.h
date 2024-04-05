#ifndef __QD_FILE_OWNER__
#define __QD_FILE_OWNER__

class qdFileOwner
{
public:
	qdFileOwner() : files_size_(0), cd_info_(0) { }

	unsigned files_size() const { return files_size_; }
	virtual bool get_files_list(qdFileNameList& files_to_copy,qdFileNameList& files_to_pack) const = 0;

	void add_to_CD(int cd_number){ cd_info_ |= 1 << cd_number; }
	void remove_from_CD(int cd_number){ cd_info_ &= ~(1 << cd_number); }
	bool is_on_CD(int cd_number = 1) const { 
		int inf = (cd_info_) ? cd_info_ : 1; 
		return (inf & (1 << cd_number)); 
	}
	void clear_cd_info() { cd_info_ = 0; }
	void calc_files_size();

	unsigned CD_info() const { return cd_info_; }
	void set_CD_info(unsigned inf){	cd_info_ = inf; }
	
private:

	unsigned cd_info_;
	unsigned files_size_;
};

#endif // __QD_FILE_OWNER__

