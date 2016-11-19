//
//  main.c
//  sdlp
//
//  Created by Bertalan Kormendy on 2016. 11. 10..
//  Copyright © 2016. Bertalan Kormendy. All rights reserved.
//

#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>

#define WIDTH 12
#define HEIGHT 8

//render flow:

//1 render last round >
//2 get direcions >
//3 check ahead >
//4 update snake accordingly >
//5 spawn new food if necessary
//6 PARSE ALL COORDS INTO TILE TYPES IN MAP                 COLORZ?
//7 DRAW ALL RECTS IN MAP (coord.x*=50, coord.y*=50)
//> Goto 1

typedef struct Coord {
    int x;
    int y;
}Coord; //int x, int y

typedef struct DynoArray {
    int length;
    Coord head;
    Coord* coords;
} DynoArray; //int length, Coord* coords, Coord head

void DynoArray_init(DynoArray *dyno, Coord *values, int length){
    dyno->length=length;
    dyno->coords=values;
    dyno->head=dyno->coords[0];
}//assigns initial snake coords, initial snake length

void DynoArray_add(DynoArray *dyno, Coord new){
    dyno->coords=realloc(dyno->coords, sizeof(dyno->coords)+sizeof(new));
    dyno->length+=1;
    for (int i=1; i<dyno->length; i++) {
        dyno->coords[i+1]=dyno->coords[i];
    }
    dyno->coords[0]=new;
} //Add new element to snake->coords, might need better realloc strategy

typedef enum Direction {
    Left,
    Right,
    Up,
    Down,
} Direction; //Left, Right, Up, Down

void update_snake(DynoArray *snake, Direction dir){
    for (int i=1; i<snake->length; i++) {
        snake->coords[i]=snake->coords[i-1];
    }
    switch (dir) {
        case Left:
            snake->head.x-=50;
            break;
        case Right:
            snake->head.x+=50;
            break;
        case Up:
            snake->head.y-=50;
            break;
        case Down:
            snake->head.y+=50;
            break;
        default:
            break;
    }
}

typedef enum Food_type {
    Fast,
    Bonus,
    Ghost,
    Normal,
} Food_type; //Normal, Fast, Bonus

typedef struct Food_item {
    Coord coords;
    Food_type type;
} Food_item; //Coord coords, Food_type type

void spawn_food(Food_item *food){
    Coord c = {rand() % 12, rand() % 8};
    Food_type ft;
    int t= rand()%10;
    if (t>2)
        ft=Normal;
    else
        ft=t;
    food->coords=c;
    food->type=ft;
} //randomly generate food - either a normal food item, a bomb, a ghost or a booster

typedef enum Tiles {
    Snake,
    Food,
    Wall,
    Empty,
} Tiles; //Snake, Food, [Wall, Empty]

typedef struct Game {
    int ghost;
    int fast;
    int player_one_score;
    int player_two_score;
} Game; //Stores game state (int p1/p2_score, int fast, int ghost)

SDL_Rect* parse_map_to_rects(DynoArray *snake, Food_item *food){
    SDL_Rect *tiles=(SDL_Rect *) malloc((sizeof(snake->length)*sizeof(SDL_Rect)+sizeof(SDL_Rect)));
    for (int i=1; i<snake->length+1; i++) {
        SDL_Rect r = {snake->coords->x*50, snake->coords->y*50, 50, 50};
        tiles[i]=r;
    }
    SDL_Rect r={food->coords.x*50, food->coords.y*50, 50, 50};
    tiles[0]=r;
    return tiles;
}

int check_bounds(DynoArray *snake , Game *game){
    if (snake->head.x==-1){
        if (game->ghost)
            snake->head.x=WIDTH;
        return 0;
    }
    if (snake->head.x==WIDTH) {
        if (game->ghost)
            snake->head.x=0;
        return 0;
    }
    
    if (snake->head.y==-1){
        if (game->ghost)
            snake->head.y=HEIGHT;
        return 0;
    }
    if (snake->head.y==HEIGHT){
        if (game->ghost)
            snake->head.y=0;
        return 0;
    }
    return 1;
} //checks bounds against ghost flag and coords     REFACTOR IF(GAME->GHOST)

void check_ahead(DynoArray *snake, Food_item *food, Game *game){
    if (snake->head.x==food->coords.x && snake->head.y==food->coords.y){
        switch (food->type) {
            case Normal:
                game->player_one_score++;
                break;
            case Bonus:
                game->player_one_score+=5;
                break;
            case Ghost:
                game->player_one_score++;
                game->ghost=60;
                break;
            case Fast:
                game->player_one_score++;
                game->fast=120;
                break;
            default:
                break;
        }
        DynoArray_add(snake, food->coords);
        spawn_food(food);
    }
} //Checks ahead to see if food [or other snake is in the way], modifies game instance accordingly

int main() {
    int map[WIDTH][HEIGHT];
    DynoArray snake;
    Game game = {0,0,0,0};
    Direction dir=Left;
    Food_item food;
    
    Coord c1={6,1};
    Coord c2={7,1};
    Coord c3={8,1};
    
    Coord coords[3]={c1,c2,c3};
    
    DynoArray_init(&snake, coords, 6);
    
    spawn_food(&food);
    
    SDL_Window *win=SDL_CreateWindow("Ultra Snake", 100, 100, WIDTH*50, HEIGHT*50, SDL_WINDOW_SHOWN);
    if(win==NULL){
        printf("Window nokay");
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *ren=SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren==NULL){
        printf("Renderer nokay");
        SDL_Quit();
        return 1;
    }
    
    while(check_bounds(&snake, &game)){
        SDL_RenderClear(ren);
        SDL_RenderFillRects(ren, snake.coords, snake.length);       //REAL problem, crux of development,
        SDL_RenderPresent(ren);
        
        if (game.ghost)
            game.ghost--;
        
        if (game.fast)
            game.fast--;
        
        if (game.fast)
            SDL_Delay(250);
        else
            SDL_Delay(500);
        
        SDL_Event event;
        SDL_PollEvent(&event);
        
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        if (dir!=Up && dir != Down){
                            dir=Up;
                            printf("Up\n");
                        }
                        break;
                    case SDLK_s:
                        if (dir!=Up && dir != Down){
                            dir=Down;
                            printf("Down\n");
                        }
                        break;
                    case SDLK_d:
                        if (dir != Right && dir != Left){
                            dir=Right;
                            printf("Right\n");
                        }
                        break;
                    case SDLK_a:
                        if (dir != Right && dir != Left){
                            dir=Left;
                            printf("Left\n");
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        } //decide direction
        
        update_snake(&snake, dir);
    }
    
    
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
