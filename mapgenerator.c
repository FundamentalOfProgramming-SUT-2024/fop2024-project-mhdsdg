#include<ncursesw/ncurses.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<math.h>
#include<wchar.h> 
#include<locale.h>
#include<signal.h>
#include <sys/select.h>  
#include <termios.h>  
#include <fcntl.h> 

struct point {  
    int y;  
    int x;  
} point;

typedef struct room {
    int x ; 
    int y ;
    int height ;
    int width ;
    int door_count ;
    struct point * doors ;
    char door_types[10][2];
    struct point key ; //for generating a new password
    int password1;
    int password2;
    int marked ;  //for checking if it was visited in game
    int accessed ;//for drawing pathways
    int filled[30][30]; // to see if a block is filled with an item ;
    struct point * traps ;
    int trap_count;
    int trap_status[10] ; // to see if it's triggered or found 
    struct point * pillars ;
    int pillar_count;
    struct point * foods ;
    int foods_count ;
    struct point * gold ;
    int gold_count;
    struct point * black_gold ;
    int black_gold_count ;

    // monster ** monsters ;
}room ;

//foods : 0 => regular , 1 => super , 2 => magical , 3 => bad; 
//weapons : 0 => mace  , 1 => dagger , 2 => magic_wand , 3 => arrow , 4 => sword;
//potions : 0 => health , 1 => speed , 2 => harm;

struct hero{
    int max_health; 
    int health; 
    int max_hunger;
    int hunger;
    struct point loc;
    int gold_count ;
    int bgold_count;
    int food_count[4];
    int weapon_count[5];
    int potion_count[3];
    int total_moves;
}hero;

void initialize_hero(){
    hero.max_health = 40;
    hero.health = 40;
    hero.max_hunger = 20;
    hero.hunger = 20;
    hero.gold_count = 0;
    hero.bgold_count = 0;
    for(int i = 0 ; i < 4; i++){
        hero.food_count[i] =0;
    }
    for(int i = 0 ; i < 5 ; i++){
        hero.weapon_count[i] = 0;
    }
    for(int i = 0 ; i < 3 ; i++){
        hero.potion_count[i] = 0;
    }
    hero.total_moves = 0;
}

room* create_hallways(WINDOW * win , room* rooms , int room_count);
void doors(room* rooms,int room_count , WINDOW * win);
void traps_and_pillars(room* rooms , int room_count , WINDOW* win);
void place_hero(WINDOW* win,room room1 ,int y , int x);
int row , col ;

int interrupted = 0;  

void handle_signal(int signal) {  
    interrupted = 1;  
}   

void delete_partly(WINDOW* win , int lines , int cols , int y , int x){
    for(int i = 0 ; i < lines ; i++){
        for(int j = 0 ; j < cols ; j++){
            mvwaddch(win , y + i - 1 , x + j - 1, ' ');
        }
    }
}

int kbhit() {  
    struct timeval tv = { 0, 0 };  
    fd_set readfds;  
    FD_ZERO(&readfds);  
    FD_SET(STDIN_FILENO, &readfds);  
    return select(1, &readfds, NULL, NULL, &tv);  
}  

struct point pick_a_random_point(room room){
    int xx = rand()%(room.width - 2) + 1;
    int yy = rand()%(room.height - 2) + 1;
    if(room.filled[yy][xx] == 1)return pick_a_random_point(room);
    struct point random ;
    random.x = xx + room.x ;
    random.y = yy + room.y ;
    return random;
}
void make_room(room room,WINDOW * win){
    for(int i = room.x ; i < room.x + room.width ; i++){
        mvwaddch(win , room.y , i , '-');
        mvwaddch(win , room.y + room.height - 1 , i , '-');
        for(int j = room.y+1 ; j < room.y+room.height-1 ; j++){
            if(i == room.x || i == room.x+room.width-1){
                mvwaddch(win ,j , i , '|');
            }
            else{
                mvwaddch(win ,j , i , '.');
            }
        }
    }
}

