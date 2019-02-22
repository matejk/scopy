#!/usr/bin/env make

export DEBEMAIL:=info@uberscopes.com
export DEBFULLNAME:=Uberscopes d.o.o.

SCOPY_VERSION:=1.0.3

# Linux distribution identifier
LNX_DISTRO:=$(shell lsb_release -si | tr A-Z a-z)

# Linux distribution codename
LNX_DISTRO_CODENAME:=$(shell lsb_release -sc)

TODAY:=$(shell date +%Y%m%d)

SCOPY_PACKAGE_VERSION:=$(SCOPY_VERSION).$(TODAY)+0$(LNX_DISTRO)~0$(LNX_DISTRO_CODENAME)

DEBUILD_OPTS:=--set-envvar "DEBFULLNAME=$(DEBFULLNAME)" --set-envvar "DEBEMAIL=$(DEBEMAIL)"

all:
	@echo "-- Generating changelog files."
	rm -f debian/changelog
	rm -f debian/changelog.dch
	dch --create --empty --package scopy --newversion $(SCOPY_PACKAGE_VERSION)
	dch --append "Scopy and dependent libraries for Uberscopes instruments."
	dch --append "Version: $(SCOPY_PACKAGE_VERSION)"
	dch --release "See product documentation for description of changes."
	@echo "-- Building deb packages"
	debuild $(DEBUILD_OPTS) --no-tgz-check -b -us -uc

