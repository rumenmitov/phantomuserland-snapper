#include "i_fs.h"

#include <ph_malloc.h>
#include <ph_string.h>
#include <phantom_fs_internal.h>
#include <vm/alloc.h>
#include <vm/internal.h>
#include <vm/internal_da.h>


/* INFO
   Mutex-lock for operations on the file-descriptor table.
   Should not be persistent. If an operation did not complete
   before a system reset, it will be restarted. Hence, this
   lock should be always available on system boot.
*/
hal_mutex_t fd_dir_lock;


/* Implementations for internal.c */


void pvm_internal_init_fs(pvm_object_t os)
{
	hal_mutex_init(&fd_dir_lock, "fd_dir_lock");

	struct data_area_4_fs *da = pvm_data_area(os, fs);

	da->fd_dir = pvm_create_directory_object();
	da->fd_freelist = pvm_create_array_object();

	for (fd_t i = MAX_FD_ENTRIES; i >= 3; --i) {
		pvm_object_t fd = pvm_create_int_object(i);
		pvm_append_array(da->fd_freelist, fd);
	}

	pvm_add_object_to_restart_list(os);
}


void pvm_restart_fs(pvm_object_t os)
{
	hal_mutex_init(&fd_dir_lock, "fd_dir_lock");
	hal_mutex_lock(&fd_dir_lock);

	pvm_object_t fd_dir = pvm_data_area(os, fs)->fd_dir;
	hashdir_t *hdir = pvm_data_area(fd_dir, directory);

	pvm_object_t keys;

	(void)hdir_keys(hdir, &keys);  // always returns 0

	int keys_size = pvm_get_array_size(keys);

	for (int i = 0; i < keys_size; ++i) {
		pvm_object_t entry_o = pvm_get_array_ofield(keys, i);
		struct fd_entry *entry = (struct fd_entry *)entry_o->da;

		/* INFO
	   Remove OPEN_MODE_CREATE so that we do not get try to create
	   files that should already exist! This is restart, so we should only be creating
	   files that already exist.
	 */
		entry->mode = entry->mode & (~OPEN_MODE_CREATE);

		int rc = genode_open(entry);
		if (rc <= 0)
			ph_printf("genode_open() failed! This could be because the system crashed "
					  "before a rename() operation could complete!\n");
	}

	hal_mutex_unlock(&fd_dir_lock);
}


void pvm_gc_iter_fs(gc_iterator_call_t func, pvm_object_t self, void *arg)
{
	hal_mutex_lock(&fd_dir_lock);

	struct data_area_4_fs *da = pvm_data_area(self, fs);

	func(da->fd_dir, arg);
	func(da->fd_freelist, arg);

	hal_mutex_unlock(&fd_dir_lock);
}


static int si_open_8(pvm_object_t me,
					 pvm_object_t *ret,
					 struct data_area_4_thread *tc,
					 int n_args,
					 pvm_object_t *args)
{
	CHECK_PARAM_COUNT(2);

	const char *path = pvm_get_str_data(args[0]);
	int mode = AS_INT(args[1]);

	int rc = fs_open(path, mode);

	pvm_object_t rc_o = pvm_create_int_object(rc);
	SYSCALL_RETURN(rc_o);
}


static int si_close_9(pvm_object_t me,
					  pvm_object_t *ret,
					  struct data_area_4_thread *tc,
					  int n_args,
					  pvm_object_t *args)
{
	CHECK_PARAM_COUNT(1);

	fd_t fd = AS_INT(args[0]);
	fs_close(fd);

	SYSCALL_RETURN_NOTHING;
}


static int si_rename_10(pvm_object_t me,
						pvm_object_t *ret,
						struct data_area_4_thread *tc,
						int n_args,
						pvm_object_t *args)
{
	CHECK_PARAM_COUNT(2);

	const char *path = pvm_get_str_data(args[0]);
	const char *newpath = pvm_get_str_data(args[1]);

	int rc = fs_rename(path, newpath);

	pvm_object_t rc_o = pvm_create_int_object(rc);
	SYSCALL_RETURN(rc_o);
}


