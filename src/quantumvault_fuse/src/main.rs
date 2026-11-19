mod ffi;

use fuser::{
    FileAttr, FileType, Filesystem, MountOption, ReplyAttr, ReplyCreate, ReplyData, ReplyDirectory,
    ReplyEmpty, ReplyEntry, ReplyOpen, ReplyWrite, Request, TimeOrNow,
};
use libc::{EIO, ENOENT};
use std::collections::HashMap;
use std::env;
use std::ffi::OsStr;
use std::fs;
use std::path::{Path, PathBuf};
use std::time::{Duration, SystemTime, UNIX_EPOCH};

const TTL: Duration = Duration::from_secs(1);

struct VirtualFile {
    ino: u64,
    name: String,
    plaintext_size: u64,
    mtime: SystemTime,
}

struct QuantumVaultFS {
    storage_dir: PathBuf,
    pub_key_path: String,
    files: HashMap<u64, VirtualFile>,
    name_to_ino: HashMap<String, u64>,
    next_ino: u64,
}

impl QuantumVaultFS {
    fn new(storage_dir: PathBuf, pub_key_path: String) -> Self {
        let mut files = HashMap::new();
        let mut name_to_ino = HashMap::new();

        // Keep hello.txt from Week 1 as a built-in diagnostic file (ino = 2)
        let hello_msg = "Hello World from QuantumVault FUSE!\n";
        files.insert(
            2,
            VirtualFile {
                ino: 2,
                name: "hello.txt".to_string(),
                plaintext_size: hello_msg.len() as u64,
                mtime: UNIX_EPOCH,
            },
        );
        name_to_ino.insert("hello.txt".to_string(), 2);

        Self {
            storage_dir,
            pub_key_path,
            files,
            name_to_ino,
            next_ino: 3,
        }
    }

    fn root_attr(&self) -> FileAttr {
        FileAttr {
            ino: 1,
            size: 0,
            blocks: 0,
            atime: UNIX_EPOCH,
            mtime: UNIX_EPOCH,
            ctime: UNIX_EPOCH,
            crtime: UNIX_EPOCH,
            kind: FileType::Directory,
            perm: 0o755,
            nlink: 2,
            uid: 1000,
            gid: 1000,
            rdev: 0,
            flags: 0,
            blksize: 512,
        }
    }

    fn file_attr(file: &VirtualFile) -> FileAttr {
        FileAttr {
            ino: file.ino,
            size: file.plaintext_size,
            blocks: (file.plaintext_size + 511) / 512,
            atime: file.mtime,
            mtime: file.mtime,
            ctime: file.mtime,
            crtime: file.mtime,
            kind: FileType::RegularFile,
            perm: 0o644,
            nlink: 1,
            uid: 1000,
            gid: 1000,
            rdev: 0,
            flags: 0,
            blksize: 512,
        }
    }
}

impl Filesystem for QuantumVaultFS {
    fn lookup(&mut self, _req: &Request, parent: u64, name: &OsStr, reply: ReplyEntry) {
        if parent == 1 {
            if let Some(name_str) = name.to_str() {
                if let Some(&ino) = self.name_to_ino.get(name_str) {
                    if let Some(file) = self.files.get(&ino) {
                        reply.entry(&TTL, &Self::file_attr(file), 0);
                        return;
                    }
                }
            }
        }
        reply.error(ENOENT);
    }

    fn getattr(&mut self, _req: &Request, ino: u64, reply: ReplyAttr) {
        if ino == 1 {
            reply.attr(&TTL, &self.root_attr());
        } else if let Some(file) = self.files.get(&ino) {
            reply.attr(&TTL, &Self::file_attr(file));
        } else {
            reply.error(ENOENT);
        }
    }

    fn setattr(
        &mut self,
        _req: &Request,
        ino: u64,
        _mode: Option<u32>,
        _uid: Option<u32>,
        _gid: Option<u32>,
        size: Option<u64>,
        _atime: Option<TimeOrNow>,
        _mtime: Option<TimeOrNow>,
        _ctime: Option<SystemTime>,
        _fh: Option<u64>,
        _crtime: Option<SystemTime>,
        _chgtime: Option<SystemTime>,
        _bkuptime: Option<SystemTime>,
        _flags: Option<u32>,
        reply: ReplyAttr,
    ) {
        if let Some(file) = self.files.get_mut(&ino) {
            if let Some(new_size) = size {
                file.plaintext_size = new_size;
            }
            file.mtime = SystemTime::now();
            let attr = Self::file_attr(file);
            reply.attr(&TTL, &attr);
        } else {
            reply.error(ENOENT);
        }
    }

    fn create(
        &mut self,
        _req: &Request,
        parent: u64,
        name: &OsStr,
        _mode: u32,
        _umask: u32,
        _flags: i32,
        reply: ReplyCreate,
    ) {
        if parent != 1 {
            reply.error(ENOENT);
            return;
        }

        let name_str = match name.to_str() {
            Some(s) => s.to_string(),
            None => {
                reply.error(ENOENT);
                return;
            }
        };

        let ino = if let Some(&existing_ino) = self.name_to_ino.get(&name_str) {
            existing_ino
        } else {
            let new_ino = self.next_ino;
            self.next_ino += 1;
            self.name_to_ino.insert(name_str.clone(), new_ino);
            self.files.insert(
                new_ino,
                VirtualFile {
                    ino: new_ino,
                    name: name_str.clone(),
                    plaintext_size: 0,
                    mtime: SystemTime::now(),
                },
            );
            new_ino
        };

        let attr = Self::file_attr(self.files.get(&ino).unwrap());
        println!("[FUSE] Created virtual file: {} (ino={})", name_str, ino);
        reply.created(&TTL, &attr, 0, 0, 0);
    }

