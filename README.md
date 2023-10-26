Non-Timeline-XT
============


Non-Timeline-XT is a reboot of original Non-Timeline with some eXTras. Currently the only changes are moving to cmake build and adding NSM optional gui feature.


Non-Timeline-XT build instructions:
--------------------------------

Dependencies :

* ntk
* liblo
* liblo-dev
* libsndfile1
* libsndfile1-dev
* jack2

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

For package maintainers, if you are building generic binary packages to be used on different architectures,
then NativeOptimizations must be disabled:

```bash
    cmake -DNativeOptimizations=OFF ..
```
