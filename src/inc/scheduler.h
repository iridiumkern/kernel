#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum thread_state {
    THREAD_READY = 0,
    THREAD_RUNNING = 1,
    THREAD_BLOCKED = 2
}thread_state;

typedef struct thread_t {
    thread_state state;
    uint64_t tid;
    uintptr_t instruction_ptr;
    uintptr_t stack_ptr;
    bool enabled; // Does the thread function
    void* archdata; // Architecture specific things, registers, the like.
    struct thread_t *next;
}thread_t;

typedef struct process_t {
    uint64_t pid;
    uint64_t nexthread;
    thread_t *threads;
    void* archdata; // Holds process level things, basically just the CR3 and such
    struct process_t *next;
}process_t;

/**
 * @brief Gives the next runnable thread
 * 
 * @return thread_t* The next thread (which can run)
 */
thread_t *schedule(void);

/**
 * @brief Adds/Creates a process (allocates a PID too)
 * 
 * @return process_t* The allocated process
 */
process_t *addproc(void);

/**
 * @brief Delets/kills a process
 * 
 * @param pid The PID of the process
 * @return true The process was killed
 * @return false The process was not killed
 */
bool delproc(uint64_t pid);

/**
 * @brief Creates a new thread
 * 
 * @param pid The PID to create a thread in
 * @return thread_t* The thread
 */
thread_t *addthrd(uint64_t pid);

/**
 * @brief Deletes/kills a thread
 * 
 * @param pid The PID the thread is in
 * @param tid The thread
 * @return true The thread was killed
 * @return false The thread was not killed
 */
bool delthrd(uint64_t pid, uint64_t tid);

/**
 * @brief Get the current thread object
 * 
 * @return thread_t* The current thread
 */
thread_t *get_current_thread(void);