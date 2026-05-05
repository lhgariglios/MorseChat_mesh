#include "nrf52833.h"
#include <stdint.h>
#include <stdbool.h>


// ── Macros de pinos ───────────────────────────────────────────────────────────

#define ROW1_DISCONNECT()    (NRF_P0->PIN_CNF[21] = 0x00000002)
#define ROW2_DISCONNECT()    (NRF_P0->PIN_CNF[22] = 0x00000002)
#define ROW3_DISCONNECT()    (NRF_P0->PIN_CNF[15] = 0x00000002)
#define ROW4_DISCONNECT()    (NRF_P0->PIN_CNF[24] = 0x00000002)
#define ROW5_DISCONNECT()    (NRF_P0->PIN_CNF[19] = 0x00000002)

#define ROW1_OUTPUT()        (NRF_P0->PIN_CNF[21] = 0x00000003)
#define ROW2_OUTPUT()        (NRF_P0->PIN_CNF[22] = 0x00000003)
#define ROW3_OUTPUT()        (NRF_P0->PIN_CNF[15] = 0x00000003)
#define ROW4_OUTPUT()        (NRF_P0->PIN_CNF[24] = 0x00000003)
#define ROW5_OUTPUT()        (NRF_P0->PIN_CNF[19] = 0x00000003)

#define ROW1_HIGH()          (NRF_P0->OUTSET = (1UL << 21))
#define ROW2_HIGH()          (NRF_P0->OUTSET = (1UL << 22))
#define ROW3_HIGH()          (NRF_P0->OUTSET = (1UL << 15))
#define ROW4_HIGH()          (NRF_P0->OUTSET = (1UL << 24))
#define ROW5_HIGH()          (NRF_P0->OUTSET = (1UL << 19))

#define COL1_DISCONNECT()    (NRF_P0->PIN_CNF[28] = 0x00000002)
#define COL2_DISCONNECT()    (NRF_P0->PIN_CNF[11] = 0x00000002)
#define COL3_DISCONNECT()    (NRF_P0->PIN_CNF[31] = 0x00000002)
#define COL4_DISCONNECT()    (NRF_P1->PIN_CNF[5]  = 0x00000002)
#define COL5_DISCONNECT()    (NRF_P0->PIN_CNF[30] = 0x00000002)

#define COL1_OUTPUT()        (NRF_P0->PIN_CNF[28] = 0x00000003)
#define COL2_OUTPUT()        (NRF_P0->PIN_CNF[11] = 0x00000003)
#define COL3_OUTPUT()        (NRF_P0->PIN_CNF[31] = 0x00000003)
#define COL4_OUTPUT()        (NRF_P1->PIN_CNF[5]  = 0x00000003)
#define COL5_OUTPUT()        (NRF_P0->PIN_CNF[30] = 0x00000003)

#define COL1_LOW()           (NRF_P0->OUTCLR = (1UL << 28))
#define COL2_LOW()           (NRF_P0->OUTCLR = (1UL << 11))
#define COL3_LOW()           (NRF_P0->OUTCLR = (1UL << 31))
#define COL4_LOW()           (NRF_P1->OUTCLR = (1UL <<  5))
#define COL5_LOW()           (NRF_P0->OUTCLR = (1UL << 30))

#define BUTTON_A_PIN        14
#define BUTTON_A_CONFIG()   (NRF_P0->PIN_CNF[BUTTON_A_PIN] = 0x0000000C)

#define BUTTON_B_PIN        23
#define BUTTON_B_CONFIG()   (NRF_P0->PIN_CNF[BUTTON_B_PIN] = 0x0000000C)

// ── Timer Handler ─────────────────────────────────────────────────────────── 

volatile uint32_t ms_ticks = 0;

void timer0_init(void)
{
    NRF_TIMER0->MODE = 0;            // Timer mode
    NRF_TIMER0->BITMODE = 3;         // 32-bit
    NRF_TIMER0->PRESCALER = 4;       // 1 MHz clock (1 us per tick)

    NRF_TIMER0->CC[0] = 1000;        // 1000 us = 1 ms
    NRF_TIMER0->INTENSET = (1 << 16); // Compare0 interrupt

    NVIC_EnableIRQ(TIMER0_IRQn);

    NRF_TIMER0->TASKS_CLEAR = 1;
    NRF_TIMER0->TASKS_START = 1;
}

void TIMER0_IRQHandler(void)
{
    if (NRF_TIMER0->EVENTS_COMPARE[0]) {
        NRF_TIMER0->EVENTS_COMPARE[0] = 0;

        ms_ticks++;  

        NRF_TIMER0->TASKS_CLEAR = 1; // reset timer
    }
}

