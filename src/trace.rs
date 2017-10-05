// Copyright (c) 2017 László Nagy
//
// Licensed under the MIT license <LICENSE or http://opensource.org/licenses/MIT>.
// This file may not be copied, modified, or distributed except according to those terms.

use serde_derive;
use serde_json;

use std::io;
use std::fs::OpenOptions;
use std::path::PathBuf;
use std::cmp::PartialEq;


#[derive(Serialize, Deserialize)]
#[derive(Debug, PartialEq)]
pub struct Trace {
    pub pid: usize,
    pub cwd: String,
    pub cmd: Vec<String>
}

#[derive(Debug)]
pub enum TraceError {
    Io(io::Error),
    Json(serde_json::Error),
}

impl From<io::Error> for TraceError {
    fn from(err: io::Error) -> TraceError {
        TraceError::Io(err)
    }
}

impl From<serde_json::Error> for TraceError {
    fn from(err: serde_json::Error) -> TraceError {
        TraceError::Json(err)
    }
}


impl Trace {
    /// Create an Trace report object from the given arguments.
    /// Capture the current process id and working directory.
    pub fn create(args: &Vec<String>) -> Option<Trace> {
        // get pid
        // get cwd
        return None;
    }

    /// Creates an trace file in the given directory, with the given
    /// content. Returns the created file path.
    pub fn write(path: &PathBuf, value: &Trace) -> Result<PathBuf, TraceError> {
        let file_name = path.join("random");
        let mut file = OpenOptions::new().write(true).open(file_name.as_path())?;
        let result = serde_json::to_writer(file, value)?;
        Ok(file_name)
    }

//    /// Read all trace files content from given directory.
//    pub fn read_all(path: &PathBuf) -> Iterator<Trace> {
//    }

    /// Read a single trace file content from given file name.
    pub fn read(path: &PathBuf) -> Result<Trace, TraceError> {
        let mut file = OpenOptions::new().read(true).open(path.as_path())?;
        let entry = serde_json::from_reader(file)?;
        Ok(entry)
    }
}
