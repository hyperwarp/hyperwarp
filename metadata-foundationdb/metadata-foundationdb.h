/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <stdlib.h>

#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include "metadata.pb-c.h"

void chk(fdb_error_t err);

/**
 * Persists MetaData in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the MetaData in
 * \param physical_disk the MetaData to persist in FoundationDB
 */
void meta_data_persist(FDBDatabase *database, const MetaData *meta_data);

/**
 * Fetches MetaData from a given FoundationDB instance.
 *
 * \param database the FoundationDB instance to fetch the MetaData from
 * \return the MetaData stored in FoundationDB
 * \retval NULL if the MetaData could not be found or if there was an error unpacking
 */
MetaData *meta_data_get(FDBDatabase *database);

/**
 * Persists a PhysicalDisk in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the PhysicalDisk in
 * \param physical_disk the PhysicalDisk to persist in FoundationDB
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

/**
 * Persists a VirtualDisk in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the VirtualDisk in
 * \param virtual_disk the VirtualDisk to persist in FoundationDB
 */
void virtual_disk_persist(FDBDatabase *database, const VirtualDisk *virtual_disk);

/**
 * Fetches a VirtualDisk from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the VirtualDisk from
 * \param key the key of the VirtualDisk to fetch from FoundationDB
 * \return the VirtualDisk stored in FoundationDB under the given key
 * \retval NULL if the VirtualDisk could not be found or if there was an error unpacking
 */
VirtualDisk *virtual_disk_get(FDBDatabase *database, const uint64_t key);