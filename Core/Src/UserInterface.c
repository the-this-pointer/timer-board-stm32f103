#include "UserInterface.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

const char* g_menuActionIcons = "! \"#";
UiMenuPtr g_currentMenu;

void waitReleaseAllKeys()
{
	while (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET);
	while (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_SET);
	while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET);
	while (HAL_GPIO_ReadPin(Key4_GPIO_Port, Key4_Pin) == GPIO_PIN_SET);
}

void UserInterface_Init()
{
	ssd1306_Init(&hi2c1);
}

void UserInterface_ChangePage(UiHandle* uih, UiPagePtr page)
{
	ssd1306_Fill(Black);
	
	if (uih->currentPage != NULL && uih->currentPage->onLeave)
		uih->currentPage->onLeave();
	
	uih->currentPage = page;
	
	if (uih->currentPage->onInit)
		uih->currentPage->onInit();
	
	if (uih->currentPage->text)
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(uih->currentPage->text, Font_11x18, White);
		UserInterface_p_DrawActions(uih->currentPage->actionIcons);
	}
	else if (uih->currentPage->menu)
	{
		g_currentMenu = uih->currentPage->menu;
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(g_currentMenu->caption, Font_11x18, White);
		UserInterface_p_DrawActions(g_menuActionIcons);
	}
	
	ssd1306_UpdateScreen(&hi2c1);
}

void UserInterface_HandleInput(UiHandle* uih)
{
	if (!uih->currentPage)
		return;

	if (uih->currentPage->menu == NULL)
	{
		if (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET) {
			uih->currentPage->onHandleInput(uih, Key1);
		}
		if (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_SET) {
			uih->currentPage->onHandleInput(uih, Key2);
		}
		if (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET) {
			uih->currentPage->onHandleInput(uih, Key3);
		}
		if (HAL_GPIO_ReadPin(Key4_GPIO_Port, Key4_Pin) == GPIO_PIN_SET) {
			uih->currentPage->onHandleInput(uih, Key4);
		}
	}
	else
	{
		if (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET) {
			UserInterface_p_onHandleMenuInput(uih, Key1);
		}
		if (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_SET) {
			UserInterface_p_onHandleMenuInput(uih, Key2);
		}
		if (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET) {
			UserInterface_p_onHandleMenuInput(uih, Key3);
		}
		if (HAL_GPIO_ReadPin(Key4_GPIO_Port, Key4_Pin) == GPIO_PIN_SET) {
			UserInterface_p_onHandleMenuInput(uih, Key4);
		}
	}

	waitReleaseAllKeys();
}

void UserInterface_p_onHandleMenuInput(UiHandle* uih, enum ActionType action)
{
	switch(action)
	{
		case Key1:
			if (g_currentMenu->prev != NULL)
				g_currentMenu = g_currentMenu->prev;
			break;
		case Key2:
			if (g_currentMenu->parent != NULL)
				g_currentMenu = g_currentMenu->parent;
			break;
		case Key3:
		{
			if (g_currentMenu->callback != NULL)
			{
				ssd1306_Fill(Black);
				ssd1306_SetCursor(5, 5);
				ssd1306_WriteString("Executing...", Icon_11x18, White);
				ssd1306_UpdateScreen(&hi2c1);
				osDelay(500);

				g_currentMenu->callback();
				
				ssd1306_Fill(Black);
				ssd1306_SetCursor(5, 5);
				ssd1306_WriteString("Done", Icon_11x18, White);
				ssd1306_UpdateScreen(&hi2c1);
				osDelay(1000);
			}
			else if (g_currentMenu->children != NULL)
			{
				g_currentMenu = g_currentMenu->children;
			}
			break;
		}
		case Key4:
			if (g_currentMenu->next != NULL)
				g_currentMenu = g_currentMenu->next;
			break;
	}
	
	ssd1306_Fill(Black);
	ssd1306_SetCursor(5, 5);
	ssd1306_WriteString(g_currentMenu->caption, Icon_11x18, White);
	ssd1306_UpdateScreen(&hi2c1);
}

void UserInterface_p_DrawActions(const char* actions)
{
	if (actions == NULL || strlen(actions) < 4)
		return;
	
	ssd1306_SetCursor(5, 40);
	ssd1306_WriteChar(actions[0], Icon_11x18, White);
	
	ssd1306_SetCursor(40, 40);
	ssd1306_WriteChar(actions[1], Icon_11x18, White);
	
	ssd1306_SetCursor(75, 40);
	ssd1306_WriteChar(actions[2], Icon_11x18, White);
	
	ssd1306_SetCursor(110, 40);
	ssd1306_WriteChar(actions[3], Icon_11x18, White);
}



// Page Callbacks

