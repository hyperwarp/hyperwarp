/* SPDX-License-Identifier: Apache-2.0 */

#include <metadata.h>
#include <stdlib.h>

PhysicalDisk *create_physical_disk(MetaData *metadata,
				   uint64_t sector_count,
				   uint64_t sector_size)
{
        PhysicalDisk *physical_disk = (PhysicalDisk*)malloc(sizeof(PhysicalDisk));
	physical_disk__init(physical_disk);
        physical_disk->key = 111444890ULL;
        physical_disk->sector_count = sector_count;
        physical_disk->sector_size = sector_size;

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
	size_t last = physical_disk->n_physical_disk_ranges;
	physical_disk->physical_disk_ranges = realloc(physical_disk->physical_disk_ranges,
						      sizeof(PhysicalDiskRange*) * (last + 1));
	physical_disk->physical_disk_ranges[last] = range;
	physical_disk->n_physical_disk_ranges = last + 1;
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
				 uint64_t size) {
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

void add_virtual_disk_range_to_virtual_disk(VirtualDisk* virtual_disk, VirtualDiskRange* range) {
	size_t last = virtual_disk->n_volume_ranges;
	virtual_disk->volume_ranges = realloc(virtual_disk->volume_ranges,
					      sizeof(VirtualDiskRange*) * (last + 1));
	virtual_disk->volume_ranges[last] = range;
	virtual_disk->n_volume_ranges = last + 1;
}

MetaData *create_metadata() {
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
