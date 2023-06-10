#include "user_mode.h"

void run(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int count, string command)
{
    /*
    preprocessing으로 sleep cycle & process status 갱신
    현재 status 출력 후 clock cycle 업데이트
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        command,
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
}

void memory_read(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int pageID, Replacement replacement_algo)
{
    /*
    유저 모드에서 1 cycle만에 read 실행
    폴트가 발생하면 폴트 핸들러에서 read 동작
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // pageID가 동일하고 is_valid == true인지
    bool isFault = true;
    for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
    {
        if (process_running->pageTable[i].pageID == pageID && process_running->pageTable[i].is_valid == true)
        {
            isFault = false;
            break;
        }
    }
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        "memory_read " + to_string(pageID),
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;

    // ONLY page fault
    if (isFault)
        fault(cycle, process_running, process_ready, process_waiting, physicalMemory, pageID, PAGE_FAULT, replacement_algo);
    else
        // (6) 명령어 처리가 끝난 다음 cycle부터 reference count 1 증가
        updateCounter(cycle, process_running, physicalMemory, pageID, replacement_algo);
}

void memory_write(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int pageID, Replacement replacement_algo)
{
    /*
    유저 모드에서 1 cycle만에 read 실행
    폴트가 발생하면 폴트 핸들러에서 read 동작
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // pageID가 동일하고 is_valid == true인지
    bool isFault = true;
    FaultType faultType = PAGE_FAULT;
    for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
    {
        if (process_running->pageTable[i].pageID == pageID && process_running->pageTable[i].is_valid == true)
        {
            if (process_running->pageTable[i].protection == WRITE)
                isFault = false;
            else
                // protection fault가 아닌 경우는 모두 page fault
                // READ -> PROTECTION_FAULT
                // is_valid == false -> PAGE_FAULT
                faultType = PROTECTION_FAULT;
            break;
        }
    }
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        "memory_write " + to_string(pageID),
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;

    // page fault or protection fault
    if (isFault)
        fault(cycle, process_running, process_ready, process_waiting, physicalMemory, pageID, faultType, replacement_algo);
    else
        // fault 처리 cycle 끝나고 reference count 1 증가
        // (6) 명령어 처리가 끝난 다음 cycle부터 reference count 1 증가
        updateCounter(cycle, process_running, physicalMemory, pageID, replacement_algo);
}

void fork_and_exec(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, string command, string path_programs, string program_name, int &last_pid)
{
    /*
    init 프로세스만 호출한다고 가정
    preprocessing으로 sleep cycle & process status 갱신
    현재 status 출력 후 clock cycle 업데이트
    system call 호출
    process_running이 NULL이거나 process_ready가 비어있지 않다면 schedule 호출
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        command,
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;

    Process *process_new = system_call_fork_and_exec(cycle, process_running, process_ready, process_waiting, physicalMemory, path_programs, program_name, last_pid);

    schedule_for_new(cycle, process_running, process_ready, process_waiting, physicalMemory, process_new);
}

void wait(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory)
{
    /*
    preprocessing으로 sleep cycle & process status 갱신
    현재 status 출력 후 clock cycle 업데이트
    system call 호출
    process_running이 NULL이거나 process_ready가 비어있지 않다면 schedule 호출
    아니면 idle 호출
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        "wait",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;

    system_call_wait(cycle, process_running, process_ready, process_waiting, physicalMemory);
    if (!process_running && !process_ready.empty())
        schedule(cycle, process_running, process_ready, process_waiting, physicalMemory);
    else
        idle(cycle, process_running, process_ready, process_waiting, physicalMemory);
}

void exit(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory)
{
    /*
    preprocessing으로 sleep cycle & process status 갱신
    현재 status 출력 후 clock cycle 업데이트
    system call 호출
    process_running이 NULL이거나 process_ready가 비어있지 않다면 schedule 호출
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        "exit",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;

    system_call_exit(cycle, process_running, process_ready, process_waiting, physicalMemory);
    /*
    system_call_exit 이후에 runnging process는 항상 없음
    1. ready queue가 비어있다면 program 종료 -> 다른 명령이라면 idle system call 호출
    2. ready queue가 있다면 scheduling
    */
    assert(process_running == NULL);
    if (!process_ready.empty())
        schedule(cycle, process_running, process_ready, process_waiting, physicalMemory);
}

void memory_allocate(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int size, Replacement replacement_algo)
{
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        "memory_allocate " + to_string(size),
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;

    system_call_memory_allocate(cycle, process_running, process_ready, process_waiting, physicalMemory, size, replacement_algo);
    if (!process_running && !process_ready.empty())
        schedule(cycle, process_running, process_ready, process_waiting, physicalMemory);
    else
        idle(cycle, process_running, process_ready, process_waiting, physicalMemory);
}

void memory_release(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int allocationID)
{
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "user",
        "memory_release " + to_string(allocationID),
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;

    system_call_memory_release(cycle, process_running, process_ready, process_waiting, physicalMemory, allocationID);
    if (!process_running && !process_ready.empty())
        schedule(cycle, process_running, process_ready, process_waiting, physicalMemory);
    else
        idle(cycle, process_running, process_ready, process_waiting, physicalMemory);
}