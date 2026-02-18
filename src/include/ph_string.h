/*
 * Contains string operations used in Phantom OS
 * Most of them are default libc functions but with `ph_` prefix
 */
#ifndef INCLUDE_PH_STRINGS
#define INCLUDE_PH_STRINGS

#include <sys/types.h>
// #include <stddef.h>


void *ph_memcpy(void *dst0, const void *src0, size_t length);
void *ph_memmove(void *dst0, const void *src0, size_t length);
int ph_memcmp(const void *s1v, const void *s2v, size_t size);
void *ph_memset(void *dst0, int c0, size_t length);

void bzero(void *dst0, size_t length);

char *ph_strcat(char *s, const char *add);
size_t ph_strlcat(char *dst, const char *src, size_t siz);
char *ph_strchr(const char *p, int ch);
int ph_strcmp(const char *s1, const char *s2);
char *ph_strcpy(char *to, const char *from);
char *ph_strdup(const char *);
char *ph_strndup(const char *str, size_t n);
size_t ph_strlcpy(char *dst, const char *src, size_t siz);
size_t ph_strlen(const char *string);
int ph_strncmp(const char *s1, const char *s2, size_t n);
char *ph_strncpy(char *to, const char *from, ssize_t count);
size_t ph_strnlen(const char *s, size_t count);
char *ph_strrchr(const char *p, int ch);
char *ph_strstr(const char *s1, const char *s2);
char *ph_strnstrn(char const *s1, int l1, char const *s2, int l2);
void *ph_memchr(void const *ptr, int ch, size_t count);

#define ph_index(p, ch) ph_strchr(p, ch)

int ph_atoi(const char *nptr);
long ph_atol(const char *nptr);
long ph_atoln(const char *str, size_t n);
int ph_atoin(const char *str, size_t n);

long ph_strtol(const char *nptr, char **endptr, int base);
long long ph_strtoq(const char *nptr, char **endptr, int base);
unsigned long long ph_strtouq(const char *nptr, char **endptr, int base);

#endif