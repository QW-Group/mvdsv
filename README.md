# MVDSV: a QuakeWorld server
![MVDSV Logo](https://raw.githubusercontent.com/QW-Group/mvdsv/master/resources/logo/mvdsv.png)

**[MVDSV][mvdsv]** (MultiView Demo SerVer) has been the most popular **QuakeWorld** server in the world for more than a decade because of its ability to record every player's point of view in a server side demo and provide many different game modes to enjoy **QuakeWorld** with.

## Getting Started

The following instructions will help you get a **[MVDSV][mvdsv]** server up and running on your local machine from prebuilt binaries. Details on how to compile your own **[MVDSV][mvdsv]** binary will also be included to match specific architectures or for development purposes.

## Supported architectures

The following architectures are fully supported by **[MVDSV][mvdsv]** and are available as prebuilt binaries:
* Linux amd64 (Intel and AMD 64-bits processors)
* Linux i686 (Intel and AMD 32-bit processors)
* Linux aarch (ARM 64-bit processors)
* Linux armhf (ARM 32-bit processors)
* Windows x64 (Intel and AMD 64-bits processors)
* Windows x86 (Intel and AMD 32-bit processors)

## Prebuilt binaries
You can find the prebuilt binaries on [this download page][mvdsv-builds].

## Prerequisites

None at the moment.

## Installing

For more detailed information we suggest looking at the [nQuake server][nquake-linux], which uses **[MVDSV][mvdsv]** and **[KTX][ktx]** as **QuakeWorld** server.

## Building binaries

### Build from source with CMake

Assuming you have installed essential build tools and ``CMake``
```bash
mkdir build && cmake -B build . && cmake --build build
```
Build artifacts would be inside ``build/`` directory, for unix like systems it would be ``mvdsv``.

You can also use ``build_cmake.sh`` script, it mostly suitable for cross compilation
and probably useless for experienced CMake user.
Some examples:
```
./build_cmake.sh linux-amd64
```
should build MVDSV for ``linux-amd64`` platform, release version, check [cross-cmake](tools/cross-cmake) directory for all platforms

```
B=Debug ./build_cmake.sh linux-amd64
```
should build MVDSV for linux-amd64 platform with debug

```
V=1 B=Debug ./build_cmake.sh linux-amd64
```
should build MVDSV for linux-amd64 platform with debug, verbose (useful if you need validate compiler flags)

```
G="Unix Makefiles" ./build_cmake.sh linux-amd64
```

force CMake generator to be unix makefiles

```
./build_cmake.sh linux-amd64
```

build MVDSV for ``linux-amd64`` version, you can provide
any platform combinations.

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

## Code of Conduct

We try to stick to our code of conduct when it comes to interaction around this project. See the [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) file for details.

## License

This project is licensed under the GPL-2.0 License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

* Thanks to **Jon "bps" Cednert** for the **[MVDSV][mvdsv]** logo.
* Thanks to the fine folks on [Quakeworld Discord][discord-qw] for their support and ideas.

[mvdsv]: https://github.com/QW-Group/mvdsv
[mvdsv-tags]: https://github.com/QW-Group/mvdsv/tags
[mvdsv-builds]: https://builds.quakeworld.nu/mvdsv
[ktx]: https://github.com/QW-Group/ktx
[nquake-linux]: https://github.com/nQuake/server-linux
[discord-qw]: http://discord.quake.world/
