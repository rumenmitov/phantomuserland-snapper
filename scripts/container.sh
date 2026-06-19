#!/usr/bin/env bash

#
# NOTE
# If you encounter "failed to open display" when running PhantomOS,
# try: `xhost +local:`.


podman run -d \
       --name phantomuserland \
       --replace \
       --publish 5555:5555 \
       -v $XDG_RUNTIME_DIR:$XDG_RUNTIME_DIR:rw \
       -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
       -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
       --device /dev/kvm \
       --privileged \
       --cap-add SYS_PTRACE \
       --user root \
       --group-add keep-groups \
       -v .:/phantomuserland \
       -w /phantomuserland \
       docker.io/rmitov/genode:25.05 \
       tail -f /dev/null

