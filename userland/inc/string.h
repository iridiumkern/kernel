#pragma once

#include <stddef.h>

/**
 * @brief Copies memory from src to dest
 * 
 * @param dest The destination
 * @param src The source
 * @param n Amount of bytes to copy
 * @return void* pointer to dest
 */
void *memcpy(void *restrict dest, const void *restrict src, size_t n);

/**
 * @brief Sets memory to a specific value
 * 
 * @param s The pointer
 * @param c The value to set it to
 * @param n The amount of bytes to wipe
 * @return void* Pointer to s
 */
void *memset(void *s, int c, size_t n);
/**
 * @brief Moves memory from one place to another
 * 
 * @param dest Destination
 * @param src Source
 * @param n Amount of bytes to move
 * @return void* Pointer to dest
 */
void *memmove(void *dest, const void *src, size_t n);

/**
 * @brief Compares two places in memory
 * 
 * @param s1 Pointer 1
 * @param s2 Pointer 2
 * @param n The amount of bytes to compare
 * @return int -1 if the first byte in s1 is LESS THAN the first byte in s2, 0 if n bytes are equal, 1 if the first differing byte in s1 is greater than the first byte in s2
 */
int memcmp(const void *s1, const void *s2, size_t n);

/**
 * @brief Copies a (null terminated) string
 * 
 * @param dest The destination string
 * @param src The source string
 * @return char* Dest
 */
char *strcpy(char *dest, const char *src);

/**
 * @brief Appends one (null terminated) string to another
 * 
 * @param dest Destrination string, must have enough space to hold both.
 * @param src String to append to the end of the dest string
 * @return char* Pointer to dest/new string
 */
char *strcat(char *dest, const char *src);

/**
 * @brief Finds the length of a null terminated string
 * 
 * @param str The string
 * @return size_t The size
 */
size_t strlen(const char *str);