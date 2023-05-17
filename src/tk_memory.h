struct memory_block
{
    u64 size;
    s8* base;
    s8* current;
};

void* stack_alloc(struct memory_block* block, u64 size)
{
    // Todo: add alignment
    u64 bytes_left = (block->base + block->size) - block->current;
    u64 bytes_needed = size + sizeof(u64);

    if (bytes_needed > bytes_left)
    {
        LOG("Not enough memory\n");

        return 0;
    }

    s8* result = block->current;

    block->current += size;

    *((u64*)block->current) = size;

    block->current += sizeof(u64);

    return result;
}

void* stack_free(struct memory_block* block)
{
    if (block->current > block->base)
    {
        block->current -= sizeof(u64);

        u64 size = *((u64*)block->current);

        block->current -= size;
    }
    else
    {
        LOG("Nothing to free in memory\n");
    }

    return block->current;
}

void memory_set(void* data, u64 size, u8 value)
{
    for (u32 i = 0; i < size; i++)
    {
        *((u8*)data + i) = value;
    }
}

void memory_copy(void* src, void* dest, u64 size)
{
    for (u32 i = 0; i < size; i++)
    {
        *((u8*)dest + i) = *((u8*)src + i);
    }
}
