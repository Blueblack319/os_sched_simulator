#ifndef PROCESS_H
#define PROCESS_H
#include <vector>
#include <deque>
#include <array>
#include <string>
#include <iostream>
#include <fstream>
#include "d_structs.h"

using namespace std;

void print_status(Status status);

void preprocessing(deque<Process *> &process_ready, vector<Process *> &process_waiting);

#endif