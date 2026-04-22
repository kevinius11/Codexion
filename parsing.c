#include "codexion.h"


int is_numeric(char *str)
{
	int i;

	i = 0;
	
	while(str[i])
	{
		if (!(str[i] >= '0' && str[i] <= '9'))
			return (-1);
		i++;
	}
	return (1);
}

long long ft_atoi_long(const char *str)
{
	long long result;
	int i;
	int sign;
	int digit;

	result = 0;
	i = 0;
	sign = 1;

	while(str[i] == ' ' || (str[i] >= '\t' && str[i] <= '\r'))
		i++;
	
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	
	while(str[i] >= '0' && str[i] <= '9')
	{
		digit = str[i] - '0';
		if (sign == 1)
		{
			if (result > LONG_MAX / 10 || (result == LONG_MAX / 10 && digit > LONG_MAX % 10))
					return (LONG_MAX);
		}
		else
		{
			if (result > -(LONG_MIN / 10) || (result == -(LONG_MIN / 10) && digit > -(LONG_MIN % 10)))
				return (LONG_MIN);
		}
		result = result * 10 + digit;
		i++;
	}
	return (result * sign);
}

int parse_args(int argc, char **argv, t_data *data)
{

	if (argc == 9)
	{
		int i;

		for(i = 1; i <= 7; i++)
		{
			if (!is_numeric(argv[i]))
				return (-1);
		}

		long coders = ft_atoi_long(argv[1]);

		if (coders <= 0)
			return (-1);

		data->number_of_coders = coders;

	}
}
