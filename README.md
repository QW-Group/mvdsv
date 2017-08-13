# MVDSV: a QuakeWorld server

MVDSV (MultiView Demo SerVer) has been the most popular QuakeWorld server in the world for the more than a decade because of its ability to record every player's point of view in a server side demo and provide many different game modes to enjoy QuakeWorld with.

_(This README.md file is stil a work in progress. bear with us while we polish it!)_
## Getting Started

The following instructions will help you get a MVDSV server up and running on your local machine from prebuilt binaries. Details on how to compile your own MVDSV binary will also be included to match specific architectures or for development purposes.

## Supported architectures

The following architectures are fully supported by MVDSV and are available as prebuilt binaries:
* Linux i686 (Intel and AMD 32-bit processors)
* Linux amd64 (Intel and AMD 64-bits processors)
* Windows x86 (Intel and AMD 32-bit processors)
* Windows x64 (Intel and AMD 64-bits processors)
* Mac OS X (Intel 64-bit processors)
* Linux armhf (ARM 32-bit processors)

## Prerequisites

TBD

## Installing

TBD

## Building binaries

### Build from source with meson

Detailed commands to install packages, tools and compilation can be found in ``.travis.yml`` file.
There are extra conditionals to install desired packages based on the TARGET.

In general:

- use Ubuntu 14.04 (but should work under 16.04 as well) as virtual machine, check out source code there
- install required packages for compilation
- set up virtualenv and install python packages (required for meson and ninja builders)
- run meson build for given directory (optionally with cross compilation settings)
- run ninja to generate .so file
- you should have ``mvdsv`` file in ``build_*`` directory, put it in your quake server/ directory.

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

#### Linux on Intel/AMD 32-bit

```
# ~/build_linux-i686# ../../meson/meson.py --buildtype=release --cross-file=../tools/cross-compilation/linux-i686.txt
The Meson build system
Version: 0.42.0.dev1
Source dir: /root/code/ktx
Build dir: /root/code/ktx/build_linux-i686
Build type: cross build
Project name: ktx
Native C compiler: cc (gcc 4.9.2)
Cross C compiler: x86_64-linux-gnu-gcc (gcc 4.9.2)
Host machine cpu family: x86
Host machine cpu: i686
Target machine cpu family: x86
Target machine cpu: i686
Build machine cpu family: x86_64
Build machine cpu: x86_64
Library m found: YES
Build targets in project: 1

# ~/build_linux-i686# ninja
[99/99] Linking target qwprogs.so.

# ~/build_linux-i686# file qwprogs.so
qwprogs.so: ELF 32-bit LSB shared object, Intel 80386, version 1 (SYSV), dynamically linked, BuildID[sha1]=428a10ae550e769df1388c129937d48176a23b51, not stripped
```
#### Linux on Intel/AMD 64-bit

```
# ~/build_linux-amd64# ../../meson/meson.py --buildtype=release --cross-file=../tools/cross-compilation/linux-amd64.txt
The Meson build system
Version: 0.42.0.dev1
Source dir: /root/code/ktx
Build dir: /root/code/ktx/build_linux-amd64
Build type: cross build
Project name: ktx
Native C compiler: cc (gcc 4.9.2)
Cross C compiler: gcc (gcc 4.9.2)
Host machine cpu family: x86_64
Host machine cpu: x86_64
Target machine cpu family: x86_64
Target machine cpu: x86_64
Build machine cpu family: x86_64
Build machine cpu: x86_64
Library m found: YES
Build targets in project: 1

# ~/build_linux-amd64# ninja
[99/99] Linking target qwprogs.so.

# ~/build_linux-amd64# file qwprogs.so
qwprogs.so: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, BuildID[sha1]=5bd27876114dbf4b0dcf6a190c90f5e800ef480c, not stripped
```
#### Windows on Intel/AMD 32-bit

