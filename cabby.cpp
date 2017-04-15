#include "rlutil.h"
#include <time.h>
#include <stdio.h>

#define DEFAULT_VIEW_WIDTH 50
#define DEFAULT_VIEW_HEIGHT 20
#define DEFAULT_VEHICLE_COUNT 20
#define DEFAULT_TIME 500
#define DEFAULT_FARE_COUNT 3

//The maximum amount of turns a fare
//will wait to be picked up.
#define MAX_FARE_WAIT 75
//What a wall character looks like
#define WALL_0 '#'
#define WALL_1 '\\'
#define WALL_2 '_'
//Offset to account for left and 
//right, top and bottom borders
#define BORDER_OFFSET 2

static const char *builtin[] = {
    "#############################################",
    "#6444444446444444454444444644444445444444445#",
    "#298888888:888888898888888:888888898888888:1#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#:98888888:888888898888888:888888898888888:9#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#6544444446444444454444444644444445444444465#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#21#######2#######1#######2#######1#######21#",
    "#2544444446444444454444444644444445444444461#",
    "#:88888888:888888898888888:88888889888888889#",
    "#############################################"
};

enum Direction {
    none  = 0,
    up    = 1,
    down  = 2,
    left  = 4,
    right = 8
};

struct Tile {
    char character;
    int  direction;
    int posx;
    int posy;
    bool occupied;
};

struct Vehicle {
    char character;
    int direction;
    int posx;
    int posy;
    bool player;
    bool hasfare;
    int money;
};

struct Fare {
    char character;
    int money;
    int posx;
    int posy;
    int destx;
    int desty;
    int wait;
    bool pickedup;
};

void drawgameover(int width, int height, Vehicle* player)
{
    int halfx = (width/2);
    int halfy = (height/2);
    
    const char* empty = " ";
    const char* gameover = "GAME OVER!";
    int halftxtlen = 5; //half the length of the gameover text.
    
    for(int x=0; x < width; x++) {
        rlutil::locate(x+BORDER_OFFSET, halfy-1);
        std::cout << '-';
        
        rlutil::locate(x+BORDER_OFFSET, halfy);
        if(x == (halfx-halftxtlen)) {
            std::cout << gameover;
        } else if(x < (halfx-halftxtlen) || x >= (halfx+halftxtlen)) {
            std::cout << empty;
        }
        
        rlutil::locate(x+BORDER_OFFSET, halfy+1);
        std::cout << '-';
    }
}

void drawdebug(int width, int height, Tile* tiles, int sizex, int sizey, Vehicle* p, Fare* fares, int farecount)
{
    int s = height+BORDER_OFFSET;
    rlutil::locate(0, s+1);
    Tile t = tiles[(p->posx)*sizey+(p->posy-1)];
    std::cout << " PT:" << (t.occupied ? "oc":"no") << " dir=" << t.direction << " x=" << (p->posx) << " y=" << (p->posy-1);
    t = tiles[(p->posx+1)*sizey+(p->posy)];
    std::cout << " PR:" << (t.occupied ? "oc":"no") << " dir=" << t.direction << " x=" << (p->posx+1) << " y=" << (p->posy);
    t = tiles[(p->posx)*sizey+(p->posy+1)];
    std::cout << " PB:" << (t.occupied ? "oc":"no") << " dir=" << t.direction << " x=" << (p->posx) << " y=" << (p->posy+1);
    t = tiles[(p->posx-1)*sizey+(p->posy)];
    std::cout << " PL:" << (t.occupied ? "oc":"no") << " dir=" << t.direction << " x=" << (p->posx-1) << " y=" << (p->posy);
    
    rlutil::locate(0, s+2);
    for(int i=0; i < farecount; i++) {
        Fare f = fares[i];
        std::cout << " F" << i << ":" << (f.pickedup ? "pu":"no") << " wait=" << f.wait << " px=" << f.posx << " py=" << f.posy << " dx=" << f.destx << " dy=" << f.desty;
    }
}

