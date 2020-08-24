#define FDB_API_VERSION 620
#include <foundationdb/fdb_c.h>

#include <metadata-foundationdb.h>

void chk(fdb_error_t err)
{
	if (err)
	{
		fprintf(stderr, "Error: %s\n", fdb_get_error(err));
		abort();
	}
}

/**
 * 
 * 
 * \param database
 * \param physical_disk
 */
void physical_disk_persist(FDBDatabase *database, const PhysicalDisk *physical_disk)
{
	void *buffer;
	unsigned length;

	length = physical_disk__get_packed_size(physical_disk);
	buffer = malloc(length);
	physical_disk__pack(physical_disk, buffer);

	FDBTransaction *transaction;
	FDBFuture *future;

	chk(fdb_database_create_transaction(database, &transaction));

	fdb_transaction_set(transaction, (uint8_t *)&physical_disk->key, sizeof(physical_disk->key), buffer, length);

	future = fdb_transaction_commit(transaction);
	chk(fdb_future_block_until_ready(future));

	fdb_future_destroy(future);
	fdb_transaction_destroy(transaction);

	free(buffer);	
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
	PhysicalDisk *physical_disk = NULL;
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
		physical_disk = physical_disk__unpack(NULL, length, buffer);
	}

	fdb_future_destroy(future);

	future = fdb_transaction_commit(transaction);
	chk(fdb_future_block_until_ready(future));
	fdb_future_destroy(future);

	fdb_transaction_destroy(transaction);

	return physical_disk;
}