    fn open(&mut self, _req: &Request, ino: u64, _flags: i32, reply: ReplyOpen) {
        if self.files.contains_key(&ino) {
            reply.opened(0, 0);
        } else {
            reply.error(ENOENT);
        }
    }

    fn write(
        &mut self,
        _req: &Request,
        ino: u64,
        _fh: u64,
        offset: i64,
        data: &[u8],
        _write_flags: u32,
        _flags: i32,
        _lock_owner: Option<u64>,
        reply: ReplyWrite,
    ) {
        let file = match self.files.get_mut(&ino) {
            Some(f) => f,
            None => {
                reply.error(ENOENT);
                return;
            }
        };

        println!(
            "[FUSE] Intercepted write() on '{}' ({} bytes) -> Encrypting via CRYSTALS-Kyber...",
            file.name,
            data.len()
        );

        match ffi::encrypt_bytes(data, &self.pub_key_path) {
            Ok(ciphertext) => {
                let backing_path = self.storage_dir.join(format!("{}.qvault", file.name));
                if let Err(e) = fs::write(&backing_path, &ciphertext) {
                    eprintln!("[ERROR] Failed to write encrypted vault file: {}", e);
                    reply.error(EIO);
                    return;
                }

                file.plaintext_size = (offset as u64) + (data.len() as u64);
                file.mtime = SystemTime::now();
                println!(
                    "[PQC] Saved {} encrypted bytes to {:?}",
                    ciphertext.len(),
                    backing_path
                );
                reply.written(data.len() as u32);
            }
            Err(e) => {
                eprintln!("[ERROR] Kyber encryption failed: {}", e);
                reply.error(EIO);
            }
        }
    }

    fn flush(&mut self, _req: &Request, _ino: u64, _fh: u64, _lock_owner: u64, reply: ReplyEmpty) {
        reply.ok();
    }

    fn release(
        &mut self,
        _req: &Request,
        _ino: u64,
        _fh: u64,
        _flags: i32,
        _lock_owner: Option<u64>,
        _flush: bool,
        reply: ReplyEmpty,
    ) {
        reply.ok();
    }

    fn read(
        &mut self,
        _req: &Request,
        ino: u64,
        _fh: u64,
        offset: i64,
        _size: u32,
        _flags: i32,
        _lock: Option<u64>,
        reply: ReplyData,
    ) {
        if ino == 2 {
            let content = b"Hello World from QuantumVault FUSE!\n";
            if (offset as usize) < content.len() {
                reply.data(&content[offset as usize..]);
            } else {
                reply.data(&[]);
            }
        } else if self.files.contains_key(&ino) {
            // Transparent decryption on read() will be implemented in Week 3
            reply.data(&[]);
        } else {
            reply.error(ENOENT);
        }
    }

    fn readdir(
        &mut self,
        _req: &Request,
        ino: u64,
        _fh: u64,
        offset: i64,
        mut reply: ReplyDirectory,
    ) {
        if ino != 1 {
            reply.error(ENOENT);
            return;
        }

        let mut entries: Vec<(u64, FileType, String)> = vec![
            (1, FileType::Directory, ".".to_string()),
            (1, FileType::Directory, "..".to_string()),
        ];

        for file in self.files.values() {
            entries.push((file.ino, FileType::RegularFile, file.name.clone()));
        }

        for (i, entry) in entries.into_iter().enumerate().skip(offset as usize) {
            if reply.add(entry.0, (i + 1) as i64, entry.1, &entry.2) {
                break;
            }
        }
        reply.ok();
    }
}

fn main() {
    let mountpoint = env::args()
        .nth(1)
        .expect("Usage: quantumvault_fuse <MOUNTPOINT>");

    let key_dir = Path::new("../../vault_keys");
    let storage_dir = PathBuf::from("../../vault_storage");
    fs::create_dir_all(key_dir).expect("Failed to create vault_keys directory");
    fs::create_dir_all(&storage_dir).expect("Failed to create vault_storage directory");

    let pub_key_path = "../../vault_keys/public.key";
    let sec_key_path = "../../vault_keys/secret.key";

    if !Path::new(pub_key_path).exists() {
        println!("[+] Generating CRYSTALS-Kyber (ML-KEM-768) master keypair...");
        ffi::generate_keypair(pub_key_path, sec_key_path)
            .expect("Failed to generate PQC keypair");
        println!("[+] Keys saved securely to vault_keys/");
    }

    let options = vec![
        MountOption::RW,
        MountOption::FSName("quantumvault".to_string()),
    ];

    println!("[+] Mounting QuantumVault Read/Write FUSE at: {}", mountpoint);
    let fs = QuantumVaultFS::new(storage_dir, pub_key_path.to_string());
    fuser::mount2(fs, mountpoint, &options).unwrap();
}
