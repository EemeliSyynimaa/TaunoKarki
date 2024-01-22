#if 0
bool gl_check_error(char* t)
{
    bool result = true;

    GLenum error = api.gl.glGetError();

    switch (error)
    {
        case GL_NO_ERROR:
            // LOG("glGetError(): NO ERRORS (%s)\n", t);
            result = false;
            break;
        case GL_INVALID_ENUM:
            LOG("glGetError(): INVALID ENUM (%s)\n", t);
            break;
        case GL_INVALID_VALUE:
            LOG("glGetError(): INVALID VALUE (%s)\n", t);
            break;
        case GL_INVALID_OPERATION:
            LOG("glGetError(): INVALID OPERATION (%s)\n", t);
            break;
        case GL_STACK_OVERFLOW:
            LOG("glGetError(): STACK OVERFLOW (%s)\n", t);
            break;
        case GL_STACK_UNDERFLOW:
            LOG("glGetError(): STACK UNDERFLOW (%s)\n", t);
            break;
        case GL_OUT_OF_MEMORY:
            LOG("glGetError(): OUT OF MEMORY (%s)\n", t);
            break;
        default:
            LOG("glGetError(): UNKNOWN ERROR (%s)\n", t);
            break;
    };

    return result;
}

// Todo: create single struct for header (requires packing)
struct color_map_spec
{
    s16 index;
    s16 length;
    s8 size;
};

struct image_spec
{
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s8 depth;
    s8 desc;
};

void tga_decode(s8* input, u64 out_size, s8* output, u32* width, u32* height)
{
    input += 8;
    struct image_spec* i_spec = (struct image_spec*)input;
    input += 10;

    // Todo: read (or skip) image id and color map stuff

    u64 bytes_per_color = i_spec->depth / 8;
    u64 byte_count = i_spec->height * i_spec->width;

    for (u64 i = 0; i < byte_count && i < out_size; i++)
    {
        output[2] = input[0];
        output[1] = input[1];
        output[0] = input[2];
        output[3] = input[3];

        output += bytes_per_color;
        input += bytes_per_color;
    }

    *width = i_spec->width;
    *height = i_spec->height;
}

u32 texture_create(struct memory_block* block, char* path)
{
    u64 read_bytes = 0;
    u32 target = GL_TEXTURE_2D;
    u32 id = 0;
    u32 width = 0;
    u32 height = 0;

    file_handle file;
    u64 file_size = 0;
    s8* file_data = 0;
    s8* pixel_data = 0;

    api.file.file_open(&file, path, true);
    api.file.file_size_get(&file, &file_size);

    file_data = stack_alloc(block, file_size);
    pixel_data = stack_alloc(block, file_size);

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    tga_decode(file_data, read_bytes, pixel_data, &width, &height);

    api.gl.glGenTextures(1, &id);
    api.gl.glBindTexture(target, id);

    api.gl.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    api.gl.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    api.gl.glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, pixel_data);

    stack_free(block);
    stack_free(block);

    gl_check_error("texture_create");

    return id;
}

u32 texture_array_create(struct memory_block* block, char* path, u32 rows,
    u32 cols)
{
    file_handle file;
    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;
    s8* pixel_data = 0;
    s8* tile_data = 0;

    api.file.file_open(&file, path, true);
    api.file.file_size_get(&file, &file_size);

    file_data = stack_alloc(block, file_size);
    pixel_data = stack_alloc(block, file_size);

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    u32 image_width = 0;
    u32 image_height = 0;

    tga_decode(file_data, read_bytes, pixel_data, &image_width, &image_height);

    u32 tile_width = image_width / cols;
    u32 tile_height = image_height / rows;
    s32 depth = rows * cols;

    u32 id = 0;
    api.gl.glGenTextures(1, &id);
    api.gl.glBindTexture(GL_TEXTURE_2D_ARRAY, id);

    api.gl.glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);
    api.gl.glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR);

    api.gl.glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, tile_width,
        tile_height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    u32 channels = 4;
    u32 tile_size_bytes = tile_width * tile_height * channels;

    tile_data = stack_alloc(block, tile_size_bytes);

    for (u32 y = 0; y < rows; y++)
    {
        for (u32 x = 0; x < cols; x++)
        {
            s8* ptr_dest = tile_data;
            memory_set(ptr_dest, tile_size_bytes, 0);

            for (u32 i = 0; i < tile_height; i++)
            {
                s8* ptr_src = pixel_data + ((y * tile_height + i) *
                    image_width + x * tile_width) * channels;

                for (u32 j = 0; j < tile_width; j++)
                {
                    for (u32 k = 0; k < channels; k++)
                    {
                        *ptr_dest++ = *ptr_src++;
                    }
                }
            }

            u32 i = y * cols + x;
            api.gl.glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tile_width,
                tile_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, tile_data);
        }
    }

    stack_free(block);
    stack_free(block);
    stack_free(block);

    gl_check_error("texture_array_create");

    return id;
}

