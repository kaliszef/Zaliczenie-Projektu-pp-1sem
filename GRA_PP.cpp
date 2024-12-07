#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>


using namespace std;

struct KolorPair {
    int Id;
    int Text;
    int Background;
};

struct GameData{
    int w, k; //wiersze kolumny
    int x, y; //kaczka 
    char SCar, SDuck,SS; //znaki dla tego i tego
    int seed;
    int DKolor, GKolor, SKolor,CFKolor,CEKolor,OKolor,CKolor,FlKolor,STKolor;
    int ObstacleNumber;
    int *FreeLines; //size = GD.w/4
};

struct Score {
    //Overall
    int HighestGameScore;
    //aktulanie wynik
    int CurrentGameScore;
    int MapSize;
    int lvl;
};

struct PlayerData {
    char name[1000];
    char surname[1000];
    char ID[7];
    Score score;
};

struct Car
{
    int x, y; //kordy
    int V, a; // predkosc kierunek
    int type, size; // typ
    int cc;
};

struct Obstacle {
    int x, y;
    int w,k ; //W 00    K 00
              //  00    K 0
              //  00    k 
};

struct stork
{
    int x, y;
};

//used
void swap(int &a, int &b)
{
    int temp = a;
    a = b;
    b = temp;
}
void bubbleSort(int tab[],int dl)
{
    for (int i = 0; i < dl; i++)
    {
        for (int j = 0;j < dl;j++)
        {
            if (tab[i] > tab[j])
            {
                swap(tab[i], tab[j]);
            }
        }
    }
}


//time section
void sleep(int milliseconds) {
    // Pobieramy czas początkowy
    clock_t start_time = clock();
    // Obliczamy czas końcowy (start_time + liczba milisekund)
    clock_t end_time = start_time + milliseconds * (CLOCKS_PER_SEC / 1000);

    // Czekamy, aż upłynie wymagany czas
    while (clock() < end_time) {
        // Pętla oczekująca
    }
}
void ShowTime(int t,bool SHOWTIME)
{
        static int i = 4;
        if (t % 10 == 0 && SHOWTIME == true)
        {
            mvprintw(2, 30, "MAGIC TIME: ");
            mvprintw(2, 42, "%d", i--);
            if (i < 0)
            {
                i = 4;
            }
        }
        else if (i <= 0 && SHOWTIME == false)
        {
            mvprintw(2, 30, "            ");
            mvprintw(2, 42, "%s", "       "  );
            i = 4;
        }
}


// files Default settings
void CreateDefaultFile(const char* filename) {
    FILE* file = nullptr;
    fopen_s(&file, filename, "w");
    // Zapisanie domyślnych wartości do pliku
    fprintf(file, "wiersze=20\n");
    fprintf(file, "kolumny=20\n");
    fprintf(file, "duck_sign=D\n");
    fprintf(file, "car_sign=#\n");

    fclose(file); // Zamknięcie pliku
}
int* SetFreeLines(GameData& GD) {
    int size = GD.w / 7;
    int* free = new int[size];

    for (int i = 0; i < size; i++) {
        bool isValid;
        do {
            isValid = true; // Przy każdej iteracji zakładamy poprawność
            free[i] = rand() % (GD.w - 4) + 2;

            for (int j = 0; j < i; j++) {
                if (free[i] == free[j] || free[i] == free[j] + 1 || free[i] == free[j] - 1) {
                    isValid = false; // Jeśli liczba nie spełnia warunku, powtarzamy losowanie
                    break;
                }
            }
        } while (!isValid); // Powtarzaj, dopóki nie znajdziemy poprawnej liczby
    }

    bubbleSort(free, size); // Sortowanie tablicy
    return free; // Zwracamy tablicę
}
GameData LoadData() {

    FILE* file = nullptr;
    GameData GM;
    if(fopen_s(&file, "GameData.txt", "r") != 0 || file == nullptr) //otwieranie pliku
        CreateDefaultFile("GameData.txt"); // Tworzenie pliku z domyślnymi wartościami jeżeli niema

    // Wczytywanie danych z pliku
    fscanf_s(file, "wiersze=%d\n", &GM.w);
    
    fscanf_s(file, "kolumny=%d\n", &GM.k);
    
    fscanf_s(file, "duck_sign=%c\n", &GM.SDuck, 1);
     
    fscanf_s(file, "car_sign=%c\n", &GM.SCar, 1);

    // Opcjonalnie seed
    if (fscanf_s(file, "seed=%lld\n", &GM.seed) != 1) {
        srand(time(NULL));
    }
    else
    {
        srand(GM.seed);
    }

    fclose(file); // Zamknięcie pliku

    GM.x = GM.w;GM.y = GM.k/2;
    GM.ObstacleNumber = rand() % 5 + 1;
    GM.FreeLines = SetFreeLines(GM);
    GM.SS = '+';

    return GM;
}
Score CreateDefaultScore()
{
    return { 0,0,1,0 };
}


