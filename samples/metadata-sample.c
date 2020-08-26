/* SPDX-License-Identifier: Apache-2.0 */

#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include <metadata.h>
#include <metadata-foundationdb.h>

void *run_net(void *_unused)
{
	chk(fdb_run_network());

	return NULL;
}

void print_physical_disk(PhysicalDisk* physical_disk) {
	printf("    PhysicalDisk\n");
	printf("      Key = %" PRIu64 "\n", physical_disk->key);
	printf("      Sector Count = %" PRIu64 "\n", physical_disk->sector_count);
	printf("      Sector Size = %" PRIu64 "\n", physical_disk->sector_size);
	printf("      Disk ranges allocated = %" PRIu64 "\n", physical_disk->n_physical_disk_ranges);
}

void print_virtual_disk(VirtualDisk* virtual_disk) {
	printf("    VirtualDisk\n");
	printf("      Key = %" PRIu64 "\n", virtual_disk->key);
	printf("      Name = %s\n", virtual_disk->name);
	if (virtual_disk->ec_profile >= 0) {
		printf("      Erasure Code Profile = %d\n", virtual_disk->ec_profile);
	}
}

void print_meta_data(MetaData* meta_data) {
	printf("Printing MetaData\n");

	if (meta_data->n_physical_disks && meta_data->n_physical_disks > 0) {
		printf("  Printing %" PRIu64 " PhysicalDisks\n", meta_data->n_physical_disks);
		for (int i = 0; i < meta_data->n_physical_disks; i++) {
			print_physical_disk(meta_data->physical_disks[i]);
		}
	}

	if (meta_data->n_virtual_disks && meta_data->n_virtual_disks > 0) {
		printf("  Printing %" PRIu64 " VirtualDisks\n", meta_data->n_virtual_disks);
		for (int i = 0; i < meta_data->n_virtual_disks; i++) {
			print_virtual_disk(meta_data->virtual_disks[i]);
		}
	}
}

int main()
{
	fdb_select_api_version(FDB_API_VERSION);

	chk(fdb_setup_network());

	pthread_t net_thread;
	assert(0 == pthread_create(&net_thread, NULL, run_net, NULL));

	FDBDatabase *database;
	chk(fdb_create_database(NULL, &database));

	MetaData *metadata = create_metadata();

	PhysicalDisk *physical_disk1 = create_physical_disk(metadata, 266144ULL, 4096ULL);
	PhysicalDiskRange *pd_range1 = create_physical_disk_range(1110ULL, physical_disk1->key, 0ULL, 266144ULL, 266144ULL);
	add_physical_disk_range_to_physical_disk(physical_disk1, pd_range1);

	PhysicalDisk *physical_disk2 = create_physical_disk(metadata, 266144ULL, 4096ULL);
	PhysicalDiskRange *pd_range2 = create_physical_disk_range(1110ULL, physical_disk2->key, 0ULL, 266144ULL, 266144ULL);
	add_physical_disk_range_to_physical_disk(physical_disk2, pd_range2);

	PhysicalDisk *physical_disk3 = create_physical_disk(metadata, 266144ULL, 4096ULL);
	PhysicalDiskRange *pd_range3 = create_physical_disk_range(1110ULL, physical_disk3->key, 0ULL, 266144ULL, 266144ULL);
	add_physical_disk_range_to_physical_disk(physical_disk3, pd_range3);

	PhysicalDisk *physical_disk4 = create_physical_disk(metadata, 266144ULL, 4096ULL);
	PhysicalDiskRange *pd_range4 = create_physical_disk_range(1110ULL, physical_disk4->key, 0ULL, 266144ULL, 266144ULL);
	add_physical_disk_range_to_physical_disk(physical_disk4, pd_range4);

	PhysicalDisk *physical_disk5 = create_physical_disk(metadata, 266144ULL, 4096ULL);
	PhysicalDiskRange *pd_range5 = create_physical_disk_range(1110ULL, physical_disk5->key, 0ULL, 266144ULL, 266144ULL);
	add_physical_disk_range_to_physical_disk(physical_disk5, pd_range5);

	PhysicalDisk *physical_disk6 = create_physical_disk(metadata, 266144ULL, 4096ULL);
	PhysicalDiskRange *pd_range61 = create_physical_disk_range(1110ULL, physical_disk6->key, 0ULL, 266143ULL, 266144ULL);
	PhysicalDiskRange *pd_range62 = create_physical_disk_range(1110ULL, physical_disk6->key, 266144ULL, 532287ULL, 266144ULL);
	add_physical_disk_range_to_physical_disk(physical_disk6, pd_range61);
	add_physical_disk_range_to_physical_disk(physical_disk6, pd_range62);

	VirtualDiskRange *vd_range1 = create_virtual_disk_range(2001ULL, 0ULL, 1064576ULL, 1064576ULL);
	vd_range1->n_ranges = 7;
	vd_range1->ranges = malloc(sizeof(PhysicalDiskRange*) * vd_range1->n_ranges);
	vd_range1->ranges[0] = pd_range1;
	vd_range1->ranges[1] = pd_range2;
	vd_range1->ranges[2] = pd_range3;
	vd_range1->ranges[3] = pd_range4;
	vd_range1->ranges[4] = pd_range5;
	vd_range1->ranges[5] = pd_range61;
	vd_range1->ranges[6] = pd_range62;

	char* virtual_disk_name = "my first virtual disk";

	VirtualDisk *virtual_disk1 = create_virtual_disk(metadata, virtual_disk_name, VIRTUAL_DISK__ERASURE_CODE_PROFILE__EC_4_2P, 1ULL);
	add_virtual_disk_range_to_virtual_disk(virtual_disk1, vd_range1);

	//physical_disk_persist(database, &meta_data1);

	print_meta_data(metadata);

	free(physical_disk1->physical_disk_ranges);
	free(physical_disk2->physical_disk_ranges);
	free(physical_disk3->physical_disk_ranges);
	free(physical_disk4->physical_disk_ranges);
	free(physical_disk5->physical_disk_ranges);
	free(physical_disk6->physical_disk_ranges);

	free(vd_range1->ranges);
	free(virtual_disk1->volume_ranges);

	free(metadata->physical_disks);
	free(metadata->virtual_disks);

/*
	printf("Wrote MetaData to FDB\n");
	printf("  Key = %" PRIu64 "\n", physical_disk1.key);
	printf("  Sector Count = %" PRIu64 "\n", physical_disk1.sector_count);
	printf("  Sector Size = %" PRIu64 "\n", physical_disk1.sector_size);

	PhysicalDisk *physical_disk2 = physical_disk_get(database, physical_disk1.key);

	printf("Read PhysicalDisk from FDB\n");
	printf("  Key = %" PRIu64 "\n", physical_disk2->key);
	printf("  Sector Count = %" PRIu64 "\n", physical_disk2->sector_count);
	printf("  Sector Size = %" PRIu64 "\n", physical_disk2->sector_size);

	physical_disk__free_unpacked(physical_disk2, NULL);
*/
	fdb_database_destroy(database);

	chk(fdb_stop_network());
	pthread_join(net_thread, NULL);

	printf("Tada!\n");

	return 0;
}
