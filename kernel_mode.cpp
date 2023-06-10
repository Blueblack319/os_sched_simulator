#include "kernel_mode.h"

void boot(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, string path_programs, int &last_pid)
{
    // (1),(2),(3) skip, 처음 동작하는 명령어임으로 할 필요 없음
    // (4) 명령어 실행
    array<PageInfo, 32> virtualMemory;
    array<PageTableEntry, 32> pageTable;
    for (auto &page : virtualMemory)
    {
        page.allocationID = -1;
        page.pageID = -1;
        page.isExist = false;
    }
    for (auto &entry : pageTable)
    {
        entry.frameID = -1;
        entry.pageID = -1;
        entry.is_valid = false;
        entry.protection = WRITE;
    }

    process_running = new Process{
        "init",
        last_pid + 1,
        last_pid,
        '0',
        0,
        {},
        NEW,
        0,
        0,
        virtualMemory,
        pageTable};
    last_pid += 1;
    path_programs += "/init";

    ifstream file;
    string command;
    file.open(path_programs, ios::in);

    if (file.is_open())
        while (getline(file, command))
            process_running->code.push_back(command);
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "boot",
        NULL,
        {},
        {},
        process_running,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
    process_ready.push_back(process_running);
}

// TODO: 특정 cycle에 동시에 ready 상태가 되는 여러 프로세스가 있는 경우 해결하기
void schedule(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory)
{
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // ready queue 한 번 더 갱신
    preprocessing(process_ready, process_waiting);
    process_running = process_ready.front();
    process_ready.pop_front();
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "schedule",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
}

void schedule_for_new(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, Process *&process_new)
{
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // ready queue 한 번 더 갱신
    // waiting -> ready가 먼저 일어난다.
    preprocessing(process_ready, process_waiting);
    // new -> ready
    process_new->state = READY;
    process_ready.push_back(process_new);
    process_running = process_ready.front();
    process_ready.pop_front();
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "schedule",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
}

void idle(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory)
{
    // ready queue가 비어있는 경우
    while (process_ready.empty())
    {
        // (1),(2),(3) preprocessing
        preprocessing(process_ready, process_waiting);
        // (4) 명령어 실행
        // (5) 해당 cycle 실행 직후 결과 출력
        Status current = {
            cycle,
            "kernel",
            "idle",
            process_running,
            process_ready,
            process_waiting,
            NULL,
            NULL,
            physicalMemory};
        print_status(current);
        cycle++;
    }
    schedule(cycle, process_running, process_ready, process_waiting, physicalMemory);
}