//Score section
//scoreBoard
void CopyString(char* dest, const char* src, int maxLen) {
    int i;
    for (i = 0; i < maxLen - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}
int CompareID(const char* id1, const char* id2) {
    for (int i = 0; i < 6; i++) {
        if (id1[i] != id2[i]) {
            return id1[i] - id2[i];
        }
        if (id1[i] == '\0') {
            break;
        }
    }
    return 0;
}
void SaveScoreToFile(PlayerData& player) {
    const int maxScores = 100;
    PlayerData scores[maxScores];

    FILE* file;
    errno_t err = fopen_s(&file, "ppScoreBoard.txt", "r");
    int count = 0;
    if (err == 0 && file != NULL) {
        while (count < maxScores && fscanf_s(file, "%6s %d", scores[count].ID, (unsigned)_countof(scores[count].ID), &scores[count].score.HighestGameScore) == 2) {
            count++;
        }
        fclose(file);
    }

    bool exists = false;
    for (int i = 0; i < count; i++) {
        if (CompareID(scores[i].ID, player.ID) == 0) {
            scores[i].score.HighestGameScore = player.score.HighestGameScore;
            exists = true;
            break;
        }
    }

    if (!exists && count < maxScores) {
        CopyString(scores[count].ID, player.ID, sizeof(scores[count].ID));
        scores[count].score.HighestGameScore = player.score.HighestGameScore;
        count++;
    }

    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j].score.HighestGameScore < scores[j + 1].score.HighestGameScore) {
                PlayerData temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    err = fopen_s(&file, "ppScoreBoard.txt", "w");
    if (err == 0 && file != NULL) {
        for (int i = 0; i < count; i++) {
            fprintf(file, "%s %d\n", scores[i].ID, scores[i].score.HighestGameScore);
        }
        fclose(file);
    }
    clear();
    printw("\nYour score has been saved. Press any key to continue...");
    refresh();
    getch();
    clear();
}
void displayScoreBoard() {
    // Stała nazwa pliku
    const char* filename = "ppScoreBoard.txt";
    clear();

    // Tytuł okna
    mvprintw(0, 0, "Scoreboard:");
    refresh();

    // Otwórz plik w trybie tekstowym do odczytu
    FILE* file = nullptr;
    if (fopen_s(&file, filename, "r") != 0 || file == nullptr) {
        mvprintw(2, 0, "Nie udalo sie otworzyc pliku: %s", filename);
        mvprintw(4, 0, "Nacisnij dowolny klawisz, aby zamknac.");
        getch();
        endwin();
        return;
    }

    // Wczytywanie i wyświetlanie danych
    PlayerData player;
    int line = 2; // Start linii wyświetlania
    while (fscanf_s(file, "%6s %d", player.ID, (unsigned)_countof(player.ID), &player.score) == 2) {
        mvprintw(line++, 0, "ID: %s  | Score: %d", player.ID, player.score);
        refresh();

        // Obsługa przewijania, gdy wyjście przekracza wysokość ekranu
        if (line >= LINES - 1) {
            mvprintw(LINES - 1, 0, "Nacisnij klawisz, aby kontynuowac...");
            getch();
            clear();
            mvprintw(0, 0, "Scoreboard:");
            line = 2; // Resetowanie linii wyświetlania
        }
    }

    fclose(file);

    // Końcowy komunikat
    mvprintw(line+1, 0, "Koniec pliku. Nacisnij dowolny klawisz, aby zamknac.");
    while (true)
    {
        char ch = getch();
        if (ch != ERR)
        {
            clear();
            return;
        }
    }
}
// int game score
int MaksScore(int x, int y)
{
    if (x > y)
        return x;
    else return y;
}
void ShowScore(PlayerData& PD)
{
    mvprintw(0, 0, "score ");
    mvprintw(0, 5, "%d", PD.score.CurrentGameScore);

    mvprintw(0, 10, "lv ");
    mvprintw(0, 13, "%d", PD.score.lvl);
    mvprintw(0, 14, "%c", '/');
    mvprintw(0, 15, "%d", PD.score.MapSize);

}


