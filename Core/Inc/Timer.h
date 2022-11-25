#ifndef TIMER_H
#define TIMER_H

#include <global_def.h>

void Timer_LoadData(EEPROM_TimeList* timeListData);
void Timer_SaveData(EEPROM_TimeList* timeListData);

void Timer_TimeListInit(TimeList* list);
uint8_t Timer_GetEmptySlot(TimeList* list);
TimePlan* Timer_AddPlan(TimeList* list, enum TimeMode mode);
void Timer_RemovePlan(TimeList* list, uint8_t offset);
TimePlan* Timer_GetNextFullSlot(TimeList* list, uint8_t start);
TimePlan* Timer_GetPrevFullSlot(TimeList* list, uint8_t start);
uint8_t Timer_ToOffset(TimeList* list, TimePlan* plan);
void Timer_Sort(TimeList* list);

TimerItem* TimePlan_AddItem(TimePlan* plan);
void TimePlan_RemoveItem(TimePlan* plan, uint8_t offset);
uint8_t TimePlan_GetEmptySlot(TimePlan* plan);
TimerItem* TimePlan_GetNextFullSlot(TimePlan* plan, uint8_t start);
TimerItem* TimePlan_GetPrevFullSlot(TimePlan* plan, uint8_t start);
uint8_t TimePlan_ToOffset(TimePlan* plan, TimerItem* item);
void TimePlan_Sort(TimePlan* plan);
void TimePlan_Swap(TimePlan* a, TimePlan* b);
uint8_t TimePlan_Greater(TimePlan* lhs, TimePlan* rhs);
uint8_t TimePlan_IsEmpty(TimePlan* plan);

void TimerItem_Swap(TimerItem* a, TimerItem* b);
uint8_t TimerItem_Greater(TimerItem* lhs, TimerItem* rhs);
uint8_t TimerItem_IsEmpty(TimerItem* item);

void Error_Handler(void);

#endif	//TIMER_H