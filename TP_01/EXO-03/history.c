#include<stdlib.h>
#include<stdio.h>

#include"history.h"

/**
 * new_history - alloue, initialise et retourne un historique.
 *
 * @name: nom de l'historique
 */
struct history *new_history(char *name)
{
	struct history *h = malloc(sizeof(struct history));
	h->commit_count = 0;
	h->commit_list = new_commit(0,0,"First");;
	h->name = name;

	return h;
}

/**
 * last_commit - retourne l'adresse du dernier commit de l'historique.
 *
 * @h: pointeur vers l'historique
 */
struct commit *last_commit(struct history *h)
{

	return h->commit_list->prev;
}

/**
 * display_history - affiche tout l'historique, i.e. l'ensemble des commits de
 *                   la liste
 *
 * @h: pointeur vers l'historique a afficher
 */
void display_history(struct history *h)
{
	printf("Historique de : %s\n", h->name);
	struct commit *p, *q;
	p = h->commit_list;
	q = h->commit_list->prev;
		
	while (p != q){
		display_commit(p);
		p = p->next;
	}
	printf("\n");
	return;

}

/**
 * infos - affiche le commit qui a pour numero de version <major>-<minor> ou
 *         'Not here !!!' s'il n'y a pas de commit correspondant.
 *
 * @major: major du commit affiche
 * @minor: minor du commit affiche
 */
void infos(struct history *h, int major, unsigned long minor)
{
	/* TODO : Exercice 3 - Question 2 */
	struct commit *p, *q;
	p = h->commit_list;
	q = h->commit_list->prev;
	int found = 0;
		
	while (!found && p != q){

		if (p->version.major == major &&
				p->version.minor == minor){
			found = 1;
		}
		p = p->next;
	}

	found ? display_commit(p) : printf("Not Here !!!\n");
	return;

}
