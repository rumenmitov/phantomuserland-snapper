#include <phantom_env.h>
#include "phantom_fs.h"


int32_t ph_fs_create(const char *path)
{
	int32_t rc = 0;

	try {
		Genode::New_file(main_obj->_fs_root, path);
	} catch (Genode::New_file::Create_failed) {
		rc = -1;
	}

	return rc;
}


int32_t ph_fs_mkdir(const char *path)
{
	int32_t rc = 0;

	main_obj->_fs_root.create_sub_directory(path);
	if (!main_obj->_fs_root.directory_exists(path))
		rc = -1;

	return rc;
}


int32_t ph_fs_write(const char *path, void *buf, size_t bufsize)
{
	int32_t rc = 0;

	try {
		Genode::New_file file(main_obj->_fs_root, path);
		Genode::Const_byte_range_ptr buffer {(char *)buf, bufsize};

		Genode::New_file::Append_result res = file.append(buffer);
		if (res != Genode::New_file::Append_result::OK) {
			rc = -2;
		}

	} catch (Genode::New_file::Create_failed) {
		rc = -1;
	}

	return rc;
}


int32_t ph_fs_read(const char *path, void *buf, size_t bufsize)
{
	int32_t rc = 0;

	try {
		Genode::Readonly_file file(main_obj->_fs_root, path);
		Genode::Byte_range_ptr buffer {(char *)buf, bufsize};

		(void)file.read(buffer);

	} catch (Genode::Readonly_file::Open_failed) {
		rc = -1;
	}

	return rc;
}

int32_t ph_fs_file_size(const char *path)
{
	int32_t rc = 0;

	try {
		rc = main_obj->_fs_root.file_size(path);
	} catch (Genode::Directory::Nonexistent_file) {
		rc = -1;
	}

	return rc;
}

int32_t ph_fs_rename(const char *oldpath, const char *newpath)
{
	int32_t rc = 0;

	Vfs::Directory_service::Rename_result res =
		main_obj->_fs_root.root_dir().rename(oldpath, newpath);

	switch (res) {
		case Vfs::Directory_service::Rename_result::RENAME_ERR_NO_ENTRY:
      Genode::error("rename: no entry");
			rc = -1;
			break;

		case Vfs::Directory_service::Rename_result::RENAME_ERR_CROSS_FS:
      Genode::error("rename: cross fs");
			rc = -2;
			break;

		case Vfs::Directory_service::Rename_result::RENAME_ERR_NO_PERM:
      Genode::error("rename: no perm");
			rc = -3;
			break;

		case Vfs::Directory_service::Rename_result::RENAME_OK:
			break;

		default:
			break;
	}

	return rc;
}


void ph_fs_unlink(const char *path)
{
	main_obj->_fs_root.unlink(path);
}
