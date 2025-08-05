#ifndef PHANTOM_SNAPPER_H
#define PHANTOM_SNAPPER_H

#include <phantom_types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

  int snapper_init_snapshot(void);
  int snapper_take_snapshot(void const * const, size_t, u_int64_t);
  int snapper_commit_snapshot(void);
  int snapper_open_generation(const char *);
  int snapper_restore(void *, size_t, u_int64_t);
  int snapper_close_generation(void);
  int snapper_purge(const char *);
  

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PHANTOM_SNAPPER_H

