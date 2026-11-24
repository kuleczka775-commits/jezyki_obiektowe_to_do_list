#ifndef GUIAPP_H
#define GUIAPP_H

#include "TaskManager.h"
#include "FileManager.h"
#include <string>

enum class AppView
{
    LIST,
    BOARD,
    STATS,
    ADD
};

class GuiApp
{
private:
    TaskManager &manager;
    FileManager &fileManager;

    int screenWidth;
    int screenHeight;
    AppView currentView;

    // Przewijanie
    float listScrollY;
    float boardScrollY[3];

    // Wyszukiwanie i filtry na liscie
    char searchBuf[128];
    int filterType; // 0=Wszystkie, 1=Termin, 2=Nawyk, 3=Wydarzenie, 4=Checklista
    int activeInput;

    // Dodawanie podzadan do istniejacej checklisty
    int addingSubtaskTaskId;
    char newSubtaskBuf[128];

    // Stan formularza dodawania zadania
    int formType;     // 0=Termin, 1=Nawyk, 2=Wydarzenie, 3=Checklista
    int formPriority; // 1, 2, 3
    char formTitle[128];
    char formTag[64];
    char formDate[32];
    char formInterval[16];
    char formStartTime[16];
    char formEndTime[16];
    char formLocation[128];
    char formSubtasks[256];

    // Powiadomienia (toast)
    std::string statusMessage;
    float statusTimer;
    bool statusIsError;

    void drawSidebar();
    void drawMainContent();
    void drawListView(float startX, float startY, float width, float height);
    void drawBoardView(float startX, float startY, float width, float height);
    void drawStatsView(float startX, float startY, float width, float height);
    void drawAddTaskView(float startX, float startY, float width, float height);

    void resetAddForm();
    int getNextTaskId() const;
    void showStatus(const std::string &msg, bool isError = false);

public:
    GuiApp(TaskManager &manager, FileManager &fileManager, int width = 1280, int height = 800);
    void run();
};

#endif