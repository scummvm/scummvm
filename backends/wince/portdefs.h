#if _WIN32_WCE < 300

void *calloc(size_t n, size_t s);
int isdigit(int c);
char *strrchr(const char *s, int c);
char *strdup(const char *s);
int _stricmp( const char *string1, const char *string2 );
int stricmp( const char *string1, const char *string2 );
void assert( void* expression );
void assert( int expression );
long int strtol(const char *nptr, char **endptr, int base);
char *strdup( const char *s);
#endif
