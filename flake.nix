{
  description = "A Nexus addon to store and copy/send recurring chat messages";
  inputs = {
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    self.submodules = true;
  };

  outputs =
    {
      flake-utils,
      nixpkgs,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          crossSystem = {
            config = "x86_64-w64-mingw32";
          };
        };
      in
      {
        packages.default = pkgs.callPackage ./package.nix { };
        devShells.default = pkgs.callPackage ./shell.nix { };
      }
    );
}
