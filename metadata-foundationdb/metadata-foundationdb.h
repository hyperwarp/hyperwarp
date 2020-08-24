#include <stdio.h>
#include <stdlib.h>

#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include <metadata.pb-c.h>

void chk(fdb_error_t err);

/**
 * 
 * 
 * \param database
 * \param physical_disk
 */
void physical_disk_persist(FDBDatabase *database, const PhysicalDisk *physical_disk);

/**
 * Fetches a PhysicalDisk from a given FoundationDB instance by key.
 * 
 * \param database the FoundationDB instance to fetch the PhysicalDisk from
 * \param key the key of the PhysicalDisk to fetch from FoundationDB
 * \return the PhysicalDisk stored in FoundationDB under the given key
 * \retval NULL if the PhysicalDisk could not be found or if there was an error unpacking
 */
PhysicalDisk *physical_disk_get(FDBDatabase *database, const uint64_t key);