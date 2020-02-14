#include <time.h>
#include <stdlib.h>

#include "ez-draw.h"
#include "ez-image.h"

#define TICK_DURATION 20         /* durée d'un tick d'horloge */
#define FLAP_TICKS 3             /* nombre de ticks avant de changer l'image de l'oiseau */
#define STARTING_SPEED 3         /* vitesse initiale de l'oiseau */
#define BIRD_ACCELERATION 0.003  /* acceleration de l'oiseau à tick d'horloge */
#define FLAP_SPEED -6            /* magitude de la vitesse après un battement d'aile, "c" dans le sujet */
#define GRAVITY 0.4              /* magitude du vecteur de gravité, "g" dans le sujet */
#define HOLE 175 		 /* taille du trou */


/* Énumération pour l'état du jeu */
typedef enum {Introduction, Playing, GameOver, Pause} State;

/* Type pour stoquer un vecteur de dimension 2 */
typedef struct {
    double x, y;
} Vector;

/* Type pour stoquer un rectangle, contenant son coin superieur gauche, sa longueur et sa largeur */
typedef struct {
    double x, y, width, height;
} MyRectangle;

/* Type pour l'oiseau */
typedef struct {
    Vector position; 		/* position dans le plan */  
    Vector velocity; 		/* vecteur de déplacement */ 
    Vector size;    		/* taille */

    Ez_image* images[4];	/* images pour l'animation */
    int current_image; 		/* numéro de l'image à afficher */
    int next_animation_tick; 	/* nombre de ticks restant avant le passage à l'image suivante */
} Bird;

/* Type pour un tube (venant du haut ou du bas) */
typedef struct {
    Vector position; 		/* position dans le plan */
    Vector size;     		/* taille */
    Ez_image* image; 		/* image à afficher */
} Tube;

/* Type pour le sol */
typedef struct {
    Vector position; 		/* position dans le plan */
    Vector size;     		/* taille */
    Ez_image* image; 		/* image à afficher */
} Floor;

/* Type pour toutes les données relatives à la partie.
   C'est un pointeur vers cette structure de données qui est associé
   à la fenêtre et récupèrable quand un évènement se produit. 
*/
typedef struct {
    Ez_image* gameover; 	/* image avec le texte "game over" */
    Ez_image* getready; 	/* image avec le texte "get ready" */
    Ez_image* background_image; /* image de fond */
    Ez_image* background_image2;

    Tube tube_top;              /* tube du haut */
    Tube tube_bottom;           /* tube du bas */
    Floor floor;                /* sol */
    Bird bird;          	/* oiseau */

    int width;          	/* largeur de la fenêtre de jeu */
    int height;         	/* hauteur de la fenêtre de jeu */

	int score;
	int distance;

    State state;        	/* état du jeu */
} Game;


void place_tubes(Game* game) {
	int trou = rand()%((int)game->floor.position.y-HOLE); 	/*le 10 est purement estétique pour toujours voir un minimum les deux tubes */
	if(game->state == Introduction){
		game->tube_top.position.x = game->width;
		game->tube_top.position.y = game->height/2-game->tube_top.size.y-HOLE;	
		game->tube_bottom.position.x = game->width;
		game->tube_bottom.position.y = -game->height/2+game->tube_top.size.y;
	} 
	else {
		game->tube_top.position.x = game->width;
		game->tube_top.position.y = trou-game->tube_top.size.y;	
		game->tube_bottom.position.x = game->width;
		game->tube_bottom.position.y = trou+HOLE;
	}
}



void start_game(Game* game) {
	game->state = Introduction; 
	game->score = 0;
	game->distance = 0;
	game->floor.position.x = 0;
	game->floor.position.y = game->height-game->floor.size.y;
	game->bird.position.x = (game->width-game->bird.size.x)/2;
	game->bird.position.y = (game->height-game->bird.size.y)/2;
	game->bird.current_image = 0;
	game->bird.velocity.x = STARTING_SPEED;
	game->bird.velocity.y = 0;
	place_tubes(game);
}


