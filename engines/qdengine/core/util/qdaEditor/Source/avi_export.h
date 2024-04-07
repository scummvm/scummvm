
bool START_AVI(const char* file_name);
bool ADD_FRAME_FROM_DIB_TO_AVI(BITMAPINFO& bi, unsigned char* data, const char* _compressor, int _frameRate);
bool STOP_AVI();

