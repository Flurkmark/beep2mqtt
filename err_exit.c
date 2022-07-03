/*
 *
 * (c) Peter Hällman 2005
 * This code is licensed under MIT license (see LICENSE.txt for details)
 */

#include "err_exit.h"
char err_exit_buf[ERR_EXIT_BUF_SIZ];
char err_exit_int_buf[ERR_EXIT_SIZ_OF_INTSTRING];
int err_exit_buf_left = ERR_EXIT_BUF_SIZ - 1;

void err_exit(int really_exit, int level, char *fmt, ...)
{
	va_list ap;
	int d;
	char c, *s;
	char *p;
	p = err_exit_buf;
	va_start(ap, fmt);

	/* parse format and add all to err_exit_buf as string */
	while (*fmt && err_exit_buf_left)
	{
		/* do we have a %<something> ? */
		if (*fmt == '%')
		{
			fmt++;
			/* yes, find out what comes after
			 * convert and add as string to buf
			 */
			switch (*fmt)
			{
			case 's': /* string */
				s = va_arg(ap, char *);
				/* concaternate strings into buf */
				strncat(err_exit_buf, s, err_exit_buf_left);
				err_exit_buf_left -= strlen(err_exit_buf);
				/* reposition pointer to point at end of string */
				while (*p != '\0')
				{
					p++;
				}

				break;
			case 'd': /* int */
				d = va_arg(ap, int);
				sprintf(err_exit_int_buf, "%d", d);
				/* concaternate strings into buf */
				strncat(err_exit_buf, err_exit_int_buf, err_exit_buf_left);
				err_exit_buf_left -= strlen(err_exit_buf);
				/* reposition pointer to point at end of string */
				while (*p != '\0')
				{
					p++;
				}
				break;
			case 'c': /* char */
				/* need a cast here since va_arg only
				   takes fully promoted types */
				c = (char)va_arg(ap, int);
				sprintf(err_exit_int_buf, "%c", c);
				/* concaternate strings into buf */
				*p = err_exit_int_buf[0];
				p++;
				*p = '\0';
				err_exit_buf_left--;

				break;
			case 'x':
				d = va_arg(ap, int);
				d = va_arg(ap, int);
				sprintf(err_exit_int_buf, "%d", d);
				/* concaternate strings into buf */
				strncat(err_exit_buf, err_exit_int_buf, err_exit_buf_left);
				err_exit_buf_left -= strlen(err_exit_buf);
				/* reposition pointer to point at end of string */
				while (*p != '\0')
				{
					p++;
				}
				break;
			}
			fmt++;
		}
		else
		{

			*p = *fmt;
			p++;
			fmt++;
			err_exit_buf_left--;
		}
	}
	/* print and log error */

	if (errno)
	{
		fprintf(stderr, "%s :", err_exit_buf);
		perror(NULL);
		syslog(level, "%s : %m", err_exit_buf);
	}
	else
	{
		fprintf(stderr, "%s\n", err_exit_buf);
		syslog(level, "%s\n", err_exit_buf);
	}

	va_end(ap);
	if (really_exit)
		exit(EXIT_FAILURE);
	memset(err_exit_buf, 0, ERR_EXIT_BUF_SIZ);
}
