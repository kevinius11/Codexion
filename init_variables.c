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


