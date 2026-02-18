#ifndef GENODE_PHANTOM_THREADS_H
#define GENODE_PHANTOM_THREADS_H

#include "base/env.h"
#include "base/heap.h"
#include <base/log.h>
#include <base/thread.h>

// TODO : kernel threads, names and mappings

namespace Phantom
{
	class PhantomThreadsRepo;

	struct PhantomGenericThread;
	struct PhantomThread;
	struct PhantomThreadWithArgs;
};  // namespace Phantom

using namespace Phantom;

// XXX : Required, but probably should be reworked
extern "C"
{

	struct phantom_thread
	{
		int tid = 0;
		char *name = nullptr;
		void *owner = nullptr;
		int prio = 0;
	};
}

// Fun fact: in original phantom this is 256 Kb
constexpr unsigned int DEFAULT_STACK_SIZE = 16 * 1024;

// Struct containing generic fields for all threads
struct Phantom::PhantomGenericThread : public Genode::Thread
{
	// Make sure not to change this field outside this class!!!
	phantom_thread _info = {0};

	// Required by VM threads
	// XXX : probably should be reworked
	void (*_death_handler)(phantom_thread *tp) = nullptr;

	void setDeathHandler(void (*death_handler)(phantom_thread *tp))
	{
		_death_handler = death_handler;
	}

	void setInfo(phantom_thread new_info)
	{
		_info = new_info;
	}

	phantom_thread getInfo()
	{
		return _info;
	}

	const phantom_thread getPhantomStruct()
	{
		return _info;
	}

	PhantomGenericThread(Genode::Env &env, const char *name, Genode::size_t stack_size)
		: Genode::Thread(env, name, stack_size)
	{}

	~PhantomGenericThread() override
	{
		if (_death_handler != nullptr) {

			phantom_thread local_thread = getPhantomStruct();
			_death_handler(&local_thread);
		}
	}
};

struct Phantom::PhantomThread : PhantomGenericThread
{
	// TODO: Probably, add tid, owner e.t.c here as fields

	void (*_thread_entry)(void);

	void entry() override
	{
		Genode::log("Entered a new thread!");
		_thread_entry();
	}

	PhantomThread(Genode::Env &env, void (*thread_entry)(void))
		: PhantomGenericThread(env, "Phantom thread", DEFAULT_STACK_SIZE),
		  _thread_entry(thread_entry)
	{}
};

struct Phantom::PhantomThreadWithArgs : PhantomGenericThread
{
	void (*_thread_entry)(void *arg);
	void *_args;

	void entry() override
	{
		_thread_entry(_args);
	}

	PhantomThreadWithArgs(Genode::Env &env, void (*thread_entry)(void *arg), void *args)
		: PhantomGenericThread(env, "Phantom arg thread", DEFAULT_STACK_SIZE),
		  _thread_entry(thread_entry),
		  _args(args)
	{}

	PhantomThreadWithArgs(const PhantomThreadWithArgs &another) = delete;
	int operator=(const PhantomThreadWithArgs &another) = delete;
};

/*
 * Repo containing Phantom threads. Reponsible for creation, storage, assigning Phantom's
 * tids
 */
class Phantom::PhantomThreadsRepo
{
	Genode::Env &_env;
	Genode::Heap &_heap;
	PhantomGenericThread *_threads[512] = {0};
	unsigned int _thread_count = 0;

private:
	int addThread(PhantomGenericThread *thread)
	{
		if (_thread_count == 512) {
			Genode::error("Threads number limit is reached! Can't create more");
			return -1;
		}

		_threads[_thread_count] = thread;

		// phantom_thread info

		// TODO : check that it works properly
		thread->setInfo({.tid = (int)_thread_count, .owner = nullptr, .prio = 0});

		thread->start();

		// XXX : Very sketchy tid, need something better
		return _thread_count++;
	}

public:
	PhantomThreadsRepo(Genode::Env &env, Genode::Heap &heap) : _env(env), _heap(heap) {}

	int createThread(void (*thread_entry)(void))
	{
		return addThread(new (_heap) PhantomThread(_env, thread_entry));
	}

	int createThread(void (*thread_entry)(void *arg), void *args)
	{
		return addThread(new (_heap) PhantomThreadWithArgs(_env, thread_entry, args));
	}

	int createThread(void (*thread_entry)(void *arg), void *args, int flags)
	{
		(void)(flags);  // XXX: Seems that they are not used
		return addThread(new (_heap) PhantomThreadWithArgs(_env, thread_entry, args));
	}

	PhantomGenericThread *getThreadByTID(int tid)
	{
		return _threads[tid];
	}

	void killThread(int tid)
	{
		// For now tid is an index
		_env.cpu().kill_thread(_threads[tid]->cap());
	}
};

#endif
