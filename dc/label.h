class Label
{
 private:
  void *texture;
  int tex_u, u;

 public:
  void create_texture(const char *text);
  void draw(float x, float y, unsigned int argb = 0xffffffff);
};
