/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "imagedata.h"
#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"

/*Pins*/
#define MOSI            14
#define RST             26
#define BUSY            25
#define CS              15
#define CLK             13
#define DC              27
#define PIN_BUTTON      16  // Main button (select)
#define PIN_BUTTON_SCROLL 17 // Scroll button

/*Constants*/
#define NTPSERVER1 "pool.ntp.org"
#define NTPSERVER2 "time.nist.gov"
#define TIMEZONE "PST8PDT,M3.2.0,M11.1.0"
#define GMT_OFFSET_SEC (-8 * 3600) 
#define DAYLIGHT_OFFSET_SEC 3600 
#define WIFI_SSID 
#define WIFI_PASSWORD 
#define SCREEN_WIDTH    200
#define SCREEN_HEIGHT   200
#define SCORE_HEIGHT    24
#define TIME_HEIGHT     20
#define UPDATE_INTERVAL 1000  // Time update interval (1 second)
#define FRAME_DELAY     200   // Animation frame rate
#define BATTERY_DRAIN   50000    // Seconds between battery decrements
#define MENU_PADDING    10    // Menu padding
#define MENU_WIDTH      160   // Menu width
#define MENU_HEIGHT     160   // Menu height
#define ANIMATION_CYCLES   10    // Number of animation cycles to play at startup
#define STATE_ANIMATING    0    // Pet is animating
#define STATE_FROZEN       1    // Pet is frozen on a frame
#define STATE_MENU         2    // In menu system


// Menu states
#define MENU_STATE_NONE     0
#define MENU_STATE_MAIN     1
#define MENU_STATE_SET_TIME 2
// Menu items
#define MENU_ITEM_RESET_SCORE  0
#define MENU_ITEM_RESET_BATTERY 1
#define MENU_ITEM_SET_TIME     2
#define MENU_ITEM_EXIT         3
#define MENU_ITEM_COUNT        4
// Button handling
#define BUTTON_DEBOUNCE_MS   25
#define DOUBLE_CLICK_TIME_MS 2000

/* types ---------------------------------------------------------------------*/
typedef struct {
    const UBYTE** frames;
    uint8_t num_frames;
} Animation;

typedef struct {
    PAINT_TIME current_time;
    uint16_t score;
    uint16_t battery;
    bool button_pressed;
    unsigned long last_time_update;
    unsigned long last_frame_time;
    uint8_t current_frame;
    uint8_t menu_state;
    uint8_t menu_selection;
    unsigned long last_button_press;
    unsigned long last_scroll_press;
    uint8_t click_count;
    uint8_t pet_state; 
    uint8_t animation_cycles; 
} GameState;

UBYTE* display_buffer = NULL;
GameState state;

const UBYTE* normal_frames[] = {
    screen_10happy_1,
    screen_11happy_2,
    screen_12happy_2_1,
    screen_13happy_3,
    screen_14happy_4,
    screen_10happy_1
};
const UBYTE* button_frames[] = {
    screen_1hungry_1_1,
    screen_1hungry_1_1,
    screen_1hungry_1_1
};
const Animation normal_anim = {normal_frames, 6};
const Animation button_anim = {button_frames, 3};
const char* menu_items[] = {
    "Reset Score",
    "Reset Battery",
    "Exit Menu"
};
/* Function prototypes*/
void updateTime(PAINT_TIME* current_time);//-
void draw_interface(GameState* state);//-
void draw_menu(GameState* state);//-
bool init_hardware(void);
void cleanup(void);//-
void handle_buttons(void);//-
void handle_menu_actions(void);//-
void printLocalTime();//-
void timeavailable(struct timeval *t);//-

