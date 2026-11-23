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
    HabitTask(int id, string title, string tag, int intervalDays, int priority);

    int getPriority() const override;
    void setPriority(int newPriority);
    void completeHabit();

    int getIntervalDays() const;
    int getCurrentStreak() const;
    void setCurrentStreak(int streak);

    string getType() const override;
    string getSummary() const override;
    string serialize() const override;

    HabitTask &operator++();
};