# GRR
The GRUB Replacing Rootkit. GRR is a "malicious" bare metal hypervisor built
atop AMD SVM. Please note that this software was written as a reasearch project
into hypervisor technology, and is not intended for, nor does it implement any
malicious functionality.

## Features
As the name suggests, it replaces GRUB as bootloader and loads Linux itself
from disk then boots it up as a virtual machine under the hypervisor's control.
It supports SMP startup using the xAPIC arcithecture bringing all cores/threads
under the hypervisor's control. The only actual hypervisor functionality
implemented is the replacment of the CPUID string `AuthenticAMD` with the string
`BootlegAMD`.

## Supported environment
KVM was used as the main development environment, but the current release of
VMWare is also confirmed to work. The MMIO emulation used for SMP bootup,
as implemented only supports decoding a very limited set of instrutions needed
to boot Debian with the 4.19.0-10-amd64 kernel build. Currently there are
criticals bugs preventing it from booting on real hardware. To see debug output
a UART must be present at I/O 0x3f8. The kernel and initrd path is currently
hard coded inside `main.c`.

## Building
GRR relies on the libefi development kit for UEFI support. This repository needs
to be cloned as a subdirectory inside the libefi tree then `make` can be used
to build an image. Please note that libefi relies on a mingw-w64 targeting
copy of the GNU toolchain to be installed on the build machine.
NOTE for driver: change the subsystem id in Makefile.efi in the libefi tree to
12

## Copying
GRR is being released under the terms of the ISC license, for more details see
`license.txt`.
