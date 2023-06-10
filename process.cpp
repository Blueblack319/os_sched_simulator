#include "process.h"

// github에 올라온 커널 시뮬레이터 결과 출력 포맷을 그대로 이용함
// ofstream class를 이용하여 result file에 write
void print_status(Status status)
{
    // TODO: 반드시 TXT 빼기!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ofstream resultFile("result.txt", ios::app);

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
                if (status.process_running->pageTable[i].is_valid)
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

        // // 4. 현재 ready 상태인 프로세스의 정보. 왼쪽에 있을 수록 먼저 queue에 들어온 프로세스이다.
        // if (status.process_ready.size() == 0)
        // {
        //     // printf("### ready: none\n");
        //     resultFile << "### ready: none\n";
        // }
        // else
        // {
        //     resultFile << "### ready:";
        //     for (int i = 0; i < status.process_ready.size(); ++i)
        //     { // 공백 한칸으로 구분
        //         resultFile << " " << status.process_ready[i]->pid;
        //     }
        //     resultFile << "\n";
        // }

        // // 5. 현재 waiting 상태인 프로세스의 정보. 왼쪽에 있을 수록 먼저 waiting이 된 프로세스이다.
        // if (status.process_waiting.size() == 0)
        // {
        //     resultFile << "### waiting: none\n";
        // }
        // else
        // {
        //     resultFile << "### waiting:";
        //     for (int i = 0; i < status.process_waiting.size(); ++i)
        //     { // 공백 한칸으로 구분
        //         resultFile << " " << status.process_waiting[i]->pid << "(" << status.process_waiting[i]->waiting_type << ")";
        //     }
        //     resultFile << "\n";
        // }

        // // 6. New 상태의 프로세스
        // if (status.process_new == NULL)
        // {
        //     resultFile << "### new: none\n";
        // }
        // else
        // {
        //     Process *i = status.process_new;
        //     resultFile << "### new: " << i->pid << "(" << i->name.c_str() << ", " << i->ppid << ")\n";
        // }

        // // 7. Terminated 상태의 프로세스
        // if (status.process_terminated == NULL)
        // {
        //     resultFile << "### terminated: none\n";
        // }
        // else
        // {
        //     Process *i = status.process_terminated;
        //     resultFile << "### terminated: " << i->pid << "(" << i->name.c_str() << ", " << i->ppid << ")\n";
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