int rooms_overlap(room new_room, room *existing_rooms, int existing_room_count) {  
    for (int i = 0; i < existing_room_count; i++) {  
        room existing_room = existing_rooms[i];  
  
        if (new_room.x < existing_room.x + existing_room.width + 9 &&  
            new_room.x + new_room.width + 9 > existing_room.x &&  
            new_room.y < existing_room.y + existing_room.height + 9 &&  
            new_room.y + new_room.height + 9 > existing_room.y) {  
            return 1; 
        }  
 
        if (new_room.x <= existing_room.x + existing_room.width &&  
            new_room.x + new_room.width >= existing_room.x + 9 &&  
            new_room.y <= existing_room.y + existing_room.height &&  
            new_room.y + new_room.height >= existing_room.y + 9) {  
            return 1;
        }  
    }  
    return 0;
} 
int room_count = 0;
room* create_random_rooms(WINDOW * win ,int row ,int col,int n){
    room_count = rand()%3 + 6 ;
    n = room_count ;
    room * rooms = (room *)malloc(room_count * sizeof(room));
    int valid_rooms = 0 ;
    int tries = 0 ;
    for(int i = 0 ; i < room_count ; i++){
    rooms[i].width  = rand()%13 + 9 ;
    rooms[i].height = rand()%9 + 9 ;
    rooms[i].y = rand()%(row/2) + row/10 ;
    rooms[i].x = rand()%(3*col/6) + col/4 ;
    rooms[i].marked = 0 ;
    rooms[i].accessed = 0;
    rooms[i].door_count = 0 ;
    rooms[i].doors = malloc(10 * sizeof(struct point));
    rooms[i].traps = malloc(5 * sizeof(struct point));
    rooms[i].pillars = malloc(10 * sizeof(struct point));
    rooms[i].key.y = -1;
    if (!rooms_overlap(rooms[i], rooms, valid_rooms)) {  
            make_room(rooms[i], win);  
            valid_rooms++;  
        } else {  
            i--;  
        }
    tries++;
    if(tries>100){
        wclear(win);
        return create_random_rooms(win , row , col , n);
    }
    }

    return create_hallways(win , rooms , room_count) ;
}

struct point* pathway ;
int path_index = 0 ; 

int compare_rooms_by_x(const void *a, const void *b) {  
    const room *roomA = (const room *)a;  
    const room *roomB = (const room *)b;  
 
    return (roomA->x - roomB->x);  
}  

void sort_rooms_by_x(room *rooms, int room_count) {  
    qsort(rooms, room_count, sizeof(room), compare_rooms_by_x);  
}

void which_room(struct point door , room* rooms , int room_count){
    for(int i = 0 ; i < room_count ; i++){
        if(door.x >= rooms[i].x && door.x <= rooms[i].x+rooms[i].width-1 &&
           door.y >= rooms[i].y && door.y <= rooms[i].y+rooms[i].height-1){
            rooms[i].accessed = 1 ;
            rooms[i].doors[rooms[i].door_count++] = door;
           }
    }
}

int which(int y , int x , room* rooms , int room_count){
    for(int i = 0 ; i < room_count ; i++){
        if(x >= rooms[i].x && x <= rooms[i].x+rooms[i].width-1 &&
           y >= rooms[i].y && y <= rooms[i].y+rooms[i].height-1){
            return i;
           }
    }
    return -1;
}

room* create_hallways(WINDOW* win , room* rooms , int room_count){
    sort_rooms_by_x(rooms , room_count);
     
    for (int i = 0; i < room_count - 1; i++) {  
        if(rooms[i+1].accessed == 1){continue;}
 
        int room1_center_y = rooms[i].y + rooms[i].height / 2;  
        int room1_center_x = rooms[i].x + rooms[i].width / 2;  
 
        int room2_center_y = rooms[i + 1].y + rooms[i + 1].height / 2;  
        int room2_center_x = rooms[i + 1].x + rooms[i + 1].width / 2;  
 
        if (room1_center_x <= room2_center_x) { 
            if(room2_center_x == rooms[i].x + rooms[i].width - 1){
                room2_center_x--;
               } 
            if(room2_center_x == rooms[i+2].x){
                room2_center_x--;
            }
            
            for (int x = room1_center_x; x <= room2_center_x; x++) { 
                char c = mvwinch(win, room1_center_y, x) ;
                if(c == ' ') {
                mvwaddch(win, room1_center_y, x, '#'); }
                if((c == '-' || c == '|')){
                    if(mvwinch(win , room1_center_y , x+1) != '.' && mvwinch(win , room1_center_y , x+1) != ' '){
                        if(room2_center_y > room1_center_y){
                            room1_center_y++;
                            mvwaddch(win, room1_center_y, x-1, '#');
                        }
                        else{
                            room1_center_y--;
                            mvwaddch(win, room1_center_y, x-1, '#');
                        }
                    }
                if(mvwinch(win , room1_center_y , x) == '-' || mvwinch(win , room1_center_y , x) == '|'){
                mvwaddch(win , room1_center_y , x , '+');
                struct point door ;
                door.y = room1_center_y ;
                door.x = x ;
                which_room(door , rooms , room_count);
                }
                else{mvwaddch(win , room1_center_y , x , '#');}
                }
            }  
        } else {  
            for (int x = room2_center_x; x <= room1_center_x; x++) { 
                char c = mvwinch(win, room1_center_y, x) ;
                if(mvwinch(win, room1_center_y, x) == ' ') { 
                mvwaddch(win, room1_center_y, x, '#'); } 
                if((c== '-' || c== '|')){
                mvwaddch(win, room1_center_y, x, '+');
                }
            }  
        }  

        if (room1_center_y < room2_center_y) {  
            for (int y = room1_center_y; y <= room2_center_y; y++) {  
                char c = mvwinch(win, y, room2_center_x) ;
                if(c == ' ') {
                mvwaddch(win, y, room2_center_x, '#');  }
                if((c== '-' || c== '|')){
                mvwaddch(win, y, room2_center_x, '+');  
                struct point door ;
                door.x = room2_center_x ;
                door.y = y ;
                which_room(door , rooms , room_count);
                }
            }  
        } else {  
            for (int y = room2_center_y; y <= room1_center_y; y++) {  
                char c = mvwinch(win, y, room2_center_x) ;
                if(c == ' ') {
                mvwaddch(win, y, room2_center_x, '#'); } 
                if((c== '-' || c== '|')){
                mvwaddch(win, y, room2_center_x, '+');
                struct point door ;
                door.x = room2_center_x ;
                door.y = y ;
                which_room(door , rooms , room_count);
                }
            }  
        }  
    }  
    doors(rooms , room_count , win);
    traps_and_pillars(rooms , room_count , win);
    initialize_hero();
    place_hero(win , rooms[0] , -1 , -1);
    return rooms; 
}

