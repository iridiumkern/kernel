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

#include <stdint.h>
#include <panic.h>

void __ubsan_handle_type_mismatch_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Type mismatch at: %p\n", rip);
}

void __ubsan_handle_alignment_assumption_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Alignment assumption at: %p\n", rip);
}

void __ubsan_handle_add_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Add overflow at: %p\n", rip);
}

void __ubsan_handle_sub_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Subtraction overflow at: %p\n", rip);
}

void __ubsan_handle_mul_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Multiplication overflow at: %p\n", rip);
}

void __ubsan_handle_negate_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Negate overflow at: %p\n", rip);
}

void __ubsan_handle_divrem_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Divide remanider overflow at: %p\n", rip);
}

void __ubsan_handle_shift_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Shift out of bounds at: %p\n", rip);
}

void __ubsan_handle_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Out of bounds at: %p\n", rip);
}

void __ubsan_handle_local_out_of_bounds_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Local out of bounds at: %p\n", rip);
}

void __ubsan_handle_builtin_unreachable_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Hit builtin_unreachable() at: %p\n", rip);
}

void __ubsan_handle_missing_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Missing return at: %p\n", rip);
}

void __ubsan_handle_vla_bound_not_positive_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] VLA bound not positive at: %p\n", rip);
}

void __ubsan_handle_float_cast_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Float cast overflow at: %p\n", rip);
}

void __ubsan_handle_load_invalid_value_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Load invalid value at: %p\n", rip);
}

void __ubsan_handle_invalid_builtin_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Invalid builtin at: %p\n", rip);
}

void __ubsan_handle_invalid_objc_cast_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Invalid objc cast at: %p\n", rip);
}

void __ubsan_handle_function_type_mismatch_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Function type mismatch at: %p\n", rip);
}

void __ubsan_handle_implicit_conversion_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Implicit conversion at: %p\n", rip);
}

void __ubsan_handle_nonnull_arg_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Nonnull argument at: %p\n", rip);
}

void __ubsan_handle_nonnull_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Nonnull return at: %p\n", rip);
}

void __ubsan_handle_nullability_arg_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Nullability argument at: %p\n", rip);
}

void __ubsan_handle_nullability_return_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Nullability return at: %p\n", rip);
}

void __ubsan_handle_pointer_overflow_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] Pointer overflow at: %p\n", rip);
}

void __ubsan_handle_cfi_check_fail_minimal(void) {
    uintptr_t rip = (uintptr_t)__builtin_return_address(0);
    kpanic("[UBSan] CFI check fail at: %p\n", rip);
}