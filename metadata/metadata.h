/* SPDX-License-Identifier: Apache-2.0 */

#ifndef HYPERWARP_METADATA_INCLUDED
#define HYPERWARP_METADATA_INCLUDED

#include "metadata.pb-c.h"
#include <sys/queue.h>

typedef struct _Allocator Allocator;

struct _Allocator {
    VirtualDisk__ErasureCodeProfile ec_profile;
    size_t n;
    size_t p;
    size_t virtual_disk_range_sector_count;
    size_t physical_disk_range_sector_count;
    size_t sector_size;
};

struct MetadataBackend {
    const char *name;
    void (*initialize)();
    Metadata *(*load)();
    void (*persist)();
    SLIST_ENTRY(MetadataBackend) slist;
};

typedef struct MetadataBackend MetadataBackend;

void metadata_storage_backend_register(MetadataBackend *backend);
MetadataBackend *get_metadata_backend_by_name(const char* name);
int use_metadata_storage_backend(const char *name);
void register_dso_module_init(void (*fn)(void));

Allocator *create_allocator(VirtualDisk__ErasureCodeProfile ec_profile);

Metadata *new_metadata();

NVMfTransport *create_nvmf_transport(NVMfTransport__NVMfTransportType type, NVMfTransport__NVMfAddressFamily address_family, char *address, char *service_id, char *subsystem_nqn);

PhysicalDisk *create_physical_disk(Metadata *metadata, NVMfTransport *transport, uint64_t sector_count, uint64_t sector_size, Allocator *allocator);

VirtualDisk *create_virtual_disk(Metadata *metadata, char *name, uint64_t size, Allocator *allocator);

// map a section of a logical disk to a list of underlying virtual disk ranges
VirtualDiskRange **translate_vdaddress_to_vdranges(Allocator *allocator, VirtualDisk *vdisk, size_t start_address, size_t io_size);

#define module_init(function)                                               \
static void __attribute__((constructor)) hyperwarp_m_init ## function(void) \
{                                                                           \
    register_dso_module_init(function);                                     \
}

void register_dso_module_init(void (*fn)(void));
void metadata_storage_backend_register(MetadataBackend *backend);

#endif