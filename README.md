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
cd ground-station-core
CROSS=aarch64-linux-gnu make
```
