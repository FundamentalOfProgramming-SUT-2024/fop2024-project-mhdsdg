// necessary headers
#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <signal.h>
#include <sys/select.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
int row, col;
int difficulty = 0;
int hero_color = 14;
int speed_flag = -20;
int regen_flag = -20;
int damage_flag = -20;
int logged_in = 0;
int speed_boost = 1;
int has_saved;
Mix_Music *prev;
Mix_Music *prev;
Mix_Music *m1;
Mix_Music *m2;
Mix_Music *m3;
Mix_Music *l1;
Mix_Music *l2;
Mix_Music *l3;
Mix_Music *l4;
Mix_Music *lt;
// functions
// declaring just in case
char address[100];
char user[100];
int check_user(int row, int col);
int Main_menu();
int load_game();
// welcome
void welcome_page(int row, int col)
{
    mvprintw(row / 2, col / 2 - 2, "rogue");
    char msg[100] = "by Mahdi Sadeghi";
    refresh();
    sleep(1);
    mvprintw(row / 2 + 1, (col - strlen(msg)) / 2, "by Mahdi Sadeghi");
    refresh();
    sleep(2);
    char msg2[100] = "press any key to continue";
    mvprintw(row / 2 + 2, (col - strlen(msg2)) / 2, "press any key to continue");
    refresh();
    sleep(1);
    noecho();
    char c;
    c = getch();
    refresh();
    if (c != ERR)
        clear();
    return;
}

// choices
void display_options(WINDOW *win, const char *options[], int option_count, int highlighted)
{
    for (int i = 0; i < option_count; i++)
    {
        if (i == highlighted)
        {
            wattron(win, A_REVERSE); // Highlight the selected option
            mvwprintw(win, i + 1, 1, "%s", options[i]);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwprintw(win, i + 1, 1, "%s", options[i]);
        }
    }
    wrefresh(win);
}

int multiple_choice(const char *options[], int option_count, int highlighted, WINDOW *win)
{
    int ch;
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    box(win, 0, 0);
    refresh();

    display_options(win, options, option_count, highlighted);
    int flag = 0;
    while ((ch = getch()) != 'q')
    {
        switch (ch)
        {
        case KEY_UP:
            highlighted = (highlighted == 0) ? option_count - 1 : highlighted - 1;
            break;
        case KEY_DOWN:
            highlighted = (highlighted + 1) % option_count;
            break;
        case '\n':
            flag = 1;
            wrefresh(win);
            break;
        }
        display_options(win, options, option_count, highlighted);
        if (flag)
            break;
    }
    return highlighted;
}

// login/signup
int is_username_available(const char *username)
{
    char file_name[100];
    sprintf(file_name, "./users/%s.txt", username);
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        return 1;
    } // user available (not found existing)
    strcpy(user, username);
    return 0; // user exists

    fclose(file);
}

int is_password_correct(const char *username, const char *password)
{
    char file_name[100];
    char pass[100]; // saved password
    sprintf(file_name, "./users/%s.txt", username);
    FILE *file = fopen(file_name, "r");
    fscanf(file, "%100[^\n]", pass);
    fclose(file);
    if (strcmp(pass, password) == 0)
        return 1; // correct!
    else
        return 0; // incorrect
}

int check_email(char email[50])
{
    int atsign_pos = 0;
    int dot_pos = 0;
    for (int i = 0; i < strlen(email); i++)
    {
        if (email[i] == '@')
        {
            atsign_pos = i;
        }
        else if (email[i] == '.')
        {
            dot_pos = i;
        }
    }
    if (atsign_pos >= 3 && dot_pos - atsign_pos >= 2 && strlen(email) - dot_pos == 4)
    {
        return 1;
    }
    return 0;
}

int create_user(int row, int col)
{
    WINDOW *win = newwin(row / 2, col / 2, row / 4, col / 4);
    box(win, 0, 0);
    refresh();
    mvwprintw(win, row / 2 - 1, 1, "ESC to exit signup menu");
    mvwprintw(win, 1, 1, "enter your email and press enter : ");
    wrefresh(win);
    char email[50];
    int email_length = 0, ch = 0;
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);
    while (ch != '\n' && email_length < 50)
    {
        ch = getch();
        if (ch == 27)
        {
            delwin(win);
            clear();
            return check_user(row, col);
        }
        if (ch >= 32 && ch <= 126)
        {
            email[email_length++] = ch;
            email[email_length] = '\0';
            mvwprintw(win, 2, 2, email);
            wmove(win, 2, 2 + email_length);
        }
        if (ch == KEY_BACKSPACE)
        {
            if (email_length > 0)
            {
                email_length--;
                email[email_length] = '\0';
                mvwaddch(win, 2, 2 + email_length, ' ');
                wmove(win, 2, 2 + email_length);
            }
        }
        wrefresh(win);
    }
    email[email_length] = '\0';
    if (!check_email(email))
    {
        mvwprintw(win, 4, 2, "email is not valid");
        wrefresh(win);
        sleep(3);
        delwin(win);
        return create_user(row, col);
    }

    mvwprintw(win, 3, 1, "enter your username and press enter : ");
    wrefresh(win);
    char username[100];
    int user_length = 0;
    ch = 0;
    while (ch != '\n' && user_length < 100)
    {
        ch = getch();
        if (ch == 27)
        {
            delwin(win);
            clear();
            return check_user(row, col);
        }
        if (ch >= 32 && ch <= 126)
        {
            username[user_length++] = ch;
            username[user_length] = '\0';
            mvwprintw(win, 4, 2, username);
            wmove(win, 4, 2 + user_length);
        }
        if (ch == KEY_BACKSPACE)
        {
            if (user_length > 0)
            {
                user_length--;
                username[user_length] = '\0';
                mvwaddch(win, 4, 2 + user_length, ' ');
                wmove(win, 4, 2 + user_length);
            }
        }
        wrefresh(win);
    }
    username[user_length] = '\0';
    wrefresh(win);

    mvwprintw(win, 5, 1, "enter your password and press enter : ");
    wrefresh(win);
    char password[100];
    int pass_length = 0;
    ch = 0;
    while (ch != '\n' && pass_length < 100)
    {
        ch = getch();
        if (ch == 27)
        {
            delwin(win);
            clear();
            return check_user(row, col);
        }
        if (ch >= 32 && ch <= 126)
        {
            password[pass_length++] = ch;
            password[pass_length] = '\0';
            mvwprintw(win, 6, 2, password);
            wmove(win, 6, 2 + pass_length);
        }
        if (ch == KEY_BACKSPACE)
        {
            if (pass_length > 0)
            {
                pass_length--;
                password[pass_length] = '\0';
                mvwaddch(win, 6, 2 + pass_length, ' ');
                wmove(win, 6, 2 + pass_length);
            }
        }
        wrefresh(win);
    }
    password[pass_length] = '\0';
    char text[104];
    sprintf(text, "./users/%s.txt", username);
    FILE *file = fopen(text, "r");
    if (file)
    {
        mvwprintw(win, 6, 1, "user already exists");
        wrefresh(win);
        sleep(2);
        delwin(win);
        return create_user(row, col);
    }
    int pass_error_flag = 0, numflag = 0, upperflag = 0, lowerflag = 0;
    if (pass_length < 7)
    {
        mvwprintw(win, 8, 1, "password should be at least 7 characters long");
        wrefresh(win);
        pass_error_flag++;
    }
    for (int i = 0; i < pass_length; i++)
    {
        if (password[i] <= '9' && password[i] >= '0' && numflag == 0)
            numflag = 1;
        if (password[i] <= 'z' && password[i] >= 'a' && lowerflag == 0)
            lowerflag = 1;
        if (password[i] <= 'Z' && password[i] >= 'A' && upperflag == 0)
            upperflag = 1;
    }
    if (!numflag)
    {
        mvwprintw(win, 8 + pass_error_flag, 1, "password should contain atleast one lowercase");
        wrefresh(win);
        pass_error_flag++;
    }
    if (!lowerflag)
    {
        mvwprintw(win, 8 + pass_error_flag, 1, "password should contain atleast one number");
        wrefresh(win);
        pass_error_flag++;
    }
    if (!upperflag)
    {
        mvwprintw(win, 8 + pass_error_flag, 1, "password should contain atleast one uppercase");
        wrefresh(win);
        pass_error_flag++;
    }
    if (pass_error_flag)
    {
        sleep(3);
        delwin(win);
        return create_user(row, col);
    }
    file = fopen(text, "w");
    fprintf(file, "%s\n", password);
    for (int i = 0; i < 22; i++)
    {
        fprintf(file, "0\n");
    }
    strcpy(user, username);
    fclose(file);
    mvwprintw(win, 8, 1, "sign up successful please login again");
    wrefresh(win);
    sleep(4);
    clear();
    delwin(win);
    return check_user(row, col);
}

int check_user(int row, int col)
{
    char username[100];
    char password[100];
    int password_length;
    mvprintw(row / 5, col / 3, "Press ESC to login as guest");
    attron(A_BOLD);
    char msg[100] = "enter your username";
    attroff(A_BOLD);
    mvprintw(row / 3 + 3, (col - strlen(msg)) / 2, "enter your username");
    mvprintw(row / 2, col / 3, "your username: ");
    refresh();
    noecho();

    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);

    int user_length = 0, ch;
    while (ch != '\n' && user_length < 100)
    {
        ch = getch();
        if (ch == 27)
        {
            clear();
            refresh();
            return 0;
        }
        if (ch >= 32 && ch <= 126)
        {
            username[user_length++] = ch;
            username[user_length] = '\0';
            mvprintw(row / 2, col / 3 + 15, username);
            move(row / 2, col / 3 + 15 + user_length);
        }
        if (ch == KEY_BACKSPACE)
        {
            if (user_length > 0)
            {
                user_length--;
                username[user_length] = '\0';
                mvaddch(row / 2, col / 3 + 15 + user_length, ' ');
                move(row / 2, col / 3 + 15 + user_length);
            }
        }
    }
    username[user_length] = '\0';
    refresh();

    if (user_length < 3)
    {
        mvprintw(row / 2 + 3, col / 3, "usernames must contain at least 3 characters");
        refresh();
        sleep(4);
        clear();
        return check_user(row, col);
    }
    // check if user exists
    // if yes do password thing
    if (is_username_available(username) == 0)
    {
        attron(A_BOLD);
        mvprintw(row / 2 + 3, col / 3, "enter your password (noecho): ");
        attroff(A_BOLD);
        int ch, pass_length = 0;
        while (ch != '\n' && pass_length < 100)
        {
            ch = getch();
            if (ch == 27)
            {
                clear();
                refresh();
                return 0;
            }
            if (ch >= 32 && ch <= 126)
            {
                password[pass_length++] = ch;
                password[pass_length] = '\0';
            }
            if (ch == KEY_BACKSPACE)
            {
                if (pass_length > 0)
                {
                    pass_length--;
                    password[pass_length] = '\0';
                }
            }
        }
        password[pass_length] = '\0';

        if (is_password_correct(username, password))
        {
            return 1;
        }
        else
        {
            mvprintw(row / 2 + 5, col / 3, "password incorrect , try again.");
            refresh();
            sleep(4);
            clear();
            return check_user(row, col);
        }
    }
    // if not do :

    if (is_username_available(username) == 1)
    {
        mvprintw(row / 2 + 3, col / 3, "It seems like you've never played here");
        mvprintw(row / 2 + 4, col / 3, "would you like to sign up or try a different username ?");
        refresh();
        const char *options[2] = {"sign up", "try again"};
        WINDOW *win = newwin(7, 30, 4, 4);
        int t = multiple_choice(options, 2, 0, win);
        delwin(win);
        refresh();
        clear();
        if (t)
        {
            return check_user(row, col);
        }
        else
        {
            return create_user(row, col);
        }
    }

    return 1; // login successful
}

void show_login_message(int m, int row, int col)
{
    if (m)
    {
        mvprintw(row / 2, col / 2 - 10, "login was successful");
        refresh();
        sleep(2);
        clear();
        return;
    }
    mvprintw(row / 2, col / 2 - 7, "Be my guest!");
    refresh();
    sleep(2);
    clear();
    return;
}
// menus
int setting_menu();
typedef struct
{
    char filename[100];
    int value; // score
    int gold_count;
    int game_count;
    int exp;
} FileEntry;

int compare(const void *a, const void *b)
{
    FileEntry *fileA = (FileEntry *)a;
    FileEntry *fileB = (FileEntry *)b;
    return (-fileA->value + fileB->value);
}

FileEntry *sortFilesBySecondLineValue(const char *directoryPath, int *count, FileEntry **users)
{
    DIR *dir;
    struct dirent *entry;
    FileEntry files[200];
    int fileCount = 0;
    dir = opendir(directoryPath);

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        char filePath[100];
        snprintf(filePath, sizeof(filePath), "%s/%s", directoryPath, entry->d_name);

        FILE *file = fopen(filePath, "r");

        char line[256];
        int lineNumber = 0;
        while (fgets(line, sizeof(line), file) != NULL && lineNumber < 5)
        {
            if (lineNumber == 1)
            {
                files[fileCount].value = atoi(line);
                strncpy(files[fileCount].filename, entry->d_name, 99);
                files[fileCount].filename[99] = '\0';
            }
            if (lineNumber == 2)
            {
                files[fileCount].gold_count = atoi(line);
            }
            if (lineNumber == 3)
            {
                files[fileCount].game_count = atoi(line);
            }
            if (lineNumber == 4)
            {
                files[fileCount].exp = atoi(line);
                fileCount++;
            }
            lineNumber++;
        }

        fclose(file);
    }

    closedir(dir);

    qsort(files, fileCount, sizeof(FileEntry), compare);
    *count = fileCount;
    *users = files;
    return files;
}

