#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "ssd1306.h"
#include "fonts.h"
#include "main.h"
#include "utils.h"
#include "Timer.h"
#include <stdlib.h>
#include <stdio.h>

enum ActionType {Key1, Key2, Key3, Key4};

typedef void(*pageLifeCycleCallback)(void* uih);
typedef void(*pageInputCallback)(void* uih, enum ActionType action);
typedef uint8_t(*pageUpdateCallback)(void* uih, uint32_t since);
typedef void(*menuCallback)();

typedef struct {
	const char* caption;
	void* prev;
	void* next;
	void* parent;
	void* children;
	
	void* parentPage;
	void* page;
	
	menuCallback callback;
} UiMenu, *UiMenuPtr;

typedef struct {
	const char* text;
	UiMenuPtr	menu;
	const char* actionIcons;
	
	pageLifeCycleCallback onInit;
	pageLifeCycleCallback onLeave;
	pageUpdateCallback onUpdate;
	pageInputCallback onHandleInput;
	
	void* data;
} UiPage, *UiPagePtr;

typedef struct {
	UiPagePtr pages;
	UiPagePtr currentPage;
	UiMenuPtr currentMenu;
	uint8_t screenDirty;
	uint8_t screenStatus;
} UiHandle;

void UserInterface_Init();
void UserInterface_ChangePage(UiHandle* uih, UiPagePtr page);
void UserInterface_HandleInput(UiHandle* uih, uint16_t input);
void UserInterface_InitMenu(UiMenuPtr menu, const char* caption, void* prev, void* next, void* parent, void* parentPage, void* page, void* children, menuCallback callback);
uint8_t UserInterface_Update(UiHandle* uih, uint32_t since);
void UserInterface_Flush(UiHandle* uih);

void UserInterface_TurnOnScreen(UiHandle* uih);
void UserInterface_TurnOffScreen(UiHandle* uih);
uint8_t UserInterface_ScreenIsOn(UiHandle* uih);

void UserInterface_p_DrawMenu(UiHandle* uih);
void UserInterface_p_onHandleMenuInput(UiHandle* uih, enum ActionType action);
void UserInterface_p_DrawActions(const char* actions);
uint16_t UserInterface_p_HandleInputs();

// Tasks
void StartUartTask(void const * argument);
void StartUiTask(void const * argument);
void SleepTimerCallback(void const * argument);


// Ui Callbacks
enum Pages {
	MainPageIdx, TimeListPageIdx, AddTimePlanPageIdx, AddTimerItemPageIdx,
	SettingPageIdx, SetTimePageIdx, SetSleepTimePageIdx, SendTimesPageIdx,
	MessagePopupIdx, 
	MaxPages
};
extern UiHandle uih;

void UserInterface_InitPages(UiHandle* uih);
void UserInterface_ShowPopup(UiHandle* uih, const char* text, uint8_t secondsToShow, UiPagePtr fallbackPage);
/* Main Page */
uint8_t mainPageUpdateCallback(void* uih, uint32_t since);
void mainPageInputCallback(void* uih, enum ActionType action);
/* Time List */
typedef struct time_list_data {
	TimePlan* plan;
	TimerItem* showingItem;
	uint8_t screenIndex;
} TimeListPageData;
void timeListPageOnInitCallback(void* uih);
void timeListPageOnLeaveCallback(void* uih);
uint8_t timeListPageUpdateCallback(void* uih, uint32_t since);
void timeListPageInputCallback(void* uih, enum ActionType action);
/* Add Time Plan */
typedef struct add_time_plan_data {
	TimePlan plan;
	TimePlan* editingPlan;
	uint8_t settingStep;
} AddTimePlanPageData;
void addTimePlanPageOnInitCallback(void* uih);
uint8_t addTimePlanPageUpdateCallback(void* uih, uint32_t since);
void addTimePlanPageInputCallback(void* uih, enum ActionType action);
/* Add Timer Item */
typedef struct add_timer_item_data {
	TimePlan* plan;
	TimerItem* editingItem;
	uint8_t settingStep;	
	uint8_t hours;
	uint8_t minutes;
	uint8_t status;
} AddTimerItemPageData;
void addTimerItemPageOnInitCallback(void* uih);
uint8_t addTimerItemPageUpdateCallback(void* uih, uint32_t since);
void addTimerItemPageInputCallback(void* uih, enum ActionType action);
/* Settings */
void settingsPageInputCallback(void* uih, enum ActionType action);
/* Set Time */
typedef struct set_time_data {
	uint8_t settingStep;	/* setting date(0) or weekday(1) or time(2) */
	uint8_t updateStep;		/* used for pointing which section is active */
	RTC_TimeTypeDef *sTime;
	RTC_DateTypeDef *sDate;
} SetTimePageData;

void setTimePageOnInitCallback(void* uih);
uint8_t setTimePageUpdateCallback(void* uih, uint32_t since);
void setTimePageInputCallback(void* uih, enum ActionType action);
/* Set Sleep Time */
typedef struct set_sleep_time_data {
	uint8_t sleepTime;
} SetSleepTimePageData;
void setSleepTimePageOnInitCallback(void* uih);
uint8_t setSleepTimePageUpdateCallback(void* uih, uint32_t since);
void setSleepTimePageInputCallback(void* uih, enum ActionType action);
/* Send Times */
typedef enum send_times_state {
	Init,
	CompatCheck,
	Halt,
	Clr,
	SendTime,
	Save,
	Start,
	Success,
	Cancel,
	ErrorOffset,	// marker for error values
	CompatError,
	HaltError,
	ClrError,
	SendTimeError,
	SaveError,
	StartError,
} SendTimesState;
typedef struct send_times_data {
	SendTimesState state;
} SendTimesPageData;
void sendTimesPageOnInitCallback(void* uih);
void sendTimesPageOnLeaveCallback(void* uih);
uint8_t sendTimesPageUpdateCallback(void* uih, uint32_t since);
void sendTimesPageInputCallback(void* uih, enum ActionType action);
/* Message Popup */
typedef struct message_popup_data {
	uint8_t secondsToShow;
	uint8_t secondCounter;
	UiPagePtr fallbackPage;
} MessagePopupData;
void messagePopupOnInitCallback(void* uih);
uint8_t messagePopupUpdateCallback(void* uih, uint32_t since);
void messagePopupInputCallback(void* uih, enum ActionType action);

uint8_t sendUartCommand(const char* cmd, void* data, uint8_t length);
uint8_t sendUartCommandChunked(const char* cmd, void* data, uint16_t length);
void processUartCommand(const char* data, uint8_t length);
#endif	// USERINTERFACE_H