/* functionnnnsssss*/
/* Time stuff */
void printLocalTime() 
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
void timeavailable(struct timeval *t) 
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}
void update_time(PAINT_TIME* current_time) 
{
  struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        
    current_time->Hour = timeinfo.tm_hour;
    current_time->Min = timeinfo.tm_min; 
    current_time->Sec = timeinfo.tm_sec;
    printLocalTime();
  
    }
    
}
void draw_interface(GameState* state)
{   
    Paint_DrawRectangle(0, 0, SCREEN_WIDTH, SCORE_HEIGHT, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "Score: %d  %d%%", state->score, state->battery);
    Paint_DrawString_EN(0, 3, score_str, &Font20, BLACK, WHITE);
    
    Paint_DrawRectangle(0, SCREEN_HEIGHT - TIME_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT,
    BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    PAINT_TIME time_cp = state->current_time;
    
    Paint_DrawTime12Hour(25, SCREEN_HEIGHT - TIME_HEIGHT, &time_cp, &Font20, BLACK, WHITE);
}

void draw_menu(GameState* state)
{
    int x = (SCREEN_WIDTH - MENU_WIDTH) / 2;
    int y = (SCREEN_HEIGHT - MENU_HEIGHT) / 2;
    Paint_DrawRectangle(x, y, x + MENU_WIDTH, y + MENU_HEIGHT, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(x + 1, y + 1, x + MENU_WIDTH - 1, y + MENU_HEIGHT - 1, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(x + MENU_PADDING, y + MENU_PADDING, "MENU", &Font20, BLACK, WHITE);
    Paint_DrawLine(x + MENU_PADDING, y + MENU_PADDING + 25, x + MENU_WIDTH - MENU_PADDING, 
                  y + MENU_PADDING + 25, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        if (i == state->menu_selection) {
            Paint_DrawRectangle(x + MENU_PADDING, y + MENU_PADDING + 35 + i * 25, 
                              x + MENU_WIDTH - MENU_PADDING, y + MENU_PADDING + 35 + (i + 1) * 25 - 5, 
                              BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
            Paint_DrawString_EN(x + MENU_PADDING + 5, y + MENU_PADDING + 35 + i * 25 + 2, 
                              menu_items[i], &Font16, WHITE, BLACK);
        } 
        else
        { 
        Paint_DrawString_EN(x + MENU_PADDING + 5, y + MENU_PADDING + 35 + i * 25 + 2, 
                              menu_items[i], &Font16, BLACK, WHITE); 
        }
        }
}
void Paint_DrawTime12Hour(UWORD x, UWORD y, PAINT_TIME* pTime, sFONT* font, UWORD background_color, UWORD font_color)
{
    char time_string[16];
    uint8_t hour12;
    const char* am_pm;
    
    if (pTime->Hour == 0) {
        hour12 = 12;
        am_pm = "AM";
    } else if (pTime->Hour < 12) {
        hour12 = pTime->Hour;
        am_pm = "AM";
    } else if (pTime->Hour == 12) {
        hour12 = 12;
        am_pm = "PM";
    } else {
        hour12 = pTime->Hour - 12;
        am_pm = "PM";
    }
    
    sprintf(time_string, "%2d:%02d:%02d %s", hour12, pTime->Min, pTime->Sec, am_pm);
    
    Paint_DrawString_EN(x, y, time_string, font, background_color, font_color);
}
void handle_buttons()
{
static bool last_main_btn_state = HIGH;
static bool last_scroll_btn_state = HIGH;
unsigned long current_time = millis();
bool main_btn_state = digitalRead(PIN_BUTTON);
     if (main_btn_state == LOW && last_main_btn_state == HIGH && 
     current_time - state.last_button_press > BUTTON_DEBOUNCE_MS) 
     {  //unfreeze pet 
        if(state.pet_state == STATE_FROZEN && state.menu_state == MENU_STATE_NONE)
        {
            Serial.println("Resuming animation");
            state.pet_state = STATE_ANIMATING;
            state.animation_cycles = 0;
            state.current_frame = 0;
            state.button_pressed = true;
            state.score++;
        } //double click detection
        else if (current_time - state.last_button_press < DOUBLE_CLICK_TIME_MS && state.click_count > 1)
        {
            Serial.println("Double Click Detected");
            state.click_count = 0;
            if (state.menu_state == MENU_STATE_NONE)
            {
                state.menu_state = MENU_STATE_MAIN;
                state.menu_selection = 0;
                state.pet_state = STATE_MENU;
                Serial.println("Entering Menu");
            }
        }
        else 
        { //single click
            state.click_count++;
            if(state.menu_state == MENU_STATE_MAIN) 
            {
                handle_menu_actions();
            }
            else if (state.pet_state == STATE_ANIMATING) 
            {
                state.score++;
                state.button_pressed = true;
                Serial.printf("Button pressed! Score: %d\n", state.score);
            }
        }
        state.last_button_press = current_time;
    }
    last_main_btn_state = main_btn_state;

    if (state.click_count > 0 && current_time - state.last_button_press > DOUBLE_CLICK_TIME_MS)
    {
        state.click_count = 0; // Reset click count if no double click detected
    }
    bool scroll_btn_state = digitalRead(PIN_BUTTON_SCROLL);
        if (scroll_btn_state == LOW && last_scroll_btn_state == HIGH &&
            current_time - state.last_scroll_press > BUTTON_DEBOUNCE_MS)
            {
               Serial.println("Scroll button pressed!");
                    if (state.menu_state == MENU_STATE_MAIN)
                     {
                     state.menu_selection = (state.menu_selection + 1) % MENU_ITEM_COUNT;
                     }  
            }
             state.last_scroll_press = current_time;
            last_scroll_btn_state = scroll_btn_state;
 } 
void cleanup(void)
{
    if (display_buffer) 
    {
        free(display_buffer);
        display_buffer = NULL;
    }
    DEV_Module_Init();
}
void handle_menu_actions(void)
{
     switch (state.menu_selection)
      {
        case MENU_ITEM_RESET_SCORE:
            state.score = 0;
            Serial.println("Score reset to 0");
            break;
        case  MENU_ITEM_RESET_BATTERY:
            state.battery = 100;
            Serial.println("Battery reset to 100%");
            break;
        case MENU_ITEM_EXIT:
            state.menu_state = MENU_STATE_NONE;
            state.pet_state = STATE_ANIMATING;
            break;
      }
}

bool init_hardware(void)
{
    if (DEV_Module_Init() != 0)
    {
        Serial.println("Hardware initialization failed!");
        return false;
    }
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BUTTON_SCROLL, INPUT_PULLUP);
    EPD_1IN54_V2_Init();
    EPD_1IN54_V2_Clear();
    UWORD buffer_size = ((EPD_1IN54_V2_WIDTH % 8 == 0) ? 
                        (EPD_1IN54_V2_WIDTH / 8) : (EPD_1IN54_V2_WIDTH / 8 + 1)) * 
                        EPD_1IN54_V2_HEIGHT;
    display_buffer = (UBYTE*)malloc(buffer_size);
    if (!display_buffer)
    {
        Serial.println("Failed to allocate display buffer");
        return false;
    }
    Paint_NewImage(display_buffer, EPD_1IN54_V2_WIDTH, EPD_1IN54_V2_HEIGHT, 0, WHITE);
    EPD_1IN54_V2_Init_Partial();
    return true;
}
void setup()
{
    Serial.begin(115200);
    Serial.println("ESP32 Virtual Pet Starting...");
    Serial.printf("Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    esp_sntp_servermode_dhcp(1);
    
    int wifi_attempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) // 10 second timeout
    {
        delay(500);
        Serial.print(".");
        wifi_attempts++;
    }

    // Remove duplicate WiFi connection loop
    
    Serial.println("Connected to WiFi!");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTPSERVER1, NTPSERVER2);
    sntp_set_time_sync_notification_cb(timeavailable);
    
    // Update global state directly instead of creating a local variable
    state.current_time.Hour = 12;
    state.current_time.Min = 34;
    state.current_time.Sec = 56;
    state.score = 0;
    state.battery = 100;
    state.button_pressed = false;
    state.last_time_update = 0;
    state.last_frame_time = 0;
    state.current_frame = 0;
    state.menu_state = MENU_STATE_NONE;
    state.menu_selection = 0;
    state.last_button_press = 0;
    state.last_scroll_press = 0;
    state.click_count = 0;
    state.pet_state = STATE_ANIMATING;
    state.animation_cycles = 0;
    
    if (!init_hardware())
    {
        Serial.println("Failed to initialize hardware!");
        while (1) { delay(1000); }
    }
    Serial.println("Initialization complete");
}
void loop()
{
unsigned long current_time = millis();
handle_buttons();
    if (current_time - state.last_time_update >= UPDATE_INTERVAL)
    {
        update_time(&state.current_time);
    }
    if (state.battery > 0 && (state.current_time.Sec % BATTERY_DRAIN == 0))
    {
        state.battery--;
    }
    state.last_time_update = current_time;
    if (current_time - state.last_frame_time >= FRAME_DELAY)
    {
        Paint_SelectImage(display_buffer);
        Paint_Clear(WHITE);
        if (state.menu_state == MENU_STATE_NONE)
        {
            const Animation* current_anim = state.button_pressed ? &button_anim : &normal_anim;
            Paint_DrawBitMap(current_anim->frames[state.current_frame]);
            draw_interface(&state);
            if (state.pet_state == STATE_ANIMATING)
            {
                state.current_frame = (state.current_frame + 1) % current_anim->num_frames;
                if (state.current_frame == 0)
                {
                     state.animation_cycles++;
                     if (state.button_pressed)
                     {
                        state.button_pressed = false;
                     }
                      if (state.animation_cycles >= ANIMATION_CYCLES && !state.button_pressed)
                      {
                        state.pet_state = STATE_FROZEN;
                        state.current_frame = current_anim->num_frames - 1;
                        Serial.println("Animation complete, freezing on last frame");
                      }
                }
            }
        }
        else if (state.menu_state == MENU_STATE_MAIN) 
        {
          draw_interface(&state);
          draw_menu(&state);   
        }
    EPD_1IN54_V2_DisplayPart(display_buffer);
    state.last_frame_time = current_time;
    }

}









