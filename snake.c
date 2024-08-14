#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define RIGHT 67
#define LEFT 68
#define DOWN 66
#define UP 65

struct coordinate{
    int x;
    int y;
}food_pos;

typedef struct snake_cell{
    struct coordinate pos;
    char chr;
    struct snake_cell* next;
    struct snake_cell* prev;
}cell;
// =========================================================================
struct snake_cell* create(char, int, int);
void add_end(cell*,cell*);
void clear_screen(void);
int read_key(void);
void hide_cursor(void);
void right(cell*);
void left(cell*);
void up(cell*);
void down(cell*);
void print_food(void);
void increase_len_snake(int, int, cell*);
int game_over(cell*,int ,int);
// =========================================================================
int main(void){

    // setting terminal not to echo what was pressed and not waiting for `enter` by user
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~(ECHO | ICANON);
    tty.c_lflag &= ~(IXON | IXOFF | IXANY);
    tcsetattr(STDIN_FILENO,TCSANOW, &tty);
    
    // non-blocking mode of terminal
    int old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK); /*set to non-blocking mode which make terminal not to wait for user input,*/
    // getting terminal winodw size
    struct winsize ws;
    int width, height;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ,&ws)==0){
	width = ws.ws_col;
	height = ws.ws_row;
    }else{
	width = -1;
	height = -1;
    }
    
    //------------------------------------
    food_pos.x = 25;       food_pos.y = 25;
    
    cell* head = create('>',20,20);
    add_end(head,create('*',0,0));

    int chr,
	key_pressed = RIGHT; // default is RIGHT

    clear_screen();
    printf("PRESS `q` TO EXIT\nMOVE BY ← ↑ → ↓\n");
    usleep(1000000);
    hide_cursor();

    while(1){
	chr = read_key();
	
	if(chr != EOF && (chr == RIGHT || chr == LEFT || chr == DOWN || chr == UP || chr == 'q')){
	    key_pressed = chr;
	}

	clear_screen();
	if(game_over(head,width,height)){
	    clear_screen();
	    printf("GAME OVER\n");
	    return 0;

	}
	print_food();
	
	if(key_pressed == 'q'){
	    printf("\033[0;0HABORTED\n");
	    return 0;
	}
	switch(key_pressed){
	    case RIGHT:
		right(head);
		head->pos.x++;
		break;
	    case LEFT:
		left(head);
		head->pos.x--;
		break;
	    case UP:
		up(head);
		head->pos.y--;
		break;
	    case DOWN:
		down(head);
		head->pos.y++;
		break;
	}

	usleep(150000);
	increase_len_snake(width, height, head);
    }

    return 0;
}
// ========================================================================
void clear_screen(void){
	printf("\033[2J\033[H");
}

int read_key(void){
    int chr = getchar();
    // check if it is escape character (\033, \e, \x1B)  \e[A, \e[B, \e[C, \e[D
    if(chr== 27){ 
	// Read the next two characters for escape sequences [A, [B, [C, [D
	getchar(); // Skip the '['
	chr = getchar(); // store the last char A,B,C,D
    }
    return chr;
}

void hide_cursor(void){
    printf("%s[?25l",ESC);
}

struct snake_cell* create(char chr,int y,int x){
    cell* tmp = (cell*) malloc(sizeof(cell));
    tmp->chr = chr;
    tmp->next = tmp->prev = NULL;
    tmp->pos.x = x;	tmp->pos.y = y;
    return tmp;
}

void add_end(cell* head,cell* new){
    cell* tmp = head;
    while(tmp->next)
	tmp = tmp->next;
    tmp->next = new;
    new->prev = tmp;
}

void right(cell* head){
   
    cell* tmp = head;
    head->chr = '>';

    for(;head;head=head->next)
       printf("\033[%d;%dH%c", head->pos.y, head->pos.x,head->chr);
    
    for(;tmp->next;tmp=tmp->next);
    for(;tmp->prev;tmp=tmp->prev){
	tmp->pos.y = tmp->prev->pos.y;
	tmp->pos.x = tmp->prev->pos.x;
    }
       
}

void left(cell* head){

    cell* tmp = head;
    head->chr = '<';

    for(;head;head=head->next)
       printf("\033[%d;%dH%c", head->pos.y, head->pos.x,head->chr);
    
    for(;tmp->next;tmp=tmp->next);
    for(;tmp->prev;tmp=tmp->prev){
	tmp->pos.y = tmp->prev->pos.y;
	tmp->pos.x = tmp->prev->pos.x;
    }
}

void down(cell* head){

    cell* tmp = head;
    head->chr = 'v';

    for(;head;head=head->next)
       printf("\033[%d;%dH%c", head->pos.y, head->pos.x,head->chr);
    
    for(;tmp->next;tmp=tmp->next);
    for(;tmp->prev;tmp=tmp->prev){
	tmp->pos.y = tmp->prev->pos.y;
	tmp->pos.x = tmp->prev->pos.x;
    }
}

void up(cell* head){

    cell* tmp = head;
    head->chr = '^';

    for(;head;head=head->next)
       printf("\033[%d;%dH%c", head->pos.y, head->pos.x,head->chr);
    
    for(;tmp->next;tmp=tmp->next);
    for(;tmp->prev;tmp=tmp->prev){
	tmp->pos.y = tmp->prev->pos.y;
	tmp->pos.x = tmp->prev->pos.x;
    }
}

void print_food(void){
    printf("\033[%d;%dH", food_pos.y, food_pos.x);
    printf("+");
}

void increase_len_snake(int width,int height, cell* head){
    if(food_pos.x != head->pos.x || food_pos.y != head->pos.y)
	return;
    
    add_end(head,create('*',0,0));
    srand(time(NULL));
    food_pos.x = rand() % width;
    food_pos.y = rand() % height;
}

int game_over(cell* head,int width,int height){
    if(head->pos.x == width || head->pos.y == height || head->pos.x == 0 || head->pos.y == height)
	return 1;
    for(cell* tmp = head->next;tmp;tmp=tmp->next){
	if(head->pos.x == tmp->pos.x && head->pos.y == tmp->pos.y){
	    return 1;
	}
    }
    return 0;
}
