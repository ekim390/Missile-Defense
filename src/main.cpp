//=================================================================
// The main program file.
//
// Copyright 2020 Georgia Tech.  All rights reserved.
// The materials provided by the instructor in this course are for
// the use of the students currently enrolled in the course.
// Copyrighted course materials may not be further disseminated.
// This file must not be made publicly available anywhere.
//==================================================================

// External libs
#include <stdlib.h>

// Project includes
#include "globals.h"
#include "hardware.h"
#include "city_landscape_public.h"
#include "missile_public.h"
#include "player_public.h"

#define CITY_HIT_MARGIN 1
#define CITY_UPPER_BOUND (SIZE_Y-(LANDSCAPE_HEIGHT+MAX_BUILDING_HEIGHT))

int num_city_g = 4;
int score = 0;
int level = 1;
int speed = 6;
int rate = 15;
int radius = 10;

// function prototypes
void set_random_seed(Timer);
int city_landscape_update(void);
int was_player_hit(void);
void missile_contact(void);
void update_score(void); 
void next_level(void);
void display_level(void);

int main()
{
    GameInputs inputs; 
    // First things first: initialize hardware
    ASSERT_P(hardware_init() == ERROR_NONE, "Hardware init failed!");
    pc.printf("Program Starting");

    // Game state variables
    int num_remain_city; // number of cities currently on the landscape
    int player_alive; // 1 if alive, 0 if hit
    
    testDLL();
    // Timer to measure game update speed (secondarily used to generate random seed)
    Timer t;
    int dt; // delta time
    set_random_seed(t); // Already implemented.
    
    //set level difficulty based on which level was chosen from the game menu
    int templevel=0;            // holds level chosen from the game menu
    templevel = level;
    while(templevel > 1) {
        speed = speed - 2;      // missile speed increase
        rate = rate - 5;        // missile rate increase
        radius = radius - 3;    
        set_missile_speed(speed);
        set_missile_interval(rate);
        display_level();
        templevel = templevel - 1;
    }
    
    //Initialization functions (already implemented)
    city_landscape_init(num_city_g);
    missile_init();
    player_init();        
    pc.printf("Initialization complete\n");
    
    //initialize score and level on screen
    update_score();     //displays intial score on screen (0)
    display_level();    //displays current level

    while(1)
    {
        t.start();
        // You must complete the implementation of this function in hardware.cpp:
        inputs = read_inputs();

        // You must write the code to dispatch to the correct action (e.g., 
        // player_moveLeft/Right, player_fire, etc.) based on the inputs read.
        // You must also implement player_moveLeft/moveRight/fire (see player
        // module).
        if (inputs.ax < 0) {
            player_moveLeft();          // if tilting left, move player left
        }
        if (inputs.ax > 0) {
            player_moveRight();         // if tilting right, move player right
        }
        if (inputs.b2 == 1) {
            player_fire();              // if pushbutton 2 is pressed, fire anti-missiles
        }     
        if (inputs.b1 && inputs.b3) {
            next_level();               // if pushbutton 1 and 3 are pressed at same time, jump to next level
        }   
        // Detect missile collisions with player anti-missiles.
        missile_contact();
        
        // Detect missile collision with player aircraft.
        //returns 0 if player is hit; else 1
        player_alive = was_player_hit(); 
        
        // Generate new missiles and draw all active missiles at current 
        // positions. (Already implemented.)
        missile_generator();
        
        // Draw all active anti-missiles (aka player missiles) at current 
        // positions. (Already implemented.)
        player_missile_draw();
        
        // Detect missile collisions with city/land and update status.
        num_remain_city = city_landscape_update();
        
        // Detects if either the player has died or all cities are destroyed
        // Game stops and GAME OVER is displayed        
        if ((player_alive == 0) || (num_remain_city == 0)) { 
            uLCD.locate(0,0);               // placed on top left of screen
            uLCD.color(0xFFFFF);            //white color
            uLCD.printf("GAME OVER \n");
            break;
        }
        
        // Compute update time to control timing of the game loop.
        // (Already implemented... you're welcome.)
        t.stop();
        dt = t.read_ms();
        if (dt < 100) wait_ms(100 - dt);
    }
    pc.printf("out of main loop\n");
    
    // You must write code to free up any dynamically allocated objects such
    // as lists of missiles (hint: destroyList can be used for doubly linked
    // lists).
    DLinkedList* missileDLL = (DLinkedList*)malloc(sizeof(DLinkedList));
    missileDLL = get_missile_list();
    destroyList(missileDLL);
    PLAYER player;
    player = player_get_info();
    destroyList(player.playerMissiles);
    return 0;
}

