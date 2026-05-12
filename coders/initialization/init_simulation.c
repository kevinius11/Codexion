/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_simulation.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 20:30:00 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/12 20:30:02 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	allocate_memory(t_data *data)
{
	data->threads = malloc(sizeof(pthread_t) * data->number_of_coders);
	if (!data->threads)
		return (-1);
	data->dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	if (!data->dongles)
		return (free(data->threads), -1);
	data->coders = malloc(sizeof(t_coders) * data->number_of_coders);
	if (!data->coders)
		return (free_resources(data, 0), -1);
	return (0);
}

static int	init_dongle(t_data *data, int i)
{
	if (pthread_mutex_init(&data->dongles[i].mutex, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&data->dongles[i].cond, NULL) != 0)
		return (pthread_mutex_destroy(&data->dongles[i].mutex), -1);
	data->dongles[i].queue.waiters = malloc(sizeof(t_waiter)
			* data->number_of_coders);
	if (!data->dongles[i].queue.waiters)
		return (-1);
	data->dongles[i].queue.size = 0;
	data->dongles[i].queue.capacity = data->number_of_coders;
	data->dongles[i].id = i;
	data->dongles[i].available = 1;
	data->dongles[i].last_used_time = 0;
	return (0);
}

static void	init_coder(t_data *data, int i)
{
	data->coders[i].id = i + 1;
	data->coders[i].data = data;
	data->coders[i].right = &data->dongles[i];
	data->coders[i].left = &data->dongles[(i - 1 + data->number_of_coders)
		% data->number_of_coders];
	if (data->number_of_coders == 1)
		data->coders[i].left = NULL;
	data->coders[i].compilation_count = 0;
	data->coders[i].last_compilation = data->start_time;
}

int	init_simulation_dynamic(t_data *data)
{
	int	i;

	if (allocate_memory(data) != 0)
		return (-1);
	i = 0;
	while (i < data->number_of_coders)
	{
		if (init_dongle(data, i) != 0)
			return (free_resources(data, i), -1);
		init_coder(data, i);
		i++;
	}
	return (0);
}
