/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kcastro- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 18:05:09 by kcastro-          #+#    #+#             */
/*   Updated: 2026/04/23 18:05:14 by kcastro-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	ft_strlen(char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

long long	ft_atoi_long(const char *str)
{
	long long	result;
	int			i;
	int			sign;
	int			digit;

	result = 0;
	i = 0;
	sign = 1;
	while (str[i] == ' ' || (str[i] >= '\t' && str[i] <= '\r'))
		i++;
	if (str[i] == '+' || str[i] == '-')
		sign = (str[i++] == '-') * -2 + 1;
	while (str[i] >= '0' && str[i] <= '9')
	{
		digit = str[i] - '0';
		if (result > LONG_MAX / 10)
			return ((sign == 1) * LONG_MAX + (sign == -1) * LONG_MIN);
		result = result * 10 + digit;
		i++;
	}
	return (result * sign);
}

size_t	ft_strspn(const char *s1, const char *chars)
{
	size_t	count;
	int		j;

	count = 0;
	while (s1[count])
	{
		j = 0;
		while (chars[j] != '\0')
		{
			if (s1[count] == chars[j])
				break ;
			j++;
		}
		if (chars[j] == '\0')
			break ;
		count++;
	}
	return (count);
}
