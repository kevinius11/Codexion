/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 18:51:59 by kcastro-          #+#    #+#             */
/*   Updated: 2026/04/30 18:52:01 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int main(int argc, char **argv)
{
	t_data data;
	int i;

    	printf("DEBUG: Iniciando programa con %d argumentos\n", argc);
	fflush(stdout); 
   	if (parse_args(argc, argv, &data) != 0)
    	{
        	printf("ERROR: El parseo falló. Revisa los valores introducidos.\n");
        	return (1);
    	}
    
    	printf("DEBUG: Parseo exitoso. Coders: %d\n", data.number_of_coders);

	if (init_data(&data) != 0)
		return (1);

    	if (init_simulation_dynamic(&data) != 0)
    	{
        	printf("ERROR: Fallo en la inicialización de memoria o mutex.\n");
		free_resources(&data, 0);
        	return (1);
    	}

	i = 0;
	while ( i < data.number_of_coders)
	{
		if (pthread_create(&data.threads[i], NULL, &coder_routine, &data.coders[i]) != 0)
		{
			printf("Error creando el hilo %d\n", i);
			break;
		}
		i++;
	}

	i = 0;

	while (i < data.number_of_coders)
	{
		pthread_join(data.threads[i], NULL);
		i++;
	}

	free_resources(&data, data.number_of_coders);
	return (0);
}
