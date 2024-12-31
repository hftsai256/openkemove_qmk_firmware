{
  description = "FPGA Manager flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
        };
      in
      {
        devShells.default = pkgs.callPackage ({ mkShell, pkg-config, ... }:
          mkShell {
            buildInputs = with pkgs; [
              gcc-arm-embedded
              gnumake
              dfu-util
            ];
          }
        ) {};
      }
    );
}
