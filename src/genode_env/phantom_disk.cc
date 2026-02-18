#include <base/log.h>
// #include <stdio.h>
// #include <stddef.h>
// #include <stdlib.h>
#include "phantom_env.h"

using namespace Phantom;

extern "C"
{

#include <disk.h>
#include <genode_disk_private.h>

	// XXX : Phantom expects this size of the block. Need to be fixed in the future
	static int block_size = 512;

	void driver_genode_update_disk_dev(genode_disk_dev_t *dest)
	{
		auto info = main_obj->_disk.getInfo();
		static char name[] = "GenodeDisk0";

		dest->block_count = info.block_count;
		dest->block_size = info.block_size;
		dest->name = name;
	}

	// // Note: block_no and len should be in blocks!
	// void driver_genode_disk_read(genode_disk_dev_t *vd, void *dest, int block_no,
	// size_t len)
	// {
	//     (void)vd;

	//     if (len % block_size != 0)
	//     {
	//         Genode::warning("Incorrect len for disk read, not block aligned! (addr=",
	//         dest, " block=", block_no, " len=", len, ")"); return;
	//     }

	//     // TODO : Do we need to sync here?
	//     main_obj->_disk.submit(Disk_backend::Operation::READ, false, block_no *
	//     block_size, len, dest);
	// }

	// void driver_genode_disk_write(genode_disk_dev_t *vd, void *src, int block_no,
	// size_t len)
	// {
	//     (void)vd;

	//     if (len % block_size != 0)
	//     {
	//         Genode::warning("Incorrect len for disk write, not block aligned! (addr=",
	//         src, " block=", block_no, " len=", len, ")"); return;
	//     }

	//     // TODO : Do we need to sync here?
	//     main_obj->_disk.submit(Disk_backend::Operation::WRITE, true, block_no *
	//     block_size, len, src);
	// }

	int driver_genode_disk_asyncIO(struct phantom_disk_partition *part,
								   pager_io_request *rq)
	{
		const int sector_size = 512;

		// Genode::log("!!! : received blockNo ", rq->blockNo);

		// Getting data from rq
		u_int64_t blockNo = rq->blockNo;

		// XXX : For some reason, data from C code looks different here
		//       so rq->blockNo is exactly 1 less than in C except for 0.
		//       Need to fix somehow
		// if (blockNo != 0){
		//     blockNo--;
		// }

		int nSect = rq->nSect;
		physaddr_t pa = rq->phys_page;

		// Calculating size
		int length_in_bytes = nSect * sector_size;
		int length_in_blocks = length_in_bytes / part->block_size +
							   ((length_in_bytes % part->block_size) ? 1 : 0);
		// Genode::log("!!! Calcs:", blockNo , ", ", length_in_bytes / part->block_size, "
		// (", length_in_bytes % part->block_size, ")" );

		// XXX : Seems to be not used
		rq->parts = nSect;

		// Creating Genode's operation
		Block::Operation op;

		if (rq->flag_pageout) {
			op.type = Block::Operation::Type::WRITE;
		} else {
			op.type = Block::Operation::Type::READ;
		}
		op.block_number = blockNo;
		op.count = length_in_blocks;

		main_obj->_disk.startAsyncJob(op, rq);

		return 0;
	}

	errno_t driver_genode_disk_fence(struct phantom_disk_partition *p)
	{
		return main_obj->_disk.waitForPendingJobs();
	}
}