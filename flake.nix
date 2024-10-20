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
      # to work with older version of flakes
      lastModifiedDate = self.lastModifiedDate or self.lastModified or "19700101";

      # Generate a user-friendly version number.
      version =
        (builtins.substring 0 8 lastModifiedDate) + "-" + (if (self ? rev) then self.rev else "dirty");

      # System types to support.
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      # Helper function to generate an attrset '{ x86_64-linux = f "x86_64-linux"; ... }'.
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

      # Nixpkgs instantiated for supported system 
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });
      libFor = forAllSystems (system: import (nixpkgs + "/lib"));
      nixosLibFor = forAllSystems (system: import (nixpkgs + "/nixos/lib"));
    in
    flake-utils.lib.eachSystem supportedSystems (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        lib = import (nixpkgs + "/lib") { inherit system; };
        nixosLib = import (nixpkgs + "/nixos/lib") { inherit system; };
        ldflags = pkgs: [ "-race" ];
        baseModule = {
          imports = [ "${nixpkgs}/nixos/modules/installer/sd-card/sd-image-aarch64.nix" ];
          sdImage.compressImage = false;
          time.timeZone = "America/New_York";
          i18n.defaultLocale = "en_US.UTF-8";
          system.stateVersion = "24.05";

          nix.settings.experimental-features = [
            "nix-command"
            "flakes"
          ];
          nix.settings.auto-optimise-store = true;
          services.openssh.enable = true;
          services.fail2ban.enable = true;

          users.users.gnc = {
            isNormalUser = true;
            description = "GNC";
            extraGroups = [ "wheel" ];
            shell = pkgs.fish;
            openssh.authorizedKeys.keys = [
              "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAINEhH+5s0m+lBC898M/nrWREaDblRCPSpL6+9wkoZdel inaba@nyiyui.ca"
            ];
            homeMode = "770";
          };
          security.sudo.wheelNeedsPassword = false;
          environment.shells = [ pkgs.fish ];
          programs.git.enable = true;
          programs.tmux.enable = true;
          programs.htop.enable = true;
        };
      in
      rec {
        devShells = {
          default = pkgs.mkShell {
            packages = with pkgs; [
              gcc
              gnumake
              stlink
              stlink-gui
              go
              gdb
              nixfmt-rfc-style
              (python3.withPackages (p: with p; [
                jinja2
                requests
              ]))
            ];
            buildInputs = with pkgs; [ ];
            nativeBuildInputs = with pkgs; [ pkg-config ];
          };
        };
        packages.ground-station-core = pkgs.stdenv.mkDerivation {
          name = "ground-station-core";
          src = ./ground-station-core;
          inherit system;
        };
        nixosConfigurations.trixie = nixpkgs.lib.nixosSystem {
          system = "aarch64-linux";
          modules = [
            baseModule
            ({ ... }: {
              environment.systemPackages = [
                packages.ground-station-core
              ];
              systemd.services.ground-station-core = {
                description = "XBee-Pi comms server for GNC";
                serviceConfig.ExecStart = "${packages.ground-station-core}/bin/ground-station-core";
                wantedBy = [ "multi-user.target" ];
                enable = true;
              };
            })
          ];
        };
        images.trixie = nixosConfigurations.trixie.config.system.build.sdImage;
      }
    );
}
