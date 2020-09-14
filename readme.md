# GRR
The GRUB Replacing Rootkit. GRR is a "malicious" bare metal hypervisor built
atop AMD SVM. Please note that this software was written as a reasearch project
into hypervisor technology, and is not intended for, nor does it implement any
malicious functionality.

## Features
GRR is implemented as a UEFI runtime driver. It gets loaded before the OS, it
hooks the `exit_boot_services` function in the UEFI system table, and when the
OS calls said function the machine is brought under full hypervisor control.
It supports SMP startup using the xAPIC arcithecture bringing all cores/threads
under the hypervisor's control. The only actual hypervisor functionality
implemented is the replacment of the CPUID string `AuthenticAMD` with the string
`BootlegAMD`.

## Supported environment
KVM was used as the main development environment, but the current release of
VMWare is also confirmed to work. Debian 10 and Windows 10 LTSC 2019 are
confirmed to work. Currently there are criticals bugs preventing it from
booting on real hardware. To see debug output a UART must be present at
I/O 0x3f8.

## Building
GRR relies on the libefi development kit for UEFI support. This repository needs
to be cloned as a subdirectory inside the libefi tree then `make` can be used
to build an image. Please note that libefi relies on a mingw-w64 targeting
copy of the GNU toolchain to be installed on the build machine.

## Copying
GRR is being released under the terms of the ISC license, for more details see
`license.txt`.
