{
  description = "Flake configuration";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  } @ inputs: let
    system_outputs = system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      formatter = pkgs.alejandra;
      devShells = {
        default = pkgs.mkShell {
          name = "ftditool env";
          nativeBuildInputs = with pkgs; [
            clang_18
            llvmPackages_18.llvm
            cmake
            gnumake
            gdb
          ];
        };
      };
    };
  in
    flake-utils.lib.eachDefaultSystem system_outputs;
}
