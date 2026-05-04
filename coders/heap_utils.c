/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:01:18 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/02 21:01:21 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_waiter heap_peek(t_heap *heap)
{
	t_waiter empty = { -1, -1 };
	if (heap->size == 0)
		return (empty);
	return (heap->waiters[0]);
}

t_waiter heap_extract_min(t_heap *heap)
{
	t_waiter empty = { -1, -1 };

	if (heap->size == 0)
		return (empty);

	t_waiter min = heap->waiters[0];

	heap->waiters[0] = heap->waiters[heap->size - 1];
	heap->size--;

	int i = 0;
	while (1)
	{
		int left = 2 * i + 1;
		int right = 2 * i + 2;
		int smallest = i;

		if (left < heap->size &&
				heap->waiters[left].timestamp < heap->waiters[smallest].timestamp)
			smallest = left;
		if (right < heap->size &&
				heap->waiters[right].timestamp < heap->waiters[smallest].timestamp)
			smallest = right;
		if (smallest == i)
			break;
		
		t_waiter tmp = heap->waiters[i];
		heap->waiters[i] = heap->waiters[smallest];
		heap->waiters[smallest] = tmp;

		i = smallest;
	}
	return (min);
}

void	heap_insert(t_heap *heap, t_waiter new)
{
	if (heap->size == heap->capacity)
		return;

	int i = heap->size;
	heap->waiters[i] = new;
	heap->size++;

	while (i > 0)
	{
		int parent = (i - 2) / 2;

		if (heap->waiters[i].timestamp >= heap->waiters[parent].timestamp)
			break;
		t_waiter tmp = heap->waiters[i];
		heap->waiters[i] = heap->waiters[parent];
		heap->waiters[parent] = tmp;

		i = parent;
	}
}
