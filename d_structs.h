#ifndef VARIABLES
#define VARIABLES
#include <vector>
#include <deque>
#include <array>
#include <string>
using namespace std;

const int VIRTUAL_MEM_SIZE = 32;
const int PHYSICAL_MEM_SIZE = 16;

enum Replacement
{
    LRU,
    FIFO,
    LFU,
    MFU
};

enum FaultType
{
    PAGE_FAULT,
    PROTECTION_FAULT
};

enum Protection
{
    READ,
    WRITE
};

enum State
{
    RUNNING,
    READY,
    WAITING,
    NEW,
    TERMINATED,
};

struct FrameInfo
{
    int pid;
    int pageID;
    int ref_count;
    // 현재 frame이 page에 할당되어 있는지
    bool isExist;
};

struct PageInfo
{
    int pageID;
    int allocationID;
    // virtual memory의 현 주소가 page로 쓰이고 있는지
    bool isExist;
};

struct PageTableEntry
{
    int pageID;
    int frameID;
    // 현 page가 frame과 mapping 되어 있는지
    bool is_valid;
    Protection protection;
};

// PCB
struct Process
{
    string name;       // 프로세스 이름
    int pid;           // 프로세스 ID
    int ppid;          // 부모 프로세스 ID
    char waiting_type; // waiting일 때 S, W중 하나
    int remaining_cycle;
    deque<string> code;
    State state;

    // For memory
    int currentPageID;
    int currentAllocationID;
    // TODO: pageID가 31을 넘기는 경우는? 0부터 다시 할당 => 잊지말고 구현해
    array<PageInfo, 32> virtualMemory;
    array<PageTableEntry, 32> pageTable;
};

struct Status
{
    int cycle;
    string mode;
    string command;
    Process *process_running;
    deque<Process *> process_ready;
    vector<Process *> process_waiting;
    Process *process_new;
    Process *process_terminated;
    array<FrameInfo, 16> physicalMemory;
};

#endif