#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/times.h>

#include "syscall.h"
#include "machine.h"
#include "util.h"

void convstat(uint8_t *pi, const struct stat* ps) {
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
    };
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
    pi[0] = ps->st_dev & 0xff; // pseudo
    pi[1] = (ps->st_dev >> 8) & 0xff; // pseudo
    pi[2] = ps->st_ino & 0xff;
    pi[3] = (ps->st_ino >> 8) & 0xff;
    pi[4] = ps->st_mode & 0xff;
    pi[5] = (ps->st_mode >> 8) & 0xff;
    pi[6] = ps->st_nlink & 0xff;
    pi[7] = ps->st_uid & 0xff;
    pi[8] = ps->st_gid & 0xff;
    pi[9] = (ps->st_size >> 16) & 0xff;
    pi[10] = ps->st_size & 0xff;
    pi[11] = (ps->st_size >> 8) & 0xff;
    // addr
    //pi[12];
    // actime
    //pi[28];
    //pi[29];
    //pi[30];
    //pi[31];
    // modtime
    //pi[32];
    //pi[33];
    //pi[34];
    //pi[35];
}

void mysyscall(machine_t *pm) {
    uint16_t word0 = 0;
    uint16_t word1 = 0;
    char path0[PATH_MAX];
    char path1[PATH_MAX];
    ssize_t sret;
    int ret;

    //fprintf(stderr, "/ [DBG] sys %d, %04x: %04x\n", pm->syscallID, pm->addr, pm->bin);
    switch (pm->syscallID) {
    case 0:
        // indir
        assert(0);
        break;
    case 1:
        // exit
        _exit((int16_t)pm->r0);
        break;
    case 2:
        // fork
        ret = fork();
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            if (ret == 0) {
                // child
                // nothing to do
            } else {
                // parent
                pm->pc += 2;
            }
            //fprintf(stderr, "/ [DBG] fork pid: %d (pc: %04x)\n", ret, pm->pc);
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 3:
        // read
        word0 = fetch(pm);
        word1 = fetch(pm);
        if (pm->dirfd != -1 && pm->dirp != NULL && (int16_t)pm->r0 == pm->dirfd) {
            // dir
            struct dirent *ent;
            ent = readdir(pm->dirp);
            if (ent == NULL) {
                if (errno == 0) {
                    // EOF
                    sret = 0;
                } else {
                    // error
                    sret = -1;
                }
            } else {
                assert(word1 == 16);
                uint8_t *p = &pm->virtualMemory[word0];
                // ino
                p[0] = ent->d_ino & 0xff;
                p[1] = (ent->d_ino >> 8) & 0xff;
                // name
                strncpy((char *)&p[2], ent->d_name, 16 - 2 - 1);
                p[15] = '\0';
                sret = word1;
            }
        } else {
            // file
            sret = read((int16_t)pm->r0, &pm->virtualMemory[word0], word1);
        }
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
        addroot(path0, sizeof(path0), (const char *)&pm->virtualMemory[word0], pm->rootdir);
        // debug
        //fprintf(stderr, "/ [DBG] sys open; %04x; %06o\n", word0, word1);
        //fprintf(stderr, "/   %s, %s\n", (const char *)&pm->virtualMemory[word0], pm->rootdir);
        //fprintf(stderr, "/   %s\n", path0);
        ret = open(path0, word1);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);

            // check file or dir
            int fd = ret;
            struct stat s;
            ret = fstat(fd, &s);
            if (ret == 0 && S_ISDIR(s.st_mode)) {
                // dir
                DIR *dirp = fdopendir(fd);
                if (dirp == NULL) {
                    pm->r0 = errno & 0xffff;
                    setC(pm); // error bit
                    close(fd);
                } else {
                    // TODO: support only one dir per process, currently
                    assert(pm->dirfd == -1);
                    assert(pm->dirp == NULL);
                    pm->dirfd = fd;
                    pm->dirp = dirp;
                }
            }
        }
        break;
    case 6:
        // close
        if (pm->dirfd != -1 && pm->dirp != NULL && (int16_t)pm->r0 == pm->dirfd) {
            // dir
            ret = closedir(pm->dirp);
            pm->dirfd = -1;
            pm->dirp = NULL;
        } else {
            // file
            ret = close((int16_t)pm->r0);
        }
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 7:
        // wait
        {
            int status;
            ret = wait(&status);
            if (ret < 0) {
                pm->r0 = errno & 0xffff;
                setC(pm); // error bit
            } else {
                pm->r0 = ret & 0xffff;
                pm->r1 = status & 0xffff;
                clearC(pm);
            }
        }
        break;
    case 8:
        // creat
        word0 = fetch(pm);
        word1 = fetch(pm);
        addroot(path0, sizeof(path0), (const char *)&pm->virtualMemory[word0], pm->rootdir);
        // debug
        //fprintf(stderr, "/ [DBG] sys creat; %04x; %06o\n", word0, word1);
        //fprintf(stderr, "/   %s, %s\n", (const char *)&pm->virtualMemory[word0], pm->rootdir);
        //fprintf(stderr, "/   %s\n", path0);
        ret = creat(path0, word1);
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
        addroot(path0, sizeof(path0), (const char *)&pm->virtualMemory[word0], pm->rootdir);
        addroot(path1, sizeof(path1), (const char *)&pm->virtualMemory[word1], pm->rootdir);
        ret = link(path0, path1);
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
        addroot(path0, sizeof(path0), (const char *)&pm->virtualMemory[word0], pm->rootdir);
        ret = unlink(path0);
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
        // debug
        //fprintf(stderr, "/ [DBG] sys exec; %04x; %04x\n", word0, word1);
        //fprintf(stderr, "/ [DBG]   %s\n", (const char *)&pm->virtualMemory[word0]);

        // calc size of args & copy args
        clearC(pm); // clear error bit
        uint16_t na = 0;
        uint16_t nc = 0;
        {
            uint8_t *argv = &pm->virtualMemory[word1];
            uint16_t addr = read16(false, argv);
            argv += 2;
            while (addr != 0) {
                const char *pa = (const char *)&pm->virtualMemory[addr];
                // debug
                //fprintf(stderr, "/ [DBG]   argv[%d]: %s\n", na, pa);
                addr = read16(false, argv);
                argv += 2;
                na++;

                do {
                    pm->args[nc++] = *pa;
                    if (nc >= sizeof(pm->args) - 1) {
                        //fprintf(stderr, "/ [ERR] Too big args\n");
                        setC(pm); // error bit
                        goto outer;
                    }
                } while (*pa++ != '\0');
            }
            outer:
            if (nc & 1) {
                pm->args[nc++] = '\0';
            }
        }
        if (isC(pm)) {
            pm->r0 = 0xffff;
        } else {
            pm->argc = na;
            pm->argsbytes = nc;

            if (!load(pm, (const char *)&pm->virtualMemory[word0])) {
                pm->r0 = 0xffff;
                setC(pm); // error bit
            } else {
                pm->r0 = 0;
                pm->pc = 0xffff; // goto the end of the memory, then run the new text
                clearC(pm);
            }
        }
        break;
    case 12:
        // chdir
        word0 = fetch(pm);
        addroot(path0, sizeof(path0), (const char *)&pm->virtualMemory[word0], pm->rootdir);
        ret = chdir(path0);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 13:
        // time
        {
            time_t t = time(NULL);
            pm->r0 = (t >> 16) & 0xffff;
            pm->r1 = t & 0xffff;
        }
        break;
    case 15:
        // chmod
        word0 = fetch(pm);
        word1 = fetch(pm);
        addroot(path0, sizeof(path0), (const char *)&pm->virtualMemory[word0], pm->rootdir);
        ret = chmod(path0, word1);
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
        {
            addroot(path0, sizeof(path0), (const char *)&pm->virtualMemory[word0], pm->rootdir);

            struct stat s;
            ret = stat(path0, &s);
            if (ret < 0) {
                pm->r0 = errno & 0xffff;
                setC(pm); // error bit
            } else {
                pm->r0 = ret & 0xffff;
                clearC(pm);
                uint8_t *pi = &pm->virtualMemory[word1];
                convstat(pi, &s);
                // debug
                //fprintf(stderr, "/ [DBG] stat src: %06o\n", s.st_mode);
                //fprintf(stderr, "/ [DBG] stat dst: %06o\n", *(uint16_t *)(pi + 4));
            }
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
    case 23:
        // setuid
        fprintf(stderr, "/ [WRN] ignore setuid: sys %d, %04x: %04x\n", pm->syscallID, pm->addr, pm->bin);
        {
            pm->r0 = 0;
            clearC(pm);
        }
        break;
    case 24:
        // getuid
        pm->r0 = ((geteuid() & 0xff) << 8) | (getuid() & 0xff);
        break;
    case 28:
        // fstat
        word0 = fetch(pm);
        {
            struct stat s;
            ret = fstat((int16_t)pm->r0, &s);
            if (ret < 0) {
                pm->r0 = errno & 0xffff;
                setC(pm); // error bit
            } else {
                pm->r0 = ret & 0xffff;
                clearC(pm);
                uint8_t *pi = &pm->virtualMemory[word0];
                convstat(pi, &s);
                // debug
                //fprintf(stderr, "/ [DBG] fstat src: %06o\n", s.st_mode);
                //fprintf(stderr, "/ [DBG] fstat dst: %06o\n", *(uint16_t *)(pi + 4));
            }
        }
        break;
    case 41:
        // dup
        ret = dup((int16_t)pm->r0);
        if (ret < 0) {
            pm->r0 = errno & 0xffff;
            setC(pm); // error bit
        } else {
            pm->r0 = ret & 0xffff;
            clearC(pm);
        }
        break;
    case 42:
        // pipe
        {
            int pipefd[2];
            ret = pipe(pipefd);
            if (ret < 0) {
                pm->r0 = errno & 0xffff;
                setC(pm); // error bit
            } else {
                pm->r0 = pipefd[0] & 0xffff;
                pm->r1 = pipefd[1] & 0xffff;
                clearC(pm);
            }
        }
        break;
    case 43:
        // times
        word0 = fetch(pm);
        /* in 1/60 seconds
        struct tbuffer {
            int16_t proc_user_time;
            int16_t proc_system_time;
            int16_t child_user_time[2];
            int16_t child_system_time[2];
        };
        */
        {
            long ticks_per_sec = sysconf(_SC_CLK_TCK);
            struct tms sbuf;
            clock_t clk = times(&sbuf);
            assert(clk >= 0);

            // to 1/60 sec
            sbuf.tms_utime = sbuf.tms_utime * 60 / ticks_per_sec;
            sbuf.tms_stime = sbuf.tms_stime * 60 / ticks_per_sec;
            sbuf.tms_cutime = sbuf.tms_cutime * 60 / ticks_per_sec;
            sbuf.tms_cstime = sbuf.tms_cstime * 60 / ticks_per_sec;

            uint16_t *dbuf = (uint16_t *)&pm->virtualMemory[word0];
            dbuf[0] = sbuf.tms_utime & 0xffff;
            dbuf[1] = sbuf.tms_stime & 0xffff;
            dbuf[2] = (sbuf.tms_cutime >> 16) & 0xffff;
            dbuf[3] = sbuf.tms_cutime & 0xffff;
            dbuf[4] = (sbuf.tms_cstime >> 16) & 0xffff;
            dbuf[5] = sbuf.tms_cstime & 0xffff;
        }
        break;
    case 46:
        // setgid
        fprintf(stderr, "/ [WRN] ignore setgid: sys %d, %04x: %04x\n", pm->syscallID, pm->addr, pm->bin);
        {
            pm->r0 = 0;
            clearC(pm);
        }
        break;
    case 47:
        // getgid
        pm->r0 = ((getegid() & 0xff) << 8) | (getgid() & 0xff);
        break;
    case 48:
        // signal
        word0 = fetch(pm);
        word1 = fetch(pm);
        fprintf(stderr, "/ [WRN] ignore signal: sys %d; %d; 0x%04x, %04x: %04x\n", pm->syscallID, word0, word1, pm->addr, pm->bin);
        {
            pm->r0 = 0; // terminate
            clearC(pm);
        }
        break;
    default:
        // TODO: not implemented
        fprintf(stderr, "/ [ERR] Not implemented: sys %d, %04x: %04x\n", pm->syscallID, pm->addr, pm->bin);
        assert(0);
        break;
    }
}