Game* create_game() {
    Game* game = malloc(sizeof(Game));
    game->width = 371;
    game->height = 700;
    game->background_image = ez_image_load ("img/background.png");
    game->background_image2 = ez_image_load ("img/background2.png");	
	if (game->background_image == NULL) exit (1);
	if (game->background_image2 == NULL) exit (1);
	game->gameover = ez_image_load ("img/gameover.png");	
	if (game->gameover == NULL) exit (1);
	game->getready = ez_image_load ("img/getready.png");	
	if (game->getready == NULL) exit (1);   
	
	game->floor.image = ez_image_load ("img/floor.png");
	if (game->floor.image == NULL) exit (1);
	game->floor.size.x = game->floor.image->width;
	game->floor.size.y = game->floor.image->height;
	
	game->bird.images[0] = ez_image_load ("img/bird1.png");
	if (game->bird.images[0] == NULL) exit (1);
	game->bird.images[1] = ez_image_load ("img/bird2.png");
	if (game->bird.images[1] == NULL) exit (1);
	game->bird.images[2] = ez_image_load ("img/bird3.png");
	if (game->bird.images[2] == NULL) exit (1);	
	game->bird.images[3] = ez_image_load ("img/bird4.png");
	if (game->bird.images[3] == NULL) exit (1);
	game->bird.next_animation_tick = 0 + FLAP_TICKS;
	game->bird.size.x = game->bird.images[0]->width;
	game->bird.size.y = game->bird.images[0]->height;
	
	game->tube_top.image = ez_image_load ("img/tube-down.png");
	if (game->tube_top.image == NULL) exit (1);
	game->tube_top.size.x = game->tube_top.image->width;
	game->tube_top.size.y = game->tube_top.image->height;
	
	game->tube_bottom.image = ez_image_load ("img/tube-up.png");
	if (game->tube_bottom.image == NULL) exit (1);
	game->tube_bottom.size.x = game->tube_bottom.image->width;
	game->tube_bottom.size.y = game->tube_bottom.image->height;
	
	start_game(game);

    return game;
}



void free_game(Game* game) {
	int i;
	ez_image_destroy(game->background_image);
	ez_image_destroy(game->gameover);
	ez_image_destroy(game->getready);
	for (i=0;i<4;i++)
		ez_image_destroy(game->bird.images[i]);
	ez_image_destroy(game->floor.image);
	ez_image_destroy(game->tube_top.image);
	ez_image_destroy(game->tube_bottom.image);
	free(game);

}

int intersection(Ez_image *Im,Vector u, MyRectangle r) { //Pixel perfect collision, on vérifie si l'un des pixels non transparent touche le rectangle du tuyau ou du sol
	int x,y;
	for (x=0;x<Im->width;x++) {
		for(y=0;y<Im->height;y++) {
			if ( Im->pixels_rgba[(y*Im->width+x)*4+3]>128 )		/* 128 est le seuil d'opacité */
				if(x+u.x < r.x + r.width && x + u.x > r.x)
					if (y + u.y < r.y + r.height && y + u.y > r.y)
						return 1;
		}
	}
	return 0;
}

int game_over(Game* game) {

	MyRectangle tube_top;
	MyRectangle tube_bottom;
	MyRectangle floor;

	tube_top.x = game->tube_top.position.x;
	tube_top.y = game->tube_top.position.y;
	tube_top.width = game->tube_top.size.x;
	tube_top.height = game->tube_top.size.y;
	
	tube_bottom.x = game->tube_bottom.position.x;
	tube_bottom.y = game->tube_bottom.position.y;
	tube_bottom.width = game->tube_bottom.size.x;
	tube_bottom.height = game->tube_bottom.size.y;
	
	floor.x = game->floor.position.x+game->bird.velocity.x;
	floor.y = game->floor.position.y;
	floor.width = game->floor.size.x;
	floor.height = game->floor.size.y;
	
	//Si l'oiseau touche le tube top ou le tube bottom ou le sol, alors c'est game over
	if ((intersection(game->bird.images[game->bird.current_image],game->bird.position,tube_top) || intersection(game->bird.images[game->bird.current_image],
		game->bird.position,tube_bottom) || intersection(game->bird.images[game->bird.current_image],game->bird.position,floor)) == 1)
		return 1;	
    return 0;
}

