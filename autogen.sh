#!/bin/sh

# This script does all the magic calls to automake/autoconf and
# friends that are needed to configure a git clone. As described in
# the file HACKING you need a couple of extra tools to run this script
# successfully.
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.

ACLOCAL=${ACLOCAL-aclocal-1.19}
AUTOCONF=${AUTOCONF-autoconf}
AUTOHEADER=${AUTOHEADER-autoheader}
AUTOMAKE=${AUTOMAKE-automake-1.19}
LIBTOOLIZE=${LIBTOOLIZE-libtoolize}

AUTOCONF_REQUIRED_VERSION=2.68
AUTOMAKE_REQUIRED_VERSION=1.11.2
LIBTOOL_REQUIRED_VERSION=1.5
LIBTOOL_WIN32_REQUIRED_VERSION=2.2
GLIB_REQUIRED_VERSION=2.0.0


PROJECT="LiquidRescale library"
TEST_TYPE=-f
FILE=lqr/lqr_carver.c


srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.
ORIGDIR=`pwd`
cd $srcdir


check_version ()
{
    VERSION_A=$1
    VERSION_B=$2

    save_ifs="$IFS"
    IFS=.
    set dummy $VERSION_A 0 0 0
    MAJOR_A=$2
    MINOR_A=$3
    MICRO_A=$4
    set dummy $VERSION_B 0 0 0
    MAJOR_B=$2
    MINOR_B=$3
    MICRO_B=$4
    IFS="$save_ifs"

    if expr "$MAJOR_A" = "$MAJOR_B" > /dev/null; then
        if expr "$MINOR_A" \> "$MINOR_B" > /dev/null; then
           echo "yes (version $VERSION_A)"
        elif expr "$MINOR_A" = "$MINOR_B" > /dev/null; then
            if expr "$MICRO_A" \>= "$MICRO_B" > /dev/null; then
               echo "yes (version $VERSION_A)"
            else
                echo "Too old (version $VERSION_A)"
                DIE=1
            fi
        else
            echo "Too old (version $VERSION_A)"
            DIE=1
        fi
    elif expr "$MAJOR_A" \> "$MAJOR_B" > /dev/null; then
        echo "Major version might be too new ($VERSION_A)"
    else
        echo "Too old (version $VERSION_A)"
        DIE=1
    fi
}

echo
echo "I am testing that you have the tools required to build the"
echo "$PROJECT from git. This test is not foolproof,"
echo "so if anything goes wrong, see the file HACKING for more information..."
echo

DIE=0


OS=`uname -s`
case $OS in
    *YGWIN* | *INGW*)
        echo "Looks like Win32, you will need libtool $LIBTOOL_WIN32_REQUIRED_VERSION or newer."
        echo
        LIBTOOL_REQUIRED_VERSION=$LIBTOOL_WIN32_REQUIRED_VERSION
        ;;
esac

printf "checking for libtool >= %s ... " "$LIBTOOL_REQUIRED_VERSION"
if ($LIBTOOLIZE --version) < /dev/null > /dev/null 2>&1; then
   LIBTOOLIZE=$LIBTOOLIZE
elif (glibtoolize --version) < /dev/null > /dev/null 2>&1; then
   LIBTOOLIZE=glibtoolize
else
    echo
    echo "  You must have libtool installed to compile $PROJECT."
    echo "  Install the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    echo
    DIE=1
fi

if test x$LIBTOOLIZE != x; then
    VER=`$LIBTOOLIZE --version \
         | grep libtool | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $LIBTOOL_REQUIRED_VERSION
fi
printf "checking for autoconf >= %s ... " "$AUTOCONF_REQUIRED_VERSION"
if ($AUTOCONF --version) < /dev/null > /dev/null 2>&1; then
    VER=`$AUTOCONF --version | head -n 1 \
         | grep -iw autoconf | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf/"
    echo
    DIE=1;
fi


printf "checking for automake >= %s ... " "$AUTOMAKE_REQUIRED_VERSION"
if ($AUTOMAKE --version) < /dev/null > /dev/null 2>&1; then
    AUTOMAKE=$AUTOMAKE
    ACLOCAL=$ACLOCAL
else
    amfound=0
    for amver in 1.19 1.18 1.17 1.16 1.15 1.14 1.13 1.12 1.11; do
        if (automake-$amver --version) < /dev/null > /dev/null 2>&1; then
            AUTOMAKE=automake-$amver
            ACLOCAL=aclocal-$amver
            amfound=1
            break
        fi
    done
    if expr $amfound = 0 > /dev/null; then
        echo
        echo "  You must have automake $AUTOMAKE_REQUIRED_VERSION or newer installed to compile $PROJECT."
        echo "  Download the appropriate package for your distribution,"
        echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/automake/"
        echo
        DIE=1
    fi
fi
if test x$AUTOMAKE != x -a $DIE = 0; then
    VER=`$AUTOMAKE --version \
         | grep automake | sed "s/.* \([0-9.]\+\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
fi


if test "$DIE" -eq 1; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo
    exit 1
fi


test $TEST_TYPE $FILE || {
    echo
    echo "You must run this script in the top-level $PROJECT directory."
    echo
    exit 1
}


if test -z "$NOCONFIGURE"; then
    echo
    echo "I am going to run ./configure with the following arguments:"
    echo
    echo "  --enable-maintainer-mode --enable-install-man $AUTOGEN_CONFIGURE_ARGS $@"
    echo

    if test -z "$*"; then
        echo "If you wish to pass additional arguments, please specify them "
        echo "on the $0 command line or set the AUTOGEN_CONFIGURE_ARGS "
        echo "environment variable."
        echo
    fi
fi


rm -rf autom4te.cache

$ACLOCAL $ACLOCAL_FLAGS
RC=$?
if test $RC -ne 0; then
    echo "$ACLOCAL gave errors. Please fix the error conditions and try again."
    exit $RC
fi


# optionally feature autoheader
($AUTOHEADER --version)  < /dev/null > /dev/null 2>&1 && $AUTOHEADER || exit 1

$LIBTOOLIZE --force --copy --install || exit 1

$AUTOMAKE --add-missing --force --copy || exit $?
$AUTOCONF || exit $?

cd $ORIGDIR

if test -z "$NOCONFIGURE"; then
    $srcdir/configure --enable-maintainer-mode --enable-install-man $AUTOGEN_CONFIGURE_ARGS "$@"
    RC=$?
    if test $RC -ne 0; then
        echo
        echo "Configure failed or did not finish!"
        exit $RC
    fi

    echo
    echo "Now type 'make' to compile the $PROJECT."
fi

