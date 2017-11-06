// Copyright (c) 2017 László Nagy
//
// Licensed under the MIT license <LICENSE or http://opensource.org/licenses/MIT>.
// This file may not be copied, modified, or distributed except according to those terms.

extern crate libc;
extern crate serde;
extern crate serde_json;
#[macro_use] extern crate serde_derive;
#[macro_use] extern crate log;
extern crate tempdir;


pub mod trace;
pub mod parameters;


use std::result;
use std::io;

#[derive(Debug)]
pub enum Error {
    Io(io::Error),
    Json(serde_json::Error),
}

impl From<io::Error> for Error {
    fn from(err: io::Error) -> Error {
        Error::Io(err)
    }
}

impl From<serde_json::Error> for Error {
    fn from(err: serde_json::Error) -> Error {
        Error::Json(err)
    }
}

type Result<T> = result::Result<T, Error>;
