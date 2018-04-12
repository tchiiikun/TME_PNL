### TP 08 – Mécanismes de gestion de la mémoire dans le noyau


## Exercice 1 : Patcher son noyau à l’ancienne
# Question 1
On utilise xzcat cat on a un fichier xz.
patch -p donne a partir d'ou on doit changer dans notre arborescence avec 0
pour racine.

xzcat ../../TME_PNL/TP_08/EXO-01/tp8-linux-4.9.83.patch.xz | patch -p 1

extern et nons static parce que ca n'a pas de sens d'export un static. Donc on
la met externe pour que la resolution se fasse lors du link. Dans le .h on la
met extern. On fait de meme pour une hashtable. On modifie fs/dcache.c et
fs/dcache.h

Ce patch met en export et extern les fonctions suivante et variables:

+unsigned int d_hash_shift __read_mostly;
+struct hlist_bl_head *dentry_hashtable __read_mostly;
+extern unsigned int d_hash_shift;
pecial version of lists, where head of the list has a lock in the lowest
* bit. This is useful for scalable hash tables without increasing memory
* footprint overhead.
*
* For modification operations, the 0 bit of hlist_bl_head->first
* pointer must be set.
*
* With some small modifications, this can easily be adapted to store
several
* arbitrary bits (not just a single lock bit), if the need arises to
store
* some fast and compact auxiliary data.
*/


# Question 2
Done
[  151.739136] HelloWorldParam: loading out-of-tree module taints kernel.
[  151.740713] Hello, world
[  151.741280] d_hash_shift [  151.741843]  d_hash_shift value : 17

# Question 3
Done

## Exercice 2 (rootkit 2) : Un cache pas très discret
# Question 1-2-3
[ 4323.599296] Weasel
[ 4323.600214] d_hash_shift value : 17
[ 4323.601611] list size : 131072
[ 4323.602865] table adress : ffffc90000005000
[ 4323.606460] value : 28223
[ 4323.607245] size max in the list : 4

La liste est enorme et le nombre de collision est faible donc c'est une bonne
table de hash.

# Question 4

# Question 5
# Question 6
# Question 7


## Exercice 3 : Gestion efficace de la mémoire avec les slab
# Question 1
# Question 2
# Question 3


## Exercice 4 : Pré-allocation de la mémoire avec les mempools
# Question 1
# Question 2


## Exercice 5 : Récupération « au besoin » de la mémoire : kref
# Question 1
# Question 2
# Question 3


## Exercice 6 : Accès aux informations de monitoring depuis l’espace utilisateur
# Question 1
# Question 2
# Question 3


## Exercice 7 : Monitoring de plusieurs processus simultanément
# Question 1
# Question 2
# Question 3


