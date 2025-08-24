# Phantom OS Genode port

This repository contains a port of Phantom OS to Genode OS framework.

## Intro

It is a new effort of developing Phantom OS. Porting Phantom OS to Genode should make it more stable, secure, portable. The main goal of this project is to make Phantom OS suitable for real-life applications and future research.

Phantom OS is an `orthogonally persistent` `managed code` general-purpose operating system.
- `Orthogonally persistent`. In short, it implies that all data objects in the system are stored in a big virtual memory the content of which is not erased between restarts (even unexpected ones).
- `Managed code`. In our case, all userspace programs are executed in bytecode inside a language virtual machine. 

Orthogonal persistence is an old concept and research on this topic almost stopped in the early 2000s. We believe that with the modern hardware (and, possibly, future hardware like Intel Optane) this concept might become relevant again. With this project, we would like to provide a foundation for testing the OP hypothesis in the modern environment. Moreover, we believe that a persistent operating system based on microkernels together with managed code userspace might become a platform suitable for industrial applications and might attract the attention of researchers and developers.

Potentially, this operating system can be used in a wide range of use cases. However, currently, we are focusing on IoT so that embedded programmers will be able to focus more on business logic.

> This section is lacking details intentionally to make it simpler. If you want to learn more about orthogonal persistence and our hypotheses, the corresponding paper will be published soon!

> Also if you want to learn more about Phantom OS refer to its official documentation https://phantomdox.readthedocs.io/en/latest/

## Current status

What is done:

- [x] Phantom Virtual Machine is able to work on Genode in 64 bits
- [x] The subset of kernel sources required to run PVM in a persistent environment
- [x] Adapters from Genode to Phantom OS low-level interfaces are implemented

What is in progress:

- [ ] The debugging and testing of adapters is in progress

What are the future plans:

- [ ] Finish the port
  - [ ] Make PVM work in persistent environment
  - [ ] Rework driver system
  - [ ] Rework and enable networking
  - [ ] Rework and enable graphics
- [ ] Add support to more languages using a WebAssembly runtime

### Project layout

For now, this repository contains a lot of unused old code since the porting process is still in progress. Directories containing sources that are actually used are:

- `src/include` - contains Phantom OS headers
- `src/include/genode-amd64` - contains architecture specific headers
- `src/include/genode-libc` - contains Genode's libc headers
- `src/phantom/isomem` - the set of sources from the old kernel that implement persistent memory layer
  - `src/phantom/isomem/genode_*.cc` - source files containing stubs for interfaces that should be implemented in Genode. Some of them are disabled using conditional compilation with `PHANTOM_THREADS_STUB` macro
- `src/genode_env` - adapters to Genode that are implementing Phantom OS low-level interfaces
- `src/phantom/vm` - Phantom Virtual Machine

and PVM dependencies (graphics subsystem):

- `src/phantom/libfreetype`
- `src/phantom/gl`
- `src/phantom/libwin`

> The detailed description of the adapters will be added soon

## How to contribute?

This project requires both research and development. Several ideas for research topics and development tasks are described in this section. If you are interested in any of them, feel free to contact us.

### Research

During this work we identified several interesting research topics:

- *Persistent errors and how to deal with them*. Since every object is allowed to live as long as it wants to, errors within them will live as well. The best case is when we can restore from one of the previous snapshots, however, we might encounter the situation when this fault is recurring. 
- *Connection between persistent and transient (non-persistent) worlds*. One of the main OP issues is that it is some sort of "closed" system where communications to other systems and even hardware devices should not affect the consistency of the persistent state.
- *Distributed persistent space*. The idea is to connect several instances of Phantom OS that would share a single global address space. In this scenario mechanisms for accessing, migrating objects should be researched as well as situations when some of the peers go offline.
- *Formal verification*. In a persistent system, the model of accessing data as well as the model of communication between other devices or systems are different. All data is assumed to be located in single-level persistent memory space and communication should be performed using special interfaces or protocols that would minimize side effects. We assume that those two factors might make formal verification of the programs easier.
- *Real-time*. We assume that the concept of Phantom OS is capable to be at least soft real-time. However, we have not researched this topic in-depth yet.

### Development

We assume that several tasks mentioned earlier as our plans can be done in parallel:

- Graphics system
- Driver system
- Networking

Also, if you want to participate in any of the current tasks feel free to contact us!

## Setting up environment to build and run Phantom OS

Here are the instructions you should follow before proceeding to building and running Phantom OS.

### Cloning required repositories

```bash
git clone -b dev --recurse-submodules https://github.com/Phantom-OS/phantomuserland

cd phantomuserland

# Repository with Genode build container wrapper
git clone https://github.com/skalk/genode-devel-docker
# Repostory with Genode
git clone https://github.com/genodelabs/genode
# Setting up with goa - tool for building genode applications
git clone https://github.com/genodelabs/goa
# Setting up Snapper
git clone --depth 1 https://github.com/rumenmitov/snapper genode/repos/snapper
```

### Genode development container

We use genode development container to use genode build toolchain. You first have to create or import docker image:

> Note: you can omit `SUDO=sudo` if your docker is set up to work without root priviliges

```bash
cd genode-devel-docker

# Recommended: import exising docker image
./docker import SUDO=sudo

# Alternative: build the image on your machine
./docker build SUDO=sudo
```

