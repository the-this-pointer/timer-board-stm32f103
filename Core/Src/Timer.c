#include "Timer.h"
#include <stdlib.h>
#include <string.h>

void Timer_TimeListInit(TimeList* list)
{
	uint8_t i,j;
	for(i = 0; i < MAX_PLANS; i++)
	{
		list->plans[i].mode = NotAssigned;
		list->plans[i].day = 0;
		
		for (j = 0; j < MAX_TIMES_PER_PLAN; j++)
			list->plans[i].items[j] = NULL;
	}
}

uint8_t Timer_GetEmptySlot(TimeList* list)
{
	uint8_t i;
	for(i = 0; i < MAX_PLANS; i++)
	{
		if (TimePlan_IsEmpty(&list->plans[i]))
			return i;
	}
	return INVALID_SLOT;
}

TimePlan* Timer_AddPlan(TimeList* list, enum TimeMode mode)
{
	if (mode == NotAssigned)
		return NULL;
	
	uint8_t slot = Timer_GetEmptySlot(list);
	if (slot == INVALID_SLOT)
		return NULL;

	list->plans[slot].mode = mode;
	list->plans[slot].day	=	0;
	memset(list->plans[slot].items, 0, sizeof(TimerItem*) * MAX_TIMES_PER_PLAN);
	
	return &(list->plans[slot]);
}

void Timer_RemovePlan(TimeList* list, uint8_t offset)
{
	if (offset == INVALID_SLOT || MAX_PLANS < offset)
		return;

	int i;
	for (i = 0; i < MAX_TIMES_PER_PLAN; i++)
		TimePlan_RemoveItem(&list->plans[offset], i);
	
	list->plans[offset].mode = NotAssigned;
	list->plans[offset].day	=	0;
}

TimePlan* Timer_GetNextFullSlot(TimeList* list, uint8_t start)
{
	if (start == INVALID_SLOT)
		return NULL;

	uint8_t i;
	for(i = start + 1; i < MAX_PLANS + start + 2; i++)
	{
		if (!TimePlan_IsEmpty(&list->plans[i % MAX_PLANS]))
			return &list->plans[i % MAX_PLANS];
	}
	return NULL;
}

TimePlan* Timer_GetPrevFullSlot(TimeList* list, uint8_t start)
{
	if (start == INVALID_SLOT)
		return NULL;

	int8_t i;
	for(i = start - 1; i >= start - MAX_PLANS - 2; i--)
	{
		uint8_t idx = i % MAX_PLANS < 0? MAX_PLANS + (i % MAX_PLANS): i % MAX_PLANS;
		if (!TimePlan_IsEmpty(&list->plans[idx]))
			return &list->plans[idx];
	}
	return NULL;
}

uint8_t Timer_ToOffset(TimeList* list, TimePlan* plan)
{
	if (plan == NULL)
		return INVALID_SLOT;
	
	return plan - list->plans;
}

void Timer_Sort(TimeList* list)
{
	uint8_t i, j;
  for (i = 0; i < MAX_PLANS - 1; i++)
		for (j = 0; j < MAX_PLANS - i - 1; j++)
			TimePlan_Sort(&list->plans[j]);
			TimePlan_Sort(&list->plans[j+1]);

			if (TimePlan_Greater(&list->plans[j], &list->plans[j+1]))
			{
				TimePlan* lhs = &list->plans[j];
				TimePlan* rhs = &list->plans[j+1];
				TimePlan_Swap(&lhs, &rhs);
			}
}

TimerItem* TimePlan_AddItem(TimePlan* plan)
{
	uint8_t slot = TimePlan_GetEmptySlot(plan);
	if (slot == INVALID_SLOT)
		return NULL;
	
	if (plan->items[slot] == NULL)
	{
		plan->items[slot] = malloc(sizeof(TimerItem));
		if (!plan->items[slot])
			return NULL;
	}
	memset(plan->items[slot], 0, sizeof(TimerItem));
	return plan->items[slot];
}

