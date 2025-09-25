#

zt_list:	zt_list.c zerotar.h
		gcc -Wall -o zt_list zt_list.c

zt_untar:	zt_untar.c zerotar.h
		gcc -Wall -o zt_untar zt_untar.c
