# MVDSV: a QuakeWorld server
![MVDSV Logo](https://raw.githubusercontent.com/deurk/mvdsv/master/resources/logo/mvdsv.png)

[![Build Status](https://travis-ci.org/deurk/mvdsv.svg?branch=master)](https://travis-ci.org/deurk/mvdsv)

**[MVDSV][mvdsv]** (MultiView Demo SerVer) has been the most popular **QuakeWorld** server in the world for more than a decade because of its ability to record every player's point of view in a server side demo and provide many different game modes to enjoy **QuakeWorld** with.

## Getting Started

The following instructions will help you get a **[MVDSV][mvdsv]** server up and running on your local machine from prebuilt binaries. Details on how to compile your own **[MVDSV][mvdsv]** binary will also be included to match specific architectures or for development purposes.

## Supported architectures

The following architectures are fully supported by **[MVDSV][mvdsv]** and are available as prebuilt binaries:
* Linux amd64 (Intel and AMD 64-bits processors)
* Linux i686 (Intel and AMD 32-bit processors)
* Linux armhf (ARM 32-bit processors)
* Windows x64 (Intel and AMD 64-bits processors)
* Windows x86 (Intel and AMD 32-bit processors)

## Prebuilt binaries
You can find the prebuilt binaries on [this download page][mvdsv_builds].

## Prerequisites

None at the moment.

## Installing

For more detailed information we suggest looking at the [nQuake server][nquake-linux], which uses **[MVDSV][mvdsv]** and **[KTX][ktx]** as **QuakeWorld** server.

## Building binaries

### Build from source with meson

Detailed commands to install packages, tools and compilation can be found in ``.travis.yml`` file.
There are extra conditionals to install desired packages based on the TARGET.

In general:

- use Ubuntu 18.04 as virtual machine, check out details about it on code on [Travis CI website][travis-build-env]
- install required packages for compilation
- set up virtualenv and install python packages (required for meson and ninja builders)
- run meson build for given directory (optionally with cross compilation settings)
- run ninja to generate the binary file
- you should have ``mvdsv`` file in ``build`` directory, put it in your quake server/ directory.

#### Example for Linux amd64

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
$ rm -rf build

$ meson build --cross-file tools/cross-compilation/${TARGET}.txt
The Meson build system
Version: 0.58.0
Source dir: /home/taps/workspace/mvdsv
Build dir: /home/taps/workspace/mvdsv/build
Build type: cross build
Project name: mvdsv
Project version: undefined
C compiler for the host machine: gcc (gcc 9.3.0 "gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0")
C linker for the host machine: gcc ld.bfd 2.34
C compiler for the build machine: cc (gcc 9.3.0 "cc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0")
C linker for the build machine: cc ld.bfd 2.34
Build machine cpu family: x86_64
Build machine cpu: x86_64
Host machine cpu family: x86_64
Host machine cpu: x86_64
Target machine cpu family: x86_64
Target machine cpu: x86_64
Run-time dependency threads found: YES
Found pkg-config: /usr/bin/pkg-config (0.29.1)
Found CMake: NO
Run-time dependency libpcre found: NO (tried pkgconfig and cmake)
Run-time dependency libcurl found: NO (tried pkgconfig and cmake)
Library m found: YES
Library dl found: YES
Build targets in project: 1


$ ninja -C build

ninja: Entering directory `build'
[47/47] Linking target mvdsv.

```

Check the output binary file:

```bash
$ file build/mvdsv
build/mvdsv: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=dedd6661cff55d457b15d2641c02baaf7be9a8b1, not stripped

```

In ``build/`` there will be ``mvdsv`` binary, change permissions to executable and copy it to quake directory to start quake server.

Known issues:

- When using cross compiling between 32bit and 64bit architecture make sure to reinstall *dev packages or run in chroot. See ``.travis.yml`` lines, there is ``apt-get remove`` command for this, because curl and pcre are in dependency but not required.
- When changing architecture builds, for example for arm, apt-get will install/remove conflicting packages. Don't be surprised that you compile ``linux-amd64``, then ``linux-armv7hl`` and then back ``linux-amd64`` and it does not work because files are missing :)


### Compiling on Ubuntu

```bash
sudo apt-get install gcc libc6-dev make
git clone https://github.com/deurk/mvdsv.git
cd mvdsv/build/make
./configure
make
```

This will create an executable binary at `mvdsv/build/make/mvdsv`.

Copy it to your server.


## Versioning

For the versions available, see the [tags on this repository][mvdsv-tags].

## Authors

(Listed by last name alphabetic order)

* **Ivan Bolsunov** - *qqshka*
* **Dominic Evans** - *oldman*
* **Anton Gavrilov** - *tonik*
* **Dmitry Musatov** - *disconnect*
* **Peter Nicol** - *meag*
* **Alexandre Nizoux** - *deurk*
* **Tero Parkkonen** - *Renzo*
* **Vladimir Vladimirovich** - *VVD*

Names of those contributors have been lost but they have also helped with this project: *bliP*, *danfe*, *hdworak*, *HighlandeR*, *jhodge*, *kreon*, *SD-Angel*.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for , and the process for submitting pull requests to us.

## Code of Conduct

We try to stick to our code of conduct when it comes to interaction around this project. See the [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) file for details.

## License

This project is licensed under the GPL-2.0 License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

* Thanks to **Jon "bps" Cednert** for the **[MVDSV][mvdsv]** logo.
* Thanks to the fine folks on [Quakeworld Discord][discord-qw] for their support and ideas.

[mvdsv]: https://github.com/deurk/mvdsv
[mvdsv-tags]: https://github.com/deurk/mvdsv/tags
[mvdsv_builds]: https://builds.quakeworld.nu/mvdsv
[ktx]: https://github.com/deurk/ktx
[nquake-linux]: https://github.com/nQuake/server-linux
[travis-build-env]: https://docs.travis-ci.com/user/reference/bionic/
[discord-qw]: http://discord.quake.world/
