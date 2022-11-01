#include "UserInterface.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#define SCR_DIRTY_ARG(uih) uih->screenDirty = 1
#define SCR_DIRTY uih->screenDirty = 1
#define SCR_DIRTY_CLR uih->screenDirty = 0

extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;

const char* g_menuActionIcons = "%#!$";

void UserInterface_Init(UiHandle* uih)
{
	ssd1306_Init(&hi2c1);
	uih->currentPage = NULL;
	uih->currentMenu = NULL;
	uih->screenDirty = 0;
	uih->screenStatus = 1;
}

void UserInterface_ChangePage(UiHandle* uih, UiPagePtr page)
{	
	if (uih->currentPage != NULL && uih->currentPage->onLeave)
		uih->currentPage->onLeave();
	
	uih->currentPage = page;
	
	if (uih->currentPage->onInit)
		uih->currentPage->onInit();
	
	if (uih->currentPage->text && uih->currentPage->menu)
	{
		ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(uih->currentPage->text, Font_11x18, White);

		ssd1306_UpdateScreen(&hi2c1);
		osDelay(1000);
	}
	
	if (!uih->currentPage->menu)
	{
		uih->currentMenu = NULL;
	}
	
	ssd1306_Fill(Black);
	
	if (uih->currentPage->text && !uih->currentPage->menu)
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(uih->currentPage->text, Font_11x18, White);
		UserInterface_p_DrawActions(uih->currentPage->actionIcons);
	}
	
	if (uih->currentPage->menu)
	{
		uih->currentMenu = uih->currentPage->menu;
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(uih->currentMenu->caption, Font_11x18, White);
		UserInterface_p_DrawActions(g_menuActionIcons);
	}
	
	SCR_DIRTY;
}

void UserInterface_HandleInput(UiHandle* uih, uint16_t input)
{
	if (!uih->currentPage)
		return;

	if (uih->currentPage->menu == NULL)
	{
		if (input == Key1_Pin) {
			uih->currentPage->onHandleInput(uih, Key1);
		}
		if (input == Key2_Pin) {
			uih->currentPage->onHandleInput(uih, Key2);
		}
		if (input == Key3_Pin) {
			uih->currentPage->onHandleInput(uih, Key3);
		}
		if (input == Key4_Pin) {
			uih->currentPage->onHandleInput(uih, Key4);
		}
	}
	else
	{
		if (input == Key1_Pin) {
			UserInterface_p_onHandleMenuInput(uih, Key1);
		}
		if (input == Key2_Pin) {
			UserInterface_p_onHandleMenuInput(uih, Key2);
		}
		if (input == Key3_Pin) {
			UserInterface_p_onHandleMenuInput(uih, Key3);
		}
		if (input == Key4_Pin) {
			UserInterface_p_onHandleMenuInput(uih, Key4);
		}
	}
}

void UserInterface_InitMenu(UiMenuPtr menu, const char* caption, void* prev, void* next, void* parent, void* parentPage, void* children, menuCallback callback)
{
	if (!menu)
		return;
	
	(menu)->caption = caption;
	(menu)->prev = prev;
	(menu)->next = next;
	(menu)->parent = parent;
	(menu)->parentPage = parentPage;
	(menu)->children = children;
	(menu)->callback = callback;
}

uint8_t UserInterface_Update(UiHandle* uih, uint32_t since)
{
	if (uih->currentPage && uih->currentPage->onUpdate && uih->screenStatus)
	{
		return uih->currentPage->onUpdate(uih, since);
	}
	return 0;
	// SCR_DIRTY;
}

void UserInterface_Flush(UiHandle* uih)
{
	if (!uih->screenDirty || !uih->screenStatus)
		return;
	
	ssd1306_UpdateScreen(&hi2c1);
	SCR_DIRTY_CLR;
}

void UserInterface_TurnOnScreen(UiHandle* uih)
{
	uih->screenStatus = 1;
}

void UserInterface_TurnOffScreen(UiHandle* uih)
{
	uih->screenStatus = 0;
	
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen(&hi2c1);
	SCR_DIRTY_CLR;
}

uint8_t UserInterface_ScreenIsOn(UiHandle* uih)
{
	return uih->screenStatus;
}

void UserInterface_p_DrawMenu(UiHandle* uih)
{
	if (!uih->currentMenu)
		return;
  ssd1306_Fill(Black);
	ssd1306_SetCursor(5, 5);
	ssd1306_WriteString(uih->currentMenu->caption, Font_11x18, White);
	UserInterface_p_DrawActions(g_menuActionIcons);
	SCR_DIRTY;
}

