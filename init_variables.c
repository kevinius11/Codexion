#include "codexion.h"

long get_time_ms(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int init_data(t_data *data)
{
	if (pthread_mutex_init(&data->log_mutex, NULL) != 0)
		return (-1);
	if (pthread_mutex_init(&data->sim_mutex, NULL) != 0)
		return (-1);

	data->simulation_over = 0;
	data->start_time = get_time_ms();

	return (0);
}

void free_resources(t_data *data, int count)
{
	int i;
	
	i = 0;
	while ( i < count)
	{
		pthread_cond_destroy(&data->dongles[i].cond);
		pthread_mutex_destroy(&data->dongles[i].mutex);
		i++;
	}
	if (data->dongles)
	{
		free(data->dongles);
		data->dongles = NULL;
	}
	if (data->coders)
	{
		free(data->coders);
		data->coders = NULL;
	}
}

int init_simulation_dynamic(t_data *data)
{
	int i ;

	data->dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	if (!data->dongles)
		return (-1);

	data->coders = malloc(sizeof(t_coder) * data->number_of_coders);
	if (!data->coders)
	{
		free(data->dongles);
		return (-1);
	}
	i = 0;

	while( i < data->number_of_coders)
	{
		if(pthread_mutex_init(&data->dongles[i].mutex, NULL) != 0)
		{
			free_resources(data, i);
			return (-1);
		}
		if (pthread_cond_init(&data->dongles[i].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&data->dongles[i].mutex);
			free_resources(data, i);
			return (-1);
		}

		data->dongles[i].id = i;

		data->coders[i].id = i + 1;
		data->coders[i].data = data;

		data->coders[i].right = &data->dongles[i];
		data->coders[i].left = &data->dongles[(i - 1 + data->number_of_coders) % data->number_of_coders];

		data->coders[i].compilation_count = 0;
		data->coders[i].last_compilation = data->start_time;
		i++;
	}
	return (0);
}
