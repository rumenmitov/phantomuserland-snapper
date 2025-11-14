#!/bin/bash

port=$(pgrep -fx "\[Genode\] init -> isomem")
gdb -p $port \
    -ex "set pagination off" \
    -ex "set interactive-mode off" \
    -ex "symbol-file /phantomuserland/var/build/x86_64/isomem.debug" \
    -ex "add-symbol-file /phantomuserland/genode/build/x86_64/debug/ld-linux.lib.so" \
    -ex "b pager_refill_free_reserve" \
    -ex "c &"
