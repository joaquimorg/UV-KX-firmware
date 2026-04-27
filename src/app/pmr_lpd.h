#ifndef APP_PMR_LPD_H
#define APP_PMR_LPD_H

#include <stdbool.h>
#include <stdint.h>

#include "misc.h"
#include "radio.h"

#define PMR_LPD_EEPROM_CHANNEL FREQ_CHANNEL_FIRST

typedef struct {
    uint32_t frequency;
    bool tx_allowed;
} PMRLPD_Channel_t;

extern uint8_t gPMRLPD_Channel[2];
extern bool gPMRLPD_MemoryMode[2];

uint8_t PMRLPD_ChannelCount(void);
const PMRLPD_Channel_t *PMRLPD_GetChannel(uint8_t channel);
uint8_t PMRLPD_GetCurrentChannel(uint8_t vfo);
void PMRLPD_GetChannelName(uint8_t channel, char *name, uint8_t name_size);
bool PMRLPD_IsMemoryMode(uint8_t vfo);
void PMRLPD_Select(uint8_t vfo, uint8_t channel);
void PMRLPD_EnterMemoryMode(uint8_t vfo);
void PMRLPD_EnterVfoMode(uint8_t vfo);
void PMRLPD_ToggleMode(uint8_t vfo);
void PMRLPD_Move(uint8_t vfo, int8_t direction);
void PMRLPD_Apply(uint8_t vfo, VFO_Info_t *pInfo);
bool PMRLPD_IsAllowedTxFrequency(uint32_t frequency);
bool PMRLPD_CanTransmit(uint8_t vfo, const VFO_Info_t *pInfo);

#endif