// ── LEDs ──────────────────────────────────────────────────────────────────────

typedef enum {
    LED11,LED12,LED13,LED14,LED15,
    LED21,LED22,LED23,LED24,LED25,
    LED31,LED32,LED33,LED34,LED35,
    LED41,LED42,LED43,LED44,LED45,
    LED51,LED52,LED53,LED54,LED55
} led_id_t;

const led_id_t dot[] = {
    LED33
};

const led_id_t slash[] = {
    LED32,LED33,LED34
};

const led_id_t sent[] = {
    LED32, LED43, LED34, LED25
};

const led_id_t Zero[] = {
    LED12, LED22, LED32, LED42, LED52, LED13, LED53, LED14, LED24, LED34, LED44, LED54
};
const led_id_t One[] = {
    LED22, LED13, LED23, LED33, LED43,LED52, LED53, LED54
};
const led_id_t Two[] = {
    LED12, LED13, LED14,LED25,LED33, LED34,LED42,LED52, LED53, LED54, LED55
};
const led_id_t Three[] = {
    LED12, LED13, LED14, LED25, LED33, LED34,
    LED45, LED52, LED53, LED54
};
const led_id_t Four[] = {
    LED14, LED23, LED24, LED32, LED33, LED34, LED35, LED44, LED54
};
const led_id_t Five[] = {
    LED11, LED12, LED13, LED14, LED15, LED21, LED31, LED32, LED33, LED34, LED45, LED51, LED52, LED53, LED54
};

const led_id_t * const Digits[] = { Zero, One, Two, Three, Four, Five };
const uint8_t Digits_size[] = {
    sizeof(Zero),
    sizeof(One),
    sizeof(Two),
    sizeof(Three),
    sizeof(Four),
    sizeof(Five)
};

const led_id_t * const figures[] = { dot, slash, sent };
const uint8_t figures_size[] = {
    sizeof(dot),
    sizeof(slash),
    sizeof(sent)
};

// ── LED ────────────────────────────────────────────────────────────
//void leds_all_off(void) {
//    ROW1_DISCONNECT(); ROW2_DISCONNECT(); ROW3_DISCONNECT();
//    ROW4_DISCONNECT(); ROW5_DISCONNECT();
//    COL1_DISCONNECT(); COL2_DISCONNECT(); COL3_DISCONNECT();
//    COL4_DISCONNECT(); COL5_DISCONNECT();
//}

void leds_all_off(void) {
    // Força ROWs para LOW (sem corrente nas linhas)
    NRF_P0->OUTCLR = (1UL << 21) | (1UL << 22) | (1UL << 15) | (1UL << 24) | (1UL << 19);

    // Força COLs para HIGH (sem corrente nas colunas)
    NRF_P0->OUTSET = (1UL << 28) | (1UL << 11) | (1UL << 31) | (1UL << 30);
    NRF_P1->OUTSET = (1UL << 5);

    // Agora desconecta tudo
    ROW1_DISCONNECT(); ROW2_DISCONNECT(); ROW3_DISCONNECT();
    ROW4_DISCONNECT(); ROW5_DISCONNECT();
    COL1_DISCONNECT(); COL2_DISCONNECT(); COL3_DISCONNECT();
    COL4_DISCONNECT(); COL5_DISCONNECT();
}

void leds_on(led_id_t led_id) {
    switch (led_id) {
        case LED11: ROW1_OUTPUT(); ROW1_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED12: ROW1_OUTPUT(); ROW1_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED13: ROW1_OUTPUT(); ROW1_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED14: ROW1_OUTPUT(); ROW1_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED15: ROW1_OUTPUT(); ROW1_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED21: ROW2_OUTPUT(); ROW2_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED22: ROW2_OUTPUT(); ROW2_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED23: ROW2_OUTPUT(); ROW2_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED24: ROW2_OUTPUT(); ROW2_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED25: ROW2_OUTPUT(); ROW2_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED31: ROW3_OUTPUT(); ROW3_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED32: ROW3_OUTPUT(); ROW3_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED33: ROW3_OUTPUT(); ROW3_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED34: ROW3_OUTPUT(); ROW3_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED35: ROW3_OUTPUT(); ROW3_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED41: ROW4_OUTPUT(); ROW4_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED42: ROW4_OUTPUT(); ROW4_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED43: ROW4_OUTPUT(); ROW4_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED44: ROW4_OUTPUT(); ROW4_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED45: ROW4_OUTPUT(); ROW4_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED51: ROW5_OUTPUT(); ROW5_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED52: ROW5_OUTPUT(); ROW5_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED53: ROW5_OUTPUT(); ROW5_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED54: ROW5_OUTPUT(); ROW5_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED55: ROW5_OUTPUT(); ROW5_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;
    }
}