void doors(room* rooms,int room_count , WINDOW * win){  // exiting doors have 33 percent chance of being encrypted
    for(int i = 0 ; i < room_count ; i++){              // doors have 30 percent chance of being hidden
        for(int j = 0 ; j < rooms[i].door_count ; j++){
            int x = rooms[i].doors[j].x ;
            int y = rooms[i].doors[j].y ;
            if(rand()%10 > 7){
                rooms[i].door_types[j][0] = 'h';
                if(y == rooms[i].y || y == rooms[i].y -1 + rooms[i].height){
                    mvwaddch(win , y , x , '-');
                }
                else{
                    mvwaddch(win , y , x , '|');
                }
                continue;
            }
            int flag = 0 ;
            if(j>0 && rand()/3 > 1){
                rooms[i].door_types[j][0] = 'e';
                wattron(win , COLOR_PAIR(1));
                mvwaddch(win , y , x , '@');
                wattroff(win , COLOR_PAIR(1));
                if(flag == 0){
                int t = rand()%4 + 1 ;
                int x1 , y1 ;
                switch (t)
                {
                case 1:
                    x1 = rooms[i].x + 1;
                    y1 = rooms[i].y + 1;
                    break;
                case 2:
                    x1 = rooms[i].x + rooms[i].width -2 ;
                    y1 = rooms[i].y + rooms[i].height - 2;
                    break;
                case 3:
                    x1 = rooms[i].x + 1;
                    y1 = rooms[i].y + rooms[i].height - 2;
                    break;
                case 4:
                    x1 = rooms[i].x + rooms[i].width - 2;
                    y1 = rooms[i].y + 1;
                    break;
                }
                if(rooms[i].key.y == -1){
                    rooms[i].key.x = x1;
                    rooms[i].key.y = y1; 
                    rooms[i].filled[y1 - rooms[i].y][x1 - rooms[i].x] = 1;
                    mvwaddch(win , y1 , x1 , '&');
                }
                }
                flag = 1 ;
                continue;
            }
            rooms[i].door_types[j][0] = 'r';
        }
    }
    wrefresh(win);
}

void traps_and_pillars(room* rooms , int room_count , WINDOW* win){
    for(int i = 0 ; i < room_count ; i++){
        if(rooms[i].width > 10 && rooms[i].height > 10){//bigger rooms may have more traps and pillars;
            rooms[i].trap_count = rand()%2 + 1;
            rooms[i].pillar_count = rand()%3 + 1;
        }
        else{
            rooms[i].trap_count = rand()%1;
            rooms[i].pillar_count = rand()%1;
        }
        for(int j = 0 ; j < rooms[i].trap_count ; j++){
            int x = rand()%(rooms[i].width-2) + 1 ;
            int y = rand()%(rooms[i].height-2) + 1 ;
            if(rooms[i].filled[y][x] == 1){
                j++;
                continue;
            }
            rooms[i].traps[j].x = rooms[i].x + x;
            rooms[i].traps[j].y = rooms[i].y + y;
            rooms[i].trap_status[j] = 0;
            rooms[i].filled[y][x] = 1;
            mvwaddch(win , rooms[i].y + y , rooms[i].x + x , '^');
        }
        for(int j = 0 ; j < rooms[i].pillar_count ; j++){
            int x = rand()%(rooms[i].width-4) + 2 ;
            int y = rand()%(rooms[i].height-4) + 2 ;
            if(rooms[i].filled[y][x] == 1){
                j++;
                continue;
            } 
            rooms[i].pillars[j].x = rooms[i].x + x;
            rooms[i].pillars[j].y = rooms[i].y + y;
            rooms[i].filled[y][x] = 1;
            mvwaddch(win , rooms[i].y + y , rooms[i].x + x , 'O');
        }
    }
}

