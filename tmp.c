#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define ESC "\x1B"
#define RIGHT 67
#define LEFT 68
#define DOWN 66
#define UP 65
struct {
    size_t length;
    char *mem;
} snake;

struct {
    int x;
    int y;
}current_pos,food_pos;
// =========================================================================
void clear_screen(void);
int read_key(void);
void hide_cursor(void);
void right(void);
void left(void);
void up(void);
void down(void);
void print_food(void);
void incr_len_snake(int, int);
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
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK); /*set to non-blocking mode which make terminal not to wait for user input,
							    and functions like read returns -1 and getchar/getc return EOF(typically -1)*/
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
    current_pos.x = current_pos.y = 20;
    snake.length = 5;
    snake.mem = malloc(snake.length);
    snake.mem[0] = '>';
    snake.mem[1] = '*';
    snake.mem[2] = '*';
    snake.mem[3] = '*';
    snake.mem[4] = 0;

    int chr,
	key_pressed =LEFT, // default is RIGHT
	pre_command = 0;

    clear_screen();
    printf("PRESS `q` TO EXIT\nMOVE BY ← ↑ → ↓\n");
    usleep(1000000);
    hide_cursor();

    while(1){
	chr = read_key();
	
	if(chr != EOF && (chr == RIGHT || chr == LEFT || chr == DOWN || chr == UP )){
	    pre_command = 1;
	    key_pressed = chr;
	}

	clear_screen();
	print_food();
	
	if(key_pressed == 'q'){
	    printf("%s[0;0HABORTED\n",ESC);
	    return 0;
	}
	switch(key_pressed){
	    case RIGHT:
		right();
		break;
	    case LEFT:
		left();
		break;
	    case UP:
		up();
		break;
	    case DOWN:
		down();
		break;
	}

	incr_len_snake(width, height);
    }
    return 0;
}
// =========================================================================
void clear_screen(void){
	printf("%s[2J%s[H",ESC,ESC);
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

void right(void){
    current_pos.x++;
    snake.mem[0] = '>';
    for(int j=0, x = current_pos.x ; snake.mem[j] ; x--,j++){
	printf("\033[%d;%dH%c", current_pos.y, x, snake.mem[j]);
	fflush(stdout);
	usleep(20000);
    }
}

void left(void){
    current_pos.x--;
    snake.mem[0] = '<';
    printf("\033[%d;%dH", current_pos.y, current_pos.x);
    for(int i=0;snake.mem[i];i++){
	printf("%c",snake.mem[i]);
	fflush(stdout);
	usleep(20000);
    }

}
void down(void){
    current_pos.y++;
    snake.mem[0] = 'v';
    for(int i=0, y=current_pos.y; snake.mem[i] ;i++,y--){
	printf("\033[%d;%dH%c", y, current_pos.x, snake.mem[i]);
	fflush(stdout);
	usleep(20000);
    }

}
void up(void){
    current_pos.y--;
    snake.mem[0] = '^';
    for(int i=0, y=current_pos.y;snake.mem[i];i++,y++){
	printf("\033[%d;%dH%c", y, current_pos.x, snake.mem[i]);
	fflush(stdout);
	usleep(20000);
    }

}

void print_food(void){
    printf("\033[%d;%dH", food_pos.y, food_pos.x);
    printf("+");
}

void incr_len_snake(int width,int height){
    if(food_pos.x != current_pos.x || food_pos.y != current_pos.y)
	return;

    snake.mem[snake.length-1] = '*';
    snake.mem[(++snake.length) - 1] = 0;
    srand(time(NULL));
    food_pos.x = rand() % width;
    food_pos.y = rand() % height;
}