void drawstatus(int width, int height, Vehicle* player, int time)
{
    width += BORDER_OFFSET * 2;
    height += BORDER_OFFSET * 2;
    rlutil::locate(width, 1);
    std::cout << "CONSOLE CABBIE";
    rlutil::locate(width, 3);
    std::cout << "TIME:  " << time;
    rlutil::locate(width, 4);
    std::cout << "MONEY: " << player->money;
    
    rlutil::locate(width, 8);
    std::cout << "WASD to move, Q to quit.";
    rlutil::locate(width, 10);
    std::cout << "Pick up fares(" << rlutil::ANSI_GREEN << '$' << rlutil::ANSI_ATTRIBUTE_RESET << ") and drop them off";
    rlutil::locate(width, 11);
    std::cout << "at the destination(" << rlutil::ANSI_RED << 'X' << rlutil::ANSI_ATTRIBUTE_RESET << ") before";
    rlutil::locate(width, 12);
    std::cout << "time runs out.";
    
    if(player->hasfare) {
        rlutil::locate(width, 6);
        std::cout << rlutil::ANSI_RED << "Meter is running" << rlutil::ANSI_ATTRIBUTE_RESET;
    }
}

void drawview(int width, int height, Tile* tiles, int sizex, int sizey, Vehicle* cars, int carcount, Fare* fares, int farecount)
{
    //Draw the borders around the viewing window
    for(int i=2; i < height+BORDER_OFFSET; i++) {
        rlutil::locate(1, i);
        std::cout << '|';
        rlutil::locate(width+BORDER_OFFSET, i);
        std::cout << '|';
    }
    for(int i=1; i <= width+BORDER_OFFSET; i++) {
        if(i == 1 || i == width+BORDER_OFFSET) {
            rlutil::locate(i, 1);
            std::cout << '+';
            rlutil::locate(i, height+BORDER_OFFSET);
            std::cout << '+';
        } else {
            rlutil::locate(i, 1);
            std::cout << '-';
            rlutil::locate(i, height+BORDER_OFFSET);
            std::cout << '-';
        }
    }
    
    //Next draw the map centered on the player.
    int offsetx = cars[0].posx-(width/2);
    int offsety = cars[0].posy-(height/2);
    for(int y=offsety; y < height+offsety; y++) {
        for(int x=offsetx; x < width+offsetx; x++) {
            char c = (x >= 0 && y >= 0 && x < sizex && y < sizey) ? tiles[x*sizey+y].character : WALL_0;
            int shiftedx = x-offsetx;
            int shiftedy = y-offsety;
            
            if(x >= 0 && y >= 0 && x < sizex && y < sizey && tiles[x*sizey+y].direction != none) {
                c = ' ';
            }
            
            if(shiftedx < width) {
                rlutil::locate(shiftedx+BORDER_OFFSET, shiftedy+BORDER_OFFSET);
                std::cout << c;
            }
            
        }
    }
    
    //Now draw the cars
    for(int i=0; i < carcount; i++) {
        int shiftedx = cars[i].posx-offsetx;
        int shiftedy = cars[i].posy-offsety;
        
        if(shiftedx >= 0 && shiftedx < width && shiftedy >= 0 && shiftedy < height) {
            rlutil::locate(shiftedx+BORDER_OFFSET, shiftedy+BORDER_OFFSET);
            std::cout << cars[i].character;
        }
    }
    
    //Finally draw the fares
    for(int i=0; i < farecount; i++) {
        int shiftedx = fares[i].posx;
        int shiftedy = fares[i].posy;
        
        rlutil::setColor(rlutil::GREEN);
        
        if(fares[i].pickedup) {
            shiftedx = fares[i].destx;
            shiftedy = fares[i].desty;
            
            rlutil::setColor(rlutil::RED);
        }
        
        shiftedx -= offsetx;
        shiftedy -= offsety;
        
        if(shiftedx >= 0 && shiftedx < width && shiftedy >= 0 && shiftedy < height) {
            rlutil::locate(shiftedx+BORDER_OFFSET, shiftedy+BORDER_OFFSET);
            std::cout << fares[i].character;
        } else if(fares[i].pickedup) {
            shiftedx = (shiftedx < 0) ? -1 : shiftedx;
            shiftedx = (shiftedx > width) ? width : shiftedx;
            
            shiftedy = (shiftedy < 0) ? -1 : shiftedy;
            shiftedy = (shiftedy > height) ? height : shiftedy;
            
            rlutil::locate(shiftedx+BORDER_OFFSET, shiftedy+BORDER_OFFSET);
            std::cout << fares[i].character;
        }
    }
    
    rlutil::resetColor();
}

int getrandomdirection()
{
    int mark = rand()%4;
    
    if(mark == 0)return up;
    else if(mark == 1)return right;
    else if(mark == 2)return down;
    return left;
}

