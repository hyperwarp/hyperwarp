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

void *run_net(void *_unused)
{
	chk(fdb_run_network());

	return NULL;
}

int main()
{

	return 0;
}