void UserInterface_p_onHandleMenuInput(UiHandle* uih, enum ActionType action)
{
	switch(action)
	{
		case Key1:
			if (uih->currentMenu->parent != NULL)
				uih->currentMenu = uih->currentMenu->parent;
			else if (uih->currentMenu->parentPage)
				((UiPagePtr)uih->currentMenu->parentPage)->onHandleInput(uih, Key1);
			break;
		case Key2:
			if (uih->currentMenu->prev != NULL)
				uih->currentMenu = uih->currentMenu->prev;
			break;
		case Key3:
		{
			if (uih->currentMenu->callback != NULL)
			{
				ssd1306_Fill(Black);
				ssd1306_SetCursor(5, 5);
				ssd1306_WriteString("Executing...", Font_11x18, White);
				ssd1306_UpdateScreen(&hi2c1);
				osDelay(500);

				uih->currentMenu->callback();
				
				ssd1306_Fill(Black);
				ssd1306_SetCursor(5, 5);
				ssd1306_WriteString("Done", Font_11x18, White);
				ssd1306_UpdateScreen(&hi2c1);
				osDelay(1000);
			}
			else if (uih->currentMenu->children != NULL)
			{
				uih->currentMenu = uih->currentMenu->children;
			}
			break;
		}
		case Key4:
			if (uih->currentMenu->next != NULL)
				uih->currentMenu = uih->currentMenu->next;
			break;
	}
	
	UserInterface_p_DrawMenu(uih);
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
	
	// Create Main Page
	uih->pages[MainPageIdx].text = "Main Page";
	uih->pages[MainPageIdx].actionIcons = NULL;
	uih->pages[MainPageIdx].onInit = NULL;
	uih->pages[MainPageIdx].onUpdate = mainPageUpdateCallback;
	uih->pages[MainPageIdx].onLeave = NULL;
	uih->pages[MainPageIdx].onHandleInput = mainPageInputCallback;

	// Create Settings Page And Menus
	
	UiMenuPtr settingMenus = (UiMenuPtr)malloc(sizeof(UiMenu) * 2);
	if (!settingMenus)
		return;
	UserInterface_InitMenu(&settingMenus[0], "Time", 					NULL, &settingMenus[1], NULL, &uih->pages[SettingPageIdx], NULL, setTimeMenuCallback);
	UserInterface_InitMenu(&settingMenus[1], "Sleep Time", 		&settingMenus[0], NULL, NULL, &uih->pages[SettingPageIdx], NULL, setSleepTimeMenuCallback);
	
	uih->pages[SettingPageIdx].text = "Settings";
	uih->pages[SettingPageIdx].menu = settingMenus;
	uih->pages[SettingPageIdx].actionIcons = NULL;
	uih->pages[SettingPageIdx].onInit = NULL;
	uih->pages[SettingPageIdx].onUpdate = NULL;
	uih->pages[SettingPageIdx].onLeave = NULL;
	uih->pages[SettingPageIdx].onHandleInput = settingsPageInputCallback;
	
	UserInterface_ChangePage(uih, &uih->pages[MainPageIdx]);
}

// TODO change the location of these two methods
uint8_t DecodeBCD(uint8_t bin) {
	return (((bin & 0xf0) >> 4) * 10) + (bin & 0x0f);
}

uint8_t EncodeBCD(uint8_t dec) {
	return (dec % 10 + ((dec / 10) << 4));
}

uint8_t mainPageUpdateCallback(void* uih, uint32_t since)
{
	if (since < 400)
		return 0;
	
	UiHandle* hnd = uih;
	RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
	
	if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
	
	static int counter;
	counter++;
	char timeBuff[8] = {0};
	sprintf(timeBuff, "%02d:%02d", DecodeBCD(sTime.Hours), DecodeBCD(sTime.Minutes));
	
	char secBuff[8] = {0};
	sprintf(secBuff, "%02d", DecodeBCD(sTime.Seconds));

	char dateBuff[8] = {0};
	sprintf(dateBuff, "%02d/%02d", DecodeBCD(sDate.Month), DecodeBCD(sDate.Date));

	ssd1306_Fill(Black);
	ssd1306_SetCursor(16, 5);
	ssd1306_WriteString(timeBuff, Font_16x26, White);
	
	ssd1306_SetCursor(101, 13);
	ssd1306_WriteString(secBuff, Font_11x18, White);

	ssd1306_SetCursor(36, 35);
	ssd1306_WriteString(dateBuff, Font_11x18, White);

	ssd1306_SetCursor(5, 40);
	ssd1306_WriteChar('&', Icon_11x18, White);
	
	SCR_DIRTY_ARG(hnd);
	return 1;
}

void mainPageInputCallback(void* uih, enum ActionType action)
{
	switch(action)
	{
		case Key1:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[SettingPageIdx]);
			break;
		default:
			break;
	}
}

void settingsPageInputCallback(void* uih, enum ActionType action)
{
	switch(action)
	{
		case Key1:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[MainPageIdx]);
			break;
		default:
			break;
	}
}

void setTimeMenuCallback()
{
	/*while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_SET);

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
*/
	  ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("0", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);
		osDelay(1000);
}

void setSleepTimeMenuCallback()
{
	  ssd1306_Fill(Black);
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("00", Font_11x18, White);
		ssd1306_UpdateScreen(&hi2c1);
		osDelay(1000);
}

