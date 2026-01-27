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

  buildInputs = [
    openssl
  ];

  installPhase = ''
    mkdir -p $out/lib
    cp ./*.dll $out/lib
  '';

  postFixup = ''
    md5sum $out/lib/lib*.dll > $out/libnexus_speedometer.dll.md5
  '';
}
