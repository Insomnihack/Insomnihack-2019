extern crate cc;

fn main() {
    cc::Build::new()
        .flag_if_supported("-Wall")
        .flag_if_supported("-Wno-format")
        .flag_if_supported("-Wno-unused-function")
        .flag_if_supported("-Wno-implicit-function-declaration")
        .flag_if_supported("-Wno-parentheses")
        .flag_if_supported("-fno-stack-protector")
        .file("src/mylibc.c")
        .compile("mylibc");
}
