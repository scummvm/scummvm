#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

class hashconfig;

class Config {
  public:
      Config(const char * = "config.cfg", const char * = "default");
      ~Config();
    const char * get(const char *, const char * = 0) const;
    const char * set(const char *, const char *, const char * = 0);
    const char * set(const char *, int, const char * = 0);
    void set_domain(const char *);
    void flush() const;
    void rename_domain(const char *);
    void delete_domain(const char *);
    void change_filename(const char *);
    void merge_config(const Config *);
  private:
    char * filename, * domain;
    hashconfig ** hash;
    int ndomains;
};

#endif
