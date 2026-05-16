/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_logic.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 22:19:43 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/12 22:19:47 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	dongle_is_acquirable(t_dongle *dongle, t_coders *coder)
{
	long	elapsed;

	if (!dongle->available)
		return (0);
	if (dongle->last_used_time != 0)
	{
		elapsed = get_time_ms() - dongle->last_used_time;
		if (elapsed < coder->data->dongle_cooldown)
			return (0);
	}
	if (heap_peek(&dongle->queue).coder_id != coder->id)
		return (0);
	return (1);
}

static void	dongle_wait_loop(t_coders *coder, t_dongle *dongle)
{
	(void)coder;
	(void)dongle;
	pthread_cond_wait(&dongle->cond, &dongle->mutex);
}

int	wait_for_dongle(t_coders *coder, t_dongle *dongle, long priority)
{
	t_waiter	ticket;

	ticket.coder_id = coder->id;
	ticket.timestamp = priority;
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(&dongle->queue, ticket);
	while (!dongle_is_acquirable(dongle, coder))
	{
		if (coder->data->simulation_over)
		{
			heap_remove_by_id(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		dongle_wait_loop(coder, dongle);
	}
	heap_remove_by_id(&dongle->queue, coder->id);
	dongle->available = 0;
	pthread_mutex_unlock(&dongle->mutex);
	print_status(coder->data, coder->id, "has taken a dongle");
	return (1);
}

void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = 1;
	dongle->last_used_time = get_time_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
