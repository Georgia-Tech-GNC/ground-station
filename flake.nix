{
  inputs.nixpkgs.url = "nixpkgs/nixos-24.05";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    let
      baseModule =
        { pkgs, modulesPath, ... }:
        {
          imports = [
            "${modulesPath}/installer/sd-card/sd-image-aarch64.nix"
            "${modulesPath}/profiles/headless.nix"
            "${modulesPath}/profiles/minimal.nix"
          ];
          boot.initrd.includeDefaultModules = false;
          boot.initrd.kernelModules = [ "ext4" ];
          disabledModules = [
            "${modulesPath}/profiles/all-hardware.nix"
            "${modulesPath}/profiles/base.nix"
          ];
          hardware.bluetooth.enable = false;
          boot.blacklistedKernelModules = [ "bluetooth" ];
          sound.enable = false;
          environment.systemPackages = with pkgs; [
            # re-add packages in base.nix
            vim
            usbutils
          ];
          sdImage.compressImage = false;
          time.timeZone = "America/New_York";
          i18n.defaultLocale = "en_US.UTF-8";
          system.stateVersion = "24.05";
          nix.settings.experimental-features = [
            "nix-command"
            "flakes"
          ];

          networking.hostName = "trixie";

          services.openssh.enable = true;
          programs.fish.enable = true;

          users.users.gnc = {
            isNormalUser = true;
            description = "GNC";
            extraGroups = [ "wheel" ];
            shell = pkgs.fish;
            openssh.authorizedKeys.keys = [
              "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAINEhH+5s0m+lBC898M/nrWREaDblRCPSpL6+9wkoZdel inaba@nyiyui.ca"
              "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQDBvcNQPc5UriJhzx+ZEkD2rb4iT3MlAzUC72LNG3E4t4FjOyQzkOtcJIORdk+ACuOJtliL8pIGIJQHBgvNhhFE/khuSIMQ/UYh1dN3DOmNCSveJlJtEVGt+PHDgAALLoXOUWbxqWy/XJFSezYOExlUBddBzvEiuvqsA4u9LcG/bWMidwcbsHI4xmzMHzVJ8Ym8d+5+g8jq+VKtDSDQr6P0aDacQqMv01zPslV94Ry589R3VWtmAY50rD8gQShGDmfDiuOdccF0HmjYi7mm8+PitTzk9cSUC4o7XPa7Atw7DKIgM6pdqSiYZ8COoijsmT48rzQMjo5euCdUnoOPCtWfwoy1EU+oZ6pa8/rT0vlOseAyLxfWt6yq0c4kuyHA8X9tChNzNTabPuavR9bUr1hRulm3TIrirWcAVXp8ZqwufM0c4rtCpVM7+QdpmGN0CC+PqozOOGnMnKMSjpktqkkItPfGmmBV/DjjntLPVK3WbrySMYbuQx3jYgKXdl8bNqE= vparikh@lawn-128-61-12-17.lawn.gatech.edu"
              "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOKWY/K+F8u5GVb0hJ9oi1cCfyxXthYSLSnIdWjBjtjH kanav@lawn-128-61-66-45.lawn.gatech.edu"
            ];
            homeMode = "770";
            hashedPassword = "$y$j9T$pxKDYuGo9ERES1E38/xc00$5IDCOwo.uYH6KiKabibXvzrIkpqbllddU7S0iOGCK/2";
          };
          security.sudo.wheelNeedsPassword = false;
          environment.shells = [ pkgs.fish ];
          programs.git.enable = true;
          programs.tmux.enable = true;
          programs.htop.enable = true;
        };
      devModule = {
        options = { };
        config = {
          nix.settings.trusted-users = [ "gnc" ];
          nix.settings.auto-optimise-store = true;
        };
      };
      coreModule =
        { pkgs, ground-station-core }:
        { ... }:
        {
          environment.systemPackages = [ ground-station-core ] ++ (with pkgs; [ picocom sqlite gdb ]);
          systemd.services.ground-station-core = {
            description = "XBee-Pi comms server for GNC";
            serviceConfig.ExecStart = "${ground-station-core}/bin/ground-station-core /dev/ttyS1 5";
            # 5 = 38400 baud
            # cf. https://scrapbox.io/nyiyui/XBee:_change_baud_rate#6731396ca5fc2b00008a491d
            wantedBy = [ "multi-user.target" ];
            enable = true;
          };
        };
      build-core =
        pkgs:
        pkgs.stdenv.mkDerivation {
          name = "ground-station-core";
          src = ./ground-station-core;
          dontStrip = true;
        };
      build-trixie =
        { pkgs, ground-station-core }:
        pkgs.nixos {
          imports = [
            baseModule
            devModule
            (coreModule { inherit pkgs ground-station-core; })
          ];
        };
      build-shell =
        pkgs:
        pkgs.mkShell {
          packages = with pkgs; [
            gcc
            gnumake
            go
            gdb
            nixfmt-rfc-style
            (python3.withPackages (
              p: with p; [
                jinja2
                requests
              ]
            ))
          ];
          buildInputs = with pkgs; [ ];
          nativeBuildInputs = with pkgs; [ pkg-config ];
        };
      supportedSystems = [ "x86_64-linux" ];
    in
    flake-utils.lib.eachSystem supportedSystems (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        pkgsCross = pkgs.pkgsCross.aarch64-multiplatform;
        pkgs-aarch64-multiplatform = if system == "aarch64-linux" then pkgs else pkgsCross;
      in
      rec {
        devShells = {
          default = build-shell pkgs;
          cross = build-shell pkgsCross;
        };
        packages.ground-station-core = build-core pkgs;
        packages.ground-station-core-aarch64-multiplatform = build-core pkgs-aarch64-multiplatform;
        packages.nixosConfigurations.trixie = build-trixie {
          pkgs = pkgs-aarch64-multiplatform;
          ground-station-core = packages.ground-station-core-aarch64-multiplatform;
        };
        images.trixie = packages.nixosConfigurations.trixie.config.system.build.sdImage;
      }
    );
}
