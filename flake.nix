{
  description = "A basic flake";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }: (
    flake-utils.lib.eachDefaultSystem
    (system: let
      pkgs = import nixpkgs {
        inherit system;

        config = {
          allowUnfree = true;
        };
      };
    in {
      # packages.default = { };
      devShells.default =
        pkgs.mkShell
        {
          name = "env-shell";

          nativeBuildInputs = with pkgs; [
            raylib
          ];
        };
    })
  );
}
