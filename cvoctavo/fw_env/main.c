#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>


static char* mtd_device = "/dev/mtd5";


static int get_bb_number(int fd, const mtd_info_t *meminfo)
{
   int isNAND = meminfo->type == MTD_NANDFLASH ? 1 : 0;
   int ibbCounter = 0;
   erase_info_t erase;
   erase.length = meminfo->erasesize;

   for (erase.start = 0;
        erase.start < meminfo->size;
        erase.start += meminfo->erasesize)
   {

       loff_t offset = erase.start;
       int ret = ioctl(fd, MEMGETBADBLOCK, &offset);
       if (ret > 0)
       {
           ibbCounter++;
           continue;
       }
       else if (ret < 0)
       {
           perror("Cannot read bad block mark");
           if (errno == EOPNOTSUPP)
           {
               if (isNAND)
               {
                   printf("\n\n%s: Bad block check not available\n", mtd_device);
                   exit(1);
               }
           }
           else
           {
               printf("\n%s: MTD get bad block failed: %s\n", mtd_device, strerror(errno));
               exit(1);
           }
       }
   }

   return ibbCounter;
}


int main(int argc, char *argv[])
{
    int md_fd;
    int iNumOfBadBlocks = 0;
    mtd_info_t  mtd_meminfo;

    // open & get meminfo for "/dev/mtd4"
    if ((md_fd = open(mtd_device, O_RDONLY)) < 0) {
        printf("\n\n%s: %s\n", mtd_device, strerror(errno));
        exit(1);
    }

    if (ioctl(md_fd, MEMGETINFO, &mtd_meminfo) != 0) {
        printf("\n\n%s: unable to get MTD device info\n", mtd_device);
        exit(1);
    }

    iNumOfBadBlocks = get_bb_number(md_fd, &mtd_meminfo);
    printf("Number of the bad blocks is %d\n", iNumOfBadBlocks);

    return 0;
}
