#! /bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

PREFIX="$DIR/../toolchain"

export PATH="$PREFIX/bin:$PATH"
echo "$PATH"