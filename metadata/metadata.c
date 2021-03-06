/* SPDX-License-Identifier: Apache-2.0 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <uuid/uuid.h>
#include <inttypes.h>
#include <stdio.h>
#include <dlfcn.h>

#include "metadata.h"

void metadata_storage_backend_register(MetadataBackend *backend, bool is_safe_to_reuse) {
    if (get_metadata_backend_context_by_name(backend->name) != NULL) {
        return;
    }

    assert(backend->name);

    MetadataBackendContext *context = malloc(sizeof(MetadataBackendContext));
    context->was_already_initialized = false;
    context->is_safe_to_reuse = is_safe_to_reuse;
    context->backend = backend;

    SLIST_INSERT_HEAD(&g_storage_backends, context, slist);
}

int metadata_persist(HyperwarpMetadata *metadata) {
    int ret = 0;

    if (metadata->backend_context == NULL) {
        return 1;
    }

    return metadata->backend_context->backend->persist(metadata->metadata);
}

int metadata_backend_initialize(HyperwarpMetadata *metadata) {
    int ret = 0;

    if (metadata->backend_context == NULL) {
        return 1;
    }

    if ((metadata->backend_context->is_safe_to_reuse == false) &&
            (metadata->backend_context->was_already_initialized)) {
        fprintf(stderr, "The selected module cannot be reinitialized\n");
        return 1;
    }

    if ((ret = metadata->backend_context->backend->initialize(metadata->backend_context->was_already_initialized)) != 0) {
        return ret;
    }

    metadata->backend_context->was_already_initialized = true;

    return ret;
}


int metadata_backend_finalize(HyperwarpMetadata *metadata) {
    int ret = 0;

    if (metadata->backend_context == NULL) {
        // if there is no open backend then there is nothing to do
        return 0;
    }

    if ((ret = metadata->backend_context->backend->finalize()) != 0) {
        return ret;
    }

    return ret;
}

MetadataBackendContext *get_metadata_backend_context_by_name(const char* name) {
    MetadataBackendContext *context;

    SLIST_FOREACH(context, &g_storage_backends, slist) {
        assert(context->backend != NULL);
        if (strcmp(context->backend->name, name) == 0) {
            return context;
        }
    }

    return NULL;
}

static int use_metadata_storage_backend(HyperwarpMetadata *metadata, const char *name) {
    if (metadata->backend_context != NULL) {
        return 1;
    }

    char *module_so_name;
    asprintf(&module_so_name, "metadata-%s.so", name);

    void *handler;
    if ((handler = dlopen(module_so_name, RTLD_LAZY)) == NULL) {
        free(module_so_name);
        return 1;
    }

    free(module_so_name);

    if ((metadata->backend_context = get_metadata_backend_context_by_name(name)) == NULL) {
        return 1;
    }

    metadata->backend_context->backend_lib_handler = handler;

    return 0;
}

void register_dso_module_init(void (*fn)(void))
{
    fn();
}

void _generate_and_set_uuid(ProtobufCBinaryData *key) {
    key->data = realloc(key->data, sizeof(uuid_t));
    uuid_generate(key->data);
    key->len = sizeof(uuid_t);
}

int _physical_disk_comparator(const void *a, const void *b) {
    PhysicalDisk *physical_disk_a = *(PhysicalDisk**) a;
    PhysicalDisk *physical_disk_b = *(PhysicalDisk**) b;

    return (int) physical_disk_b->n_unallocated_ranges - physical_disk_a->n_unallocated_ranges;
}

Allocator *create_allocator(VirtualDisk__ErasureCodeProfile ec_profile)
{
    Allocator *result = malloc(sizeof(Allocator));
    result->ec_profile = ec_profile;

    switch (ec_profile)
    {
    case VIRTUAL_DISK__ERASURE_CODE_PROFILE__EC_8_2P:
        result->n = 8;
        result->p = 2;
        break;
    case VIRTUAL_DISK__ERASURE_CODE_PROFILE__EC_4_2P:
        result->n = 4;
        result->p = 2;
        break;
    }

    // currently, always assuming 4k devices
    result->sector_size = 4096;

    // assuming 4k devices = 1GiB = 262144
    result->virtual_disk_range_sector_count = 262144;

    // virtual disk range is being distributed across n physical disk ranges
    result->physical_disk_range_sector_count = result->virtual_disk_range_sector_count / result->n;

    return result;
}

HyperwarpMetadata *new_metadata(const char *backend_name, VirtualDisk__ErasureCodeProfile ec_profile)
{
    HyperwarpMetadata *metadata = (HyperwarpMetadata *)calloc(1, sizeof(HyperwarpMetadata));
    metadata->metadata = (Metadata*)calloc(1, sizeof(Metadata));
    metadata__init(metadata->metadata);
    metadata->metadata->ec_profile = ec_profile;
    metadata->allocator = create_allocator(ec_profile);

    if (use_metadata_storage_backend(metadata, backend_name) != 0) {
        fprintf(stderr, "Could not initialize the selected storage backend");
        free(metadata);
        return NULL;
    }

    if (metadata_backend_initialize(metadata) != 0) {
        fprintf(stderr, "Could not initialize the selected storage backend\n");
        free(metadata);
        return NULL;
    }

    return metadata;
}

HyperwarpMetadata *load_metadata(const char *backend_name) {
    HyperwarpMetadata *metadata = (HyperwarpMetadata *)calloc(1, sizeof(HyperwarpMetadata));

    if (use_metadata_storage_backend(metadata, backend_name) != 0) {
        fprintf(stderr, "Could not initialize the selected storage backend");
        free(metadata);
        return NULL;
    }

    if (metadata_backend_initialize(metadata) != 0) {
        fprintf(stderr, "Could not initialize the selected storage backend\n");
        free(metadata);
        return NULL;
    }

    metadata->metadata = metadata->backend_context->backend->load();

    if (metadata->metadata != NULL) {
        metadata->allocator = create_allocator(metadata->metadata->ec_profile);
        return metadata;
    } else {
        metadata_close(metadata);
        return NULL;
    }
}

void metadata_close(HyperwarpMetadata *metadata) {

    metadata__free_unpacked(metadata->metadata, NULL);
    metadata->metadata = NULL;

    if (metadata->backend_context->backend_lib_handler != NULL) {
        metadata->backend_context->backend->finalize();
        dlclose(metadata->backend_context->backend_lib_handler);
    }

    if (metadata->allocator != NULL) {
        free(metadata->allocator);
    }

    free(metadata);
}

void _add_physical_disk(Metadata *metadata, PhysicalDisk *disk)
{
    size_t last = metadata->n_physical_disks;
    metadata->physical_disks = realloc(metadata->physical_disks,
                                       sizeof(PhysicalDisk) * (last + 1));
    metadata->physical_disks[last] = disk;
    metadata->n_physical_disks = last + 1;
}

int _physical_disk_range_key_compare(const void *a, const void *b) {
    PhysicalDiskRange *range_a = *(PhysicalDiskRange**) a;
    PhysicalDiskRange *range_b = *(PhysicalDiskRange**) b;

    int disk_cmp = uuid_compare(range_a->key->disk_key.data, range_b->key->disk_key.data);
    if (disk_cmp != 0) {
        return disk_cmp;
    }

    return uuid_compare(range_a->key->range_key.data, range_b->key->range_key.data);
}

void _add_physical_disk_range(Metadata *metadata, PhysicalDiskRange *range) {
    size_t last = metadata->n_physical_disk_ranges;
    metadata->physical_disk_ranges = realloc(metadata->physical_disk_ranges,
                                     sizeof(PhysicalDiskRange *) * (last + 1));
    metadata->physical_disk_ranges[last] = range;
    metadata->n_physical_disk_ranges = last + 1;

    qsort(metadata->physical_disk_ranges, metadata->n_physical_disk_ranges, sizeof(PhysicalDiskRange *), _physical_disk_range_key_compare);
}

void _sort_physical_disks_by_unallocated_ranges(Metadata *metadata) {
    qsort(metadata->physical_disks, metadata->n_physical_disks, sizeof(PhysicalDisk *), _physical_disk_comparator);
}

PhysicalDiskRange *_new_physical_disk_range() {
    PhysicalDiskRange *range = malloc(sizeof(PhysicalDiskRange));
    physical_disk_range__init(range);

    range->key = malloc(sizeof(DiskRangeKey));
    disk_range_key__init(range->key);

    range->size = malloc(sizeof(DiskRangeSize));
    disk_range_size__init(range->size);

    return range;
}

PhysicalDiskRange *_get_physical_disk_range(Metadata *metadata, DiskRangeKey *key) {
    PhysicalDiskRange *key_range = _new_physical_disk_range();
    key_range->key = key;

    return (PhysicalDiskRange *) bsearch(key_range, metadata->physical_disk_ranges, metadata->n_physical_disk_ranges, sizeof(PhysicalDiskRange *), _physical_disk_range_key_compare);
}

void _add_virtual_disk(Metadata *metadata, VirtualDisk *disk)
{
    size_t last = metadata->n_virtual_disks;
    metadata->virtual_disks = realloc(metadata->virtual_disks,
                                      sizeof(VirtualDisk) * (last + 1));
    metadata->virtual_disks[last] = disk;
    metadata->n_virtual_disks = last + 1;
}

NVMfTransport *_new_nvmf_transport()
{
    NVMfTransport *transport = (NVMfTransport *)malloc(sizeof(NVMfTransport));
    nvmf_transport__init(transport);

    return transport;
}

char *_memcpy_from_str(char *src) {
    char *dest = (char *)malloc(strlen(src) + 1);
    memcpy(dest, src, strlen(src) + 1);
    return dest;
}

NVMfTransport *create_nvmf_transport(NVMfTransport__NVMfTransportType type, NVMfTransport__NVMfAddressFamily address_family, char *address, char *service_id, char *subsystem_nqn)
{
    NVMfTransport *transport = _new_nvmf_transport();

    transport->type = type;
    transport->address_family = address_family;

    transport->address = _memcpy_from_str(address);
    transport->service_id = _memcpy_from_str(service_id);
    transport->subsystem_nqn = _memcpy_from_str(subsystem_nqn);

    return transport;
}

DiskRangeKey *_new_disk_range_key() {
    DiskRangeKey *dest = malloc(sizeof(DiskRangeKey));
    disk_range_key__init(dest);
    return dest;
}

void _memcpy_protobufcbinary(ProtobufCBinaryData *dest, ProtobufCBinaryData *src) {
    dest->data = malloc(src->len);
    memcpy(dest->data, src->data, src->len);
    dest->len = src->len;
}

void _memcpy_disk_range_key(DiskRangeKey *dest, DiskRangeKey *src) {
    _memcpy_protobufcbinary(&dest->disk_key, &src->disk_key);
    _memcpy_protobufcbinary(&dest->range_key, &src->range_key);
}

void _add_unallocated_physical_disk_range(PhysicalDisk *physical_disk, PhysicalDiskRange *range)
{
    size_t last = physical_disk->n_unallocated_ranges;
    physical_disk->unallocated_ranges = realloc(physical_disk->unallocated_ranges,
                                        sizeof(DiskRangeKey *) * (last + 1));

    physical_disk->unallocated_ranges[last] = _new_disk_range_key();
    _memcpy_disk_range_key(physical_disk->unallocated_ranges[last], range->key);

    physical_disk->n_unallocated_ranges = last + 1;
}

void _add_allocated_physical_disk_range(PhysicalDisk *physical_disk, DiskRangeKey *range)
{
    size_t last = physical_disk->n_allocated_ranges;
    physical_disk->allocated_ranges = realloc(physical_disk->allocated_ranges,
                                      sizeof(DiskRangeKey *) * (last + 1));
    physical_disk->allocated_ranges[last] = range;
    physical_disk->n_allocated_ranges = last + 1;
}

void _initialize_disk_range_size(DiskRangeSize *size, uint64_t sector_count, int disk_index) {
    size->sector_count = sector_count;
    size->sector_start = disk_index * sector_count;
    size->sector_end = ((disk_index + 1) * sector_count) - 1;
}

void _generate_and_set_disk_range_key(DiskRangeKey *key, ProtobufCBinaryData parent_disk_key) {
    _memcpy_protobufcbinary(&key->disk_key, &parent_disk_key);
    _generate_and_set_uuid(&key->range_key);
}

PhysicalDiskRange *_initialize_next_physical_disk_range(HyperwarpMetadata *metadata,
        PhysicalDisk *disk,
        int disk_index)
{
    PhysicalDiskRange *range = _new_physical_disk_range();
    _generate_and_set_disk_range_key(range->key, disk->key);
    _initialize_disk_range_size(range->size, metadata->allocator->physical_disk_range_sector_count, disk_index);
    _add_physical_disk_range(metadata->metadata, range);
    _add_unallocated_physical_disk_range(disk, range);

    return range;
}

PhysicalDisk *create_physical_disk(HyperwarpMetadata *metadata, NVMfTransport *transport, uint64_t sector_count, uint64_t sector_size)
{
    // allocator sector size incompatible with physical disk sector size
    if (sector_size != metadata->allocator->sector_size)
    {
        return NULL;
    }

    PhysicalDisk *physical_disk = (PhysicalDisk *)malloc(sizeof(PhysicalDisk));
    physical_disk__init(physical_disk);
    _generate_and_set_uuid(&physical_disk->key);
    physical_disk->sector_count = sector_count;
    physical_disk->sector_size = sector_size;
    physical_disk->size = sector_count * sector_size;
    physical_disk->transport = transport;

    size_t n_ranges = ceil(sector_count / metadata->allocator->physical_disk_range_sector_count);
    for (int i = 0; i < n_ranges; i++)
    {
        _initialize_next_physical_disk_range(metadata, physical_disk, i);
    }

    _add_physical_disk(metadata->metadata, physical_disk);
    _sort_physical_disks_by_unallocated_ranges(metadata->metadata);

    return physical_disk;
}

static VirtualDisk *_new_virtual_disk(char *name, uint64_t size, VirtualDisk__ErasureCodeProfile ec_profile)
{
    VirtualDisk *virtual_disk = (VirtualDisk *)malloc(sizeof(VirtualDisk));
    virtual_disk__init(virtual_disk);

    _generate_and_set_uuid(&virtual_disk->key);
    virtual_disk->name = _memcpy_from_str(name);
    virtual_disk->ec_profile = ec_profile;
    virtual_disk->size = size;

    return virtual_disk;
}

VirtualDiskRange *_new_virtual_disk_range()
{
    VirtualDiskRange *range = (VirtualDiskRange *)malloc(sizeof(VirtualDiskRange));
    virtual_disk_range__init(range);

    range->key = malloc(sizeof(DiskRangeKey));
    disk_range_key__init(range->key);

    range->size = malloc(sizeof(DiskRangeSize));
    disk_range_size__init(range->size);

    return range;
}

VirtualDiskRange *_create_virtual_disk_range(Allocator *allocator, ProtobufCBinaryData disk_key, int disk_size)
{
    VirtualDiskRange *vd_range = _new_virtual_disk_range();

    _generate_and_set_disk_range_key(vd_range->key, disk_key);
    _initialize_disk_range_size(vd_range->size, allocator->virtual_disk_range_sector_count, disk_size);

    return vd_range;
}

void _add_physical_disk_range_to_virtual_disk_range(VirtualDiskRange *vdr, DiskRangeKey *pdr)
{
    size_t last = vdr->n_ranges;
    vdr->ranges = realloc(vdr->ranges, sizeof(DiskRangeKey *) * (last + 1));
    vdr->ranges[last] = _new_disk_range_key();
    _memcpy_disk_range_key(vdr->ranges[last], pdr);

    vdr->n_ranges = last + 1;
}

DiskRangeKey* _allocate_next_physical_disk_range(PhysicalDisk *physical_disk) {
    DiskRangeKey* range = physical_disk->unallocated_ranges[0];
    _add_allocated_physical_disk_range(physical_disk, range);
    for (int i = 0; i < physical_disk->n_unallocated_ranges - 1; i++) {
        physical_disk->unallocated_ranges[i] = physical_disk->unallocated_ranges[i+1];
    }

    physical_disk->n_unallocated_ranges--;
    if (physical_disk->n_unallocated_ranges >= 0) {
        physical_disk->unallocated_ranges = realloc(physical_disk->unallocated_ranges, sizeof(DiskRangeKey *) * physical_disk->n_unallocated_ranges);
    }

    return range;
}

void _add_virtual_disk_range_to_virtual_disk(VirtualDisk *virtual_disk, VirtualDiskRange *range)
{
    size_t last = virtual_disk->n_ranges;
    virtual_disk->ranges = realloc(virtual_disk->ranges,
                                   sizeof(VirtualDiskRange *) * (last + 1));
    virtual_disk->ranges[last] = range;
    virtual_disk->n_ranges = last + 1;
}

VirtualDisk *create_virtual_disk(HyperwarpMetadata *hwmetadata, char *name, uint64_t size)
{
    size_t n_physical_ranges = (hwmetadata->allocator->n + hwmetadata->allocator->p);

    Metadata *metadata = hwmetadata->metadata;

    // abort if there are less than n+p physical disks registered in the metadata
    if (metadata->n_physical_disks < n_physical_ranges) {
        return NULL;
    }

    // abort if the requested virtual disk size can not be evenly divided by the number of data chunks of the selected allocator
    if ((size * hwmetadata->allocator->virtual_disk_range_sector_count) % hwmetadata->allocator->n != 0)
    {
        return NULL;
    }

    /*
     * so let's say you need a virtual disk that is 100GiB
     * we're going to cut that into 100 x 1GB VirtualDiskRanges
     * then each VirtualDiskRange is going to map to 6xPhysicalDiskRanges
     * (for 4+2 EC)
     */
    VirtualDisk *virtual_disk = _new_virtual_disk(name, size, metadata->ec_profile);

    int rc = 0;

    for (int i = 0; i < size; i++)
    {
        VirtualDiskRange *vdr = _create_virtual_disk_range(hwmetadata->allocator, virtual_disk->key, i);
        for (int j = 0; j < n_physical_ranges; j++)
        {
            PhysicalDisk *physical_disk = metadata->physical_disks[j];
            // abort if the current physical disk has no unallocated ranges since there's not enough physical space for the virtual disk then
            if (physical_disk->n_unallocated_ranges <= 0) {
                rc = 1;
                break;
            }

            DiskRangeKey *pdr = _allocate_next_physical_disk_range(physical_disk);
            _add_physical_disk_range_to_virtual_disk_range(vdr, pdr);
        }

        _add_virtual_disk_range_to_virtual_disk(virtual_disk, vdr);
        _sort_physical_disks_by_unallocated_ranges(metadata);

        if (rc > 0) {
            break;
        }
    }

    if (rc < 0) {
        return NULL;
    }

    if (metadata != NULL)
    {
        _add_virtual_disk(metadata, virtual_disk);
    }

    return virtual_disk;
}

VirtualDiskRange **translate_vdaddress_to_vdranges(HyperwarpMetadata *metadata, VirtualDisk *vdisk, uint64_t start_address, uint64_t io_size)
{
    uint64_t end_address = start_address + io_size - 1;

    size_t start_index = floor(start_address / metadata->allocator->virtual_disk_range_sector_count);
    size_t end_index = ceil(end_address / metadata->allocator->virtual_disk_range_sector_count);

    size_t range_count = (end_index - start_index) + 1;

    VirtualDiskRange **result = malloc(range_count * sizeof(VirtualDiskRange));
    for (int i = start_index; i <= end_index; i++)
    {
        result[i] = vdisk->ranges[i];
    }

    return result;
}
