/**
 * @file main.c
 * @author Guéwen Maréchal
 * @brief snake SAE 1.01
 * @version 2
 * @date 24/11/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define MAX_SCORE 10
#define SNAKE_SIZE 10
#define X_DEPART 40
#define Y_DEPART 20
#define MAP_WIDTH 80
#define MAP_HEIGHT 40
#define SQUARE_WIDTH 5
#define SQUARE_HEIGHT 5
#define NOMBRE_PAVES 4
#define STOP 'a'
#define DROITE 'd'
#define GAUCHE 'q'
#define HAUT 'z'
#define BAS 's'
#define INT_LIMIT 32767
#define TETE 'O'
#define CORPS 'X'
#define BORDS '#'
#define ATTENTE_DEPART 2
#define DIR_DEPART DROITE
#define POMME '6'

void dessinerSerpent(int[SNAKE_SIZE], int[SNAKE_SIZE], int);
void progresser(int[SNAKE_SIZE], int[SNAKE_SIZE], char, bool*, bool*, int*);
void gotoXY(int, int);
void effacer(int, int);
void attendre(float);
void afficher(int, int, char);
void enableEcho();
void disableEcho();
void initTableau();
void dessinerPlateau(char[MAP_WIDTH][MAP_HEIGHT]);
void dessinerPaves();
void genererPave();
void ajoutePomme(char[MAP_WIDTH][MAP_HEIGHT]);
int kbhit();

// Stockage des coordonnées des pavés
typedef struct {
    int x;
    int y;
} Pave;
Pave paves[NOMBRE_PAVES];

// Stockage des coordonnées de la pomme
typedef struct {
    int x;
    int y;
} Pomme;
Pomme pomme;

int score = 0;
float attente = ATTENTE_DEPART;
int lesX[SNAKE_SIZE + 10], lesY[SNAKE_SIZE + 10];

int main() {
    char key, dir = DIR_DEPART, map[MAP_WIDTH][MAP_HEIGHT];
    bool perdu, aPomme = false;
    int taille = SNAKE_SIZE;

    // Initialisation de rand()
    srand(time(NULL));

    // Initialisation du serpent
    for (int i = 0; i < SNAKE_SIZE; i++) {
        lesX[i] = X_DEPART - i;
        lesY[i] = Y_DEPART;
    }

    // Génération de la map
	system("clear");
    initTableau(map);
    ajoutePomme(map);
    dessinerPlateau(map);

    do {
        disableEcho();
        
        if (!aPomme) {
            effacer(lesX[taille - 1], lesY[taille - 1]);
        } else {
            aPomme = false;
            ajoutePomme(map);
        }
        
        // Recalcul des coordonnées
        progresser(lesX, lesY, dir, &perdu, &aPomme, &taille);

        // Effacer le bout de la queue uniquement si le serpent n'a pas mangé une pomme
        

        // Serpent grossi
        if (aPomme) {
            taille++;
            attente = attente - 0.15;
        }
        
        // Fin de partie sous condition de défaite
        if (perdu) {
            gotoXY(0, 41);
            printf("Perdu !\n");
            enableEcho();
            return EXIT_SUCCESS;
        }

        dessinerSerpent(lesX, lesY, taille);

        // Détection et lecture d'une touche
        if (kbhit()) {
            key = getchar();
        }

        // Changements de directions
        if (key == DROITE && dir != GAUCHE) dir = DROITE;
        if (key == GAUCHE && dir != DROITE) dir = GAUCHE;
        if (key == HAUT && dir != BAS) dir = HAUT;
        if (key == BAS && dir != HAUT) dir = BAS;

        // Normalisation de l'attente
        if (dir == HAUT || dir == BAS) { 
            attendre(2 * attente);
        } else {
            attendre(attente);
        }
    } while (key != STOP);

    // Appui sur 'a'
	gotoXY(0, 41);
    printf("\n");
    enableEcho();
	return EXIT_SUCCESS;
}

void ajoutePomme(char map[MAP_WIDTH][MAP_HEIGHT]) {
    int x, y;
    do {
        x = (rand() % (MAP_WIDTH - SQUARE_WIDTH + 1)) + 2;
        do {
            y = (rand() % (MAP_HEIGHT - SQUARE_HEIGHT + 1)) + 2;
        } while(y >= lesY[0] - 5 && y <= lesY[0]);
    } while (map[x][y] == BORDS);
    map[x][y] = POMME;
    pomme.x = x; pomme.y = y;
}

/**
 * \fn initTableau(tableau cractères map)
 * \param map : caractère à chaque point de coordonnées
 * \brief Entre une valeur pour chaque coordonnées de la map
 */