```
# ~/build_windows-x86# ../../meson/meson.py --buildtype=release --cross-file=../tools/cross-compilation/windows-x86.txt
The Meson build system
Version: 0.42.0.dev1
Source dir: /root/code/ktx
Build dir: /root/code/ktx/build_windows-x86
Build type: cross build
Project name: ktx
Native C compiler: cc (gcc 4.9.2)
Cross C compiler: i686-w64-mingw32-gcc (gcc 4.9.1)
Host machine cpu family: x86
Host machine cpu: i686
Target machine cpu family: x86
Target machine cpu: i686
Build machine cpu family: x86_64
Build machine cpu: x86_64
Library m found: YES
Build targets in project: 1

# ~/build_windows-x86# ninja
[99/99] Linking target qwprogs.dll.

# ~/build_windows-x86# file qwprogs.dll
qwprogs.dll: PE32 executable (DLL) (console) Intel 80386, for MS Windows
```

#### Windows on Intel/AMD 64-bit

```
# ~/build_windows-x64# ../../meson/meson.py --buildtype=release --cross-file=../tools/cross-compilation/windows-x64.txt
The Meson build system
Version: 0.42.0.dev1
Source dir: /root/code/ktx
Build dir: /root/code/ktx/build_windows-x64
Build type: cross build
Project name: ktx
Native C compiler: cc (gcc 4.9.2)
Cross C compiler: x86_64-w64-mingw32-gcc (gcc 4.9.1)
Host machine cpu family: x86_64
Host machine cpu: x86_64
Target machine cpu family: x86_64
Target machine cpu: x86_64
Build machine cpu family: x86_64
Build machine cpu: x86_64
Library m found: YES
Build targets in project: 1

# ~/build_windows-x64# ninja
[99/99] Linking target qwprogs.dll.

# ~/build_windows-x64# file qwprogs.dll
qwprogs.dll: PE32+ executable (DLL) (console) x86-64, for MS Windows
```
#### Linux on Rapsberry 3

For Jessie
```
echo 'deb http://emdebian.org/tools/debian/ jessie main' >> /etc/apt/sources.list.d/crosstools.list
curl http://emdebian.org/tools/debian/emdebian-toolchain-archive.key | sudo apt-key add -
dpkg --add-architecture armhf
apt-get update
apt-get install
```

```
# ~/build_linux-armhf# ../../meson/meson.py --buildtype=release --cross-file=../tools/cross-compilation/linux-armhf.txt
The Meson build system
Version: 0.42.0.dev1
Source dir: /root/code/ktx
Build dir: /root/code/ktx/build_linux-armhf
Build type: cross build
Project name: ktx
Native C compiler: cc (gcc 4.9.2)
Cross C compiler: arm-none-eabi-gcc (gcc 4.8.4)
Host machine cpu family: arm
Host machine cpu: armv7hl
Target machine cpu family: arm
Target machine cpu: armv7hl
Build machine cpu family: x86_64
Build machine cpu: x86_64
Library m found: YES
Build targets in project: 1

# ~/build_linux-armhf# ninja
[99/99] Linking target qwprogs.so.

# ~/build_linux-armhf# file qwprogs.so
qwprogs.so: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, BuildID[sha1]=e5019921d8b642d8eb6e1e38c0218e26b0a7ded3, not stripped
```

## Built With

TBD

## Versioning

We use a pretty crappy system for versioning for now. For the versions available, see the [tags on this repository](https://github.com/deurk/ktx/tags). 

## Authors

(Listed by last name alphabetic order)

* **Ivan Bolshunov** - *qqshka*
* **Dominic Evans** - *oldman*
* **Anton Gavrilov** - *tonik*
* **Andrew Grondalski** - *ult*
* **Dmitry Musatov** - *disconnect*
* **Alexandre Nizoux** - *deurk*
* **Tero Parkkonen** - *Renzo*
* **Vladimir Vladimirovich** - *VVD*

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for , and the process for submitting pull requests to us.

## Code of Conduct

We try to stick to our code of conduct when it comes to interaction around this project. See the [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) file for details.

## License

This project is licensed under the GPL-2.0 License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

* Hat tip to anyone who's code was used
* Inspiration
* etc
