{
  cmake,
  openssl,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nexus_chat_shorts";
  version = "0.3.1.0";
  src = ./.;

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [openssl];

  installPhase = ''
    x86_64-w64-mingw32-strip ./*.dll
    mkdir -p $out/lib
    cp ./*.dll $out/lib
  '';
}