int score_menu()
{
    WINDOW *win = newwin(30, 120, row / 2 - 15, col / 2 - 60);
    box(win, 0, 0);
    int file_count = 0;
    FileEntry *users = malloc(200 * sizeof(FileEntry));
    sortFilesBySecondLineValue("./users", &file_count, &users);
    mvwprintw(win, 2, 2, "%10s%20s%20s%20s%20s", "Name: ", "All-time score: ", "Total gold: ", "Games played: ", "EXP: ");
    for (int i = 0; i < 3; i++)
    {
        char mamad[100];
        int size = strlen(users[i].filename);
        strncpy(mamad, users[i].filename, size - 4);
        mamad[size - 4] = '\0';
        if (strcmp(mamad, user) == 0)
            wattron(win, A_REVERSE);
        if (i == 0)
            wattron(win, COLOR_PAIR(6));
        if (i == 1)
            wattron(win, COLOR_PAIR(10));
        if (i == 2)
            wattron(win, COLOR_PAIR(12));
        mvwprintw(win, 4 + 8 * i, 2, "%10s%20d%20d%20d%20d", mamad, users[i].value, users[i].gold_count, users[i].game_count, users[i].exp);
        if (i == 0)
            wattroff(win, COLOR_PAIR(6));
        if (i == 1)
            wattroff(win, COLOR_PAIR(10));
        if (i == 2)
            wattroff(win, COLOR_PAIR(12));
        if (strcmp(mamad, user) == 0)
            wattroff(win, A_REVERSE);
        chtype color;
        if (i == 0)
        {
            color = COLOR_PAIR(6);
            wattron(win, color);
            mvwprintw(win, 4 + 8 * i, 100, "  _________");
            mvwprintw(win, 5 + 8 * i, 100, " |         |");
            mvwprintw(win, 6 + 8 * i, 100, "(| G.O.A.T |)");
            mvwprintw(win, 7 + 8 * i, 100, " |   #1    |");
            mvwprintw(win, 8 + 8 * i, 100, "  \\       /");
            mvwprintw(win, 9 + 8 * i, 100, "   `-----'");
            mvwprintw(win, 10 + 8 * i, 100, "    _|_|_");
            wattroff(win, color);
        }
        if (i == 1)
        {
            color = COLOR_PAIR(10);
            wattron(win, color);
            mvwprintw(win, 4 + 8 * i, 100, "  _________");
            mvwprintw(win, 5 + 8 * i, 100, " |         |");
            mvwprintw(win, 6 + 8 * i, 100, "(| LEGEND  |)");
            mvwprintw(win, 7 + 8 * i, 100, " |   #2    |");
            mvwprintw(win, 8 + 8 * i, 100, "  \\       /");
            mvwprintw(win, 9 + 8 * i, 100, "   `-----'");
            mvwprintw(win, 10 + 8 * i, 100, "    _|_|_");
            wattroff(win, color);
        }
        if (i == 2)
        {
            color = COLOR_PAIR(12);
            wattron(win, color);
            mvwprintw(win, 4 + 8 * i, 100, "  _________");
            mvwprintw(win, 5 + 8 * i, 100, " |         |");
            mvwprintw(win, 6 + 8 * i, 100, "(| DIGGER  |)");
            mvwprintw(win, 7 + 8 * i, 100, " |   #3    |");
            mvwprintw(win, 8 + 8 * i, 100, "  \\       /");
            mvwprintw(win, 9 + 8 * i, 100, "   `-----'");
            mvwprintw(win, 10 + 8 * i, 100, "    _|_|_");
            wattroff(win, color);
        }
    }
    wrefresh(win);
    wgetch(win);
    wclear(win);
    box(win, 0, 0);
    for (int j = 0; j < 9; j++)
    {
        mvwprintw(win, 2, 2, "%10s%20s%20s%20s%20s", "Name: ", "All-time score: ", "Total gold: ", "Games played: ", "EXP: ");
        int i = 3 + 8 * j;
        for (i; i < 11 + 8 * j && i < file_count; i++)
        {
            char mamad[100];
            int size = strlen(users[i].filename);
            strncpy(mamad, users[i].filename, size - 4);
            mamad[size - 4] = '\0';
            if (strcmp(mamad, user) == 0)
                wattron(win, A_REVERSE);
            mvwprintw(win, 4 + 3 * (i - 3), 2, "%10s%20d%20d%20d%20d", mamad, users[i].value, users[i].gold_count, users[i].game_count, users[i].exp);
            if (strcmp(mamad, user) == 0)
                wattroff(win, A_REVERSE);
        }
        if (i >= file_count)
            break;
    }
    wrefresh(win);
    wgetch(win);
    wclear(win);
    wrefresh(win);
    delwin(win);
    return Main_menu();
}

int choose_color()
{
    WINDOW *win = newwin(9, 30, row / 2 - 4, col / 2 - 15);
    box(win, 0, 0);
    const char *options[5] = {"White", "Gold", "Green", "Blue", "Red"};
    int t = multiple_choice(options, 5, 0, win);
    switch (t)
    {
    case 0:
        hero_color = 14;
        break;
    case 1:
        hero_color = 6;
        break;
    case 2:
        hero_color = 2;
        break;
    case 3:
        hero_color = 5;
        break;
    case 4:
        hero_color = 1;
        break;
    }
    wclear(win);
    delwin(win);
    return setting_menu();
}

int difficulty_menu()
{
    WINDOW *win = newwin(9, 30, row / 2 - 4, col / 2 - 15);
    box(win, 0, 0);
    const char *options[3] = {"Easy", "Normal", "Hard"};
    int t = multiple_choice(options, 3, 0, win);
    wclear(win);
    wrefresh(win);
    delwin(win);
    switch (t)
    {
    case 0:
        difficulty = 0;
        break;
    case 1:
        difficulty = 1;
        break;
    case 2:
        difficulty = 2;
        break;
    default:
        break;
    }
    return setting_menu();
}

int music_menu()
{
    WINDOW *win = newwin(9, 30, row / 2 - 4, col / 2 - 15);
    box(win, 0, 0);
    const char *options[5] = {"1", "2", "3", "none"};
    int t = multiple_choice(options, 4, 0, win);
    wclear(win);
    delwin(win);
    Mix_HaltMusic();
    if (t == 0)
    {
        Mix_PlayMusic(m1, -1);
    }
    if (t == 1)
    {
        Mix_PlayMusic(m2, -1);
    }
    if (t == 2)
    {
        Mix_PlayMusic(m3, -1);
    }
    return setting_menu();
}

int setting_menu()
{
    WINDOW *win = newwin(9, 30, row / 2 - 4, col / 2 - 15);
    box(win, 0, 0);
    const char *options[5] = {"Hero's skin", "Difficulty", "music", "Back"};
    int t = multiple_choice(options, 4, 0, win);
    wclear(win);
    delwin(win);
    if (t == 0)
        return choose_color();
    else if (t == 3)
        return Main_menu();
    else if (t == 1)
        return difficulty_menu();
    else if (t == 2)
        return music_menu();
}

int Main_menu()
{
    WINDOW *win = newwin(9, 30, row / 2 - 4, col / 2 - 15);
    box(win, 0, 0);
    const char *options[5] = {"New game", "Resume last game", "Scores", "Settings"};
    int t = multiple_choice(options, 4, 0, win);
    delwin(win);
    if (t == 0)
    {
        return t;
    }
    if (t == 1)
    {
        return load_game();
    }
    if (t == 2)
    {
        return score_menu();
    }
    if (t == 3)
    {
        return setting_menu();
    }
}

// map and game
struct point
{
    int y;
    int x;
} point;

struct monster
{
    char name[50];
    struct point loc;
    char type;
    int health;
    int damage;
    int move_count;
    int turn;
};

typedef struct room
{
    int x;
    int y;
    int height;
    int width;
    int door_count;
    struct point *doors;
    char door_types[10][2];
    struct point key; // for generating a new password
    int password1;
    int password2;
    int marked;         // for checking if it was visited in game
    int accessed;       // for drawing pathways
    int filled[30][30]; // to see if a block is filled with an item ;
    struct point *traps;
    int trap_count;
    int trap_status[10]; // to see if it's triggered or found
    struct point *pillars;
    int pillar_count;
    struct point *foods;
    int foods_count;
    struct point *gold;
    int gold_count;
    struct point *black_gold;
    int black_gold_count;
    int stair_flag;
    struct point stair;
    int potion_count;
    struct point *potions;
    int weapon_count;
    struct point *weapons;
    int monster_count;
    struct monster *monsters;
    int enchanted;

} room;

// foods : 0 => regular , 1 => super , 2 => magical , 3 => bad;
// weapons : 0 => mace  , 1 => dagger , 2 => magic_wand , 3 => arrow , 4 => sword;
// potions : 0 => health , 1 => speed , 2 => harm;

char foods[4][10] = {"regular", "super", "magical", "corrupted"};
char weapons[5][10] = {"mace", "sword", "arrow", "dagger", "magic wand"};
char potions[3][10] = {"healing", "swift", "harming"};

struct hero
{
    int max_health;
    int health;
    int max_hunger;
    int hunger;
    struct point loc;
    int gold_count;
    int bgold_count;
    int food_count[4];
    int weapon_count[5];
    int current_weapon;
    int potion_count[3];
    int total_exp;
    int score;
} hero;

void initialize_hero()
{
    hero.max_health = 99 - difficulty * 25;
    hero.health = hero.max_health;
    hero.max_hunger = 20;
    hero.hunger = 20;
    hero.gold_count = 0;
    hero.bgold_count = 0;
    hero.current_weapon = -1;
    hero.loc.y = -1;
    for (int i = 0; i < 4; i++)
    {
        hero.food_count[i] = 0;
    }
    for (int i = 0; i < 5; i++)
    {
        hero.weapon_count[i] = 0;
    }
    hero.weapon_count[0] = 1;
    for (int i = 0; i < 3; i++)
    {
        hero.potion_count[i] = 0;
    }
    hero.total_exp = 0;
    hero.score = 0;
}

room *create_hallways(WINDOW *win, room *rooms, int room_count, int dir, struct point contain);
void doors(room *rooms, int room_count, WINDOW *win);
void traps_and_pillars(room *rooms, int room_count, WINDOW *win);
void place_hero(WINDOW *win, room room1, int y, int x);
int row, col;
int input_manager(int in, room *rooms, WINDOW *win, int *pre);
void place_stair(int roomnum, room *rooms, WINDOW *win);
void bless_african(room *rooms, int room_count, WINDOW *win);
void getting_rich(room *rooms, int room_count, WINDOW *win);
void getting_richer(room *rooms, int room_count, WINDOW *win);
void drugs_and_cocaine(room *rooms, int room_count, WINDOW *win);
void this_is_america(room *rooms, int room_count, WINDOW *win);
void show_stats();
void show_msg(const char *message, int n);
void save_stats();
int interrupted = 0;
int level = 1;
int hunger0 = 0;
int damage_boost = 1;
int regen_boost = 1;
int revealed = 0;
WINDOW *gamewin;
WINDOW *stats;
WINDOW *msg;

void ha_ha_loser()
{
    WINDOW *win = newwin(row - 2, col - 2, 1, 1);
    box(win, 0, 0);
    wrefresh(win);
    wattron(win, A_BOLD);
    mvwprintw(win, row / 2 - 4, col / 2 - 5, "YOU ARE DEAD");
    mvwprintw(win, row / 2 - 2, col / 2 - 40, "Your journey of danger for the goal of wealth and power is over . ");
    mvwprintw(win, row / 2 - 1, col / 2 - 40, "YOU could have lived happily amongst your family but now the gream reaper is your only companion .");
    mvwprintw(win, row / 2 - 0, col / 2 - 40, "But the memories that you built ... ");
    mvwprintw(win, row / 2 - -1, col / 2 - 40, "Not even those will remain as this code doesn't save the memories .");
    mvwprintw(win, row / 2 - -2, col / 2 - 40, "But the count of your gold coins ...  Hell yeah! SCORE: %d pts", hero.bgold_count * 200 + hero.gold_count * 50);
    wrefresh(win);
}

void you_think_you_win()
{
    WINDOW *win = newwin(row - 2, col - 2, 1, 1);
    box(win, 0, 0);
    wrefresh(win);
    wattron(win, A_BOLD);
    mvwprintw(win, row / 2 - 4, col / 2 - 10, "YOU WIN ... OR DO YOU?");
    mvwprintw(win, row / 2 - 2, col / 2 - 40, "At best you are a wild animal killing others to achive more profit or another day of sunlight.");
    mvwprintw(win, row / 2 - 1, col / 2 - 40, "But you don't do it to have a nice living . you enjoy piling up coins . you're a coin addict.");
    mvwprintw(win, row / 2 - 0, col / 2 - 40, "If you don't believe me tell me how much would be enough ?");
    mvwprintw(win, row / 2 - -1, col / 2 - 40, "If you have an answer you are now furious that a number higher than that exists . you want to get it.");
    mvwprintw(win, row / 2 - -2, col / 2 - 40, "anyway here is what you gathered: %d pts", hero.bgold_count * 200 + hero.gold_count * 50 + hero.score);
    wrefresh(win);
}

typedef struct
{
    char ch;
    int attr;
    int pathway;
    int visited;
} Cell;

Cell map[1000][1000];

void scan_map(WINDOW *win)
{
    for (int y = 0; y < row - 4; y++)
    {
        for (int x = 0; x < col * 3 / 4 - 5; x++)
        {
            chtype ch = mvwinch(win, 2 + y, col / 4 - 1 + x);
            map[y][x].ch = ch & A_CHARTEXT;
            map[y][x].attr = ch & A_ATTRIBUTES;
            if (ch & A_CHARTEXT == '#')
            {
                map[y][x].pathway = 1;
                map[y][x].visited = 0;
            }
            else
            {
                map[y][x].pathway = 0;
            }
        }
    }
}

void print_map_black()
{
    for (int y = 0; y < row - 5; y++)
    {
        for (int x = 0; x < col * 3 / 4 - 5; x++)
        {
            wattr_on(gamewin, COLOR_PAIR(13), NULL);
            mvwaddch(gamewin, 2 + y, col / 4 - 1 + x, map[y][x].ch);
            wattr_off(gamewin, COLOR_PAIR(13), NULL);
        }
    }
    wrefresh(gamewin);
}

