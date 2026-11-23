#include "EventTask.h"
#include "Exceptions.h"
#include <iostream>

using namespace std;

EventTask::EventTask(int id, string title, string tag, string date, string startTime, string endTime, string location) : Task(id, title, tag), date(date), startTime(startTime), endTime(endTime), location(location)
{
    validateDate(date);
    cout << "Utworzono nowe wydarzenie: " << title << " (" << date << ", " << startTime << " - " << endTime << ", miejsce: " << location << ")" << endl;
}

string EventTask::getDate() const
{
    return date;
}

string EventTask::getStartTime() const
{
    return startTime;
}

string EventTask::getEndTime() const
{
    return endTime;
}

string EventTask::getLocation() const
{
    return location;
}

void EventTask::setDate(string newDate)
{
    validateDate(newDate);
    date = newDate;
}

void EventTask::setStartTime(string newStartTime)
{
    startTime = newStartTime;
}

void EventTask::setEndTime(string newEndTime)
{
    endTime = newEndTime;
}

void EventTask::setLocation(string newLocation)
{
    location = newLocation;
}

string EventTask::getType() const
{
    return "Wydarzenie";
}

string EventTask::getSummary() const
{
    return date + " (" + startTime + " - " + endTime + ") | Miejsce: " + location;
}

string EventTask::serialize() const
{
    return "Event|" + to_string(id) + "|" + title + "|" + tag + "|" + status + "|" + date + "|" + startTime + "|" + endTime + "|" + location;
}