//Player info section
PlayerData SetPlayerData()
{
    echo();
    int i = 0;
    char ch;
    PlayerData p;
    p.score = CreateDefaultScore();
    echo();
    printw("WELCOM TO JUMPING FROG ");
    move(2, 0);
    printw("input your name: ");
    getstr(p.name);
    printw("input your surname: ");
    getstr(p.surname);
    printw("input your ID: ");
    getstr(p.ID);
    clear();
    noecho();
    return p;
}
void ShowPlayer(PlayerData &PD)
{
    move(27, 0);
    printw("Creator: Wiktor Adamczyk 203519");
    move(23, 1);
    printw( PD.name);
    printw(" ");
    printw( PD.surname);
    printw(" ");
    printw( PD.ID);
}


//kolor section
void KolorInit()
{
    init_color(12, 600, 600, 600);
    init_color(13, 400, 700, 610);
    init_pair(1, COLOR_BLACK, COLOR_YELLOW);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_CYAN);
    init_pair(4, COLOR_BLACK, COLOR_RED);
    init_pair(5, COLOR_GREEN, COLOR_GREEN);
    init_pair(6, COLOR_BLACK, COLOR_WHITE);
    init_pair(7, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(8, COLOR_MAGENTA, COLOR_GREEN);
    init_pair(9, 12, 12);
    init_pair(10,COLOR_BLACK, 13);

}
void DefineKolors(GameData &GD)
{
    KolorInit();
    GD.CFKolor = 7;
    GD.OKolor = 6;
    GD.GKolor = 5;
    GD.DKolor = 1;
    GD.SKolor = 2;
    GD.CKolor = 3;
    GD.CEKolor = 4;
    GD.FlKolor = 9;
    GD.STKolor = 10;

}
void GiveKolor(int x,int y,int kp,char znak)
{
    attron(COLOR_PAIR(kp));
    mvprintw(x, y, "%c", znak);
    attroff(COLOR_PAIR(kp));
}


// map section 
void SetMap(GameData GD)
{
    for (int i = 1; i <= GD.w;i++)
    {
        bool is = false;
        for (int k = 0;k < GD.w / 4;k++) {
            
            if (i == GD.FreeLines[k]+1)
            {
                is = true;
            }
        }
        for (int j = 1; j <= GD.k;j++)
        {
                if (is ==true)
                {
                    GiveKolor(i, j, GD.FlKolor, '0');
                }
                else if (i == 1 or i == GD.w)
                {
                    GiveKolor(i, j, GD.GKolor, '0');
                }
                else
                {
                    GiveKolor(i, j, GD.SKolor, '-');
                }
        }
    }
    
    refresh();
}


