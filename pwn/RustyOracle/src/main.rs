// #![allow(dead_code)]
// #![allow(unused_imports)]
// #![allow(unused_variables)]
// #![allow(unused_assignments)]

extern crate libc;
extern crate regex;
#[macro_use] extern crate lazy_static;

use regex::RegexSet;
use std::io::{self, Read};

const MAX_BUF_SIZE: usize = 2048;

#[link(name = "mylibc")]
extern {
    fn vuln_code(s: *const libc::c_uchar, size: usize);
}

#[derive(Debug)]
struct Input<'a> {
    data: &'a [u8],
    default_size: usize,
}

lazy_static!{
    static ref RE: RegexSet = RegexSet::new(&[
        r"u1F52E.*",
    ]).unwrap();
}

impl<'a> Input<'a> {
    fn new() -> Self {
        return Input{
            data: "\u{1F507} The ORACLE has nothing so say \u{1f507}".as_bytes(),
            default_size: 1024,
        }
    }
    fn set_data(mut self, data: &'a [u8]) -> Self {
        self.data = data;
        self
    }
    fn print_oracle(&self, size: usize) {
        println!("μαντείο says: ");
        let data = self.data;
        unsafe {
            vuln_code(data.as_ptr(), size);
        }
    }
}


fn does_it_match(data: &[u8]) -> bool {
    let matches: Vec<usize> = RE.matches(&String::from_utf8_lossy(data))
                                .into_iter()
                                .collect();
    if matches.len() == 0 {
        return false
    } else {
        return true
    }
}


fn main() {
    println!("---------------------------------------------");
    println!("| \u{1F52E} WELCOME TO THE ORACLE \u{1F52E}               |");
    println!("|                                           |");
    println!("| μιλήστε και το μαντείο θα σας απαντήσει   |");
    println!("|                                           |");
    println!("---------------------------------------------");

    let mut tmpbuff = String::with_capacity(MAX_BUF_SIZE);
    println!("Ask μαντείο your question !");
    let _: usize = match io::stdin().read_line(&mut tmpbuff) {
        Ok(n) => n as usize,
        Err(_error) => 0 as usize,
    };

    // cheet code -> need a leak because of PIE binary
    if tmpbuff.contains("boogyismyhero") {
        let foobar = (libc::strncpy as *const ()) as isize;
        println!("here is your fortune cookie \u{1f960} : {}", foobar);
    }

    println!("Do you have enything else to ask μαντείο ?");
    let mut user_input = [0; MAX_BUF_SIZE];
    let read_size = MAX_BUF_SIZE;
    io::stdin().read_exact(&mut user_input).unwrap();

    let data = user_input;
    if data.is_empty() == false
    {
        let mut work_data = Vec::new();
        work_data.push(data);

        for _line in work_data.into_iter()
        {
            if does_it_match(&_line)
            {
                Input::new()
                    .set_data( &_line[ 6..(*&_line.len()) ] )
                    .print_oracle(read_size - 6);
            } else {
                println!("Try harder god damn it !!!");
            }
        }
    } else {
        Input::new().print_oracle(1024)
    }
}
