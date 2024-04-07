#ifndef __FILE_TIME_H__
#define __FILE_TIME_H__

class FileTime
{
public:
	FileTime(const char* fileName = 0);

	void setCurrentTime();

	bool operator==(const FileTime& rhs) const { return low_ == rhs.low_ && high_ == rhs.high_; }
	bool operator!=(const FileTime& rhs) const { return low_ != rhs.low_ || high_ != rhs.high_; }
	bool operator<(const FileTime& rhs) const { return high_ == rhs.high_ ? low_ < rhs.low_ : high_ < rhs.high_; }

	void serialize(Archive& ar);

	int year() const;
	int month() const;
	int day() const;
	int hour() const;
	int minute() const;
	int second() const;
	int milliseconds() const;

private:
	unsigned long low_;
	unsigned long high_;
};

#endif //__FILE_TIME_H__