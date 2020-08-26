/* SPDX-License-Identifier: Apache-2.0 */

#include <metadata.h>
#include <stdlib.h>
#include <string.h>

PhysicalDisk *create_physical_disk(MetaData *metadata,
				   uint64_t sector_count,
				   uint64_t sector_size)
{
        PhysicalDisk *physical_disk = (PhysicalDisk*)malloc(sizeof(PhysicalDisk));
	physical_disk__init(physical_disk);

        physical_disk->key = 111444890ULL;
        physical_disk->sector_count = sector_count;
        physical_disk->sector_size = sector_size;
        physical_disk->size = sector_count * sector_size;
        physical_disk->unallocated_sector_count = sector_count;
        physical_disk->allocated_sector_count = 0;

	if (metadata != NULL)
	{
		add_physical_disk(metadata, physical_disk);
	}

	return physical_disk;
}

PhysicalDiskRange *create_physical_disk_range(uint64_t key, uint64_t physical_disk_key, uint64_t sector_start, uint64_t sector_end, uint64_t sector_count) {
	PhysicalDiskRange *range = (PhysicalDiskRange*)malloc(sizeof(PhysicalDiskRange));
	physical_disk_range__init(range);

        range->key = key;
	range->physical_disk_key = physical_disk_key;
	range->sector_start = sector_start;
	range->sector_end = sector_end;
	range->sector_count = sector_count;

	return range;
}

void add_physical_disk_range_to_physical_disk(PhysicalDisk* physical_disk, PhysicalDiskRange* range) {
	size_t last = physical_disk->n_ranges;
	physical_disk->ranges = realloc(physical_disk->ranges,
					sizeof(PhysicalDiskRange*) * (last + 1));
	physical_disk->ranges[last] = range;
        physical_disk->allocated_sector_count += range->sector_count;
        physical_disk->unallocated_sector_count -= range->sector_count;
	physical_disk->n_ranges = last + 1;
}

VirtualDiskRange *create_virtual_disk_range(uint64_t key, uint64_t sector_start, uint64_t sector_end, uint64_t sector_count) {
	VirtualDiskRange *vd_range = (VirtualDiskRange*)malloc(sizeof(VirtualDiskRange));
	virtual_disk_range__init(vd_range);

        vd_range->key = key;
	vd_range->sector_start = sector_start;
	vd_range->sector_count = sector_count;
	vd_range->sector_end = sector_end;

	return vd_range;
}

VirtualDisk *create_virtual_disk(MetaData *metadata,
				 const char *name,
				 VirtualDisk__ErasureCodeProfile ec_profile,
				 uint64_t size)
{
	VirtualDisk *virtual_disk = (VirtualDisk*)malloc(sizeof(VirtualDisk));
	virtual_disk__init(virtual_disk);

	virtual_disk->key = 123ULL;
	virtual_disk->name = (char *)malloc(strlen(name) + 1);
	memcpy(virtual_disk->name, name, strlen(name) + 1);
	virtual_disk->ec_profile = ec_profile;
	virtual_disk->size = size;

	if (metadata != NULL)
	{
		add_virtual_disk(metadata, virtual_disk);
	}

	return virtual_disk;
}

size_t find_free_disk_index(MetaData *metadata, int index, uint64_t sector_count)
{
	return index % metadata->n_physical_disks;
}

PhysicalDiskRange *allocate_next_physical_disk_range(MetaData *metadata,
                                                     int disk_index,
                                                     uint64_t sector_count)
{
	PhysicalDisk *disk = metadata->physical_disks[disk_index];

	PhysicalDiskRange *range = (PhysicalDiskRange*)malloc(sizeof(PhysicalDiskRange));
	physical_disk_range__init(range);

	if (disk->n_ranges > 0)
	{
		range->sector_start = disk->ranges[disk->n_ranges - 1]->sector_end + 1;
	} else {
		range->sector_start = 0;
	}
	range->sector_end = range->sector_start + sector_count - 1;
	range->sector_count = sector_count;

	add_physical_disk_range_to_physical_disk(disk, range);

	return range;
}

void add_physical_disk_range_to_virtual_disk_range(VirtualDiskRange *vdr, PhysicalDiskRange *pdr)
{
	size_t last = vdr->n_ranges;
	vdr->ranges = realloc(vdr->ranges,
			      sizeof(PhysicalDiskRange*) * (last + 1));
	vdr->ranges[last] = pdr;
	vdr->n_ranges = last + 1;
}

VirtualDiskRange *new_virtual_disk_range()
{
	VirtualDiskRange *vdr = (VirtualDiskRange*)malloc(sizeof(VirtualDiskRange));
	virtual_disk_range__init(vdr);
	return vdr;
}

VirtualDisk *create_and_allocate_virtual_disk(MetaData *metadata, const char *name, VirtualDisk__ErasureCodeProfile ec_profile, uint64_t size)
{
	/*
	 * if we see a profile that we did not encounter for,
	 * this will assure that our check later will fail
	 */
	uint64_t minimum_step_size = size + 1;

	if (ec_profile == 0) { // 4+2
		minimum_step_size = 4;
	}

	if (size % minimum_step_size != 0)
	{
		return NULL;
	}

        /*
         * so let's say you need a virtual disk that is 100GiB
         * we're going to cut that into 100 x 1GB VirtualDiskRanges
         * then each VirtualDiskRange is going to map to 6xPhysicalDiskRanges
         * (for 4+2 EC)
         */

        VirtualDisk *virtual_disk = create_virtual_disk(metadata, name, ec_profile, size);

        for (int i = 0; i < size; i++) {
		VirtualDiskRange *vdr = new_virtual_disk_range();

		for (int j = 0; j < 6; j++) {
			int next_disk_index = find_free_disk_index(metadata,
					i*6 + j,
					SIMPLE_ALLOCATOR_PHYSICAL_DISK_RANGE_SECTOR_COUNT);
			PhysicalDiskRange *pdr = allocate_next_physical_disk_range(metadata,
					next_disk_index,
					SIMPLE_ALLOCATOR_PHYSICAL_DISK_RANGE_SECTOR_COUNT);
			add_physical_disk_range_to_virtual_disk_range(vdr, pdr);
		}

		add_virtual_disk_range_to_virtual_disk(virtual_disk, vdr);
        }

	return virtual_disk;
}

void add_virtual_disk_range_to_virtual_disk(VirtualDisk* virtual_disk, VirtualDiskRange* range) {
	size_t last = virtual_disk->n_ranges;
	virtual_disk->ranges = realloc(virtual_disk->ranges,
				       sizeof(VirtualDiskRange*) * (last + 1));
	virtual_disk->ranges[last] = range;
	virtual_disk->n_ranges = last + 1;
}

MetaData *new_metadata() {
	MetaData *metadata = (MetaData*)malloc(sizeof(MetaData));
	meta_data__init(metadata);

	return metadata;
}

void add_physical_disk(MetaData *metadata, PhysicalDisk *disk)
{
	size_t last = metadata->n_physical_disks;
	metadata->physical_disks = realloc(metadata->physical_disks,
					   sizeof(PhysicalDisk) * (last + 1));
	metadata->physical_disks[last] = disk;
	metadata->n_physical_disks = last + 1;
}

void add_virtual_disk(MetaData *metadata, VirtualDisk *disk)
{
	size_t last = metadata->n_virtual_disks;
	metadata->virtual_disks = realloc(metadata->virtual_disks,
					   sizeof(VirtualDisk) * (last + 1));
	metadata->virtual_disks[last] = disk;
	metadata->n_virtual_disks = last + 1;
}
