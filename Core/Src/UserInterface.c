#include "UserInterface.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include <string.h>

#define SCR_DIRTY_ARG(uih) uih->screenDirty = 1
#define SCR_DIRTY uih->screenDirty = 1
#define SCR_DIRTY_CLR uih->screenDirty = 0

extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;
extern osMutexId lcdMutexHandle;
extern osTimerId sleepTimerHandle;
extern TimeList* timeList;

uint8_t	g_menuActionsOffset = 0;
const char* g_menuActionIcons = "%#$!";

#define ACT_OFST(a, b) \
	if (g_menuActionsOffset == 0) \
	{ \
		a; \
	} else { \
		b; \
	}


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
		uih->currentPage->onLeave(uih);
	
	uih->currentPage = page;
	g_menuActionsOffset = 0;
	
	if (uih->currentPage->onInit)
		uih->currentPage->onInit(uih);
	
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

void UserInterface_InitMenu(UiMenuPtr menu, const char* caption, void* prev, void* next, void* parent, void* parentPage, void* page, void* children, menuCallback callback)
{
	if (!menu)
		return;
	
	(menu)->caption = caption;
	(menu)->prev = prev;
	(menu)->next = next;
	(menu)->parent = parent;
	(menu)->parentPage = parentPage;
	(menu)->page = page;
	(menu)->children = children;
	(menu)->callback = callback;
}

uint8_t UserInterface_Update(UiHandle* uih, uint32_t since)
{
	xSemaphoreTake(lcdMutexHandle, portMAX_DELAY);
	if (uih->currentPage && uih->currentPage->onUpdate && uih->screenStatus)
	{
		xSemaphoreGive(lcdMutexHandle);
		return uih->currentPage->onUpdate(uih, since);
	}
	xSemaphoreGive(lcdMutexHandle);
	return 0;
	// SCR_DIRTY;
}

void UserInterface_Flush(UiHandle* uih)
{
	xSemaphoreTake(lcdMutexHandle, portMAX_DELAY);
	if (!uih->screenDirty || !uih->screenStatus) {
		xSemaphoreGive(lcdMutexHandle);
		return;
	}
	xSemaphoreGive(lcdMutexHandle);

	ssd1306_UpdateScreen(&hi2c1);
	SCR_DIRTY_CLR;
}

void UserInterface_TurnOnScreen(UiHandle* uih)
{
	xSemaphoreTake(lcdMutexHandle, portMAX_DELAY);
	if (uih->screenStatus == 0) 
	{
		uih->screenStatus = 1;
		UserInterface_ChangePage(uih, &uih->pages[0]);
	}
	xSemaphoreGive(lcdMutexHandle);
}

void UserInterface_TurnOffScreen(UiHandle* uih)
{
	if (uih->screenStatus == 1) 
	{
		uih->screenStatus = 0;
		
		ssd1306_Fill(Black);
		ssd1306_UpdateScreen(&hi2c1);
		SCR_DIRTY_CLR;
	}
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
			if (uih->currentMenu->next != NULL)
				uih->currentMenu = uih->currentMenu->next;
			break;
		case Key4:
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
			else if (uih->currentMenu->page != NULL)
			{
				UserInterface_ChangePage(uih, uih->currentMenu->page);
			}
			break;
		}
	}
	
	UserInterface_p_DrawMenu(uih);
}

void UserInterface_p_DrawActions(const char* actions)
{
	if (actions == NULL || strlen(actions) < 4)
		return;
	
	if (actions[g_menuActionsOffset+0] != ' ')
	{
		ssd1306_SetCursor(5, 40);
		ssd1306_WriteChar(actions[g_menuActionsOffset+0], Icon_11x18, White);
	}
	if (actions[g_menuActionsOffset+1] != ' ')
	{
		ssd1306_SetCursor(40, 40);
		ssd1306_WriteChar(actions[g_menuActionsOffset+1], Icon_11x18, White);
	}
	if (actions[g_menuActionsOffset+2] != ' ')
	{
		ssd1306_SetCursor(75, 40);
		ssd1306_WriteChar(actions[g_menuActionsOffset+2], Icon_11x18, White);
	}
	if (actions[g_menuActionsOffset+3] != ' ')
	{
		ssd1306_SetCursor(110, 40);
		ssd1306_WriteChar(actions[g_menuActionsOffset+3], Icon_11x18, White);
	}
}



