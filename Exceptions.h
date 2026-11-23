#pragma once
#include <stdexcept>
#include <string>
#include <cctype>

using namespace std;

class InvalidDateException : public runtime_error
{
public:
    InvalidDateException(const string &msg) : runtime_error(msg) {}
};

class InvalidPriorityException : public runtime_error
{
public:
    InvalidPriorityException(const string &msg) : runtime_error(msg) {}
};

class TaskNotFoundException : public runtime_error
{
public:
    TaskNotFoundException(const string &msg) : runtime_error(msg) {}
};

class FileException : public runtime_error
{
public:
    FileException(const string &msg) : runtime_error(msg) {}
};

inline void validatePriority(int priority)
{
    if (priority < 1 || priority > 3)
    {
        throw InvalidPriorityException("Nieprawidowy priorytet: " + to_string(priority) + ". Dozwolony zakres to 1-3.");
    }
}

inline void validateDate(const string &date)
{
    if (date.length() != 10 || date[4] != '-' || date[7] != '-')
    {
        throw InvalidDateException("Nieprawidłowy format daty: '" + date + "'. Wymagany format: RRRR-MM-DD.");
    }

    int digitIndices[] = {0, 1, 2, 3, 5, 6, 8, 9};
    for (int i : digitIndices)
    {
        if (!isdigit(static_cast<unsigned char>(date[i])))
        {
            throw InvalidDateException("Data zawiera niedozwolone znaki: '" + date + "'. Wymagany format: RRRR-MM-DD.");
        }
    }

    int year = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    int day = stoi(date.substr(8, 2));

    if (month < 1 || month > 12)
    {
        throw InvalidDateException("Nieprawidłowy miesiąc w dacie: '" + date + "'.");
    }

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (isLeap)
    {
        daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1])
    {
        throw InvalidDateException("Nieprawidłowy dzień w dacie: '" + date + "'.");
    }
}