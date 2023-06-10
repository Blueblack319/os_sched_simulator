#ifndef KERNEL_MODE_H
#define KERNEL_MODE_H
#include <fstream>
#include <array>
#include <cassert>
#include <limits.h>
#include "variables.h"
#include "process.h"
#include "utils.h"

void boot(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, string path_programs, int &last_pid);
void schedule(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);
void schedule_for_new(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, Process *&process_new);
void idle(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);
// TODO:
void fault(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int pageID, FaultType type, Replacement replacement_algo);

void system_call_exit(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);
void system_call_wait(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);
Process *system_call_fork_and_exec(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, string path_programs, string program, int &last_pid);
// TODO:
void system_call_memory_allocate(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int size, Replacement replacement_algo);
// TODO:
void system_call_memory_release(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int allocationID);
// TODO:
void replacement(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int pageID, FaultType type, Replacement replacement_algo);

// replacement algorithms
void evict_page_fifo_lru_lfu(Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);
// void evict_page_lru(Process *&process_running, array<FrameInfo, 16> &physicalMemory);
// void evict_page_lfu(Process *&process_running, array<FrameInfo, 16> &physicalMemory);
void evict_page_mfu(Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);
void mapping(Process *&process_running, array<FrameInfo, 16> &physicalMemory, int pageID);

void updateCounter(const int cycle, Process *&process_running, array<FrameInfo, 16> &physicalMemory, int pageID, Replacement replacement_algo);
int checkFreeFrames(const array<FrameInfo, 16> physicalMemory);

#endif