void UserInterface_InitPages(UiHandle* uih)
{
	uih->pages = malloc(sizeof(UiPage) * MaxPages);
	if (!uih->pages)
		return;
	
	uih->pages[MainPageIdx].text = "Welcome!";
	uih->pages[MainPageIdx].actionIcons = NULL;
	uih->pages[MainPageIdx].onInit = NULL;
	uih->pages[MainPageIdx].onLeave = NULL;
	uih->pages[MainPageIdx].onHandleInput = mainPageInputCallback;
		
	uih->pages[Page1Idx].text = "Page1";
	uih->pages[Page1Idx].actionIcons = "!!!!";
	uih->pages[Page1Idx].onInit = NULL;
	uih->pages[Page1Idx].onLeave = NULL;
	uih->pages[Page1Idx].onHandleInput = mainPageInputCallback;
	
	uih->pages[Page2Idx].text = "Page2";
	uih->pages[Page2Idx].actionIcons = "!!!!";
	uih->pages[Page2Idx].onInit = NULL;
	uih->pages[Page2Idx].onLeave = NULL;
	uih->pages[Page2Idx].onHandleInput = mainPageInputCallback;

	UserInterface_ChangePage(uih, &uih->pages[MainPageIdx]);
}

void mainPageInputCallback(void* uih, enum ActionType action)
{
	switch(action)
	{
		case Key1:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[Page1Idx]);
			break;
		case Key2:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[Page2Idx]);
			break;
		case Key3:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[Page1Idx]);
			break;
		case Key4:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[Page2Idx]);
			break;
	}
}











void UIMenus_Init(UiMenuPtr* menuList)
{
	
	// Main Menu
	UiMenuPtr menus = (UiMenuPtr)malloc(sizeof(UiMenu) * 3);
	if (!menus)
		return;
	menuList = &menus;
	
	// Settings Menu
	UiMenuPtr settingMenus = (UiMenuPtr)malloc(sizeof(UiMenu) * 2);
	if (!settingMenus)
		return;
	UIMenu_Init(&settingMenus[0], "Time", 					NULL, &settingMenus[1], &menus[0], NULL, setTime);
	UIMenu_Init(&settingMenus[1], "Sleep Time", 		&settingMenus[0], NULL, &menus[0], NULL, setSleepTime);
	// Settings Menu End
	
	UIMenu_Init(&menus[0], "!\"#", 		NULL, &menus[1], NULL, NULL, NULL); // !\"#$%&
	UIMenu_Init(&menus[1], "Settings", 		&menus[0], &menus[2], NULL, settingMenus, NULL);
	UIMenu_Init(&menus[2], "Set Output", 	&menus[1], &menus[3], NULL, NULL, setOutput);
	UIMenu_Init(&menus[3], "Exit", 				&menus[2], NULL, NULL, NULL, exitFromMenu);
	
	g_currentMenu = &menus[0];
	UIMenu_Draw();
}

void UIMenu_Init(UiMenuPtr menu, const char* caption, void* prev, void* next, void* parent, void* children, menuCallback callback)
{
	if (!menu)
		return;
	
	(menu)->caption = caption;
	(menu)->prev = prev;
	(menu)->next = next;
	(menu)->parent = parent;
	(menu)->children = children;
	(menu)->callback = callback;
}

void UIMenu_HandleInput(UiMenuPtr* menus)
{
	if (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET) {
		// prev
		if (g_currentMenu->prev != NULL)
		{
			g_currentMenu = g_currentMenu->prev;
		}
	}
	if (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_SET) {
		// back
		if (g_currentMenu->parent != NULL)
		{
			g_currentMenu = g_currentMenu->parent;
		}
	}
	if (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET) {
		// set
		if (g_currentMenu->callback != NULL)
		{
			g_currentMenu->callback();
			
			while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET);
			while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_RESET);
		}
		else if (g_currentMenu->children != NULL)
		{
			g_currentMenu = g_currentMenu->children;
		}
	}
	if (HAL_GPIO_ReadPin(Key4_GPIO_Port, Key4_Pin) == GPIO_PIN_SET) {
		// next
		if (g_currentMenu->next != NULL)
		{
			g_currentMenu = g_currentMenu->next;
		}
	}
	
	UIMenu_Draw();
	waitReleaseAllKeys();
}

void UIMenu_Draw()
{
	if (g_currentMenu == NULL)
	{
		ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("<NULL>", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);

		return;
	}
    ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(g_currentMenu->caption, Icon_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);
}

// Callback functions

void setTime()
{
	while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET);

	char time[6] = "00:00";
	char charPtr = 0;
	char showChar = 1;
	char confirmed = 0;
	
	while (confirmed == 0)
	{
		if (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET) {
			// prev
			if (charPtr > 0)
			{
				charPtr--;
				if (charPtr == 2)
					charPtr--;
			}
		}
		if (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_SET) {
			// back
			confirmed = 1;
		}
		if (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET) {
			// set
			char* s = (time+charPtr);
			if (*s < 0x39)
				*(time+charPtr) = *s + 1;
			else
				*s = 0x30;
		}
		if (HAL_GPIO_ReadPin(Key4_GPIO_Port, Key4_Pin) == GPIO_PIN_SET) {
			// next
			if (charPtr < 4)
			{
				charPtr++;
				if (charPtr == 2)
					charPtr++;
			}
		}
		
		ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(time, Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);
		
		waitReleaseAllKeys();
		osDelay(500);
	}
	
		ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("Confirmed", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);

}

void setSleepTime()
{
	  ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("00", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);
}

void setOutput()
{
	  ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("...", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);
}

void exitFromMenu()
{
	  ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(".", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);
}
