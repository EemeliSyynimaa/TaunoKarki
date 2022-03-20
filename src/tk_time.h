type_ticks_current_get* ticks_current_get;
type_ticks_frequency_get* ticks_frequency_get;

void time_functions_set(struct time_functions* time)
{
    LOG("Copying time functions...");
    ticks_current_get = time->ticks_current_get;
    ticks_frequency_get = time->ticks_frequency_get;
    LOG("done\n");
}