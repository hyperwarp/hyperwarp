#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>

#include "util.h"

char *_get_key_as_string(ProtobufCBinaryData *key) {
    char *key_str = malloc(sizeof(char) * UUID_STR_LEN);
    uuid_unparse(key->data, key_str);
    return key_str;
}

void print_uuid(ProtobufCBinaryData *key, int indent) {
    char *disk_key = _get_key_as_string(key);
    printf("%*sKey = %s\n", indent, "", disk_key);
    free(disk_key);
}

void print_disk_range_key(DiskRangeKey *key, int indent) {
    char *disk_key = _get_key_as_string(&key->disk_key);
    char *range_key = _get_key_as_string(&key->range_key);
    printf("%*s%s / %s\n", indent + 4, "", disk_key, range_key);
    free(disk_key);
    free(range_key);
}

void print_with_indent(int indent, char *string)
{
    printf("%*s%s", indent, "", string);
}

void print_nvmf_transport(NVMfTransport *transport, int indent) {
    printf("%*sNQN = %s\n", (indent + 2), "", transport->subsystem_nqn);
}

void print_physical_disk(PhysicalDisk *physical_disk, int indent)
{
    printf("%*sPhysicalDisk\n", indent, "");
    print_uuid(&physical_disk->key, indent+2);
    printf("%*sSector Count = %" PRIu64 "\n", (indent + 2), "", physical_disk->sector_count);
    printf("%*sSector Size = %" PRIu64 "\n", (indent + 2), "", physical_disk->sector_size);
    print_nvmf_transport(physical_disk->transport, (indent));
    printf("%*sDisk ranges unallocated = %" PRIu64 "\n", (indent + 2), "", physical_disk->n_unallocated_ranges);
    printf("%*sDisk ranges allocated = %" PRIu64 "\n", (indent + 2), "", physical_disk->n_allocated_ranges);

    printf("%*sPhysicalDiskRanges\n", indent, "");
    if (physical_disk->n_unallocated_ranges && physical_disk->n_unallocated_ranges > 0) {
        printf("%*sunallocated\n", indent + 2, "");
        for (int i = 0; i < physical_disk->n_unallocated_ranges; i++) {
            print_disk_range_key(physical_disk->unallocated_ranges[i], indent + 4);
        }
    }

    if (physical_disk->n_allocated_ranges && physical_disk->n_allocated_ranges > 0) {
        printf("%*sallocated\n", indent + 2, "");
        for (int i = 0; i < physical_disk->n_allocated_ranges; i++) {
            print_disk_range_key(physical_disk->allocated_ranges[i], indent + 4);
        }
    }
}

void print_virtual_disk(VirtualDisk *virtual_disk, int indent)
{
    printf("%*sVirtualDisk\n", indent, "");
    print_uuid(&virtual_disk->key, indent+2);
    printf("%*sName = %s\n", (indent + 2), "", virtual_disk->name);
    printf("%*sSize = %" PRIu64 " GB\n", (indent + 2), "", virtual_disk->size);
    if (virtual_disk->ec_profile >= 0)
    {
        printf("%*sErasure Code Profile = %s\n", (indent + 2), "", ((ProtobufCEnumDescriptor *)virtual_disk->base.descriptor->fields[2].descriptor)->values[virtual_disk->ec_profile].name);
    }

    if (virtual_disk->n_ranges && virtual_disk->n_ranges > 0) {
        printf("%*sVirtualDiskRanges\n", indent + 2, "");
        for (int i = 0; i < virtual_disk->n_ranges; i++) {
            print_disk_range_key(virtual_disk->ranges[i]->key, indent + 4);
            if (virtual_disk->ranges[i]->n_ranges && virtual_disk->ranges[i]->n_ranges > 0) {
                printf("%*sPhysicalDiskRanges\n", indent + 6, "");
                for (int j = 0; j < virtual_disk->ranges[i]->n_ranges; j++) {
                    print_disk_range_key(virtual_disk->ranges[i]->ranges[j], indent + 4);
                }
            }
        }
    }
}

void print_metadata(Metadata *metadata, int indent)
{
    printf("Printing MetaData\n");

    if (metadata->n_physical_disks && metadata->n_physical_disks > 0)
    {
        printf("  Printing %" PRIu64 " PhysicalDisks\n", metadata->n_physical_disks);
        for (int i = 0; i < metadata->n_physical_disks; i++)
        {
            print_physical_disk(metadata->physical_disks[i], indent + 4);
        }
        printf("  PhysicalDisks END\n");
    }

    if (metadata->n_virtual_disks && metadata->n_virtual_disks > 0)
    {
        printf("  Printing %" PRIu64 " VirtualDisks\n", metadata->n_virtual_disks);
        for (int i = 0; i < metadata->n_virtual_disks; i++)
        {
            print_virtual_disk(metadata->virtual_disks[i], indent + 4);
        }
        printf("  VirtualDisks END\n");
    }

    printf("END\n");
}