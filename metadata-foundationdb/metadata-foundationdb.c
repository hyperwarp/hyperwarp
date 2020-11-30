/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <stdlib.h>

#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include "metadata.pb-c.h"

#include <pthread.h>
#include <metadata.h>
#include <stdbool.h>

static uint64_t metadata_key = 0ULL;
static FDBDatabase *foundationdb_database = NULL;
static pthread_t net_thread;

static int chk(fdb_error_t err)
{
    if (err)
    {
        fprintf(stderr, "Error: %s\n", fdb_get_error(err));
        return -1;
    }
    return 0;
}

/**
 * Persists a ProtobufCMessage with the given key in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the ProtobufCMessage in
 * \param key the key to store the ProtobufCMessage with in FoundationDB
 * \param message the ProtobufCMessage to persist in FoundationDB
 */
static int proto_message_persist(FDBDatabase *database, const uint8_t *key, int key_len, const ProtobufCMessage *message)
{
    void *buffer;
    unsigned length;
    int ret = 0;

    length = protobuf_c_message_get_packed_size(message);
    buffer = malloc(length);
    protobuf_c_message_pack(message, buffer);

    FDBTransaction *transaction;
    FDBFuture *future;

    if ((ret = chk(fdb_database_create_transaction(database, &transaction))) != 0) {
        return ret;
    }

    fdb_transaction_set(transaction, key, key_len, buffer, length);

    future = fdb_transaction_commit(transaction);
    if ((ret = chk(fdb_future_block_until_ready(future)) != 0)) {
        return ret;
    }

    fdb_future_destroy(future);
    fdb_transaction_destroy(transaction);

    free(buffer);
}

/**
 * Fetches a ProtobufCMessage from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the ProtobufCMessage from
 * \param key the key of the ProtobufCMessage to fetch from FoundationDB
 * \param descriptor the ProtobufCMessageDescriptor to use to unpack the message
 * \return the ProtobufCMessage stored in FoundationDB under the given key
 * \retval NULL if the ProtobufCMessage could not be found or if there was an error unpacking
 */
static ProtobufCMessage *proto_message_get(FDBDatabase *database, const uint8_t *key, int key_len, const ProtobufCMessageDescriptor *descriptor)
{
    ProtobufCMessage *message = NULL;
    FDBTransaction *transaction;
    FDBFuture *future;

    if (chk(fdb_database_create_transaction(database, &transaction)) != 0) {
        return NULL;
    }
    future = fdb_transaction_get(transaction, key, key_len, 0);
    if (chk(fdb_future_block_until_ready(future)) != 0) {
        return NULL;
    }

    fdb_bool_t exists;
    const uint8_t *buffer;
    int length;

    if (chk(fdb_future_get_value(future, &exists, &buffer, &length)) != 0) {
        return NULL;
    }

    if (exists)
    {
        message = protobuf_c_message_unpack(descriptor, NULL, length, buffer);
    }

    fdb_future_destroy(future);

    future = fdb_transaction_commit(transaction);
    if (chk(fdb_future_block_until_ready(future)) != 0) {
        return NULL;
    }
    fdb_future_destroy(future);

    fdb_transaction_destroy(transaction);

    return message;
}

/**
 * Persists MetaData in FoundationDB.
 */
static int persist(Metadata *metadata)
{
    assert(metadata->base.descriptor == &metadata__descriptor);
    return proto_message_persist(foundationdb_database, (uint8_t *)&metadata_key, sizeof(uint64_t), (const ProtobufCMessage *)(metadata));
}

/**
 * Fetches MetaData from a given FoundationDB instance.
 *
 * \param database the FoundationDB instance to fetch the MetaData from
 * \return the Metadata stored in FoundationDB
 * \retval NULL if the MetaData could not be found or if there was an error unpacking
 */
static Metadata *load()
{
    return (Metadata *)proto_message_get(foundationdb_database, (uint8_t *)&metadata_key, sizeof(uint64_t), &metadata__descriptor);
}

/**
 * Persists a PhysicalDisk in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the PhysicalDisk in
 * \param physical_disk the PhysicalDisk to persist in FoundationDB
 */
static int physical_disk_persist(FDBDatabase *database, PhysicalDisk *physical_disk)
{
    assert(physical_disk->base.descriptor == &physical_disk__descriptor);
    return proto_message_persist(database, physical_disk->key.data, physical_disk->key.len, (const ProtobufCMessage *)(physical_disk));
}

