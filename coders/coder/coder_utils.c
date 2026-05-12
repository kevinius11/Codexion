/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 22:20:08 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/12 22:20:13 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_status(t_data *data, int id, char *status)
{
	pthread_mutex_lock(&data->log_mutex);
	pthread_mutex_lock(&data->sim_mutex);
	if (!data->simulation_over)
		printf("%ld %d %s\n", get_time_ms() - data->start_time, id, status);
	pthread_mutex_unlock(&data->sim_mutex);
	pthread_mutex_unlock(&data->log_mutex);
}

int	is_sim_over(t_data *data)
{
	int	over;

	pthread_mutex_lock(&data->sim_mutex);
	over = data->simulation_over;
	pthread_mutex_unlock(&data->sim_mutex);
	return (over);
}

long	make_ticket_timestamp(t_coders *coder)
{
	long	ts;

	if (coder->data->scheduler == FIFO)
		return (get_time_ms());
	pthread_mutex_lock(&coder->data->sim_mutex);
	ts = coder->last_compilation + coder->data->time_to_burnout;
	pthread_mutex_unlock(&coder->data->sim_mutex);
	return (ts);
}

int	try_take_dongle(t_coders *coder, t_dongle *dongle)
{
	t_waiter	ticket;
	int			acquired;

	ticket.coder_id = coder->id;
	ticket.timestamp = make_ticket_timestamp(coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(&dongle->queue, ticket);
	acquired = dongle_is_acquirable(dongle, coder);
	heap_remove_by_id(&dongle->queue, coder->id);
	if (acquired)
		dongle->available = 0;
	pthread_mutex_unlock(&dongle->mutex);
	if (acquired)
		print_status(coder->data, coder->id, "has taken a dongle");
	return (acquired);
}

void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = 1;
	dongle->last_used_time = get_time_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