void fault(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int pageID, FaultType type, Replacement replacement_algo)
{
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    /*
    PAGE_FAULT
    case1. physical memory에 빈 frame이 있는 경우
        1) pageID physical memory에 할당
    case2. physical memory에 빈 frame이 없는 경우
        1) Replacement algorithm에 따라 physical memory에 frame 1개 확보
        2) pageID physical memory에 할당

    PROTECTION_FAULT
    parent process와 child process의 behavior가 다름
    Parent process
    Protection fault가 발생하여 커널 모드로 전환되고, fault handler에 의해
    부모와 자식 프로세스 모두에서 해당 페이지가 ‘W 권한’으로 변경된다. 부모 프로세스는 새로운
    프레임을 생성하지는 않고, 기존의 프레임을 그대로 활용한다. 이 과정에서 자식 프로세스들의
    새로운 물리 프레임 할당까지 이어지지는 않는다.
    1) parent, child 모두 W 권한으로 변경

    Child process
     Protection fault가 발생하여 커널 모드로 전환되고, fault handler에 의해
    새로운 프레임이 생성되고 물리 메모리에 할당된다. 이때, 물리 메모리에 할당할 공간이 없다면
    protection fault handler가 처리되고 있는 같은 cycle에 페이지 교체 알고리즘에 의한 페이지
    교체까지 진행한다. 이 새로운 프레임 할당은 해당 자식 프로세스에 대해서만 이루어지고, 또다른
    자식 프로세스가 해당 페이지를 공유하고 있었다면, 다른 자식 프로세스에 대한 프레임 할당을 이
    과정에서 하지는 않는다.
    case1. physical memory에 빈 frame이 있는 경우
        1) pageID physical memory에 할당
    case1. physical memory에 빈 frame이 없는 경우
        1) Replacement algorithm에 따라 physical memory에 frame 1개 확보
        2) 해당 자식 process의 page만 pageID physical memory에 할당
    */
    Protection protectionMapped;
    if (type == PAGE_FAULT)
    {
        // 빈 frame이 없는 경우
        // replacement algorithm에 따라 frame 확보
        int numFreed = checkFreeFrames(physicalMemory);
        if (numFreed == 0)
            evict_page(process_running, process_ready, process_waiting, physicalMemory, replacement_algo);
        // 빈 frame이 있는 경우
        protectionMapped = mapping(process_running, physicalMemory, pageID);
    }
    // PROTECTION_FAULT
    /*
    1. 부모와 자식 모두에서 권한을 W로 바꾸기 => pageTable
    2. Update physical memory: pid, ref_count
    */
    else
    {
        protectionMapped = WRITE;
        /*
        부모인 경우 <=> init인 경우 => ppid == 0
        Protection fault가 발생하여 커널 모드로 전환되고, fault handler에 의해
        부모와 자식 프로세스 모두에서 해당 페이지가 ‘W 권한’으로 변경된다. 부모 프로세스는 새로운
        프레임을 생성하지는 않고, 기존의 프레임을 그대로 활용한다. 이 과정에서 자식 프로세스들의
        새로운 물리 프레임 할당까지 이어지지는 않는다.
        */
        if (process_running->ppid == 0)
        {
            for (auto &entry : process_running->pageTable)
                if (entry.pageID == pageID)
                {
                    entry.protection = WRITE;
                    break;
                }
            // 자식 프로세스로 접근은 어떻게? => readyQ에서 찾기
            // 1. 자식 프로세스의 readyQ index 찾기
            // 2. 부모, 자식 모두 W 권한으로 변경, 부모는 나머지 FrameInfo, PageInfo, PageTableEntry 그대로 유지
            // 3. 자식은 pageTableEntry의 is_valid = false
            // TODO: 나중에 자식이 memory_read or memory_write 하면 page fault가 떠야 함
            for (auto &process : process_ready)
                for (auto &entry : process->pageTable)
                    // pageID는 동일하지만 protection이 WRITE인 경우는 이미 독립적인 page!
                    if (entry.pageID == pageID && entry.protection == READ)
                    {
                        entry.protection = WRITE;
                        entry.is_valid = false;
                        break;
                    }
        }
        /*
        자식인 경우 => ppid == 1
        Protection fault가 발생하여 커널 모드로 전환되고, fault handler에 의해
        새로운 프레임이 생성되고 물리 메모리에 할당된다. 이때, 물리 메모리에 할당할 공간이 없다면
        protection fault handler가 처리되고 있는 같은 cycle에 페이지 교체 알고리즘에 의한 페이지
        교체까지 진행한다. 이 새로운 프레임 할당은 해당 자식 프로세스에 대해서만 이루어지고, 또다른
        자식 프로세스가 해당 페이지를 공유하고 있었다면, 다른 자식 프로세스에 대한 프레임 할당을 이
        과정에서 하지는 않는다.
        */
        else
        {
            // 부모의 권한 변경
            for (auto &process : process_ready)
                if (process->pid == 1)
                {
                    for (auto &entry : process->pageTable)
                        if (entry.pageID == pageID)
                        {
                            entry.protection = WRITE;
                            break;
                        }
                    break;
                }
            for (auto &process : process_waiting)
                if (process->pid == 1)
                {
                    for (auto &entry : process->pageTable)
                        if (entry.pageID == pageID)
                        {
                            entry.protection = WRITE;
                            break;
                        }
                    break;
                }

            // 자식의 권한 변경
            // 여기서 자식은 가상 메모리에 page는 있지만 물리 메모리에 frame은 없는 상황
            for (auto &entry : process_running->pageTable)
                // pageID는 동일하지만 protection이 WRITE인 경우는 이미 독립적인 page!
                if (entry.pageID == pageID && entry.protection == READ)
                {
                    entry.protection = WRITE;
                    entry.is_valid = false;
                    break;
                }

            // 자식 프로세스에 새로운 프레임이 생성되고 물리 메모리에 할당
            // 빈 frame 확보
            // 여기서 부모의 frame을 뺏어야 해
            int numFreed = checkFreeFrames(physicalMemory);
            if (numFreed == 0)
                evict_page(process_running, process_ready, process_waiting, physicalMemory, replacement_algo);

            numFreed = checkFreeFrames(physicalMemory);
            assert(numFreed > 0);

            Protection protectionMapped = mapping(process_running, physicalMemory, pageID);
            numFreed = checkFreeFrames(physicalMemory);
            assert(numFreed == 0);

            // TODO: ============================DEBUGGING============================
            // 1. pageID에 해당하는 page가 물리 메모리에 frame 확보
            FrameInfo frameMapped = {-1, -1, -1, false};
            for (auto frame : physicalMemory)
            {
                if (frame.isExist == true && frame.pageID == pageID && frame.pid == process_running->pid)
                {
                    frameMapped = frame;
                    break;
                }
            }
            assert(frameMapped.ref_count == 0);

            // 2. 부모, 자식 모두 W 권한으로 바뀌었는지
            PageTableEntry parent = {-1, -1, false, READ};
            PageTableEntry child = {-1, -1, false, READ};
            for (auto entry : process_running->pageTable)
            {
                if (entry.is_valid == true && entry.pageID == pageID)
                {
                    child = entry;
                    break;
                }
            }

            for (auto process : process_ready)
            {
                if (process->pid == 1)
                {
                    for (auto entry : process->pageTable)
                    {
                        if (entry.pageID == pageID)
                        {
                            parent = entry;
                            break;
                        }
                    }
                    break;
                }
            }
            for (auto process : process_waiting)
            {
                if (process->pid == 1)
                {
                    for (auto entry : process->pageTable)
                    {
                        if (entry.pageID == pageID)
                        {
                            parent = entry;
                            break;
                        }
                    }
                    break;
                }
            }
            assert(child.protection == WRITE);
            assert(parent.protection == WRITE);
        }
    }

    process_running->state = READY;
    process_ready.push_back(process_running);
    process_running = NULL;

    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "fault",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
    updateCounter(cycle, process_ready.back(), physicalMemory, pageID, replacement_algo, protectionMapped);
    schedule(cycle, process_running, process_ready, process_waiting, physicalMemory);
}