//duck SHOW/CLEAR section
void DuckClear(GameData &GD)
{
    bool is = false;
    for (int k = 0;k < GD.w / 4;k++) {

        if (GD.x == GD.FreeLines[k]+1)
        {
            is = true;
        }
    }
    if (is == true)
    {
        GiveKolor(GD.x, GD.y, GD.FlKolor, '0');
    }
    else if (GD.x == GD.w || GD.x == 1)
    {
        GiveKolor(GD.x, GD.y,GD.GKolor, '0');
    }
    else GiveKolor(GD.x, GD.y, GD.SKolor, '-');
}
void DuckShow(GameData& GD)
{
    GiveKolor(GD.x, GD.y,GD.DKolor, GD.SDuck);
}


//cars section
Car CreateCars(GameData &GD,int i)
{
    Car c;
    c.a = rand() % 1000 > 499 ? 1 : -1;
    c.size = rand() % 6 + 1;
    c.x = i;
    c.y = c.a > 0 ? 0 : GD.k+1;

    int r = rand() % 100;
    if (r < 50) c.type = 0;
    else if (r < 80) c.type = 1;
    else c.type = 2;

    c.V = rand() % 7 + 1;
    c.cc = rand() & 100;
    return c;
}
bool CarTouchO(GameData& GD, Car &Car, Obstacle o[])
{
        for (int i = 0; i < GD.ObstacleNumber;i++)
        {
            for (int w = 0;w < o[i].w;w++)
            {
                for (int k = 0;k < o[i].k;k++)
                {
                    if(Car.x == o[i].x + w && o[i].y + k == Car.y) return true;
                }
            }
        }
    return false;
}
void CarOutside(int a, Car &car, GameData &GD,int i)
{
    if (a == -1)
    {
        if (car.y <= 0)
        {
            for (int j = car.y;j <= 0;j++)
            {
                mvprintw(car.x, j, "%c", ' ');
            }
            if (car.y + car.size < 0)
            {
                car = CreateCars(GD, i + 1);
            }
        }
    }
    else
    {
        if (car.y>= GD.k)
        {
            for (int j = GD.k+1;j <= car.y;j++)
            {
                mvprintw(car.x, j, "%c", ' ');
            }
            if (car.y - car.size > GD.k)
            {
                car = CreateCars(GD, i + 1);
            }
        }
    }
}
void CarSlowingChance(Car &car,GameData &GD)
{
    if (car.x == GD.x)
    {
        if(car.a == -1)
        { 
            if(GD.y < car.y){
                if (car.cc < 50)
                {
                    if (car.type == 0)
                    {
                        car.V *= 5;
                    }
                }
                if (car.cc < 50)
                {
                    if (car.type == 1)
                    {
                        car.V = 1;
                    }
                }
            }
        }
        if (car.a == 1)
        {
            if (GD.y > car.y) {
                if (car.cc < 50)
                {
                    if (car.type == 0)
                    {
                        car.V *= 5;
                    }
                }
                if (car.cc < 50)
                {
                    if (car.type == 1)
                    {
                        car.V = 1;
                    }
                }
            }
        }
    }
    else
    {
        if (car.V > 50)
        {
            car.V = rand() % 10 + 1;
        }
    }
}
void ShowCar(GameData &GD,Obstacle o[],Car &car)
{
    if (car.a == -1) //kierunek lewo
    {
        if (!(car.y + car.size > GD.k))
        {
            mvprintw(car.x, car.y + car.size, "%c", '-');
        }
        else
        {
            mvprintw(car.x, car.y + car.size, "%c", ' ');
        }

        car.y += car.a;

        if (CarTouchO(GD, car, o) == true)
        {
            car.y = car.y + car.size + 1;
            car.a *= -1;
        }
        //mvprintw(Cars[i].x, Cars[i].y, "%c", GD.SCar);

    }
    else if (car.a == 1)
    {
        if (!(car.y - car.size < 1))
        {
            mvprintw(car.x, car.y - car.size, "%c", '-');
        }
        else
        {
            mvprintw(car.x, car.y - car.size, "%c", ' ');
        }

        car.y += car.a;

        if (CarTouchO(GD, car, o) == true)
        {
            car.y = car.y - car.size - 1;
            car.a *= -1;
        }
        //mvprintw(Cars[i].x, Cars[i].y, "%c", GD.SCar);

    }
}
void UpdateCars(GameData &GD, Car Cars[], int time,Obstacle o[])
{
    for (int i = 1; i < GD.w-1;i++)
    {
        if (time % Cars[i].V == 0 ) //czy moze jechać
        {
            ShowCar(GD, o, Cars[i]);
            GiveKolor(Cars[i].x, Cars[i].y, Cars[i].type == 1 ? GD.CEKolor : Cars[i].type == 2 ? GD.CFKolor : GD.CKolor, GD.SCar);
            CarOutside(Cars[i].a, Cars[i], GD, i);
        }
        CarSlowingChance(Cars[i], GD);
    }
}
//friendly Car TP
bool LineToTp(GameData& GD) 
{
    for (int i = GD.x;i>0;i--)
    {
        for(int j =0 ; j< GD.w/2;j++)
        if (GD.FreeLines[j]+1 == i)
        {
            DuckClear(GD);
            GD.x = i;
            DuckShow(GD);
            refresh();
            return true;
        }
    }
    return false;

}
bool CanBeTp(GameData &GD,Car &car, bool& st)
{
    if (st ==true &&car.type == 2)
    {
        if (car.x == GD.x)
        {
            if (car.a == -1)
            {
                for (int j = car.y - 1;j < car.size+2 + car.y;j++)
                {
                    if (GD.y == j)
                    {
                        if (LineToTp(GD) != true)
                        {
                            return true;
                        }
                        else return false;
                    }
                }
                return false;
            }
            else if (car.a == 1)
            {
                for (int j = car.y + 1;j > car.y - car.size-1;j--)
                {
                    if (GD.y == j)
                    {
                        if (LineToTp(GD) != true)
                        {
                            return true;
                        }
                        else return false;
                    }
                }
                return false;
            }
        }
    }
    return false;
}


