{
  mkShell,
  cmake,
  clang-tools,
  bintools,
  openssl,
  stdenv,
}:
mkShell {
  nativeBuildInputs = [
    cmake
    clang-tools
    bintools
  ];

  buildInputs = [
    openssl
  ];

  shellHook = ''
    export CPLUS_INCLUDE_PATH="${stdenv.cc.cc}/include/c++/${stdenv.cc.cc.version}";
  '';
}
