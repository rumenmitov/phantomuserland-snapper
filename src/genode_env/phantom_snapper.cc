#include "ph_snapper.h"
#include "phantom_env.h"

extern "C" {
Snapper_result snapper_init_snapshot(void) {
  Snapper_result res;
  res.Tag = Snapper_result::Recoverable;

  res.Result.recoverableState =
      (decltype(res.Result.recoverableState))main_obj->snapper.init_snapshot();
  return res;
}

Snapper_result snapper_take_snapshot(void const *const payload, size_t size,
                                     u_int64_t identifier) {
  Snapper_result res;
  res.Tag = Snapper_result::Recoverable;

  res.Result.recoverableState =
      (decltype(res.Result.recoverableState))main_obj->snapper.take_snapshot(
          payload, size, identifier);
}

Snapper_result snapper_commit_snapshot(void) {
  Snapper_result res;

  try {
    res.Tag = Snapper_result::Recoverable;
    res.Result.recoverableState = (decltype(res.Result.recoverableState))
                                      main_obj->snapper.commit_snapshot();
  } catch (CrashStates crash) {
    res.Tag = Snapper_result::Crash;
    res.Result.crashState = (decltype(res.Result.crashState))crash;
  }

  return res;
}

Snapper_result snapper_open_generation(const char *generation) {
  Snapper_result res;
  res.Tag = Snapper_result::Recoverable;

  res.Result.recoverableState =
      (decltype(res.Result.recoverableState))main_obj->snapper.open_generation(
          generation);
  return res;
}

Snapper_result snapper_restore(void *dest, size_t size, u_int64_t identifier) {
  Snapper_result res;
  res.Tag = Snapper_result::Recoverable;

  res.Result.recoverableState =
      (decltype(res.Result.recoverableState))main_obj->snapper.restore(
          dest, size, identifier);
  return res;
}

Snapper_result snapper_close_generation(void) {
  Snapper_result res;
  res.Tag = Snapper_result::Recoverable;

  res.Result.recoverableState = (decltype(res.Result.recoverableState))
                                    main_obj->snapper.close_generation();

  return res;
}

Snapper_result snapper_purge(const char *generation) {
  Snapper_result res;
  res.Tag = Snapper_result::Recoverable;

  res.Result.recoverableState =
      (decltype(res.Result.recoverableState))main_obj->snapper.purge(
          generation);
  return res;
}

void snapper_purge_expired(void) { main_obj->snapper.purge_expired(); }
}