syscall_func_t syscall_table_4_fs[] = {&si_void_0_construct,
									   &si_void_1_destruct,
									   &si_void_2_class,
									   &si_void_3_clone,
									   &si_void_4_equals,
									   &si_void_5_tostring,
									   &si_void_6_toXML,
									   &si_void_7_fromXML,

									   /* INFO
                        Main funcitonality of fs_class.
                     */
									   &si_open_8,
									   &si_close_9,
									   &si_rename_10,
									   &invalid_syscall,

									   [15] = &invalid_syscall};


DECLARE_SIZE(fs);


/* Implementations for i_fs.h */


/**
 * @brief		Retrieves a free file-descriptor identifier.
 * @panic		Panics if there are no more free file-descriptors.
 * @return	fd_t - An unused file-descriptor.
 */
static fd_t __get_free_fd(void)
{
	pvm_object_t fs_o = pvm_get_fs_class();
	struct data_area_4_fs *da = pvm_data_area(fs_o, fs);

	pvm_object_t fd_o = pvm_get_array_ofield(da->fd_freelist, 0);
	pvm_pop_array(da->fd_freelist, fd_o);

	fd_t ret = pvm_data_area(fd_o, int)->value;
	ref_dec_o(fd_o);

	return ret;
}


/**
 * @brief		Converts fd_t into a char *.
 * @param		fd_t		- File-descriptor.
 * @return	char *	- ph_malloc() buffer. After use, free with
 * 										ph_free().
 */
static char *__fd_to_str(fd_t fd)
{
	char *buf = (char *)ph_malloc(FD_STR_LEN /* null terminator */ + 1);

	ph_snprintf(buf, FD_STR_LEN + 1, "%08x", fd);
	return buf;
}


int fs_mkdir(const char *path, u_int32_t mode)
{
	/* INFO
	 Genode does not have the equivalent of opendir. So no need
	 for MODE.
  */
	(void)mode;

	return genode_mkdir(path);
}


fd_t fs_open(const char *path, Open_mode mode)
{
	pvm_object_t entry_o = pvm_object_alloc(sizeof(struct fd_entry), 0, false);
	struct fd_entry *entry = (struct fd_entry *)entry_o->da;

	ph_strncpy(entry->path, path, ph_strlen(path));
	entry->mode = mode;

	int rc = genode_open(entry);
	if (rc <= 0)
		return rc;

	fd_t fd = __get_free_fd();

	pvm_object_t fs_o = pvm_get_fs_class();
	pvm_object_t fd_dir = pvm_data_area(fs_o, fs)->fd_dir;
	hashdir_t *hdir = pvm_data_area(fd_dir, directory);

	char *fdbuf = __fd_to_str(fd);

	hal_mutex_lock(&fd_dir_lock);
	hdir_add(hdir, fdbuf, ph_strlen(fdbuf), entry_o);
	hal_mutex_unlock(&fd_dir_lock);

	ph_free(fdbuf);
	fdbuf = NULL;

	return fd;
}


void fs_close(fd_t fd)
{
	pvm_object_t fs_o = pvm_get_fs_class();

	pvm_object_t fd_dir = pvm_data_area(fs_o, fs)->fd_dir;
	pvm_object_t fd_freelist = pvm_data_area(fs_o, fs)->fd_freelist;

	hashdir_t *hdir = pvm_data_area(fd_dir, directory);

	char *fdbuf = __fd_to_str(fd);

	hal_mutex_lock(&fd_dir_lock);

	pvm_object_t out;
	hdir_find(hdir, fdbuf, ph_strlen(fdbuf), &out, 1);

	hal_mutex_unlock(&fd_dir_lock);

	ph_free(fdbuf);
	fdbuf = NULL;

	struct fd_entry *entry = (struct fd_entry *)out->da;
	genode_close(entry);
	entry = NULL;
	ref_dec_o(out);

	pvm_object_t fd_o = pvm_create_int_object(fd);
	pvm_append_array(fd_freelist, fd_o);
}


