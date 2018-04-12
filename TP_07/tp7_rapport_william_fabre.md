### TP 08 – Mécanismes de gestion de la mémoire dans le noyau


## Exercice 1 : Monitoring de l’activité d’un processus (suite)
# Question 1
https://www.kernel.org/doc/html/v4.13/process/applying-patches.html
https://lwn.net/Articles/167034/


The -mm patches and the linux-next tree

The -mm patches are experimental patches released by Andrew Morton.

In the past, -mm tree were used to also test subsystem patches, but this function is now done via the linux-next <https://www.kernel.org/doc/man-pages/linux-next.html> tree. The Subsystem maintainers push their patches first to linux-next, and, during the merge window, sends them directly to Linus.

The -mm patches serve as a sort of proving ground for new features and other experimental patches that aren’t merged via a subsystem tree. Once such patches has proved its worth in -mm for a while Andrew pushes it on to Linus for inclusion in mainline.

The linux-next tree is daily updated, and includes the -mm patches. Both are in constant flux and contains many experimental features, a lot of debugging patches not appropriate for mainline etc., and is the most experimental of the branches described in this document.

These patches are not appropriate for use on systems that are supposed to be stable and they are more risky to run than any of the other branches (make sure you have up-to-date backups – that goes for any experimental kernel but even more so for -mm patches or using a Kernel from the linux-next tree).

Testing of -mm patches and linux-next is greatly appreciated since the whole point of those are to weed out regressions, crashes, data corruption bugs, build breakage (and any other bug in general) before changes are merged into the more stable mainline Linus tree.

But testers of -mm and linux-next should be aware that breakages are more common than in any other tree.


# Question 2
# Question 3
# Question 4

## Exercice 2 : Récupération automatique de la mémoire, le shrinker
# Question 1
# Question 2
# Question 3


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