void update_map(int y, int x, chtype c)
{
    map[y - 2][x - col / 4 + 1].ch = c & A_CHARTEXT;
    map[y - 2][x - col / 4 + 1].attr = c & A_ATTRIBUTES;
}

void reveal_area(int start_y, int start_x, int height, int width)
{
    start_y -= 2;
    start_x -= col / 4 - 1;
    for (int y = start_y; y < start_y + height; y++)
    {
        for (int x = start_x; x < start_x + width; x++)
        {
            if (y >= 0 && y < row - 4 && x >= 0 && x < col * 3 / 4 - 5)
            {

                wattron(gamewin, map[y][x].attr);
                mvwaddch(gamewin, 2 + y, col / 4 - 1 + x, map[y][x].ch);
                wattroff(gamewin, map[y][x].attr);
            }
        }
    }
    wrefresh(gamewin);
}

void print_back()
{
    for (int y = 0; y < row - 4; y++)
    {
        for (int x = 0; x < col * 3 / 4 - 5; x++)
        {
            wattron(gamewin, map[y][x].attr);
            mvwaddch(gamewin, 2 + y, col / 4 - 1 + x, map[y][x].ch);
            wattroff(gamewin, map[y][x].attr);
        }
    }
    wrefresh(gamewin);
}

void reveal_marked(room *rooms, int room_count)
{
    for (int i = 0; i < room_count; i++)
    {
        if (rooms[i].marked == 1)
        {
            reveal_area(rooms[i].y, rooms[i].x, rooms[i].height, rooms[i].width);
        }
    }
    for (int i = 0; i < row - 4; i++)
    {
        for (int j = 0; j < col * 3 / 4 - 5; j++)
        {
            if (map[i][j].pathway == 1 && map[i][j].visited == 1)
            {
                mvwaddch(gamewin, i + 2, j + col / 4 - 1, '#');
            }
        }
    }
    wrefresh(gamewin);
}

void color_the_room(room room, chtype color)
{
    for (int i = 0; i < room.width; i++)
    {
        for (int j = 0; j < room.height; j++)
        {
            char ch = mvwinch(gamewin, room.y + j, room.x + i);
            if (ch == '|' || ch == '-')
            {
                wattron(gamewin, color);
                mvwaddch(gamewin, room.y + j, room.x + i, ch);
                wattroff(gamewin, color);
            }
        }
    }
}

void handle_signal(int signal)
{
    interrupted = 1;
}

void delete_partly(WINDOW *win, int lines, int cols, int y, int x)
{
    for (int i = 0; i < lines; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            mvwaddch(win, y + i - 1, x + j - 1, ' ');
        }
    }
}

int kbhit()
{
    struct timeval tv = {0, 0};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    return select(1, &readfds, NULL, NULL, &tv);
}

struct point pick_a_random_point(room room)
{
    int xx = rand() % (room.width - 2) + 1;
    int yy = rand() % (room.height - 2) + 1;
    if (room.filled[yy][xx] == 1)
        return pick_a_random_point(room);
    struct point random;
    random.x = xx + room.x;
    random.y = yy + room.y;
    return random;
}
void make_room(room room, WINDOW *win)
{
    for (int i = room.x; i < room.x + room.width; i++)
    {
        mvwaddch(win, room.y, i, '-');
        mvwaddch(win, room.y + room.height - 1, i, '-');
        for (int j = room.y + 1; j < room.y + room.height - 1; j++)
        {
            if (i == room.x || i == room.x + room.width - 1)
            {
                mvwaddch(win, j, i, '|');
            }
            else
            {
                mvwaddch(win, j, i, '.');
            }
        }
    }
}

int rooms_overlap(room new_room, room *existing_rooms, int existing_room_count)
{
    for (int i = 0; i < existing_room_count; i++)
    {
        room existing_room = existing_rooms[i];

        if (new_room.x < existing_room.x + existing_room.width + 9 &&
            new_room.x + new_room.width + 9 > existing_room.x &&
            new_room.y < existing_room.y + existing_room.height + 9 &&
            new_room.y + new_room.height + 9 > existing_room.y)
        {
            return 1;
        }

        if (new_room.x <= existing_room.x + existing_room.width &&
            new_room.x + new_room.width >= existing_room.x + 9 &&
            new_room.y <= existing_room.y + existing_room.height &&
            new_room.y + new_room.height >= existing_room.y + 9)
        {
            return 1;
        }
    }
    return 0;
}
int room_count = 0;
room *create_random_rooms(WINDOW *win, int row, int col, int n, int dir, struct point contain)
{
    room_count = rand() % 3 + 6;
    n = room_count;
    room *rooms = (room *)malloc(room_count * sizeof(room));
    int valid_rooms = 0;
    int tries = 0;
    for (int i = 0; i < room_count; i++)
    {
        rooms[i].width = rand() % 13 + 9;
        rooms[i].height = rand() % 9 + 9;
        rooms[i].y = rand() % (row / 2) + row / 10;
        rooms[i].x = rand() % (3 * col / 6) + col / 4;
        rooms[i].marked = 0;
        rooms[i].accessed = 0;
        rooms[i].door_count = 0;
        rooms[i].doors = malloc(10 * sizeof(struct point));
        rooms[i].traps = malloc(5 * sizeof(struct point));
        rooms[i].pillars = malloc(10 * sizeof(struct point));
        rooms[i].foods = malloc(10 * sizeof(struct point));
        rooms[i].gold = malloc(10 * sizeof(struct point));
        rooms[i].black_gold = malloc(10 * sizeof(struct point));
        rooms[i].potions = malloc(10 * sizeof(struct point));
        rooms[i].weapons = malloc(10 * sizeof(struct point));
        rooms[i].monster_count = 0;
        rooms[i].monsters = malloc(10 * sizeof(struct monster));
        rooms[i].key.y = -1;
        rooms[i].stair_flag = 0;
        rooms[i].enchanted = 0;
        if (!rooms_overlap(rooms[i], rooms, valid_rooms))
        {
            make_room(rooms[i], win);
            valid_rooms++;
        }
        else
        {
            i--;
        }
        tries++;
        if (tries > 100)
        {
            delete_partly(win, row - 4, col * 3 / 4 - 5, 2, col / 4 - 1);
            return create_random_rooms(win, row, col, n, dir, contain);
        }
    }
    if ((contain.y != -1 && mvwinch(win, contain.y, contain.x) != '.'))
    {
        delete_partly(win, row - 4, col * 3 / 4 - 5, 2, col / 4 - 1);
        return create_random_rooms(win, row, col, n, dir, contain);
    }

    return create_hallways(win, rooms, room_count, dir, contain);
}

int compare_rooms_by_x(const void *a, const void *b)
{
    const room *roomA = (const room *)a;
    const room *roomB = (const room *)b;

    return (roomA->x - roomB->x);
}
int compare_rooms_by_x_descend(const void *a, const void *b)
{
    const room *roomA = (const room *)a;
    const room *roomB = (const room *)b;

    return (-roomA->x + roomB->x);
}

void sort_rooms_by_x(room *rooms, int room_count, int dir)
{
    if (dir == 0)
        qsort(rooms, room_count, sizeof(room), compare_rooms_by_x);
    if (dir == 1)
        qsort(rooms, room_count, sizeof(room), compare_rooms_by_x_descend);
}

void which_room(struct point door, room *rooms, int room_count)
{
    for (int i = 0; i < room_count; i++)
    {
        if (door.x >= rooms[i].x && door.x <= rooms[i].x + rooms[i].width - 1 &&
            door.y >= rooms[i].y && door.y <= rooms[i].y + rooms[i].height - 1)
        {
            rooms[i].accessed = 1;
            rooms[i].doors[rooms[i].door_count++] = door;
        }
    }
}

int which(int y, int x, room *rooms, int room_count)
{
    for (int i = 0; i < room_count; i++)
    {
        if (x >= rooms[i].x && x <= rooms[i].x + rooms[i].width - 1 &&
            y >= rooms[i].y && y <= rooms[i].y + rooms[i].height - 1)
        {
            return i;
        }
    }
    return -1;
}
// void do_some_x(WINDOW* win , room* rooms , int room_count ,int dir , struct point contain , int i){
//     int room1_center_y = rooms[i].y + rooms[i].height / 2;
//     int room1_center_x = rooms[i].x + rooms[i].width / 2;

//     int room2_center_y = rooms[i + 1].y + rooms[i + 1].height / 2;
//     int room2_center_x = rooms[i + 1].x + rooms[i + 1].width / 2;

// }

// void do_some_y(WINDOW* win , room* rooms , int room_count ,int dir , struct point contain , int i){
//     int room1_center_y = rooms[i].y + rooms[i].height / 2;
//     int room1_center_x = rooms[i].x + rooms[i].width / 2;

//     int room2_center_y = rooms[i + 1].y + rooms[i + 1].height / 2;
//     int room2_center_x = rooms[i + 1].x + rooms[i + 1].width / 2;
// }

void place_deamon(room *rooms, int room_count)
{
    int b = rand() % room_count;
    struct point p = pick_a_random_point(rooms[b]);
    wattron(gamewin, A_BOLD);
    mvwaddch(gamewin, p.y, p.x, 'D');
    wattroff(gamewin, A_BOLD);
    rooms[b].filled[p.y - rooms[b].y][p.x - rooms[b].x] = 1;
    rooms[b].monsters[rooms[b].monster_count].type = 'D';
    rooms[b].monsters[rooms[b].monster_count].damage = 2;
    rooms[b].monsters[rooms[b].monster_count].health = 5;
    rooms[b].monsters[rooms[b].monster_count].loc.y = p.y;
    rooms[b].monsters[rooms[b].monster_count].loc.x = p.x;
    rooms[b].monsters[rooms[b].monster_count].move_count = 0;
    strcpy(rooms[b].monsters[rooms[b].monster_count].name, "Deamon");
    rooms[b].monsters[rooms[b].monster_count].turn = 0;
    rooms[b].monster_count++;
}
void place_fire_monster(room *rooms, int room_count)
{
    int b = rand() % room_count;
    struct point p = pick_a_random_point(rooms[b]);
    wattron(gamewin, A_BOLD);
    mvwaddch(gamewin, p.y, p.x, 'F');
    wattroff(gamewin, A_BOLD);
    rooms[b].filled[p.y - rooms[b].y][p.x - rooms[b].x] = 1;
    rooms[b].monsters[rooms[b].monster_count].type = 'F';
    rooms[b].monsters[rooms[b].monster_count].damage = 5;
    rooms[b].monsters[rooms[b].monster_count].health = 10;
    rooms[b].monsters[rooms[b].monster_count].loc.y = p.y;
    rooms[b].monsters[rooms[b].monster_count].loc.x = p.x;
    rooms[b].monsters[rooms[b].monster_count].move_count = 0;
    strcpy(rooms[b].monsters[rooms[b].monster_count].name, "Fire monster");
    rooms[b].monsters[rooms[b].monster_count].turn = 0;
    rooms[b].monster_count++;
}
void place_giant(room *rooms, int room_count)
{
    int b = rand() % room_count;
    struct point p = pick_a_random_point(rooms[b]);
    wattron(gamewin, A_BOLD);
    mvwaddch(gamewin, p.y, p.x, 'G');
    wattroff(gamewin, A_BOLD);
    rooms[b].filled[p.y - rooms[b].y][p.x - rooms[b].x] = 1;
    rooms[b].monsters[rooms[b].monster_count].type = 'G';
    rooms[b].monsters[rooms[b].monster_count].damage = 7;
    rooms[b].monsters[rooms[b].monster_count].health = 15;
    rooms[b].monsters[rooms[b].monster_count].loc.y = p.y;
    rooms[b].monsters[rooms[b].monster_count].loc.x = p.x;
    rooms[b].monsters[rooms[b].monster_count].move_count = 0;
    strcpy(rooms[b].monsters[rooms[b].monster_count].name, "Giant");
    rooms[b].monsters[rooms[b].monster_count].turn = 0;
    rooms[b].monster_count++;
}
void place_snake(room *rooms, int room_count)
{
    int b = rand() % room_count;
    struct point p = pick_a_random_point(rooms[b]);
    wattron(gamewin, A_BOLD);
    mvwaddch(gamewin, p.y, p.x, 'S');
    wattroff(gamewin, A_BOLD);
    rooms[b].filled[p.y - rooms[b].y][p.x - rooms[b].x] = 1;
    rooms[b].monsters[rooms[b].monster_count].type = 'S';
    rooms[b].monsters[rooms[b].monster_count].damage = 10;
    rooms[b].monsters[rooms[b].monster_count].health = 20;
    rooms[b].monsters[rooms[b].monster_count].loc.y = p.y;
    rooms[b].monsters[rooms[b].monster_count].loc.x = p.x;
    rooms[b].monsters[rooms[b].monster_count].move_count = 0;
    strcpy(rooms[b].monsters[rooms[b].monster_count].name, "Snake");
    rooms[b].monsters[rooms[b].monster_count].turn = 0;
    rooms[b].monster_count++;
}
void place_undead(room *rooms, int room_count)
{
    int b = rand() % room_count;
    struct point p = pick_a_random_point(rooms[b]);
    wattron(gamewin, A_BOLD);
    mvwaddch(gamewin, p.y, p.x, 'U');
    wattroff(gamewin, A_BOLD);
    rooms[b].filled[p.y - rooms[b].y][p.x - rooms[b].x] = 1;
    rooms[b].monsters[rooms[b].monster_count].type = 'U';
    rooms[b].monsters[rooms[b].monster_count].damage = 13;
    rooms[b].monsters[rooms[b].monster_count].health = 30;
    rooms[b].monsters[rooms[b].monster_count].loc.y = p.y;
    rooms[b].monsters[rooms[b].monster_count].loc.x = p.x;
    rooms[b].monsters[rooms[b].monster_count].move_count = 0;
    strcpy(rooms[b].monsters[rooms[b].monster_count].name, "Undead");
    rooms[b].monsters[rooms[b].monster_count].turn = 0;
    rooms[b].monster_count++;
}

