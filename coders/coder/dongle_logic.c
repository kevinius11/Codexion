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

static void	set_wait_ts(struct timespec *ts, long remaining)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec += remaining / 1000;
	ts->tv_nsec += (remaining % 1000) * 1000000;
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}

static void	dongle_wait_loop(t_coders *coder, t_dongle *dongle)
{
	long			rem;
	struct timespec	ts;

	rem = 0;
	if (dongle->available && dongle->last_used_time != 0)
		rem = coder->data->dongle_cooldown
			- (get_time_ms() - dongle->last_used_time);
	if (rem > 0)
	{
		set_wait_ts(&ts, rem);
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
	}
	else
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
}

int	wait_for_dongle(t_coders *coder, t_dongle *dongle)
{
	t_waiter	ticket;

	ticket.coder_id = coder->id;
	ticket.timestamp = make_ticket_timestamp(coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(&dongle->queue, ticket);
	while (1)
	{
		if (is_sim_over(coder->data))
		{
			heap_remove_by_id(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		if (dongle_is_acquirable(dongle, coder))
			break ;
		dongle_wait_loop(coder, dongle);
	}
	heap_remove_by_id(&dongle->queue, coder->id);
	dongle->available = 0;
	pthread_mutex_unlock(&dongle->mutex);
	print_status(coder->data, coder->id, "has taken a dongle");
	return (1);
}
