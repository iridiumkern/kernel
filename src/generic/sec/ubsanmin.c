/**
 * @file ubsanmin.c
 * @author apixeldev
 * @brief UBSan (Undefined Behavior) runtime for the minimal version
 * @version 0.1
 * @date 2026-09-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <panic.h>

void __ubsan_handle_type_mismatch_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_alignment_assumption_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_add_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_sub_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_mul_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_negate_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_divrem_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_shift_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_local_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_builtin_unreachable_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_missing_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_vla_bound_not_positive_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_float_cast_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_load_invalid_value_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_invalid_builtin_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_invalid_objc_cast_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_function_type_mismatch_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_implicit_conversion_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_nonnull_arg_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_nonnull_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_nullability_arg_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_nullability_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_pointer_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}

void __ubsan_handle_cfi_check_fail_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("UBSAN ERROR at: %p\n", rip);
}
