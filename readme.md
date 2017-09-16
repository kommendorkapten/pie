# PIE

Free photo processor and photo organiser written as a HTML 5
client/server implementation. Licensed under CDDL.

Written in C99 & POSIX 2001 with SIMD (SSE, AVX & AltiVec/VMX) extensions.

Dependent on:

* libwebsockets (LGPL 2.1)
* libraw (CDDL)
* libexif (LGPL 2.1)
* libpng (libpng license)
* libjpeg (custom BSD)
* openssl (BSD)

## Included thirdparty components

* [JSMN](https://github.com/zserge/jsmn), MIT license.
* [Chart.js](http://chartjs.org/), MIT license.
* [CollapsibleLists](http://code.stephenmorley.org), CC0 1.0 Universal License

## Built and tested on

* Solaris 11, x86-64, Oracle Developer Studio 12.5
* Solaris 11, x86-64, gcc 4.9.4
* Solaris 11, sparcv9, Oracle Developer Studio 12.5
* Solaris 11, sparcv9, gcc 4.9.4
* ~~FreeBSD 11, PowerPC64, gcc 4.2.1~~ Currently broken, libraw is
  outdated in FreeBSD 11 PPC64.
* Mac OSX 10.11.2, x86-64, LLVM 6.0
