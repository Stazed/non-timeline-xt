Non-Timeline-XT
===============

Screenshot
----------

![screenshot](https://raw.github.com/Stazed/non-timeline-xt/main/timeline/doc/non-timeline-xt-2.0.0.png "Non-Timeline-XT Release 2.0.0")


Non-Timeline-XT is a reboot of original Non-Timeline with some eXTras.

Beginning with version 2.0.0 the default build will use the FLTK library instead of NTK.


Non-Timeline-XT build instructions:
--------------------------------

Dependencies :

For fltk build:
* fltk
* fltk-dev

For NTK build:
* ntk

Other Dependencies:
* liblo
* liblo-dev
* libsndfile1
* libsndfile1-dev
* jack2       (Need development packages also)

Getting submodules (nonlib and FL):
---------------

```bash
    git submodule update --init
```

Getting NTK:
------------

Your distribution will likely have NTK available. If not then you can get NTK at:

```bash
    git clone https://github.com/linuxaudio/ntk
```

Build Non-Timeline-XT:
-------------------

For cmake build:

```bash
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
```

To uninstall:

```bash
    sudo make uninstall
```

To build with NTK:
```bash
    mkdir build
    cd build
    cmake -DEnableNTK=ON ..
    make
    sudo make install
```

For package maintainers, if you are building generic binary packages to be used on different architectures,
then NativeOptimizations must be disabled:

```bash
    cmake -DNativeOptimizations=OFF ..
```
