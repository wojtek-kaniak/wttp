let
	unstable = import (fetchTarball https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz) { };
in
{ nixpkgs ? import <nixpkgs> {} }:
with nixpkgs;
(mkShell.override { stdenv = unstable.llvmPackages_18.stdenv; }) {
	nativeBuildInputs = [
		unstable.llvmPackages_18.bintools
		pkg-config
		cmake

		# clangd
		unstable.clang-tools_18
		gdb
	];

	buildInputs = [
		raylib
		gtk3
		glib
		dbus.dev
	];

	shellHook =
		''
		export LD="lld"
		export XDG_DATA_DIRS=$GSETTINGS_SCHEMAS_PATH
		'';
	
	# Breaks debug (non-optimized) builds
	hardeningDisable = [ "fortify" ];
}
