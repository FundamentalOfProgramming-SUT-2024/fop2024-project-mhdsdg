//necessary libraries
#include<ncurses.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>

//functions
//declaring just in case
int check_user(int row ,int col) ;
//welcome 
void welcome_page(int row , int col){
    mvprintw(row/2 , col/2 - 2, "rogue");
    char msg[100] = "by Mahdi Sadeghi";
    refresh();
    sleep(1);
    mvprintw(row/2 +1 , (col-strlen(msg))/2 , "by Mahdi Sadeghi");
    refresh();
    sleep(2);
    char msg2[100] = "press any key to continue";
    mvprintw(row/2 +2 , (col-strlen(msg2))/2 , "press any key to continue");
    refresh();
    sleep(1);
    noecho();
    char c ;
    c = getch();
    refresh();
    if(c != ERR)clear() ;
    return ;
}

// choices 
void display_options(WINDOW *win, const char *options[], int option_count, int highlighted) {  
    for (int i = 0; i < option_count; i++) {  
        if (i == highlighted) {  
            wattron(win, A_REVERSE);  // Highlight the selected option  
            mvwprintw(win, i + 1, 1, "%s", options[i]);  
            wattroff(win, A_REVERSE);  
        } else {  
            mvwprintw(win, i + 1, 1, "%s", options[i]);  
        }  
    }  
    wrefresh(win);  
} 

int multiple_choice(const char* options[] ,int option_count ,int highlighted ,WINDOW * win){
	int ch;  
    noecho();  
    cbreak(); 
    keypad(stdscr, TRUE);  
  
    box(win, 0, 0);  
    refresh();  

    display_options(win, options, option_count, highlighted);  
    int flag =0 ;
    while ((ch = getch()) != 'q') {
        switch (ch) {  
            case KEY_UP:  
                highlighted = (highlighted == 0) ? option_count - 1 : highlighted - 1;  
                break;  
            case KEY_DOWN:  
                highlighted = (highlighted + 1) % option_count;  
                break;  
            case '\n': 
                flag =1;
                wrefresh(win);  
                break;  
        }  
        display_options(win, options, option_count, highlighted); 
        if(flag)break ;
    }  
    return highlighted;  
}

//login/signup
int is_username_available(const char* username) {  
    char file_name[100];
    sprintf(file_name , "%s.txt",username);
    FILE *file = fopen(file_name, "r"); 
    if(file == NULL){return 1 ;}//user available (not found existing)
    else return 0 ;//user exists

    fclose(file);  
} 

int is_password_correct(const char* username , const char* password){
    char file_name[100];
    char pass[100]; //saved password
    sprintf(file_name , "%s.txt",username);
    FILE *file = fopen(file_name, "r");
    fscanf(file , "%100[^\n]" , pass);
    fclose(file);
    if(strcmp(pass , password) == 0)return 1 ; //correct!
    else return 0 ; //incorrect
}

int check_email(char email[50]) {
    int atsign_pos = 0; int dot_pos = 0;
    for(int i=0; i<strlen(email); i++) {
        if(email[i] == '@') {
            atsign_pos = i;
        }
        else if(email[i] =='.') {
            dot_pos = i;
        }
    }
    if(atsign_pos>=3 && dot_pos-atsign_pos>=2 && strlen(email)-dot_pos==4) {
        return 1;
    }
    return 0;
}

