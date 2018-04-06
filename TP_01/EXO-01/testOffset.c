#include<stdlib.h>
#include<stdio.h>

#include"version.h"

struct commit {
    unsigned long id;
    struct version version;
    char *comment;
    struct commit *next;
    struct commit *prev;
};

struct commit *commit_off(struct version *v){

    return (struct commit *)
        ((void *)v - (void *)(&((struct commit *) 0)->version));
}


int main (void){

    struct commit *c = malloc(sizeof(struct commit));
    printf("struct commit : %p\n", (void*)&(c));
    printf("id : %p\n", (void*)&(c->id));
    printf("version : %p\n", (void*)&(c->version));
    printf("comment : %p\n", (void*)&(c->comment));
    printf("next : %p\n", (void*)&(c->next));
    printf("previous : %p\n", (void*)&(c->prev));
    printf("base pointer: %p\n", (commit_off(&c->version)));

}


