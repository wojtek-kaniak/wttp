let
	unstable = import (fetchTarball https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz) { };
in
{ nixpkgs ? import <nixpkgs> {} }:
with nixpkgs;
(mkShell.override { stdenv = unstable.llvmPackages_18.stdenv; }) {
	nativeBuildInputs = [
		unstable.llvmPackages_18.bintools
		cmake
		# clangd
		unstable.clang-tools_18
	];

	shellHook =
		''
		export LD="lld"
		'';
}
