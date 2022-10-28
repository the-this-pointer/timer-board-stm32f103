#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "ssd1306.h"
#include "fonts.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>

enum ActionType {Key1, Key2, Key3, Key4};

typedef void(*pageLifeCycleCallback)();
typedef void(*pageInputCallback)(void* uih, enum ActionType action);
typedef void(*menuCallback)();

typedef struct {
	const char* caption;
	void* prev;
	void* next;
	void* parent;
	void* children;
	
	menuCallback callback;
} UiMenu, *UiMenuPtr;

typedef struct {
	const char* text;
	UiMenuPtr	menu;
	const char* actionIcons;
	
	pageLifeCycleCallback onInit;
	pageLifeCycleCallback onLeave;
	pageInputCallback onHandleInput;
} UiPage, *UiPagePtr;

typedef struct {
	UiPagePtr pages;
	UiPagePtr currentPage;
} UiHandle;

void UserInterface_Init();
void UserInterface_ChangePage(UiHandle* uih, UiPagePtr page);
void UserInterface_HandleInput(UiHandle* uih);

void UserInterface_p_onHandleMenuInput(UiHandle* uih, enum ActionType action);
void UserInterface_p_DrawActions(const char* actions);


// Ui Callbacks
enum Pages {MainPageIdx, Page1Idx, Page2Idx, MaxPages};

void UserInterface_InitPages(UiHandle* uih);
void mainPageInputCallback(void* uih, enum ActionType action);



void UIMenus_Init(UiMenuPtr* menus);
void UIMenu_Init(UiMenuPtr menu, const char* caption, void* prev, void* next, void* parent, void* children, menuCallback callback);
void UIMenu_HandleInput(UiMenuPtr* menus);
void UIMenu_Draw();

// Callback functions

void setTime();
void setSleepTime();
void setOutput();
void exitFromMenu();


#endif	// USERINTERFACE_H
