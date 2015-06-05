README ETRACE
=============

A user-space daemon companion to Linux/ftrace

Building:
---------

Get documentation below first. Build depends on source-file in
documentation to generate manpage. Build-system is CMake-based. Please read
more about it in the wiki.

Documentation:
--------------

All documentation is a gitit wiki. Start reading in these simple steps:

1) Get all sub-modules - documentation is one of them

    git submodule init
    git submodule update

2) Install gitit if you don't have it already:

http://gitit.net/Installing

3) Execute the following:

    ./start_wiki.sh

4) Open browser and go to:

[http://localhost:5011](http://localhost:5011)

Cloning project from github:
----------------------------

To clone project including it's submodules you can also do as follows:

    git clone --recursive https://github.com/mambrus/etrace.git

In this case #1 under "Documentation" can be omitted.


