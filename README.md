MVDSV: a QuakeWorld server
--------------------------

Official website:
  https://github.com/deurk/mvdsv

Development team:
  see CREDITS file

License:
  GPL2, see LICENSE file


Build from source with meson
----------------------------

In general:

- use Ubuntu 14.04 (but should work under 16.04 as well) as virtual machine, check out source code there
- install required packages for compilation
- set up virtualenv and install python packages (required for meson and ninja builders)
- run meson build for given directory (optionally with cross compilation settings)
- run ninja to generate .so file
- you should have ``qwprogs.so`` file in ``build_*`` directory, put it in your quake server/ktx/ directory.

Detailed commands to install packages, tools and compilation can be found in ``.travis.yml`` file.
You should be able to compile binaries for most popular platforms, such as:

- Linux 32-bit and 64-bit
- Windows 32-bit and 64-bit (WoW64)
- Arm 7 - for RaspBerry Pi 3 with Raspbian

Example for Linux amd64

```bash
$ export TARGET=linux-amd64
$ rm -rf build_${TARGET}

$ meson build_${TARGET} --cross-file cross-compilation_${TARGET}.txt
The Meson build system
Version: 0.41.2
Source dir: /home/kaszpir/src/deurk/mvdsv
Build dir: /home/kaszpir/src/deurk/mvdsv/build_linux-linux-amd64
Build type: cross build
Project name: mvdsv
Native c compiler: cc (gcc 5.4.0)
Cross c compiler: gcc (gcc 5.4.0)
Host machine cpu family: x86_64
Host machine cpu: x86_64
Target machine cpu family: x86_64
Target machine cpu: x86_64
Build machine cpu family: x86_64
Build machine cpu: x86_64
Dependency threads found: YES
Cross dependency libpcre found: YES 8.38
Cross dependency libcurl found: YES 7.47.0
Library m found: YES
Library dl found: YES
Build targets in project: 1

$ ninja -C build_${TARGET}

ninja: Entering directory `build_linux-amd64'
[13/46] Compiling C object 'mvdsv@exe/src_net.c.o'.

...

[43/46] Compiling C object 'mvdsv@exe/src_central.c.o'.
../src/central.c: In function ‘Web_SubmitRequestForm’:
../src/central.c:332:15: warning: unused variable ‘code’ [-Wunused-variable]
  CURLFORMcode code = curl_formadd(&first_form_ptr, &last_form_ptr,
               ^
../src/central.c: In function ‘Central_VerifyChallengeResponse’:
../src/central.c:363:15: warning: variable ‘code’ set but not used [-Wunused-but-set-variable]
  CURLFORMcode code;
               ^
../src/central.c: In function ‘Central_GenerateChallenge’:
../src/central.c:388:15: warning: variable ‘code’ set but not used [-Wunused-but-set-variable]
  CURLFORMcode code;
               ^
../src/central.c: In function ‘Web_SendRequest’:
../src/central.c:463:12: warning: comparison between ‘CURLFORMcode {aka enum <anonymous>}’ and ‘enum <anonymous>’ [-Wenum-compare]
   if (code != CURLE_OK) {
            ^
../src/central.c: In function ‘Web_PostFileRequest_f’:
../src/central.c:490:15: warning: variable ‘code’ set but not used [-Wunused-but-set-variable]
  CURLFORMcode code = 0;
               ^
[46/46] Linking target mvdsv.

$ file build_${TARGET}/mvdsv
build_linux-amd64/mvdsv: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=dedd6661cff55d457b15d2641c02baaf7be9a8b1, not stripped


```

In ``build_*/`` there will be mvdsv binary, change permissions to executable and use it to start server.