// Page Callbacks

void UserInterface_InitPages(UiHandle* uih)
{
	uih->pages = malloc(sizeof(UiPage) * MaxPages);
	if (!uih->pages)
		return;
	
	/* Init Main Page */
	uih->pages[MainPageIdx].text = "Main Page";
	uih->pages[MainPageIdx].actionIcons = "&  )";
	uih->pages[MainPageIdx].onInit = NULL;
	uih->pages[MainPageIdx].onUpdate = mainPageUpdateCallback;
	uih->pages[MainPageIdx].onLeave = NULL;
	uih->pages[MainPageIdx].onHandleInput = mainPageInputCallback;
	uih->pages[MainPageIdx].data = NULL;

	/* Init Time List Page And Menus */
	TimeListPageData* timeListData = malloc(sizeof(TimeListPageData));	
	timeListData->plan = NULL;

	uih->pages[TimeListPageIdx].text = "Time List";
	uih->pages[TimeListPageIdx].menu = NULL;
	uih->pages[TimeListPageIdx].actionIcons = "%#$**+- ";
	uih->pages[TimeListPageIdx].onInit = timeListPageOnInitCallback;
	uih->pages[TimeListPageIdx].onUpdate = timeListPageUpdateCallback;
	uih->pages[TimeListPageIdx].onLeave = NULL;
	uih->pages[TimeListPageIdx].onHandleInput = timeListPageInputCallback;
	uih->pages[TimeListPageIdx].data = NULL;

	/* Init Settings Page And Menus */
	
	UiMenuPtr settingMenus = (UiMenuPtr)malloc(sizeof(UiMenu) * 2);
	if (!settingMenus)
		return;
	UserInterface_InitMenu(&settingMenus[0], "Time", 					NULL, &settingMenus[1], NULL, &uih->pages[SettingPageIdx], &uih->pages[SetTimePageIdx], NULL, NULL);
	UserInterface_InitMenu(&settingMenus[1], "Sleep Time", 		&settingMenus[0], NULL, NULL, &uih->pages[SettingPageIdx], &uih->pages[SetSleepTimePageIdx], NULL, NULL);
	
	uih->pages[SettingPageIdx].text = "Settings";
	uih->pages[SettingPageIdx].menu = settingMenus;
	uih->pages[SettingPageIdx].actionIcons = NULL;
	uih->pages[SettingPageIdx].onInit = NULL;
	uih->pages[SettingPageIdx].onUpdate = NULL;
	uih->pages[SettingPageIdx].onLeave = NULL;
	uih->pages[SettingPageIdx].onHandleInput = settingsPageInputCallback;
	uih->pages[SettingPageIdx].data = NULL;

	/* Init Set Time Page And Data */
	
	RTC_TimeTypeDef *sTime = malloc(sizeof(RTC_TimeTypeDef));
	if (HAL_RTC_GetTime(&hrtc, sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
		
	RTC_DateTypeDef *sDate = malloc(sizeof(RTC_DateTypeDef));
	if (HAL_RTC_GetDate(&hrtc, sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}

	SetTimePageData* setTimeData = malloc(sizeof(SetTimePageData));	
	setTimeData->sTime = sTime;
	setTimeData->sDate = sDate;
	
	uih->pages[SetTimePageIdx].text = NULL;
	uih->pages[SetTimePageIdx].actionIcons = "%('!";
	uih->pages[SetTimePageIdx].onInit = setTimePageOnInitCallback;
	uih->pages[SetTimePageIdx].onUpdate = setTimePageUpdateCallback;
	uih->pages[SetTimePageIdx].onLeave = NULL;
	uih->pages[SetTimePageIdx].onHandleInput = setTimePageInputCallback;
	uih->pages[SetTimePageIdx].data = setTimeData;

	/* Init Set Sleep Time Page And Data */

	SetSleepTimePageData* setSleepTimeData = malloc(sizeof(SetSleepTimePageData));	
	setSleepTimeData->sleepTime = 0;
	
	uih->pages[SetSleepTimePageIdx].text = NULL;
	uih->pages[SetSleepTimePageIdx].actionIcons = "%('!";
	uih->pages[SetSleepTimePageIdx].onInit = setSleepTimePageOnInitCallback;
	uih->pages[SetSleepTimePageIdx].onUpdate = setSleepTimePageUpdateCallback;
	uih->pages[SetSleepTimePageIdx].onLeave = NULL;
	uih->pages[SetSleepTimePageIdx].onHandleInput = setSleepTimePageInputCallback;
	uih->pages[SetSleepTimePageIdx].data = setSleepTimeData;

	/* Init Message Popup And Data */
	
	MessagePopupData* msgPopupData = malloc(sizeof(MessagePopupData));
	msgPopupData->secondCounter = 0;
	msgPopupData->secondsToShow = 0;
	msgPopupData->fallbackPage = NULL;
	
	uih->pages[MessagePopupIdx].text = NULL;
	uih->pages[MessagePopupIdx].actionIcons = "";
	uih->pages[MessagePopupIdx].onInit = messagePopupOnInitCallback;
	uih->pages[MessagePopupIdx].onUpdate = messagePopupUpdateCallback;
	uih->pages[MessagePopupIdx].onLeave = NULL;
	uih->pages[MessagePopupIdx].onHandleInput = messagePopupInputCallback;
	uih->pages[MessagePopupIdx].data = msgPopupData;


	UserInterface_ChangePage(uih, &uih->pages[MainPageIdx]);
}

void UserInterface_ShowPopup(UiHandle* uih, const char* text, uint8_t secondsToShow, UiPagePtr fallbackPage)
{
	if (fallbackPage == NULL)
		fallbackPage = uih->currentPage;
	
	MessagePopupData* popupData = uih->pages[MessagePopupIdx].data;
	popupData->secondsToShow = secondsToShow;
	popupData->fallbackPage = fallbackPage;
	uih->pages[MessagePopupIdx].text = text;
	UserInterface_ChangePage(uih, &uih->pages[MessagePopupIdx]);
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

	UserInterface_p_DrawActions(hnd->currentPage->actionIcons);
	
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
		case Key4:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[TimeListPageIdx]);
			break;
		default:
			break;
	}
}

void timeListPageOnInitCallback(void* uih)
{
	UiHandle* hnd = uih;
	TimeListPageData* data = hnd->currentPage->data;

	data->plan = Timer_GetNextFullSlot(timeList, 0);
}

uint8_t timeListPageUpdateCallback(void* uih, uint32_t since)
{
	if (since < 400)
		return 0;
	UiHandle* hnd = uih;
	TimeListPageData* data = hnd->currentPage->data;
	
	ssd1306_Fill(Black);
	
	if (data->plan == NULL)
	{
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString("Empty List!", Font_11x18, White);
	} else {
		char buff[16] = {0};
		snprintf(buff, 16, "%c: %d", data->plan->mode == Monthly? 'M': (data->plan->mode == Weekly? 'W': 'D'), data->plan->day);
		
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(buff, Font_11x18, White);
	}
	
	UserInterface_p_DrawActions(hnd->currentPage->actionIcons);

	SCR_DIRTY_ARG(hnd);
	return 1;
}

void timeListPageInputCallback(void* uih, enum ActionType action)
{
	UiHandle* hnd = uih;
	TimeListPageData* data = hnd->currentPage->data;

	switch(action)
	{
		case Key1:
			ACT_OFST(
							UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[MainPageIdx]), 
		
							g_menuActionsOffset -= 4
			)
			break;
		case Key2:
		{
			ACT_OFST(
							TimePlan* p = Timer_GetPrevFullSlot(timeList, Timer_ToOffset(timeList, data->plan)); 
							if(p) data->plan = p, 

							UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[MainPageIdx]) /* Go to add page */
			)
			break;
		}
		case Key3:
			ACT_OFST(
							TimePlan* p = Timer_GetNextFullSlot(timeList, Timer_ToOffset(timeList, data->plan)); 
							if(p) data->plan = p, 

							Timer_RemovePlan(timeList, Timer_ToOffset(timeList, data->plan)); 
							data->plan = Timer_GetNextFullSlot(timeList, Timer_ToOffset(timeList, data->plan))
			)
			break;
		case Key4:
			ACT_OFST(
							g_menuActionsOffset += 4, 
		
							UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[MainPageIdx])
			)
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

