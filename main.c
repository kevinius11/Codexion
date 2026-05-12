/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:51:59 by kcastro-          #+#    #+#             */
/*   Updated: 2026/05/06 19:23:49 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_threads(t_data *data, pthread_t *monitor)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		if (pthread_create(&data->threads[i], NULL, &coder_routine,
				&data->coders[i]) != 0)
			return (printf("Error creando hilo %d\n", i), 1);
		i++;
	}
	if (pthread_create(monitor, NULL, &monitor_routine, data) != 0)
		return (printf("Error creando monitor\n"), 1);
	return (0);
}

static void	wait_threads(t_data *data, pthread_t monitor)
{
	int	i;

	pthread_join(monitor, NULL);
	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_join(data->threads[i], NULL);
		i++;
	}
}

static int	init_all(int argc, char **argv, t_data *data)
{
	if (parse_args(argc, argv, data) != 0)
		return (printf("Error: invalid parsing...\n"), 1);
	if (init_data(data) != 0)
		return (1);
	if (init_simulation_dynamic(data) != 0)
	{
		printf("Error: Failure in memory or mutex...\n");
		free_resources(data, 0);
		return (1);
	}
	return (0);
}

int	main(int argc, char **argv)
{
	t_data		data;
	pthread_t	monitor;

	if (init_all(argc, argv, &data))
		return (1);
	if (start_threads(&data, &monitor))
		return (free_resources(&data, data.number_of_coders), 1);
	wait_threads(&data, monitor);
	free_resources(&data, data.number_of_coders);
	return (0);
}
