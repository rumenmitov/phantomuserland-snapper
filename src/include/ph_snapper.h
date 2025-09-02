/**
 * @brief PhantomOS wrapper for Snapper. Refer to the Snapper docs for
 *        more info.
 * @author Rumen Mitov
 * @date 2025-08-24
 */
#ifndef PHANTOM_SNAPPER
#define PHANTOM_SNAPPER

#include <phantom_types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct Snapper_result {
  enum { Recoverable, Crash } Tag;

  union {
    enum RecoverableState {
      Ok,
      InvalidState,
      InitFailed,
      LoadGenFailed,
      InvalidVersion,
      IntegrityFailed,
      NoMatches,
      NoData,
      RestoreFailed,
      PurgeDenied,
    } recoverableState;

    enum CrashState {
      SNAPSHOT_NOT_POSSIBLE,
      INVALID_ARCHIVE_FILE,
      INVALID_ARCHIVE_ENTRY,
      INVALID_SNAPSHOT_FILE,
      REF_COUNT_FAILED,
      PURGE_FAILED,
    } crashState;
  } Result;
};

struct Snapper_result snapper_init_snapshot(void);

struct Snapper_result snapper_take_snapshot(void const *const payload, size_t size,
                                     u_int64_t identifier);

struct Snapper_result snapper_commit_snapshot(void);

struct Snapper_result snapper_open_generation(const char *generation);

struct Snapper_result snapper_restore(void *dest, size_t size, u_int64_t identifier);

struct Snapper_result snapper_close_generation(void);

struct Snapper_result snapper_purge(const char *generation);

void snapper_purge_expired(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PHANTOM_SNAPPER
