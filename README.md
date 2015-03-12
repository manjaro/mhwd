## Manjaro HardWare Detection (mhwd)

#### Copyright (C) 2012 2013 2014 2015 Manjaro Developers
#### Project licensed under GNU GPL v.3 - Check COPYING file

#### The Manjaro HardWare Detection (mhwd) command is a unique feature of [Manjaro Linux](http://manjaro.org/) operating system.
#### There are currently two types of mhwd command:
1. mhwd: Enables the automatic detection and configuration of computer hardware the system is running on. This includes both hardware connected internally via PCI (e.g. graphics cards), and connected externally via [USB](http://en.wikipedia.org/wiki/USB) (e.g. flashdrives).
2. mhwd-kernel: Enables the installation and easy management of multiple kernels on your system.

#### For more info about mhwd go to [mhwd wiki page](https://wiki.manjaro.org/index.php?title=Manjaro_Hardware_Detection_Overview).

#### TODO list
 - [ ] allow to set the base lib dir
 - [ ] force to remove db config
 - [ ] show detailed info for one config
 - [ ] set architecture from library not from bash
 - [ ] pass ids and bus ids to script
 - [ ] don't remove packages on reinstallation...
 - [ ] add option similar to pacman --root ...

### Contribute

#### Code Convention

The code convention used in this project is CamelCase. For example:

Instead of:

```c++
void Mhwd::set_version_mhwd(std::string version_of_software, std::string year_copyright)
{
}
```

write:

```c++
void Mhwd::setVersionMhwd(std::string versionOfSoftware, std::string yearCopyright)
{
}
```

#### Software need for coding or testing

* C++ tool chain: [g++](https://gcc.gnu.org/) or [clang](http://clang.llvm.org/), [cmake](http://www.cmake.org/), [make](http://www.gnu.org/software/make/)
* Good Knowledge of [C++11](http://isocpp.org/)
* [git](http://git-scm.com/)

#### Building

In a terminal window do:

```shell
[your-name@your-name mhwd]$ mkdir build
[your-name@your-name mhwd]$ cd build/
[your-name@your-name build]$ cmake ..
[your-name@your-name build]$ make
```
now you have the mhwd program in the folder mhwd/bin/

or

```shell
[your-name@your-name build]$ make install
```
to install the files in linux directories!

#### Have a happy coding ! :thumbsup:
