## Exercice 1 : Environnement de développement de l'UE

### Question 1
lorsque la commande :
> qemu-system-x86_64 -hda pnl-tp.img
a été lancée pour la première fois, grub n'a pas rèussi à lancer la distribution Linux fournie,  pour cause de corruption de l'image disque
* Au deuxième essai, après chargement de systemd, la console virtuelle nous a accueilli avec un prompt root.

## Exercice 2 : Configuration et compilation du noyau Linux

### Question 1
On peut trouver quelques optons de déboguages utiles dans les différentes sections de configuration mais le pls gros se trouve dans les sections :
> \- Kernel hacking
> \- File systems

### Question 2
La commande _nbproc_ permet de savoir le nombre de coeurs d'une machine sous environnement Linux.
On peut atomatiser la compilation au maximum des capacités de la macine avec la commande :
> make -j$(nbproc)

### Question 4
Dans notre cas, e fait que la version de kernel utilisée par défaut soit la 4.13 et que la version compilée soit la 4.89 nous a indiquée que nous n'utilisions pas le bon kernel. Mais si les deux versions de release sont similaires, il n'est pas possible de les différencier à priori. 

### Question 5
Dans la section _General Setup_, l'option _Local version_ permet de concaténer après le numero de version une chaîne de caractères du choix du programmeur.

### Question 6
Lors de l'exécution de la commande _lsmod_, on se rend compte qu'aucun module n'est chargé. Cela est dû au fait que le kernel aie été compilé statiquement, sans modules.

## Exercice 3 : Le processus _init_

### Question 1
```c
#include <stdio.h>
#include <stdlib.h>
#include <unsitd.h>

int main(void)
{
	printf("Hello World!\n");
	sleep(5);
	return EXIT_SUCCESS;	
}
```

### Question 3
Le système entre en KERNEL PANIC parce que le 1e processus (init) n'est pas censé être terminé sans terminaison du système.

### Question 4
ps demande l'utilisation de fichiers du répertoire */proc*, qui en temps normal est monté par _init_.

### Question 6
> exec init
permet de substituer le code de sh avec le code d'init, qui fonctionnera correctement. On ne peux pas lancer directement init parce que ce dernier a comme pré-requis d'être de PID 1.

## Question 4 : Comprendre le fonctionnement du initramfs

### Question 1
L'option _-static_ permet d'éviter de compiler un exécutable ne ne reposant pas sur les librairies dynamiques. Cette option est nécessaire car au boot, pas toutes les partitions ne sont encore montées et l'executable init doit donc être monolithique.

## Exercice 5

### Question 1
cron_func est un programme qui va exécuter périodiquement une fonction func(), ici toutes les 3 secondes. Le mécanisme est le suivant:
* un sig_handler est spécifié de telle sorte qu'à chaque réception du signal SIGALRM, l fonction func soit exécutée et une nouvelle émission de signal SIGALRM soit planifiée pour 3s plus tard. Cela permet le mécanisme de execution de tâches prériodiques.

### Question 2
Après être allé consulter TLDP, un moyen de compiler une librairie partagée est 
```shell
gcc -shared -Wl,-soname,your_soname \
	-o library_name file_list library_list
```
WARN : ne pas oublier de rentrer !
> export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

```make
.PHONY : all clean check
.ONESHELL:

CHECK_PATCH=../../../../biblio/eudyptula/torvalds/scripts/checkpatch.pl

all : cron_func

%.o : %.c
	gcc -Wall -c $<

cron_func : cron_func.o libfunc.so
	gcc $(PWD)/libfunc.so -Wall $^ -o $@
libfunc.so : nothing.o
	gcc -Wall -shared -Wl,-soname,$@ \
		-o $@ $<
libread.so : tchao.o
	gcc -Wall -shared -Wl,-soname,$@ \
		-o $@ $<
check :
	for f in *.c *.h ; do
		$(CHECK_PATCH) -f $$f
	done

clean :
	rm -rf *.o cron_func libfunc.so
```

### Question 3
Pour mener à bien une telle attaque, il semble préferable de 
