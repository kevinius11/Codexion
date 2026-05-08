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

int	has_priority(t_waiter a, t_waiter b)
{
	if (a.timestamp < b.timestamp)
		return (1);
	if (a.timestamp == b.timestamp && a.coder_id < b.coder_id)
		return (1);
	return (0);
}

t_waiter	heap_peek(t_heap *heap)
{
	t_waiter	empty;

	empty.coder_id = -1;
	empty.timestamp = -1;
	if (heap->size == 0)
		return (empty);
	return (heap->waiters[0]);
}

static void	heap_sift_down(t_heap *heap, int i)
{
	int		smallest;
	int		left;
	int		right;
	t_waiter	tmp;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < heap->size
			&& has_priority(heap->waiters[left], heap->waiters[smallest]))
			smallest = left;
		if (right < heap->size
			&& has_priority(heap->waiters[right], heap->waiters[smallest]))
			smallest = right;
		if (smallest == i)
			break ;
		tmp = heap->waiters[i];
		heap->waiters[i] = heap->waiters[smallest];
		heap->waiters[smallest] = tmp;
		i = smallest;
	}
}

static void	heap_sift_up(t_heap *heap, int i)
{
	int		parent;
	t_waiter	tmp;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (has_priority(heap->waiters[i], heap->waiters[parent]))
		{
			tmp = heap->waiters[i];
			heap->waiters[i] = heap->waiters[parent];
			heap->waiters[parent] = tmp;
			i = parent;
		}
		else
			break ;
	}
}

t_waiter	heap_extract_min(t_heap *heap)
{
	t_waiter	empty;
	t_waiter	min;

	empty.coder_id = -1;
	empty.timestamp = -1;
	if (heap->size == 0)
		return (empty);
	min = heap->waiters[0];
	heap->waiters[0] = heap->waiters[heap->size - 1];
	heap->size--;
	if (heap->size > 0)
		heap_sift_down(heap, 0);
	return (min);
}

void	heap_remove_by_id(t_heap *heap, int coder_id)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->waiters[i].coder_id == coder_id)
		{
			heap->waiters[i] = heap->waiters[heap->size - 1];
			heap->size--;
			if (i < heap->size)
			{
				heap_sift_up(heap, i);
				heap_sift_down(heap, i);
			}
			return ;
		}
		i++;
	}
}

void	heap_insert(t_heap *heap, t_waiter new)
{
	int	i;

	if (heap->size == heap->capacity)
		return ;
	i = heap->size;
	heap->waiters[i] = new;
	heap->size++;
	heap_sift_up(heap, i);
}