int getautomateddirection(Tile *tiles, int sizex, int sizey, int x, int y, int dir, bool reverse)
{
    int want;
    do {
        want = getrandomdirection();
    } while(!(dir & want));
    
    int newx = x;
    int newy = y;
    
    if(want == up)        newy += (reverse ? 1 : -1);
    else if(want == down) newy += (reverse ? -1 : 1);
    else if(want == left) newx += (reverse ? 1 : -1);
    else if(want == right)newx += (reverse ? -1 : 1);
    
    //If the wanted position is invalid then try to find another.
    if(newx < 0 || newy < 0 || newx >= sizex || newy >= sizey || tiles[newx*sizey+newy].occupied) {
        want = dir - want;
        newx = x;
        newy = y;
        
        if(want == up)        newy += (reverse ? 1 : -1);
        else if(want == down) newy += (reverse ? -1 : 1);
        else if(want == left) newx += (reverse ? 1 : -1);
        else if(want == right)newx += (reverse ? -1 : 1);
    }
    
    //If we can't find another direction then return none.
    if(want == none || newx < 0 || newy < 0 || newx >= sizex || newy >= sizey || tiles[newx*sizey+newy].occupied)return none;
    
    return want;
}

Tile* findrandommovabletile(Tile* tiles, int sizex, int sizey, int dir)
{
    int randx = rand() % sizex;
    int randy = rand() % sizey;
    
    if((tiles[randx*sizey+randy].direction & dir) && !tiles[randx*sizey+randy].occupied) {
        return &tiles[randx*sizey+randy];
    }
    
    // Check the neighbors
    if(randx < sizex-1 && (tiles[(randx+1)*sizey+randy].direction & dir) && !tiles[(randx+1)*sizey+randy].occupied) {
        return &tiles[(randx+1)*sizey+randy];
    } else if(randx > 0 && (tiles[(randx-1)*sizey+randy].direction & dir) && !tiles[(randx-1)*sizey+randy].occupied) {
        return &tiles[(randx-1)*sizey+randy];
    } else if(randy < sizey-1 && (tiles[randx*sizey+(randy+1)].direction & dir) && !tiles[randx*sizey+(randy+1)].occupied) {
        return &tiles[randx*sizey+(randy+1)];
    } else if(randy > 0 && (tiles[randx*sizey+(randy-1)].direction & dir) && !tiles[randx*sizey+(randy-1)].occupied) {
        return &tiles[randx*sizey+(randy-1)];
    }
    
    // Check all remaining tiles ahead inclusive
    for(int x=randx; x < sizex; x++) {
        for(int y=randy; y < sizey; y++) {
            if((tiles[x*sizey+y].direction & dir) && !tiles[x*sizey+y].occupied) {
                return &tiles[x*sizey+y];
            }
        }
    }
    
    // Check all remaining tiles behind
    for(int x=randx-1; x >= 0; x--) {
        for(int y=randy-1; y >= 0; y--) {
            if((tiles[x*sizey+y].direction & dir) && !tiles[x*sizey+y].occupied) {
                return &tiles[x*sizey+y];
            }
        }
    }
    
    // Couldn't find anything.
    return NULL;
}

void movecars(Tile* tiles, int sizex, int sizey, Vehicle* cars, int carcount, char key, bool reverse)
{
    for(int i=carcount-1; i >= 0; i--) {
        Tile* t = &tiles[cars[i].posx*sizey+cars[i].posy];
        int dir = t->direction;
        
        // If the car is currently on a non-road tile then move it to a road tile
        if(dir == none) {
            dir = getrandomdirection();
            
            t = findrandommovabletile(tiles, sizex, sizey, dir);
            if(t == NULL)break;
            
            cars[i].direction = dir;
            cars[i].posx = t->posx;
            cars[i].posy = t->posy;
            t->occupied = true;
        }
        
        int x = cars[i].posx;
        int y = cars[i].posy;
        
        if(cars[i].player) {
            if(key == 'w')y--;
            else if(key == 's')y++;
            else if(key == 'a')x--;
            else if(key == 'd')x++;
        } else {
            //If the tile's direction and the car's direction are in agreement,
            //move in that direction. Otherwise try to get another direction.
            dir = (dir == cars[i].direction) ?
                    cars[i].direction :
                    getautomateddirection(tiles, sizex, sizey, x, y, dir, reverse);
            
            if(dir & up)        y += (reverse ? 1 : -1);
            else if(dir & down) y += (reverse ? -1 : 1);
            else if(dir & left) x += (reverse ? 1 : -1);
            else if(dir & right)x += (reverse ? -1 : 1);
            else continue; //this car can't move, try again next time.
        }
        
        if(x >= 0 && y >= 0 && x < sizex && y < sizey && !tiles[x*sizey+y].occupied && tiles[x*sizey+y].direction != none) {
            tiles[x*sizey+y].occupied = true;
            t->occupied = false;
            
            cars[i].direction = dir;
            cars[i].posx = x;
            cars[i].posy = y;
        }
    }
}