void TimePlan_RemoveItem(TimePlan* plan, uint8_t offset)
{
	if (offset == INVALID_SLOT || MAX_TIMES_PER_PLAN < offset || plan->items[offset] == NULL)
		return;

	free(plan->items[offset]);
	plan->items[offset] = NULL;
}

uint8_t TimePlan_GetEmptySlot(TimePlan* plan)
{
	uint8_t i;
	for(i = 0; i < MAX_TIMES_PER_PLAN; i++)
	{
		if (TimerItem_IsEmpty(plan->items[i]))
			return i;
	}
	return INVALID_SLOT;
}

TimerItem* TimePlan_GetNextFullSlot(TimePlan* plan, uint8_t start)
{
	if (start == INVALID_SLOT)
		return NULL;

	uint8_t i;
	for(i = start + 1; i < MAX_TIMES_PER_PLAN + start + 2; i++)
	{
		if (!TimerItem_IsEmpty(plan->items[i % MAX_TIMES_PER_PLAN]))
			return plan->items[i % MAX_TIMES_PER_PLAN];
	}
	return NULL;
}

TimerItem* TimePlan_GetPrevFullSlot(TimePlan* plan, uint8_t start)
{
	if (start == INVALID_SLOT)
		return NULL;

	int8_t i;
	for(i = start - 1; i >= start - MAX_TIMES_PER_PLAN - 2; i--)
	{
		uint8_t idx = i % MAX_TIMES_PER_PLAN < 0? MAX_TIMES_PER_PLAN + (i % MAX_TIMES_PER_PLAN): i % MAX_TIMES_PER_PLAN;
		if (!TimerItem_IsEmpty(plan->items[idx]))
			return plan->items[idx];
	}
	return NULL;
}

uint8_t TimePlan_ToOffset(TimePlan* plan, TimerItem* item)
{
	if (item == NULL)
		return INVALID_SLOT;
	
	uint8_t offset = INVALID_SLOT;
	uint8_t i;
	for (i = 0; i < MAX_TIMES_PER_PLAN; i++)
		if (item == plan->items[i])
		{
			offset = i;
			break;
		}
	
	return offset;
}

void TimePlan_Sort(TimePlan* plan)
{
	uint8_t i, j;
  for (i = 0; i < MAX_TIMES_PER_PLAN - 1; i++)
		for (j = 0; j < MAX_TIMES_PER_PLAN - i - 1; j++)
			if (TimerItem_Greater(plan->items[j], plan->items[j+1]))
				TimerItem_Swap(&plan->items[j], &plan->items[j+1]);
}

void TimePlan_Swap(TimePlan** a, TimePlan** b)
{
	TimePlan* temp = *a;
  *a = *b;
  *b = temp;
}

uint8_t TimePlan_Greater(TimePlan* lhs, TimePlan* rhs)
{
	uint32_t lhs_val = 0, rhs_val = 0;
	
	if (lhs != NULL)
		lhs_val = (Monthly - lhs->mode) * 60 + (32 - lhs->day);
	
	if (rhs != NULL)
		rhs_val = (Monthly - rhs->mode) * 60 + (32 - rhs->day);
	
	if (lhs_val > rhs_val)
		return 1;
	return 0;
}

uint8_t TimePlan_IsEmpty(TimePlan* plan)
{
	return plan->mode == NotAssigned;
}

void TimerItem_Swap(TimerItem** a, TimerItem** b)
{
  TimerItem* temp = *a;
  *a = *b;
  *b = temp;
}

uint8_t TimerItem_Greater(TimerItem* lhs, TimerItem* rhs)
{
	uint16_t lhs_val = 0, rhs_val = 0;
	
	if (lhs != NULL)
		lhs_val = lhs->hours * 60 + lhs->minutes;
	
	if (rhs != NULL)
		rhs_val = rhs->hours * 60 + rhs->minutes;
	
	if (lhs_val > rhs_val)
		return 1;
	return 0;
}

uint8_t TimerItem_IsEmpty(TimerItem* item)
{
	return item == NULL;
}
