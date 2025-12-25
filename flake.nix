{
  description = "Flake to manage hsnow dependencies";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem
    (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        libs = with pkgs; [
          wayland
          wayland-scanner
          wayland-protocols
          libffi
          libGLU
          egl-wayland
          libxkbcommon
        ];
        nativeBuildInputs = with pkgs; [
          pkg-config
          cmake
          unixtools.xxd
        ];
        build_project = optimize:
          pkgs.stdenv.mkDerivation {
            pname = "hsnow";
            version = "1.0.0";

            src = ./.;

            buildInputs = libs;
            nativeBuildInputs = nativeBuildInputs;

            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=${
                if optimize
                then "Release"
                else "Debug"
              }"
            ];

            meta = with pkgs.lib; {
              description = "hsnow - `xsnow` implementation in Wayland";
              license = licenses.mit;
              maintainers = with maintainers; [yunusey];
              platforms = platforms.linux;
            };
          };
      in {
        packages = rec {
          default = release;
          debug = build_project false;
          release = build_project true;
        };
        devShells.default = pkgs.mkShell {
          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath libs;
          CMAKE_PREFIX_PATH = pkgs.lib.makeLibraryPath libs;
          buildInputs = libs;
          nativeBuildInputs = nativeBuildInputs ++ libs;
        };
      }
    );
}
