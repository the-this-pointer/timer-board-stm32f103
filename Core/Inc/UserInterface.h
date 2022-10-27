#include "ssd1306.h"
#include "fonts.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>

typedef void(*menuCallback)();

typedef struct {
	const char* caption;
	void* prev;
	void* next;
	
	void* parent;
	void* children;
	menuCallback callback;
} UiMenu, *UiMenuPtr;

void UIMenus_Init(UiMenuPtr* menus);
void UIMenu_Init(UiMenuPtr menu, const char* caption, void* prev, void* next, void* parent, void* children, menuCallback callback);
void UIMenu_HandleInput(UiMenuPtr* menus);
void UIMenu_Draw();

// Callback functions

void setTime();
void setSleepTime();
void setOutput();
void exitFromMenu();
