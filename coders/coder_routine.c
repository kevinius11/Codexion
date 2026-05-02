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

void	print_status(t_data *data, int id, char *status)
{
	pthread_mutex_lock(&data->log_mutex);
	if (!data->simulation_over)
	{
		printf("%ld %d %s\n", get_time_ms() - data->start_time, id, status);
	}
	pthread_mutex_unlock(&data->log_mutex);
}

void	take_dongle(t_coders *coder, t_dongle *dongle)
{
	    struct timeval  tv;
	    struct timespec ts;
	    long            remaining;

	    pthread_mutex_lock(&dongle->mutex);
	    while (!dongle->available
        	|| (get_time_ms() - dongle->last_used_time) < coder->data->dongle_cooldown)
	    {
            remaining = coder->data->dongle_cooldown
               - (get_time_ms() - dongle->last_used_time);
	    if (remaining <= 0)
		    remaining = 1;
	    gettimeofday(&tv, NULL);
	    ts.tv_sec = tv.tv_sec + (remaining / 1000);
	    ts.tv_nsec = (tv.tv_usec * 1000) + ((remaining % 1000) * 1000000);
	    if (ts.tv_nsec >= 1000000000)
	    {
		    ts.tv_sec += 1;
		    ts.tv_nsec -= 1000000000;
	    }
	    pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
	    }
	    dongle->available = 0;
	    pthread_mutex_unlock(&dongle->mutex);
	    print_status(coder->data, coder->id, "has taken a dongle");
}

void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = 1;
	dongle->last_used_time = get_time_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void *coder_routine(void *arg)
{
	t_coders *coder = (t_coders *)arg;
	t_data *data = coder->data;
	t_dongle *first;
	t_dongle *second;

	usleep(coder->id * 1000);
	while (1)
	{	
		pthread_mutex_lock(&data->sim_mutex);
		if (data->simulation_over || coder->compilation_count >= data->number_of_compiles_required)
		{
			pthread_mutex_unlock(&data->sim_mutex);
			break;
		}
		pthread_mutex_unlock(&data->sim_mutex);

		if (coder->id == 1)
		{
			first = coder->right;
			second = coder->left;
		}
		else
		{
			first = coder->left;
			second = coder->right;
		}

		take_dongle(coder, first);
		take_dongle(coder, second);
		coder->last_compilation = get_time_ms();
		print_status(data, coder->id, "is compiling");

		usleep(data->time_to_compile * 1000);

		pthread_mutex_lock(&data->sim_mutex);
		coder->compilation_count++;
		pthread_mutex_unlock(&data->sim_mutex);

		release_dongle(first);
		release_dongle(second);
		
		print_status(data, coder->id, "is debugging");
        	usleep(data->time_to_debug * 1000);

		print_status(data, coder->id, "is refactoring");
        	usleep(data->time_to_refactor * 1000);
	}
	return (NULL);
}
