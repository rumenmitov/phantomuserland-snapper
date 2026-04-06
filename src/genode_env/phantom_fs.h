#ifndef PHANTOM_FS_H
#define PHANTOM_FS_H

/**
   phantom_fs.h - C API for file-system operations.
 */


#include <phantom_types.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

	/**
	 * @brief 	Creates a new file.
	 * @param 	path	- The absolute path of the new file.
	 * @return 	0 on success, -1 on failure.
	 */
	int32_t ph_fs_create(const char *path);


	/**
	 * @brief 	Creates a new directory.
	 * @param 	path	- The absolute path of the new directory.
	 * @returns 0 on sucess, -1 on failure.
	 */
	int32_t ph_fs_mkdir(const char *path);


	/**
	 * @brief 	Writes to a file. Creates one if it does not exist.
	 * @param 	path 		- The absolute path to the file.
	 * @param 	buf  		- A buffer of the data that should be written to the
	 * 										file.
	 * @param 	bufsize	- The number of bytes in buf.
	 * @return 	0 on sucess, negative value on failure.
	 */
	int32_t ph_fs_write(const char *path, void *buf, size_t bufsize);


	/**
	 * @brief 	Reads from a file into a pre-allocated buffer.
	 * @param 	path 		- The absolute path to the file.
	 * @param 	buf  		- A buffer of the data that should be written to the
	 * 										file. It should be pre-allocated.
	 * @param 	bufsize	- The number of bytes allocated in buf.
	 * @return 	0 on sucess, -1 on failure.
	 */
	int32_t ph_fs_read(const char *path, void *buf, size_t bufsize);


	/**
	 * @brief 	Returns the file size.
	 * @param 	path 		- The absolute path to the file.
	 * @return 	Positive value is the file size, negative value
	 * 					indicates failure.
	 */
	int32_t ph_fs_file_size(const char *path);


	/**
	 * @brief 	Renames a file or directory.
	 * @param 	oldpath 		- The absolute old path to the file /
	 * 												directory.
	 * @param 	oldpath 		- The absolute new path to the file / directory.
	 * @return 	0 on success, negative value on failure.
	 */
	int32_t ph_fs_rename(const char *oldpath, const char *newpath);


	/**
	 * @brief 	Remove a file or directory.
	 * @param 	path	- The absolute path to the file / directory.
	 */
	void ph_fs_unlink(const char *path);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // PHANTOM_FS_H
