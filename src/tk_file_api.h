
typedef u64 file_handle;

typedef void type_file_open(file_handle* file, s8* path);
typedef void type_file_close(file_handle* file);
typedef void type_file_read(file_handle* file, s8* data, u64 bytes_max,
    u64* bytes_read);
typedef void type_file_size_get(file_handle* file, u64* file_size);

struct file_functions
{
    type_file_open* file_open;
    type_file_close* file_close;
    type_file_read* file_read;
    type_file_size_get* file_size_get;
};
