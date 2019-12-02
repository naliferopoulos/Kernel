#! /bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

PREFIX="$DIR/../toolchain"
SYSROOT="$DIR/../root"
TARGET=i686-elf
BINUTILS="binutils-2.32"
BINUTILS_ARCHIVE="binutils-2.32.tar.gz"
GCC="gcc-8.3.0"
GCC_ARCHIVE="gcc-8.3.0.tar.gz"
NASM="nasm-2.14.02"
NASM_ARCHIVE="nasm-2.14.02.tar.gz"

mkdir -p $PREFIX
cd $PREFIX

mkdir src
pushd src

	if [ ! -e $BINUTILS_ARCHIVE ]; then
		wget "https://ftp.gnu.org/gnu/binutils/$BINUTILS_ARCHIVE"
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

	rm -rf $NASM
	tar -xf $NASM_ARCHIVE

popd

mkdir -p build/gcc build/binutils build/nasm

pushd build
	pushd binutils
		$PREFIX/src/$BINUTILS/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-nls --disable-werror
		make
		make install
		rm -f "$PREFIX/src/$BINUTILS_ARCHIVE"
		rm -rf "$PREFIX/src/$BINUTILS"
	popd

	pushd gcc
		$PREFIX/src/$GCC/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-nls --enable-languages=c --without-headers
		make all-gcc
		make all-target-libgcc
		make install-gcc
		make install-target-libgcc
		rm -f "$PREFIX/src/$GCC_ARCHIVE"
		rm -rf "$PREFIX/src/$GCC"
	popd

	pushd nasm
		$PREFIX/src/$NASM/autogen.sh
		$PREFIX/src/$NASM/configure --prefix="$PREFIX"
		make
		make install
		rm -f "$PREFIX/src/$NASM_ARCHIVE"
		rm -rf "$PREFIX/src/$NASM"
	popd
popd
