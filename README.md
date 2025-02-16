# Ground Station V2

## Organization

### Threads
- Main thread: update SQL task queue
- Serial thread: read serial input
- Web thread: host the web socket`

## Development (ground-station-core)

Shell environment (with Python, etc):
`nix develop`

Shell for cross-compiling (only when running gcc et al yourself):
`nix develop .#devShells.x86_64-linux.cross`

```bash
# run an interactive test with a virtual machine
nix build -L .#checks.x86_64-linux.<test-name>.driverInteractive && ./result/bin/nixos-test-driver
```

## Building (ground-station-core)

To cross-compile for Raspberry Pi (64-bit):
```sh
apt install build-essential crossbuild-essential-arm64 # Debian

cd ground-station-core
CROSS=aarch64-linux-gnu make
```
