#include "process.h"

// github에 올라온 커널 시뮬레이터 결과 출력 포맷을 그대로 이용함
// ofstream class를 이용하여 result file에 write
void print_status(Status status)
{
    ofstream resultFile("result", ios::app);

    if (resultFile.is_open())
    {
        // 0. 몇번째 cycle인지
        // printf("[cycle #%d]\n", status.cycle);
        resultFile << "[cycle #" << status.cycle << "]\n";

        // 1. 현재 실행 모드 (user or kernel)
        // printf("1. mode: %s\n", status.mode.c_str());
        resultFile << "1. mode: " << status.mode.c_str() << "\n";

        // 2. 현재 실행 명령어
        // printf("2. command: %s\n", status.command.c_str());
        resultFile << "2. command: " << status.command.c_str() << "\n";

        // 3. 현재 실행중인 프로세스의 정보. 없을 시 none 출력
        if (status.process_running == NULL)
        {
            // printf("3. running: none\n");
            resultFile << "3. running: none\n";
        }
        else
        {
            Process *i = status.process_running;
            // printf("3. running: %d(%s, %d)\n", i->pid, i->name.c_str(), i->ppid);
            resultFile << "3. running: " << i->pid << "(" << i->name.c_str() << ", " << i->ppid << ")\n";
        }

        // 4. Physical memory of the whole system
        resultFile << "4. physical memory:\n";
        resultFile << "|";
        for (int i = 0; i < PHYSICAL_MEM_SIZE; i++)
        {
            if (status.physicalMemory[i].isExist)
            {
                resultFile << to_string(status.physicalMemory[i].pid) + "(" + to_string(status.physicalMemory[i].pageID) + ")";
            }
            else
            {
                resultFile << "-";
            }
            if (i % 4 == 3)
            {
                resultFile << "|";
            }
            else
            {
                resultFile << " ";
            }
        }
        resultFile << "\n";
        // TODO: DEBUGGING
        // Reference count
        // resultFile << "4-1. pid(ref_count):\n";
        // resultFile << "|";
        // for (int i = 0; i < PHYSICAL_MEM_SIZE; i++)
        // {
        //     if (status.physicalMemory[i].isExist)
        //     {
        //         resultFile << to_string(status.physicalMemory[i].pid) + "(" + to_string(status.physicalMemory[i].ref_count) + ")";
        //     }
        //     else
        //     {
        //         resultFile << "-";
        //     }
        //     if (i % 4 == 3)
        //     {
        //         resultFile << "|";
        //     }
        //     else
        //     {
        //         resultFile << " ";
        //     }
        // }
        // resultFile << "\n";

        // 5. Virtual memory info of a process
        if (status.process_running != NULL)
        {
            resultFile << "5. virtual memory:\n";
            resultFile << "|";
            for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
            {
                if (status.process_running->virtualMemory[i].isExist)
                {
                    resultFile << to_string(status.process_running->virtualMemory[i].pageID);
                }
                else
                {
                    resultFile << "-";
                }
                if (i % 4 == 3)
                {
                    resultFile << "|";
                }
                else
                {
                    resultFile << " ";
                }
            }
            resultFile << "\n";
        }

        // 6. Page table of a process
        if (status.process_running != NULL)
        {
            resultFile << "6. page table:\n";
            resultFile << "|";
            for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
            {
                if (status.process_running->pageTable[i].is_valid)
                {
                    resultFile << status.process_running->pageTable[i].frameID;
                }
                else
                {
                    resultFile << "-";
                }
                if (i % 4 == 3)
                {
                    resultFile << "|";
                }
                else
                {
                    resultFile << " ";
                }
            }
            resultFile << "\n";
            resultFile << "|";
            for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
            {
                if (status.process_running->virtualMemory[i].isExist)
                {
                    if (status.process_running->pageTable[i].protection == READ)
                    {
                        resultFile << "R";
                    }
                    else
                    {
                        resultFile << "W";
                    }
                }
                else
                {
                    resultFile << "-";
                }
                if (i % 4 == 3)
                {
                    resultFile << "|";
                }
                else
                {
                    resultFile << " ";
                }
            }
            resultFile << "\n";
        }

        // TODO: DEBUGGING
        // 7. 부모의 page tabel
        // if (status.process_ready.size() != 0 || status.process_waiting.size() != 0)
        // {

        //     for (const auto process : status.process_ready)
        //     {
        //         resultFile << "7. page table of : " << process->pid << "\n";
        //         resultFile << "|";
        //         for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
        //         {
        //             if (process->pageTable[i].is_valid)
        //             {
        //                 resultFile << process->pageTable[i].frameID;
        //             }
        //             else
        //             {
        //                 resultFile << "-";
        //             }
        //             if (i % 4 == 3)
        //             {
        //                 resultFile << "|";
        //             }
        //             else
        //             {
        //                 resultFile << " ";
        //             }
        //         }
        //         resultFile << "\n";
        //         resultFile << "|";
        //         for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
        //         {
        //             if (process->virtualMemory[i].isExist)
        //             {
        //                 if (process->pageTable[i].protection == READ)
        //                 {
        //                     resultFile << "R";
        //                 }
        //                 else
        //                 {
        //                     resultFile << "W";
        //                 }
        //             }
        //             else
        //             {
        //                 resultFile << "-";
        //             }
        //             if (i % 4 == 3)
        //             {
        //                 resultFile << "|";
        //             }
        //             else
        //             {
        //                 resultFile << " ";
        //             }
        //         }
        //         resultFile << "\n";
        //     }
        //     for (const auto process : status.process_waiting)
        //     {
        //         resultFile << "7. page table of : " << process->pid << "\n";
        //         resultFile << "|";
        //         for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
        //         {
        //             if (process->pageTable[i].is_valid)
        //             {
        //                 resultFile << process->pageTable[i].frameID;
        //             }
        //             else
        //             {
        //                 resultFile << "-";
        //             }
        //             if (i % 4 == 3)
        //             {
        //                 resultFile << "|";
        //             }
        //             else
        //             {
        //                 resultFile << " ";
        //             }
        //         }
        //         resultFile << "\n";
        //         resultFile << "|";
        //         for (int i = 0; i < VIRTUAL_MEM_SIZE; i++)
        //         {
        //             if (process->virtualMemory[i].isExist)
        //             {
        //                 if (process->pageTable[i].protection == READ)
        //                 {
        //                     resultFile << "R";
        //                 }
        //                 else
        //                 {
        //                     resultFile << "W";
        //                 }
        //             }
        //             else
        //             {
        //                 resultFile << "-";
        //             }
        //             if (i % 4 == 3)
        //             {
        //                 resultFile << "|";
        //             }
        //             else
        //             {
        //                 resultFile << " ";
        //             }
        //         }
        //         resultFile << "\n";
        //     }
        // }

        // 매 cycle 간의 정보는 두번의 개행으로 구분
        resultFile << "\n";

        resultFile.close();
    }
}

void preprocessing(deque<Process *> &process_ready, vector<Process *> &process_waiting)
{
    // sleep 시간 갱신 & 프로세스 상태 갱신 & process_ready 갱신
    if (process_waiting.size())
    {
        for (vector<Process *>::iterator iter = process_waiting.begin(); iter != process_waiting.end(); iter++)
        {
            if ((*iter)->waiting_type == 'S')
            {
                // sleep 시간 갱신
                (*iter)->remaining_cycle--;
                if ((*iter)->remaining_cycle <= 0)
                {
                    // 프로세스 상태 갱신 waiting(S) -> ready
                    (*iter)->state = READY;
                    process_ready.push_back(*iter);
                    process_waiting.erase(iter--);
                }
            }
        }
    }
}
