Raspberry Pi 4 platform config:
`aarch64-unknown-linux-gnu`

To build the image:
```sh
nix build .#packages.aarch64-linux.ground-station-core -L -j1 --cores 4
```
Flags:
- `-L` `--print-build-logs` print build logs
- `-j1` one derivation built at a time
- `--cores 4` one derivation can use 4 cores