void system_call_exit(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory)
{
    /*
    (주의) wait 명령으로 인해 waiting하고 있는 부모 프로세스가 ready로 바뀌어 ready queue에
    삽입되는 시점은 자식 프로세스의 exit 명령어에 대한 시스템 콜 처리 시점임에 주의한다.
    정확히는 해당 프로세스의 memory_release 연산들이 모두 실행되는 결과와 같다.
    즉, read 권한만 있는 경우에는 물리 메모리의 해당 프레임을 해제하지는 않는다.
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    Process *process_terminated = process_running;
    process_running = NULL;
    Process *process_parent = NULL;
    for (vector<Process *>::iterator iter = process_waiting.begin(); iter != process_waiting.end(); iter++)
    {
        if ((*iter)->waiting_type == 'W' && (*iter)->pid == process_terminated->ppid)
        {
            process_parent = *iter;
            process_waiting.erase(iter--);
            break;
        }
    }
    // 해당 프로세스의 memory_release 연산들이 모두 실행
    // 즉, read 권한만 있는 경우에는 물리 메모리의 해당 프레임을 해제하지는 않는다.
    for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
    {
        if (process_terminated->pageTable[i].is_valid == true && process_terminated->pageTable[i].protection != READ)
        {
            physicalMemory[process_terminated->pageTable[i].frameID].isExist = false;
            physicalMemory[process_terminated->pageTable[i].frameID].pageID = -1;
            physicalMemory[process_terminated->pageTable[i].frameID].pid = -1;
            physicalMemory[process_terminated->pageTable[i].frameID].ref_count = -1;
        }
    }

    // 부모 프로세스: wait -> ready
    if (process_parent)
        process_ready.push_back(process_parent);

    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "system call",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        process_terminated,
        physicalMemory};
    print_status(current);
    delete process_terminated;
    cycle++;
}

void system_call_wait(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory)
{
    /*
    process_running을 process_waiting에 넣기
    process_running NULL
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // 자식 프로세스 찾기
    bool exist = false;
    // waiting 상태의 자식 프로세스 찾기
    for (vector<Process *>::iterator iter = process_waiting.begin(); iter != process_waiting.end(); iter++)
    {
        // 자식 프로세스의 ppid와 부모 프로세스의 pid 비교
        if ((*iter)->ppid == process_running->pid)
        {
            exist = true;
            break;
        }
    }
    // ready 상태의 자식 프로세스 찾기
    for (deque<Process *>::iterator iter = process_ready.begin(); iter != process_ready.end(); iter++)
    {
        // 자식 프로세스의 ppid와 부모 프로세스의 pid 비교
        if ((*iter)->ppid == process_running->pid)
        {
            exist = true;
            break;
        }
    }

    if (exist)
    {
        // 종료되지 않은 자식 프로세스가 존재하는 경우
        process_running->state = WAITING;
        process_running->waiting_type = 'W';
        process_waiting.push_back(process_running);
    }
    else
    {
        // 종료되지 않은 자식 프로세스가 존재하지 않는 경우
        process_running->state = READY;
        process_ready.push_back(process_running);
    }
    process_running = NULL;
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "system call",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
}

Process *system_call_fork_and_exec(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, string path_programs, string program, int &last_pid)
{
    /*
    프로세스 ID는 다르지만 이름이 같은 프로그램이 여러개 실행될 수 있다.
    Create a new process
    ready queue에 넣기
    생성되는 시점(즉, 시스템 콜 처리 과정)에 부모 프로세스의 가상 메모리와 페이지 테이블을 Copy-on-Write 형식으로 복사
    allocationID 정보도 같이 복사
    CoW로 복사된 부모 프로세스와 자식 프로세스의 페이지는 동일한 물리 메모리의 프레임을 가리킴
    나중에 write 명령이 들어올 때 새로운 프레임으로 복사
    부모, 자식이 공유하는 페이지는 R 권한
    */
    // waiting -> ready가 먼저 일어난다
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    Process *process_new = new Process{program, last_pid + 1, process_running->pid, '0', 0, {}, NEW};
    last_pid += 1;
    path_programs += "/" + program;

    ifstream file;
    string command;
    file.open(path_programs, ios::in);

    if (file.is_open())
        while (getline(file, command))
            process_new->code.push_back(command);

    /*
    1. 부모 프로세스의 pageTableEntry를 R 권한으로 수정
    TODO: 나중에 자식 프로세스가 exit 한다면? 다시 부모 프로세스의 모든 pageTableEntry의 권한은 W로 변경?
    virtual memory에 할당되어 있는 모든 page를 R 권한으로 수정
    */
    for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
        if (process_running->virtualMemory[i].isExist)
            process_running->pageTable[i].protection = READ;

    // 2. 자식 프로세스로 부모 프로세스의 가상 메모리, 페이지 테이블 복사
    process_new->currentAllocationID = process_running->currentAllocationID;
    process_new->currentPageID = process_running->currentPageID;
    process_new->virtualMemory = process_running->virtualMemory;
    process_new->pageTable = process_running->pageTable;

    // running -> ready
    // 부모 프로세스 역시 scheduling의 대상이므로 process_ready의 맨 뒤에 삽입한다.
    process_running->state = READY;
    process_ready.push_back(process_running);
    process_running = NULL;
    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "system call",
        process_running,
        process_ready,
        process_waiting,
        process_new,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
    return process_new;
}

