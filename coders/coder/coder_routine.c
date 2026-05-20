/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:24:43 by kcastro-          #+#    #+#             */
/*   Updated: 2026/04/30 18:24:46 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	perform_actions(t_coders *coder, t_data *data)
{
	print_status(data, coder->id, "is compiling");
	usleep(data->time_to_compile * 1000);
	pthread_mutex_lock(&data->sim_mutex);
	coder->compilation_count++;
	pthread_mutex_unlock(&data->sim_mutex);
	release_dongle(coder->right);
	release_dongle(coder->left);
	print_status(data, coder->id, "is debugging");
	usleep(data->time_to_debug * 1000);
	print_status(data, coder->id, "is refactoring");
	usleep(data->time_to_refactor * 1000);
}

static int	handle_single_coder(t_coders *coder)
{
	if (!wait_for_dongle(coder, coder->right, make_ticket_timestamp(coder)))
		return (0);
	while (!coder->data->simulation_over)
		usleep(1000);
	release_dongle(coder->right);
	return (0);
}

static void	assign_dongles(t_coders *coder, t_dongle **f, t_dongle **s)
{
	if (coder->id % 2 == 0)
	{
		*f = coder->left;
		*s = coder->right;
	}
	else
	{
		*f = coder->right;
		*s = coder->left;
	}
}

int	take_both_dongles(t_coders *coder)
{
	t_dongle	*f;
	t_dongle	*s;
	long		b;
	long		priority;

	if (coder->data->number_of_coders == 1)
		return (handle_single_coder(coder));
	assign_dongles(coder, &f, &s);
	priority = make_ticket_timestamp(coder);
	b = 200 + (coder->id * 137) % 400;
	while (!coder->data->simulation_over)
	{
		if (!wait_for_dongle(coder, f, priority))
			return (0);
		if (try_take_dongle(coder, s, priority))
			return (1);
		release_dongle(f);
		usleep(b);
		if (b < 8000)
			b *= 2;
	}
	return (0);
}

void	*coder_routine(void *arg)
{
	t_coders	*coder;
	t_data		*data;

	coder = (t_coders *)arg;
	data = coder->data;
	usleep(coder->id * 1000);
	while (1)
	{
		pthread_mutex_lock(&data->sim_mutex);
		if (data->simulation_over
			|| coder->compilation_count >= data->number_of_compiles_required)
		{
			pthread_mutex_unlock(&data->sim_mutex);
			break ;
		}
		pthread_mutex_unlock(&data->sim_mutex);
		if (!take_both_dongles(coder))
			break ;
		pthread_mutex_lock(&data->sim_mutex);
		coder->last_compilation = get_time_ms();
		pthread_mutex_unlock(&data->sim_mutex);
		perform_actions(coder, data);
	}
	return (NULL);
}
