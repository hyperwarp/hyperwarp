/* SPDX-License-Identifier: Apache-2.0 */

#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include <metadata-foundationdb.h>

const uint64_t meta_data_key = 0ULL;

void chk(fdb_error_t err)
{
	if (err)
	{
		fprintf(stderr, "Error: %s\n", fdb_get_error(err));
		abort();
	}
}

/**
 * Persists a ProtobufCMessage with the given key in FoundationDB.
 * 
 * \param database the FoundationDB instance to persist the ProtobufCMessage in
 * \param key the key to store the ProtobufCMessage with in FoundationDB
 * \param message the ProtobufCMessage to persist in FoundationDB
 */
void proto_message_persist(FDBDatabase *database, uint64_t key, const ProtobufCMessage *message)
{
	void *buffer;
	unsigned length;

	length = protobuf_c_message_get_packed_size(message);
	buffer = malloc(length);
	protobuf_c_message_pack(message, buffer);

	FDBTransaction *transaction;
	FDBFuture *future;

	chk(fdb_database_create_transaction(database, &transaction));

	fdb_transaction_set(transaction, (uint8_t *)&key, sizeof(key), buffer, length);

	future = fdb_transaction_commit(transaction);
	chk(fdb_future_block_until_ready(future));

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
ProtobufCMessage *proto_message_get(FDBDatabase *database, const uint64_t key, const ProtobufCMessageDescriptor *descriptor)
{
	ProtobufCMessage *message = NULL;
	FDBTransaction *transaction;
	FDBFuture *future;

	chk(fdb_database_create_transaction(database, &transaction));
	future = fdb_transaction_get(transaction, (uint8_t *)&key, sizeof(key), 0);
	chk(fdb_future_block_until_ready(future));

	fdb_bool_t exists;
	const uint8_t *buffer;
	int length;

	chk(fdb_future_get_value(future, &exists, &buffer, &length));

	if (exists)
	{
		message = protobuf_c_message_unpack(descriptor, NULL, length, buffer);
	}

	fdb_future_destroy(future);

	future = fdb_transaction_commit(transaction);
	chk(fdb_future_block_until_ready(future));
	fdb_future_destroy(future);

	fdb_transaction_destroy(transaction);

	return message;
}

/**
 * Persists MetaData in FoundationDB.
 * 
 * \param database the FoundationDB instance to persist the MetaData in
 * \param physical_disk the MetaData to persist in FoundationDB
 */
void meta_data_persist(FDBDatabase *database, const MetaData *meta_data)
{
	assert(meta_data->base.descriptor == &meta_data__descriptor);
	proto_message_persist(database, meta_data_key, (const ProtobufCMessage*)(meta_data));
}

/**
 * Fetches MetaData from a given FoundationDB instance.
 * 
 * \param database the FoundationDB instance to fetch the MetaData from
 * \return the MetaData stored in FoundationDB
 * \retval NULL if the MetaData could not be found or if there was an error unpacking
 */
MetaData *meta_data_get(FDBDatabase *database)
{
	return (MetaData*) proto_message_get(database, meta_data_key, &meta_data__descriptor);
}

/**
 * Persists a PhysicalDisk in FoundationDB.
 * 
 * \param database the FoundationDB instance to persist the PhysicalDisk in
 * \param physical_disk the PhysicalDisk to persist in FoundationDB
 */
void physical_disk_persist(FDBDatabase *database, const PhysicalDisk *physical_disk)
{
	assert(physical_disk->base.descriptor == &physical_disk__descriptor);
	proto_message_persist(database, physical_disk->key, (const ProtobufCMessage*)(physical_disk));
}

/**
 * Fetches a PhysicalDisk from a given FoundationDB instance by key.
 * 
 * \param database the FoundationDB instance to fetch the PhysicalDisk from
 * \param key the key of the PhysicalDisk to fetch from FoundationDB
 * \return the PhysicalDisk stored in FoundationDB under the given key
 * \retval NULL if the PhysicalDisk could not be found or if there was an error unpacking
 */
PhysicalDisk *physical_disk_get(FDBDatabase *database, const uint64_t key)
{
	return (PhysicalDisk*) proto_message_get(database, key, &physical_disk__descriptor);
}

/**
 * Persists a VirtualDisk in FoundationDB.
 * 
 * \param database the FoundationDB instance to persist the VirtualDisk in
 * \param virtual_disk the VirtualDisk to persist in FoundationDB
 */
void virtual_disk_persist(FDBDatabase *database, const VirtualDisk *virtual_disk)
{
	assert(virtual_disk->base.descriptor == &virtual_disk__descriptor);
	proto_message_persist(database, virtual_disk->key, (const ProtobufCMessage*)(virtual_disk));
}

/**
 * Fetches a VirtualDisk from a given FoundationDB instance by key.
 * 
 * \param database the FoundationDB instance to fetch the VirtualDisk from
 * \param key the key of the VirtualDisk to fetch from FoundationDB
 * \return the VirtualDisk stored in FoundationDB under the given key
 * \retval NULL if the VirtualDisk could not be found or if there was an error unpacking
 */
VirtualDisk *virtual_disk_get(FDBDatabase *database, const uint64_t key)
{
	return (VirtualDisk*) proto_message_get(database, key, &virtual_disk__descriptor);
}