#pragma once
#include "Task.h"

using namespace std;

class HabitTask : public Task
{
private:
    int intervalDays;
    int currentStreak;

public:
    HabitTask(int id, string title, string tag, int intervalDays);

    void completeHabit();
};