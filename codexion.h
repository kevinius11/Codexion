/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 18:07:04 by kcastro-          #+#    #+#             */
/*   Updated: 2026/04/23 18:07:28 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stddef.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct s_data t_data;
typedef struct s_dongle t_dongle;
typedef struct s_coders t_coders;

typedef struct s_waiter
{
	int coder_id;
	long timestamp;
}	t_waiter;

typedef struct s_heap
{
	t_waiter	*waiters;
	int	size;
	int	capacity;
}	t_heap;

typedef enum s_scheduler
{
  FIFO,
  EDF
} t_scheduler;

typedef struct s_data
{
  int	number_of_coders;
  long	time_to_burnout;
  long	time_to_compile;
  long	time_to_debug;
  long	time_to_refactor;
  int	number_of_compiles_required;
  long	dongle_cooldown;
  long	start_time;
  int	simulation_over;
  int	compiling_count;
  pthread_mutex_t	compile_mutex;
  pthread_cond_t	compile_cond;
  pthread_mutex_t	log_mutex;
  pthread_mutex_t	sim_mutex;
  pthread_t	*threads;
  t_dongle	*dongles;
  t_coders	*coders;
  t_scheduler	scheduler;
}	t_data;

typedef struct s_dongle
{
  int	id;
  long	cooldown;
  long	last_used_time;
  int	available;
  pthread_mutex_t	mutex;
  pthread_cond_t	cond;
  t_heap	queue;
}	t_dongle;

typedef struct s_coders
{
  int	id; 
  long	last_compilation;
  int	compilation_count;
  t_dongle	*left;
  t_dongle	*right;
  t_data	*data;
}	t_coders;

t_waiter	heap_peek(t_heap *heap);
t_waiter	heap_extract_min(t_heap *heap);
void	heap_insert(t_heap *heap, t_waiter new);
int	ft_strlen(char *s);
size_t	ft_strspn(const char *s1, const char *chars);
void	*coder_routine(void *arg);
long	get_time_ms(void);
int	init_data(t_data *data);
void	free_resources(t_data *data, int count);
int	init_simulation_dynamic(t_data *data);
int	is_numeric(char *str);
long long	ft_atoi_long(const char *str);
void	print_status(t_data *data, int id, char *status);
void	*monitor_routine(void *arg);
int	parse_args(int argc, char **argv, t_data *data);
int	main(int argc, char **argv);
