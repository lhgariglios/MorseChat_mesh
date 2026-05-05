/**
 * malha.h — Definições compartilhadas entre todos os nós
 *
 * Substitui o protocolo de strings do MakeCode ("T|rssi_r|rssi_l")
 * por um PDU binário compacto, mantendo a mesma semântica.
 *
 * Formato do PDU no ar (6 bytes):
 *
 *  Byte  Campo       Descrição
 *  [0]   header      Sempre 0x00 (compatível com projeto base)
 *  [1]   length      Tamanho do payload = 4
 *  [2]   tipo        'T'=transmissor, 'R'=right node, 'L'=left node
 *  [3]   rssi_right  RSSI medido pelo right node (int8, 0 se não preenchido)
 *  [4]   rssi_left   RSSI medido pelo left node  (int8, 0 se não preenchido)
 *  [5]   reservado   0x00
 *
 * Equivalência com o MakeCode:
 *   "T|0|0"       → tipo='T', rssi_right=0,    rssi_left=0
 *   "T|0|<rssi>"  → tipo='R', rssi_right=0,    rssi_left=rssi
 *   "T|<rssi>|0"  → tipo='L', rssi_right=rssi, rssi_left=0
 *
 * Configuração de rádio (idêntica em todos os nós):
 *   Modo    : BLE LR 125 Kbit/s
 *   Canal   : 10  (grupo 1 MakeCode → 2400+10=2410 MHz)
 *   Potência: +4 dBm (≈ setTransmitPower(7) do MakeCode)
 *   Endereço: BASE0=0xAAAAAAAA, PREFIX=0x00
 *   CRC     : 3 bytes
 */

#ifndef MALHA_H
#define MALHA_H

#include <stdint.h>
#include <nrf.h>

/* ── Rádio ─────────────────────────────────────────────── */
#define RADIO_FREQ         10
#define RADIO_TXPOWER_VAL  RADIO_TXPOWER_TXPOWER_Pos4dBm

/* ── Tipos de nó ───────────────────────────────────────── */
#define TIPO_TX     'T'
#define TIPO_RIGHT  'R'
#define TIPO_LEFT   'L'

/* ── PDU ───────────────────────────────────────────────── */
#define PDU_PAYLOAD_LEN  4
#define PDU_TOTAL_LEN    6

#define PDU_IDX_HEADER      0
#define PDU_IDX_LENGTH      1
#define PDU_IDX_TIPO        2
#define PDU_IDX_RSSI_RIGHT  3
#define PDU_IDX_RSSI_LEFT   4
#define PDU_IDX_RESERVADO   5

/* ── Pinos de LED (micro:bit v2) ───────────────────────── *
 * ROW ativo alto, COL ativo baixo.
 * LED (col, row)  ROW-pin  COL-pin
 *  (0,0) sup.esq  ROW1=P0.21  COL1=P0.28   ← left node
 *  (2,2) central  ROW3=P0.15  COL3=P0.11   ← TX
 *  (4,0) sup.dir  ROW1=P0.21  COL4=P0.31   ← right node
 * ─────────────────────────────────────────────────────── */
#define LED_ROW1  21U
#define LED_ROW3  15U
#define LED_COL1  28U
#define LED_COL3  11U
#define LED_COL4  31U

/* ── Utilitários inline ────────────────────────────────── */

static inline void delay(volatile uint32_t n) { while (n--); }

static inline void clock_init(void) {
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {}
}

static inline void radio_init(uint8_t *buf, uint32_t maxlen) {
    NRF_RADIO->MODE    = (RADIO_MODE_MODE_Ble_LR125Kbit << RADIO_MODE_MODE_Pos);
    NRF_RADIO->TXPOWER = (RADIO_TXPOWER_VAL             << RADIO_TXPOWER_TXPOWER_Pos);
    NRF_RADIO->PCNF0   =
        (8                           << RADIO_PCNF0_LFLEN_Pos)   |
        (1                           << RADIO_PCNF0_S0LEN_Pos)   |
        (0                           << RADIO_PCNF0_S1LEN_Pos)   |
        (2                           << RADIO_PCNF0_CILEN_Pos)   |
        (RADIO_PCNF0_PLEN_LongRange  << RADIO_PCNF0_PLEN_Pos)    |
        (3                           << RADIO_PCNF0_TERMLEN_Pos);
    NRF_RADIO->PCNF1   =
        (maxlen                       << RADIO_PCNF1_MAXLEN_Pos)  |
        (0                            << RADIO_PCNF1_STATLEN_Pos) |
        (3                            << RADIO_PCNF1_BALEN_Pos)   |
        (RADIO_PCNF1_ENDIAN_Little    << RADIO_PCNF1_ENDIAN_Pos)  |
        (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos);
    NRF_RADIO->BASE0       = 0xAAAAAAAAUL;
    NRF_RADIO->PREFIX0     = 0x00UL;
    NRF_RADIO->TXADDRESS   = 0UL;
    NRF_RADIO->RXADDRESSES = (RADIO_RXADDRESSES_ADDR0_Enabled << RADIO_RXADDRESSES_ADDR0_Pos);
    NRF_RADIO->CRCCNF  = (RADIO_CRCCNF_LEN_Three    << RADIO_CRCCNF_LEN_Pos)      |
                         (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);
    NRF_RADIO->CRCINIT = 0xFFFFUL;
    NRF_RADIO->CRCPOLY = 0x00065bUL;
    NRF_RADIO->FREQUENCY = RADIO_FREQ;
    NRF_RADIO->PACKETPTR = (uint32_t)buf;
    NRF_RADIO->INTENCLR  = 0xFFFFFFFFUL;
    NRF_RADIO->SHORTS    =
        (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
}

static inline void radio_send(void) {
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0) {}
    NRF_RADIO->EVENTS_DISABLED = 0;
}

/* Retorna 1 se pacote recebido com CRC ok, 0 se timeout ou erro.
 * timeout=0 → espera infinita.                                   */
static inline int radio_recv(uint32_t timeout) {
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_RXEN = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0) {
        if (timeout && --timeout == 0) {
            NRF_RADIO->TASKS_DISABLE = 1;
            while (NRF_RADIO->EVENTS_DISABLED == 0) {}
            NRF_RADIO->EVENTS_DISABLED = 0;
            return 0;
        }
    }
    NRF_RADIO->EVENTS_DISABLED = 0;
    return (NRF_RADIO->CRCSTATUS == RADIO_CRCSTATUS_CRCSTATUS_CRCOk) ? 1 : 0;
}

static inline int8_t radio_rssi(void) {
    /* Dispara medição e aguarda */
    NRF_RADIO->TASKS_RSSISTART = 1;
    while (NRF_RADIO->EVENTS_RSSIEND == 0) {}
    NRF_RADIO->EVENTS_RSSIEND = 0;
    /* RSSISAMPLE é positivo; RSSI real = -RSSISAMPLE dBm */
    return -(int8_t)(NRF_RADIO->RSSISAMPLE & 0x7F);
}

static inline void led_on(uint32_t row_pin, uint32_t col_pin) {
    NRF_P0->DIRSET = (1UL << row_pin) | (1UL << col_pin);
    NRF_P0->OUTSET = (1UL << row_pin);
    NRF_P0->OUTCLR = (1UL << col_pin);
}

static inline void led_off(uint32_t row_pin, uint32_t col_pin) {
    NRF_P0->OUTCLR = (1UL << row_pin);
    NRF_P0->OUTSET = (1UL << col_pin);
}

#endif /* MALHA_H */