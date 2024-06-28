// Compile: gcc -o rpsaas rpsaas.c

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define MAX_NAME_LEN 0x50
#define MAX_TIME_SZ 0x20

// ==========================
// ========= COLORS =========
// ==========================
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

__attribute__((constructor))
void __constructor__() {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    signal(SIGALRM, exit);
}

void get_input(int *in) {
    // Secure integer input function.
    // https://stackoverflow.com/questions/41145908/how-to-stop-user-entering-char-as-int-input-in-c
    char next;
    if (scanf("%d", in) < 0 || *in < 0 || ((next = getchar()) != EOF && next != '\n')) {
         clearerr(stdin);
         do next = getchar(); while (next != EOF && next != '\n');
         clearerr(stdin);
    }
}

void ranged_input(int *in, int _beg, int _end) {
    get_input(in);
    while(*in < _beg && *in > _end) {
        printf("Invalid input. Try again: ");
        get_input(in);
    }
}

// =================================
// === Logging related functions ===
// =================================
// > Modified: https://gist.github.com/TheFlash2k/a32b2b1ffac86afe9faca844a9a6b0d9
#define base(file, method, msg, ...) { fprintf(file, "[%s] ", method); fprintf(file, msg, ##__VA_ARGS__); fprintf(file, "\n"); }
#define Log(x, ...) { base(stdout, "\033[32mLOG\033[0m", x, ##__VA_ARGS__) }
#define Error(x, ...) { base(stdout, "\033[31mERROR\033[0m", x, ##__VA_ARGS__) }
#define Fail(x, ...) { base(stdout, "\033[31mFAIL\033[0m", x, ##__VA_ARGS__) }
#define Info(x, ...) {base(stdout, "\033[34mINFO\033[0m", x, ##__VA_ARGS__) }
#define Warn(x, ...) { base(stdout, "\033[33mWARNING\033[0m", x, ##__VA_ARGS__) }
#define Debug(x, ...) { base(stdout, "\033[35mDEBUG\033[0m", x, ##__VA_ARGS__) }
#define Msg(method, msg, ...) { base(stdout, method, msg, ##__VA_ARGS__) }
#define RPS_PROMPT(x, ...) { printf("[\033[32mRPS-\033[33maaS\033[0m] "); printf(x, ##__VA_ARGS__); }

typedef enum {
    ROCK,
    PAPER,
    SCISSOR
} GameObject;

typedef struct {
    char t_str[MAX_TIME_SZ+1];
    time_t time;
} rptime_t;

rptime_t gettime() {
    rptime_t handler;
    handler.time = time(NULL);
    struct tm* timeInfo = localtime(&handler.time);
    memset(handler.t_str, '\0', MAX_TIME_SZ+1);
	strftime(handler.t_str, MAX_TIME_SZ, "%d/%m/%Y-%H:%M:%S", timeInfo);
    return handler;
}

typedef struct {
    char fullname[MAX_NAME_LEN+1];
    int games_won;
    rptime_t created_at;
} player_t;

bool validate(player_t *player, bool _exit) {
    bool rt = !player || player->fullname[0] == '\0';
    if(rt) {
        Error("Player is not registered yet!");
        if(_exit) exit(1);
    }
    return rt;
}

// ====================================
// === Player related functionality ===
// ====================================
void init_player(player_t *player, bool interactive) {
    if(player && player->fullname[0] != '\0') {
        Error("You are already registered!");
        return;
    }
    memset(player->fullname, '\0', MAX_NAME_LEN);
    player->games_won = 0;
    player->created_at = gettime();
    if(interactive) {
        RPS_PROMPT("Identify yourself: ");
        fgets(player->fullname, MAX_NAME_LEN, stdin);
        player->fullname[strcspn(player->fullname, "\n")] = '\0';
    }
}

void update_player_name(player_t *player, char* name, size_t sz) {
    validate(player, true);
    memset(player->fullname, '\0', MAX_NAME_LEN);
    strncpy(player->fullname, name, sz);
}