void bless_african(room* rooms , int room_count , WINDOW* win){

}

//funcitons for gameplay
void place_hero(WINDOW* win,room room1 ,int y , int x){
    if(y == -1){//random place
        struct point rand = pick_a_random_point(room1);
        hero.loc.y = rand.y;
        hero.loc.x = rand.x;
        mvwprintw(win , rand.y , rand.x , "H");
    }
    else {
        hero.loc.y = y;
        hero.loc.x = x;
        mvwprintw(win , y , x , "H");
    }
}

void mov(WINDOW* win , int keypressed , int * pre , room* rooms){
    int j , i ;
    switch(keypressed - 48){
        case 1 : 
        i = -1 ; j = 1 ; break ;
        case 2 : 
        i = 0  ; j = 1 ; break ;
        case 3 : 
        i = 1  ; j = 1 ; break ;
        case 4 : 
        i = -1 ; j = 0  ; break ;
        case 6 :
        i = 1  ; j = 0  ; break ;
        case 7 : 
        i = -1 ; j = -1  ; break ;
        case 8 :
        i = 0  ; j = -1  ; break ;
        case 9 : 
        i = 1  ; j = -1  ; break ;
        default :
        break;
    }
    int next_block = mvwinch(win , hero.loc.y + j , hero.loc.x + i);
    int w = which(hero.loc.y , hero.loc.x , rooms , room_count);  
    if(w == -1)w = which(hero.loc.y + j , hero.loc.x + i , rooms , room_count);     
    if(next_block == '.' || next_block == '#' || next_block == '+' || next_block == '?'){//basic movement not picking items nor heading through traps
        mvwaddch(win , hero.loc.y , hero.loc.x , *pre);
        *pre = next_block;
        hero.loc.y += j ; hero.loc.x += i ;
        mvwaddch(win , hero.loc.y , hero.loc.x , 'H');
    }
    else if(next_block == '&'){//password generation
        mvwaddch(win , hero.loc.y , hero.loc.x , *pre);
        *pre = next_block;
        hero.loc.y += j ; hero.loc.x += i ;
        mvwaddch(win , hero.loc.y , hero.loc.x , 'H');
        wrefresh(win);
        int password = rand()%9000 + 1000 ;
        rooms[w].password1 = password;
        WINDOW* pass = newwin(4 , 8 , 2, col - 10);
        box(pass , 0 , 0);
        wrefresh(pass);
        mvwprintw(pass , 1 , 2 , "code" );
        mvwprintw(pass , 2 , 2 ,"%d" , rooms[w].password1);
        wrefresh(pass);
        signal(SIGINT, handle_signal);
        for (int i = 0; i < 10; i++) {  
        if (kbhit()) {  
            char ch = getchar();  
            if (ch) {    
                interrupted = 1;  
                break;  
            }  
        }  
        sleep(1);  
        }
        wrefresh(win);
        delete_partly(win , 4 ,8 ,2 , col - 10);
        delwin(pass);
    }
    else if(next_block == '-' || next_block == '|'){//hidden doors
        for(int m = 0 ; m < rooms[w].door_count ; m++){
            if(rooms[w].door_types[m][0] == 'h' && rooms[w].doors[m].y == hero.loc.y+j && rooms[w].doors[m].x == hero.loc.x+i){
                mvwaddch(win , hero.loc.y , hero.loc.x , *pre);
                *pre = '?';
                hero.loc.y += j ; hero.loc.x += i ;
                mvwaddch(win , hero.loc.y , hero.loc.x , 'H');
                wrefresh(win);
            }
        }
    }
    else if(next_block == '@'){
        
    }

    wrefresh(win);
}

int main(){
    srand(time(NULL)); 
    initscr();
    
    getmaxyx(stdscr , row , col);
    start_color();
    setlocale(LC_CTYPE, "");
    init_pair(1 , COLOR_RED ,COLOR_BLACK );
    
    WINDOW * gamewin = newwin(row - 2 , col - 2 , 1 , 1);
    box(gamewin , 0 , 0);
    refresh();
    int n=0 ;
    room * rooms = create_random_rooms(gamewin , row , col , n);
    keypad(stdscr, TRUE); 
    nodelay(stdscr, TRUE);
    noecho();
    // curs_set(0);
    wrefresh(gamewin);
    int in = 0;
    int pre = '.';
    while(1){
        in = getch();
        if(in >= '1' && in <= '9' && in != '5'){
            mov(gamewin , in , &pre , rooms);

            continue;
        }
        if(in == 27){
            break;
        }
    }
    endwin();
    return 0 ;
}