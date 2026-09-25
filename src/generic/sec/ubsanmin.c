/**
 * @file ubsanmin.c
 * @author apixeldev
 * @brief UBSan (Undefined Behavior) minimal runtime
 * @version 0.1
 * @date 2026-09-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "../lib/flanterm/src/flanterm.h"
#include <stdint.h>
#include <stdio.h>

extern void print_logo(void);

extern struct flanterm_context *flantermctx;

__attribute((noreturn)) void ubsan_handle(const char* string, uintptr_t rip) {
    #ifdef __x86_64__
    __asm("cli");
    #endif

    flanterm_set_text_bg(flantermctx, 3, false);
    flanterm_clear(flantermctx, true);

    print_logo();
    printf("%s at: %p\n", string, rip);

    while(1);
    __builtin_unreachable();
}

void __ubsan_handle_type_mismatch_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Type mismatch", rip);
}

void __ubsan_handle_alignment_assumption_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Alignment assumption", rip);
}

void __ubsan_handle_add_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Add overflow", rip);
}

void __ubsan_handle_sub_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Subtraction overflow", rip);
}

void __ubsan_handle_mul_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Multiplication overflow", rip);
}

void __ubsan_handle_negate_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Negate overflow", rip);
}

void __ubsan_handle_divrem_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Divide remanider overflow", rip);
}

void __ubsan_handle_shift_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Shift out of bounds", rip);
}

void __ubsan_handle_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Out of bounds", rip);
}

void __ubsan_handle_local_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Local out of bounds", rip);
}

void __ubsan_handle_builtin_unreachable_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Hit builtin_unreachable()", rip);
}

void __ubsan_handle_missing_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Missing return", rip);
}

void __ubsan_handle_vla_bound_not_positive_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] VLA bound not positive", rip);
}

void __ubsan_handle_float_cast_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Float cast overflow", rip);
}

void __ubsan_handle_load_invalid_value_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Load invalid value", rip);
}

void __ubsan_handle_invalid_builtin_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Invalid builtin", rip);
}

void __ubsan_handle_invalid_objc_cast_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Invalid objc cast", rip);
}

void __ubsan_handle_function_type_mismatch_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Function type mismatch", rip);
}

void __ubsan_handle_implicit_conversion_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Implicit conversion", rip);
}

void __ubsan_handle_nonnull_arg_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Nonnull argument", rip);
}

void __ubsan_handle_nonnull_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Nonnull return", rip);
}

void __ubsan_handle_nullability_arg_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Nullability argument", rip);
}

void __ubsan_handle_nullability_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Nullability return", rip);
}

void __ubsan_handle_pointer_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] Pointer overflow", rip);
}

void __ubsan_handle_cfi_check_fail_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    ubsan_handle("[UBSan] CFI check fail", rip);
}