#### Starting the container

Start the genode development container and return to the working directory:

```bash
./docker run SUDO=sudo DOCKER_CONTAINER_ARGS=" --network host "

cd <PATH_TO_PHANTOMUSERLAND>

# You should add goa to $PATH every time you restart the conatiner
export PATH=$PATH:$(pwd)/goa/bin
```

> Note: you can also add goa to your `PATH` permanently (i.e. in `.bashrc`), in which case you won't need to manually add it every time container is started.

> `--network host` is optional, but it helps to avoid the process of setting up the network for debugging. You can omit `DOCKER_CONTAINER_ARGS=" --network host "` if you won't use debuggers.

#### Alternative Container Setup
For a container setup with less steps, run the following:  

```bash
cd <PATH_TO_PHANTOMUSERLAND>

docker run -d --name phantomuserland --replace --cap-add SYS_PTRACE -v .:/phantomuserland -w /phantomuserland rmitov/genode:latest tail -f /dev/null

docker exec phantomuserland sed -i "s|/genode/goa|/phantomuserland/goa|g" /root/.bashrc
```

### Initial setup

This section contains commands that would prepare the environment to build Phantom OS. 

> Following commands should be executed inside the container!

```bash
cd ./genode/

# Switch to supported version
git checkout 25.05

# Creating build directory
./tool/create_builddir x86_64

# Preparing ports required for building the system 
./tool/ports/prepare_port jitterentropy linux x86emu grub2

# Enable optional repositories in build configuration
sed -i 's/#REPOSITORIES/REPOSITORIES/g' build/x86_64/etc/build.conf
# Remove `-no-kvm` qemu option
sed -i '/QEMU_OPT += -no-kvm/d' build/x86_64/etc/build.conf

# Enable Snapper
echo "REPOSITORIES += \$(GENODE_DIR)/repos/snapper" >> build/x86_64/etc/build.conf

# go back
cd ../

# Creating soft link required to assemble system image
mkdir -p genode/build/x86_64/bin
# what is it for again?
ln -s $(pwd)/raw/phantom_bins.tar genode/build/x86_64/bin
# softlink main phantom binary
ln -s $(pwd)/var/build/x86_64/isomem genode/build/x86_64/bin/isomem
# softlink run recepies
ln -s $(pwd)/run/* genode/repos/ports/run/
# softlink phantom classes 
ln -s $(pwd)/src/plib/bin/classes genode/build/x86_64/bin/phantom_classes

# Create a 2 GB empty disk image
dd if=/dev/zero of=empty_disk.raw bs=1M count=0 seek=2048
dd conv=notrunc bs=4096 count=1 seek=16 if=src/run/img/phantom.superblock of=empty_disk.raw

# Copy disk image to bin/
cp empty_disk.raw genode/build/x86_64/bin/block0.raw
```

> Note: executing command `cp empty_disk.raw genode/build/x86_64/bin/block0.raw` again will result in erasing contents of disk used by emulated Phantom OS. You can use it to reset the system state. 

## Building & running Phantom OS

Once you completed the instructions in previous section, you should be ready to build and run Phantom OS. 

### Build main binary

> Before building Phantom OS, make sure that you are in the genode development container, and that `goa` is added to the `$PATH` (see **Starting the container** section)

Running the following command from `phantomuserland` directory builds main phantom binary:

`goa build`

> Note: As a part of build process this command will create `var` subdirectory, which holds all the build files, including CMake-related files containing build configuration.

#### Build options

When building phantom you may want to configure some of the options:

---

`PHANTOM_BUILD_NO_DISPLAY`:
* 0 to enable graphics, 1 to disable. Default: 0

`PHANTOM_BUILD_TESTS_ONLY`:
* 0 to build phantom kernel, 1 to build only kernel tests. Default: 0

---

You can specify build options by setting environment variables passed to goa, e.g.:

`PHANTOM_BUILD_NO_DISPLAY=1 goa build` 

This command will build phantom with graphics disabled. Note that you have to specify your custom build options every time you issue `goa build` command. Alternatively, run the following command (note: you should run `goa build` at least once before using the following command, in order to create CMake build directory):

`cmake -D<build-option-name>=<value> var/build/x86_64`

e.g.:

`cmake -DPHANTOM_BUILD_NO_DISPLAY=1 var/build/x86_64`

This command will permanently change build configuration. In this particular case it configures to build phantom with graphics disabled, i.e. when you run `goa build`, phantom will be built without graphics.


### Running Phantom OS

After you have built the main phantom binary, run the following command in order to build system image and run phantom using QEMU with graphics enabled:

> Note: running this command might fail with message: `Error: missing depot archives`. In this case execute the command suggested below the error message to create the required archives.

`make -C genode/build/x86_64/ KERNEL=hw BOARD=pc run/phantom`

If you have disabled graphics with `PHANTOM_BUILD_NO_DISPLAY=1`, use the following command instead:

`make -C genode/build/x86_64/ KERNEL=hw BOARD=pc run/phantom_no_video`

## Contacts

a.antonov@innopolis.ru - Anton Antonov, author of the port \
k.samburskiy@innopolis.university - Kirill Samburskiy, author of WAMR port \
dz@dz.ru - Dmitry Zavalishin, author of Phantom OS