room *create_hallways(WINDOW *win, room *rooms, int room_count, int dir, struct point contain)
{
    sort_rooms_by_x(rooms, room_count, dir);

    for (int i = 0; i < room_count - 1; i++)
    {
        if (rooms[i + 1].accessed == 1)
        {
            continue;
        }

        int room1_center_y = rooms[i].y + rooms[i].height / 2;
        int room1_center_x = rooms[i].x + rooms[i].width / 2;

        int room2_center_y = rooms[i + 1].y + rooms[i + 1].height / 2;
        int room2_center_x = rooms[i + 1].x + rooms[i + 1].width / 2;
        // left to right
        if (dir == 0)
        {
            if (room2_center_x == rooms[i].x + rooms[i].width - 1)
            {
                room2_center_x += 2;
            }
            if (room2_center_x == rooms[i + 2].x)
            {
                room2_center_x -= 2;
            }
            if (room2_center_x == rooms[i].x + rooms[i].width)
            {
                room2_center_x++;
            }
            if (room2_center_x == rooms[i + 2].x - 1)
            {
                room2_center_x--;
            }
            if (room1_center_y == rooms[i + 1].y - 1)
            {
                room1_center_y += 2;
            }
            if (room1_center_y == rooms[i + 1].y + rooms[i + 1].height)
            {
                room1_center_y -= 2;
            }
            if (room1_center_y == rooms[i + 2].y)
            {
                room1_center_y -= 2;
            }
            if (room1_center_y == rooms[i + 2].y + rooms[i + 2].height - 1)
            {
                room1_center_y += 2;
            }

            for (int x = room1_center_x; x <= room2_center_x; x++)
            {
                char c = mvwinch(win, room1_center_y, x);
                if (c == ' ')
                {
                    mvwaddch(win, room1_center_y, x, '#');
                }
                if ((c == '-' || c == '|'))
                {
                    int t = mvwinch(win, room1_center_y, x - 1);
                    if (t != '.' && (room1_center_y == rooms[i + 1].y || room1_center_y == rooms[i + 1].y + rooms[i + 1].height - 1))
                    {
                        if (room2_center_y > room1_center_y)
                        {
                            room1_center_y += 2;
                            x--;
                            mvwaddch(win, room1_center_y, x - 1, '#');
                            mvwaddch(win, room1_center_y - 1, x - 1, '#');
                            mvwaddch(win, room1_center_y - 2, x, ' ');
                        }
                        else
                        {
                            room1_center_y -= 2;
                            x--;
                            mvwaddch(win, room1_center_y + 1, x - 1, '#');
                            mvwaddch(win, room1_center_y + 2, x, ' ');
                            mvwaddch(win, room1_center_y, x - 1, '#');
                        }
                    }
                    if (mvwinch(win, room1_center_y, x) == '-' || mvwinch(win, room1_center_y, x) == '|')
                    {
                        mvwaddch(win, room1_center_y, x, '+');
                        struct point door;
                        door.y = room1_center_y;
                        door.x = x;
                        which_room(door, rooms, room_count);
                    }
                    else
                    {
                        mvwaddch(win, room1_center_y, x, '#');
                    }
                }
            }
            // right to left
        }
        else
        {
            if (room2_center_x == rooms[i].x)
            {
                room2_center_x -= 3;
            }
            if (room2_center_x == rooms[i + 2].x + rooms[i + 2].width - 1)
            {
                room2_center_x += 3;
            }
            if (room2_center_x == rooms[i].x + 1)
            {
                room2_center_x -= 2;
            }
            if (room2_center_x == rooms[i + 2].x + rooms[i + 2].width)
            {
                room2_center_x += 2;
            }
            if (room1_center_y == rooms[i + 1].y - 1)
            {
                room1_center_y += 2;
            }
            if (room1_center_y == rooms[i + 1].y + rooms[i + 1].height)
            {
                room1_center_y -= 2;
            }
            if (room1_center_y == rooms[i + 2].y)
            {
                room1_center_y -= 2;
            }
            if (room1_center_y == rooms[i + 2].y + rooms[i + 2].height - 1)
            {
                room1_center_y += 2;
            }

            for (int x = room1_center_x; x >= room2_center_x; x--)
            {
                char c = mvwinch(win, room1_center_y, x);
                if (c == ' ')
                {
                    mvwaddch(win, room1_center_y, x, '#');
                }
                if ((c == '-' || c == '|'))
                {
                    int t = mvwinch(win, room1_center_y, x + 1);
                    if (t != '.' && (room1_center_y == rooms[i + 1].y || room1_center_y == rooms[i + 1].y + rooms[i + 1].height - 1))
                    {
                        if (room2_center_y > room1_center_y)
                        {
                            room1_center_y += 2;
                            mvwaddch(win, room1_center_y, x + 1, '#');
                            mvwaddch(win, room1_center_y - 1, x + 1, '#');
                            mvwaddch(win, room1_center_y - 2, x, ' ');
                        }
                        else
                        {
                            room1_center_y -= 2;
                            mvwaddch(win, room1_center_y, x + 1, '#');
                            mvwaddch(win, room1_center_y + 1, x + 1, '#');
                            mvwaddch(win, room1_center_y + 2, x, ' ');
                        }
                    }
                    if (mvwinch(win, room1_center_y, x) == '-' || mvwinch(win, room1_center_y, x) == '|')
                    {
                        mvwaddch(win, room1_center_y, x, '+');
                        struct point door;
                        door.y = room1_center_y;
                        door.x = x;
                        which_room(door, rooms, room_count);
                    }
                    else
                    {
                        mvwaddch(win, room1_center_y, x, '#');
                    }
                }
            }
        }

        if (room1_center_y < room2_center_y)
        {
            for (int y = room1_center_y; y <= room2_center_y; y++)
            {
                char c = mvwinch(win, y, room2_center_x);
                if (c == ' ')
                {
                    mvwaddch(win, y, room2_center_x, '#');
                }
                if ((c == '-' || c == '|'))
                {
                    mvwaddch(win, y, room2_center_x, '+');
                    struct point door;
                    door.x = room2_center_x;
                    door.y = y;
                    which_room(door, rooms, room_count);
                }
            }
        }
        else
        {
            for (int y = room2_center_y; y <= room1_center_y; y++)
            {
                char c = mvwinch(win, y, room2_center_x);
                if (c == ' ')
                {
                    mvwaddch(win, y, room2_center_x, '#');
                }
                if ((c == '-' || c == '|'))
                {
                    mvwaddch(win, y, room2_center_x, '+');
                    struct point door;
                    door.x = room2_center_x;
                    door.y = y;
                    which_room(door, rooms, room_count);
                }
            }
        }
    }

    doors(rooms, room_count, win);
    traps_and_pillars(rooms, room_count, win);
    place_stair(room_count - 1, rooms, win);

    if (hero.loc.y == -1)
        place_hero(win, rooms[0], -1, -1);
    else
        place_hero(win, rooms[0], contain.y, contain.x);
    if (contain.y != -1)
    {
        int n = which(contain.y, contain.x, rooms, room_count);
        rooms[n].stair_flag = 2;
        rooms[n].stair = contain;
        rooms[n].filled[contain.y - rooms[n].y][contain.x - rooms[n].x] = 1;
        mvwaddch(win, contain.y, contain.x, '>');
    }

    bless_african(rooms, room_count, win);
    getting_rich(rooms, room_count, win);
    getting_richer(rooms, room_count, win);
    drugs_and_cocaine(rooms, room_count, win);
    if (level <= 4)
    {
        int t = rand() % room_count;
        color_the_room(rooms[t], COLOR_PAIR(5));
        rooms[t].enchanted = 1;
        for (int i = rooms[t].potion_count; i < rooms[t].potion_count + 5; i++)
        {
            rooms[t].potions[i] = pick_a_random_point(rooms[t]);
            rooms[t].filled[rooms[t].potions[i].y - rooms[t].y][rooms[t].potions[i].x - rooms[t].x] == 1;
            wattron(win, COLOR_PAIR(5));
            wattron(win, A_BOLD);
            mvwaddch(win, rooms[t].potions[i].y, rooms[t].potions[i].x, '~');
            wattroff(win, A_BOLD);
            wattroff(win, COLOR_PAIR(5));
        }
        rooms[t].potion_count += 5;
    }
    this_is_america(rooms, room_count, win);
    if (level == 1)
    {
        for (int i = 0; i < 4; i++)
            place_deamon(rooms, room_count);
        for (int i = 0; i < 2; i++)
            place_fire_monster(rooms, room_count);
    }
    if (level == 2)
    {
        for (int i = 0; i < 2; i++)
            place_deamon(rooms, room_count);
        for (int i = 0; i < 2; i++)
            place_fire_monster(rooms, room_count);
        place_giant(rooms, room_count);
    }
    if (level == 3)
    {
        for (int i = 0; i < 2; i++)
            place_deamon(rooms, room_count);
        for (int i = 0; i < 2; i++)
            place_fire_monster(rooms, room_count);
        place_giant(rooms, room_count);
        place_snake(rooms, room_count);
    }
    if (level == 4)
    {
        for (int i = 0; i < 2; i++)
            place_giant(rooms, room_count);
        for (int i = 0; i < 2; i++)
            place_snake(rooms, room_count);
        place_undead(rooms, room_count);
    }
    if (level == 5)
    {
        for (int i = 0; i < 2; i++)
            place_snake(rooms, room_count);
        for (int i = 0; i < 2; i++)
            place_undead(rooms, room_count);
    }
    return rooms;
}

