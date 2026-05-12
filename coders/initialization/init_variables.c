/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_variables.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 20:28:20 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/12 20:28:25 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int	init_data(t_data *data)
{
	if (pthread_mutex_init(&data->log_mutex, NULL) != 0)
		return (-1);
	if (pthread_mutex_init(&data->sim_mutex, NULL) != 0)
		return (-1);
	if (pthread_mutex_init(&data->compile_mutex, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&data->compile_cond, NULL) != 0)
		return (-1);
	data->simulation_over = 0;
	data->compiling_count = 0;
	data->start_time = get_time_ms();
	return (0);
}

static void	free_dongles(t_data *data, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&data->dongles[i].cond);
		pthread_mutex_destroy(&data->dongles[i].mutex);
		free(data->dongles[i].queue.waiters);
		i++;
	}
}

void	free_resources(t_data *data, int count)
{
	if (!data)
		return ;
	free_dongles(data, count);
	pthread_mutex_destroy(&data->log_mutex);
	pthread_mutex_destroy(&data->sim_mutex);
	pthread_mutex_destroy(&data->compile_mutex);
	pthread_cond_destroy(&data->compile_cond);
	free(data->threads);
	free(data->dongles);
	free(data->coders);
}