void print_player(const player_t *player) {
    if(validate(player, false)) return;
    printf("[======= %sPlayer Info%s =======]\n", BLUE, RESET);
    printf("%sPlayer Name%s   : %s%s%s\n", GREEN, RESET, YELLOW, player->fullname, RESET);
    printf("%sRegistered at%s : %s%s%s\n", GREEN, RESET, YELLOW, player->created_at.t_str, RESET);
    printf("%sGames won%s     : %s%d%s\n", GREEN, RESET, YELLOW, player->games_won, RESET);
    puts("=============================");
}

int menu(const player_t *player) {
    int input;
    printf("===== %sRPS-aaS\033[0m =====\n", CYAN);
    if(player && player->fullname[0] != '\0') {
        printf("==> %sLogged in as%s: %s%s%s\n", GREEN, RESET, RED, player->fullname, RESET);
        printf("==> %sGames won%s   : %s%d%s\n", GREEN, RESET, YELLOW, player->games_won, RESET);
    }
    puts("1. Register yourself");
    puts("2. Play RPS");
    puts("3. Get the flag");
    puts("4. Print your details");
    puts("0. Exit");
    RPS_PROMPT(">> ");
    ranged_input(&input, 0, 4);
    return input;
}

void init_game(player_t *player) {
    if(validate(player, false)) return;
    RPS_PROMPT("Initializing RPS...\n");
    GameObject comp_c = rand() % 3;
    GameObject user_c;
    char input[3];
    do {
        RPS_PROMPT("R/P/S? ");
        fgets(input, 3, stdin);
        char _ = tolower(input[0]);
        if(_ != 'r' && _ != 'p' && _ != 's') {
            Error("Only R/P/S allowed!");
            continue;
        }
        user_c = (ROCK ? _ == 'r' : (PAPER ? _ == 'p' : SCISSOR));
        break;
    } while(true);
    if(
        (user_c == ROCK && comp_c == SCISSOR) ||
        (user_c == PAPER && comp_c == ROCK) ||
        (user_c == SCISSOR && comp_c == PAPER)
    ) {
        Info("You won the match!");
        ++player->games_won;
        Info("You have won %s%d%s games so far!", YELLOW, player->games_won, RESET);
    } else {
        Fail("You lost!");
    }
}

void get_flag(const player_t *player) {
    if(validate(player, false)) return;
    if(player->games_won < 1337) {
        Error("You haven't won %s1337%s games to call yourself a hacker (or a guesser)!", RED, RESET);
        return;
    }
    FILE* fp = fopen("flag.txt", "r");
    char flag[0x100];
    memset(flag, '\0', 0x100);
    if(!fp) {
        Error("%s[FLAG_ERROR]%s: Please contact an adminitrator.", RED, RESET);
        exit(1);
    }
    fgets(flag, 0x100, fp);
    Info("Truly a GOAT-tier pwner (or a guesser)");
    RPS_PROMPT("Flag, specially for you >.<: %s%s%s", RED, flag, RESET);
    exit(0);
}

int main(int argc, char* argv[], char* envp[]) {

    /* ** Game Setup ** */
    bool init = false;
    player_t user;
    memset(user.fullname, '\0', MAX_NAME_LEN);

    int choice;
    do {
        choice = menu(&user);
        switch (choice)
        {
        case 0:
            RPS_PROMPT("Thank you for using my program!");
            exit(0);
        case 1:
            if(user.fullname[0] != '\0') {
                Warn("You are already registered")
            }
            init_player(&user, true);
            break;
        case 2:
            if(!init) {
                if(validate(&user, false)) break;
                srand(user.created_at.time);
                init = true;
            }
            init_game(&user);
            break;
        case 3:
            get_flag(&user);
            break;
        case 4:
            print_player(&user);
            break;
        default:
            Warn("Invalid input..");
            break;
        }
    } while(choice != 0);

    exit(0);
}