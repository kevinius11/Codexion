#include "codexion.h"

int is_numeric(char *str)
{
	int i;

	i = 0;
  if(!str || *str == '\0')
    return (-1);

  if (ft_strspn(str, "0123456789") == ft_strlen(str))
    return (1);
  return (0);
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

    i = 1;
		while(i <= 7)
		{
			if (!is_numeric(argv[i]))
				return (-1);
      i++;
		}

		long coders = ft_atoi_long(argv[1]);
    long time_to_burnout = ft_atoi_long(argv[2]);
    long time_to_compile = ft_atoi_long(argv[3]);
    long time_to_debug = ft_atoi_long(argv[4]);
    long time_to_refactor = ft_atoi_long(argv[5]);
    long number_of_compiles_required = ft_atoi_long(argv[6]);
    long dongle_cooldown = ft_atoi_long(argv[7]);

		if (coders <= 0)
			return (-1);
		data->number_of_coders = coders;

    if (time_to_burnout <= 0)
      return (-1);
    data->time_to_burnout = time_to_burnout;
    
    if (time_to_compile <= 0)
      return (-1);
    data->time_to_compile = time_to_compile;

    if (time_to_debug <= 0)
      return (-1);
    data->time_to_debug = time_to_debug;

    if (time_to_refactor <= 0)
      return (-1);
    data->time_to_refactor = time_to_refactor;

    if (number_of_compiles_required < 1)
      return (-1);
    data->number_of_compiles_required = number_of_compiles_required;

    if (dongle_cooldown < 0)
      return (-1);
    data->dongle_cooldown = dongle_cooldown;

    if(strcmp(argv[8], "fifo") == 0)
      data->scheduler = FIFO;
    else if(strcmp(argv[8], "edf") == 0)
      data->scheduler = EDF;
    else
     return (-1);

    return (0);
	}
  return (-1);
}
