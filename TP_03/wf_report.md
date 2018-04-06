
#   Premier pas dans le Noyau 

### Fabre William PNL

#   Exercice 1 Environnement de developpement de l'UE

### Question 1)
il y a des probles de dependances sur /dev/sdb, /root et le filesystem.
Du coup il faut creer un deuxieme disque pour que sdb fonctionne.

### Question 2)
Nous allons creer un deuxieme disque dans notre HOME.

# Question 3)
OK

#   Exercice 2 Configuration et compilation du noyau

### Question 1)
Copier depuis www.kernel.org
On utilise make nconfig pour la configuration

### Question 2)
make -j pour le nomnbre de jobs et nproc pour trouver le nombre de coeurs

### Question 3)
file arch/x86/boot/bzimage renvoie : 

arch/x86/boot/bzImage: Linux kernel x86 boot executable bzImage,
version 4.14.20 (tchi@archlinux) #1 SMP Thu Feb 22 15:10:08 CET 2018,
RO-rootFS, swap_dev 0x9, Normal VGA

### Question 4)
Il faut remplacer KERNEL pour mettre notre kernel que l'on vient de
compiler.

### Question 5)
make menuconfig -> default host name a changer avec -pnl
et ensuite on change version pour que uname -r nous affiche 
4.XX.XX-pnl
loadkeys us pour avoir le clavier en anglais.
et uname -r renvoie : /** TODO **/

### Question 6)
lsmod et dmesg pour les messages de ces modules

Il faut utiliser menuconfig pour avoir des precisions pre make sur les options
et de la documentation.

Il n'y a aucun module de charger car les modules sont des drivers que l'on
peut charger et decharger dynamiquement de la memoire et aucun module n'a
ete precise lors du make. Donc aucun module ne sera present.

#   Exercice 3 Le processus init

### Question 1)
OK

### Question 2)
CMDLINE='root=/dev/sda1 rw vga=792 console=ttyS0 kgdboc=ttyS1 init=hello'
KERNEL PANIC Attempted tu kill init exit code 0x0000 0000

### Question 3)
Le systeme fini par crash car init est le processus qui lance tous les autres
processus donc apres avoir lance hello on ne va rien faire d'autres et init
ne va jamais etre appele

### Question 4)
CMDLINE='root=/dev/sda1 rw vga=792 console=ttyS0 kgdboc=ttyS1 init=/bin/bash'
proc est un procfs qui n'est pas monte et il n'est pas obligatoire dans 
notre systeme mais il est necessaire a la fonction ps par exemple qui va elle
aller lire dans /proc pour donner la liste des processus.
il contient les informations au runtime du systeme  (memoire, hardware, conf..)
c'est un centre de controle et d'information sur le kernel, lsmod ne fonctionne
pas non plus.

lspci est en fait cat /proc/pci. si on change les fichiers dans /proc on peut
meme changer les parametres du kernel (sysctl) pendant que le systeme tourne.

tous les fichiers ont une taille de 0 sauf kcore, mtrr et self.

### Question 5)
mount -t proc proc /proc
on rattache le FS proc, de nom proc sur /proc pour monter proc dans notre
arbre (file system) et cela nous permet de faire fonctionner ps etc.
j'observe qu'il y a une seule thread qui tourne.

### Question 6)
exec pour changer de programme et garder le meme pid car on a pris le pid 0
avec notre bash et il nous suffira de faire exec de init pour finaliser le
lancement.

#   Exercice 4 Comprendre le fonctionnement du initramfs 

### Question 1)
il faut utiliser celui de la machine hote pas de la VM.
zcat /boot/initramfs-linux.img | cpio -i -d -H newc --no-absolute-filenames
On y trouve un fichier init.

### Question 2)

Empeche le link avec avec des librairies partagees si on est sur un systeme
qui supporte le link dynamique. Le code doit etre compile avec toutes ses 
dependances pour etre autonome.


### Question 3)

https://blog.feabhas.com/2014/04/static-and-dynamic-libraries-on-linux/

gcc -Wall -fPIC $^ -o $@ -L /home/tchi/Lecture/TME_PNL/TP_03/EXO-05 cron_func.so -Wl,-rpath /home/tchi/Lecture/TME_PNL/TP_03/EXO-05

   fPIC :
   Wl,rpath=/XXX/XXX :
   -L :
   -l :

### Question 4)


#   Exercice 5

### Question 1)
OK

### Question 2)
OK

### Question 3)
https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/

### Question 4)
https://rafalcieslak.wordpress.com/2013/04/02/dynamic-linker-tricks-using-ld_preload-to-cheat-inject-features-and-investigate-programs/

### Question 5)
https://security.stackexchange.com/questions/63599/is-there-any-way-to-block-ld-preload-and-ld-library-path-on-linux

### Question 6)
'''c
#define _GNU_SOURCE
#include <dlfcn.h>

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<signal.h>

#include"func.h"

void handle_sigalrm(int signal)
{
	if (signal == SIGALRM) {
		func();
		alarm(3);
	}
}

int main(int argc, char const *argv[])
{
	struct sigaction sa;
	char c = 0;


	/** MY PART **/
	void* dlhandle_glibc;
	void* dlhandle_pnl;
	void* sym_glibc;
	void* sym_pnl;
	int (*alternative_read)(int, void*, int);

	dlhandle_pnl = dlopen("/home/tchi/Lecture/TME_PNL/TP_03/EXO-05/cron_func.so", RTLD_NOW|RTLD_LOCAL);
	sym_pnl = dlsym(dlhandle_pnl, "read");


	alternative_read = (int (*)(int, void*, int))sym_pnl;
	/** MY PART **/



	sa.sa_handler = &handle_sigalrm;
	sa.sa_flags = SA_RESTART;
	sigfillset(&sa.sa_mask);
	if (sigaction(SIGALRM, &sa, NULL) == -1)
		perror("Error: cannot handle SIGALRM");

	alarm(3);

	do {
		if (c == 0)
			printf("Enter : (i) insert (r) remove (e) end\n");

		/* if (read(STDIN_FILENO, &c, sizeof(char)) != sizeof(char))*/
		/*         return -1;*/
		if (alternative_read(STDIN_FILENO,
					&c,
					sizeof(char)) != sizeof(char))
			return -1;

		switch (c) {

			case 'i':
				printf("Inserting a new function\n");
				alternative_read =
					(int (*)(int, void*, int))sym_pnl;


				c = 0;

				break;
			case 'r':
				printf("Restoring the default function\n");
				alternative_read =
					(int (*)(int, void*, int))read;

				c = 0;
				break;
			default:
				break;

		}
	} while (c != 'e');

	return 0;
}
'''












