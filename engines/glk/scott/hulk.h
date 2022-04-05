#ifndef HULK_H
#define HULK_H

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
