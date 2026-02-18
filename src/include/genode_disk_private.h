/**
 *
 * Functions implemented in Genode environment
 *
 */

#include <arch/arch-types.h>

typedef struct genode_disk_dev
{
	char *name;
	size_t block_size;  /* size of one block in bytes */
	size_t block_count; /* number of blocks */
} genode_disk_dev_t;

void driver_genode_update_disk_dev(genode_disk_dev_t *dest);

void driver_genode_disk_read(genode_disk_dev_t *vd, void *data, int block_no, size_t len);
void driver_genode_disk_write(genode_disk_dev_t *vd,
							  void *data,
							  int block_no,
							  size_t len);