Tile* findrandomfaretile(Tile* tiles, int sizex, int sizey)
{
    int dir = getrandomdirection();
    Tile* t = findrandommovabletile(tiles, sizex, sizey, dir);
    
    if(t == NULL) {
        return NULL;
    }
    
    int x = t->posx;
    int y = t->posy;
    
    do {
        // Check the neighbors
        if(x < sizex-1 && tiles[(x+1)*sizey+y].direction == none && !tiles[(x+1)*sizey+y].occupied) {
            return &tiles[(x+1)*sizey+y];
        } else if(x > 0 && tiles[(x-1)*sizey+y].direction == none && !tiles[(x-1)*sizey+y].occupied) {
            return &tiles[(x-1)*sizey+y];
        } else if(y < sizey-1 && tiles[x*sizey+(y+1)].direction == none && !tiles[x*sizey+(y+1)].occupied) {
            return &tiles[x*sizey+(y+1)];
        } else if(y > 0 && tiles[x*sizey+(y-1)].direction == none && !tiles[x*sizey+(y-1)].occupied) {
            return &tiles[x*sizey+(y-1)];
        }
        
        //Didn't find any good spots so move along the road.
        dir = getautomateddirection(tiles, sizex, sizey, x, y, tiles[x*sizey+y].direction, false);
        
        if(dir & up)y--;
        else if(dir & down)y++;
        else if(dir & left)x--;
        else if(dir & right)x++;
        
    } while(dir != none && x >= 0 && y >= 0 && x < sizex && y < sizey);
    
    return NULL;
}

void spawnfares(Tile* tiles, int sizex, int sizey, Fare* fares, int farecount, Vehicle* player)
{
    for(int i=0; i < farecount; i++) {
        if(fares[i].wait > 0) {
            fares[i].wait--;
        } else if(!fares[i].pickedup) {
            Tile* pt = findrandomfaretile(tiles, sizex, sizey);
            Tile* dt = findrandomfaretile(tiles, sizex, sizey);
            
            //Remove the old occupied destinations
            tiles[fares[i].posx*sizey+fares[i].posy].occupied = false;
            tiles[fares[i].destx*sizey+fares[i].desty].occupied = false;
            
            //If we couldn't find a place for this fare then skip it.
            if(pt == NULL || dt == NULL)continue;
            
            //Mark the new tiles as occupied.
            tiles[pt->posx*sizey+pt->posy].occupied = true;
            tiles[dt->posx*sizey+dt->posy].occupied = true;
            //Set the new fare position
            fares[i].posx = pt->posx;
            fares[i].posy = pt->posy;
            fares[i].destx = dt->posx;
            fares[i].desty = dt->posy;
            
            //Set the new fare characteristics.
            fares[i].wait = (rand() % MAX_FARE_WAIT) + 1;
            fares[i].money = (rand() % 20) + 1;
        }
        
        bool byposx = player->posx == fares[i].posx-1 ||
                      player->posx == fares[i].posx+1;
        
        bool bydestx = player->posx == fares[i].destx-1 ||
                       player->posx == fares[i].destx+1;
        
        bool byposy = player->posy == fares[i].posy-1 ||
                      player->posy == fares[i].posy+1;
        
        bool bydesty = player->posy == fares[i].desty-1 ||
                       player->posy == fares[i].desty+1;
        
        //If the player is next to a fare and doesn't already have one, then pick the fare up.
        if(!player->hasfare && ((byposx && player->posy == fares[i].posy) || (byposy && player->posx == fares[i].posx))) {
            player->hasfare = true;
            fares[i].pickedup = true;
            fares[i].wait = 0;
            fares[i].character = 'X';
            tiles[fares[i].posx*sizey+fares[i].posy].occupied = false;
        //If this fare is picked up and the player is next to its destination then drop the fare off.
        } else if(fares[i].pickedup && ((bydestx && player->posy == fares[i].desty) || (bydesty && player->posx == fares[i].destx))) {
            player->hasfare = false;
            player->money += fares[i].money;
            fares[i].pickedup = false;
            fares[i].wait = 0;
            fares[i].character = '$';
            fares[i].posx = fares[i].destx;
            fares[i].posy = fares[i].desty;
        }
    }
}

