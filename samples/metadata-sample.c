/* SPDX-License-Identifier: Apache-2.0 */

#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


#include "util.h"
#include "metadata.h"

int main()
{
    int ret = 0;

    HyperwarpMetadata *metadata = new_metadata("foundationdb", VIRTUAL_DISK__ERASURE_CODE_PROFILE__EC_4_2P);

    if (metadata == NULL) {
        fprintf(stderr, "Creation of metadata failed, exiting\n");
        exit(1);
    }

    NVMfTransport__NVMfTransportType type = NVMF_TRANSPORT__NVMF_TRANSPORT_TYPE__TRTYPE_TCP;
    NVMfTransport__NVMfAddressFamily address_family = NVMF_TRANSPORT__NVMF_ADDRESS_FAMILY__ADRFAM_IPV4;
    char *address = "127.0.0.1";

    NVMfTransport *transport1 = create_nvmf_transport(type, address_family, address, "4420", "nqn.2020-11.com.github.hyperwarp:cnode1");
    PhysicalDisk *physical_disk1 = create_physical_disk(metadata, transport1, 266144ULL, 4096ULL);


    NVMfTransport *transport2 = create_nvmf_transport(type, address_family, address, "4421", "nqn.2020-11.com.github.hyperwarp:cnode2");
    PhysicalDisk *physical_disk2 = create_physical_disk(metadata, transport2, 266144ULL, 4096ULL);

    NVMfTransport *transport3 = create_nvmf_transport(type, address_family, address, "4422", "nqn.2020-11.com.github.hyperwarp:cnode3");
    PhysicalDisk *physical_disk3 = create_physical_disk(metadata, transport3, 266144ULL, 4096ULL);

    NVMfTransport *transport4 = create_nvmf_transport(type, address_family, address, "4423", "nqn.2020-11.com.github.hyperwarp:cnode4");
    PhysicalDisk *physical_disk4 = create_physical_disk(metadata, transport4, 266144ULL, 4096ULL);

    NVMfTransport *transport5 = create_nvmf_transport(type, address_family, address, "4424", "nqn.2020-11.com.github.hyperwarp:cnode5");
    PhysicalDisk *physical_disk5 = create_physical_disk(metadata, transport5, 266144ULL, 4096ULL);

    NVMfTransport *transport6 = create_nvmf_transport(type, address_family, address, "4425", "nqn.2020-11.com.github.hyperwarp:cnode6");
    PhysicalDisk *physical_disk6 = create_physical_disk(metadata, transport6, 266144ULL, 4096ULL);

    NVMfTransport *transport7 = create_nvmf_transport(type, address_family, address, "4426", "nqn.2020-11.com.github.hyperwarp:cnode7");
    PhysicalDisk *physical_disk7 = create_physical_disk(metadata, transport7, 266144ULL, 4096ULL);

    NVMfTransport *transport8 = create_nvmf_transport(type, address_family, address, "4427", "nqn.2020-11.com.github.hyperwarp:cnode8");
    PhysicalDisk *physical_disk8 = create_physical_disk(metadata, transport8, 266144ULL, 4096ULL);

    NVMfTransport *transport9 = create_nvmf_transport(type, address_family, address, "4428", "nqn.2020-11.com.github.hyperwarp:cnode9");
    PhysicalDisk *physical_disk9 = create_physical_disk(metadata, transport9, 266144ULL, 4096ULL);

    NVMfTransport *transport10 = create_nvmf_transport(type, address_family, address, "4429", "nqn.2020-11.com.github.hyperwarp:cnode10");
    PhysicalDisk *physical_disk10 = create_physical_disk(metadata, transport10, 266144ULL, 4096ULL);

    VirtualDisk *virtual_disk1 = create_virtual_disk(metadata, "vdisk1", 3ULL);
    VirtualDisk *virtual_disk2 = create_virtual_disk(metadata, "vdisk2", 3ULL);

    print_metadata(metadata->metadata, 0);
    ret = metadata_persist(metadata);
    if (ret == 0) {
        printf("wrote metadata to foundationdb!");
    } else {
        printf("could not write metadata to foundationdb! :(");
        exit(ret);
    }
    metadata_close(metadata);

    metadata = load_metadata("foundationdb");

    if (metadata == NULL) {
        fprintf(stderr, "Could not load the metadata from the foundationdb database\n");
        exit(1);
    }

    print_metadata(metadata->metadata, 0);

    metadata_close(metadata);
    return ret;
}
