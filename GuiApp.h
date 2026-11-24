#ifndef GUIAPP_H
#define GUIAPP_H

#include "TaskManager.h"
#include "FileManager.h"
#include <string>

enum class AppView
{
    LIST,
    BOARD,
    STATS
};

class GuiApp
{
private:
    TaskManager &manager;
    FileManager &fileManager;

    int screenWidth;
    int screenHeight;
    AppView currentView;

    std::string statusMessage;
    float statusTimer;

    void drawSidebar();
    void drawMainContent();
    void showStatus(const std::string &msg);

public:
    GuiApp(TaskManager &manager, FileManager &fileManager, int width = 1200, int height = 750);
    void run();
};

#endif