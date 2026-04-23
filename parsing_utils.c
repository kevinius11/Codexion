#include "codexion.h"

int ft_strlen(char *s)
{
	int i;

	i = 0;

	while(s[i])
		i++;
	return i;
}

size_t ft_strspn(const char *s1, const char *chars)
{
	size_t count;
	int j;

	count = 0;

	while(s1[count])
	{
		j = 0;
		while(chars[j] != '\0')
		{
			if (s1[count] == chars[j])
				break;
		}
		
		if (chars[j] == '\0')
			break;

		count++;
	}
	return count;
}
