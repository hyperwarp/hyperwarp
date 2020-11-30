/* SPDX-License-Identifier: Apache-2.0 */

#ifndef HYPERWARP_METADATA_INCLUDED
#define HYPERWARP_METADATA_INCLUDED

#include "metadata.pb-c.h"
#include <sys/queue.h>
#include <stdbool.h>

typedef struct _Allocator Allocator;

static SLIST_HEAD(, MetadataBackendContext) g_storage_backends = SLIST_HEAD_INITIALIZER(MetadataBackendContext);

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
    int (*initialize)();
    Metadata *(*load)();
    int (*persist)(Metadata *);
    int (*finalize)();
};

typedef struct MetadataBackend MetadataBackend;


/*
    we need to build a context around the backends, because some backends
    like fdb absolutely do not allow you to call their initialization routines
    twice in a single process due to a lot of global state inside the lib

    see https://github.com/apple/foundationdb/issues/2981
*/
struct MetadataBackendContext {
    bool was_already_initialized;
    MetadataBackend *backend;
    void *backend_lib_handler;
    bool is_safe_to_reuse;
    SLIST_ENTRY(MetadataBackendContext) slist;
};

typedef struct MetadataBackendContext MetadataBackendContext;

struct HyperwarpMetadata {
    Metadata *metadata;

    MetadataBackendContext *backend_context;

    Allocator *allocator;
};

typedef struct HyperwarpMetadata HyperwarpMetadata;


void metadata_storage_backend_register(MetadataBackend *backend, bool is_safe_to_reuse);
static MetadataBackendContext *get_metadata_backend_context_by_name(const char* name);
void register_dso_module_init(void (*fn)(void));


int metadata_persist(HyperwarpMetadata *metadata);


HyperwarpMetadata *new_metadata(const char *backend, VirtualDisk__ErasureCodeProfile ec_profile);
HyperwarpMetadata *load_metadata(const char *backend);
void metadata_close(HyperwarpMetadata *metadata);

NVMfTransport *create_nvmf_transport(NVMfTransport__NVMfTransportType type, NVMfTransport__NVMfAddressFamily address_family, char *address, char *service_id, char *subsystem_nqn);

PhysicalDisk *create_physical_disk(HyperwarpMetadata *metadata, NVMfTransport *transport, uint64_t sector_count, uint64_t sector_size);

VirtualDisk *create_virtual_disk(HyperwarpMetadata *metadata, char *name, uint64_t size);

// map a section of a logical disk to a list of underlying virtual disk ranges
VirtualDiskRange **translate_vdaddress_to_vdranges(HyperwarpMetadata *metadata, VirtualDisk *vdisk, size_t start_address, size_t io_size);

#define module_init(function)                                               \
static void __attribute__((constructor)) hyperwarp_m_init ## function(void) \
{                                                                           \
    register_dso_module_init(function);                                     \
}

void register_dso_module_init(void (*fn)(void));

#endif