b32 str_compare(char* str1, char* str2)
{
    while (*str1++ == *str2++)
    {
        if (*str1 == '\0')
        {
            return true;
        }
    }

    return false;
}

b32 is_digit(char c)
{
    return c >= '0' && c <= '9';
}

b32 is_space(char c)
{
    return (c >= 9 && c <= 13) || c == 32;
}

s32 s32_parse(char* str, u64* size)
{
    // Todo: ignore leading whitespaces
    s32 value = 0;
    u64 bytes = 0;
    b32 negative = false;

    if (*str == '-')
    {
        negative = true;
        str++;
        bytes++;
    }

    while (is_digit(*str))
    {
        u8 val = *str++ - '0';

        value *= 10;
        value += val;

        bytes++;
    }

    if (negative)
    {
        value *= -1;
    }

    if (size)
    {
        *size = bytes;
    }

    return value;
}

f32 f32_parse(char* str, u64* size)
{
    // Todo: ignore leading whitespaces
    // Todo: int parser has duplicate code
    f32 value = 0;
    b32 negative = false;
    u64 bytes = 0;

    if (*str == '-')
    {
        negative = true;
        str++;
        bytes++;
    }

    while (is_digit(*str))
    {
        u8 val = *str++ - '0';

        value *= 10.0;
        value += val;
        bytes++;
    }

    if (*str++ == '.')
    {
        s32 num_decimals = 0;
        bytes++;

        while (is_digit(*str))
        {
            u8 val = *str++ - '0';

            value *= 10.0;
            value += val;

            num_decimals--;
            bytes++;
        }

        if (*str++ == 'e')
        {
            b32 negative_exponent = false;

            bytes++;

            if (*str == '-')
            {
                negative_exponent = true;
                str++;
                bytes++;
            }
            else if (*str == '+')
            {
                negative_exponent = false;
                str++;
                bytes++;
            }

            s32 exponent = 0;

            while (is_digit(*str))
            {
                u8 val = *str++ - '0';

                exponent *= 10;
                exponent += val;

                bytes++;
            }

            num_decimals += negative_exponent ? -exponent : exponent;
        }

        while (num_decimals < 0)
        {
            value *= 0.1;

            num_decimals++;
        }

        while (num_decimals > 0)
        {
            value *= 10;

            num_decimals--;
        }
    }

    if (negative)
    {
        value *= -1;
    }

    if (size)
    {
        *size = bytes;
    }

    return value;
}

u64 string_read(char* data, char* str, u64 max_size)
{
    u64 bytes_read = 0;

    while (is_space(*data) && *data != '\0')
    {
        data++;
        bytes_read++;
    }

    u64 str_size = 0;

    while (!is_space(*data) && *data != '\0' && str_size < max_size - 1)
    {
        *str++ = *data++;

        str_size++;
        bytes_read++;
    }

    *str = '\0';

    return bytes_read;
}

