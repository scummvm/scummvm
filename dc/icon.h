
class Icon
{
 private:
  unsigned char bitmap[32*32/2];
  unsigned int palette[16];
  void *texture;

  int find_unused_pixel();
  bool load_image1(void *data, int len, int offs);
  bool load_image2(void *data, int len);

 public:
  bool load(void *data, int len, int offs = 0);
  bool load(const char *filename);
  void create_texture();
  void set_palette(int pal);
  void draw(float x1, float y1, float x2, float y2, int pal);
  void create_vmicon(void *buffer);
};
