#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "syscall.h"
#include "machine.h"

void mysyscall(machine_t *pm) {
    uint16_t word0 = 0;
    uint16_t word1 = 0;
    ssize_t sret;
    int ret;

    //fprintf(stderr, "/ [DBG] %04x: %04x, sys %d\n", pm->addr, pm->bin, pm->syscallID);
    switch (pm->syscallID) {
    case 0:
        // indir
        assert(0);
        break;
    case 1:
        // exit
        _exit((int16_t)pm->r0);
        break;
    case 3:
        // read
        word0 = fetch(pm);
        word1 = fetch(pm);
        sret = read((int16_t)pm->r0, &pm->virtualMemory[word0], word1);
        if (sret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = sret & 0xffff;
            clearC(pm);
        }
        break;
    case 4:
        // write
        word0 = fetch(pm);
        word1 = fetch(pm);
        sret = write((int16_t)pm->r0, &pm->virtualMemory[word0], word1);
        if (sret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = sret & 0xffff;
            clearC(pm);
        }
        break;
    case 5:
        // open
        word0 = fetch(pm);
        word1 = fetch(pm);
        ret = open((const char *)&pm->virtualMemory[word0], word1);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 6:
        // close
        ret = close((int16_t)pm->r0);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 8:
        // creat
        word0 = fetch(pm);
        word1 = fetch(pm);
        ret = creat((const char *)&pm->virtualMemory[word0], word1);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 9:
        // link
        word0 = fetch(pm);
        word1 = fetch(pm);
        ret = link((const char *)&pm->virtualMemory[word0], (const char *)&pm->virtualMemory[word1]);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 10:
        // unlink
        word0 = fetch(pm);
        ret = unlink((const char *)&pm->virtualMemory[word0]);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 11:
        // exec
        word0 = fetch(pm);
        word1 = fetch(pm);
        assert(0);
        break;
    case 15:
        // chmod
        word0 = fetch(pm);
        word1 = fetch(pm);
        ret = chmod((const char *)&pm->virtualMemory[word0], word1);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 17:
        // break
        word0 = fetch(pm);
        uint16_t addr = (word0 + 63) & ~63;
        if (addr < pm->bssEnd || pm->sp < addr) {
            pm->r0 = 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = pm->brk;
            pm->brk = addr;
            clearC(pm);
        }
        break;
    case 18:
        // stat
        word0 = fetch(pm);
        word1 = fetch(pm);
        struct stat s;
        ret = stat((const char *)&pm->virtualMemory[word0], &s);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);

            struct inode {
                char  minor;         /* +0: minor device of i-node */
                char  major;         /* +1: major device */
                int   inumber;       /* +2 */
                int   flags;         /* +4: see below */
                char  nlinks;        /* +6: number of links to file */
                char  uid;           /* +7: user ID of owner */
                char  gid;           /* +8: group ID of owner */
                char  size0;         /* +9: high byte of 24-bit size */
                int   size1;         /* +10: low word of 24-bit size */
                int   addr[8];       /* +12: block numbers or device number */
                int   actime[2];     /* +28: time of last access */
                int   modtime[2];    /* +32: time of last modification */
            } i;
            /* flags
            100000   i-node is allocated
            060000   2-bit file type:
                000000   plain file
                040000   directory
                020000   character-type special file
                060000   block-type special file.
            010000   large file
            004000   set user-ID on execution
            002000   set group-ID on execution
            001000   save text image after execution
            000400   read (owner)
            000200   write (owner)
            000100   execute (owner)
            000070   read, write, execute (group)
            000007   read, write, execute (others)
            */
            uint8_t *pi = &pm->virtualMemory[word1];
            pi[0] = s.st_dev & 0xff; // pseudo
            pi[1] = (s.st_dev >> 8) & 0xff; // pseudo
            pi[2] = s.st_ino & 0xff;
            pi[3] = (s.st_ino >> 8) & 0xff;
            //pi[4];
            //pi[5];
            pi[6] = s.st_nlink & 0xff;
            pi[7] = s.st_uid & 0xff;
            pi[8] = s.st_gid & 0xff;
            pi[9] = (s.st_size >> 16) & 0xff;
            pi[10] = s.st_size & 0xff;
            pi[11] = (s.st_size >> 8) & 0xff;
            //pi[12];
            pi[28];
            pi[29];
            pi[30];
            pi[31];
            pi[32];
            pi[33];
            pi[34];
            pi[35];
        }
        break;
    case 19:
        // seek
        word0 = fetch(pm);
        word1 = fetch(pm);
        off_t offset;
        if (word1 == 0 || word1 == 3) {
            offset = word0;
        } else {
            offset = (int16_t)word0;
        }
        if (word1 == 3 || word1 == 4 || word1 == 5) {
            offset *= 512;
            word1 -= 3;
        }
        offset = lseek((int16_t)pm->r0, offset, word1);
        if (offset < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = offset & 0xffff;
            clearC(pm);
        }
        break;
    case 20:
        // getpid
        pm->r0 = getpid() & 0xffff;
        break;
    case 48:
        // signal
        word0 = fetch(pm);
        word1 = fetch(pm);
        fprintf(stderr, "/ [WRN] ignore signal %04x: %04x, sys %d; %d; 0x%04x\n", pm->addr, pm->bin, pm->syscallID, word0, word1);
        {
            pm->r0 = 0; // terminate
            clearC(pm);
        }
        break;
    default:
        // TODO: not implemented
        fprintf(stderr, "/ [ERR] Not implemented, %04x: %04x, sys %d\n", pm->addr, pm->bin, pm->syscallID);
        assert(0);
        break;
    }
}
