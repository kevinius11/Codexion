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

void *coder_routine(void *arg)
{
	t_coders *coder = (t_coders *)arg;

	int acciones = 0;
	while (acciones < 5)
	{
		printf("Coder %d haciendo accion %d\n", coder->id, acciones);
		usleep(200000);
		acciones++;
	}
	return NULL;
}
