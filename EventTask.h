#pragma once
#include "Task.h"

using namespace std;

class EventTask : public Task
{
private:
    string date;
    string startTime;
    string endTime;
    string location;

public:
    EventTask(int id, string title, string tag, string date, string startTime, string endTime, string location);

    string getDate() const;
    string getStartTime() const;
    string getEndTime() const;
    string getLocation() const;

    void setDate(string newDate);
    void setStartTime(string newStartTime);
    void setEndTime(string newEndTime);
    void setLocation(string newLocation);
};