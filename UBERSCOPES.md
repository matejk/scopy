= Build instructions =

== Obtain sources of scopy and dependencies ==

You obviously already did this since you are reading this file. Switch to proper
branch and update submodules:

git checkout uberscope-dev
git submodules update --init --recursive

== Install Debian build devscripts ==

sudo apt install git devscripts equivs

== Create build dep package and install it ==

In the root directory of the cloned repo:

mk-build-deps debian/control

A debian package with all build dependencies is created. Install it to get all
packages required for building Scopy.

sudo apt install ./scopy-build-deps_*_all.deb

== Compile and create deb packages ==

A makefile makes building easier:

make -f uberscope-deb.make

