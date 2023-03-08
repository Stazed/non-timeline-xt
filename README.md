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

Getting NTK:
------------

Your distribution may have the NTK library. If not, then do the following to build and install the NTK submodule.

If you just cloned the non-timeline-xt repository or just executed git pull, then you should also run :

```bash
    git submodule update --init
```

to pull down the latest NTK code required by Non. Git does *not* do this automatically.

Building NTK:
-------------

If you don't have NTK installed system-wide (which isn't very likely yet) you *MUST* begin the build process by typing:

```bash
    cd lib/ntk
    ./waf configure
    ./waf
```

Once NTK has been built you must install it system-wide before attempting to build non-timeline-xt.

To install NTK type:

```bash
    sudo ./waf install
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