int create_user(int row , int col){
    WINDOW * win = newwin(row/2 , col/2 , row/4 , col/4);
    box(win , 0 , 0);
    refresh();
    mvwprintw(win ,row/2 - 1 ,1 , "ESC to exit signup menu");
    mvwprintw(win , 1 ,1 , "enter your email and press enter : ");
    wrefresh(win);
    char email[50];
    int email_length = 0 , ch = 0;
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);
    while(ch != '\n' && email_length < 50){
        ch = getch();
        if(ch == 27){
            delwin(win);
            clear();
            return check_user(row , col);
        }
        if(ch >= 32 && ch <= 126){
        email[email_length++] = ch ;
        email[email_length] = '\0' ;
        mvwprintw(win ,2 , 2 , email);
        wmove(win ,2 , 2+email_length);
        }
        if(ch == KEY_BACKSPACE){
            if(email_length > 0){
                email_length-- ;
                email[email_length] = '\0';
                mvwaddch(win,2 , 2+email_length , ' ');
                wmove(win,2 , 2+email_length);
            }
        }
        wrefresh(win);
    }
    email[email_length] = '\0';
    if(!check_email(email)){
        mvwprintw(win , 4 , 2 , "email is not valid");
        wrefresh(win);
        sleep(3);
        delwin(win);
        return create_user(row , col);
    }

    mvwprintw(win , 3 ,1 , "enter your username and press enter : ");
    wrefresh(win);
    char username[100];
    int user_length = 0 ;
    ch = 0;
    while(ch != '\n' && user_length < 100){
        ch = getch();
        if(ch == 27){
            delwin(win);
            clear();
            return check_user(row , col) ;
        }
        if(ch >= 32 && ch <= 126){
            username[user_length++] = ch ;
            username[user_length] = '\0' ;
            mvwprintw(win ,4 , 2 , username);
            wmove(win ,4 , 2+user_length);
        }
        if(ch == KEY_BACKSPACE){
            if(user_length > 0){
                user_length-- ;
                username[user_length] = '\0';
                mvwaddch(win,4 , 2+user_length , ' ');
                wmove(win,4 , 2+user_length);
            }
        }
        wrefresh(win);
    }
    username[user_length] = '\0';
    wrefresh(win);


    mvwprintw(win , 5 ,1 , "enter your password and press enter : ");
    wrefresh(win);
    char password[100];
    int pass_length = 0 ;
    ch = 0;
        while(ch != '\n' && pass_length < 100){
        ch = getch();
        if(ch == 27){
            delwin(win);
            clear();
            return check_user(row , col);
        }
        if(ch >= 32 && ch <= 126){
            password[pass_length++] = ch ;
            password[pass_length] = '\0' ;
            mvwprintw(win , 6 , 2, password);
            wmove(win , 6 , 2+pass_length);
        }
        if(ch == KEY_BACKSPACE){
            if(pass_length > 0){
                pass_length-- ;
                password[pass_length] = '\0';
                mvwaddch(win , 6 , 2+pass_length , ' ');
                wmove(win , 6 , 2+pass_length);
            }
        }
        wrefresh(win);
        }
    password[pass_length] = '\0';
    char text[104] ;
    sprintf(text , "%s.txt" , username);
    FILE* file = fopen(text , "r");
    if(file){
        mvwprintw(win , 6 , 1 , "user already exists");
        wrefresh(win);
        sleep(2);
        delwin(win);
       return create_user(row , col);
    }
    int pass_error_flag = 0 , numflag = 0 , upperflag = 0 , lowerflag = 0;
    if(pass_length < 7){
        mvwprintw(win , 8 , 1 , "password should be at least 7 characters long");
        wrefresh(win);
        pass_error_flag++ ;
    }
    for(int i = 0 ; i < pass_length ; i++){
        if( password[i] <= '9' && password[i] >= '0' && numflag == 0)numflag =1;
        if( password[i] <= 'z' && password[i] >= 'a' && lowerflag == 0)lowerflag=1;
        if( password[i] <= 'Z' && password[i] >= 'A' && upperflag == 0)upperflag=1;
    }
    if(!numflag){
        mvwprintw(win , 8+pass_error_flag , 1 , "password should contain atleast one lowercase");
        wrefresh(win);
        pass_error_flag++;
    }
    if(!lowerflag){
        mvwprintw(win , 8+pass_error_flag , 1 , "password should contain atleast one number");
        wrefresh(win);
        pass_error_flag++;
    }
    if(!upperflag){
        mvwprintw(win , 8+pass_error_flag , 1 , "password should contain atleast one uppercase");
        wrefresh(win);
        pass_error_flag++;
    }
    if(pass_error_flag){
        sleep(3);
        delwin(win);
        return create_user(row , col);
    }
    file = fopen(text , "w");
    fprintf(file , "%s\n" , password);
    fclose(file);
    mvwprintw(win , 8 , 1 , "sign up successful please login again");
    wrefresh(win);
    sleep(4);
    clear();
    delwin(win);
    return check_user(row , col);
}

