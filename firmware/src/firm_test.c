/*
  Author: Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation: 22/02/2002
*/

xdata char id[] = "@(#) $Id$";

xdata at 0x1000 unsigned char my_var_0;
xdata at 0x1001 unsigned char my_var_1;
xdata at 0x1002 unsigned char my_var_2;
xdata at 0x1003 unsigned char my_var_3;
xdata at 0x1010 char tab[];

idata at 0xff unsigned char my_idata;

void main()
{
	int i;

	my_var_0 = 0xfa;
	my_var_1 = 0xda;
	my_var_2 = 0xfa;
	my_var_3 = 0xda;

	my_idata = 0;
	if (my_idata == 0)
	{
		my_idata = 0xff;
		if (my_idata == 0xff)
			my_var_2 = 0xde;
	}

	for (i=0;i<1000;i++)
		tab[i] = 0xda;

	/* infinite loop */
	for (;;)
		;
}
