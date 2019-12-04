#! /bin/bash

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

PREFIX="$DIR/../toolchain"
SYSROOT="$DIR/../root"
TARGET=i686-elf

export PATH="$PREFIX/bin:$PATH"

BINUTILS="binutils-2.33.1"
BINUTILS_ARCHIVE="binutils-2.33.1.tar.gz"

GCC="gcc-9.2.0"
GCC_ARCHIVE="gcc-9.2.0.tar.gz"

NASM="nasm-2.14.02"
NASM_ARCHIVE="nasm-2.14.02.tar.gz"

echo "Building toolchain in $PREFIX"  

mkdir -p $PREFIX
cd $PREFIX

mkdir src
pushd src
	echo "Cloning sources in `pwd`"

	if [ ! -e $BINUTILS_ARCHIVE ]; then
		curl -O "https://ftp.gnu.org/gnu/binutils/$BINUTILS_ARCHIVE"
	fi
	if [ ! -e $GCC_ARCHIVE ]; then
		wget "https://ftp.gnu.org/gnu/gcc/$GCC/$GCC_ARCHIVE"
	fi
	if [ ! -e $NASM_ARCHIVE ]; then
		wget "https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/$NASM_ARCHIVE"
	fi

	rm -rf $BINUTILS
	tar -xf $BINUTILS_ARCHIVE

	rm -rf $GCC
	tar -xf $GCC_ARCHIVE

	# NASM will be built on the fly, in the src directory, due to issues with autoreconf.
	rm -rf $NASM
	tar -xf $NASM_ARCHIVE
popd

mkdir -p build/gcc build/binutils

pushd build
	unset PKG_CONFIG_LIBDIR # Just in case

	echo "Building sources in `pwd`"

	pushd binutils
		$PREFIX/src/$BINUTILS/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-nls --disable-werror > /dev/null
		make > /dev/null
		make install > /dev/null
		#rm -f "$PREFIX/src/$BINUTILS_ARCHIVE"
		rm -rf "$PREFIX/src/$BINUTILS"
	popd

	pushd gcc
		$PREFIX/src/$GCC/configure --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers #--with-sysroot="$SYSROOT"
		make all-gcc
		make all-target-libgcc
		make install-gcc
		make install-target-libgcc
		#rm -f "$PREFIX/src/$GCC_ARCHIVE"
		rm -rf "$PREFIX/src/$GCC"
	popd
popd

pushd src
	pushd $NASM
		sh $PREFIX/src/$NASM/autogen.sh
		$PREFIX/src/$NASM/configure --prefix="$PREFIX"
		make
		make install
		#rm -f "$PREFIX/src/$NASM_ARCHIVE"
		rm -rf "$PREFIX/src/$NASM"
	popd
popd
