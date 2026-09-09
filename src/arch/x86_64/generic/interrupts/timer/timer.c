#include <x86_64/schedarch.h>
#include <x86_64/apic.h>
#include <scheduler.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint64_t ticks = 0;

void lapic_timer_drv(regs_frame_t *frame, fxsave_area_t* fxsave) {
    (void)fxsave;
    ticks++;
    // Check if we are in userland or not.
    // If so we start our fun work!
    /*
    NOTE: The scheduler is entirely untested and no functional userland code has been ran so far
    This may entirely crash, if anyone wants to check if it will work, PLEASE DO
    */
    if ((ticks % 5) == 0) {
        if (frame->cs & 0x03) {
            thread_t *current = get_current_thread();
            if (current) {
                // Save current thread state
                regs_thread_state_t *state = (regs_thread_state_t *)current->archdata;
                memcpy(&state->frame, frame, sizeof(regs_frame_t));
                memcpy(&state->fxsave, fxsave, sizeof(fxsave_area_t));
            }
            // Run the next task
            thread_t *next = schedule();
            if (next) {
                regs_thread_state_t *state = (regs_thread_state_t *)next->archdata;
                memcpy(frame, &state->frame, sizeof(regs_frame_t));
                memcpy(fxsave, &state->fxsave, sizeof(fxsave_area_t));
            }
        }
    }
    lapic_eoi();
}