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
          ${pkgs.socat}/bin/socat PTY,link=/dev/ttyS10,raw,echo=0,B38400 PTY,link=/dev/ttyS11,raw,echo=0
        '';
        systemd.services.ground-station-core = {
          serviceConfig.ExecStart = ''
            ${ground-station-core}/bin/ground-station-core /dev/ttyS10 5
          '';
          requires = [ "socat-link.service" ];
          serviceConfig.Type = "notify";
        };
      };
    testScript =
      { ... }:
      ''
        start_all()

        default.fail("ground-station-core")
        default.fail("ground-station-core /dev/notexist")
        default.fail("ground-station-core /dev/notexist 5")
        default.succeed("systemctl start socat-link")
        default.succeed("systemctl start ground-station-core")
      '';
  };
}