//duck COLISION section
bool DuckColisionCar(GameData& GD, Car Cars[],bool &st)
{
    for (int i = 1; i < GD.w-1;i++)
    {
        if (CanBeTp(GD, Cars[i],st) == true && st == true) return false;
        else             DuckShow(GD);;
        for (int j = 0; j <= Cars[i].size;j++)
        {
            if (Cars[i].x == GD.x && Cars[i].y - Cars[i].a*j == GD.y)
            {
                return true;
            }
        }
    }
    return false;
}


//obstacle section
Obstacle CreateObstacle(GameData &GD)
{
    Obstacle o;
    o.w = rand()%3 +1;
    o.k= rand() % 3 + 1;
    o.x = rand() % (GD.w -4) + 2;
    o.y = rand() % (GD.k -4)+2;
    return o;
}
void ShowObstacle(Obstacle o[], GameData &GD)
{
    for (int i = 0; i < GD.ObstacleNumber;i++)
    {
        for (int w = 0;w < o[i].w;w++)
        {
            for (int k = 0;k < o[i].k;k++)
            {
                GiveKolor(o[i].x +w, o[i].y + k, GD.OKolor, '8');
            }
        }
    }
}
bool OTouchD (GameData& GD, Obstacle o[])
{
    for (int i = 1; i < GD.w - 1;i++)
    {
        if (i < GD.ObstacleNumber)
        {
            for (int w = 0;w < o[i].w;w++)
            {
                for (int k = 0;k < o[i].k;k++)
                {
                    if (o[i].x + w == GD.x && o[i].y + k == GD.y)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


//stork section
void ShowStork(stork &s, GameData &GD)
{
    GiveKolor(s.x, s.y, GD.STKolor,GD.SS);
}
void ClearStork(stork &s, GameData& GD, Obstacle o[], Car cars[])
{
    //czy auto
    for (int i = 1; i < GD.w - 1;i++)
    {
        if (cars[i].x == s.x)
        {
            if (cars[i].a == -1)
            {
                if (s.y < cars[i].y + cars[i].size && cars[i].y < s.y)
                {
                    GiveKolor(s.x, s.y, cars[i].type == 1 ? GD.CEKolor : cars[i].type == 2 ? GD.CFKolor : GD.CKolor, GD.SCar);
                    return;
                }
            }
            else
            {
                if (s.y > cars[i].y - cars[i].size && cars[i].y > s.y)
                {
                    GiveKolor(s.x, s.y, cars[i].type == 1 ? GD.CEKolor : cars[i].type == 2 ? GD.CFKolor : GD.CKolor, GD.SCar);
                    return;
                }
            }
        }
    }
    //czy linia wolna
    for (int k = 0;k < GD.w / 6;k++)
    {
        if (s.x == GD.FreeLines[k] + 1)
        {
            GiveKolor(s.x, s.y,GD.FlKolor,'0');
            return;
        }
    }
    //czy przeszkoda na lini
    for (int i = 0; i < GD.ObstacleNumber;i++)
    {
        for (int w = 0;w < o[i].w;w++)
        {
            for (int k = 0;k < o[i].k;k++)
            {
                if (o[i].x + w == s.x && o[i].y + k == s.y)
                {
                    GiveKolor(s.x,s.y, GD.OKolor, '8');
                    return;
                }
            }
        }
    }
    if (s.x == 1 or s.x == GD.w)
    {
         GiveKolor(s.x, s.y, GD.GKolor, '0');
         return;
    }
    else if (s.x < GD.w && s.x>1)
    {
        mvprintw(s.x, s.y,"%c", '-');
        return;
    }

    
}
void StorkFollow(stork &s,int &t, GameData& GD, Obstacle o[], Car cars[])
{
    //makapaka lubi kamienie
    if (t > 50)
    {
        //4 ty jebany betoniarzu 
        if (t % 2 == 0)
        {
            int r = rand() % 2;
            ClearStork(s, GD, o, cars);
            if (r < 1)
            {
                //skibidi toilet sigma W rizz yap yap yap
                if (s.y < GD.y)
                {
                    s.y = s.y +1;
                }
                else if (s.y > GD.y)
                {
                    s.y = s.y-1;
                }
            }
            else
            {
                if (s.x > GD.x)
                {
                    s.x = s.x -1;
                }
                else if (s.x < GD.x)
                {
                    s.x = s.x+1;
                }
            }
            ShowStork(s, GD);
        }
    }
}
bool StorkColision(stork &s, GameData& GD)
{
    if (s.x == GD.x && s.y == GD.y)
    {
        return true;
    }
    return false;
}


//game sequence section
void GUIDE()
{
    move(1, 22);
    printw("WELCOME TO JUMPING FROG");
    move(2, 25);
    printw("GAME IS ENDLESSS");
    move(4, 0);
    printw("THE GOAL IS TO ACHIVE THE GREEN LINE ON TOP AS MANY TIMES AS POSSIBLE");
    GiveKolor(6, 12, 1, 'D');
    printw(" THAT'S YOU");
    GiveKolor(7, 12, 7, '#');
    printw(" Friendly car");
    GiveKolor(8, 12, 4, '#');
    printw(" Teleport to nearest pavement nn demand by steping in friendly car");
    GiveKolor(9, 12, 8, '#');
    printw(" Enemy car");
    GiveKolor(10, 12, 3, '#');
    printw(" Neutral car");
    GiveKolor(12, 12, 9, '0');
    printw(" Pavement");
    GiveKolor(12, 12, 5, '0');
    printw(" START AND GOAL");
    GiveKolor(13, 12, 2, '-');
    printw(" Street");
    GiveKolor(14, 12, 6, '8');
    printw(" Obstacle");
    mvprintw(16, 0, "Press any key to continue");
    while (true)
    {
        char ch = getch();
        if (ch != ERR)
        {
            clear();
            return;
        }
    }

}
void DuckMovement(GameData &GD,Obstacle o[],bool& SHOWTIME, int& MAGICTIME,int time)
{
    char ch;
    ch = getch();
    if (ch != ERR)
    {
        DuckClear(GD);
        //movement //toodooo Duck time 


        if (ch == 'w' && GD.x > 1)
        {
            GD.x -= 1;
            if (OTouchD(GD, o) == true) GD.x += 1;

            }
        else if (ch == 's' && GD.x <= GD.w - 1)
        {
            GD.x += 1;
            if (OTouchD(GD, o) == true) GD.x -= 1;;

            }
        else if (ch == 'd' && GD.y <= GD.k - 1)
        {
            GD.y += 1;
            if (OTouchD(GD, o) == true) GD.y -= 1;
            }
        else if (ch == 'a' && GD.y > 0 + 1)
        {
            GD.y -= 1;
            if (OTouchD(GD, o) == true)  GD.y += 1;

        }
        else if (ch == 27) return;
        else if (ch == 32)
        {
            GD.CFKolor = 8;
            SHOWTIME = true;
            MAGICTIME = time;
        }
        DuckShow(GD);
    }
}
bool GameLoop(GameData& GD, Obstacle o[], Car Cars[],PlayerData& PD,stork &s)
{
    // sigma sigma boy sigma boy sigma boy
    ShowScore(PD);
    bool SHOWTIME = false;
    int MAGICTIME, time = 0;
    while (true)
    {
        //while dotkniety
        DuckMovement(GD, o, SHOWTIME, MAGICTIME, time);

        sleep(100);
        bool CanTP = true;
        //MAGIC CARS
        if (SHOWTIME == true)
        {
            
            if (time - MAGICTIME == 5 * 10)
            {
                SHOWTIME = false;
                GD.CFKolor = 7;
            }
        }
        ShowTime(time,SHOWTIME);
        //STYKANIE SIE KONCOWKAMI
        if(DuckColisionCar(GD, Cars,SHOWTIME) == true) return false;

        //czass
        time++;

        // BOCIAN 0,7L
        if (StorkColision(s,GD))return false;
        StorkFollow(s, time, GD, o, Cars);

        //auta brumm brumm
        UpdateCars(GD, Cars, time, o);

        //update player score
        if (PD.score.HighestGameScore < PD.score.CurrentGameScore)
        {
            PD.score.HighestGameScore = PD.score.CurrentGameScore;
        }
        if (GD.x == 1)
        {
            PD.score.CurrentGameScore += 1 *10;
            return true;
        }

        refresh();
    }
}
void GamePlay(PlayerData& PD,bool guideShowed = false)
{
    //GAME INIT
    start_color();
    if (!has_colors()) {
        endwin();  // Kończymy program, jeśli terminal nie wspiera kolorów
        printw("brak kolorów");
        return ;
    }
    GameData GD = LoadData();
    DefineKolors(GD);
    if (guideShowed == false)  GUIDE();
    SetMap(GD);
    Car* Cars = new Car[GD.w-1];
    Obstacle* o = new Obstacle[GD.w -1];
    stork s = { GD.w,0 };

    for (int i = 1;i < GD.w-1;i++)
    {
        bool is = false;
        for (int k = 0;k < GD.w / 6;k++) {

            if (i == GD.FreeLines[k])
            {
                is = true;
            }
        }
        o[i] = CreateObstacle(GD);
        if(is == false) Cars[i] = CreateCars(GD, i + 1);
    }
    DuckShow(GD);
    ShowObstacle(o, GD);
    ShowPlayer(PD);
    nodelay(stdscr, TRUE);

    //GAME LOOP
   
    bool gra = GameLoop(GD,o,Cars,PD,s);
    
    //save highest score
    
    delete[]Cars;
    clear();
    if (gra == false)
    {
        PD.score.HighestGameScore = MaksScore(PD.score.CurrentGameScore, PD.score.HighestGameScore);
        PD.score.CurrentGameScore = 0;
        PD.score.lvl = 0;
        PD.score.MapSize = 1;
        return;
    }
    else
    {
        if(PD.score.lvl == PD.score.MapSize)
        {
            PD.score.MapSize += 1;
            PD.score.lvl = 0;
        }
        else PD.score.lvl += 1;
        GamePlay(PD,true);
    }
    return;
}


//settings section
void ShowSettings()
{
    mvprintw(0, 1, "SETTINGS");
    mvprintw(1, 1, "CHANGE USER");
    mvprintw(2, 1, "SAVE HIGHEST SCORE");
    mvprintw(3, 1, "BACK");

}
void Settings(PlayerData &PD,Score score)
{
    nodelay(stdscr, TRUE);
    char ch;
    int x = 1;
    while (true)
    {
        ShowSettings();
        mvprintw(x, 0, ">");
        ch = getch();
        if (ch != ERR)
        {
            if (ch == 'w')
            {
                mvprintw(x, 0, " ");
                x -= 1;
                mvprintw(x, 0, ">");
            }
            else if (ch == 's')
            {
                mvprintw(x, 0, " ");
                x += 1;
                mvprintw(x, 0, ">");
            }
            else if (ch == 10)
            {
                clear();
                switch (x)
                {
                    case 1:
                    {
                        PD = SetPlayerData();
                    }
                    case 2:
                    {
                       SaveScoreToFile(PD);
                    }
                    case 3:
                    {
                        return;
                    }
                }
            }
            if (x > 3)
            {
                mvprintw(x, 0, " ");
                x %= 3;
                mvprintw(x, 0, " ");
                mvprintw(x, 0, ">");
            }
            if (x < 1)
            {
                mvprintw(x, 0, " ");
                x += 3;
                mvprintw(x, 0, " ");
                mvprintw(x, 0, ">");
            }
        }
        refresh();
    }
}


//menu section
void ShowOptions()
{
    mvprintw(0, 1, "WITAJ W KACZUSZCE");
    mvprintw(1, 1, "PLAY");
    mvprintw(2, 1, "SETTINGS");
    mvprintw(3, 1, "SCOREBOARD");
}
void ShowMenu()
{
    //Game logic
    //global score and user name

    PlayerData PD = SetPlayerData();
    char ch;
    int x = 1;

    while (true)
    {
        ShowOptions();
        mvprintw(x, 0, ">");
        ch = getch();
        if (ch != ERR)
        {
            if (ch == 27) return ;
            else if (ch == 'w')
            {
                mvprintw(x, 0, " ");
                x -= 1;
                mvprintw(x, 0, ">");
            }
            else if (ch == 's')
            {
                mvprintw(x, 0, " ");
                x += 1;
                mvprintw(x, 0, ">");
            }
            else if (ch == 10)
            {
                clear();
                if (x == 1) GamePlay(PD);
                else if (x == 2) Settings(PD, PD.score);
                else if (x == 3) displayScoreBoard();
            }
            if (x > 3)
            {
                mvprintw(x, 0, " ");
                x %= 3;
                mvprintw(x, 0, " ");
                mvprintw(x, 0, ">");
            }
            if (x < 1)
            {
                mvprintw(x, 0, " ");
                x += 3;
                mvprintw(x, 0, " ");
                mvprintw(x, 0, ">");
            }
        }
        refresh();
    }
}


//main 
int main() {
    //PDcureses init
    initscr();              
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    start_color();
    ShowMenu();
    




    getch(); // nie zakończenie programu
    endwin();        
    return 0;
}