void doors(room *rooms, int room_count, WINDOW *win)
{ // exiting doors have 33 percent chance of being encrypted
    for (int i = 0; i < room_count; i++)
    { // doors have 30 percent chance of being hidden
        for (int j = 0; j < rooms[i].door_count; j++)
        {
            int x = rooms[i].doors[j].x;
            int y = rooms[i].doors[j].y;
            if (rand() % 10 > 7 && j > 0)
            {
                rooms[i].door_types[j][0] = 'h';
                if (y == rooms[i].y || y == rooms[i].y - 1 + rooms[i].height)
                {
                    mvwaddch(win, y, x, '-');
                }
                else
                {
                    mvwaddch(win, y, x, '|');
                }
                continue;
            }
            int flag = 0;
            if (j > 0 && rand() % 4 > 1)
            {
                rooms[i].door_types[j][0] = 'e';
                rooms[i].door_types[j][1] = 1;
                wattron(win, COLOR_PAIR(1));
                mvwaddch(win, y, x, '@');
                wattroff(win, COLOR_PAIR(1));
                if (flag == 0)
                {
                    int t = rand() % 4 + 1;
                    int x1, y1;
                    switch (t)
                    {
                    case 1:
                        x1 = rooms[i].x + 1;
                        y1 = rooms[i].y + 1;
                        break;
                    case 2:
                        x1 = rooms[i].x + rooms[i].width - 2;
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
                    if (rooms[i].key.y == -1)
                    {
                        rooms[i].key.x = x1;
                        rooms[i].key.y = y1;
                        rooms[i].filled[y1 - rooms[i].y][x1 - rooms[i].x] = 1;
                        mvwaddch(win, y1, x1, '&');
                    }
                }
                flag = 1;
                continue;
            }
            rooms[i].door_types[j][0] = 'r';
        }
    }
}

void traps_and_pillars(room *rooms, int room_count, WINDOW *win)
{
    for (int i = 0; i < room_count; i++)
    {
        if (rooms[i].width > 10 && rooms[i].height > 10)
        { // bigger rooms may have more traps and pillars;
            rooms[i].trap_count = rand() % 2 + 1;
            rooms[i].pillar_count = rand() % 3 + 1;
        }
        else
        {
            rooms[i].trap_count = rand() % 1;
            rooms[i].pillar_count = rand() % 1;
        }
        for (int j = 0; j < rooms[i].trap_count; j++)
        {
            int x = rand() % (rooms[i].width - 2) + 1;
            int y = rand() % (rooms[i].height - 2) + 1;
            if (rooms[i].filled[y][x] == 1)
            {
                j++;
                continue;
            }
            rooms[i].traps[j].x = rooms[i].x + x;
            rooms[i].traps[j].y = rooms[i].y + y;
            rooms[i].trap_status[j] = 0;
            rooms[i].filled[y][x] = 1;
            mvwaddch(win, rooms[i].y + y, rooms[i].x + x, '.');
        }
        for (int j = 0; j < rooms[i].pillar_count; j++)
        {
            int x = rand() % (rooms[i].width - 4) + 2;
            int y = rand() % (rooms[i].height - 4) + 2;
            if (rooms[i].filled[y][x] == 1)
            {
                j++;
                continue;
            }
            rooms[i].pillars[j].x = rooms[i].x + x;
            rooms[i].pillars[j].y = rooms[i].y + y;
            rooms[i].filled[y][x] = 1;
            mvwaddch(win, rooms[i].y + y, rooms[i].x + x, 'O');
        }
    }
}

void place_stair(int roomnum, room *rooms, WINDOW *win)
{
    struct point p = pick_a_random_point(rooms[roomnum]);
    rooms[roomnum].stair = p;
    rooms[roomnum].filled[p.y - rooms[roomnum].y][p.x - rooms[roomnum].x] = 1;
    mvwaddch(win, p.y, p.x, '<');
}

void bless_african(room *rooms, int room_count, WINDOW *win)
{
    for (int i = 0; i < room_count; i++)
    {
        rooms[i].foods_count = rand() % 3;
        for (int j = 0; j < rooms[i].foods_count; j++)
        {
            rooms[i].foods[j] = pick_a_random_point(rooms[i]);
            rooms[i].filled[rooms[i].foods[j].y - rooms[i].y][rooms[i].foods[j].x - rooms[i].x] == 1;
            wattron(win, COLOR_PAIR(7));
            mvwaddch(win, rooms[i].foods[j].y, rooms[i].foods[j].x, '*');
            wattroff(win, COLOR_PAIR(7));
        }
    }
}

void drugs_and_cocaine(room *rooms, int room_count, WINDOW *win)
{
    for (int i = 0; i < room_count; i++)
    {
        rooms[i].potion_count = rand() % 5;
        if (rooms[i].potion_count < 3)
            rooms[i].potion_count = 0;
        else
            rooms[i].potion_count = 1;
        for (int j = 0; j < rooms[i].potion_count; j++)
        {
            rooms[i].potions[j] = pick_a_random_point(rooms[i]);
            rooms[i].filled[rooms[i].potions[j].y - rooms[i].y][rooms[i].potions[j].x - rooms[i].x] == 1;
            wattron(win, COLOR_PAIR(5));
            wattron(win, A_BOLD);
            mvwaddch(win, rooms[i].potions[j].y, rooms[i].potions[j].x, '~');
            wattroff(win, A_BOLD);
            wattroff(win, COLOR_PAIR(5));
        }
    }
}

void this_is_america(room *rooms, int room_count, WINDOW *win)
{
    for (int i = 0; i < room_count; i++)
    {
        rooms[i].weapon_count = rand() % 5;
        if (rooms[i].weapon_count < 3)
            rooms[i].weapon_count = 0;
        else
            rooms[i].weapon_count = 1;
        for (int j = 0; j < rooms[i].weapon_count; j++)
        {
            rooms[i].weapons[j] = pick_a_random_point(rooms[i]);
            rooms[i].filled[rooms[i].weapons[j].y - rooms[i].y][rooms[i].weapons[j].x - rooms[i].x] == 1;
            wattron(win, COLOR_PAIR(1));
            wattron(win, A_BOLD);
            mvwaddch(win, rooms[i].weapons[j].y, rooms[i].weapons[j].x, '\\');
            wattroff(win, A_BOLD);
            wattroff(win, COLOR_PAIR(1));
        }
    }
}

void getting_rich(room *rooms, int room_count, WINDOW *win)
{
    for (int i = 0; i < room_count; i++)
    {
        rooms[i].gold_count = rand() % 3;
        for (int j = 0; j < rooms[i].gold_count; j++)
        {
            rooms[i].gold[j] = pick_a_random_point(rooms[i]);
            rooms[i].filled[rooms[i].gold[j].y - rooms[i].y][rooms[i].gold[j].x - rooms[i].x] == 1;
            wattron(win, COLOR_PAIR(6));
            mvwaddch(win, rooms[i].gold[j].y, rooms[i].gold[j].x, '$');
            wattroff(win, COLOR_PAIR(6));
        }
    }
}

void getting_richer(room *rooms, int room_count, WINDOW *win)
{
    for (int i = 0; i < room_count; i++)
    {
        rooms[i].black_gold_count = rand() % 5 - 3;
        if (rooms[i].black_gold_count < 0)
            rooms[i].black_gold_count = 0;
        for (int j = 0; j < rooms[i].black_gold_count; j++)
        {
            rooms[i].black_gold[j] = pick_a_random_point(rooms[i]);
            rooms[i].filled[rooms[i].black_gold[j].y - rooms[i].y][rooms[i].black_gold[j].x - rooms[i].x] == 1;
            wattron(win, COLOR_PAIR(8));
            mvwaddch(win, rooms[i].black_gold[j].y, rooms[i].black_gold[j].x, '$');
            wattroff(win, COLOR_PAIR(8));
        }
    }
}

// funcitons for gameplay
void place_hero(WINDOW *win, room room1, int y, int x)
{
    if (y == -1)
    { // random place
        struct point rand = pick_a_random_point(room1);
        hero.loc.y = rand.y;
        hero.loc.x = rand.x;
        wattron(gamewin, COLOR_PAIR(hero_color));
        mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
        wattroff(gamewin, COLOR_PAIR(hero_color));
        update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
        room1.filled[hero.loc.y - room1.y][hero.loc.x - room1.x] = 1;
    }
    else
    {
        hero.loc.y = y;
        hero.loc.x = x;
        wattron(gamewin, COLOR_PAIR(hero_color));
        mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
        wattroff(gamewin, COLOR_PAIR(hero_color));
        update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
        room1.filled[hero.loc.y - room1.y][hero.loc.x - room1.x] = 1;
    }
}

void show_msg(const char *message, int n)
{
    wattron(msg, A_BOLD);
    wattron(msg, COLOR_CYAN);
    mvwprintw(msg, 2, 2, message);
    wattron(msg, A_BOLD);
    wattron(msg, COLOR_CYAN);
    wrefresh(msg);
    int in = 0;
    while (1)
    {
        in = wgetch(msg);
        if (n)
        {
            delete_partly(msg, 2, 35, 2, 2);
            wrefresh(msg);
            break;
        }
        if (in == ' ')
        {
            delete_partly(msg, 2, 35, 2, 2);
            wrefresh(msg);
            break;
        }
    }
}

int which_monster(room room, struct point p)
{
    for (int i = 0; i < room.monster_count; i++)
    {
        if (p.y == room.monsters[i].loc.y && p.x == room.monsters[i].loc.x)
        {
            return i;
        }
    }
    return -1;
}

int manage_monsters(room *room)
{
    for (int i = -4 - difficulty; i <= 4 + difficulty; i++)
    {
        for (int j = -4 - difficulty; j <= 4 + difficulty; j++)
        {
            if (i <= 1 && i >= -1 && j >= -1 && j <= 1)
            {
                int ch = mvwinch(gamewin, hero.loc.y + j, hero.loc.x + i);
                int c = ch & A_CHARTEXT;
                struct point p;
                p.y = hero.loc.y + j;
                p.x = hero.loc.x + i;
                if (c == 'D' || c == 'F' || c == 'U' || c == 'G' || c == 'S')
                {
                    int t = which_monster(*room, p);
                    if (t == -1)
                        continue;
                    if (room->monsters[t].turn)
                        continue;
                    room->monsters[t].move_count = 0;
                    if (rand() % 5 > 1)
                    {
                        hero.health -= room->monsters[t].damage;
                        char message[50];
                        sprintf(message, "The %s injured you", room->monsters[t].name);
                        if (hero.health <= 0)
                        {
                            int in;
                            ha_ha_loser();
                            if (logged_in)
                                save_stats(0);
                            while (1)
                            {
                                in = getch();
                                if (in == 'q')
                                {
                                    endwin();
                                    return 0;
                                }
                            }
                        }
                        show_msg(message, 0);
                        show_stats();
                        room->monsters[t].turn = 1;
                        continue;
                    }
                    else
                    {
                        char message[50];
                        sprintf(message, "The %s missed you", room->monsters[t].name);
                        show_msg(message, 0);
                        show_stats();
                        room->monsters[t].turn = 1;
                        continue;
                    }
                }
            }
            else
            {
                int ch = mvwinch(gamewin, hero.loc.y + j, hero.loc.x + i);
                int c = ch & A_CHARTEXT;
                struct point p;
                p.y = hero.loc.y + j;
                p.x = hero.loc.x + i;
                if (c == 'U' || c == 'G' || c == 'D')
                {
                    int f = which_monster(*room, p);
                    if (f == -1)
                        continue;
                    if (room->monsters[f].turn)
                        continue;
                    if (room->monsters[f].move_count >= 5)
                        continue;
                    int i, j;
                    if (hero.loc.y > p.y)
                        j = 1;
                    else if (hero.loc.y == p.y)
                        j = 0;
                    else
                        j = -1;
                    if (hero.loc.x > p.x)
                        i = 1;
                    else if (hero.loc.x == p.x)
                        i = 0;
                    else
                        i = -1;
                    int flag = 1;
                    int i_copy = i, j_copy = j;
                    int t = mvwinch(gamewin, p.y + j, p.x + i) & A_CHARTEXT;
                    if (t == '.')
                        flag = 0;
                    if (flag)
                    {
                        i = i ? 0 : 1;
                        t = mvwinch(gamewin, p.y + j, p.x + i) & A_CHARTEXT;
                        if (t == '.')
                            flag = 0;
                    }
                    if (flag)
                    {
                        i = i_copy;
                        j = j ? 0 : 1;
                        t = mvwinch(gamewin, p.y + j, p.x + i) & A_CHARTEXT;
                        if (t == '.')
                            flag = 0;
                    }
                    if (flag)
                        continue;
                    wattron(gamewin, A_BOLD);
                    mvwaddch(gamewin, p.y + j, p.x + i, c);
                    update_map(p.y + j, p.x + i, c | A_BOLD);
                    wattroff(gamewin, A_BOLD);
                    mvwaddch(gamewin, p.y, p.x, '.');
                    update_map(p.y, p.x, '.');
                    room->monsters[f].loc.y = p.y + j;
                    room->monsters[f].loc.x = p.x + i;
                    room->monsters[f].move_count++;
                    room->monsters[f].turn = 1;
                }
                else if (c == 'S')
                {
                    int f = which_monster(*room, p);
                    if (f == -1)
                        continue;
                    if (room->monsters[f].turn)
                        continue;
                    int i, j;
                    if (hero.loc.y > p.y)
                        j = 1;
                    else if (hero.loc.y == p.y)
                        j = 0;
                    else
                        j = -1;
                    if (hero.loc.x > p.x)
                        i = 1;
                    else if (hero.loc.x == p.x)
                        i = 0;
                    else
                        i = -1;
                    int flag = 1;
                    int i_copy = i, j_copy = j;
                    int t = mvwinch(gamewin, p.y + j, p.x + i) & A_CHARTEXT;
                    if (t == '.')
                        flag = 0;
                    if (flag)
                    {
                        i = i ? 0 : 1;
                        t = mvwinch(gamewin, p.y + j, p.x + i) & A_CHARTEXT;
                        if (t == '.')
                            flag = 0;
                    }
                    if (flag)
                    {
                        i = i_copy;
                        j = j ? 0 : 1;
                        t = mvwinch(gamewin, p.y + j, p.x + i) & A_CHARTEXT;
                        if (t == '.')
                            flag = 0;
                    }
                    if (flag)
                        continue;
                    wattron(gamewin, A_BOLD);
                    mvwaddch(gamewin, p.y + j, p.x + i, c);
                    update_map(p.y + j, p.x + i, c | A_BOLD);
                    wattroff(gamewin, A_BOLD);
                    mvwaddch(gamewin, p.y, p.x, '.');
                    update_map(p.y, p.x, '.');
                    room->monsters[f].loc.y = p.y + j;
                    room->monsters[f].loc.x = p.x + i;
                    room->monsters[f].move_count++;
                    room->monsters[f].turn = 1;
                }
            }
        }
    }
    for (int i = 0; i < room->monster_count; i++)
    {
        room->monsters[i].turn = 0;
    }
    wrefresh(gamewin);
}

int we_attack(room *room)
{
    if (hero.current_weapon == -1)
        show_msg("No weapon in hand", 0);
    else if (hero.current_weapon == 0 || hero.current_weapon == 1)
    {
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                int ch = mvwinch(gamewin, hero.loc.y + j, hero.loc.x + i);
                int c = ch & A_CHARTEXT;
                struct point p;
                p.y = hero.loc.y + j;
                p.x = hero.loc.x + i;
                if (c == 'D' || c == 'F' || c == 'G' || c == 'S' || c == 'U')
                {
                    int f = which_monster(*room, p);
                    room->monsters[f].health -= hero.current_weapon == 0 ? 5 * damage_boost : 10 * damage_boost;
                    char ma[50];
                    sprintf(ma, "You hit %s", room->monsters[f].name);
                    show_msg(ma, 0);
                    if (room->monsters[f].health <= 0)
                    {
                        mvwaddch(gamewin, p.y, p.x, '.');
                        update_map(p.y, p.x, '.');
                        sprintf(ma, "You defeated %s", room->monsters[f].name);
                        hero.score += 50;
                        show_stats();
                        show_msg(ma, 0);
                    }
                    if (hero.health <= 0)
                    {
                        ha_ha_loser();
                        if (logged_in)
                            save_stats(0);
                        endwin();
                        return 3;
                    }
                }
            }
        }
    }
    else
    {
        int in = wgetch(gamewin);
        if (in <= '9' && in >= '0')
            ;
        else
            return -1;
        int j, i;
        switch (in - 48)
        {
        case 1:
            i = -1;
            j = 1;
            break;
        case 2:
            i = 0;
            j = 1;
            break;
        case 3:
            i = 1;
            j = 1;
            break;
        case 4:
            i = -1;
            j = 0;
            break;
        case 6:
            i = 1;
            j = 0;
            break;
        case 7:
            i = -1;
            j = -1;
            break;
        case 8:
            i = 0;
            j = -1;
            break;
        case 9:
            i = 1;
            j = -1;
            break;
        default:
            break;
        }
        int next = mvwinch(gamewin, hero.loc.y + j, hero.loc.x + i);
        if (next == '-' || next == '|' || next == 'O')
        {
            return 0;
        }

        if (i >= 0 && j >= 0)
        {
            for (int m = 0, n = 0; m <= 5 * j && n <= 5 * i; m += j, n += i)
            {
                struct point p;
                p.y = hero.loc.y + m;
                p.x = hero.loc.x + n;
                int c = mvwinch(gamewin, hero.loc.y + m, hero.loc.x + n) & A_CHARTEXT;
                if (m == 0 && n == 0)
                    continue;
                if (c == 'O' || c == '|' || c == '-')
                {
                    if (hero.current_weapon != 4 && mvwinch(gamewin, hero.loc.y + m - j, hero.loc.x + n - i) == '.')
                    {
                        if (hero.current_weapon == 2)
                            mvwaddch(gamewin, hero.loc.y + m - j, hero.loc.x + n - i, '!');
                        else if (hero.current_weapon == 3)
                            mvwaddch(gamewin, hero.loc.y + m - j, hero.loc.x + n - i, '/');
                        else
                            show_msg("The projectile was broken", 1);
                        return 2;
                    }
                }
                if (c == 'D' || c == 'F' || c == 'U' || c == 'G' || c == 'S')
                {
                    int f = which_monster(*room, p);
                    int damage;
                    switch (hero.current_weapon)
                    {
                    case 2:
                        damage = 5;
                        break;
                    case 3:
                        damage = 12;
                        break;
                    case 4:
                        damage = 15;
                        break;
                    }
                    room->monsters[f].health -= damage * damage_boost;
                    char ma[50];
                    sprintf(ma, "You shot %s", room->monsters[f].name);
                    hero.weapon_count[hero.current_weapon]--;
                    show_msg(ma, 0);
                    if (room->monsters[f].health <= 0)
                    {
                        mvwaddch(gamewin, p.y, p.x, '.');
                        update_map(p.y, p.x, '.');
                        sprintf(ma, "You defeated %s", room->monsters[f].name);
                        hero.score += 50;
                        show_stats();
                        show_msg(ma, 0);
                    }
                    if (hero.health <= 0)
                    {
                        ha_ha_loser();
                        if (logged_in)
                            save_stats(0);
                        endwin();
                        return 3;
                    }
                    return 1;
                }

                if (n == 5 * i && m == 5 * j)
                {
                    if (hero.current_weapon != 4 && mvwinch(gamewin, hero.loc.y + m, hero.loc.x + n) == '.')
                    {
                        if (hero.current_weapon == 2)
                            mvwaddch(gamewin, hero.loc.y + m, hero.loc.x + n, '!');
                        else if (hero.current_weapon == 3)
                            mvwaddch(gamewin, hero.loc.y + m, hero.loc.x + n, '/');
                        else
                            show_msg("The projectile was broken", 1);
                        return 2;
                    }
                }
            }
        }
        else
        {
            for (int m = 0, n = 0; m >= 5 * j && n >= 5 * i; m += j, n += i)
            {
                struct point p;
                p.y = hero.loc.y + m;
                p.x = hero.loc.x + n;
                int c = mvwinch(gamewin, hero.loc.y + m, hero.loc.x + n) & A_CHARTEXT;
                if (m == 0 && n == 0)
                    continue;
                if (c == 'O' || c == '|' || c == '-')
                {
                    if (hero.current_weapon != 4 && mvwinch(gamewin, hero.loc.y + m - j, hero.loc.x + n - i) == '.')
                    {
                        if (hero.current_weapon == 2)
                            mvwaddch(gamewin, hero.loc.y + m - j, hero.loc.x + n - i, '!');
                        else if (hero.current_weapon == 3)
                            mvwaddch(gamewin, hero.loc.y + m - j, hero.loc.x + n - i, '/');
                        else
                            show_msg("The projectile was broken", 1);
                        return 2;
                    }
                }
                if (c == 'D' || c == 'F' || c == 'U' || c == 'G' || c == 'S')
                {
                    int f = which_monster(*room, p);
                    int damage;
                    switch (hero.current_weapon)
                    {
                    case 2:
                        damage = 5;
                        break;
                    case 3:
                        damage = 12;
                        break;
                    case 4:
                        damage = 15;
                        break;
                    }
                    room->monsters[f].health -= damage * damage_boost;
                    char ma[50];
                    sprintf(ma, "You shot %s", room->monsters[f].name);
                    hero.weapon_count[hero.current_weapon]--;
                    show_msg(ma, 0);
                    if (room->monsters[f].health <= 0)
                    {
                        mvwaddch(gamewin, p.y, p.x, '.');
                        update_map(p.y, p.x, '.');
                        sprintf(ma, "You defeated %s", room->monsters[f].name);
                        hero.score += 50;
                        show_stats();
                        show_msg(ma, 0);
                    }
                    if (hero.health <= 0)
                    {
                        ha_ha_loser();
                        if (logged_in)
                            save_stats(0);
                        endwin();
                        return 3;
                    }
                    return 1;
                }

                if (n == 5 * i && m == 5 * j)
                {
                    if (hero.current_weapon != 4 && mvwinch(gamewin, hero.loc.y + m, hero.loc.x + n) == '.')
                    {
                        if (hero.current_weapon == 2)
                            mvwaddch(gamewin, hero.loc.y + m, hero.loc.x + n, '!');
                        else if (hero.current_weapon == 3)
                            mvwaddch(gamewin, hero.loc.y + m, hero.loc.x + n, '/');
                        else
                            show_msg("The projectile was broken", 1);
                        return 2;
                    }
                }
            }
        }
    }
    wrefresh(gamewin);
}

