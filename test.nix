{
  pkgs,
  nixos-lib,
  build-core,
  build-fake-rocket,
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
      { pkgs, ... }: let
        ground-station-core = build-core { inherit pkgs; use-sd-notify = true; };
      in
      {
        imports = [ baseModule ];
        environment.systemPackages = [
          ground-station-core
          pkgs.socat
        ];
        systemd.services.socat-link.script = ''
          ${pkgs.socat}/bin/socat PTY,link=/dev/ttyS10,raw,echo=0,B38400 PTY,link=/dev/ttyS11,raw,echo=0
        '';
        systemd.services.fake-rocket = {
          serviceConfig.ExecStart = ''
            ${build-fake-rocket pkgs}/bin/fake-rocket /dev/ttyS10 731875067
          '';
          requires = [ "socat-link.service" ];
          serviceConfig.Type = "oneshot";
        };
        systemd.services.ground-station-core = {
          serviceConfig.ExecStart = ''
            ${pkgs.valgrind}/bin/valgrind --leak-check=yes ${ground-station-core}/bin/ground-station-core /dev/ttyS11 5
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
        default.succeed("systemctl start fake-rocket")
        default.succeed("journalctl -eu ground-station-core | grep 'Attempting to process message of size 17'")
      '';
  };
}
