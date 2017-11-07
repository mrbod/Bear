// Copyright (c) 2017 László Nagy
//
// Licensed under the MIT license <LICENSE or http://opensource.org/licenses/MIT>.
// This file may not be copied, modified, or distributed except according to those terms.

use std::ffi;

pub enum CompilerPass {
    Preprocessor,
    Compilation,
    Assembly,
    Linking
}

pub struct Compilation {
    compiler: ffi::OsString,
    phase: CompilerPass,
    flags: Vec<ffi::OsString>,
    source: ffi::OsString,
    output: Option<ffi::OsString>,
    cwd: ffi::OsString,
}