void setTimePageOnInitCallback(void* uih)
{
	UiHandle* hnd = uih;
	SetTimePageData* data = hnd->currentPage->data;

	data->settingStep = 0;
	data->updateStep = 0;
	
	if (HAL_RTC_GetTime(&hrtc, data->sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
	
	if (HAL_RTC_GetDate(&hrtc, data->sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
}

uint8_t setTimePageUpdateCallback(void* uih, uint32_t since)
{
	if (since < 300)
		return 0;
	
	UiHandle* hnd = uih;
	SetTimePageData* data = hnd->currentPage->data;

	ssd1306_Fill(Black);
	if (data->settingStep == 0)
	{
		char dateBuff[24] = {0};
		char ptr[7] = {' ', ' ', ' ', ' ', ' ', ' ', '\0'};
		if (data->updateStep == 0) {
			ptr[0] = '[';
			ptr[1] = ']';
		}
		else if (data->updateStep == 1) {
			ptr[2] = '[';
			ptr[3] = ']';
		}
		else if (data->updateStep == 2) {
			ptr[4] = '[';
			ptr[5] = ']';
		}
		
		if (data->updateStep != 2)
			snprintf(dateBuff, 16, "%c%02d%c-%c%02d%c-", ptr[0], _D(data->sDate->Year), ptr[1], 
																										ptr[2], _D(data->sDate->Month)+1, ptr[3]);
		else
			snprintf(dateBuff, 16, "-%c%02d%c-%c%02d%c", ptr[2], _D(data->sDate->Month)+1, ptr[3], 
																									ptr[4], _D(data->sDate->Date)+1, ptr[5]);
		
		// should set date
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(dateBuff, Font_11x18, White);
	}
	else if (data->settingStep == 1)
	{
		const char* week_days[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(week_days[data->sDate->WeekDay], Font_11x18, White);
	}
	else if (data->settingStep == 2)
	{
		char timeBuff[16] = {0};
		char ptr[5] = {' ', ' ', ' ', ' ', '\0'};
		if (data->updateStep == 0) {
			ptr[0] = '[';
			ptr[1] = ']';
		}
		else if (data->updateStep == 1) {
			ptr[2] = '[';
			ptr[3] = ']';
		}
		snprintf(timeBuff, 16, "%c%02d%c:%c%02d%c", ptr[0], _D(data->sTime->Hours), ptr[1], 
																								ptr[2], _D(data->sTime->Minutes), ptr[3]);

		// should set time
		ssd1306_SetCursor(5, 5);
		ssd1306_WriteString(timeBuff, Font_11x18, White);
	}

	UserInterface_p_DrawActions(hnd->currentPage->actionIcons);
	
	SCR_DIRTY_ARG(hnd);
	return 1;
}

void setTimePageInputCallback(void* uih, enum ActionType action)
{
	UiHandle* hnd = uih;
	SetTimePageData* data = hnd->currentPage->data;

	switch(action)
	{
		case Key1:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[SettingPageIdx]);
			break;
		case Key2:
			if (data->settingStep == 0)
			{
				data->updateStep == 0 && (data->sDate->Year = _E((_D(data->sDate->Year) + 99 - 1) % 99));
				data->updateStep == 1 && (data->sDate->Month = _E((_D(data->sDate->Month) + 12 - 1) % 12));
				data->updateStep == 2 && (data->sDate->Date = _E((_D(data->sDate->Date) + 31 - 1) % 31));
			}
			else if (data->settingStep == 1)
			{
				data->sDate->WeekDay = _E((_D(data->sDate->WeekDay) + 7 - 1) % 7);
			}
			else if (data->settingStep == 2)
			{
				data->updateStep == 0 && (data->sTime->Hours = _E((_D(data->sTime->Hours) + 24 - 1) % 24));
				data->updateStep == 1 && (data->sTime->Minutes = _E((_D(data->sTime->Minutes) + 60 - 1) % 60));
			}
			break;
		case Key3:
			if (data->settingStep == 0)
			{
				data->updateStep == 0 && (data->sDate->Year = _E((_D(data->sDate->Year) + 1) % 99));
				data->updateStep == 1 && (data->sDate->Month = _E((_D(data->sDate->Month) + 1) % 12));
				data->updateStep == 2 && (data->sDate->Date = _E((_D(data->sDate->Date) + 1) % 31));
			}
			else if (data->settingStep == 1)
			{
				data->sDate->WeekDay = (data->sDate->WeekDay + 1) % 7;
			}
			else if (data->settingStep == 2)
			{
				data->sTime->Seconds = 0;
				data->updateStep == 0 && (data->sTime->Hours = _E((_D(data->sTime->Hours) + 1) % 24));
				data->updateStep == 1 && (data->sTime->Minutes = _E((_D(data->sTime->Minutes) + 1) % 60));
			}
			break;
		case Key4:
			if (data->settingStep == 0)
			{
				data->updateStep = (data->updateStep + 1) % 3;
				if (data->updateStep == 0)
					data->settingStep++;
			}
			else if (data->settingStep == 1)
			{
				data->settingStep++;
			}
			else if (data->settingStep == 2)
			{
				data->updateStep = (data->updateStep + 1) % 2;
				if (data->updateStep == 0) {
					if (HAL_RTC_SetDate(&hrtc, data->sDate, RTC_FORMAT_BCD) != HAL_OK || HAL_RTC_SetTime(&hrtc, data->sTime, RTC_FORMAT_BCD) != HAL_OK)
						Error_Handler();
					else
						UserInterface_ShowPopup(uih, "Time Set!", 3, &((UiHandle*)uih)->pages[SettingPageIdx]);
				}
			}
			break;
		default:
			break;
	}
}

void setSleepTimePageOnInitCallback(void* uih)
{
	UiHandle* hnd = uih;
	SetSleepTimePageData* data = hnd->currentPage->data;

	data->sleepTime = 30;	// set to saved value
}

uint8_t setSleepTimePageUpdateCallback(void* uih, uint32_t since)
{
	if (since < 300)
		return 0;
	
	UiHandle* hnd = uih;
	SetSleepTimePageData* data = hnd->currentPage->data;
	
	ssd1306_Fill(Black);

	char timeBuff[8] = {0};
	snprintf(timeBuff, 8, "%d", data->sleepTime);
	
	ssd1306_SetCursor(5, 5);
	ssd1306_WriteString(timeBuff, Font_11x18, White);

	UserInterface_p_DrawActions(hnd->currentPage->actionIcons);
	SCR_DIRTY_ARG(hnd);
	return 1;
}

void setSleepTimePageInputCallback(void* uih, enum ActionType action)
{
	UiHandle* hnd = uih;
	SetSleepTimePageData* data = hnd->currentPage->data;

	switch(action)
	{
		case Key1:
			UserInterface_ChangePage(uih, &((UiHandle*)uih)->pages[SettingPageIdx]);
			break;
		case Key2:
			if (data->sleepTime == 0)
				data->sleepTime = 120;
			else
				data->sleepTime--;
			break;
		case Key3:
			if (data->sleepTime == 120)
				data->sleepTime = 0;
			else
				data->sleepTime++;
			break;
		case Key4:
			if (xTimerChangePeriod(sleepTimerHandle, data->sleepTime * 1000 / portTICK_PERIOD_MS, 100) == pdPASS)
				UserInterface_ShowPopup(uih, "Time Set!", 3, &((UiHandle*)uih)->pages[SettingPageIdx]);
			else
				Error_Handler();
			break;
		default:
			break;
	}
}

void messagePopupOnInitCallback(void* uih)
{
	UiHandle* hnd = uih;
	MessagePopupData* data = hnd->currentPage->data;

	data->secondCounter = 0;
}

uint8_t messagePopupUpdateCallback(void* uih, uint32_t since)
{
	if (since < 950)
		return 0;
	
	UiHandle* hnd = uih;
	MessagePopupData* data = hnd->currentPage->data;
	if (++data->secondCounter >= data->secondsToShow)
		UserInterface_ChangePage(uih, data->fallbackPage);
	
	return 1;
}

void messagePopupInputCallback(void* uih, enum ActionType action)
{
	UiHandle* hnd = uih;
	MessagePopupData* data = hnd->currentPage->data;

	UserInterface_ChangePage(uih, data->fallbackPage);
}

