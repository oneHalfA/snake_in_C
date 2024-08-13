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

struct coord{
    int x;
    int y;
}current_pos,food_pos;
// =========================================================================
void clear_screen(void);
int read_key(void);
void hide_cursor(void);
void right(struct coord,int);
void left(struct coord,int);
void up(struct coord,int);
void down(struct coord,int);
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
    struct coord changed_dir;
    snake.length = 8;
    snake.mem = malloc(snake.length);
    snake.mem[1] = '*';
    snake.mem[2] = '*';
    snake.mem[3] = '*';
    snake.mem[4] = '*';
    snake.mem[5] = '*';
    snake.mem[6] = '*';
    snake.mem[7] = '\0';

    int chr,
	key_pressed = LEFT, // default is RIGHT
	pre_command;

    clear_screen();
    printf("PRESS `q` TO EXIT\nMOVE BY ← ↑ → ↓\n");
    usleep(1000000);
    hide_cursor();

    while(1){
	chr = read_key();
	
	if(chr != EOF && (chr == RIGHT || chr == LEFT || chr == DOWN || chr == UP )){
	    pre_command = key_pressed;
	    changed_dir.x = current_pos.x;	changed_dir.y = current_pos.y;
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
		right(changed_dir,pre_command);
		break;
	    case LEFT:
		left(changed_dir,pre_command);
		break;
	    case UP:
		up(changed_dir,pre_command);
		break;
	    case DOWN:
		down(changed_dir,pre_command);
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

void right(struct coord changed_dir,int pre_command){
    current_pos.x++;
    
    int round = current_pos.x - changed_dir.x;
    int i=0;
    
    snake.mem[0] = '>';
    
    /*for(int j=0, x = current_pos.x, y = current_pos.y ; snake.mem[j] ; x--,j++){
	printf("\033[%d;%dH%c", y, x, snake.mem[j]);
	fflush(stdout);
    }
    usleep(25000);*/

    switch(pre_command){
	case UP:
	    for(int x = current_pos.x; i < round; x--,i++){
		printf("\033[%d;%dH%c", current_pos.y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int x = changed_dir.x, y = changed_dir.y ; snake.mem[i];y++,i++){
		printf("\033[%d;%dH%c",y,x,snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;

	case DOWN:
	    for(int x = current_pos.x; i < round; x--,i++){
		printf("\033[%d;%dH%c",current_pos.y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int x =changed_dir.x, y=changed_dir.y;snake.mem[i];y--,i++){
		printf("\033[%d;%dH%c",y,x,snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;

	default:
	    i = 0;
	    for(int x = current_pos.x; snake.mem[i]; x--,i++){
		printf("\033[%d;%dH%c", current_pos.y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;
    }

}

void left(struct coord changed_dir,int pre_command){
    current_pos.x--;

    int round = changed_dir.x - current_pos.x;
    int i=0;

    snake.mem[0] = '<';
    switch(pre_command){
	case UP:
	    for(int x = current_pos.x;i<round;i++,x++){
		printf("\033[%d;%dH%c", current_pos.y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int x = changed_dir.x, y = changed_dir.y ; snake.mem[i] ;y++,i++){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;
	case DOWN:
	    for(int x = current_pos.x;i<round;i++,x++){
		printf("\033[%d;%dH%c", current_pos.y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int x = changed_dir.x, y = changed_dir.y ; snake.mem[i] ;y--,i++){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;
	default:
	    printf("\033[%d;%dH", current_pos.y, current_pos.x);
	    for(i=0;snake.mem[i];i++){
		printf("%c",snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;
    }
}

void down(struct coord changed_dir,int pre_command){
    current_pos.y++;
    int round = current_pos.y - changed_dir.y;
    int i = 0;
    snake.mem[0] = 'v';
    switch(pre_command){
	case RIGHT:
	    for(int y=current_pos.y; i<round ;i++,y--){
		printf("\033[%d;%dH%c", y, current_pos.x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int y = changed_dir.y, x = changed_dir.x; snake.mem[i] ;i++,x--){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;
	case LEFT:
	    for(int y=current_pos.y; i<round ;i++,y--){
		printf("\033[%d;%dH%c", y, current_pos.x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int y=changed_dir.y,x=changed_dir.x; snake.mem[i] ;i++,x++){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;
    }
}

void up(struct coord changed_dir,int pre_command){
    current_pos.y--;
    int round = changed_dir.y - current_pos.y;
    int i=0;
    snake.mem[0] = '^';
    switch(pre_command){
	case RIGHT:
	    for(int y = current_pos.y, x=current_pos.x;i<round;i++,y++){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int y = changed_dir.y, x = changed_dir.x ;snake.mem[i];i++,x--){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;

	case LEFT:
	    for(int y = current_pos.y, x = current_pos.x;i<round;i++,y++){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    for(int y = changed_dir.y, x = changed_dir.x ;snake.mem[i];i++,x++){
		printf("\033[%d;%dH%c", y, x, snake.mem[i]);
		fflush(stdout);
	    }
	    usleep(95000);
	    break;
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

