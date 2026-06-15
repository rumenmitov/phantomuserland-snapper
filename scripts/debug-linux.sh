#!/bin/bash

# NOTE If using gdb inside EMACS set the environment variable: EMACS=-i=mi.

port=$(pgrep -fx "\[Genode\] init -> isomem")
gdb $EMACS -p $port \
    -ex "set pagination off" \
    -ex "set interactive-mode off" \
    -ex "symbol-file /phantomuserland/var/build/x86_64/isomem.debug" \
    -ex "add-symbol-file /phantomuserland/genode/build/x86_64/debug/ld-linux.lib.so" \
    -ex "set substitute-path /depot/genodelabs /phantomuserland/var/depot/genodelabs" \
    -ex "b openat" \
    -ex "c &"
