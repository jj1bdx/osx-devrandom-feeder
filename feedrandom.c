/*
 * Feeder for /dev/random for OS X
 * by Kenji Rikitake
 *
 * Copyright (c) 2015 Kenji Rikitake
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sysexits.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define OUTPUTFILE "/dev/random"

void usage(void) {
    errx(EX_USAGE,
        "Usage: %s [-d cu.-device] [-s speed] [-o] [-h]\n"
        "Only cu.[.+] and /dev/cu.[.+] are accepted\n"
        "Speed range: 9600 to 1000000 [bps] (default: 115200)\n"
        "Default output device: %s (use -o to output to stdout)\n"
        "Use -h for help", getprogname(), OUTPUTFILE);
}

/* 
 * buffer size
 * for fetching from the TRNG tty device
 * designed for NeuG (~80kbytes/sec)
 * change the value for a higher-speed device
 */

#define BUFFERSIZE (512)

int main(int argc, char *argv[]) {

    uint8_t rbuf[BUFFERSIZE], *p;
    int ttyfd, randomfd;
    struct termios ttyconfig;
    ssize_t rsize, wsize;
    int i;
    int dflag = 0;
    int ch;
    char input[MAXPATHLEN];
    char *inputbase;
    char devname[MAXPATHLEN];
    long speedval = 115200L;
    int oflag = 0;

    if (argc < 2) {
        usage();
    }
    while ((ch = getopt(argc, argv, "d:s:oh")) != -1) {
        switch (ch) {
        case 'd':
            dflag = 1;
            if (strlcpy(input, optarg, MAXPATHLEN) >= MAXPATHLEN) {
                errx(EX_USAGE, "device input string size error");
            }
            if (NULL == (inputbase = basename(input))) {
                errx(EX_OSERR, "device input basename failed");
            }
            if ((*inputbase == '/') || (*inputbase == '.')) {
                errx(EX_USAGE, "illegal path in inputbase");
            }
            break;
        case 's':
            errno = 0;
            speedval = strtol(optarg, NULL, 10);
            if (errno > 0) {
                err(EX_OSERR, "strtol for speedval failed");
            }
            if ((speedval < 9600) || (speedval > 1000000)) {
                errx(EX_USAGE, "speedval %ld out of range", speedval);
            }
            break;
        case 'o':
            oflag = 1;
            break;
        case 'h':
            usage();
            break;
        case '?':
        default:
            usage();
        }
    }
    if (dflag == 0) {
        errx(EX_USAGE, "no device name given");
    }
    if (strnlen(inputbase, 4) < 4) {
        errx(EX_USAGE, "input basename less than four letters");
    }
    if ((inputbase[0] != 'c') ||
        (inputbase[1] != 'u') ||
        (inputbase[2] != '.')) {
        errx(EX_USAGE, "not a /dev/cu.* device");
    }
    if ((strlcpy(devname, "/dev/", MAXPATHLEN)) >= MAXPATHLEN) {
        errx(EX_OSERR, "strlcpy devname failed");
    }
    if ((strlcat(devname, inputbase, MAXPATHLEN)) >= MAXPATHLEN) {
        errx(EX_OSERR, "strlcat devname failed");
    }

    /* open TRNG tty */
    if ((ttyfd = open(devname, O_RDONLY)) == -1) {
        err(EX_IOERR, "cannot open tty file");
    }
    /* check if really a tty */
    if (0 == isatty(ttyfd)) {
        err(EX_IOERR, "input not a tty");
    }
    /* set exclusive access */
    if (-1 == ioctl(ttyfd, TIOCEXCL, 0)) {
        err(EX_IOERR, "input ioctl(TIOCEXCL) failed");
    }
    /* get tty discipline */
    if (-1 == tcgetattr(ttyfd, &ttyconfig)) {
        err(EX_IOERR, "input tcgetattr failed");
    }
    /* set RAW mode (see cfmakeraw(4)) */
    /* and set all transparency flags */
    /* no CTS/RTS flow control */
    /* CLOCAL cleared (modem control enabled) */
    ttyconfig.c_iflag &=
        ~(IMAXBEL|IXOFF|INPCK|BRKINT|PARMRK|
                ISTRIP|INLCR|IGNCR|ICRNL|IXON|IGNPAR);
    ttyconfig.c_iflag |= IGNBRK;
    ttyconfig.c_oflag &= ~OPOST;
    ttyconfig.c_lflag &=
        ~(ECHO|ECHOE|ECHOK|ECHOKE|ECHOCTL|
                ECHONL|ICANON|ISIG|IEXTEN|NOFLSH|TOSTOP|PENDIN);
    ttyconfig.c_cflag &=
        ~(CSIZE|PARENB|CRTS_IFLOW|CCTS_OFLOW|MDMBUF);
    ttyconfig.c_cflag |= CS8|CREAD;
    ttyconfig.c_cflag &= ~CLOCAL;
    ttyconfig.c_cc[VMIN] = 1;
    ttyconfig.c_cc[VTIME] = 0;
    /* set speed */
    if (-1 == cfsetspeed(&ttyconfig, (speed_t)speedval)) {
        err(EX_IOERR, "input cfsetspeed to %ld failed", speedval);
    }
    if (-1 == tcsetattr(ttyfd, TCSANOW, &ttyconfig)) {
        err(EX_IOERR,
            "input tcsetattr for raw and speed %ld failed", speedval);
    }

    /* open random output device */
    if (oflag) {
        /* use stdout */
        if ((randomfd = fcntl(STDOUT_FILENO, F_DUPFD, 0)) == -1) {
            err(EX_IOERR, "cannot open stdout");
        }
    } else {
        /* use default output file */
        if ((randomfd = open(OUTPUTFILE, O_WRONLY)) == -1) {
            errx(EX_IOERR, "cannot open %s", OUTPUTFILE);
        }
    }

    /* infinite loop */
    while (1) {
        /* fill the receive buffer first */
        for (i = 0, p = rbuf; i < BUFFERSIZE; ) {
            /* try reading from tty */
            if ((rsize = read(ttyfd, p + i,
                            BUFFERSIZE - i)) < 1) {
                err(EX_IOERR, "read from tty failed");
            }
            /* add the number of bytes read */
            i += rsize;
        }
        if ((wsize = write(randomfd, rbuf,
                       (size_t)BUFFERSIZE)) == -1) {
            err(EX_IOERR, "/dev/random write failed");
        }
    }
    /* notreached */
    return 0;
}
