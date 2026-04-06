#include "ph_snapper.h"
#include "phantom_env.h"

extern "C"
{

	int snapper_handle_result(const struct Snapper_result *res)
	{
		assert(res != nullptr);

		if (res->Tag == Snapper_result::Crash) {
			switch (res->Result.crashState) {
				case CrashState::SNAPSHOT_NOT_POSSIBLE:
					panic("SNAPPER CRASH: snapshot not possible");
					break;
				case CrashState::INVALID_ARCHIVE_FILE:
					panic("SNAPPER CRASH: invalid archive file");
					break;
				case CrashState::INVALID_ARCHIVE_ENTRY:
					panic("SNAPPER CRASH: invalid archive entry");
					break;
				case CrashState::INVALID_SNAPSHOT_FILE:
					panic("SNAPPER CRASH: invalid snapshot file");
					break;
				case CrashState::REF_COUNT_FAILED:
					panic("SNAPPER CRASH: reference count failed");
					break;
				case CrashState::PURGE_FAILED:
					panic("SNAPPER CRASH: purge failed");
					break;
			}
		}

		switch (res->Result.recoverableState) {
			case RecoverableState::Ok:
				return 0;
			case RecoverableState::NoPriorGen:
				warning("SNAPPER: no prior generation exists");
				return 1;
			case RecoverableState::InvalidState:
				warning("SNAPPER: invalid snapper state");
				break;
			case RecoverableState::InitFailed:
				warning("SNAPPER: initialization failed");
				break;
			case RecoverableState::LoadGenFailed:
				warning("SNAPPER: failed to load generation");
				break;
			case RecoverableState::InvalidVersion:
				warning("SNAPPER: invalid snapper version");
				break;
			case RecoverableState::IntegrityFailed:
				warning("SNAPPER: integrity check failed");
				break;
			case RecoverableState::NoMatches:
				warning("SNAPPER: no matches");
				break;
			case RecoverableState::NoData:
				warning("SNAPPER: no data");
				break;
			case RecoverableState::RestoreFailed:
				warning("SNAPPER: restoration failed");
				break;
			case RecoverableState::PurgeDenied:
				warning("SNAPPER: purge denied");
				break;
		}

		return -1;
	}

	struct Snapper_result snapper_init_snapshot(void)
	{
		Snapper_result res;
		res.Tag = Snapper_result::Recoverable;

		res.Result.recoverableState =
			(decltype(res.Result.recoverableState))main_obj->_snapper.init_snapshot();
		return res;
	}

	struct Snapper_result snapper_take_snapshot(void const *const payload,
												size_t size,
												u_int64_t identifier)
	{
		Snapper_result res;
		res.Tag = Snapper_result::Recoverable;

		res.Result.recoverableState =
			(decltype(res.Result.recoverableState))main_obj->_snapper.take_snapshot(
				payload, size, identifier);

		return res;
	}

	struct Snapper_result snapper_commit_snapshot(void)
	{
		Snapper_result res;

		try {
			res.Tag = Snapper_result::Recoverable;
			res.Result.recoverableState = (decltype(res.Result.recoverableState))
											  main_obj->_snapper.commit_snapshot();
		} catch (CrashStates crash) {
			res.Tag = Snapper_result::Crash;
			res.Result.crashState = (decltype(res.Result.crashState))crash;
		}

		return res;
	}

	struct Snapper_result snapper_open_generation(const char *generation)
	{
		Snapper_result res;
		res.Tag = Snapper_result::Recoverable;

		res.Result.recoverableState =
			(decltype(res.Result.recoverableState))main_obj->_snapper.open_generation(
				generation);
		return res;
	}

	struct Snapper_result snapper_restore(void *dest, size_t size, u_int64_t identifier)
	{
		Snapper_result res;
		res.Tag = Snapper_result::Recoverable;

		res.Result.recoverableState =
			(decltype(res.Result.recoverableState))main_obj->_snapper.restore(dest,
																			 size,
																			 identifier);
		return res;
	}

	struct Snapper_result snapper_close_generation(void)
	{
		Snapper_result res;
		res.Tag = Snapper_result::Recoverable;

		res.Result.recoverableState =
			(decltype(res.Result.recoverableState))main_obj->_snapper.close_generation();

		return res;
	}

	struct Snapper_result snapper_purge(const char *generation)
	{
		Snapper_result res;
		res.Tag = Snapper_result::Recoverable;

		res.Result.recoverableState =
			(decltype(res.Result.recoverableState))main_obj->_snapper.purge(generation);
		return res;
	}

	void snapper_purge_expired(void)
	{
		main_obj->_snapper.purge_expired();
	}
}
