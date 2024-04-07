#ifndef __INPUT_RECORDER_H__
#define __INPUT_RECORDER_H__

class inputRecorderMessage
{
public:
	enum message_id_t {
		MSG_MOUSE_LBUTTON_PRESS,
		MSG_MOUSE_LBUTTON_RELEASE,
		MSG_MOUSE_RBUTTON_PRESS,
		MSG_MOUSE_RBUTTON_RELEASE,
		MSG_MOUSE_MOVE,
		MSG_KEY_PRESS,
		MSG_KEY_RELESASE
	};

	inputRecorderMessage(){ init(MSG_MOUSE_MOVE,0,-1,0,0,0); }
	inputRecorderMessage(message_id_t id,unsigned time,int key_id,int x,int y,int flags){ init(id,time,key_id,x,y,flags); }

	message_id_t message_id() const { return message_id_; }

	unsigned time() const { return time_; }

	int key_id() const { return key_id_; }

	unsigned short cursor_x() const { return cursor_pos_.x; }
	unsigned short cursor_y() const { return cursor_pos_.y; }

	int flags() const { return flags_; }

	void read(XStream& fh)
	{
		char msg_id;
		fh > msg_id > time_ > flags_;

		message_id_ = message_id_t(msg_id);
		if(is_mouse_message(message_id_))
			fh > cursor_pos_.x > cursor_pos_.y; 
		else
			fh > key_id_; 
	}

	void write(XStream& fh) const
	{
		fh < (char)message_id_ < time_ < flags_; 

		if(is_mouse_message(message_id_))
			fh < cursor_pos_.x < cursor_pos_.y; 
		else
			fh < key_id_; 
	}

	static bool is_mouse_message(message_id_t id){ if(id == MSG_KEY_PRESS || id == MSG_KEY_RELESASE) return false; return true; }

private:

	message_id_t message_id_;

	unsigned time_;

	struct cursorPos {
		unsigned short x;
		unsigned short y;
	};

	union {
		int key_id_;
		cursorPos cursor_pos_;
	};

	int flags_;

	void init(message_id_t id,unsigned time,int key_id,int x,int y,int flags)
	{
		message_id_ = id;

		time_ = time;

		if(is_mouse_message(id)){
			cursor_pos_.x = x;
			cursor_pos_.y = y;
		}
		else
			key_id_ = key_id;

		flags_ = flags;
	}
};

class inputRecorder
{
public:
	~inputRecorder();

	enum recorder_mode_t {
		RECORDER_NONE,
		RECORDER_WRITE,
		RECORDER_PLAY
	};

	bool open(const char* file_name,recorder_mode_t mode);
	void close();

	bool add_message(const inputRecorderMessage& msg);
	bool dispatch_message(const MSG& msg);

	void quant();

	static const char* write_comline(){ static const char* p = "write_replay"; return p; }
	static const char* play_comline(){ static const char* p = "show_replay"; return p; }

	static inputRecorder& instance();

private:

	unsigned time_;
	recorder_mode_t recorder_mode_;

	XStream file_;

	enum {
		RECORDER_BUFFER_SIZE = 1024
	};

	inputRecorderMessage messages_buffer_[RECORDER_BUFFER_SIZE];

	int buffer_size_;
	int current_message_;

	int read();
	int write();

	inputRecorder();

	bool is_buffer_full() const { return (buffer_size_ >= RECORDER_BUFFER_SIZE - 1); }
	bool dispatch_message(const inputRecorderMessage& msg);
};

#endif /* __INPUT_RECORDER_H__ */

