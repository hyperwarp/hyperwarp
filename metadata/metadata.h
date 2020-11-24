/* SPDX-License-Identifier: Apache-2.0 */

#ifndef HYPERWARP_METADATA_INCLUDED
#define HYPERWARP_METADATA_INCLUDED

#include "metadata.pb-c.h"

typedef struct _Allocator Allocator;

struct _Allocator {
    VirtualDisk__ErasureCodeProfile ec_profile;
    size_t n;
    size_t p;
    size_t virtual_disk_range_sector_count;
    size_t physical_disk_range_sector_count;
    size_t sector_size;
};

Allocator *create_allocator(VirtualDisk__ErasureCodeProfile ec_profile);

Metadata *new_metadata();

NVMfTransport *create_nvmf_transport(NVMfTransport__NVMfTransportType type, NVMfTransport__NVMfAddressFamily address_family, char *address, char *service_id, char *subsystem_nqn);

PhysicalDisk *create_physical_disk(Metadata *metadata, NVMfTransport *transport, uint64_t sector_count, uint64_t sector_size, Allocator *allocator);

VirtualDisk *create_virtual_disk(Metadata *metadata, char *name, uint64_t size, Allocator *allocator);

// map a section of a logical disk to a list of underlying virtual disk ranges
VirtualDiskRange **translate_vdaddress_to_vdranges(Allocator *allocator, VirtualDisk *vdisk, size_t start_address, size_t io_size);

#endif