int manage_other_inputs(WINDOW *win, int keypressed, int *pre, room *rooms)
{
    if (keypressed == 'e' || keypressed == 'E')
    {
        for (int i = 0; i < 3; i++)
        {
            if (i == 0)
            {
                mvwprintw(stats, 7 + 2 * i, 1, "%d- %s: %d", i + 1, foods[i], hero.food_count[i] + hero.food_count[3]);
            }
            mvwprintw(stats, 7 + 2 * i, 1, "%d- %s: %d", i + 1, foods[i], hero.food_count[i]);
        }
        show_stats();
        int in = 0;
        wrefresh(gamewin);
        cbreak();
        noecho();
        in = wgetch(stats);

        if (in == '1')
        {

            if (hero.food_count[0] && hero.food_count[3])
            {
                if (rand() % 3 == 0)
                {
                    hero.food_count[3]--;
                    hero.health--;
                    show_msg("That tastes awful", 0);
                }
                else
                {
                    hero.food_count[0]--;
                    hero.hunger += 5;
                    hero.health += 5 * regen_boost;
                    show_msg("That tastes good", 0);
                }
            }
            else if (hero.food_count[3] == 0 && hero.food_count[0] > 0)
            {
                hero.food_count[0]--;
                hero.hunger += 5;
                hero.health += 5 * regen_boost;
                show_msg("That tastes good", 0);
            }
            else if (hero.food_count[0] == 0 && hero.food_count[3] > 0)
            {
                hero.food_count[3]--;
                hero.health--;
                show_msg("That tastes awful", 0);
            }

            if (hero.health > hero.max_health)
                hero.health = hero.max_health;
            if (hero.hunger > hero.max_hunger)
                hero.hunger = hero.max_hunger;
        }
        if (in == '2' && hero.food_count[1] > 0)
        {
            hero.food_count[1]--;
            damage_flag = hero.total_exp;
            damage_boost = 2;
            show_msg("You feel stronger", 0);
            hero.hunger += 4;
            hero.health += 4 * regen_boost;
            if (hero.health > hero.max_health)
                hero.health = hero.max_health;
            if (hero.hunger > hero.max_hunger)
                hero.hunger = hero.max_hunger;
        }
        if (in == '3' && hero.food_count[2] > 0)
        {
            hero.food_count[2]--;
            speed_flag = hero.total_exp;
            show_stats();
            show_msg("Your speed got doubled", 0);
            show_stats();
        }

        show_stats();
        delete_partly(stats, 10, 20, 7, 2);
        wrefresh(stats);
    }
    else if (keypressed == 'w' || keypressed == 'W')
    {
        mvwprintw(stats, 7, 1, "melee: ");
        mvwprintw(stats, 9, 3, "1-mace(%d dmg) = %d", 5 * damage_boost, hero.weapon_count[0]);
        mvwprintw(stats, 11, 3, "2-sword(%d dmg) = %d", 10 * damage_boost, hero.weapon_count[1]);
        mvwprintw(stats, 13, 1, "range: ");
        mvwprintw(stats, 15, 3, "3-arrow(%d dmg) = %d", 5 * damage_boost, hero.weapon_count[2]);
        mvwprintw(stats, 17, 3, "4-dagger(%d dmg) = %d", 12 * damage_boost, hero.weapon_count[3]);
        mvwprintw(stats, 19, 3, "5-wand(%d dmg) = %d", 15 * damage_boost, hero.weapon_count[4]);
        mvwprintw(stats, 21, 1, "q-put away weapon");
        show_stats();
        int in = 0;
        wrefresh(gamewin);
        cbreak();
        noecho();
        in = wgetch(stats);

        if (in == 'q')
        {
            show_msg("weapon put back", 0);
            hero.current_weapon = -1;
            show_msg("pick a weapon", 0);
            in = wgetch(gamewin);
            switch (in)
            {
            case 1:
                if (hero.weapon_count[0] > 0)
                    show_msg("You wield your mace", 0);
                else
                    show_msg("You don't have that", 0);
                break;
            case 2:
                if (hero.weapon_count[1] > 0)
                    show_msg("You wield your sword", 0);
                else
                    show_msg("You don't have that", 0);
                break;
            case 3:
                if (hero.weapon_count[2] > 0)
                    show_msg("You take your bow", 0);
                else
                    show_msg("You don't have that", 0);
                break;
            case 4:
                if (hero.weapon_count[3] > 0)
                    show_msg("You hold a dagger", 0);
                else
                    show_msg("You don't have that", 0);
                break;
            case 5:
                if (hero.weapon_count[4] > 0)
                    show_msg("You take your wand", 0);
                else
                    show_msg("You don't have that", 0);
                break;
            }
        }
        if (in == '1')
        {
            if (hero.current_weapon == -1)
            {
                hero.current_weapon = 0;
                show_msg("You wield your mace", 0);
            }
            else if (hero.current_weapon == 0)
            {
                show_msg("You are already using a mace", 0);
            }
            else
            {
                show_msg("First put your current weapon back", 0);
            }
        }
        if (in == '2')
        {
            if (hero.weapon_count[1] == 0)
            {
                show_msg("You don't have that", 0);
            }
            else if (hero.current_weapon == -1)
            {
                hero.current_weapon = 1;
                show_msg("You wield your sword", 0);
            }
            else if (hero.current_weapon == 1)
            {
                show_msg("You are already using a sword ", 0);
            }
            else
            {
                show_msg("First put your current weapon back", 0);
            }
        }
        if (in == '3')
        {
            if (hero.weapon_count[2] == 0)
            {
                show_msg("You don't have that", 0);
            }
            else if (hero.current_weapon == -1)
            {
                hero.current_weapon = 2;
                show_msg("You take your bow", 0);
            }
            else if (hero.current_weapon == 2)
            {
                show_msg("You are already using a bow", 0);
            }
            else
            {
                show_msg("First put your current weapon back", 0);
            }
        }
        if (in == '4')
        {
            if (hero.weapon_count[3] == 0)
            {
                show_msg("You don't have that", 0);
            }
            else if (hero.current_weapon == -1)
            {
                hero.current_weapon = 3;
                show_msg("You hold a dagger", 0);
            }
            else if (hero.current_weapon == 3)
            {
                show_msg("You are already using a dagger", 0);
            }
            else
            {
                show_msg("First put your current weapon back", 0);
            }
        }
        if (in == '5')
        {
            if (hero.weapon_count[4] == 0)
            {
                show_msg("You don't have that", 0);
            }
            else if (hero.current_weapon == -1)
            {
                hero.current_weapon = 4;
                show_msg("You take your wand", 0);
            }
            else if (hero.current_weapon == 4)
            {
                show_msg("You are already using a wand", 0);
            }
            else
            {
                show_msg("First put your current weapon back", 0);
            }
        }

        show_stats();
        delete_partly(stats, 20, 30, 7, 2);
        wrefresh(stats);
    }
    else if (keypressed == 'q' || keypressed == 'Q')
    {
        for (int i = 0; i < 3; i++)
        {
            mvwprintw(stats, 7 + 2 * i, 1, "%d- %s: %d", i + 1, potions[i], hero.potion_count[i]);
        }
        show_stats();
        int in = 0;
        wrefresh(gamewin);
        cbreak();
        noecho();
        in = wgetch(stats);

        if (in == '1' && hero.potion_count[0] > 0)
        {
            hero.potion_count[0]--;
            hero.health += 8;
            regen_boost = 2;
            regen_flag = hero.total_exp;
            if (hero.health > hero.max_health)
                hero.health = hero.max_health;
            show_stats();
            show_msg("You immidiately feel better", 0);
        }
        if (in == '2' && hero.potion_count[1] > 0)
        {
            hero.potion_count[1]--;
            speed_flag = hero.total_exp;
            speed_boost = 2;
            show_stats();
            show_msg("Your speed got doubled", 0);
        }
        if (in == '3' && hero.potion_count[2] > 0)
        {
            hero.potion_count[2]--;
            damage_flag = hero.total_exp;
            damage_boost = 2;
            show_stats();
            show_msg("You feel stronger", 0);
        }

        show_stats();
        delete_partly(stats, 10, 20, 7, 2);
        wrefresh(stats);
    }
    else if (keypressed == ' ')
    {
        int w = which(hero.loc.y, hero.loc.x, rooms, room_count);
        if (w >= 0)
        {
            we_attack(&rooms[w]);
            manage_monsters(&rooms[w]);
        }
    }
    else if (keypressed == 'm' || keypressed == 'm')
    {
        if (revealed == 0)
        {
            print_back();
            revealed = 1;
        }
        else
        {
            print_map_black();
            reveal_marked(rooms, room_count);
            wrefresh(gamewin);
            revealed = 0;
        }
    }
    else if (keypressed == 'o' || keypressed == 'O')
    {
        Mix_HaltMusic();
    }
    else if (keypressed == 'p' || keypressed == 'P')
    {
        Mix_HaltMusic();
        switch (level)
        {
        case 1:
            Mix_PlayMusic(l1, -1);
            break;
        case 2:
            Mix_PlayMusic(l2, -1);
            break;
        case 3:
            Mix_PlayMusic(l3, -1);
            break;
        case 4:
            Mix_PlayMusic(l4, -1);
            break;
        case 5:
            Mix_PlayMusic(lt, -1);
            break;
        }
    }
    else
    {
        return 4;
    }
    return 1;
}