/** Detect whether any missile has hit a city and if so, call 
    city_demolish (hint: city_get_info may be useful).
    Also, if any missile has hit a city or the landscape,
    mark the missile's status as MISSILE_EXPLODED 
    which will cue the missile's deletion and erasure from the screen 
    on the next missile_generator call.
    @return  Number of remaining cities.
*/
int city_landscape_update(void){
    // Complete this function.
    int Mx=0;
    int My=0;
    int maxx=0;
    int maxy=0;
    DLinkedList* missileDLL = (DLinkedList*)malloc(sizeof(DLinkedList));
    missileDLL = get_missile_list();
    MISSILE* newMissile;
    LLNode* anticurrent = missileDLL->head; 
    while(anticurrent) {
        int i;
        newMissile = (MISSILE*)anticurrent->data; 
        Mx = newMissile->x;
        My = newMissile->y;
        for (i=0; i <= 4; i++) {
            CITY current;
            current = city_get_info(i);
            maxx = current.x + current.width;
            maxy = current.y - current.height;
            if ((current.status == EXIST) && (Mx < maxx) && (Mx > (current.x)) && (My > maxy) && (My < (current.y))) {
                newMissile->status = MISSILE_EXPLODED;
                uLCD.filled_circle((current.x + (current.width/2)), maxy, 10, 0x5F5F5F); //dark grey color explosion made
                wait(.25);
                uLCD.filled_circle((current.x + (current.width/2)), maxy , 10, 0x00000);
                num_city_g = num_city_g - 1;
                city_demolish(i);
                return num_city_g;
            }
        }
        anticurrent = anticurrent->next;
    }              
    return num_city_g;
}

/** Detect whether any missile has hit the player aircraft and if so, call 
    player_destroy (hint: player_get_info may be useful) and mark the 
    missile's status as MISSILE_EXPLODED which will cue the missile's deletion
    and erasure from the screen on the next missile_generator call.
    @return 1 if the player aircraft was hit and 0 otherwise.
*/
int was_player_hit(){
    PLAYER player;
    player = player_get_info();
    int px = player.x;
    int pxw = px + player.width;
    int py = player.y;
    int pyh = py + player.height;
    int d = player.delta;
    DLinkedList* missileDLL = (DLinkedList*)malloc(sizeof(DLinkedList));
    missileDLL = get_missile_list();
    MISSILE* newMissile;
    LLNode* anticurrent = missileDLL->head;
    while(anticurrent) {
        newMissile = (MISSILE*)anticurrent->data;
        int x = newMissile->x;
        int y = newMissile->y;
        if (((x >= px) && (x <= pxw) && (y >= py) && (y <= pyh)) || ((x >= (px + d)) && (x <= (pxw - d)) && (y >= (py - d)) && (y <= pyh))) {
            player_destroy();
            newMissile->status = MISSILE_EXPLODED;
            return 0;
        }
        anticurrent = anticurrent->next;
    } 
    return 1;
}

/** Displays the current level on the screen.
*/
void display_level(void) {
    int l = level - 1;
    uLCD.locate(0,0);           //gets rid of old level on top left corner
    uLCD.color(0x00000); 
    uLCD.printf("%d", l);
    uLCD.locate(0,0);           //places new score in top left corner to replace old score
    uLCD.color(0xFFFFF);        //white color
    uLCD.printf("%d", level);
}

/** Moves to the next level of the game.
*/
void next_level(void) {
    //moves to the next level by first erasing all missiles on screen
    DLinkedList* missileDLL = (DLinkedList*)malloc(sizeof(DLinkedList));    
    missileDLL = get_missile_list();
    MISSILE* newMissile;
    LLNode* current = missileDLL->head;
    //deletes each missile on screen after 10 points were reached in that level
    while(current) {
        newMissile = (MISSILE*) current->data;
        missile_draw(newMissile, BACKGROUND_COLOR);
        deleteNode(missileDLL, current);
        current = current->next;
    }
    //adjusts the speed and rate of the missiles to make next level harder
    //changes the radius in which the anti-missile can detect a missile
    speed = speed - 2;              //increases speed of missile
    rate = rate - 5;                //increases rate of missile
    radius = radius - 3;            //decreases radius
    set_missile_speed(speed);
    set_missile_interval(rate);
    level++;
    display_level();                //displays next level on screen
}

