/**
 * Wrapper file-system functions for Genode's VFS interface.
 */
#ifndef __PHANTOM_FS_INTERNAL_H
#define __PHANTOM_FS_INTERNAL_H

#include <vm/object.h>
#include "stats.h"

enum
{
  MAX_FD_ENTRIES = 0xFFFF,
	MAX_PATH_LEN = 512,
	FD_STR_LEN = 8 /* fd_t is an int => 2^32 bits = (32 / 4) hex chars */
};


/* INFO
   Using an X-macro for the modes, so that their values can be
   asserted in phantom_fs_internal.cc.
 */
#define Open_mode_ITER    \
	X(OPEN_MODE_RDONLY, 0)  \
	X(OPEN_MODE_WRONLY, 1)  \
	X(OPEN_MODE_RDWR, 2)    \
	X(OPEN_MODE_ACCMODE, 3) \
	X(OPEN_MODE_CREATE, 0x0800)

typedef enum
{
#define X(mode, val) mode = val,
  Open_mode_ITER
#undef X
} Open_mode;


typedef int fd_t;


/**
 * @brief A file descriptor entry.
 */
struct fd_entry
{
	char path[MAX_PATH_LEN];
	void *handle;
	Open_mode mode;
};


/**
 * @brief	Stores the internal state of the file-system
 * 				(used and available file-descriptors).
 */
struct data_area_4_fs
{
	pvm_object_t fd_dir;       /* Directory of open file descriptors. */
	pvm_object_t fd_freelist;  /* Stack of unused file descriptors. */
};


#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

  /**
   * @brief   Creates directory.
   * @param   const char * 	- the absolute path of the directory.
   * @return  int          	- negative value on error, 0 otherwise.
   */
  int genode_mkdir(const char *path);

  
	/**
	 * @brief		Opens a file using Genode's VFS.
	 * @param		struct fd_entry *		- file descriptor entry.
	 * @return	int 								- positive value on success, zero or
	 * 																negative value on
	 * 																error.
	 */
	fd_t genode_open(struct fd_entry *entry);


	/**
	 * @brief		Closes a file using Genode's VFS.
	 * @param		struct fd_entry *		- file descriptor entry.
	 */
	void genode_close(struct fd_entry *entry);


  /**
   * @brief   Writes up to N bytes of BUF to the file referred to by
   *          ENTRY.
   * @param   struct fd_entry * 						- file descriptor entry.
   * @param   const void *                  - buffer to be written to file.
   * @param   size_t                        - maximum bytes to write.
   * @return  ssize_t                       - the number of bytes written on
   *                                          success, negative value on failure.
   */
	ssize_t genode_write(struct fd_entry *entry, const void *buf, size_t n);


	/**
   * @brief   Attempts to read up to N bytes to BUF from the file referred to by
   *          ENTRY.
   * @param   struct fd_entry * 						- file descriptor entry.
   * @param   const void *                  - buffer to be read into from file.
   * @param   size_t                        - maximum bytes to read.
   * @return  ssize_t                       - the number of bytes read on
   *                                          success, zero if the
   *                                          read queue is full, and negative value on failure.
   */
	ssize_t genode_read(struct fd_entry *entry, void *buf, size_t n);


  /**
   * @brief   Places  the  contents  of the symbolic link PATH in
   * 					the buffer BUF, which has size BUFSIZ.
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
  ssize_t genode_readlinkat(const char *path, char *buf, size_t bufsize);


  /**
   * @brief   Retrieves  information about the file pointed to by
   *          ENTRY.
   * @param   struct fd_entry *             - the file's file
   *                                          descriptor entry.
   * @param   struct stat *                 - the file's stat struct
   *                                          is returned here.
   */
  int genode_fstat(struct fd_entry *entry, struct stat *buf);
  

	/**
	 * @brief		Renames a path using Genode's VFS.
	 * @return	int 	- positive value on success, zero or
	 * 									negative value on	error.
	 */
	int genode_rename(const char *path, const char *newpath);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __PHANTOM_FS_INTERNAL_H