int check_user(int row ,int col){
    char username[100];
    char password[100];
    int password_length; 
    mvprintw(row/5 , col/3 , "Press ESC to login as guest");
    attron(A_BOLD);
    char msg[100] = "enter your username";
    attroff (A_BOLD);
    mvprintw(row/3 + 3 , (col - strlen(msg))/2 , "enter your username");
    mvprintw(row/2 , col/3 , "your username: ");
    refresh();
    noecho();
    
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);
    
    int user_length = 0 , ch;
    while(ch != '\n' && user_length < 100){
        ch = getch();
        if(ch == 27){
            clear();
            refresh();
            return 0 ;
        }
        if(ch >= 32 && ch <= 126){
            username[user_length++] = ch ;
            username[user_length] = '\0' ;
            mvprintw(row/2 , col/3 + 15, username);
            move(row/2 , col/3 + 15 + user_length);
        }
        if(ch == KEY_BACKSPACE){
            if(user_length > 0){
                user_length-- ;
                username[user_length] = '\0';
                mvaddch(row/2 , col/3 + 15 + user_length , ' ');
                move(row/2 , col/3 +15 + user_length);
            }
        }
    }
    username[user_length] = '\0';
    refresh();
    
    if(user_length < 3){
        mvprintw(row/2 + 3 , col/3 , "usernames must contain at least 3 characters");
        refresh();
        sleep(4);
        clear();
        return check_user(row , col);
    }
    //check if user exists 
    //if yes do password thing
    if(is_username_available(username) == 0){
        attron(A_BOLD);
        mvprintw(row/2 + 3 , col/3 , "enter your password (noecho): ");
        attroff(A_BOLD);
        int ch , pass_length = 0 ;
        while(ch != '\n' && pass_length < 100){
        ch = getch();
        if(ch == 27){
            clear();
            refresh();
            return 0 ;
        }
        if(ch >= 32 && ch <= 126){
            password[pass_length++] = ch ;
            password[pass_length] = '\0' ;
            // mvprintw(row/2 , col/3 + 30, password);
            //move(row/2 , col/3 + 30 + pass_length);
        }
        if(ch == KEY_BACKSPACE){
            if(pass_length > 0){
                pass_length-- ;
                password[pass_length] = '\0';
                // mvaddch(row/2 , col/3 + 30 + pass_length , ' ');
                //move(row/2 , col/3 +30 + pass_length);
            }
        }
        }
        password[pass_length] = '\0';


        if(is_password_correct(username , password)){
            return 1 ;
        }
        else{
            mvprintw(row/2 + 5 , col/3 , "password incorrect , try again.");
            refresh();
            sleep(4);
            clear();
            return check_user(row , col);
        }
    }
    //if not do : 
    
    if(is_username_available(username) == 1){
        mvprintw(row/2 + 3 , col/3 , "It seems like you've never played here");
        mvprintw(row/2 + 4 , col/3 , "would you like to sign up or try a different username ?");
        refresh();
        const char* options[2] = {"sign up" , "try again"};
        WINDOW * win = newwin(7 , 30 , 4 ,4);
        int t = multiple_choice(options , 2 , 0 , win);
        delwin(win);
        refresh();
        clear();
        if(t){return check_user(row, col);}
        else{return create_user(row , col);}
    }

    return 1;//login successful
}

void show_login_message(int m ,int row ,int col){
    if(m){
        mvprintw(row/2 , col/2 - 10 , "login was successful");
        refresh();
        sleep(2);
        clear();
        return ;
    }
    mvprintw(row/2 , col/2 -7, "Be my guest!");  
    refresh();
    sleep(2);
    clear();
    return;
}
//menus
void Main_menu(WINDOW* win){
    const char * options[5] = {"New game" , "Continue last saved game" , "Scores" , "Settings" , "Log out"};
    int t = multiple_choice(options , 5 , 0 , win);
}

//map

//main
int main(){
    initscr();
    cbreak() ;
    int row , col;
    getmaxyx(stdscr , row , col);
    welcome_page(row , col);
    int logged_in = check_user(row , col);
    clear();
    show_login_message(logged_in , row , col);

    WINDOW * main_menu = newwin(7 , 30 , row/2-4 ,col/2-15 );
    Main_menu(main_menu);

    endwin();
    return 0 ;
}
