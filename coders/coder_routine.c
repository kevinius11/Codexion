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
	pthread_mutex_lock(&data->sim_mutex);
	if (data->simulation_over)
	{
		pthread_mutex_unlock(&data->sim_mutex);
		return ;
	}
	pthread_mutex_unlock(&data->sim_mutex);
	pthread_mutex_lock(&data->log_mutex);
	printf("%ld %d %s\n", get_time_ms() - data->start_time, id, status);
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

/*
** Comprueba si el dongle es adquirible AHORA por este coder.
** Debe llamarse con dongle->mutex ya tomado.
**
** Condiciones necesarias:
**   1. dongle->available == 1           (nadie lo tiene)
**   2. cooldown expirado                (tiempo desde liberacion >= cooldown)
**   3. este coder es el top del heap    (politica de prioridad FIFO/EDF)
**
** Separar esta funcion de la adquisicion elimina el TOCTOU: la
** comprobacion y la toma ocurren bajo el mismo lock sin soltarlo entre medias.
*/
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
** try_take_dongle — adquisicion NO bloqueante.
**
** Toma el mutex, inserta ticket en la cola de prioridad, comprueba
** inmediatamente si puede adquirir. Si puede: lo marca como no disponible,
** extrae el ticket del heap y devuelve 1. Si no puede: elimina el ticket
** del heap (sin dejar zombie) y devuelve 0.
**
** No hace ningun wait. Retorna siempre de inmediato.
** Esto garantiza que nunca hay hold-and-wait: si falla, el coder
** no retiene ningun recurso mientras espera.
*/
static int	try_take_dongle(t_coders *coder, t_dongle *dongle)
{
	t_waiter	ticket;
	int		acquired;

	ticket.coder_id = coder->id;
	ticket.timestamp = make_ticket_timestamp(coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(&dongle->queue, ticket);
	acquired = dongle_is_acquirable(dongle, coder);
	if (acquired)
	{
		heap_extract_min(&dongle->queue);
		dongle->available = 0;
	}
	else
		heap_remove_by_id(&dongle->queue, coder->id);
	pthread_mutex_unlock(&dongle->mutex);
	if (acquired)
		print_status(coder->data, coder->id, "has taken a dongle");
	return (acquired);
}

/*
** wait_for_dongle — espera bloqueante sobre UN dongle hasta que
** sea adquirible o la simulacion termine.
**
** Solo se llama cuando el coder no retiene ningun otro recurso,
** por lo que no hay hold-and-wait.
** Usa timedwait para auto-despertarse cuando el cooldown expira,
** sin depender exclusivamente de broadcasts externos.
*/
static int	wait_for_dongle(t_coders *coder, t_dongle *dongle)
{
	t_waiter		ticket;
	long			remaining;
	struct timespec	ts;

	ticket.coder_id = coder->id;
	ticket.timestamp = make_ticket_timestamp(coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(&dongle->queue, ticket);
	while (1)
	{
		if (is_sim_over(coder->data))
		{
			heap_remove_by_id(&dongle->queue, coder->id);
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
	heap_extract_min(&dongle->queue);
	dongle->available = 0;
	pthread_mutex_unlock(&dongle->mutex);
	print_status(coder->data, coder->id, "has taken a dongle");
	return (1);
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
** take_both_dongles — adquisicion dual sin hold-and-wait ni TOCTOU.
**
** Algoritmo:
**   1. Espera bloqueante sobre el PRIMER dongle (no retiene nada mientras
**      espera, asi que no hay hold-and-wait en esta fase).
**   2. Intenta trylock inmediato sobre el SEGUNDO.
**      - Si tiene exito: ambos adquiridos, sin race posible porque
**        try_take_dongle comprueba Y adquiere bajo el mismo mutex.
**      - Si falla: libera el primero inmediatamente → vuelve al estado
**        "sin recursos retenidos" → backoff aleatorio → reintento.
**
** El backoff aleatorio (no fijo) rompe la sincronizacion colectiva
** que causaria livelock si todos los coders reintentaran en el mismo tick.
**
** La alternancia par/impar en el orden first/second reduce la probabilidad
** de contention en el caso comun, pero no es la barrera contra deadlock —
** esa es la garantia de "nunca retengo uno esperando otro".
*/
static int	take_both_dongles(t_coders *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long		backoff;

	if (coder->data->number_of_coders == 1)
		return (wait_for_dongle(coder, coder->right));
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
	backoff = 200 + (coder->id * 137) % 600;
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
		backoff = 200 + (backoff * 3 + coder->id * 71) % 600;
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
		if (data->number_of_coders > 1)
			release_dongle(coder->left);
		print_status(data, coder->id, "is debugging");
		usleep(data->time_to_debug * 1000);
		print_status(data, coder->id, "is refactoring");
		usleep(data->time_to_refactor * 1000);
	}
	return (NULL);
}
