/* SPDX-License-Identifier: Apache-2.0 */

#include <inttypes.h>
#include <util.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "metadata.h"

void setup(MetaData *metadata, size_t physical_disk_size, size_t physical_disk_count, size_t virtual_disk_size, size_t virtual_disk_count)
{
	// Create some physical disks
	for (int i = 0; i < physical_disk_count; i++)
	{
		create_physical_disk(metadata, physical_disk_size, 4096);
	}

	// Create some virtual disks
	for (int i = 0; i < virtual_disk_count; i++) {
		char buffer [20];
		snprintf(buffer, 20, "disk-%d", i+1);
		create_and_allocate_virtual_disk(
			metadata,
			buffer,
			VIRTUAL_DISK__ERASURE_CODE_PROFILE__EC_4_2P,
			virtual_disk_size);
	}
}

int main()
{
	srand((unsigned) time(NULL));

	MetaData *metadata = new_metadata();

	size_t physical_disk_size = 937684566; // 3.84 TB
	size_t physical_disk_count = 8;

	size_t virtual_disk_size = 100; // 100 GB
	size_t virtual_disk_size_max_sector_size = 262144 * virtual_disk_size;
	size_t virtual_disk_count = floor((physical_disk_count * physical_disk_size) / 1.5 / virtual_disk_size_max_sector_size);

	setup(metadata, physical_disk_size, physical_disk_count, virtual_disk_size, virtual_disk_count);

	// randomly select 10 virtual disks and do a random write
	for (int i = 0; i < 10; i++) {
		size_t index = rand() % virtual_disk_count;

		uint64_t start_sector = rand() % virtual_disk_size_max_sector_size;
		uint64_t io_size = rand() % (virtual_disk_size_max_sector_size - start_sector);

		printf("Virtual disk: %s\n", metadata->virtual_disks[index]->name);
		printf("- Start: %" PRIu64 "\n", start_sector);
		printf("- Size:  %" PRIu64 "\n", io_size);

		VirtualDiskRange **ranges = translate_vdaddress_to_vdranges(metadata->virtual_disks[index], start_sector, io_size);
	}

	return 0;
}
