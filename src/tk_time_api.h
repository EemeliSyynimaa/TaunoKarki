typedef u64 type_ticks_current_get();
typedef u64 type_ticks_frequency_get();

struct time_functions
{
    type_ticks_current_get* ticks_current_get;
    type_ticks_frequency_get* ticks_frequency_get;
};
