/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_routine.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 19:26:41 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/01 19:26:44 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	broadcast_all_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_mutex_lock(&data->dongles[i].mutex);
		pthread_cond_broadcast(&data->dongles[i].cond);
		pthread_mutex_unlock(&data->dongles[i].mutex);
		i++;
	}
	pthread_mutex_lock(&data->compile_mutex);
	pthread_cond_broadcast(&data->compile_cond);
	pthread_mutex_unlock(&data->compile_mutex);
}

static int	handle_burnout(t_data *data, int i)
{
	data->simulation_over = 1;
	pthread_mutex_unlock(&data->sim_mutex);
	pthread_mutex_lock(&data->log_mutex);
	pthread_mutex_lock(&data->sim_mutex);
	printf("%ld %d burned out\n", get_time_ms() - data->start_time,
		data->coders[i].id);
	pthread_mutex_unlock(&data->sim_mutex);
	pthread_mutex_unlock(&data->log_mutex);
	broadcast_all_dongles(data);
	return (1);
}

static int	check_coder_state(t_data *data, int *all_done, int i)
{
	if (data->coders[i].compilation_count < data->number_of_compiles_required)
	{
		*all_done = 0;
		if (get_time_ms()
			- data->coders[i].last_compilation > data->time_to_burnout)
			return (handle_burnout(data, i));
	}
	return (0);
}

static int	check_simulation(t_data *data)
{
	int	i;
	int	all_done;

	all_done = 1;
	i = 0;
	while (i < data->number_of_coders)
	{
		if (check_coder_state(data, &all_done, i))
			return (1);
		i++;
	}
	if (all_done)
	{
		data->simulation_over = 1;
		pthread_mutex_unlock(&data->sim_mutex);
		broadcast_all_dongles(data);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (1)
	{
		pthread_mutex_lock(&data->sim_mutex);
		if (data->simulation_over)
			return (pthread_mutex_unlock(&data->sim_mutex), NULL);
		if (check_simulation(data))
			return (NULL);
		pthread_mutex_unlock(&data->sim_mutex);
		usleep(2000);
	}
	return (NULL);
}
