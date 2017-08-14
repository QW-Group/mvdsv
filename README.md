# MVDSV: a QuakeWorld server
[![Build Status](https://travis-ci.org/deurk/mvdsv.svg?branch=master)](https://travis-ci.org/deurk/mvdsv)

**[MVDSV][mvdsv]** (MultiView Demo SerVer) has been the most popular **QuakeWorld** server in the world for the more than a decade because of its ability to record every player's point of view in a server side demo and provide many different game modes to enjoy **QuakeWorld** with.

_(This README.md file is still a work in progress. bear with us while we polish it!)_

## Getting Started

The following instructions will help you get a **[MVDSV][mvdsv]** server up and running on your local machine from prebuilt binaries. Details on how to compile your own **[MVDSV][mvdsv]** binary will also be included to match specific architectures or for development purposes.

## Supported architectures

The following architectures are fully supported by **[MVDSV][mvdsv]** and are available as prebuilt binaries:
* Linux i686 (Intel and AMD 32-bit processors)
* Linux amd64 (Intel and AMD 64-bits processors)
* Windows x86 (Intel and AMD 32-bit processors)
* Windows x64 (Intel and AMD 64-bits processors)
* Mac OS X (Intel 64-bit processors)
* Linux armhf (ARM 32-bit processors)

## Prerequisites

TBD

## Installing

For more detailed information we suggest looking at the [nQuake server][nquake-linux], which uses **[MVDSV][mvdsv]** and **[KTX][ktx]** as **QuakeWorld** server.

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

$ meson build_${TARGET} --cross-file tools/cross-compilation/${TARGET}.txt
The Meson build system
Version: 0.41.2
Source dir: /mvdsv/src
Build dir: /mvdsv/src/build_linux-linux-amd64
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

```bash
sudo apt-get install gcc libc6-dev make
git clone https://github.com/deurk/mvdsv.git
cd mvdsv/build/make
./configure
make
```

This will create an executable binary at `mvdsv/build/make/mvdsv`.

Copy it to your server.

## Built With

TBD

## Versioning

We use a pretty crappy system for versioning for now. For the versions available, see the [tags on this repository][mvdsv-tags].

## Authors

(Listed by last name alphabetic order)

* **Ivan Bolshunov** - *qqshka*
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

* Hat tip to anyone who's code was used
* Inspiration
* etc

[ktx]: https://github.com/deurk/ktx
[mvdsv]: https://github.com/deurk/mvdsv
[mvdsv-tags]: https://github.com/deurk/mvdsv/tags
[nquake-linux]: https://github.com/nQuake/server-linux