/**
 * Fetches a PhysicalDisk from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the PhysicalDisk from
 * \param key the key of the PhysicalDisk to fetch from FoundationDB
 * \return the PhysicalDisk stored in FoundationDB under the given key
 * \retval NULL if the PhysicalDisk could not be found or if there was an error unpacking
 */
static PhysicalDisk *physical_disk_get(FDBDatabase *database, ProtobufCBinaryData *key)
{
    return (PhysicalDisk *)proto_message_get(database, key->data, key->len, &physical_disk__descriptor);
}

/**
 * Persists a PhysicalDiskRange in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the PhysicalDiskRange in
 * \param physical_disk the PhysicalDiskRange to persist in FoundationDB
 */
static int physical_disk_range_persist(FDBDatabase *database, PhysicalDiskRange *physical_disk_range)
{
    assert(physical_disk_range->base.descriptor == &physical_disk_range__descriptor);

    int length = disk_range_key__get_packed_size(physical_disk_range->key);
    uint8_t *buffer = malloc(length);
    disk_range_key__pack(physical_disk_range->key, buffer);

    return proto_message_persist(database, buffer, length, (const ProtobufCMessage *)(physical_disk_range));
}

/**
 * Fetches a PhysicalDiskRange from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the PhysicalDiskRange from
 * \param key the key of the PhysicalDiskRange to fetch from FoundationDB
 * \return the PhysicalDiskRange stored in FoundationDB under the given key
 * \retval NULL if the PhysicalDiskRange could not be found or if there was an error unpacking
 */
static PhysicalDiskRange *physical_disk_range_get(FDBDatabase *database, DiskRangeKey *key)
{
    int length = disk_range_key__get_packed_size(key);
    uint8_t *buffer = malloc(length);
    disk_range_key__pack(key, buffer);

    return (PhysicalDiskRange *)proto_message_get(database, buffer, length, &physical_disk__descriptor);
}


/**
 * Persists a VirtualDisk in FoundationDB.
 *
 * \param database the FoundationDB instance to persist the VirtualDisk in
 * \param virtual_disk the VirtualDisk to persist in FoundationDB
 */
static int virtual_disk_persist(FDBDatabase *database, VirtualDisk *virtual_disk)
{
    assert(virtual_disk->base.descriptor == &virtual_disk__descriptor);
    return proto_message_persist(database, virtual_disk->key.data, virtual_disk->key.len, (const ProtobufCMessage *)(virtual_disk));
}

/**
 * Fetches a VirtualDisk from a given FoundationDB instance by key.
 *
 * \param database the FoundationDB instance to fetch the VirtualDisk from
 * \param key the key of the VirtualDisk to fetch from FoundationDB
 * \return the VirtualDisk stored in FoundationDB under the given key
 * \retval NULL if the VirtualDisk could not be found or if there was an error unpacking
 */
static VirtualDisk *virtual_disk_get(FDBDatabase *database, ProtobufCBinaryData *key)
{
    return (VirtualDisk *)proto_message_get(database, key->data, key->len, &virtual_disk__descriptor);
}

static void *run_net(void *_unused)
{
    chk(fdb_run_network());

    return NULL;
}

static int initialize() {
    int ret = 0;

    fdb_select_api_version(FDB_API_VERSION);

    if ((ret = chk(fdb_setup_network())) != 0) {
        return ret;
    }

    if ((ret = pthread_create(&net_thread, NULL, run_net, NULL) != 0)) {
        return ret;
    }

    if ((ret = chk(fdb_create_database(NULL, &foundationdb_database))) != 0) {
        return ret;
    }

    return 0;
}

static int finalize() {
    int ret = 0;

    fdb_database_destroy(foundationdb_database);
    foundationdb_database = NULL;
    if (ret = chk(fdb_stop_network()) != 0) {
        return ret;
    }

    pthread_join(net_thread, NULL);
    return ret;
}

static MetadataBackend foundationdb_backend = {
    .name = "foundationdb",
    .initialize = initialize,
    .load = load,
    .persist = persist,
    .finalize = finalize
};

static void metadata_backend_foundationdb_init(void) {
    metadata_storage_backend_register(&foundationdb_backend, false);
}

module_init(metadata_backend_foundationdb_init);