void redraw(Ez_event *ev) {
    
    Game* game = (Game*) ez_get_data(ev->win);
	

	ez_image_paint (ev->win, game->background_image, 0, 0);
	ez_image_paint (ev->win, game->bird.images[game->bird.current_image], game->bird.position.x, game->bird.position.y);
	ez_image_paint (ev->win, game->tube_top.image, game->tube_top.position.x, game->tube_top.position.y);
	ez_image_paint (ev->win, game->tube_bottom.image, game->tube_bottom.position.x, game->tube_bottom.position.y);	
	ez_image_paint (ev->win, game->floor.image, game->floor.position.x, game->floor.position.y);    
	if (game->state == Introduction)
    	ez_image_paint (ev->win, game->getready, game->width/2-game->getready->width/2, game->height/2-2*game->getready->height);    
    if (game->state == GameOver )
    	ez_image_paint (ev->win, game->gameover, game->width/2-game->gameover->width/2, game->height/2-2*game->gameover->height);  
    ez_set_color (ez_blue);
    ez_draw_text (ev->win, EZ_TL, 2, 1, "Score : %d",game->score);
    if (game->state == Pause) 
    	ez_draw_text (ev->win, EZ_MC, game->width/2, game->height/2-game->floor.size.y/2, "PAUSE");
	// L'alignement avec EZ_BR et EZ_BL ne tient pas bien en compte
	// de la taille de la chaine de caractère on doit l'ajuster directement, -10 a été choisi à arbitrairement
    ez_draw_text (ev->win, EZ_BR, game->width - 10, game->height - 10, "By PARRA Quentin and TOTH Benoit"); 
    ez_draw_text (ev->win,EZ_BL, 2, game->height - 10, "Espace : Sauter\np: Pause\nq: Quitter\nr: Relancer\nFleche gauche: Nuit\nFleche droite: Jour"); 
}


void update_simulation(Ez_event* ev) {
    Game* game = (Game*) ez_get_data(ev->win);
	if (game->state != Pause) 
	{		
		
		if (game->state == Playing)
		{
			if (game->bird.position.x > game->bird.size.x )	
				game->bird.position.x -= game->bird.velocity.x;
				
		    if (game->bird.position.x < game->bird.size.x ) {
				game->distance += game->bird.velocity.x;
				if ( game->width - game->distance < game->bird.size.y) {
					game->score += 1;
					game->distance = 0;
				}
				game->bird.velocity.x += BIRD_ACCELERATION;
				game->bird.velocity.y += GRAVITY;
				game->bird.position.y += game->bird.velocity.y;
				game->tube_top.position.x -= game->bird.velocity.x;
				game->tube_bottom.position.x -= game->bird.velocity.x;
				if (game_over(game)==1)
					game->state = GameOver;
				if (game->tube_top.position.x < -game->tube_top.size.x)
				{
					place_tubes(game);
					game->distance = 0;
				}
			}
			
		}
 		if (game->state != GameOver)
 		{
			game->floor.position.x=(int) (game->floor.position.x-game->bird.velocity.x)%19;
			game->bird.next_animation_tick -=1;
			if (game->bird.next_animation_tick == 0)
			{
				game->bird.next_animation_tick += FLAP_TICKS;
				game->bird.current_image =(game->bird.current_image +1)%3;
			}
		}
	}
    ez_send_expose(ev->win);
}

void event_callback (Ez_event *ev) {
    Game* game = (Game*) ez_get_data(ev->win);
    switch (ev->type) {
        case Expose:
            redraw(ev);
	    break;

        case TimerNotify:
            ez_start_timer(ev->win, TICK_DURATION);

			update_simulation(ev);
	    break;

        case ButtonPress:
					if (game->state == Introduction || game->state == Pause)
						game->state = Playing; 
					if (game->state == Playing)
						game->bird.velocity.y=FLAP_SPEED;	
            break;

        case KeyPress:
            switch (ev->key_sym) {
				case XK_q : ez_quit (); break;
				case XK_Q : ez_quit (); break;
				case XK_space : 
					if (game->state == Introduction || game->state == Pause)
						game->state = Playing; 
					if (game->state == Playing)
						game->bird.velocity.y=FLAP_SPEED;			
				break;
				case XK_r : start_game(game); break;
				case XK_R : start_game(game); break;
				case XK_p : game->state=Pause; break;
				case XK_Left : 
					ez_image_destroy(game->background_image);
					game->background_image = ez_image_load ("img/background2.png");
				break;
				case XK_Right : 
					ez_image_destroy(game->background_image);
					game->background_image = ez_image_load ("img/background.png");
				break;
					
			}

            break;
    }
}


int main() {
    if (ez_init() < 0) exit(1);
    srand(time(NULL));
    Game* game = create_game();

    Ez_window window = ez_window_create(game->width, game->height, "Flappy bird", event_callback);
    ez_window_dbuf(window, 1); 	/* pour une animation fluide */
    ez_set_data(window, game); 	/* mémorise le pointeur vers les paramètres du jeu */
    ez_start_timer(window, 0); 	/* lancer les ticks d'horloge */
    ez_main_loop ();           	/* boucle de gestion des énénements d'ez-draw */

    free_game(game);           	/* libérer la mémoire occupée par les images et le jeu */

    return 0;
}