int mov(WINDOW *win, int keypressed, int *pre, room *rooms)
{
    int g = 0;
    if (keypressed == 'g')
    {
        g = 1;
        keypressed = wgetch(gamewin);
    }
    int j, i;
    switch (keypressed - 48)
    {
    case 1:
        i = -1;
        j = 1;
        break;
    case 2:
        i = 0;
        j = 1;
        break;
    case 3:
        i = 1;
        j = 1;
        break;
    case 4:
        i = -1;
        j = 0;
        break;
    case 6:
        i = 1;
        j = 0;
        break;
    case 7:
        i = -1;
        j = -1;
        break;
    case 8:
        i = 0;
        j = -1;
        break;
    case 9:
        i = 1;
        j = -1;
        break;
    default:
        break;
    }
    struct point current = hero.loc;

    chtype next_block = mvwinch(win, hero.loc.y + j, hero.loc.x + i);
    chtype next = next_block & A_CHARTEXT;
    int w = which(hero.loc.y, hero.loc.x, rooms, room_count);
    if (w == -1)
        w = which(hero.loc.y + j, hero.loc.x + i, rooms, room_count);
    if (w >= 0 && rooms[w].marked == 0)
    {
        rooms[w].marked = 1;
        reveal_area(rooms[w].y, rooms[w].x, rooms[w].height, rooms[w].width);
    }
    if (g == 1)
    {
        if (next == '~' || next == '\\' || next == '$' || next == '*')
        {
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = next_block;
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
        }
    }
    if (next_block == '+' || next_block == '@' || next_block == '#' || next == '#')
    {
        for (int m = -3; m <= 3; m++)
        {
            for (int n = -3; n <= 3; n++)
            {
                int t = mvwinch(gamewin, hero.loc.y + n, hero.loc.x + m) & A_CHARTEXT;
                if (t == '#' || t == '+')
                {
                    mvwaddch(gamewin, hero.loc.y + n, hero.loc.x + m, t);
                    if (t == '#')
                    {
                        map[hero.loc.y + n - 2][hero.loc.x + m - col / 4 + 1].pathway = 1;
                        map[hero.loc.y + n - 2][hero.loc.x + m - col / 4 + 1].visited = 1;
                    }
                }
            }
        }
    }
    if (g != 1)
    {
        if ((next == '.' || next == '#' || next == '+' || next == '?' || next_block == '@'))
        {
            int flag = 0;
            if (w >= 0)
            {
                for (int t = 0; t < rooms[w].trap_count; t++)
                {
                    if (hero.loc.y + j == rooms[w].traps[t].y && hero.loc.x + i == rooms[w].traps[t].x)
                    {
                        rooms[w].trap_status[t] = 1;
                        mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
                        update_map(hero.loc.y, hero.loc.x, *pre);
                        *pre = '^';
                        hero.loc.y += j;
                        hero.loc.x += i;
                        wattron(gamewin, COLOR_PAIR(hero_color));
                        mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
                        wattroff(gamewin, COLOR_PAIR(hero_color));
                        update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
                        hero.health -= 4;
                        show_stats();
                        flag = 1;
                    }
                }
            }
            if(!flag){
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = next_block;
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            }
        }
        else if (next_block == '&')
        { // password generation
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = next_block;
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            wrefresh(win);
            int password = rand() % 9000 + 1000;
            rooms[w].password1 = password;
            WINDOW *pass = newwin(6, 8, 2, col - 10);
            box(pass, 0, 0);
            wrefresh(pass);
            mvwprintw(pass, 1, 2, "code:");
            mvwprintw(pass, 2, 2, "%d", rooms[w].password1);
            mvwprintw(pass, 4, 2, "20s");
            wrefresh(pass);
            signal(SIGINT, handle_signal);
            for (int i = 0; i < 20; i++)
            {
                if (kbhit())
                {
                    char ch = getchar();
                    if (ch)
                    {
                        interrupted = 1;
                        break;
                    }
                }
                sleep(1);
                mvwprintw(pass, 4, 2, "%2ds", 19 - i);
                wrefresh(pass);
            }
            delete_partly(win, 6, 8, 2, col - 10);
            delwin(pass);
        }
        else if (next == '-' || next == '|')
        { // hidden doors
            for (int m = 0; m < rooms[w].door_count; m++)
            {
                if (rooms[w].door_types[m][0] == 'h' && rooms[w].doors[m].y == hero.loc.y + j && rooms[w].doors[m].x == hero.loc.x + i)
                {
                    mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
                    *pre = '?';
                    hero.loc.y += j;
                    hero.loc.x += i;
                    mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
                }
            }
        }
        else if (next_block == '@' + COLOR_PAIR(1))
        {
            WINDOW *pass = newwin(7, 20, 2, col - 22);
            box(pass, 0, 0);
            mvwprintw(pass, 1, 1, "Enter the passcode");
            wrefresh(pass);
            echo();
            int code = 0;
            for (int t = 0; t < 4; t++)
            {
                int n = mvwgetch(pass, 3, 8 + t) - '0';
                code *= 10;
                code += n;
                wrefresh(pass);
            }
            noecho();
            if (code == rooms[w].password1)
            {
                wattron(win, COLOR_PAIR(2));
                mvwprintw(pass, 5, 1, "access granted");
                wattroff(win, COLOR_PAIR(2));
                wrefresh(pass);
                sleep(2);
                delwin(pass);
                delete_partly(win, 7, 20, 2, col - 22);
                mvwaddch(win, hero.loc.y + j, hero.loc.x + i, '@');
                update_map(hero.loc.y + j, hero.loc.x + i, '@');
            }
            else
            {
                wattron(win, COLOR_PAIR(3));
                mvwprintw(pass, 5, 1, "access denied");
                wattroff(win, COLOR_PAIR(3));
                wrefresh(pass);
                sleep(2);
                delwin(pass);
                delete_partly(win, 7, 20, 2, col - 22);
                wattron(win, COLOR_PAIR(3));
                mvwaddch(win, hero.loc.y + j, hero.loc.x + i, '@');
                update_map(hero.loc.y + j, hero.loc.x + i, '@' | COLOR_PAIR(3));
                wattroff(win, COLOR_PAIR(3));
            }
        }
        else if (next_block == '@' + COLOR_PAIR(3))
        {
            WINDOW *pass = newwin(7, 20, 2, col - 22);
            box(pass, 0, 0);
            mvwprintw(pass, 1, 1, "Enter the passcode");
            wrefresh(pass);
            echo();
            int code = 0;
            for (int t = 0; t < 4; t++)
            {
                int n = mvwgetch(pass, 3, 8 + t) - '0';
                code *= 10;
                code += n;
                wrefresh(pass);
            }
            noecho();
            if (code == rooms[w].password1)
            {
                wattron(win, COLOR_PAIR(2));
                mvwprintw(pass, 5, 1, "access granted");
                wattroff(win, COLOR_PAIR(2));
                wrefresh(pass);
                sleep(2);
                delwin(pass);
                delete_partly(win, 7, 20, 2, col - 22);
                mvwaddch(win, hero.loc.y + j, hero.loc.x + i, '@');
                update_map(hero.loc.y + j, hero.loc.x + i, '@');
            }
            else
            {
                wattron(win, COLOR_PAIR(4));
                mvwprintw(pass, 5, 1, "access denied");
                wattroff(win, COLOR_PAIR(4));
                wrefresh(pass);
                sleep(2);
                delwin(pass);
                delete_partly(win, 7, 20, 2, col - 22);
                wattron(win, COLOR_PAIR(4));
                mvwaddch(win, hero.loc.y + j, hero.loc.x + i, '@');
                update_map(hero.loc.y + j, hero.loc.x + i, '@' | COLOR_PAIR(4));
                wattroff(win, COLOR_PAIR(4));
            }
        }
        else if (next_block == '@' + COLOR_PAIR(4))
        {
            WINDOW *pass = newwin(7, 20, 2, col - 22);
            box(pass, 0, 0);
            mvwprintw(pass, 1, 1, "Enter the passcode");
            wrefresh(pass);
            echo();
            int code = 0;
            for (int t = 0; t < 4; t++)
            {
                int n = mvwgetch(pass, 3, 8 + t) - '0';
                code *= 10;
                code += n;
                wrefresh(pass);
            }
            noecho();
            if (code == rooms[w].password1)
            {
                wattron(win, COLOR_PAIR(2));
                mvwprintw(pass, 5, 1, "access granted");
                wattroff(win, COLOR_PAIR(2));
                wrefresh(pass);
                sleep(2);
                delwin(pass);
                delete_partly(win, 7, 20, 2, col - 22);
                mvwaddch(win, hero.loc.y + j, hero.loc.x + i, '@');
                update_map(hero.loc.y + j, hero.loc.x + i, '@');
            }
            else
            {
                wattron(win, COLOR_PAIR(1));
                mvwprintw(pass, 5, 1, "access denied");
                wattroff(win, COLOR_PAIR(1));
                wrefresh(pass);
                sleep(2);
                delwin(pass);
                delete_partly(win, 7, 20, 2, col - 22);
                wattron(win, COLOR_PAIR(5));
                mvwaddch(win, hero.loc.y + j, hero.loc.x + i, '@');
                update_map(hero.loc.y + j, hero.loc.x + i, '@' | COLOR_PAIR(5));
                wattroff(win, COLOR_PAIR(5));
            }
        }
        else if (next_block == '$' + COLOR_PAIR(6))
        {
            hero.gold_count++;
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '.';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            for (int t = 0; t < rooms[w].gold_count; t++)
            {
                if (rooms[w].gold[t].y == hero.loc.y && rooms[w].gold[t].x == hero.loc.x)
                {
                    rooms[w].gold[t].y = -1;
                    rooms[w].gold[t].x = -1;
                }
            }
            show_stats();
        }
        else if (next_block == '$' + COLOR_PAIR(8))
        {
            hero.bgold_count++;
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '.';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            for (int t = 0; t < rooms[w].black_gold_count; t++)
            {
                if (rooms[w].black_gold[t].y == hero.loc.y && rooms[w].black_gold[t].x == hero.loc.x)
                {
                    rooms[w].black_gold[t].y = -1;
                    rooms[w].black_gold[t].x = -1;
                }
            }
            show_stats();
        }
        else if (next_block == '*' + COLOR_PAIR(7))
        {
            int b = rand() % 6;
            if (b > 2)
                b = 0;
            else
                b++;
            hero.food_count[b]++;
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '.';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            for (int t = 0; t < rooms[w].foods_count; t++)
            {
                if (rooms[w].foods[t].y == hero.loc.y && rooms[w].foods[t].x == hero.loc.x)
                {
                    rooms[w].foods[t].y = -1;
                    rooms[w].foods[t].x = -1;
                }
            }
            show_stats();
        }
        else if (next_block == '\\' + COLOR_PAIR(1) + A_BOLD)
        {
            int b = rand() % 9;
            if (b <= 1)
            {
                if (hero.weapon_count[1] == 0)
                    hero.weapon_count[1]++;
                else
                {
                    show_msg("You wouldn't need another sword", 0);
                }
            }
            else if (b == 2)
                hero.weapon_count[3] += 8;
            else if (b >= 3 && b <= 5)
                hero.weapon_count[2] += 20;
            else if (b >= 6)
                hero.weapon_count[4] += 5;

            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '.';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            for (int t = 0; t < rooms[w].weapon_count; t++)
            {
                if (rooms[w].weapons[t].y == hero.loc.y && rooms[w].weapons[t].x == hero.loc.x)
                {
                    rooms[w].weapons[t].y = -1;
                    rooms[w].weapons[t].x = -1;
                }
            }
            show_stats();
        }
        else if (next_block == '~' + COLOR_PAIR(5) + A_BOLD)
        {
            int b = rand() % 3;
            hero.potion_count[b]++;

            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '.';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            for (int t = 0; t < rooms[w].potion_count; t++)
            {
                if (rooms[w].potions[t].y == hero.loc.y && rooms[w].potions[t].x == hero.loc.x)
                {
                    rooms[w].potions[t].y = -1;
                    rooms[w].potions[t].x = -1;
                }
            }
            show_stats();
        }
        else if (next_block == '!')
        {
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '.';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            hero.weapon_count[2]++;
        }
        else if (next_block == '/')
        {
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '.';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            hero.weapon_count[3]++;
        }
        else if (next_block == '^')
        {
            mvwaddch(win, hero.loc.y, hero.loc.x, *pre);
            update_map(hero.loc.y, hero.loc.x, *pre);
            *pre = '^';
            hero.loc.y += j;
            hero.loc.x += i;
            wattron(gamewin, COLOR_PAIR(hero_color));
            mvwaddch(win, hero.loc.y, hero.loc.x, 'H');
            wattroff(gamewin, COLOR_PAIR(hero_color));
            update_map(hero.loc.y, hero.loc.x, 'H' | COLOR_PAIR(hero_color));
            hero.health -= 4;
            show_stats();
        }
        else if (next_block == '<')
        {
            return 2;
        }
        else
        {
            return 4;
        }
    }
    wrefresh(win);
    wrefresh(stats);
    wrefresh(msg);

    int movflag = 0;
    if (hero.loc.y - current.y || hero.loc.x - current.x)
        movflag = 1;

    if (hero.total_exp > 0 && hero.total_exp % (50 - (difficulty * 10)) == 0 && hero.hunger > 0 && movflag)
    {
        hero.hunger--;
        show_stats();
    }
    if (hero.hunger == 0)
    {
        hero.health -= 2;
        show_stats();
    }
    if (hero.total_exp <= regen_flag + 20)
    {
        regen_boost = 2;
    }
    else
    {
        regen_boost = 1;
    }
    if (hero.total_exp <= damage_flag + 20)
    {
        damage_boost = 2;
    }
    else
    {
        damage_boost = 1;
    }
    if (hero.total_exp <= speed_flag + 20)
    {
        speed_boost = 2;
    }
    else
    {
        speed_boost = 1;
    }
    if (hero.hunger == 20 && hero.health < hero.max_health)
    {
        hero.health += 1 * regen_boost;
        show_stats();
    }
    if (movflag && rooms[w].enchanted == 1)
    {
        hero.health--;
        show_stats();
    }

    if (movflag && (hero.total_exp % speed_boost == speed_flag % speed_boost))
    {
        manage_monsters(&rooms[w]);
    }

    if (hero.health <= 0)
        return 3; // deceased
    return 1;
}

