use std::ffi::CString;
use std::os::raw::{c_char, c_int};
use std::ptr;
use std::slice;

extern "C" {
    fn qv_generate_keypair(pub_key_path: *const c_char, sec_key_path: *const c_char) -> c_int;
    fn qv_encrypt_buffer(
        plaintext: *const u8,
        plaintext_len: usize,
        pub_key_path: *const c_char,
        ciphertext_out: *mut *mut u8,
        ciphertext_len_out: *mut usize,
    ) -> c_int;
    fn qv_free_buffer(buffer: *mut u8);
}

pub fn generate_keypair(pub_path: &str, sec_path: &str) -> Result<(), String> {
    let c_pub = CString::new(pub_path).map_err(|e| e.to_string())?;
    let c_sec = CString::new(sec_path).map_err(|e| e.to_string())?;
    let res = unsafe { qv_generate_keypair(c_pub.as_ptr(), c_sec.as_ptr()) };
    if res == 0 {
        Ok(())
    } else {
        Err(format!("qv_generate_keypair failed with code {}", res))
    }
}

pub fn encrypt_bytes(data: &[u8], pub_path: &str) -> Result<Vec<u8>, String> {
    let c_pub = CString::new(pub_path).map_err(|e| e.to_string())?;
    let mut out_ptr: *mut u8 = ptr::null_mut();
    let mut out_len: usize = 0;

    let res = unsafe {
        qv_encrypt_buffer(
            data.as_ptr(),
            data.len(),
            c_pub.as_ptr(),
            &mut out_ptr,
            &mut out_len,
        )
    };

    if res != 0 || out_ptr.is_null() {
        return Err(format!("qv_encrypt_buffer failed with code {}", res));
    }

    let encrypted = unsafe {
        let slice = slice::from_raw_parts(out_ptr, out_len);
        let vec = slice.to_vec();
        qv_free_buffer(out_ptr);
        vec
    };

    Ok(encrypted)
}
