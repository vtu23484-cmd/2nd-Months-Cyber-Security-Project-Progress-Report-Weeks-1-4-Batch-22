fn main() {
    println!("cargo:rustc-link-search=native=../../build");
    println!("cargo:rustc-link-search=native=/usr/local/lib");
    println!("cargo:rustc-link-lib=dylib=pqc_engine");
    println!("cargo:rustc-link-lib=dylib=oqs");
    println!("cargo:rerun-if-changed=../../src/pqc_engine.c");
    println!("cargo:rerun-if-changed=../../src/pqc_engine.h");
}
