{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    devkitnix = {
      url = "github:NekoNoor/devkitnix/devkitarm-20180522";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
  };

  outputs = {
    self,
    nixpkgs,
    devkitnix,
    flake-compat,
  }: let
    pkgs = import nixpkgs {system = "x86_64-linux";};
    devkitARM = devkitnix.packages.x86_64-linux.devkitARM;
    libc = pkgs.writeText "libc.txt" ''
      include_dir=${devkitARM}/devkitARM/arm-none-eabi/include
      sys_include_dir=${devkitARM}/devkitARM/arm-none-eabi/include
      crt_dir=${devkitARM}/devkitARM/lib
      msvc_lib_dir=
      kernel32_lib_dir=
      gcc_dir=
    '';
  in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = [
        devkitARM
      ];
      nativeBuildInputs = [
        (pkgs.callPackage ./firmtool.nix {})
      ];
      shellHook = ''
        export LIBC=${libc}
        export DEVKITPRO=${devkitARM}
        export DEVKITARM=${devkitARM}/devkitARM
        export PATH=$PATH:$DEVKITPRO/tools/bin
      '';
    };
  };
}
