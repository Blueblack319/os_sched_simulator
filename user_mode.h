#ifndef USER_MODE_H
#define USER_MODE_H
#include <string>
#include <array>
#include "process.h"
#include "kernel_mode.h"
// #include "utils.h"

/*
참조자를 사용하기 전에 짠 코드
이렇게 코딩하게 되면 &, *가 매우 많이 사용되기 때문에 더러움...
pointer를 사용해서도 kernel simulator 구현에 성공했지만 너무 이쁘지 않아서
더 가독성이 좋은 코드를 짜기 위해 reference를 공부하고 refactoring 성공!
*/
// void run(int *cycle, Process *process_running, deque<Process *> *process_ready, vector<Process *> *process_waiting, int count, string command);
// void sleep(int *cycle, Process *process_running, deque<Process *> *process_ready, vector<Process *> *process_waiting, string command, int sleep_cycle);
// void fork_and_exec(int *cycle, Process *process_running, deque<Process *> *process_ready, vector<Process *> *process_waiting, string command, string path_programs, string program_name, int *last_pid);
// void wait(int *cycle, Process *process_running, deque<Process *> *process_ready, vector<Process *> *process_waiting);
// void exit(int *cycle, Process *process_running, deque<Process *> *process_ready, vector<Process *> *process_waiting);

void run(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int count, string command);
// TODO:
void memory_read(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int pageID, Replacement replacement_algo);
// TODO:
void memory_write(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int pageID, Replacement replacement_algo);

void fork_and_exec(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, string command, string path_programs, string program_name, int &last_pid);

void wait(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);

void exit(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory);
// TODO:
void memory_allocate(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int size, Replacement replacement_algo);
// TODO:
void memory_release(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int allocationID);

#endif