void add3dcharacters(Tile* tiles, int sizex, int sizey)
{
    for(int x=0; x < sizex; x++) {
        for(int y=0; y < sizey; y++) {
            if(tiles[x*sizey+y].direction == none) {
                continue;
            }
            
            bool topblocked = (y > 0 && tiles[x*sizey+(y-1)].direction == none);
            bool sideblocked = (x > 0 && tiles[(x-1)*sizey+y].direction == none);
            
            //Is this an enclosed corner?
            if(topblocked && sideblocked) {
                tiles[(x-1)*sizey+(y-1)].character = WALL_1;
                tiles[x*sizey+(y-1)].character = WALL_2;
                tiles[(x-1)*sizey+y].character = WALL_1;
            } else if(topblocked) {
                tiles[x*sizey+(y-1)].character = (tiles[(x-1)*sizey+(y-1)].character == WALL_1 ? WALL_2 : WALL_1);
            } else if(sideblocked) {
                tiles[(x-1)*sizey+y].character = WALL_1;
            }
        }
    }
}

char** readfile(const char* name, int* rows, int* cols)
{
    char** map;
    char* line = NULL;
    size_t lsize;
    int fsize, trim;
    
    FILE* f = fopen(name, "rb");
    fseek(f, 0L, SEEK_END);
    fsize = ftell(f);
    rewind(f);
    
    getline(&line, &lsize, f);
    *cols = strcspn(line, "\r\n");
    trim = strlen(line) - *cols;
    line[*cols] = 0;
    *rows = fsize / *cols;
    
    map = new char*[*rows];
    map[0] = line;
    
    for(int i=1; i < *rows; i++) {
        map[i] = new char[*cols];
        fgets(map[i], *cols+1, f);
        fseek(f, trim, SEEK_CUR);
    }
    
    fclose(f);
    
    return map;
}

