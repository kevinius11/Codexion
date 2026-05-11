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

/*
** FIX PROBLEMA 1:
** log_mutex y sim_mutex se adquieren SIEMPRE en el mismo orden:
**   log_mutex → sim_mutex
** Esto elimina la ventana entre "comprobé simulation_over" y "hice printf"
** donde el monitor podía activar simulation_over y el coder imprimía igual.
** Al mantener ambos locks durante el printf, o imprimimos dentro de la
** simulación activa, o no imprimimos en absoluto. Sin ventana posible.
*/
void	print_status(t_data *data, int id, char *status)
{
	pthread_mutex_lock(&data->log_mutex);
	pthread_mutex_lock(&data->sim_mutex);
	if (!data->simulation_over)
		printf("%ld %d %s\n", get_time_ms() - data->start_time, id, status);
	pthread_mutex_unlock(&data->sim_mutex);
	pthread_mutex_unlock(&data->log_mutex);
}

static int	is_sim_over(t_data *data)
{
	int	over;

	pthread_mutex_lock(&data->sim_mutex);
	over = data->simulation_over;
	pthread_mutex_unlock(&data->sim_mutex);
	return (over);
}

static long	make_ticket_timestamp(t_coders *coder)
{
	long	ts;

	if (coder->data->scheduler == FIFO)
		return (get_time_ms());
	pthread_mutex_lock(&coder->data->sim_mutex);
	ts = coder->last_compilation + coder->data->time_to_burnout;
	pthread_mutex_unlock(&coder->data->sim_mutex);
	return (ts);
}

static int	dongle_is_acquirable(t_dongle *dongle, t_coders *coder)
{
	long	elapsed;

	if (!dongle->available)
		return (0);
	if (dongle->last_used_time != 0)
	{
		elapsed = get_time_ms() - dongle->last_used_time;
		if (elapsed < coder->data->dongle_cooldown)
			return (0);
	}
	if (heap_peek(&dongle->queue).coder_id != coder->id)
		return (0);
	return (1);
}

/*
** FIX PROBLEMA 2 y 3:
** Ticket ownership explícito con flag 'inserted'.
** El ticket se inserta UNA SOLA VEZ y se elimina exactamente una vez,
** siempre con heap_remove_by_id(coder->id), nunca con heap_extract_min.
**
** ¿Por qué heap_remove_by_id en vez de heap_extract_min en el caso exitoso?
** heap_extract_min asume que "si soy adquirible, soy el top del heap".
** Aunque dongle_is_acquirable comprueba heap_peek == coder->id antes de
** adquirir, cualquier corrupción futura podría extraer el ticket de otro
** coder. heap_remove_by_id busca por ID explícito: sabemos quiénes somos,
** no necesitamos asumir que seguimos siendo el mínimo en el instante exacto
** de la extracción.
**
** El flag 'inserted' garantiza que no haya doble-remove si el hilo
** es cancelado por simulation_over en ramas distintas del bucle.
*/
static int	wait_for_dongle(t_coders *coder, t_dongle *dongle)
{
	t_waiter		ticket;
	long			remaining;
	struct timespec	ts;
	int			inserted;

	ticket.coder_id = coder->id;
	ticket.timestamp = make_ticket_timestamp(coder);
	inserted = 0;
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(&dongle->queue, ticket);
	inserted = 1;
	while (1)
	{
		if (is_sim_over(coder->data))
		{
			if (inserted)
			{
				heap_remove_by_id(&dongle->queue, coder->id);
				inserted = 0;
			}
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		if (dongle_is_acquirable(dongle, coder))
			break ;
		remaining = 0;
		if (dongle->available && dongle->last_used_time != 0)
			remaining = coder->data->dongle_cooldown
				- (get_time_ms() - dongle->last_used_time);
		if (remaining > 0)
		{
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += remaining / 1000;
			ts.tv_nsec += (remaining % 1000) * 1000000;
			if (ts.tv_nsec >= 1000000000)
			{
				ts.tv_sec++;
				ts.tv_nsec -= 1000000000;
			}
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
		else
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	if (inserted)
	{
		heap_remove_by_id(&dongle->queue, coder->id);
		inserted = 0;
	}
	dongle->available = 0;
	pthread_mutex_unlock(&dongle->mutex);
	print_status(coder->data, coder->id, "has taken a dongle");
	return (1);
}

static int	try_take_dongle(t_coders *coder, t_dongle *dongle)
{
	t_waiter	ticket;
	int		acquired;

	ticket.coder_id = coder->id;
	ticket.timestamp = make_ticket_timestamp(coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(&dongle->queue, ticket);
	acquired = dongle_is_acquirable(dongle, coder);
	heap_remove_by_id(&dongle->queue, coder->id);
	if (acquired)
		dongle->available = 0;
	pthread_mutex_unlock(&dongle->mutex);
	if (acquired)
		print_status(coder->data, coder->id, "has taken a dongle");
	return (acquired);
}

static void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = 1;
	dongle->last_used_time = get_time_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

/*
** FIX PROBLEMA 4:
** Backoff exponencial con cap para prevenir starvation en reintentos.
** backoff empieza en 200+offset_por_id para desincronizar coders vecinos,
** se duplica en cada fallo (presión creciente → menor contención),
** y se limita a 8000µs para no degradar latencia de burnout.
** El offset basado en id garantiza que dos coders vecinos nunca empiezan
** en el mismo punto del ciclo de backoff.
*/
static int	take_both_dongles(t_coders *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long		backoff;


	if (coder->id % 2 == 0)
	{
		first = coder->left;
		second = coder->right;
	}
	else
	{
		first = coder->right;
		second = coder->left;
	}
	backoff = 200 + (coder->id * 137) % 400;
	while (1)
	{
		if (is_sim_over(coder->data))
			return (0);
		if (!wait_for_dongle(coder, first))
			return (0);
		if (try_take_dongle(coder, second))
			return (1);
		release_dongle(first);
		if (is_sim_over(coder->data))
			return (0);
		usleep(backoff);
		if (backoff < 8000)
			backoff *= 2;
	}
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
	return (NULL);
}
