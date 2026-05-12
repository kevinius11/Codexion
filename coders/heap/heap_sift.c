/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_sift.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 20:34:20 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/12 20:34:27 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_waiters(t_waiter *a, t_waiter *b)
{
	t_waiter	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_sift_down(t_heap *heap, int i)
{
	int	smallest;
	int	left;
	int	right;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < heap->size
			&& has_priority(heap->waiters[left],
				heap->waiters[smallest]))
			smallest = left;
		if (right < heap->size
			&& has_priority(heap->waiters[right],
				heap->waiters[smallest]))
			smallest = right;
		if (smallest == i)
			break ;
		swap_waiters(&heap->waiters[i],
			&heap->waiters[smallest]);
		i = smallest;
	}
}

void	heap_sift_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!has_priority(heap->waiters[i],
				heap->waiters[parent]))
			break ;
		swap_waiters(&heap->waiters[i],
			&heap->waiters[parent]);
		i = parent;
	}
}
