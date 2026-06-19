#include <os/vfs.h>

#include <ph_string.h>
#include <phantom_env.h>
#include <phantom_fs_internal.h>
#include <vm/alloc.h>
#include <vm/internal_da.h>


static_assert((u_int64_t)MAX_PATH_LEN == Vfs::MAX_PATH_LEN,
			  "compile-time constants do not match!");

#define X(mode, val)                                                          \
	static_assert((u_int64_t)mode == Vfs::Directory_service::Open_mode::mode, \
				  "compile-time constants do not match!");

Open_mode_ITER

#undef X


	extern "C"
{

	int genode_mkdir(const char *path)
	{
		main_obj->_fs_root.create_sub_directory(path);
		if (!main_obj->_fs_root.directory_exists(path)) {
			Genode::error("genode_mkdir(): could not create directory: ", path);
			return -1;
		}

		return 0;
	}

	fd_t genode_open(struct fd_entry * entry)
	{
		Vfs::Directory_service::Open_result res =
			main_obj->_fs.open(entry->path,
							   entry->mode,
							   (Vfs::Vfs_handle **)&entry->handle,
							   main_obj->_heap);

		switch (res) {
			case Vfs::Directory_service::OPEN_ERR_UNACCESSIBLE:
				Genode::error("genode_open(): unaccessible");
				return -res;

			case Vfs::Directory_service::OPEN_ERR_NO_PERM:
				Genode::error("genode_open(): no permission");
				return -res;

			case Vfs::Directory_service::OPEN_ERR_EXISTS:
				Genode::error(
					"genode_open(): trying to create a file that already exists");
				return -res;

				break;

			case Vfs::Directory_service::OPEN_ERR_NAME_TOO_LONG:
				Genode::error(
					"genode_open(): path is too long, exceeds Vfs::MAX_PATH_LEN");
				return -res;

			case Vfs::Directory_service::OPEN_ERR_NO_SPACE:
				Genode::error("genode_open(): out of disk space");
				return -res;

			case Vfs::Directory_service::OPEN_ERR_OUT_OF_RAM:
				Genode::error("genode_open(): out of memory");
				return -res;

			case Vfs::Directory_service::OPEN_ERR_OUT_OF_CAPS:
				Genode::error("genode_open(): out of caps");
				return -res;

			case Vfs::Directory_service::OPEN_OK:
				break;

			default:
				break;
		}

		return res;
	}


	void genode_close(struct fd_entry * entry)
	{
    Genode::warning("closing handle: ", entry->handle);
		main_obj->_fs.close((Vfs::Vfs_handle *)entry->handle);
	}


	ssize_t genode_write(struct fd_entry * entry, const void *buf, size_t n)
	{
		size_t out = 0;
		const Genode::Span byte_range {(char *)buf, n};
    Vfs::Vfs_handle *handle = (Vfs::Vfs_handle *)entry->handle;

    handle->seek(out);

  SYNC:
    while (! handle->fs().write_ready(*handle)) {
      main_obj->_fs_root.io().commit_and_wait();
    }
    
		Vfs::File_io_service::Write_result res =
			handle->fs().write(handle, byte_range, out);

		switch (res) {
			case Vfs::File_io_service::Write_result::WRITE_ERR_WOULD_BLOCK:
				goto SYNC;
        
			case Vfs::File_io_service::Write_result::WRITE_ERR_INVALID:
				Genode::error("genode_write(): invalid write");
				return -res;

			case Vfs::File_io_service::Write_result::WRITE_ERR_IO:
				Genode::error("genode_write(): i/o error");
				return -res;

			case Vfs::File_io_service::Write_result::WRITE_OK:
        handle->seek(out);
				break;

			default:
				break;
		}

    main_obj->_fs_root.io().commit_and_wait();    
		return out;
	}


	ssize_t genode_read(struct fd_entry * entry, void *buf, size_t n)
	{
		size_t out = 0;
		Genode::Byte_range_ptr byte_range {(char *)buf, n};

		if (!main_obj->_fs.queue_read((Vfs::Vfs_handle *)entry->handle, n)) {
			Genode::error("genode_read(): read queue is full");
			return 0;
		}

		Vfs::File_io_service::Read_result res = main_obj->_fs.complete_read(
			(Vfs::Vfs_handle *)entry->handle, byte_range, out);

		switch (res) {
			case Vfs::File_io_service::Read_result::READ_ERR_WOULD_BLOCK:
				Genode::error("genode_read(): read would block");
				return -res;

			case Vfs::File_io_service::Read_result::READ_ERR_INVALID:
				Genode::error("genode_read(): invalid read");
				return -res;

			case Vfs::File_io_service::Read_result::READ_ERR_IO:
				Genode::error("genode_read(): i/o error");
				return -res;

			case Vfs::File_io_service::Read_result::READ_OK:
				break;

			default:
				break;
		}

		return out;
	}


	ssize_t genode_readlinkat(const char *path, char *buf, size_t bufsize)
	{
    if (!main_obj->_fs_root.symlink_exists(path))
      return -1;
    
    /* TODO Make a distinction between open and read error codes! */
		Vfs::Vfs_handle *handle;
		size_t out = 0;
		Genode::Byte_range_ptr byte_range {buf, bufsize};

		Vfs::Directory_service::Openlink_result open_res =
			main_obj->_fs.openlink(path, false, &handle, main_obj->_heap);

		switch (open_res) {
			case Vfs::Directory_service::Openlink_result::OPENLINK_ERR_LOOKUP_FAILED:
				return -open_res;

			case Vfs::Directory_service::Openlink_result::OPENLINK_ERR_NAME_TOO_LONG:
				return -open_res;

			case Vfs::Directory_service::Openlink_result::OPENLINK_ERR_NODE_ALREADY_EXISTS:
				return -open_res;

			case Vfs::Directory_service::Openlink_result::OPENLINK_ERR_NO_SPACE:
				return -open_res;

			case Vfs::Directory_service::Openlink_result::OPENLINK_ERR_OUT_OF_RAM:
				return -open_res;

			case Vfs::Directory_service::Openlink_result::OPENLINK_ERR_OUT_OF_CAPS:
				return -open_res;

			case Vfs::Directory_service::Openlink_result::OPENLINK_ERR_PERMISSION_DENIED:
				return -open_res;

			case Vfs::Directory_service::Openlink_result::OPENLINK_OK:
				break;

			default:
				break;
		}


		if (!main_obj->_fs.queue_read(handle, bufsize)) {
			main_obj->_fs.close(handle);

			Genode::error("genode_openlinkat(): read queue is full");
			return 0;
		}

		Vfs::File_io_service::Read_result read_res = main_obj->_fs.complete_read(
			handle, byte_range, out);

		switch (read_res) {
			case Vfs::File_io_service::Read_result::READ_ERR_WOULD_BLOCK:
				main_obj->_fs.close(handle);
				Genode::error("genode_openlinkat(): read would block");
				return -read_res;

			case Vfs::File_io_service::Read_result::READ_ERR_INVALID:
				main_obj->_fs.close(handle);
				Genode::error("genode_openlinkat(): invalid read");
				return -read_res;

			case Vfs::File_io_service::Read_result::READ_ERR_IO:
				main_obj->_fs.close(handle);
				Genode::error("genode_openlinkat(): i/o error");
				return -read_res;

			case Vfs::File_io_service::Read_result::READ_OK:
				break;

			default:
				break;
		}
    
		main_obj->_fs.close(handle);
		return out;
	}


	int genode_fstat(struct fd_entry * entry, struct stat * buf)
	{
		Vfs::Directory_service::Stat stats;
		Vfs::Directory_service::Stat_result res = main_obj->_fs.stat(Genode::Directory::join("/", entry->path).string(), stats);

		switch (res) {
			case Vfs::Directory_service::Stat_result::STAT_ERR_NO_ENTRY:
				Genode::error("genode_fstat(): no entry: ", Genode::String<512>(entry->path));
				return -res;

			case Vfs::Directory_service::Stat_result::STAT_ERR_NO_PERM:
				Genode::error("genode_fstat(): no permission");
				return -res;

			case Vfs::Directory_service::Stat_result::STAT_OK:
				break;

			default:
				break;
		}

		buf->st_size = stats.size;
		buf->st_dev = stats.device;
		buf->st_ino = stats.inode;

		/* NOTE
	   Genode does not support hard-linking,
	   hence a file's link count cannot exceed 1.
	*/
		buf->st_nlink = 1;


		uint64_t modification_time_sec = stats.modification_time.ms_since_1970 / 1000;
		uint64_t modification_time_nsec = (stats.modification_time.ms_since_1970 % 1000) *
										  1000000UL;

		struct timespec ts = {
			.tv_sec = modification_time_sec,
			.tv_nsec = modification_time_nsec,
		};

		buf->st_atim = ts;
		buf->st_mtim = ts;
		buf->st_ctim = ts;


		int file_mode;

		switch (stats.type) {
			case Vfs::Node_type::DIRECTORY:
				file_mode = S_IFDIR;
				break;

			case Vfs::Node_type::SYMLINK:
				file_mode = S_IFLNK;
				break;

			case Vfs::Node_type::CONTINUOUS_FILE:
				file_mode = S_IFREG;
				break;

			case Vfs::Node_type::TRANSACTIONAL_FILE:
				file_mode = S_IFREG;
				break;

			default:
				break;
		}

		if (stats.rwx.readable)
			file_mode |= S_IRUSR | S_IRGRP | S_IROTH;

		if (stats.rwx.writeable)
			file_mode |= S_IWUSR | S_IWGRP | S_IWOTH;

		if (stats.rwx.executable)
			file_mode |= S_IXUSR | S_IXGRP | S_IXOTH;

		buf->st_mode = file_mode;

		return 0;
	}


	int genode_rename(const char *path, const char *newpath)
	{
		Vfs::Directory_service::Rename_result res = main_obj->_fs.rename(path, newpath);

		switch (res) {
			case Vfs::Directory_service::Rename_result::RENAME_ERR_NO_ENTRY:
				Genode::error("genode_rename(): no entry");
				return -res;

			case Vfs::Directory_service::Rename_result::RENAME_ERR_CROSS_FS:
				Genode::error("genode_rename(): cross fs");
				return -res;

			case Vfs::Directory_service::Rename_result::RENAME_ERR_NO_PERM:
				Genode::error("genode_rename(): no permission");
				return -res;

			case Vfs::Directory_service::Rename_result::RENAME_OK:
				break;

			default:
				break;
		}

		return res;
	}
}
