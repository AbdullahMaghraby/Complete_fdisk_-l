#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

typedef struct {
    uint8_t status;
    uint8_t first_CHS[3];
    uint8_t partitionType;
    uint8_t last_CHS[3];
    uint32_t LBA;
    uint32_t sectorCount;
} partitionEntry;

int main(int argc, char **argv)
{
    char buf[512];		/*512: is sector size */
    /*chech on number of arguments */
    if (argc > 2) {
	printf("Error at number of inputs\n");
	return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    read(fd, buf, 512);

    partitionEntry *prtEntry_Ptr = (partitionEntry *) & buf[446];	/*first 446-bytes at this zero sector for boot-loader */

    printf("%-5s      %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
	   "Device", "Boot", "Start", "End", "Sectors", "Size", "Id",
	   "Type");

    for (int i = 0; i < 4; i++) {
	if (prtEntry_Ptr[i].LBA == 0) {	/*Do nothing if there is no partition with provided index */
	} else {
	    printf
		("%s%-5d %-5c %-10u  %-10u %-10u    %.1fG       %-10X %-10s\n",
		 argv[1], i + 1,
		 prtEntry_Ptr[i].status == 0x80 ? '*' : ' ',
		 prtEntry_Ptr[i].LBA,
		 (prtEntry_Ptr[i].LBA + prtEntry_Ptr[i].sectorCount - 1),
		 prtEntry_Ptr[i].sectorCount, ((float)
					       (((double)
						 prtEntry_Ptr
						 [i].sectorCount * 512) /
						(1024 * 1024 * 1024))),
		 prtEntry_Ptr[i].partitionType,
		 prtEntry_Ptr[i].partitionType ==
		 7 ? "HPFS/NTFS/exFAT" : prtEntry_Ptr[i].partitionType ==
		 0x83 ? "Linux" : prtEntry_Ptr[i].partitionType ==
		 0x82 ? "Linux swap / Solaris" : prtEntry_Ptr[i].
		 partitionType == 5 ? "Extended" : "  ");
	}
    }

    /* Reading Extended Partitions */

    uint32_t offset = 0;	/*offset for correct use of lseek with correct number of sector */
    uint32_t j = 4;		/*index to start numbering Extended Partitions after the primary 4 partitions */

    for (int i = 0; i < 4; i++) {
	/*check if the partition from type Extended or not */
	if (prtEntry_Ptr[i].partitionType == 5) {

	    /*offset to start reading from required Extended partition */
	    lseek(fd, (off_t) ((prtEntry_Ptr[i].LBA) + offset) * 512,
		  SEEK_SET);
	    offset = offset + prtEntry_Ptr[i].LBA;	/*Edit offset for up-coming iterations */
	    read(fd, buf, 512);

	    i = 0;		/*Every time reading from extended consider the logical partition is the first partition */
	    printf
		("%s%-5d %-5c %-10u  %-10u %-10u    %.1fG       %-10X %-10s\n",
		 argv[1], j + 1,
		 prtEntry_Ptr[i].status == 0x80 ? '*' : ' ',
		 (offset + prtEntry_Ptr[i].LBA),
		 (offset + prtEntry_Ptr[i].LBA +
		  prtEntry_Ptr[i].sectorCount - 1),
		 prtEntry_Ptr[i].sectorCount, ((float)
					       (((double)
						 prtEntry_Ptr
						 [i].sectorCount * 512) /
						(1024 * 1024 * 1024))),
		 prtEntry_Ptr[i].partitionType,
		 prtEntry_Ptr[i].partitionType ==
		 7 ? "HPFS/NTFS/exFAT" : prtEntry_Ptr[i].partitionType ==
		 0x83 ? "Linux" : prtEntry_Ptr[i].partitionType ==
		 0x82 ? "Linux swap / Solaris" : prtEntry_Ptr[i].
		 partitionType == 5 ? "Extended" : "  ");

	    j++;		/*increment partition numbering index for up coming iterations */
	}
    }

    return 0;
}
