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


typedef enum s_scheduler
{
  FIFO,
  EDF
} t_scheduler;

typedef struct s_data
{
  int number_of_coders;
  long time_to_burnout;
  long time_to_compile;
  long time_to_debug;
  long time_to_refactor;
  int number_of_compiles_required;
  long dongle_cooldown;
  long start_time;
  int simulation_over;
  pthread_mutex_t log_mutex;
  pthread_mutex_t sim_mutex;
  t_dongle *dongles;
  t_coders *coders;
  t_scheduler scheduler;
} t_data;

typedef struct s_dongle
{
  int id;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  long cooldown;
  long last_used_time;
} t_dongle;

typedef struct s_coders
{
  int id;
  long last_compilation;
  int compilation_count;
  t_dongle *left;
  t_dongle *right;
  t_data *data;
} t_coders;

int ft_strlen(char *s);
size_t ft_strspn(const char *s1, const char *chars);

