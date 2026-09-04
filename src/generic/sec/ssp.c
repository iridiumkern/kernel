#include <stdio.h>
#include <sec/random.h>
#include <stdint.h>
#include <stdbool.h>
#include <panic.h>

uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;
static bool hasbeeninit = false;

/**
 * @brief Sets up the SSP
 * 
 * @return true SSP setup worked
 * @return false SSP setup did not work
 */
__attribute__((no_stack_protector)) bool sspsetup(void) {
    (void)hasbeeninit;

    printf("SSP: entered sspsetup()\n");

    printf("SSP: declaring data\n");
    uint64_t data[8];

    printf("SSP: data allocated at %p\n", (void *)data);

    printf("SSP: calling csprng_getrand()\n");
    if (!csprng_getrand((uint8_t *)data)) {
        printf("SSP: csprng_getrand() FAILED\n");
        return false;
    }

    printf("SSP: csprng_getrand() returned successfully\n");

    printf("SSP: data[0] = 0x%016llx\n",
           (unsigned long long)data[0]);

    printf("SSP: setting __stack_chk_guard\n");
    __stack_chk_guard = data[0];

    printf("SSP: __stack_chk_guard = 0x%016llx\n",
           (unsigned long long)__stack_chk_guard);

    printf("SSP: setting hasbeeninit\n");
    hasbeeninit = true;

    printf("SSP: sspsetup() returning true\n");
    return true;
}

/**
 * @brief Stack check fail condition function, wrapper for kpanic more or less
 * 
 */
__attribute__((noreturn)) void __stack_chk_fail(void) {
    kpanic("SSP triggered!\n");
}