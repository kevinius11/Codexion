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

void	clean_threads(t_dongle *dongles, int count)
{
	int j;

	for (j = 0; j < count ; j++)
	{
		pthread_mutex_destroy(&dongles[j].mutex);
		pthread_cond_destroy(&dongles[j].cond);
	}
}

int init_simulation_dynamic(t_data *data)
{
	int i ;

	data->dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	data->coders = malloc(sizeof(t_coder) * data->number_of_coders);

	if (!data->dongles || !data->coders)
		return (-1);

	i = 0;

	while( i < data->number_of_coders)
	{
		pthread_mutex_init(&data->dongles[i].mutex, NULL);
		if

		data->dongles[i].id = i;

		data->coders[i].id = i + 1;
		data->coders[i].data = data;

		data->coders[i].right = &data->dongles[i];
		data->coders[i].left = &data->dongles[(i + 1) % data->number_of_coders];

		data->coders[i].compilation_count = 0;
		data->coders[i].last_compilation = 0;
		i++;
	}
	return (0);
}
