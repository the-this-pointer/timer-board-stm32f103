#include "UserInterface.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

UiMenuPtr g_currentItem;

void waitReleaseAllKeys()
{
	while (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET);
	while (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_SET);
	while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET);
	while (HAL_GPIO_ReadPin(Key4_GPIO_Port, Key4_Pin) == GPIO_PIN_SET);
}

void UIMenus_Init(UiMenuPtr* menuList)
{
	ssd1306_Init(&hi2c1);
	
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
	
	UIMenu_Init(&menus[0], "Settings", 		NULL, &menus[1], NULL, settingMenus, NULL);
	UIMenu_Init(&menus[1], "Set Output", 	&menus[0], &menus[2], NULL, NULL, setOutput);
	UIMenu_Init(&menus[2], "Exit", 				&menus[1], NULL, NULL, NULL, exitFromMenu);
	
	g_currentItem = &menus[0];
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
		if (g_currentItem->prev != NULL)
		{
			g_currentItem = g_currentItem->prev;
		}
	}
	if (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_SET) {
		// back
		if (g_currentItem->parent != NULL)
		{
			g_currentItem = g_currentItem->parent;
		}
	}
	if (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET) {
		// set
		if (g_currentItem->callback != NULL)
		{
			g_currentItem->callback();
			
			while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET);
			while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_RESET);
		}
		else if (g_currentItem->children != NULL)
		{
			g_currentItem = g_currentItem->children;
		}
	}
	if (HAL_GPIO_ReadPin(Key4_GPIO_Port, Key4_Pin) == GPIO_PIN_SET) {
		// next
		if (g_currentItem->next != NULL)
		{
			g_currentItem = g_currentItem->next;
		}
	}
	
	UIMenu_Draw();
	waitReleaseAllKeys();
}

void UIMenu_Draw()
{
	if (g_currentItem == NULL)
	{
		ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("<NULL>", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);

		return;
	}
    ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(g_currentItem->caption, Font_11x18, White);
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
