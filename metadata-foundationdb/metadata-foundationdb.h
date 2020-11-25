/* SPDX-License-Identifier: Apache-2.0 */

#ifndef HYPERWARP_METADATA_FDB_INCLUDED
#define HYPERWARP_METADATA_FDB_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include "metadata.pb-c.h"

void chk(fdb_error_t err);

/**
 * Persists MetaData in FoundationDB.
 */
void metadata_persist(Metadata *metadata);

/**
 * Fetches MetaData from a given FoundationDB instance.
 *
 * \param database the FoundationDB instance to fetch the MetaData from
 * \return the MetaData stored in FoundationDB
 * \retval NULL if the MetaData could not be found or if there was an error unpacking
 */
Metadata *metadata_get();

/**
 * Persists a PhysicalDiskRange in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the PhysicalDiskRange in
 * \param physical_disk the PhysicalDiskRange to persist in FoundationDB
 */
void physical_range_disk_persist(FDBDatabase *database, PhysicalDiskRange *physical_disk_range);

/**
 * Fetches a PhysicalDiskRange from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the PhysicalDiskRange from
 * \param key the key of the PhysicalDiskRange to fetch from FoundationDB
 * \return the PhysicalDiskRange stored in FoundationDB under the given key
 * \retval NULL if the PhysicalDiskRange could not be found or if there was an error unpacking
 */
PhysicalDiskRange *physical_disk_range_get(FDBDatabase *database, DiskRangeKey *key);

/**
 * Persists a PhysicalDisk in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the PhysicalDisk in
 * \param physical_disk the PhysicalDisk to persist in FoundationDB
 */
void physical_disk_persist(FDBDatabase *database, PhysicalDisk *physical_disk);

/**
 * Fetches a PhysicalDisk from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the PhysicalDisk from
 * \param key the key of the PhysicalDisk to fetch from FoundationDB
 * \return the PhysicalDisk stored in FoundationDB under the given key
 * \retval NULL if the PhysicalDisk could not be found or if there was an error unpacking
 */
PhysicalDisk *physical_disk_get(FDBDatabase *database, ProtobufCBinaryData *key);

/**
 * Persists a VirtualDisk in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the VirtualDisk in
 * \param virtual_disk the VirtualDisk to persist in FoundationDB
 */
void virtual_disk_persist(FDBDatabase *database, VirtualDisk *virtual_disk);

/**
 * Fetches a VirtualDisk from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the VirtualDisk from
 * \param key the key of the VirtualDisk to fetch from FoundationDB
 * \return the VirtualDisk stored in FoundationDB under the given key
 * \retval NULL if the VirtualDisk could not be found or if there was an error unpacking
 */
VirtualDisk *virtual_disk_get(FDBDatabase *database, ProtobufCBinaryData *key);

#endif