void mesh_create(struct memory_block* block, char* path, struct mesh* mesh)
{
    // Todo: remove statics
    static struct v3 in_vertices[4096];
    static struct v3 in_normals[4096];
    static struct v2 in_uvs[4096];
    static struct vertex vertices[4096];
    static u32 in_faces[4096*3];
    static u32 indices[4096];

    u32 num_in_vertices = 0;
    u32 num_in_normals = 0;
    u32 num_in_uvs = 0;
    u32 num_in_faces = 0;
    u32 num_vertices = 0;

    // Todo:
    // X read file size
    // X reserve space for file data
    // - reserve space for vertices, faces, texture coords and normals
    //   - can it be calculated beforehand?
    //   - read each line and count each face, vertex normal etc and then
    //     reserve memory accordingly?
    // - reserve space for vertices and indices
    // - count indices

    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;

    file_handle file;
    api.file.file_open(&file, path, true);
    api.file.file_size_get(&file, &file_size);

    // Todo: haxy way, but +1 for the ending \0, otherwise following loops may
    // fail if there's already something in the memory. Fix the loop to not go
    // beyond reserved memory!
    file_data = stack_alloc(block, file_size + 1);
    *(file_data + file_size) = '\0';

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    char* data = (char*)file_data;
    char str[255] = {0};

    for (u64 i = 0; i < read_bytes && *data != '\0'; i++)
    {
        u64 str_size = string_read(data, str, 255);

        if (str_compare(str, "v"))
        {
            struct v3* v = &in_vertices[num_in_vertices++];

            // LOG("v");

            data += str_size;
            str_size = string_read(data, str, 255);
            v->x = f32_parse(str, NULL);
            // LOG(" %f", v->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->y = f32_parse(str, NULL);
            // LOG(" %f", v->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->z = f32_parse(str, NULL);
            // LOG(" %f\n", v->z);
        }
        else if (str_compare(str, "vt"))
        {
            struct v2* uv = &in_uvs[num_in_uvs++];

            // LOG("vt");

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->x = f32_parse(str, NULL);
            // LOG(" %f", uv->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->y = f32_parse(str, NULL);
            // LOG(" %f\n", uv->y);
        }
        else if (str_compare(str, "vn"))
        {
            struct v3* n = &in_normals[num_in_normals++];

            // LOG("vn");

            data += str_size;
            str_size = string_read(data, str, 255);
            n->x = f32_parse(str, NULL);
            // LOG(" %f", n->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->y = f32_parse(str, NULL);
            // LOG(" %f", n->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->z = f32_parse(str, NULL);
            // LOG(" %f\n", n->z);
        }
        else if (str_compare(str, "f"))
        {
            // LOG("f");

            for (u32 i = 0; i < 3; i++)
            {
                u32* face = &in_faces[num_in_faces];

                data += str_size;
                str_size = string_read(data, str, 255);

                char* s = str;

                u64 bytes_read = 0;
                face[0] = s32_parse(s, &bytes_read);
                // LOG(" %d", face[0]);
                s += bytes_read + 1;
                // LOG("%c", *s++);
                face[1] = s32_parse(s, &bytes_read);
                // LOG("%d", face[1]);
                s += bytes_read + 1;
                // LOG("%c", *s++);
                face[2] = s32_parse(s, &bytes_read);
                // LOG("%d", face[2]);

                num_in_faces += 3;
            }

            // LOG("\n");
        }
        else
        {
            data += str_size;
        }
    }

    for (u32 i = 0; i < num_in_faces; i += 3)
    {
        u32* face = &in_faces[i];

        struct vertex v;

        v.position = in_vertices[face[0] - 1];
        v.uv = in_uvs[face[1] - 1];
        v.normal = in_normals[face[2] - 1];
        v.color.r = 1.0f;
        v.color.g = 1.0f;
        v.color.b = 1.0f;
        v.color.a = 1.0f;

        b32 found = false;

        for (u32 j = 0; j < num_vertices; j++)
        {
            struct vertex other = vertices[j];

            if (v3_equals(v.position, other.position) &&
                v2_equals(v.uv, other.uv) &&
                v3_equals(v.normal, other.normal))
            {
                indices[mesh->num_indices++] = j;

                found = true;
                break;
            }
        }

        if (!found)
        {
            vertices[num_vertices++] = v;
            indices[mesh->num_indices++] = num_vertices - 1;
        }
    }

    stack_free(block);

    LOG("Vertices: %d Indices: %d\n", num_vertices, mesh->num_indices);
    api.gl.glGenVertexArrays(1, &mesh->vao);
    api.gl.glBindVertexArray(mesh->vao);

    api.gl.glGenBuffers(1, &mesh->vbo);
    api.gl.glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    api.gl.glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(struct vertex),
        vertices, GL_DYNAMIC_DRAW);

    api.gl.glGenBuffers(1, &mesh->ibo);
    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh->num_indices * sizeof(u32), indices, GL_DYNAMIC_DRAW);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);

    // Todo: implement offsetof
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)0);
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)12);
    api.gl.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)20);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)32);

    gl_check_error("mesh_create");
}

u32 program_create(struct memory_block* block, char* vertex_shader_path,
    char* fragment_shader_path)
{
    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;

    u32 result = 0;
    u32 program = api.gl.glCreateProgram();
    u32 vertex_shader = api.gl.glCreateShader(GL_VERTEX_SHADER);
    u32 fragment_shader = api.gl.glCreateShader(GL_FRAGMENT_SHADER);

    // Todo: implement assert
    // assert(program);
    // assert(vertex_shader);
    // assert(fragment_shader);

    // Note: glShaderSource requires for each string to be null terminated.
    // If read directly from a file, each line ends in CR LF (0d 0a).
    // These should be replaced with 0.
    file_handle file;

    api.file.file_open(&file, vertex_shader_path, true);
    api.file.file_size_get(&file, &file_size);

    // Reserve space for 0 at the end of shader data.
    file_data = stack_alloc(block, file_size + 1);
    *(file_data + file_size) = 0;

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    const GLchar* temp = (const GLchar*)file_data;

    api.gl.glShaderSource(vertex_shader, 1, &temp, 0);
    api.gl.glCompileShader(vertex_shader);
    api.gl.glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    stack_free(block);

    api.file.file_open(&file, fragment_shader_path, true);
    api.file.file_size_get(&file, &file_size);

    // Reserve space for 0 at the end of shader data.
    file_data = stack_alloc(block, file_size + 1);
    *(file_data + file_size) = 0;

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    temp = (const GLchar*)file_data;

    api.gl.glShaderSource(fragment_shader, 1, &temp, 0);
    api.gl.glCompileShader(fragment_shader);
    api.gl.glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    stack_free(block);

    api.gl.glAttachShader(program, vertex_shader);
    api.gl.glAttachShader(program, fragment_shader);

    api.gl.glLinkProgram(program);
    api.gl.glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&result);
    // assert(result);

    api.gl.glDeleteShader(vertex_shader);
    api.gl.glDeleteShader(fragment_shader);

    gl_check_error("shader_create");

    return program;
}
#endif
