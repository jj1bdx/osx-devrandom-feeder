# feedrandom: an entropy injection feeder for OS X

## WARNING

This software is still at the *experimental stage*. There is no guarantee for any damage
caused by the use of this software. Caveat emptor.

## What this driver is for

To accommodate True Random Number Generator (TRNG) random bits into OS X kernel.

## IMPORTANT SECURITY NOTICE

*Note well: providing incorrect permission and unauthenticated or unscreened
data to /dev/random may degrade the quality of /dev/random, /dev/urandom, and
the security of the entire OS X operating system.*

## Tested environment

* macOS 12.1
* This code was formerly tested with OS X 10.11.1

## How this works

`feedrandom.c` is a C code example to transfer TRNG data from a tty device to
`/dev/random`. The code sets input tty disciplines and locks the tty, then feeds
the contents to `/dev/random`.

The source to write to `/dev/random` *must* be a real TRNG. Possible candidates are:

* [NeuG](http://www.gniibe.org/memo/development/gnuk/rng/neug.html), claiming ~80kbytes/sec generation speed
* [TrueRNG 2](https://www.tindie.com/products/ubldit/truerng-hardware-random-number-generator/), claiming ~43.5kbytes/sec generation speed

The following TRNG is slow (~2kbytes/sec) but may work well (disclaimer: Kenji
Rikitake develops the software and hardware):

* [avrhwrng](https://github.com/jj1bdx/avrhwrng/), an experimental hardware on Arduino Duemilanove/UNO

## Version

* 17-JAN-2021: 0.2.2 (Documentation fix)
* 16-SEP-2019: 0.2.1 (Bugfix and source code reformatting)
* 8-NOV-2015: 0.2.0 (Use SHA512 hash for 1:8 compression as default)
* 23-SEP-2015: 0.1.2 (Fix termios; now CLOCAL cleared, modem control enabled)
* 19-AUG-2015: 0.1.1 (Fix bug on tty read(2) of feedrandom)
* 12-AUG-2015: 0.1.0 (Initial release, based on FreeBSD /dev/trng 0.2.1)

## How to run feedrandom

    # Only /dev/cu.* devices are accepted
    feedrandom -d /dev/cu.usbmodem-device
    # only the basename(3) part is used and attached to `/dev/` directly
    # so this is also OK
    feedrandom -d cu.usbmodem-device
    # tty speed [bps] can be set (9600 ~ 1000000, default 115200)
    feedrandom -d cuaU1 -s 9600
    # for usage
    feedrandom -h

## tty discipline of the input tty

    # result of `sudo stty -f /dev/cu.usbmodem1d11341`
    # (sudo needed to override TIOCEXCL)
    speed 115200 baud;
    lflags: -icanon -isig -iexten -echo
    iflags: -icrnl -ixon -ixany -imaxbel ignbrk -brkint
    oflags: -opost -onlcr -oxtabs
    cflags: cs8 -parenb -clocal crtscts

## License

BSD 2-clause. See LICENSE.

SHA512 hashing code is from the following page: [Fast SHA-2 hashes in x86
assembly](http://www.nayuki.io/page/fast-sha2-hashes-in-x86-assembly) by
Project Nayuki. The related code is distributed under the MIT License.