ssize_t fs_write(fd_t fd, const void *buf, size_t n)
{
	pvm_object_t fs_o = pvm_get_fs_class();

	pvm_object_t fd_dir = pvm_data_area(fs_o, fs)->fd_dir;
	hashdir_t *hdir = pvm_data_area(fd_dir, directory);

	char *fdbuf = __fd_to_str(fd);

	hal_mutex_lock(&fd_dir_lock);

	pvm_object_t out;
	hdir_find(hdir, fdbuf, ph_strlen(fdbuf), &out, 0);

	hal_mutex_unlock(&fd_dir_lock);

	ph_free(fdbuf);
	fdbuf = NULL;

	struct fd_entry *entry = (struct fd_entry *)out->da;
	return genode_write(entry, buf, n);
}


ssize_t fs_read(fd_t fd, void *buf, size_t n)
{
	pvm_object_t fs_o = pvm_get_fs_class();

	pvm_object_t fd_dir = pvm_data_area(fs_o, fs)->fd_dir;
	hashdir_t *hdir = pvm_data_area(fd_dir, directory);

	char *fdbuf = __fd_to_str(fd);

	hal_mutex_lock(&fd_dir_lock);

	pvm_object_t out;
	hdir_find(hdir, fdbuf, ph_strlen(fdbuf), &out, 0);

	hal_mutex_unlock(&fd_dir_lock);

	ph_free(fdbuf);
	fdbuf = NULL;

	struct fd_entry *entry = (struct fd_entry *)out->da;
	return genode_read(entry, buf, n);
}


ssize_t fs_readlinkat(int dirfd, const char *path, char *buf, size_t bufsize)
{
	// TODO Add support for relative paths (i.e. relative to dirfd)
  (void) path;
  
	return genode_readlinkat(path, buf, bufsize);
}


int fs_fstat(fd_t fd, struct stat *buf)
{
	pvm_object_t fs_o = pvm_get_fs_class();

	pvm_object_t fd_dir = pvm_data_area(fs_o, fs)->fd_dir;
	hashdir_t *hdir = pvm_data_area(fd_dir, directory);

	char *fdbuf = __fd_to_str(fd);

	hal_mutex_lock(&fd_dir_lock);

	pvm_object_t out;
	hdir_find(hdir, fdbuf, ph_strlen(fdbuf), &out, 0);

	hal_mutex_unlock(&fd_dir_lock);

	ph_free(fdbuf);
	fdbuf = NULL;

	struct fd_entry *entry = (struct fd_entry *)out->da;
	return genode_fstat(entry, buf);
}


int fs_rename(const char *path, const char *newpath)
{
	pvm_object_t fs_o = pvm_get_fs_class();

	pvm_object_t fd_dir = pvm_data_area(fs_o, fs)->fd_dir;
	hashdir_t *hdir = pvm_data_area(fd_dir, directory);

	pvm_object_t keys;

	hal_mutex_lock(&fd_dir_lock);

	(void)hdir_keys(hdir, &keys);  // always returns 0

	int keys_size = pvm_get_array_size(keys);

	for (int i = 0; i < keys_size; ++i) {
		pvm_object_t entry_o = pvm_get_array_ofield(keys, i);
		struct fd_entry *entry = (struct fd_entry *)entry_o->da;

		if (ph_strncmp(entry->path, path, MAX_PATH_LEN) == 0)
			ph_strncpy(entry->path, newpath, MAX_PATH_LEN);
	}

	/* INFO
	 If system crashes here, hdir's entries' paths will not match what
	 is in the file-system!

   This may result in failures to open the entries inside
   pvm_internal_restart_fs().

   This is the downside of using the file-system for persistence. If
   fault-tolerant persistence is required, use PhantomOS' native persistence.
   */
	int rc = genode_rename(path, newpath);

	hal_mutex_unlock(&fd_dir_lock);

	return rc;
}
