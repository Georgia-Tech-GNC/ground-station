{
  pkgs,
  nixos-lib,
  ground-station-core,
  ...
}:
let
  baseModule =
    { ... }:
    {
      services.getty.autologinUser = "root";
      services.logrotate.enable = false; # clogs up the logs
    };
in
{
  sanity = nixos-lib.runTest {
    name = "sanity";
    hostPkgs = pkgs;
    nodes.default =
      { pkgs, ... }:
      {
        imports = [ baseModule ];
        environment.systemPackages = [
          ground-station-core
          pkgs.socat
        ];
        systemd.services.socat-link.script = ''
          socat PTY,link=/dev/ttyS10,raw,echo=0 PTY,link=/dev/ttyS11,raw,echo=0
        '';
      };
    testScript =
      { ... }:
      ''
        start_all()

        default.fail("ground-station-core")
        default.fail("ground-station-core /dev/notexist")
        default.fail("ground-station-core /dev/notexist 5")
        default.succeed("systemctl start socat-link")
        #default.succeed("ground-station-core /dev/ttyS10 5")
      '';
  };
}
