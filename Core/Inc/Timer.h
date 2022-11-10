#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define INVALID_SLOT			255
#define MAX_PLANS					50
#define MAX_TIMES					100
#define MAX_TIMES_PER_DAY	16

enum TimeMode {
	NotAssigned,
	Daily,
	Weekly,
	Monthly
};

typedef struct timer_item {
	uint16_t time;
	uint8_t  status;
} TimerItem;

typedef struct time_plan {
	enum TimeMode mode;
	uint8_t day;		// used in monthly and weekly

	TimerItem* items[MAX_TIMES_PER_DAY];
} TimePlan;

typedef struct time_list {
	TimePlan plans[MAX_PLANS];
} TimeList;

void Timer_NewTimeList(TimeList** list);
uint8_t Timer_GetEmptySlot(TimeList* list);
TimePlan* Timer_AddPlan(TimeList* list, enum TimeMode mode);
void Timer_RemovePlan(TimeList* list, uint8_t offset);
TimePlan* Timer_GetNextFullSlot(TimeList* list, uint8_t start);
TimePlan* Timer_GetPrevFullSlot(TimeList* list, uint8_t start);

TimerItem* TimePlan_AddItem(TimePlan* plan);
void TimePlan_RemoveItem(TimePlan* plan, uint8_t offset);
uint8_t TimePlan_GetEmptySlot(TimePlan* plan);
TimerItem* TimePlan_GetNextFullSlot(TimePlan* plan, uint8_t start);
TimerItem* TimePlan_GetPrevFullSlot(TimePlan* plan, uint8_t start);

uint8_t TimePlan_IsEmpty(TimePlan* plan);
uint8_t TimerItem_IsEmpty(TimerItem* item);

#endif	//TIMER_H