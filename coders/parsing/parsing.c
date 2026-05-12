/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 18:04:04 by kcastro-          #+#    #+#             */
/*   Updated: 2026/04/23 18:04:17 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_numeric(char *str)
{
	if (!str || *str == '\0')
		return (-1);
	if (ft_strspn(str, "0123456789") == (size_t)ft_strlen(str))
		return (1);
	return (0);
}

static int	validate_numeric_args(char **argv)
{
	int	i;

	i = 1;
	while (i <= 7)
	{
		if (!is_numeric(argv[i]))
			return (-1);
		i++;
	}
	return (0);
}

static int	validate_scheduler(char *str, t_data *data)
{
	if (strcmp(str, "fifo") == 0)
		data->scheduler = FIFO;
	else if (strcmp(str, "edf") == 0)
		data->scheduler = EDF;
	else
		return (-1);
	return (0);
}

static int	assign_values(char **argv, t_data *data)
{
	data->number_of_coders = ft_atoi_long(argv[1]);
	data->time_to_burnout = ft_atoi_long(argv[2]);
	data->time_to_compile = ft_atoi_long(argv[3]);
	data->time_to_debug = ft_atoi_long(argv[4]);
	data->time_to_refactor = ft_atoi_long(argv[5]);
	data->number_of_compiles_required = ft_atoi_long(argv[6]);
	data->dongle_cooldown = ft_atoi_long(argv[7]);
	if (data->number_of_coders <= 0 || data->time_to_burnout <= 0
		|| data->time_to_compile <= 0 || data->time_to_debug <= 0
		|| data->time_to_refactor <= 0 || data->number_of_compiles_required < 1
		|| data->dongle_cooldown < 0)
		return (-1);
	return (0);
}

int	parse_args(int argc, char **argv, t_data *data)
{
	if (argc != 9)
		return (-1);
	if (validate_numeric_args(argv) != 0)
		return (-1);
	if (assign_values(argv, data) != 0)
		return (-1);
	if (validate_scheduler(argv[8], data) != 0)
		return (-1);
	return (0);
}