/** Updates the score by erasing the past score on the uLCD and then prints out
    the new score on the uLCD every time it is called.
*/
void update_score(void) {
    int s = score - 1;
    uLCD.locate(16,0);          //takes away old score in top right corner
    uLCD.color(0x00000); 
    uLCD.printf("%d", s);
    uLCD.locate(16,0);          //replaces old score with new updated score in top right corner
    uLCD.color(0xFFFFF); 
    uLCD.printf("%d", score);
}

/** Detect whether any missile has hit any player missile and if so, 
    mark the status of both the missile and the player missile as 
    MISSILE_EXPLODED which will cue the missile's deletion and erasure 
    from the screen on the next missile_generator call.
*/
void missile_contact(void) {
    double x = 0;
    double y = 0;
    double distance = 0;
    PLAYER player;
    player = player_get_info();
    DLinkedList* missileDLL = (DLinkedList*)malloc(sizeof(DLinkedList));
    missileDLL = get_missile_list();
    PLAYER_MISSILE* playerMissile;
    LLNode* current = player.playerMissiles->head;
    while(current){
        playerMissile = (PLAYER_MISSILE*)current->data; 
        MISSILE* newMissile;
        LLNode* anticurrent = missileDLL->head;                  
        while(anticurrent) {
            newMissile = (MISSILE*)anticurrent->data;
            x = (playerMissile->x) - (newMissile->x);
            x = x*x;
            y = (playerMissile->y) - (newMissile->y);
            y = y*y;
            distance = sqrt(x + y);
            if(distance <= radius) {
                playerMissile->status = PMISSILE_EXPLODED;  
                newMissile->status =  MISSILE_EXPLODED;
                uLCD.filled_circle(newMissile->x, newMissile->y , radius, 0xFFFFF);
                wait(.15);
                uLCD.filled_circle(newMissile->x, newMissile->y , radius, 0x00000);
                score++;
                update_score();
                if (((score == 10) || (score == 20) || (score == 30)) && (level < 3)) {
                    next_level();
                }
            }
            anticurrent = anticurrent->next;
        } 
        current = current->next;      
    }  
}

/* We need a random number generator (e.g., in missile_create to generate
   random positions for the missiles to start, making the game different
   every time).  C provides a pseudo random number generator (in stdlib) but
   it requires that we give it a different seed for each new game.  A common
   way to do this is to sample a clock and use the time as the seed.  However
   if we do this by starting a simple Timer t when the mbed starts running the 
   program and then sample it (t.read_ms), we will always get exactly the same
   time sample -- t.read_ms will always occur at the same time in the program's
   execution.  We introduce variability in when we sample the time by waiting
   for the user to push any button before we call t.read_ms.
*/
void set_random_seed(Timer t) {
    GameInputs inputs; 
    t.start();
    uLCD.printf("Push any button to start.\n");
    while(1){
        inputs = read_inputs();
        if (inputs.b1 || inputs.b2 || inputs.b3) break;
      }
    uLCD.cls();
    // Beginning of the title screen code
    uLCD.locate(6,4);                       //names the game MISSILE COMMAND in red
    uLCD.color(RED);
    uLCD.printf("MISSILE\n");
    uLCD.locate(6,6);
    uLCD.printf("COMMAND\n");
    uLCD.locate(3,8);
    uLCD.color(BLUE);
    uLCD.printf("Choose level\n");          //displays "Choose level" in blue
    uLCD.locate(1,10);
    uLCD.color(WHITE);
    uLCD.printf("1: Push button 1\n");      //option of level 1 by pressing button 1
    uLCD.locate(1,12);                      
    uLCD.color(WHITE);
    uLCD.printf("2: Push button 2\n");      //option of level 2 by pressing button 2
    uLCD.locate(1,14);
    uLCD.color(WHITE);
    uLCD.printf("3: Push button 3\n");      //option of level 3 by pressing button 3
    //changes level based on which button is pressed
    while(1) {                              
        inputs = read_inputs();
        if (inputs.b1) break;
        if (inputs.b2) {
            level = 2;
            break;
        }
        if (inputs.b3) {
            level = 3;
            break;
        }
    }
    uLCD.cls();
    t.stop();
    int seed = t.read_ms();    
    srand(seed);
}