int main(int argc, const char* argv[])
{
    int sizex, sizey;
    char** smap = NULL;
    int timer = DEFAULT_TIME;
    bool reverse = false;
    int viewwidth = DEFAULT_VIEW_WIDTH;
    int viewheight = DEFAULT_VIEW_HEIGHT;
    int carcount = DEFAULT_VEHICLE_COUNT;
    int farecount = DEFAULT_FARE_COUNT;
    
    if(argc > 1) {
        for(int i=1; i < argc; i++) {
            if(strcmp(argv[i], "-m") == 0) {
                smap = readfile(argv[i+1], &sizey, &sizex);
                i++;
            } else if(strcmp(argv[i], "-r") == 0) {
                reverse = true;
            } else if(strcmp(argv[i], "-h") == 0) {
                std::cout << "Console Cabby" << std::endl << "A simple rogue like where you drive around in a taxi, pick up fares, and drop them off at their destination." << std::endl;
                std::cout << "-cc <int>  " << "Adjust the number of cars on the map." << std::endl;
                std::cout << "           Default is " << DEFAULT_VEHICLE_COUNT << std::endl;
                std::cout << "-fc <int>  " << "Adjust the number of fares on the map." << std::endl;
                std::cout << "           Default is " << DEFAULT_FARE_COUNT << std::endl;
                std::cout << "-h         " << "Print this help text" << std::endl;
                std::cout << "-m <path>  " << "Import your own city map file." << std::endl;
                std::cout << "           Map files are just text files with the characters #1245689:" << std::endl;
                std::cout << "           Each character represents a direction a car can travel." << std::endl;
                std::cout << "           '#' means no direction (a wall)" << std::endl;
                std::cout << "           '1' means cars can only travel up" << std::endl;
                std::cout << "           '2' means cars can only travel down" << std::endl;
                std::cout << "           '4' means cars can only travel left" << std::endl;
                std::cout << "           '5' means cars can either travel up or left" << std::endl;
                std::cout << "           '6' means cars can either travel down or left" << std::endl;
                std::cout << "           '8' means cars can only travel right" << std::endl;
                std::cout << "           '9' means cars can either travel up or right" << std::endl;
                std::cout << "           ':' means cars can either travel down or right" << std::endl;
                std::cout << "           All rows should be the same length." << std::endl;
                std::cout << "-r         " << "Make traffic drive on the other side of the road (for U.K. folks)." << std::endl;
                std::cout << "-t <int>   " << "Adjust the amount of time before game over." << std::endl;
                std::cout << "           Default is " << DEFAULT_TIME << std::endl;
                return 0;
            } else if(strcmp(argv[i], "-cc") == 0) {
                carcount = atoi(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-fc") == 0) {
                farecount = atoi(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-t") == 0) {
                timer = atoi(argv[i+1]);
                i++;
            }
        }
    }
    
    // If the map wasn't defined as a parameter then load the default.
    if(smap == NULL) {
        sizex = strlen(builtin[0]);
        sizey = sizeof(builtin) / sizeof(builtin[0]);
        smap = new char*[sizey];
        for(int i=0; i < sizey; i++) {
            smap[i] = new char[sizex];
            strcpy(smap[i], builtin[i]);
        }
    }
    
    /**********************************
     ** Init Tiles
     **********************************/
    
    Tile* tiles = new Tile[sizex*sizey];
    for(int x=0; x < sizex; x++) {
        for(int y=0; y < sizey; y++) {
            char c = smap[y][x];
            tiles[x*sizey+y].character = c;
            tiles[x*sizey+y].direction = (c == WALL_0) ? none : (c - '0');
            tiles[x*sizey+y].posx = x;
            tiles[x*sizey+y].posy = y;
            tiles[x*sizey+y].occupied  = false;
        }
    }
    
    //Clean up smap, we don't need it anymore
    for(int i=0; i < sizey; i++) {
        delete [] smap[i];
    }
    delete [] smap;
    
    //Adjust wall characters to look kinda 3d
    add3dcharacters(tiles, sizex, sizey);
    
    /**********************************
     ** Init Cars
     **********************************/
    
    //Add an extra spot for the player
    carcount++;
    Vehicle* cars = new Vehicle[carcount];
    
    //Add the player car first
    cars[0].character = '@';
    cars[0].direction = none; //This will be the direction of the road tile
    cars[0].posx = 0; //This will be the position of the road tile
    cars[0].posy = 0; //This will be the position of the road tile
    cars[0].player = true;
    cars[0].hasfare = false;
    cars[0].money = 0;
    
    //Add all the other cars
    for(int i=1; i < carcount; i++) {
        cars[i].character = 'C';
        cars[i].direction = none; //This will be the direction of the road tile
        cars[i].posx = 0; //This will be the position of the road tile
        cars[i].posy = 0; //This will be the position of the road tile
        cars[i].player = false;
        cars[i].hasfare = false;
        cars[i].money = 0;
    }
    
    /**********************************
     ** Init Fares
     **********************************/
    
    Fare* fares = new Fare[farecount];
    for(int i=0; i < farecount; i++) {
        fares[i].character = '$';
        fares[i].money = 0;
        fares[i].posx = 0;
        fares[i].posy = 0;
        fares[i].destx = 0;
        fares[i].desty = 0;
        fares[i].wait = 0;
        fares[i].pickedup = false;
    }
    
    char key;
    
    srand(time(NULL));
    
    rlutil::cls();
    rlutil::hidecursor();
    
    // Main game loop
    do {
        if(timer > 0) {
            movecars(tiles, sizex, sizey, cars, carcount, key, reverse);
            spawnfares(tiles, sizex, sizey, fares, farecount, &cars[0]);
            
            rlutil::resetColor();
            rlutil::cls(); //Clear screen on each iteration to get rid of artifacts. This makes the screen flicker a bit
            drawview(viewwidth, viewheight, tiles, sizex, sizey, cars, carcount, fares, farecount);
            drawstatus(viewwidth, viewheight, &cars[0], timer);
            timer--;
        } else {
            drawstatus(viewwidth, viewheight, &cars[0], timer);
            drawgameover(viewwidth, viewheight, &cars[0]);
        }
        
        //drawdebug(viewwidth, viewheight, tiles, sizex, sizey, &cars[0], fares, farecount);
        
        //Make sure this is last in the loop.
        key = getch();
    } while(key != 'q');
    
    rlutil::showcursor();
    rlutil::cls();
    
    delete [] tiles;
    delete [] cars;
    delete [] fares;
    
    return 0;
}
