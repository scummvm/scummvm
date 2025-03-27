#include <fstream>
#include <iostream>
int main() {
    std::ifstream file("E:/ScummVM/scummvm/gui/themes/fonts/NotoSans-Regular.ttf");
    if (file.is_open()) {
        std::cout << "Mở tệp thành công." << std::endl;
        file.close();
    } else {
        std::cout << "Không thể mở tệp." << std::endl;
    }
    return 0;
}