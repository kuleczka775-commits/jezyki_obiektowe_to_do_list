#pragma once
#include "Task.h"

using namespace std;

class HabitTask : public Task
{
private:
    int intervalDays;
    int currentStreak;
    int priority;

public:
    HabitTask(int id, string title, string tag, int intervalDays);

    int getPriority() const override;
    void setPriority(int newPriority);
    void completeHabit();
};