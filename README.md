## Development (ground-station-core)

Shell environment (with Python, etc):
`nix develop`

Shell for cross-compiling (only when running gcc et al yourself):
`nix develop .#devShells.x86_64-linux.cross`

## Building (ground-station-core)

Raspberry Pi 4 platform config:
`aarch64-unknown-linux-gnu`

To build ground-station-core (binary only):
```sh
nix build .#packages.aarch64-linux.ground-station-core -L -j1 --cores 4
```
Flags:
- `-L` `--print-build-logs` print build logs
- `-j1` one derivation built at a time
- `--cores 4` one derivation can use 4 cores

To build the image:
```sh
nix build .#images.x86_64-linux.trixie # aarch64-linux support coming soon
```
