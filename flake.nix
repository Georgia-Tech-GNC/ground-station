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
          #boot.initrd.includeDefaultModules = false;
          #boot.initrd.kernelModules = [ "ext4" ];
          disabledModules = [
            "${modulesPath}/profiles/all-hardware.nix"
            "${modulesPath}/profiles/base.nix"
          ];
          environment.systemPackages = with pkgs; [
            # re-add packages in base.nix
            neovim
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
          environment.systemPackages = [ ground-station-core ] ++ (with pkgs; [ picocom gdb ]);
          systemd.services.ground-station-core = {
            description = "XBee-Pi comms server for GNC";
            serviceConfig.ExecStart = "${ground-station-core}/bin/ground-station-core /dev/ttyS1";
            wantedBy = [ "multi-user.target" ];
            enable = true;
          };
        };
      build-core =
        pkgs:
        pkgs.stdenv.mkDerivation {
          name = "ground-station-core";
          src = ./ground-station-core;
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
        nixosConfigurations.trixie = build-trixie {
          pkgs = pkgs-aarch64-multiplatform;
          ground-station-core = packages.ground-station-core-aarch64-multiplatform;
        };
        images.trixie = nixosConfigurations.trixie.config.system.build.sdImage;
      }
    );
}
