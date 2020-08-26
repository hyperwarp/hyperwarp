/* SPDX-License-Identifier: Apache-2.0 */

#include <metadata.pb-c.h>

PhysicalDisk *create_physical_disk(MetaData *metadata, uint64_t sector_count, uint64_t sector_size);

PhysicalDiskRange *create_physical_disk_range(uint64_t key, uint64_t physical_disk_key, uint64_t sector_start, uint64_t sector_end, uint64_t sector_count);

void add_physical_disk_range_to_physical_disk(PhysicalDisk* physical_disk, PhysicalDiskRange* range);

VirtualDiskRange *create_virtual_disk_range(uint64_t key, uint64_t sector_start, uint64_t sector_end, uint64_t sector_count);

VirtualDisk *create_virtual_disk(MetaData *metadata, const char *name, VirtualDisk__ErasureCodeProfile ec_profile, uint64_t size);

void add_virtual_disk_range_to_virtual_disk(VirtualDisk* virtual_disk, VirtualDiskRange* range);

MetaData *create_metadata();

void add_physical_disk(MetaData *metadata, PhysicalDisk *disk);
void add_virtual_disk(MetaData *metadata, VirtualDisk *disk);