void initTableau(char map[MAP_WIDTH][MAP_HEIGHT]) {
    // Initialisation des bordures
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1) {
                // Prise en compte des portes
                if (x == (int)(MAP_WIDTH / 2) - 1 || y == (int)(MAP_HEIGHT / 2) - 1) {
                    map[x][y] = ' ';
                } else {
                    map[x][y] = BORDS;
                }
            } else {
                map[x][y] = ' ';
            }
        }
    }

    // Génération et placement des pavés
    for (int p = 0; p < NOMBRE_PAVES; p++) {
        int x, y;

        // Générer la position aléatoire du pavé en fonction de la position du serpent
        x = (rand() % (MAP_WIDTH - SQUARE_WIDTH + 1)) + 2;
        if (x > MAP_WIDTH - 7) x = MAP_WIDTH - 7;
        do {
            y = (rand() % (MAP_HEIGHT - SQUARE_HEIGHT + 1)) + 2;
        } while(y >= lesY[0] - 5 && y <= lesY[0]);
        if (y > MAP_HEIGHT - 7) y = MAP_HEIGHT - 7;

        // Placer le pavé dans le tableau
        for (int h = 0; h < SQUARE_HEIGHT; h++) {
            for (int w = 0; w < SQUARE_WIDTH; w++) {
                map[x + w][y + h] = BORDS; // Placer le pavé sur la carte
            }
        }

        // Enregistrer la position du pavé
        paves[p].x = x;
        paves[p].y = y;
    }
}

/**
 * \fn dessinerPlateau(tableau caractères map)
 * \param map : tableau à 2 dimensions qui contient les caractères à chaque coordonnée
 * \brief Affiche le tableau en entier
 */
void dessinerPlateau(char map[MAP_WIDTH][MAP_HEIGHT]) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            printf("%c", map[x][y]);
        }
        printf("\n");
    }
}

/**
 * \fn effacer(entier x, entier y)
 * \param x : abscisse du charcatère à effacer
 * \param y : ordonnée du charactère à effacer
 * \brief Éfface le cractère aux coordonnées
 */
void effacer(int x, int y) {
    gotoXY(x, y);
    printf(" ");
}

/**
 * \fn dessinerSerpent(tableau lesX, tableau lesY)
 * \brief Ecrit le corps entier puis la tête dans le terminal
 * \param lesX : abscisse de chaque morceau du serpent
 * \param lesY : ordonnée de chqaue morceau du serpent
 * \param taille : taille variable
 */
void dessinerSerpent(int lesX[SNAKE_SIZE], int lesY[SNAKE_SIZE], int taille) {
    // Affichage du CORPS
    for (int i = 1; i < taille; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }

    // Affichage de la tête
    afficher(lesX[0], lesY[0], TETE);
}

/**
 * \fn progresser(tableau lesX, tableau lesY, caractère direction)
 * \param lesX : abscisse de chaque morceau du serpent
 * \param lesY : ordonnée de chqaue morceau du serpent
 * \param direction : direction vers laquelle le serpent doit se diriger
 * \brief Recalcul les coordonnées des points
 */
