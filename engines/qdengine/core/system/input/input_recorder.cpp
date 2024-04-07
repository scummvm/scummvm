/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "input_recorder.h"
#include "input_wndproc.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

inputRecorder::inputRecorder()
{
	time_ = 0;
	recorder_mode_ = RECORDER_NONE;

	buffer_size_ = current_message_ = 0;
}

inputRecorder::~inputRecorder()
{
	close();
}

bool inputRecorder::open(const char* file_name,recorder_mode_t mode)
{
	close();

	recorder_mode_ = mode;

	switch(mode){
	case RECORDER_WRITE:
		file_.open(file_name,XS_OUT);
		break;
	case RECORDER_PLAY:
		file_.open(file_name,XS_IN);
		read();
		break;
	}

	return true;
}

void inputRecorder::close()
{
	if(recorder_mode_ == RECORDER_WRITE)
		write();

	file_.close();

	time_ = 0;
	recorder_mode_ = RECORDER_NONE;

	buffer_size_ = current_message_ = 0;
}

int inputRecorder::read()
{
	assert(recorder_mode_ == RECORDER_PLAY);

	int count = 0;

	current_message_ = buffer_size_ = 0;

	inputRecorderMessage msg;
	while(!is_buffer_full() && !file_.eof()){
		msg.read(file_);
		add_message(msg);
		count++;
	}

	return count;
}

int inputRecorder::write()
{
	assert(recorder_mode_ == RECORDER_WRITE);

	for(int i = 0; i < buffer_size_; i++)
		messages_buffer_[i].write(file_);

	int count = buffer_size_;

	buffer_size_ = 0;

	return count;
}

bool inputRecorder::add_message(const inputRecorderMessage& msg)
{
	if(recorder_mode_ == RECORDER_WRITE && is_buffer_full())
		write();

	if(!is_buffer_full()){
		messages_buffer_[buffer_size_++] = msg;
		return true;
	}

	return false;
}

bool inputRecorder::dispatch_message(const MSG& msg)
{
	if(recorder_mode_ != RECORDER_WRITE) return false;

	inputRecorderMessage::message_id_t msg_id = inputRecorderMessage::MSG_MOUSE_MOVE;
	int key_id = -1;
	int cursor_x = 0;
	int cursor_y = 0;
	int flags = 0;

	switch(msg.message){
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		msg_id = inputRecorderMessage::MSG_KEY_PRESS;
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		msg_id = inputRecorderMessage::MSG_KEY_RELESASE;
		break;
	case WM_LBUTTONDOWN:
		msg_id = inputRecorderMessage::MSG_MOUSE_LBUTTON_PRESS;
		break;
	case WM_RBUTTONDOWN:
		msg_id = inputRecorderMessage::MSG_MOUSE_RBUTTON_PRESS;
		break;
	case WM_LBUTTONUP:
		msg_id = inputRecorderMessage::MSG_MOUSE_LBUTTON_RELEASE;
		break;
	case WM_RBUTTONUP:
		msg_id = inputRecorderMessage::MSG_MOUSE_RBUTTON_RELEASE;
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		return false;
	}

	if(inputRecorderMessage::is_mouse_message(msg_id)){
		flags = int(msg.wParam);
		cursor_x = LOWORD(msg.lParam);
		cursor_y = HIWORD(msg.lParam);
	}
	else {
		key_id = int(msg.wParam);
		flags = int(msg.lParam);
	}

	return add_message(inputRecorderMessage(msg_id,time_,key_id,cursor_x,cursor_y,flags));
}

bool inputRecorder::dispatch_message(const inputRecorderMessage& imsg)
{
	MSG msg;
	ZeroMemory(&msg,sizeof(MSG));

	switch(imsg.message_id()){
	case inputRecorderMessage::MSG_KEY_PRESS:
		msg.message = WM_KEYDOWN;
		break;
	case inputRecorderMessage::MSG_KEY_RELESASE:
		msg.message = WM_KEYUP;
		break;
	case inputRecorderMessage::MSG_MOUSE_LBUTTON_PRESS:
		msg.message = WM_LBUTTONDOWN;
		break;
	case inputRecorderMessage::MSG_MOUSE_RBUTTON_PRESS:
		msg.message = WM_RBUTTONDOWN;
		break;
	case inputRecorderMessage::MSG_MOUSE_LBUTTON_RELEASE:
		msg.message = WM_LBUTTONUP;
		break;
	case inputRecorderMessage::MSG_MOUSE_RBUTTON_RELEASE:
		msg.message = WM_RBUTTONUP;
		break;
	case inputRecorderMessage::MSG_MOUSE_MOVE:
		msg.message = WM_MOUSEMOVE;
		break;
	default:
		return false;
	}

	if(inputRecorderMessage::is_mouse_message(imsg.message_id())){
		msg.wParam = (WPARAM)imsg.flags();
		msg.lParam = MAKELPARAM(imsg.cursor_x(),imsg.cursor_y());
		return input::mouse_wndproc(msg,mouseDispatcher::instance());
	}
	else {
		msg.wParam = (WPARAM)imsg.key_id();
		msg.lParam = (LPARAM)imsg.flags();
		return input::keyboard_wndproc(msg,keyboardDispatcher::instance());
	}
}

void inputRecorder::quant()
{
	time_++;

	if(recorder_mode_ == RECORDER_PLAY){
		if(current_message_ >= buffer_size_ - 1){
			read();
		}

		if(current_message_ < buffer_size_ - 1){
			while(messages_buffer_[current_message_].time() == time_){
				dispatch_message(messages_buffer_[current_message_]);
				if(++current_message_ >= buffer_size_){
					if(!read())
						break;
				}
			}
		}
	}
}

inputRecorder& inputRecorder::instance()
{
	static inputRecorder rec;
	return rec;
}