void show(led_id_t* figure, int size, uint32_t frame_time_ms){
    int i = 0;
    uint32_t start = ms_ticks;

    while((ms_ticks - start) < frame_time_ms) {
        leds_all_off();
        leds_on(figure[i]);
        i = (i+1)%size;
    }
    leds_all_off();
}


// ── Bottoms Handler ───────────────────────────────────────────────────────────

#define SIMULTANEOUS_WINDOW_MS 50

volatile bool dot_select = false;
volatile bool slash_select = false;
volatile bool send = false;
volatile bool change_id = false;
volatile bool simultaneous_handled = false;

volatile uint32_t press_start_A = 0;
volatile uint32_t press_start_B = 0;


static void gpiote_init(void)
{
    // Bottom A
    NRF_GPIOTE->CONFIG[0] =
        (1 << 0) |                // MODE = EVENT
        (BUTTON_A_PIN << 8) |     // PSEL = P0.14
        (3 << 16) |               // POLARITY = Toggle (LoToHi e HiToLo)
        (0 << 20);                // OUTINIT 

    // Bottom B
    NRF_GPIOTE->CONFIG[1] =
        (1 << 0) |
        (BUTTON_B_PIN << 8) |
        (3 << 16) |
        (0 << 20);

    NRF_GPIOTE->INTENSET = (1 << 0) | (1 << 1);

    NVIC_EnableIRQ(GPIOTE_IRQn);
}

void GPIOTE_IRQHandler(void)
{
    if (NRF_GPIOTE->EVENTS_IN[0]) {
        NRF_GPIOTE->EVENTS_IN[0] = 0;

        bool a = ((NRF_P0->IN & (1 << BUTTON_A_PIN)) == 0);

        if (a) {
            press_start_A = ms_ticks;
            simultaneous_handled = false;
        } else {
            uint32_t duration = ms_ticks - press_start_A;

            // Verifica se B foi pressionado dentro da janela de simultaneidade
            bool simultaneous = (press_start_B != 0) &&
                                 (press_start_A >= press_start_B 
                                    ? press_start_A - press_start_B 
                                    : press_start_B - press_start_A) 
                                 < SIMULTANEOUS_WINDOW_MS;

            if (simultaneous && !simultaneous_handled) {
                simultaneous_handled = true;
                press_start_A = 0;  
                press_start_B = 0;
                change_id = true;
                duration = 0;
            } else if (duration > 150 && duration < 300 && !simultaneous_handled) {
                dot_select = true;
            } else if (duration > 300 && !simultaneous_handled){
                slash_select = true;
            }else{}
        }
    }

    if (NRF_GPIOTE->EVENTS_IN[1]) {
        NRF_GPIOTE->EVENTS_IN[1] = 0;

        bool b = ((NRF_P0->IN & (1 << BUTTON_B_PIN)) == 0);

        if (b) {
            press_start_B = ms_ticks;
            simultaneous_handled = false;
        } else {
            uint32_t duration = ms_ticks - press_start_B;
            bool simultaneous = (press_start_A != 0) &&
                                 (press_start_B >= press_start_A 
                                    ? press_start_B - press_start_A 
                                    : press_start_A - press_start_B) 
                                 < SIMULTANEOUS_WINDOW_MS;

            if (simultaneous && !simultaneous_handled) {
                simultaneous_handled = true;
                press_start_A = 0;  
                press_start_B = 0;
                change_id = true;
                duration = 0;
            } else if (duration > 200 && !simultaneous_handled){
                send = true;
            }else{}
        }
    }
}

// ── Id configuration ──────────────────────────────────────────────────────────

#define  N_Ids 6
const int My_ID = 1;
int Receiver_ID = 0;

// ── Main ──────────────────────────────────────────────────────────────────────

int main(void) {

    BUTTON_A_CONFIG();
    BUTTON_B_CONFIG();
    gpiote_init();
    timer0_init();

    leds_all_off();
    show(figures[1], figures_size[1], 500);

    while(1){
        if (dot_select){
            dot_select = false;
            show(figures[0], figures_size[0], 500);
        }else if (slash_select){
            slash_select = false;
            show(figures[1], figures_size[1], 500); 
        }
        else if (send){
            send = false;
            show(figures[2], figures_size[2], 500);
        }
        else if (change_id){
            change_id = false;
            Receiver_ID = (Receiver_ID + 1) % N_Ids; 
            show(Digits[Receiver_ID], Digits_size[Receiver_ID], 500);
        }
        else{}  
    }
    
}