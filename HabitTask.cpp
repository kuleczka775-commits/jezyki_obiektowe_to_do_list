#include "HabitTask.h"
#include "Exceptions.h"
#include <iostream>

using namespace std;

HabitTask::HabitTask(int id, string title, string tag, int intervalDays) : Task(id, title, tag), intervalDays(intervalDays), currentStreak(0)
{
    validatePriority(priority);
    cout << "Utworzono nowy nawyk:" << title << "powatarzany co " << intervalDays << " dni" << endl;
}

int HabitTask::getPriority() const
{
    return priority;
}

void HabitTask::setPriority(int newPriority)
{
    validatePriority(newPriority);
    priority = newPriority;
}

void HabitTask::completeHabit()
{
    currentStreak++;
    setStatus("Wykonane");
    cout << "Twój streak dla '" << title << "' to aktualnie: " << currentStreak << " dni" << endl;
}

int HabitTask::getIntervalDays() const
{
    return intervalDays;
}

int HabitTask::getCurrentStreak() const
{
    return currentStreak;
}
void HabitTask::setCurrentStreak(int streak)
{
    currentStreak = streak;
}

string HabitTask::getType() const
{
    return "Nawyk";
}

string HabitTask::getSummary() const
{
    return "Co" + to_string(intervalDays) + " dni | Streak: " + to_string(currentStreak) + " | Priorytet: " + to_string(priority);
}

string HabitTask::serialize() const
{
    return "Habit|" + to_string(id) + "|" + title + "|" + tag + "|" + status + "|" + to_string(intervalDays) + "|" + to_string(currentStreak) + "|" + to_string(priority);
}

HabitTask &HabitTask::operator++()
{
    completeHabit();
    return *this;
}