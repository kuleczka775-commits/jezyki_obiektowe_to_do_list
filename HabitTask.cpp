#include "HabitTask.h"
#include <iostream>

using namespace std;

HabitTask::HabitTask(int id, string title, string tag, int intervalDays) : Task(id, title, tag), intervalDays(intervalDays), currentStreak(0)
{
    cout << "Utworzono nowy nawyk:" << title << "powatarzany co " << intervalDays << " dni" << endl;
}

void HabitTask::completeHabit()
{
    currentStreak++;
    setStatus("Wykonane");
    cout << "Twój streak dla '" << title << "' to aktualnie: " << currentStreak << " dni" << endl;
}