void system_call_memory_allocate(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int allocated_size, Replacement replacement_algo)
{
    /*
    가상 메모리는 연속적으로 할당, N개의 페이지가 연속으로 할당될 수 있는 가장 하위의 가상 메모리 공간에 할당된다.
    물리 메모리는 비연속적으로 할당해도 돼
    가상 메모리 공간이 부족한 경우는 고려하지 않음
    최대 메모리 할당 allocated_size = 16 => 물리 메모리보다 큰 사이즈로 할당하지는 않음
    pageID: 0 부터 1씩 증가, 부모 프로세스의 마지막 pageID에서 시작
            32 이상의 pageID는 어떻게 처리? pageID와 virtual memory address는 독립적
    allocationID: 0 부터 1씩 증가, 부모 프로세스로부터 allocation ID 정보 복사해오기, 해당 ID 부터 시작
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    // physical memory에 할당할 공간이 부족한 경우 => 필요한 만큼만 빈 프레임 확보
    int numFreed = checkFreeFrames(physicalMemory);
    if (allocated_size > numFreed)
    {
        int required = allocated_size - numFreed;
        for (int i = 0; i < required; i++)
            evict_page(process_running, process_ready, process_waiting, physicalMemory, replacement_algo);

        // TODO: DEBUGGING
        int check = 0;
        for (int i = 0; i < PHYSICAL_MEM_SIZE; i++)
        {
            if (physicalMemory[i].isExist == false)
                check++;
        }
        assert(check == allocated_size);
    }

    // currentPageID ~ currentPage + allocated_size까지 virtualMemory에 연속적으로 할당
    // N개의 페이지가 연속으로 할당될 수 있는 가장 하위의 가상 메모리 주소 찾기
    int startIdx = 0;
    for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
    {
        bool found = true;
        for (int j = i; j < i + allocated_size; j++)
        {
            if (process_running->virtualMemory[j].isExist)
                found = false;
        }
        if (found)
        {
            startIdx = i;
            break;
        }
    }

    int startID = process_running->currentPageID;
    int currentID = startID;

    for (int i = startIdx; i < startIdx + allocated_size; i++)
    {
        process_running->virtualMemory[i].pageID = currentID;
        process_running->virtualMemory[i].allocationID = process_running->currentAllocationID;
        process_running->virtualMemory[i].isExist = true;
        currentID++;
    }

    // physicalMemory에 비연속적으로 할당 <- 빈 프레임 allocated_size만큼 확보 보장
    // 페이지 테이블에 allocation 정보 기록
    int frameID = 0;
    int pageIDIdx = 0;
    int s = 0;
    for (int i = 0; i < allocated_size; i++)
    {
        for (int j = 0; j < PHYSICAL_MEM_SIZE; j++)
        {
            if (!physicalMemory[j].isExist)
            {
                physicalMemory[j].pageID = process_running->currentPageID + pageIDIdx;
                physicalMemory[j].pid = process_running->pid;
                physicalMemory[j].isExist = true;
                physicalMemory[j].ref_count = 0;
                process_running->pageTable[startIdx + pageIDIdx].frameID = j;
                process_running->pageTable[startIdx + pageIDIdx].pageID = process_running->currentPageID + pageIDIdx;
                process_running->pageTable[startIdx + pageIDIdx].is_valid = true;
                process_running->pageTable[startIdx + pageIDIdx].protection = WRITE;
                pageIDIdx++;
                s++;
                break;
            }
        }
    }

    assert(pageIDIdx == allocated_size);
    assert(s == allocated_size);
    // currentAllocationID & currentPageID 업데이트
    assert(currentID == process_running->currentPageID + allocated_size);
    process_running->currentAllocationID += 1;
    process_running->currentPageID += allocated_size;

    // running -> ready
    // 부모 프로세스 역시 scheduling의 대상이므로 process_ready의 맨 뒤에 삽입한다.
    process_running->state = READY;
    process_ready.push_back(process_running);
    process_running = NULL;

    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "system call",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
    // (6) 명령어 처리가 끝난 다음 cycle부터 reference count 1 증가
    for (int i = 0; i < allocated_size; i++)
    {
        updateCounter(cycle, process_ready.back(), physicalMemory, startID + i, replacement_algo, WRITE);
    }
}

void system_call_memory_release(int &cycle, Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, int allocationID)
{

    /*
    allocationID에 해당하는 페이지들을 가상 메모리에서 해제, 물리 메모리에도 존재한다면 물리 메모리에서도 해제
    CoW로 공유되어 있다면(R 권한)
    1. allocationID에 해당하는 페이지들을 부모, 자식 프로세스 모두에서 W 권한으로 변경 => 지금부터는 부모, 자식 간 서로 다른 페이지들로 인식
    2. 이 시점에서 복사된 페이지가 바로 물리 메모리에 할당되지는 않는다. 추후 read, write 명령어 실행 시 폴트가 발생하고 그때 물리 메모리에 할당
    3.
    */
    // (1),(2),(3) preprocessing
    preprocessing(process_ready, process_waiting);
    // (4) 명령어 실행
    /*
    1. W 권한인 경우
        1) 가상 메모리에서 해제
        2) 물리 메모리에 할당되어 있다면 물리 메모리에서도 해제
    2. R 권한인 경우
        (1) 먼저, page X를 각자의 프로세스로 복사하고, 'R' 권한을 'W' 권한으로 변경합니다.

        (2) 그 다음 page X를 물리 메모리에서 해제합니다.
        (2)를 처리하는 과정에서 process A (부모)와 process B (자식)의 처리 방식이 다를 것 같습니다.
        Case 1. process A가 memory_release를 호출한 경우, page X가 물리 메모리에서 해제됩니다.
        Case 2. process B가 memory_release를 호출한 경우, 현재 page X가 process A의 것이므로 page X를 물리 메모리에서 해제하지 않습니다.
        이 시스템 콜 처리시점에서 폴트 핸들러까지 이어지지는 않기 때문에, 이 시점에서 process B의 page X의 실제 물리 메모리 할당이 발생하지 않습니다.
        따라서, page X가 process B의 가상 메모리에서 제거되기만 하면 됩니다.
    */
    for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
    {
        // allocationID에 해당하는 page들에 대해 수행
        if (process_running->virtualMemory[i].isExist && process_running->virtualMemory[i].allocationID == allocationID)
        {
            // 이미 독립적인 경우 -> 가상, 물리 메모리에서 해제
            if (process_running->pageTable[i].protection == WRITE)
            {
                // 물리 메모리에 할당되어 있는 경우, 물리 메모리에서 해제
                if (process_running->pageTable[i].is_valid)
                {
                    physicalMemory[process_running->pageTable[i].frameID].isExist = false;
                    physicalMemory[process_running->pageTable[i].frameID].pageID = -1;
                    physicalMemory[process_running->pageTable[i].frameID].pid = -1;
                    physicalMemory[process_running->pageTable[i].frameID].ref_count = -1;
                }
                // 가상 메모리에서 해제
                process_running->virtualMemory[i].isExist = false;
                process_running->virtualMemory[i].allocationID = -1;
                process_running->virtualMemory[i].pageID = -1;
                process_running->pageTable[i].is_valid = false;
                process_running->pageTable[i].frameID = -1;
                process_running->pageTable[i].pageID = -1;
                process_running->pageTable[i].protection = WRITE;
            }
            // CoW로 부모와 자식이 공유중인 R권한인 경우
            else
            {
                //
                /*
                1. page X를 각자의 프로세스로 복사하고, 'R' 권한을 'W' 권한으로 변경
                    1) 부모인 경우, 권한만 변경
                    2) 자식인 경우, is_valid와 권한 변경
                2. 가상 메모리에서 해제하고 물리 메모리에 할당되어 있다면 그것 또한 해제
                    1) 부모인 경우, 가상 메모리, 물리 메모리 모두 해제
                    2) 자식인 경우, 가상 메모리만 해제
                */
                int pageID = process_running->virtualMemory[i].pageID;

                // 부모가 호출한 경우
                if (process_running->pid == 1)
                {
                    // 1. 권한 변경
                    // 부모 권한 변경
                    process_running->pageTable[i].protection = WRITE;
                    // 자식 권한 변경, 자식은 무조건 readyQ에 존재
                    for (auto &process : process_ready)
                    {
                        // break하면 안돼, 모든 자식들에 대해 적용
                        if (process->ppid == 1)
                        {
                            for (int j = 0; j < VIRTUAL_MEM_SIZE; j++)
                            {
                                if (process->virtualMemory[j].allocationID == allocationID && process->virtualMemory[j].pageID == pageID && process->pageTable[j].protection == READ)
                                {
                                    process->pageTable[j].is_valid = false;
                                    process->pageTable[j].protection = WRITE;
                                    break;
                                }
                            }
                        }
                    }
                    // 2. 메모리 해제
                    // 부모의 가상 메모리 해제, 물리 메모리에 할당되어 있다면 그것 또한 해제
                    // 1) 물리 메모리에 할당되어 있는 경우
                    if (process_running->pageTable[i].is_valid)
                    {
                        physicalMemory[process_running->pageTable[i].frameID].isExist = false;
                        physicalMemory[process_running->pageTable[i].frameID].pageID = -1;
                        physicalMemory[process_running->pageTable[i].frameID].pid = -1;
                        physicalMemory[process_running->pageTable[i].frameID].ref_count = -1;
                    }
                    // 2) 가상 메모리 해제
                    process_running->virtualMemory[i].isExist = false;
                    process_running->virtualMemory[i].allocationID = -1;
                    process_running->virtualMemory[i].pageID = -1;
                    process_running->pageTable[i].is_valid = false;
                    process_running->pageTable[i].frameID = -1;
                    process_running->pageTable[i].pageID = -1;
                    process_running->pageTable[i].protection = WRITE;
                }
                // 자식이 호출한 경우
                else
                {
                    // 1. 권한 변경
                    // 자식 권한 변경
                    process_running->pageTable[i].is_valid = false;
                    process_running->pageTable[i].protection = WRITE;
                    // 부모 권한 변경
                    // 부모는 readyQ 혹은 waitQ에 존재
                    for (auto &process : process_ready)
                    {
                        if (process->pid == 1)
                        {
                            // 해제할 page 찾기 => allocationID, pageID, protection
                            for (int j = 0; j < VIRTUAL_MEM_SIZE; j++)
                            {
                                if (process->virtualMemory[j].allocationID == allocationID && process->virtualMemory[j].pageID == pageID && process->pageTable[j].protection == READ)
                                {
                                    process->pageTable[j].protection = WRITE;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    for (auto &process : process_waiting)
                    {
                        if (process->pid == 1)
                        {
                            // 해제할 page 찾기 => allocationID, pageID, protection
                            for (int j = 0; j < VIRTUAL_MEM_SIZE; j++)
                            {
                                if (process->virtualMemory[j].allocationID == allocationID && process->virtualMemory[j].pageID == pageID && process->pageTable[j].protection == READ)
                                {
                                    process->pageTable[j].protection = WRITE;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    // 2. 메모리 해제
                    // 자식의 가상 메모리 해제, R 권한이었던 것이기 때문에 무조건 물리 메모리에 할당되어 있지 않음
                    process_running->virtualMemory[i].isExist = false;
                    process_running->virtualMemory[i].allocationID = -1;
                    process_running->virtualMemory[i].pageID = -1;
                    process_running->pageTable[i].is_valid = false;
                    process_running->pageTable[i].frameID = -1;
                    process_running->pageTable[i].pageID = -1;
                    process_running->pageTable[i].protection = WRITE;
                }
            }
        }
    }

    // running -> ready
    // 부모 프로세스 역시 scheduling의 대상이므로 process_ready의 맨 뒤에 삽입한다.
    process_running->state = READY;
    process_ready.push_back(process_running);
    process_running = NULL;

    // (5) 해당 cycle 실행 직후 결과 출력
    Status current = {
        cycle,
        "kernel",
        "system call",
        process_running,
        process_ready,
        process_waiting,
        NULL,
        NULL,
        physicalMemory};
    print_status(current);
    cycle++;
}

/*
한 개의 frame만 확보하는 algorithm
*/
//
void evict_page(Process *&process_running, deque<Process *> &process_ready, vector<Process *> &process_waiting, array<FrameInfo, 16> &physicalMemory, Replacement replacement_algo)
{
    // 1. Find frame that has minimum ref_count
    // 2. evit page from the frame
    // 3. Update page table => R 권한 이라면 부모의 page table 역시 update!
    int ref = -1;
    if (replacement_algo == MFU)
    {
        ref = INT_MIN;
        for (auto &frame : physicalMemory)
            if (frame.isExist)
            {
                ref = ref < frame.ref_count ? frame.ref_count : ref;
            }
    }
    // FIFO, LRU, LFU
    else
    {
        ref = INT_MAX;
        for (auto &frame : physicalMemory)
            if (frame.isExist)
            {
                ref = ref > frame.ref_count ? frame.ref_count : ref;
            }
    }

    // for (auto &frame : physicalMemory)
    //     if (frame.isExist)
    //     {
    //         min_ref = min_ref > frame.ref_count ? frame.ref_count : min_ref;
    //     }
    assert(ref > 0);

    for (int i = 0; i < PHYSICAL_MEM_SIZE; i++)
    {
        if (physicalMemory[i].isExist && physicalMemory[i].ref_count == ref)
        {
            /*
            W 권한인 경우에 주인 process의 page table 하나만 update
            R 권한인 경우에  공유중인 모든 process의 page table update
            1. process_running이 부모인 경우
            2. process_running이 자식인 경우
                case1. W 권한인 경우
                    isEvicted를 이용해서 하나만 update
                case2. R 권한인 경우
                    자신과 부모 + 다른 자식들도
            // TODO: memory_read에서 R 권한인 page를 physical memory에 가져올 때도 공유중인 모든 process의 page table update
            Update pageTable 후보
            1. running process
            2. readyQ
            3. waiting
            */
            // 권한 찾기
            bool isFound = false;
            Protection protectionEvicted = WRITE;
            for (const auto entry : process_running->pageTable)
                if (entry.is_valid && entry.frameID == i)
                {
                    protectionEvicted = entry.protection;
                    isFound = true;
                    break;
                }

            if (!isFound && process_ready.size() != 0)
                for (auto &process : process_ready)
                    if (process->pid == process_running->ppid)
                    {
                        for (auto &parent_entry : process->pageTable)
                        {
                            if (parent_entry.is_valid && parent_entry.frameID == i)
                            {
                                protectionEvicted = parent_entry.protection;
                                isFound = true;
                                break;
                            }
                        }
                        break;
                    }

            if (!isFound && process_waiting.size() != 0)
                for (auto &process : process_waiting)
                    if (process->pid == process_running->ppid)
                    {
                        for (auto &parent_entry : process->pageTable)
                        {
                            if (parent_entry.is_valid && parent_entry.frameID == i)
                            {
                                protectionEvicted = parent_entry.protection;
                                isFound = true;
                                break;
                            }
                        }
                        break;
                    }

            // R인 경우
            // process_running이 자식인 경우 부모와 모든 자식의 page table update
            // process_running이 부모인 경우 부모와 모든 자식의 page table update
            if (protectionEvicted == READ)
            {
                // process_running이 부모인 경우
                // 자기 자신과 자식들, 자식들은 무조건 readyQ에 존재
                if (process_running->pid == 1)
                {
                    for (auto &entry : process_running->pageTable)
                    {
                        if (entry.is_valid && entry.frameID == i)
                        {
                            // virtual memory에 page는 할당되어 있지만 physical memory에 frame은 할당되어 있지 않을 수도 있다
                            // entry.is_valid == false && frame.isExist == false && page.isExist == true
                            entry.is_valid = false;
                            entry.frameID = -1;
                            break;
                        }
                    }
                    if (process_ready.size() != 0)
                    {
                        for (auto &process : process_ready)
                        {
                            if (process->ppid == process_running->pid)
                            {
                                for (auto &parent_entry : process->pageTable)
                                {
                                    if (parent_entry.is_valid && parent_entry.frameID == i)
                                    {
                                        parent_entry.is_valid = false;
                                        parent_entry.frameID = -1;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
                // process_running이 자식인 경우
                // 부모와 다른 자식들
                else
                {
                    // 자기 자신에 대한 update
                    for (auto &entry : process_running->pageTable)
                    {
                        if (entry.is_valid && entry.frameID == i)
                        {
                            // virtual memory에 page는 할당되어 있지만 physical memory에 frame은 할당되어 있지 않을 수도 있다
                            // entry.is_valid == false && frame.isExist == false && page.isExist == true
                            entry.is_valid = false;
                            entry.frameID = -1;
                            break;
                        }
                    }
                    // readyQ에 있는 다른 자식들과 부모에 대한 update
                    if (process_ready.size() != 0)
                    {
                        for (auto &process : process_ready)
                        {
                            // 다른 자식들 혹은 부모인 경우
                            if (process->ppid == 1 || process->pid == 1)
                            {
                                for (auto &parent_entry : process->pageTable)
                                {
                                    if (parent_entry.is_valid && parent_entry.frameID == i)
                                    {
                                        parent_entry.is_valid = false;
                                        parent_entry.frameID = -1;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                    // waitingQ에 있는 부모에 대한 update
                    if (process_waiting.size() != 0)
                    {
                        for (auto &process : process_waiting)
                        {
                            // 다른 자식들 혹은 부모인 경우
                            if (process->pid == 1)
                            {
                                for (auto &parent_entry : process->pageTable)
                                {
                                    if (parent_entry.is_valid && parent_entry.frameID == i)
                                    {
                                        parent_entry.is_valid = false;
                                        parent_entry.frameID = -1;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
            // W인 경우
            else
            {
                bool isEvicted = false;
                for (auto &entry : process_running->pageTable)
                {
                    if (entry.is_valid && entry.frameID == i)
                    {
                        // virtual memory에 page는 할당되어 있지만 physical memory에 frame은 할당되어 있지 않을 수도 있다
                        // entry.is_valid == false && frame.isExist == false && page.isExist == true
                        entry.is_valid = false;
                        entry.frameID = -1;
                        isEvicted = true;
                        break;
                    }
                }
                if (!isEvicted && process_ready.size() != 0)
                {
                    for (auto &process : process_ready)
                    {
                        if (process->pid == process_running->ppid)
                        {
                            for (auto &parent_entry : process->pageTable)
                            {
                                if (parent_entry.is_valid && parent_entry.frameID == i)
                                {
                                    parent_entry.is_valid = false;
                                    parent_entry.frameID = -1;
                                    isEvicted = true;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
                if (!isEvicted && process_waiting.size() != 0)
                {
                    for (auto &process : process_waiting)
                    {
                        if (process->pid == process_running->ppid)
                        {
                            for (auto &parent_entry : process->pageTable)
                            {
                                if (parent_entry.is_valid && parent_entry.frameID == i)
                                {
                                    parent_entry.is_valid = false;
                                    parent_entry.frameID = -1;
                                    isEvicted = true;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }

            // Update physical memory
            physicalMemory[i].isExist = false;
            physicalMemory[i].pageID = -1;
            physicalMemory[i].pid = -1;
            physicalMemory[i].ref_count = -1;
            break;
        }
    }
}

/*
확보된 빈 frame에 page mapping
@pageID: mapping 하고자 하는 page의 pageID
*/
Protection mapping(Process *&process_running, array<FrameInfo, 16> &physicalMemory, int pageID)
{
    // if (frame.isExist == true && frame.pageID == pageID && frameMapped.pid == process_running->pid)
    bool working = false;
    Protection protection = WRITE;
    for (int i = 0; i < PHYSICAL_MEM_SIZE; i++)
        if (physicalMemory[i].isExist == false)
        {
            working = true;
            // Update pageTable
            for (auto &entry : process_running->pageTable)
                if (entry.pageID == pageID)
                {
                    entry.frameID = i;
                    entry.is_valid = true;
                    protection = entry.protection;
                    break;
                }
            // Update physical memory
            physicalMemory[i].isExist = true;
            physicalMemory[i].pageID = pageID;
            // entry의 권한에 따라 frame의 pid 결정
            physicalMemory[i].pid = protection == WRITE ? process_running->pid : process_running->ppid;
            physicalMemory[i].ref_count = 0;
            break;
        }
    // TODO: DEBUGGING
    assert(working == true);
    return protection;
}
/*
페이지 교체 알고리즘의 ‘참조’ 시점은 memory_allocate, memory_read, 그리고
memory_write 명령어의 호출한 cycle이 종료되는 시점이다. 즉, 해당 명령어 처리가 끝난 다음
cycle부터 reference count가 1 증가한다. read 또는 write 명령어로 인해 폴트가 발생해서
새롭게 할당되는 경우에는, 폴트 처리 cycle 처리 과정 중에 ‘참조’된 것이고, 해당 cycle이 끝나면
reference count가 1 증가한다. 페이지 교체 알고리즘 등으로 인해 해당 페이지가 물리
메모리에서 해제되면 reference count가 초기화된다.
명령어 처리가 끝난 다음 cycle부터 reference count가 1 증가
pageID 해당하는 frame 1개만 update
어떤 경우에 R을 업데이트 해야 하고 어떤 경우에 W를 업데이트 해야 하지?


R인 경우
frame.pid == process_running->ppid
W인 경우
frame.pid == process_running->pid
*/
void updateCounter(const int cycle, Process *&process_running, array<FrameInfo, 16> &physicalMemory, int pageID, Replacement replacement_algo, Protection prot)
{
    if (replacement_algo == LFU || replacement_algo == MFU)
        for (auto &frame : physicalMemory)
        {
            // TODO: frame.pid가 ppid인 경우 고려! 즉, R 권한인 경우도 고려
            if (frame.isExist && frame.pageID == pageID && frame.pid == (prot == WRITE ? process_running->pid : process_running->ppid))
            {
                frame.ref_count += 1;
                break;
            }
        }
    else if (replacement_algo == LRU)
        for (auto &frame : physicalMemory)
        {
            // TODO: frame.pid가 ppid인 경우 고려! 즉, R 권한인 경우도 고려
            if (frame.isExist && frame.pageID == pageID && frame.pid == (prot == WRITE ? process_running->pid : process_running->ppid))
            {
                frame.ref_count = cycle;
                break;
            }
        }
    else if (replacement_algo == FIFO)
        for (auto &frame : physicalMemory)
        {
            // TODO: frame.pid가 ppid인 경우 고려! 즉, R 권한인 경우도 고려
            if (frame.isExist && frame.pageID == pageID && frame.pid == (prot == WRITE ? process_running->pid : process_running->ppid) && frame.ref_count <= 0)
            {
                frame.ref_count = cycle;
                break;
            }
        }
}

int checkFreeFrames(const array<FrameInfo, 16> physicalMemory)
{
    int numFreed = 0;
    for (const auto frame : physicalMemory)
    {
        if (frame.isExist == false)
            numFreed++;
    }
    return numFreed;
}