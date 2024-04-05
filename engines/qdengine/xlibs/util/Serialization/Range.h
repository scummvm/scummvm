#ifndef __RANGE_H_INCLUDED__
#define __RANGE_H_INCLUDED__

class Archive;

class Rangef
{
public:
	Rangef(float _min = 0.f, float _max = 0.f)
	: min_(_min)
	, max_(_max)
	{}

	float minimum() const { return min_; }
	void setMinimum(float _min) { min_ = _min; }
	
	float maximum() const { return max_; }
	void setMaximum(float _max) { max_ = _max; }
	
	void set(float _min, float _max);

	float length() const { return max_ - min_; }
	float center() const { return (max_ + min_) / 2.f; }

	/// Корректен ли интервал (нет - в случае когда minimum > maximum);
	bool is_valid() const { return min_ <= max_; }

	/// Включает ли отрезок (закрытый интервал) точку \c _value.
	bool include(float _value) const { return (min_ <= _value) && (max_ >= _value);	}
	/// Включает ли интервал в себя \c _range.
	bool include(const Rangef& _range) const { return min_ <= _range.min_ && max_ >= _range.max_; }
	
	/// Возвращает пересечение интервала *this и \c _range.
	Rangef intersection(const Rangef& _range) const;
	/// Возвращает интервал включающий в себя оба интервала (*this и \c _range).
	Rangef merge(const Rangef& _range) const;

	/// Возвращает \c _value в пределах интервала [minimum, maximum].
	float clip(float& _value) const;

	void serialize(Archive& ar);

private:
	float min_;
	float max_;
};

// --------------------- Rangei

class Rangei
{
public:
	Rangei(int _min = 0.f, int _max = 0.f)
	: min_(_min)
	, max_(_max)
	{}

	int minimum() const { return min_; }
	void setMinimum(int _min) { min_ = _min; }

	int maximum() const { return max_; }
	void setMaximum(int _max) { max_ = _max; }

	void set(int _min, int _max);

	int length() const { return max_ - min_; }
	int center() const { return (max_ + min_) / 2; }

	/// Корректен ли интервал (нет - в случае когда minimum > maximum);
	bool is_valid() const { return min_ <= max_; }

	/// Включает ли отрезок (закрытый интервал) точку \c _value.
	bool include(int _value) const { return (min_ <= _value) && (max_ >= _value);	}
	/// Включает ли интервал в себя \c _range.
	bool include(const Rangei& _range) const { return min_ <= _range.min_ && max_ >= _range.max_; }

	/// Возвращает пересечение интервала *this и \c _range.
	Rangei intersection(const Rangei& _range) const;
	/// Возвращает интервал включающий в себя оба интервала (*this и \c _range).
	Rangei merge(const Rangei& _range) const;

	/// Возвращает \c _value в пределах интервала [minimum, maximum].
	int clip(int& _value);

	void serialize(Archive& ar);

private:
	int min_;
	int max_;
};

#endif
