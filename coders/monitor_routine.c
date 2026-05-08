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
}

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i;
	int		all_done;

	data = (t_data *)arg;
	while (1)
	{
		pthread_mutex_lock(&data->sim_mutex);
		if (data->simulation_over)
		{
			pthread_mutex_unlock(&data->sim_mutex);
			return (NULL);
		}
		all_done = 1;
		i = 0;
		while (i < data->number_of_coders)
		{
			if (data->coders[i].compilation_count
				< data->number_of_compiles_required)
			{
				all_done = 0;
				if (get_time_ms() - data->coders[i].last_compilation
					> data->time_to_burnout)
				{
					data->simulation_over = 1;
					pthread_mutex_unlock(&data->sim_mutex);
					pthread_mutex_lock(&data->log_mutex);
					printf("%ld %d burned out\n",
						get_time_ms() - data->start_time,
						data->coders[i].id);
					pthread_mutex_unlock(&data->log_mutex);
					broadcast_all_dongles(data);
					return (NULL);
				}
			}
			i++;
		}
		if (all_done)
		{
			data->simulation_over = 1;
			pthread_mutex_unlock(&data->sim_mutex);
			broadcast_all_dongles(data);
			return (NULL);
		}
		pthread_mutex_unlock(&data->sim_mutex);
		usleep(1000);
	}
	return (NULL);
}
