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

void	*monitor_routine(void *arg)
{
	t_data *data = (t_data *)arg;
	int i;

	while (1)
	{
		i = 0;

		while (i < data->number_of_coders)
		{
			pthread_mutex_lock(&data->sim_mutex);

			if(get_time_ms() - data->coders[i].last_compilation > data->time_to_burnout)
			{
				data->simulation_over = 1;
				pthread_mutex_unlock(&data->sim_mutex);
				pthread_mutex_lock(&data->log_mutex);
				printf("%ld %d burned out\n", 
    				get_time_ms() - data->start_time, 
    				data->coders[i].id);
				pthread_mutex_unlock(&data->log_mutex);
				return (NULL);
			}

			pthread_mutex_unlock(&data->sim_mutex);
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
