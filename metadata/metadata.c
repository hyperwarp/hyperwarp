#include <metadata.h>

PhysicalDisk create_physical_disk(uint64_t sector_count, uint64_t sector_size)
{
        PhysicalDisk physical_disk = PHYSICAL_DISK__INIT;
        physical_disk.key = 111444890ULL;
        physical_disk.sector_count = sector_count;
        physical_disk.sector_size = sector_size;

	return physical_disk;
}
