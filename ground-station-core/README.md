# ground-station-core

ground-station-core provides an HTTP API to the rocket through an XBee radio link.

## Setup Cross Compilation on Debian (for Raspberry Pi 64-bit)

```sh
# dpkg --add-architecture arm64
# apt update
# apt install build-essential crossbuild-essential-arm64
# apt install install libsystemd-dev:arm64
```


## Development

Shell environment (with Python, etc):
`nix develop`

```sh
# run an interactive test with a virtual machine
nix build -L .#checks.x86_64-linux.<test-name>.driverInteractive && ./result/bin/nixos-test-driver
```

## Building

To cross-compile for Raspberry Pi (64-bit):
```sh
USE_SD_NOTIFY=yes CROSS=aarch64-linux-gnu make
```
