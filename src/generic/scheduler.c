#include <scheduler.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

static process_t *rootproc = NULL;
static process_t *nextproc = NULL;
static thread_t *current_thread = NULL;

static uint64_t pid_counter = 0;
static uint64_t tid_counter = 0;
static uint64_t taskcount = 0;

static uint64_t getpid(void) {
    return pid_counter++;
}

static uint64_t gettid(void) {
    return tid_counter++;
}

static process_t *findproc(uint64_t pid) {
    process_t *proc = rootproc;

    while (proc != NULL) {
        if (proc->pid == pid)
            return proc;

        proc = proc->next;
    }

    return NULL;
}

static thread_t *findthrd(process_t *proc, uint64_t tid) {
    thread_t *thread = proc->threads;

    while (thread != NULL) {
        if (thread->tid == tid)
            return thread;

        thread = thread->next;
    }

    return NULL;
}

thread_t *schedule(void) {
    if (rootproc == NULL)
        return NULL;

    if (current_thread != NULL &&
        current_thread->state == THREAD_RUNNING) {
        current_thread->state = THREAD_READY;
    }

    process_t *proc = nextproc;

    if (proc == NULL)
        proc = rootproc;

    process_t *start = proc;

    while (proc != NULL) {
        if (proc->threads != NULL) {
            thread_t *thread = findthrd(proc, proc->nexthread);

            if (thread == NULL)
                thread = proc->threads;

            thread_t *startthread = thread;

            while (thread != NULL) {
                if (thread->enabled &&
                    thread->state == THREAD_READY) {

                    if (thread->next != NULL)
                        proc->nexthread = thread->next->tid;
                    else
                        proc->nexthread = proc->threads->tid;

                    nextproc = proc->next;

                    if (nextproc == NULL)
                        nextproc = rootproc;

                    thread->state = THREAD_RUNNING;
                    current_thread = thread;

                    return thread;
                }

                thread = thread->next;

                if (thread == NULL)
                    thread = proc->threads;

                if (thread == startthread)
                    break;
            }
        }

        proc = proc->next;

        if (proc == NULL)
            proc = rootproc;

        if (proc == start)
            break;
    }

    current_thread = NULL;
    return NULL;
}

thread_t *get_current_thread(void) {
    return current_thread;
}

process_t *addproc(void) {
    process_t *proc = kmalloc(sizeof(process_t));

    if (proc == NULL)
        return NULL;

    proc->pid = getpid();
    proc->nexthread = 0;
    proc->threads = NULL;
    proc->archdata = NULL;
    proc->next = NULL;

    if (rootproc == NULL) {
        rootproc = proc;
        nextproc = proc;
        taskcount++;

        return proc;
    }

    process_t *last = rootproc;

    while (last->next != NULL)
        last = last->next;

    last->next = proc;

    taskcount++;

    return proc;
}

bool delproc(uint64_t pid) {
    process_t *proc = rootproc;
    process_t *prev = NULL;

    while (proc != NULL) {
        if (proc->pid == pid) {
            if (prev == NULL)
                rootproc = proc->next;
            else
                prev->next = proc->next;

            if (nextproc == proc) {
                nextproc = proc->next;

                if (nextproc == NULL)
                    nextproc = rootproc;
            }

            thread_t *thread = proc->threads;

            while (thread != NULL) {
                thread_t *next = thread->next;

                if (thread->archdata != NULL)
                    kfree(thread->archdata);

                kfree(thread);

                thread = next;
            }

            if (proc->archdata != NULL)
                kfree(proc->archdata);

            kfree(proc);

            taskcount--;

            if (taskcount == 0)
                nextproc = NULL;

            return true;
        }

        prev = proc;
        proc = proc->next;
    }

    return false;
}

thread_t *addthrd(uint64_t pid) {
    process_t *proc = findproc(pid);

    if (proc == NULL)
        return NULL;

    thread_t *thread = kmalloc(sizeof(thread_t));

    if (thread == NULL)
        return NULL;

    // Set to THREAD_BLOCKED so the kernel can set it up and get it ready before its ran
    thread->state = THREAD_BLOCKED;
    thread->tid = gettid();
    thread->instruction_ptr = 0;
    thread->stack_ptr = 0;
    thread->enabled = true;
    thread->archdata = NULL;
    thread->next = NULL;

    if (proc->threads == NULL) {
        proc->threads = thread;
        proc->nexthread = thread->tid;

        return thread;
    }

    thread_t *last = proc->threads;

    while (last->next != NULL)
        last = last->next;

    last->next = thread;

    return thread;
}

bool delthrd(uint64_t pid, uint64_t tid) {
    process_t *proc = findproc(pid);

    if (proc == NULL) return false;

    thread_t *thread = proc->threads;
    thread_t *prev = NULL;

    while (thread != NULL) {
        if (thread->tid == tid) {

            if (proc->nexthread == tid) {
                if (thread->next != NULL)
                    proc->nexthread = thread->next->tid;
                else if (prev != NULL)
                    proc->nexthread = prev->tid;
                else
                    proc->nexthread = 0;
            }

            if (prev == NULL)
                proc->threads = thread->next;
            else
                prev->next = thread->next;

            if (thread->archdata != NULL)
                kfree(thread->archdata);

            kfree(thread);

            if (proc->threads == NULL)
                proc->nexthread = 0;

            return true;
        }

        prev = thread;
        thread = thread->next;
    }

    return false;
}