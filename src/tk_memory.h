#ifndef __TK_MEMORY_H__
#define __TK_MEMORY_H__

#include "tk_platform.h"

struct memory_block
{
    u64 size;
    u8* base;
    u8* current;
};

struct object_pool
{
    void* data;
    u32 size;
    u32 count;
    u32 next;
};

static struct memory_block stack_init(u8* base_address, u64 size)
{
    struct memory_block result = { 0 };

    result.base = base_address;
    result.current = result.base;
    result.size = size;

    return result;
}

static void* stack_alloc(struct memory_block* block, u64 size)
{
    // Todo: add alignment
    u64 bytes_left = (block->base + block->size) - block->current;
    u64 bytes_needed = size + sizeof(u64);

    u8* result = 0;

    if (bytes_needed <= bytes_left)
    {
        result = block->current;

        block->current += size;

        *((u64*)block->current) = size;

        block->current += sizeof(u64);
    }

    return result;
}

static void* stack_free(struct memory_block* block)
{
    if (block->current > block->base)
    {
        block->current -= sizeof(u64);

        u64 size = *((u64*)block->current);

        block->current -= size;
    }

    return block->current;
}

static void memory_set(void* data, u64 size, u8 value)
{
    for (u32 i = 0; i < size; i++)
    {
        *((u8*)data + i) = value;
    }
}

static void memory_copy(void* src, void* dest, u64 size)
{
    for (u32 i = 0; i < size; i++)
    {
        *((u8*)dest + i) = *((u8*)src + i);
    }
}

static void object_pool_init(struct object_pool* pool, u32 object_size,
    u32 object_count, struct memory_block* block)
{
    pool->size = object_size;
    pool->count = object_count;
    pool->next = 0;
    pool->data = stack_alloc(block, object_size * object_count);
}

static void object_pool_reset(struct object_pool* pool)
{
    memory_set(pool->data, pool->size * pool->count, 0);
    pool->next = 0;
}

static void* object_pool_get_next(struct object_pool* pool)
{
    void* result = 0;

    if (pool->data)
    {
        result = (void*)((u8*)pool->data + pool->next * pool->size);

        if (++pool->next >= pool->count)
        {
            pool->next = 0;
        }
    }

    return result;
}

#endif
