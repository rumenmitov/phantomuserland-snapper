/**
 * File-system functions that translate the file path to an internal
 * file descriptor structure, then call the desired operation.
 */
#ifndef __I_FS_H
#define __I_FS_H

#include <phantom_fs_internal.h>
#include <phantom_types.h>


/**
 * @brief   Creates directory.
 * @param   const char * - the absolute path of the directory.
 * @param   u_int32_t		 - unused parameter. Exists for the sole purpose of
 *                 				 POSIX compatibility.
 * @return  int          - negative value on error, zero on success.
 */
int fs_mkdir(const char *path, u_int32_t mode);


/**
 * @brief   Opens a file, returning a file descriptor.
 * @param   const char *		- the file's path.
 * @param   Open_mode	      - the mode in which to open the file.
 * @return  fd_t            - positive value on success, zero or
 * 														negative value on	error.
 */
fd_t fs_open(const char *path, Open_mode mode);


/**
 * @brief		Closes a file.
 * @param		fd_t		- file descriptor.
 */
void fs_close(fd_t fd);


/**
 * @brief   Writes up to N bytes of BUF to the file referred to by
 *          file descriptor FD.
 * @param   fd_t 						- file descriptor of the file.
 * @param   const void *    - buffer to be written to file.
 * @param   size_t          - maximum bytes to write.
 * @return  ssize_t         - the number of bytes written on
 *                            success, negative value on failure.
 */
ssize_t fs_write(fd_t fd, const void *buf, size_t n);


/**
 * @brief   Attempts to read up to N bytes of BUF from the file referred to by
 *          file descriptor FD.
 * @param   fd_t 						- file descriptor of the file.
 * @param   void *          - buffer to be read from file.
 * @param   size_t          - maximum bytes to read.
 * @return  ssize_t         - the number of bytes read on
 *                            success, zero if the read queue is
 *                            full, and negative value on failure.
 */
ssize_t fs_read(fd_t fd, void *buf, size_t n);


  /**
   * @brief   Places  the  contents  of the symbolic link PATH in
   * 					the buffer BUF, which has size BUFSIZ.
   * @param   int                         - specifies the directory in
   * 					                              the case of a relative
   * 					                              path; currently PhantomOS
   * 					                              only supports absolute
   * 					                              paths, hence this parameter is ignored.
   * @param   const char *								- symbolic link path.
   * @param   char *                      - buffer wherein contents
   * 					                              are to be placed.
   * @param   size_t                      - size of buffer.
   * @return  ssize_t                     - on success, these calls return the number of bytes placed in buf. (If the
   *                                        returned value equals
   * 					                              bufsiz, then truncation may have occurred.)  On error,
   * 					                              -1 is returned and errno
   * 					                              is set to indicate the error.
   */
ssize_t fs_readlinkat(int dirfd, const char *path, char *buf, size_t bufsize);


/**
 * @brief   Retrieves information about the file pointed to by
 *          FD. The information is returned in BUF.
 * @param   fd_t             - file descriptor.
 * @param   struct stat *    - the file's stat struct is returned here.
 */
int fs_fstat(fd_t fd, struct stat *buf);


/**
 * @brief		Renames a path using Genode's VFS.
 * @return	int 	- positive value on success, zero or
 * 									negative value on	error.
 */
int fs_rename(const char *path, const char *newpath);


#endif  // __I_FS_H
