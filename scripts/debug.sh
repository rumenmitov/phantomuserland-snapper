#!/bin/sh

# NOTE If using gdb inside EMACS set the environment variable: EMACS=-i=mi.

/usr/local/genode/tool/current/bin/genode-x86-gdb $EMACS /phantomuserland/genode/build/x86_64/debug/ld-hw.lib.so \
                                                  -ex "set pagination off" \
                                                  -ex "set interactive-mode off" \
                                                  -ex "set non-stop on" \
                                                  -ex "symbol-file /phantomuserland/var/build/x86_64/isomem.debug" \
                                                  -ex "add-symbol-file /phantomuserland/genode/build/x86_64/debug/ld-hw.lib.so" \
                                                  -ex "target extended-remote localhost:5555" \
                                                  -ex "b binary_ready_hook_for_gdb" \
                                                  -ex "c" \
                                                  -ex "d 1" \
                                                  -ex "thread 2"
