#include <inttypes.h>
#include <stdio.h>
#include <util.h>

void print_with_indent(int indent, char *string)
{
	printf("%*s%s", indent, "", string);
}

void print_physical_disk(PhysicalDisk *physical_disk, int indent)
{
	printf("%*sPhysicalDisk\n", indent, "");
	printf("%*sKey = %" PRIu64 "\n", (indent + 2), "", physical_disk->key);
	printf("%*sSector Count = %" PRIu64 "\n", (indent + 2), "", physical_disk->sector_count);
	printf("%*sSector Size = %" PRIu64 "\n", (indent + 2), "", physical_disk->sector_size);
	printf("%*sDisk ranges allocated = %" PRIu64 "\n", (indent + 2), "", physical_disk->n_ranges);
}

void print_virtual_disk(VirtualDisk *virtual_disk, int indent)
{
	printf("%*sVirtualDisk\n", indent, "");
	printf("%*sKey = %" PRIu64 "\n", (indent + 2), "", virtual_disk->key);
	printf("%*sName = %s\n", (indent + 2), "", virtual_disk->name);
	if (virtual_disk->ec_profile >= 0)
	{
		printf("%*sErasure Code Profile = %d\n", (indent + 2), "", virtual_disk->ec_profile);
	}
}

void print_meta_data(MetaData *meta_data, int indent)
{
	printf("Printing MetaData\n");

	if (meta_data->n_physical_disks && meta_data->n_physical_disks > 0)
	{
		printf("  Printing %" PRIu64 " PhysicalDisks\n", meta_data->n_physical_disks);
		for (int i = 0; i < meta_data->n_physical_disks; i++)
		{
			print_physical_disk(meta_data->physical_disks[i], indent + 4);
		}
	}

	if (meta_data->n_virtual_disks && meta_data->n_virtual_disks > 0)
	{
		printf("  Printing %" PRIu64 " VirtualDisks\n", meta_data->n_virtual_disks);
		for (int i = 0; i < meta_data->n_virtual_disks; i++)
		{
			print_virtual_disk(meta_data->virtual_disks[i], indent + 4);
		}
	}
}