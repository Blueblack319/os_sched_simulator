#include "process.h"
#include "user_mode.h"
#include "kernel_mode.h"
#include "utils.h"
#include "variables.h"

using namespace std;

int main(int argc, char **argv)
{
    // kernel의 자료구조
    int cycle = 0;
    int last_pid = 0;
    Process *process_running;
    deque<Process *> process_ready;
    vector<Process *> process_waiting;

    // 프로그램이 있는 디렉토리 & Replacement algorightm
    string path_programs = argv[1];
    Replacement REPLACEMENT_ALGO;
    string algo = argv[2];
    if (algo == "lru")
        REPLACEMENT_ALGO = LRU;
    else if (algo == "fifo")
        REPLACEMENT_ALGO = FIFO;
    else if (algo == "lfu")
        REPLACEMENT_ALGO = LFU;
    else if (algo == "mfu")
        REPLACEMENT_ALGO = MFU;
    assert(REPLACEMENT_ALGO == LRU || REPLACEMENT_ALGO == FIFO || REPLACEMENT_ALGO == LFU || REPLACEMENT_ALGO == MFU);

    // TODO: physical memory
    array<FrameInfo, 16> physicalMemory;
    for (auto &frame : physicalMemory)
    {
        frame.pageID = -1;
        frame.pid = -1;
        frame.ref_count = -1;
        frame.isExist = false;
    }

    // init process 생성
    boot(cycle, process_running, process_ready, process_waiting, physicalMemory, path_programs, last_pid);
    schedule(cycle, process_running, process_ready, process_waiting, physicalMemory);

    /*
    running process가 있거나 ready queue가 비어있지 않거나 process_waiting이 비어있지 않으면
    process_running의 code를 1개씩 읽기
    */
    while (process_running || !process_ready.empty() || !process_waiting.empty())
    {
        // process_running의 code 중 맨 첫 번째 command 읽기
        vector<string> command_split;
        string command = process_running->code.front();
        process_running->code.pop_front();
        customSplit(command_split, command, ' ');
        string op = command_split[0];
        // system call: memory_allocate, memory_release, fork_and_exec, wait, exit

        // User mode: memory_read, memory_write, run, system call(fork_and_exec, wait, exit, memory_allocate, memory_release)
        // INSTRUCTIONS: memory_read, memory_write, run, memory_allocate, memory_release, fork_and_exec, wait, exit

        /*
        Kernel mode: system call(system_call_memory_allocate, system_call_memory_release,
                    system_call_fork_and_exec, system_call_wait, system_call_exit),
                    boot, schedule, idle,
                    fault(page fault, protection fault)
        */
        /*
        SYSTEM_CALL: system_call_memory_allocate, system_call_memory_release, system_call_fork_and_exec, system_call_wait, system_call_exit
        */
        // page fault or protection fault가 날 만한 instruction?
        // memory_read, memory_write, memory_allocate

        {
            cout << op << endl;
            // Common instructions(not system calls)
            if (!op.compare("run"))
            {
                // N번 run 명령어 실행
                int count = stoi(command_split[1]);
                while (count)
                {
                    run(cycle, process_running, process_ready, process_waiting, physicalMemory, count, command);
                    count--;
                }
            }
            else if (!op.compare("memory_read"))
            {
                // memory_read 명령어 실행
                int id = stoi(command_split[1]);
                memory_read(cycle, process_running, process_ready, process_waiting, physicalMemory, id, REPLACEMENT_ALGO);
            }
            else if (!op.compare("memory_write"))
            {
                // memory_write 명령어 실행
                int id = stoi(command_split[1]);
                memory_write(cycle, process_running, process_ready, process_waiting, physicalMemory, id, REPLACEMENT_ALGO);
            }
            // System calls
            else if (!op.compare("fork_and_exec"))
            {
                // fork_and_exec 명령어 실행
                string program_name = command_split[1];
                fork_and_exec(cycle, process_running, process_ready, process_waiting, physicalMemory, command, path_programs, program_name, last_pid);
            }
            else if (!op.compare("wait"))
                // wait 명령어 실행
                wait(cycle, process_running, process_ready, process_waiting, physicalMemory);
            else if (!op.compare("exit"))
                // exit 명령어 실행
                exit(cycle, process_running, process_ready, process_waiting, physicalMemory);
            else if (!op.compare("memory_allocate"))
            {
                // memory_allocate 명령어 실행
                int size = stoi(command_split[1]);
                memory_allocate(cycle, process_running, process_ready, process_waiting, physicalMemory, size, REPLACEMENT_ALGO);
            }
            else if (!op.compare("memory_release"))
            {
                // memory_release 명령어 실행
                int id = stoi(command_split[1]);
                memory_release(cycle, process_running, process_ready, process_waiting, physicalMemory, id);
            }
        }
    }
    return 0;
}