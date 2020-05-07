#include "file_api.h"

type_file_open* file_open;
type_file_close* file_close;
type_file_read* file_read;
type_file_size_get* file_size_get;

void file_functions_set(file_functions* file)
{
    file_open = file->file_open;
    file_close = file->file_close;
    file_read = file->file_read;
    file_size_get = file->file_size_get;
}