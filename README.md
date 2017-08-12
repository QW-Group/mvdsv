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

Detailed commands to install packages, tools and compilation can be found in ``.travis.yml`` file.
There are extra conditionals to install desired packages based on the TARGET.

In general:

- use Ubuntu 14.04 (but should work under 16.04 as well) as virtual machine, check out source code there
- install required packages for compilation
- set up virtualenv and install python packages (required for meson and ninja builders)
- run meson build for given directory (optionally with cross compilation settings)
- run ninja to generate .so file
- you should have ``mvdsv`` file in ``build_*`` directory, put it in your quake server/ directory.

You should be able to compile binaries for most popular platforms, such as:

- Linux 32-bit and 64-bit
- Windows 32-bit and 64-bit (WoW64)
- Arm 7 - for RaspBerry Pi 3 with Raspbian

Example for Linux amd64 under Ubuntu 14.04 (should be similar under 16.04)

Install required packages:

```bash
$ sudo apt-get update
$ sudo apt-get -y upgrade
$ sudo apt-get -y install build-essential python-virtualenv python3-dev python3-pip ninja-build cmake gcc-multilib
```

Check out the code to the current directory:

```bash
git clone https://github.com/deurk/mvdsv.git .
```

Create virtualenv + install python packages:

```bash
$ virtualenv .venv --python=python3
$ . .venv/bin/activate
$ pip3 install --upgrade pip
$ pip3 install -r requirements.txt
```

For more detailed TARGET see ``.travis.yml`` - ``matrix`` section.
Export env var to define what target to compile, run the build commands.

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

```

Check the output binary file:

```bash
$ file build_${TARGET}/mvdsv
build_linux-amd64/mvdsv: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=dedd6661cff55d457b15d2641c02baaf7be9a8b1, not stripped

```

In ``build_*/`` there will be ``mvdsv`` binary, change permissions to executable and copy it to quake directory to start quake server.

Known issues:

- When using cross compiling between 32bit and 64bit architecture make sure to reinstall *dev packages or run in chroot. See ``.travis.yml`` lines, there is ``apt-get remove`` command for this, because curl and pcre are in dependency but not required.
- When changing architecture builds, for example for arm, apt-get will install/remove conflicting packages. Don't be surprised that you compile ``linux-amd64``, then ``linux-armv7hl`` and then back ``linux-amd64`` and it does not work because files are missing :)

=======
### Compiling on Ubuntu

```
sudo apt-get install gcc libc6-dev make
git clone https://github.com/deurk/mvdsv.git
cd mvdsv/build/make
./configure
make
```

This will create an executable binary at `mvdsv/build/make/mvdsv`.

Copy it to your server.