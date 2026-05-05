/**
 * pessoa.cpp — Firmware do nó "pessoa"
 *
 * Responsabilidade:
 *   - Transmite periodicamente um pacote de presença via rádio.
 *   - Identifica-se com um ID fixo (PESSOA_ID).
 *   - Incrementa seq_num a cada envio para que repetidores e o
 *     robô possam detectar e descartar duplicatas.
 *
 * Hardware: micro:bit v2 (CODAL / C++)
 * Build:    https://github.com/lancaster-university/microbit-v2-samples
 */

#include "nrf52833.h"

/* ── Configurações ─────────────────────────────────────── */
#define RADIO_GROUP     42      // Mesmo grupo em todos os nós
#define RADIO_POWER      6      // 0-7 (6 ≈ +4 dBm)
#define PESSOA_ID        1      // ID único desta unidade
#define TTL_INICIAL      4      // Máximo de saltos permitidos
#define INTERVALO_MS   500      // Período de broadcast (ms)

/* ── Estrutura do pacote (shared entre todos os firmwares) ─ */
// IMPORTANTE: manter idêntica em pessoa.cpp, repetidor.cpp e robo.cpp
struct Pacote {
    uint8_t  origem_id;   // ID fixo da pessoa (nunca muda)
    uint16_t seq_num;     // Contador de sequência
    uint8_t  ttl;         // Time-to-live: repetidor decrementa antes de reenviar
    int8_t   rssi_salto;  // RSSI do último salto (preenchido pelo receptor)
};

/* ── Instância global do micro:bit ─────────────────────── */
MicroBit uBit;

/* ─────────────────────────────────────────────────────── */
int main()
{
    uBit.init();

    /* Configura rádio */
    uBit.radio.enable();
    uBit.radio.setGroup(RADIO_GROUP);
    uBit.radio.setTransmitPower(RADIO_POWER);

    /* Feedback visual: mostra "P" no display ao iniciar */
    uBit.display.scroll("P", 400);

    uint16_t seq = 0;

    while (true) {
        Pacote pkt;
        pkt.origem_id  = PESSOA_ID;
        pkt.seq_num    = seq++;
        pkt.ttl        = TTL_INICIAL;
        pkt.rssi_salto = 0;   // Preenchido pelos receptores, não pela origem

        /* Envia o pacote como blob binário */
        uBit.radio.datagram.send((uint8_t*)&pkt, sizeof(pkt));

        /* Pisca LED central como heartbeat visual */
        uBit.display.image.setPixelValue(2, 2, 255);
        uBit.sleep(80);
        uBit.display.image.setPixelValue(2, 2, 0);

        uBit.sleep(INTERVALO_MS - 80);
    }

    release_fiber();
    return 0;
}