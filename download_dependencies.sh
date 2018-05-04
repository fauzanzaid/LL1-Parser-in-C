#!/bin/bash

rm -rf ext

export GIT_TERMINAL_PROMPT=0

echo -n 'Downloading BitSet ' ;		git clone -b v0.1.0 --depth=1 https://github.com/fauzanzaid/BitSet-in-C ./ext/BitSet			&> /dev/null && { echo 'done' ; } || { echo 'failed' ; exit 1; }
echo -n 'Downloading HashTable ' ;	git clone -b v0.1.0 --depth=1 https://github.com/fauzanzaid/Hash-Table-in-C ./ext/HashTable		&> /dev/null && { echo 'done' ; } || { echo 'failed' ; exit 1; }
echo -n 'Downloading LinkedList ' ;	git clone -b v0.1.0 --depth=1 https://github.com/fauzanzaid/Linked-List-in-C ./ext/LinkedList		&> /dev/null && { echo 'done' ; } || { echo 'failed' ; exit 1; }
echo -n 'Downloading Token ' ;		git clone -b v0.1.0 --depth=1 https://github.com/fauzanzaid/Compiler-Token-in-C ./ext/Token		&> /dev/null && { echo 'done' ; } || { echo 'failed' ; exit 1; }
echo -n 'Downloading ParseTree ' ;	git clone -b v0.1.0 --depth=1 https://github.com/fauzanzaid/Parse-Tree-in-C ./ext/ParseTree		&> /dev/null && { echo 'done' ; } || { echo 'failed' ; exit 1; }
