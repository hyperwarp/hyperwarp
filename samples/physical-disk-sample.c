/* SPDX-License-Identifier: Apache-2.0 */

#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include <metadata-foundationdb.h>
#include <metadata.h>

void *run_net(void *_unused)
{
	chk(fdb_run_network());

	return NULL;
}

int main()
{
	fdb_select_api_version(FDB_API_VERSION);

	chk(fdb_setup_network());

	pthread_t net_thread;
	assert(0 == pthread_create(&net_thread, NULL, run_net, NULL));

	FDBDatabase *database;
	chk(fdb_create_database(NULL, &database));

	PhysicalDisk *physical_disk1 = create_physical_disk(NULL, 266144ULL, 4096ULL);
	physical_disk_persist(database, physical_disk1);

	printf("Wrote PhysicalDisk to FDB\n");
	printf("  Key = %" PRIu64 "\n", physical_disk1->key);
	printf("  Sector Count = %" PRIu64 "\n", physical_disk1->sector_count);
	printf("  Sector Size = %" PRIu64 "\n", physical_disk1->sector_size);

	PhysicalDisk *physical_disk2 = physical_disk_get(database, physical_disk1->key);

	printf("Read PhysicalDisk from FDB\n");
	printf("  Key = %" PRIu64 "\n", physical_disk2->key);
	printf("  Sector Count = %" PRIu64 "\n", physical_disk2->sector_count);
	printf("  Sector Size = %" PRIu64 "\n", physical_disk2->sector_size);

	physical_disk__free_unpacked(physical_disk1, NULL);
	physical_disk__free_unpacked(physical_disk2, NULL);

	fdb_database_destroy(database);

	chk(fdb_stop_network());
	pthread_join(net_thread, NULL);

	printf("Tada!\n");

	return 0;
}
