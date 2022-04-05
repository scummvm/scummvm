#ifndef GLK_SCOTT_HULK
#define GLK_SCOTT_HULK

struct GameInfo;

namespace Glk {
namespace Scott {

void hulkShowImageOnExamine(int noun);
void hulkLook();
void drawHulkImage(int p);
int tryLoadingHulk(GameInfo info, int dictStart);	

} // End of namespace Scott
} // End of namespace Glk

#endif
