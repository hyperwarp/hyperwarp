/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>

#include "util.h"
#include "metadata.h"

int main()
{
	MetaData *metadata = new_metadata();
	for (int i = 0; i < 8; i++)
	{
		create_physical_disk(metadata, 937684566, 4096); // 3.84 TB
	}
	VirtualDisk *vdisk = create_and_allocate_virtual_disk(
		metadata,
		"disk1",
		VIRTUAL_DISK__ERASURE_CODE_PROFILE__EC_4_2P,
		100);

	if (vdisk != NULL)
	{
		printf("virtual disk successfully allocated!\n");
		print_virtual_disk(vdisk, 0);

	}
	return 0;
}
