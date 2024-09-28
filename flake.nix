{
    description = "Flake configuration";

    inputs = {
         nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";
    };

    outputs = {self, nixpkgs, ...}:{
        devShells.x86_64-linux.default = (import ./shell.nix {pkgs=nixpkgs.legacyPackages."x86_64-linux";} );
    };
}
