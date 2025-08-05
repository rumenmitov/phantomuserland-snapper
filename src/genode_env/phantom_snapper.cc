#include <phantom_snapper.h>

#ifdef __cplusplus

#include <snapper.h>

int snapper_init_snapshot(void) {
  SnapperNS::Snapper::Result res = SnapperNS::snapper->init_snapshot();

  // TODO better error handling
  if (res != SnapperNS::Snapper::Ok) {
    return -1;
  }

  return 0;
}

int snapper_take_snapshot(void const *const payload, size_t size,
                          u_int64_t identifier) {
  SnapperNS::Snapper::Result res =
      SnapperNS::snapper->take_snapshot(payload, size, identifier);

  // TODO better error handling
  if (res != SnapperNS::Snapper::Ok) {
    return -1;
  }

  return 0;
}

int snapper_commit_snapshot(void) {
  SnapperNS::Snapper::Result res;

  try {
  SnapperNS::snapper->commit_snapshot();

  // TODO better error handling
  if (res != SnapperNS::Snapper::Ok) {
    return -1;
  }
    
  } catch (SnapperNS::Snapper::CrashStates) {
    // TODO handle snapshot commit not possible! this is a fatal state
    return -1;
  }

  return 0;
}

int snapper_open_generation(const char *generation) 
{
  SnapperNS::Snapper::Result res =
      SnapperNS::snapper->open_generation(generation);

  // TODO better error handling
  if (res != SnapperNS::Snapper::Ok) {
    return -1;
  }

  return 0;
}

int snapper_restore(void *dst, size_t size, u_int64_t identifier) 
{
  SnapperNS::Snapper::Result res =
    SnapperNS::snapper->restore(dst, size, identifier);

  // TODO better error handling
  if (res != SnapperNS::Snapper::Ok) {
    return -1;
  }

  return 0;
}

int snapper_close_generation(void) 
{
  SnapperNS::Snapper::Result res =
    SnapperNS::snapper->close_generation();

  // TODO better error handling
  if (res != SnapperNS::Snapper::Ok) {
    return -1;
  }

  return 0;
}

int snapper_purge(const char * generation) 
{
  SnapperNS::Snapper::Result res =
    SnapperNS::snapper->purge(generation);

  // TODO better error handling
  if (res != SnapperNS::Snapper::Ok) {
    return -1;
  }

  return 0;
}

#endif // __cplusplus