void progresser(int lesX[SNAKE_SIZE], int lesY[SNAKE_SIZE], char direction, bool* perdu, bool* aPomme, int* taille) {
    // Déplacer le corps du serpent
    for (int i = *taille - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }

    // Mettre à jour la tête en fonction de la direction
    if (direction == HAUT) {
        if (lesY[0] == 0) {
            lesY[0] = MAP_HEIGHT + 1;
        }
        lesY[0]--;
    } else if (direction == BAS) {
        if (lesY[0] == MAP_HEIGHT) {
            lesY[0] = 0;
        }
        lesY[0]++;
    } else if (direction == DROITE) {
        if (lesX[0] == MAP_WIDTH) {
            lesX[0] = 0;
        }
        lesX[0]++;
    } else if (direction == GAUCHE) {
        if (lesX[0] == 0) {
            lesX[0] = MAP_WIDTH + 1;
        }
        lesX[0]--;
    }

    // Détection pomme
    if (lesX[0] - 1 == pomme.x && lesY[0] == pomme.y) {
        *aPomme = true;
    }

    // Collision des murs
    if ((lesX[0] <= 1 && lesY[0] != (MAP_HEIGHT / 2)) || (lesX[0] >= MAP_WIDTH && lesY[0] != (MAP_HEIGHT / 2)) || (lesY[0] <= 1 && lesX[0] != (MAP_WIDTH / 2)) || (lesY[0] >= MAP_HEIGHT && lesX[0] != (MAP_WIDTH / 2))) {
        *perdu = true;
    }

    // Collision du corps
    for (int i = 1; i < SNAKE_SIZE; i++) {
        if (lesX[0] == lesX[i] && lesY[0] == lesY[i]) {
            *perdu = true;
            break;
        }
    }

    // Collision des pavés
    for (int p = 0; p < NOMBRE_PAVES; p++) {
        for (int w = paves[p].x + 1 ; w <= paves[p].x + SQUARE_WIDTH; w++) {
            for (int h = paves[p].y + 1; h <= paves[p].y + SQUARE_HEIGHT; h++) {
                if (lesX[0] == w && lesY[0] == h) {
                    *perdu = true;
                    break;
                }
                if (*perdu) break;
            }
            if (*perdu) break;
        }
        if (*perdu) break;
    }
}

/**
 * \fn afficher(entier x, entier y, caractère c)
 * \param x : abcisse du caractère à afficher
 * \param y : ordonnée du caractère à afficher
 * \param c : caractère à afficher
 * \brief affiche le caractère aux coordonnées indiquées
 */
void afficher(int x, int y, char c) {
    gotoXY(x, y);
    printf("%c", c);
}

/**
 * \fn gotoXY(entier x, entier y)
 * \param x : abscisse de la position
 * \param y : ordonnée de la position
 * \brief Déplace le curseur
 */
void gotoXY(int x, int y) { 
    printf("\033[%d;%df", y, x);
}

/**
 * \fn attendre(flottant n)
 * \param n : multiple du temps maximal de la fonction usleep()
 * \brief Fonction qui met le programme en pause plus ou moins longtemps selon n
 */
void attendre(float n) {
	if (n != (int)n) {
        if (n < 0) {
            usleep((int)(INT_LIMIT * n));
        } else {
            usleep((int)(INT_LIMIT * (n - (int)n)));
            for (int i = (int)n; i > 0; i--) {
			    usleep(INT_LIMIT);
            }
        }
	} else {
        if (n < 0) n = n * -1;
		for (int i = n; i > 0; i--) {
			usleep(INT_LIMIT);
		}
	}
}

/**
 * \fn int kbhit()
 * \return int 0 si il y a un charcatère sinon 1
 * \brief Détecte l'activation d'une touche
 */
int kbhit() {
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    // Mettre le terminal en mode non bloquant
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
    ch = getchar();

    // Restaurer le mode du terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
 
    if (ch != EOF) {
        ungetc(ch, stdin);
        unCaractere = 1;
    } 
    return unCaractere;
}

/**
 * \fn disableEcho()
 * \brief Désactive l'affichage des inputs au clavier
 */
void disableEcho() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Desactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/**
 * \fn disableEcho()
 * \brief Active l'affichage des inputs au clavier
 */
void enableEcho() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Reactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}