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
#include <util.h>

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
	print_physical_disk(physical_disk1, 0);

	PhysicalDisk *physical_disk2 = physical_disk_get(database, physical_disk1->key);

	printf("Read PhysicalDisk from FDB\n");
	print_physical_disk(physical_disk2, 0);

	physical_disk__free_unpacked(physical_disk1, NULL);
	physical_disk__free_unpacked(physical_disk2, NULL);

	fdb_database_destroy(database);

	chk(fdb_stop_network());
	pthread_join(net_thread, NULL);

	printf("Tada!\n");

	return 0;
}
