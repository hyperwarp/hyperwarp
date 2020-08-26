/* SPDX-License-Identifier: Apache-2.0 */

#include <metadata.h>

PhysicalDisk create_physical_disk(uint64_t sector_count, uint64_t sector_size)
{
        PhysicalDisk physical_disk = PHYSICAL_DISK__INIT;
        physical_disk.key = 111444890ULL;
        physical_disk.sector_count = sector_count;
        physical_disk.sector_size = sector_size;

	return physical_disk;
}

PhysicalDiskRange create_physical_disk_range(uint64_t key, uint64_t physical_disk_key, uint64_t sector_start, uint64_t sector_end, uint64_t sector_count) {
	PhysicalDiskRange range = PHYSICAL_DISK_RANGE__INIT;
	range.key = key;
	range.physical_disk_key = physical_disk_key;
	range.sector_start = sector_start;
	range.sector_end = sector_end;
	range.sector_count = sector_count;

	return range;
}

void add_physical_disk_range_to_physical_disk(PhysicalDisk* physical_disk, PhysicalDiskRange* range){
	physical_disk->n_physical_disk_ranges = 1;
	physical_disk->physical_disk_ranges = malloc(sizeof(PhysicalDiskRange*));
	physical_disk->physical_disk_ranges[0] = range;
}

VirtualDiskRange create_virtual_disk_range(uint64_t key, uint64_t sector_start, uint64_t sector_end, uint64_t sector_count) {
	VirtualDiskRange vd_range = VIRTUAL_DISK_RANGE__INIT;
	vd_range.key = key;
	vd_range.sector_start = sector_start;
	vd_range.sector_count = sector_count;
	vd_range.sector_end = sector_end;

	return vd_range;
}

VirtualDisk create_virtual_disk(uint64_t key, char *name, VirtualDisk__ErasureCodeProfile ec_profile) {
	VirtualDisk virtual_disk = VIRTUAL_DISK__INIT;
	virtual_disk.key = key;
	virtual_disk.name = name;
	virtual_disk.ec_profile = ec_profile;

	return virtual_disk;
}

void add_virtual_disk_range_to_virtual_disk(VirtualDisk* virtual_disk, VirtualDiskRange* range) {
	virtual_disk->n_volume_ranges = 1;
	virtual_disk->volume_ranges = malloc(sizeof(VirtualDiskRange*));
	virtual_disk->volume_ranges[0] = range;
}