void treasure_room(room *t)
{

    t->black_gold_count = 6;
    t->gold_count = 10;
    t->trap_count = 12;
    t->height = 20;
    t->width = 30;
    t->y = row / 2 - 10;
    t->x = col / 2 - 15;
    room_count = 1;
    t->traps = malloc(20 * sizeof(struct point));
    t->black_gold = malloc(20 * sizeof(struct point));
    t->gold = malloc(20 * sizeof(struct point));
    t->monsters = malloc(20 * sizeof(struct monster));
    t->monster_count = 0;
    make_room(*t, gamewin);
    color_the_room(*t, COLOR_PAIR(6));
    place_hero(gamewin, *t, -1, -1);
    for (int i = 0; i < t->gold_count; i++)
    {
        t->gold[i] = pick_a_random_point(*t);
        t->filled[t->gold[i].y - t->y][t->gold[i].x - t->x] == 1;
        wattron(gamewin, COLOR_PAIR(6));
        mvwaddch(gamewin, t->gold[i].y, t->gold[i].x, '$');
        wattroff(gamewin, COLOR_PAIR(6));
    }
    for (int i = 0; i < t->black_gold_count; i++)
    {
        t->black_gold[i] = pick_a_random_point(*t);
        t->filled[t->black_gold[i].y - t->y][t->black_gold[i].x - t->x] == 1;
        wattron(gamewin, COLOR_PAIR(8));
        mvwaddch(gamewin, t->black_gold[i].y, t->black_gold[i].x, '$');
        wattroff(gamewin, COLOR_PAIR(8));
    }
    for (int i = 0; i < t->trap_count; i++)
    {
        t->traps[i] = pick_a_random_point(*t);
        t->trap_status[i] = 0;
        t->filled[t->traps[i].y - t->y][t->traps[i].x - t->x] == 1;
        mvwaddch(gamewin, t->traps[i].y, t->traps[i].x, '.');
    }
    place_stair(0, t, gamewin);
    wrefresh(gamewin);
    for (int i = 0; i < 4; i++)
        place_undead(t, room_count);
    for (int i = 0; i < 4; i++)
        place_snake(t, room_count);
    for (int i = 0; i < 4; i++)
        place_giant(t, room_count);

    wrefresh(gamewin);
    return;
}

int input_manager(int in, room *rooms, WINDOW *win, int *pre)
{
    if (in == 27)
    {
        return 0;
    }
    if (in == 'g' || (in >= '1' && in <= '9' && in != '5'))
    {
        int c = mov(win, in, pre, rooms);
        return c;
    }
    else
    {
        int c = manage_other_inputs(win, in, pre, rooms);
        return c;
    }
}

void show_stats()
{
    wattron(stats, COLOR_PAIR(9));
    mvwprintw(stats, 1, 1, "SCORE: %d(%d gold)", hero.gold_count * 50 + hero.bgold_count * 200 + hero.score, hero.gold_count + hero.bgold_count * 4);
    wattroff(stats, COLOR_PAIR(9));

    wattron(stats, COLOR_PAIR(11));
    mvwprintw(stats, 3, 1, "HP: %2d/%2d", hero.health, hero.max_health);
    wattroff(stats, COLOR_PAIR(11));
    mvwprintw(stats, 5, 1, "HUNGER: %2d/%2d", hero.hunger, hero.max_hunger);
    mvwprintw(stats, 27, 1, "LEVEL: %d", level);
    wrefresh(gamewin);
    wrefresh(stats);
}

void save_stats(int save)
{
    char filename[100];
    sprintf(filename, "./users/%s.txt", user);
    FILE *userfile = fopen(filename, "r");
    FILE *temp = fopen("./users/temp.txt", "w+");
    char line[2000];
    if (!save)
    {
        fgets(line, 2000, userfile);
        fputs(line, temp);
        fgets(line, 2000, userfile);
        int t = atoi(line);
        fprintf(temp, "%d\n", t + hero.bgold_count * 200 + hero.gold_count * 50 + hero.score);
        fgets(line, 2000, userfile);
        t = atoi(line);
        fprintf(temp, "%d\n", t + hero.gold_count + hero.bgold_count * 4);
        fgets(line, 2000, userfile);
        t = atoi(line);
        fprintf(temp, "%d\n", t + 1);
        fgets(line, 2000, userfile);
        t = atoi(line);
        fprintf(temp, "%d\n", t + hero.total_exp);
    }

    if (save)
    {
        for (int i = 0; i < 5; i++)
        {
            fgets(line, 2000, userfile);
            fputs(line, temp);
        }
        fprintf(temp, "1\n");
        fprintf(temp, "%d\n", hero.loc.y);
        fprintf(temp, "%d\n", hero.loc.x);
        fprintf(temp, "%d\n", level);
        fprintf(temp, "%d\n", hero.bgold_count * 200 + hero.gold_count * 50 + hero.score);
        fprintf(temp, "%d\n", hero.gold_count + hero.bgold_count * 4);
        for (int i = 0; i < 5; i++)
        {
            fprintf(temp, "%d\n", hero.weapon_count[i]);
        }
        for (int i = 0; i < 4; i++)
        {
            fprintf(temp, "%d\n", hero.food_count[i]);
        }
        for (int i = 0; i < 3; i++)
        {
            fprintf(temp, "%d\n", hero.potion_count[i]);
        }
        fprintf(temp, "%d\n", hero.health);
        fprintf(temp, "%d\n", hero.hunger);
        for (int i = 0; i < 20; i++)
        {
            fgets(line, 2000, userfile);
        }
    }
    else
    {
        fprintf(temp, "0\n");
    }

    while (fgets(line, 2000, userfile))
    {
        fputs(line, temp);
    }
    remove(filename);
    rename("./users/temp.txt", filename);
}

int load_game()
{
    if (logged_in == 0)
    {
        has_saved = 0;
        WINDOW *win = newwin(9, 30, row / 2 - 4, col / 2 - 15);
        box(win, 0, 0);
        mvwprintw(win, 4, 2, "No saved games found");
        wrefresh(win);
        sleep(4);
        return Main_menu();
    }
    char filename[100];
    sprintf(filename, "./users/%s.txt", user);
    FILE *userfile = fopen(filename, "r");
    char line[2000];
    for (int i = 0; i < 5; i++)
    {
        fgets(line, 2000, userfile);
    }
    fgets(line, 2000, userfile);
    int has_saved = atoi(line);
    if (has_saved == 0)
    {
        WINDOW *win = newwin(9, 30, row / 2 - 4, col / 2 - 15);
        box(win, 0, 0);
        mvwprintw(win, 4, 2, "No saved games found");
        wrefresh(win);
        sleep(4);
        return Main_menu();
    }
    fgets(line, 2000, userfile);
    hero.loc.y = atoi(line);
    fgets(line, 2000, userfile);
    hero.loc.x = atoi(line);
    fgets(line, 2000, userfile);
    level = atoi(line);
    fgets(line, 2000, userfile);
    hero.score = atoi(line);
    fgets(line, 2000, userfile);
    hero.gold_count = atoi(line);
    hero.score -= 50 * hero.gold_count;
    for (int i = 0; i < 5; i++)
    {
        fgets(line, 2000, userfile);
        hero.weapon_count[i] = atoi(line);
    }
    for (int i = 0; i < 4; i++)
    {
        fgets(line, 2000, userfile);
        hero.food_count[i] = atoi(line);
    }
    for (int i = 0; i < 3; i++)
    {
        fgets(line, 2000, userfile);
        hero.potion_count[i] = atoi(line);
    }
    fgets(line, 2000, userfile);
    hero.health = atoi(line);
    fgets(line, 2000, userfile);
    hero.hunger = atoi(line);
    int n;

    return 1;
}

// main
int main()
{
    srand(time(NULL));
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    initscr();
    curs_set(0);
    cbreak();
    getmaxyx(stdscr, row, col);
    start_color();
    setlocale(LC_CTYPE, "");
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_color(10, 750, 300, 0); // orange
    init_color(11, 750, 450, 0); // gold
    init_color(12, 1000, 850, 500);
    init_color(13, 400, 0, 400); // black gold
    init_color(14, 600, 400, 400);
    init_color(15, 800, 800, 800); // light gray
    init_color(16, 102, 750, 178); // bluish green
    init_color(17, 650, 600, 300);
    init_pair(12, 17, COLOR_BLACK);
    init_pair(11, 16, COLOR_BLACK);
    init_pair(10, 15, COLOR_BLACK);
    init_pair(9, 14, COLOR_BLACK);
    init_pair(8, 13, COLOR_BLACK);
    init_pair(7, 12, COLOR_BLACK);
    init_pair(6, 11, COLOR_BLACK);
    init_pair(4, 10, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
    init_pair(13, COLOR_BLACK, COLOR_BLACK);
    init_pair(14, COLOR_WHITE, COLOR_BLACK);
    m1 = Mix_LoadMUS("./music/menu1.mp3");
    m2 = Mix_LoadMUS("./music/menu2.mp3");
    m3 = Mix_LoadMUS("./music/menu3.mp3");
    l1 = Mix_LoadMUS("./music/level1.mp3");
    l2 = Mix_LoadMUS("./music/level2.mp3");
    l3 = Mix_LoadMUS("./music/level3.mp3");
    l4 = Mix_LoadMUS("./music/level4.mp3");
    lt = Mix_LoadMUS("./music/treasure.mp3");
    initialize_hero();
    welcome_page(row, col);
    logged_in = check_user(row, col);
    clear();
    show_login_message(logged_in, row, col);

    Mix_PlayMusic(m1, -1);
    prev = m1;

    Main_menu();
    Mix_HaltMusic();

    gamewin = newwin(row - 2, col - 2, 1, 1);
    stats = newwin(row / 4 * 3 - 5, col / 5, row / 4, 5);
    msg = newwin(row / 8, col / 4 - 6, 4, 5);
    wattron(gamewin, COLOR_PAIR(11));
    box(gamewin, 0, 0);
    wattroff(gamewin, COLOR_PAIR(11));
    wattron(msg, COLOR_PAIR(7));
    box(msg, 0, 0);
    wattroff(msg, COLOR_PAIR(7));
    wattron(stats, COLOR_PAIR(7));
    box(stats, 0, 0);
    wattroff(stats, COLOR_PAIR(7));
    refresh();

    int n = 0;

    struct point *stairs = (struct point *)malloc(4 * sizeof(struct point));
    stairs[0].y = -1;
    stairs[0].x = -1;
    room **rooms = (room **)malloc(4 * sizeof(room *));
    for (int i = 0; i < 4; i++)
    {
        rooms[i] = (room *)malloc(10 * sizeof(room));
    }
    int room_counts[4];
    show_stats();

    for (level; level < 5; level++)
    {
        rooms[level - 1] = create_random_rooms(gamewin, row, col, n, -((level % 2) - 1), hero.loc);
        Mix_HaltMusic();
        switch (level)
        {
        case 1:
            Mix_PlayMusic(l1, -1);
            show_msg("welcome to the dungeon", 1);
            break;
        case 2:
            Mix_PlayMusic(l2, -1);
            show_msg("here comes the second floor", 1);
            break;
        case 3:
            Mix_PlayMusic(l3, -1);
            show_msg("you enter the third floor", 1);
            break;
        case 4:
            Mix_PlayMusic(l4, -1);
            show_msg("I heard the fourth one is the last", 1);
            break;
        }
        room_counts[level - 1] = room_count;
        scan_map(gamewin);
        print_map_black();
        int kj = which(hero.loc.y, hero.loc.x, rooms[level - 1], room_counts[level - 1]);
        rooms[level - 1][kj].marked = 1;
        reveal_area(rooms[level - 1][kj].y, rooms[level - 1][kj].x, rooms[level - 1][kj].height, rooms[level - 1][kj].width);
        // input
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        noecho();
        cbreak();
        int w = which(hero.loc.y, hero.loc.x, rooms[level - 1], room_counts[level - 1]);
        wrefresh(gamewin);
        wrefresh(stats);
        wrefresh(msg);
        int in = 0;
        int pre = '.';
        while (1)
        {
            in = getch();
            int c = input_manager(in, rooms[level - 1], gamewin, &pre);
            if (c == 0)
            {
                if (logged_in)
                    save_stats(1);
                endwin();
                return 0;
            }
            if (c == 2)
            {
                break;
            }
            if (c == 3)
            {
                ha_ha_loser();
                if (logged_in)
                    save_stats(0);
                while (1)
                {
                    in = getch();
                    if (in == 'q')
                    {
                        endwin();
                        return 0;
                    }
                }
            }
            if (c == 1)
            {
                hero.total_exp++;
            }
        }
        stairs[level] = rooms[level - 1][room_counts[level - 1] - 1].stair;
        delete_partly(gamewin, row - 4, col * 3 / 4 - 1, 2, col / 4 - 1);
    }
    for (int i = 0; i < 1; i++)
    {
        Mix_HaltMusic();
        show_msg("OK this is the last for sure", 1);
        Mix_PlayMusic(lt, -1);
        room treasure;
        treasure_room(&treasure);
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        noecho();
        cbreak();
        wrefresh(gamewin);
        wrefresh(stats);
        wrefresh(msg);
        int in = 0;
        int pre = '.';
        while (1)
        {
            in = getch();
            int c = input_manager(in, &treasure, gamewin, &pre);
            if (c == 0)
            {
                endwin();
                return 0;
            }
            if (c == 2)
            {
                break;
            }
            if (c == 3)
            {
                ha_ha_loser();
                if (logged_in)
                    save_stats(0);
                while (1)
                {
                    in = getch();
                    if (in == 'q')
                    {
                        endwin();
                        return 0;
                    }
                }
            }
            if (c == 1)
            {
                hero.total_exp++;
            }
        }
    }
    hero.score += 750 * (difficulty + 1);
    you_think_you_win();
    if (logged_in)
        save_stats(0);
    int in ;
    while(1){
        in = getch();
        if(in == 'q')
        {
            endwin();
            return 0;
        }
    }
    endwin();
    return 0;
}
