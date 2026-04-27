#include "app/pmr_lpd.h"

#include <string.h>

#include "misc.h"
#include "settings.h"
#include "ui/ui.h"

uint8_t gPMRLPD_Channel[2] = {0, 16};
bool gPMRLPD_MemoryMode[2] = {true, false};

static const PMRLPD_Channel_t gPMRLPD_Channels[] = {
    {44600625, true}, {44601875, true}, {44603125, true}, {44604375, true},
    {44605625, true}, {44606875, true}, {44608125, true}, {44609375, true},
    {44610625, true}, {44611875, true}, {44613125, true}, {44614375, true},
    {44615625, true}, {44616875, true}, {44618125, true}, {44619375, true},

    {43307500, true}, {43310000, true}, {43312500, true}, {43315000, true},
    {43317500, true}, {43320000, true}, {43322500, true}, {43325000, true},
    {43327500, true}, {43330000, true}, {43332500, true}, {43335000, true},
    {43337500, true}, {43340000, true}, {43342500, true}, {43345000, true},
    {43347500, true}, {43350000, true}, {43352500, true}, {43355000, true},
    {43357500, true}, {43360000, true}, {43362500, true}, {43365000, true},
    {43367500, true}, {43370000, true}, {43372500, true}, {43375000, true},
    {43377500, true}, {43380000, true}, {43382500, true}, {43385000, true},
    {43387500, true}, {43390000, true}, {43392500, true}, {43395000, true},
    {43397500, true}, {43400000, true}, {43402500, true}, {43405000, true},
    {43407500, true}, {43410000, true}, {43412500, true}, {43415000, true},
    {43417500, true}, {43420000, true}, {43422500, true}, {43425000, true},
    {43427500, true}, {43430000, true}, {43432500, true}, {43435000, true},
    {43437500, true}, {43440000, true}, {43442500, true}, {43445000, true},
    {43447500, true}, {43450000, true}, {43452500, true}, {43455000, true},
    {43457500, true}, {43460000, true}, {43462500, true}, {43465000, true},
    {43467500, true}, {43470000, true}, {43472500, true}, {43475000, true},
    {43477500, true},
};

uint8_t PMRLPD_ChannelCount(void)
{
    return ARRAY_SIZE(gPMRLPD_Channels);
}

const PMRLPD_Channel_t *PMRLPD_GetChannel(uint8_t channel)
{
    if (channel >= PMRLPD_ChannelCount()) {
        channel = 0;
    }
    return &gPMRLPD_Channels[channel];
}

static uint8_t PMRLPD_NormalizeVfo(uint8_t vfo)
{
    return (vfo < 2) ? vfo : 0;
}

bool PMRLPD_IsMemoryMode(uint8_t vfo)
{
    return gPMRLPD_MemoryMode[PMRLPD_NormalizeVfo(vfo)];
}

uint8_t PMRLPD_GetCurrentChannel(uint8_t vfo)
{
    return gPMRLPD_Channel[PMRLPD_NormalizeVfo(vfo)];
}

void PMRLPD_GetChannelName(uint8_t channel, char *name, uint8_t name_size)
{
    if (name == NULL || name_size < 7) {
        return;
    }

    uint8_t number;
    if (channel < 16) {
        name[0] = 'P';
        name[1] = 'M';
        name[2] = 'R';
        number = channel + 1;
    } else {
        name[0] = 'L';
        name[1] = 'P';
        name[2] = 'D';
        number = channel - 15;
    }

    name[3] = ' ';
    name[4] = '0' + (number / 10);
    name[5] = '0' + (number % 10);
    name[6] = '\0';
}

void PMRLPD_Select(uint8_t vfo, uint8_t channel)
{
    vfo = PMRLPD_NormalizeVfo(vfo);
    gPMRLPD_Channel[vfo] = channel % PMRLPD_ChannelCount();
    gPMRLPD_MemoryMode[vfo] = true;
    gEeprom.ScreenChannel[vfo] = PMR_LPD_EEPROM_CHANNEL;
    gEeprom.VFO_OPEN = true;
    gUpdateStatus = true;
    gUpdateDisplay = true;
    gRequestDisplayScreen = DISPLAY_MAIN;
}

void PMRLPD_EnterMemoryMode(uint8_t vfo)
{
    PMRLPD_Select(vfo, PMRLPD_GetCurrentChannel(vfo));
}

void PMRLPD_EnterVfoMode(uint8_t vfo)
{
    vfo = PMRLPD_NormalizeVfo(vfo);
    gPMRLPD_MemoryMode[vfo] = false;

    if (!IS_FREQ_CHANNEL(gEeprom.FreqChannel[vfo]) || gEeprom.FreqChannel[vfo] == PMR_LPD_EEPROM_CHANNEL) {
        gEeprom.FreqChannel[vfo] = FREQ_CHANNEL_FIRST + BAND6_400MHz;
    }

    gEeprom.ScreenChannel[vfo] = gEeprom.FreqChannel[vfo];
    gEeprom.VFO_OPEN = true;
    gUpdateStatus = true;
    gUpdateDisplay = true;
    gRequestDisplayScreen = DISPLAY_MAIN;
}

void PMRLPD_ToggleMode(uint8_t vfo)
{
    if (PMRLPD_IsMemoryMode(vfo)) {
        PMRLPD_EnterVfoMode(vfo);
    } else {
        PMRLPD_EnterMemoryMode(vfo);
    }
}

void PMRLPD_Move(uint8_t vfo, int8_t direction)
{
    vfo = PMRLPD_NormalizeVfo(vfo);
    int16_t channel = gPMRLPD_Channel[vfo] + direction;
    if (channel < 0) {
        channel = PMRLPD_ChannelCount() - 1;
    } else if (channel >= PMRLPD_ChannelCount()) {
        channel = 0;
    }
    PMRLPD_Select(vfo, (uint8_t)channel);
}

void PMRLPD_Apply(uint8_t vfo, VFO_Info_t *pInfo)
{
    const PMRLPD_Channel_t *channel = PMRLPD_GetChannel(PMRLPD_GetCurrentChannel(vfo));
    char name[8];

    RADIO_InitInfo(pInfo, PMR_LPD_EEPROM_CHANNEL, channel->frequency);
    pInfo->STEP_SETTING = STEP_6_25kHz;
    pInfo->StepFrequency = gStepFrequencyTable[pInfo->STEP_SETTING];
    pInfo->CHANNEL_BANDWIDTH = BANDWIDTH_NARROW;
    pInfo->OUTPUT_POWER = OUTPUT_POWER_HIGH;
    pInfo->TX_LOCK = !channel->tx_allowed;
    PMRLPD_GetChannelName(PMRLPD_GetCurrentChannel(vfo), name, sizeof(name));
    strncpy(pInfo->Name, name, sizeof(pInfo->Name) - 1);
}

bool PMRLPD_IsAllowedTxFrequency(uint32_t frequency)
{
    for (uint8_t i = 0; i < PMRLPD_ChannelCount(); ++i) {
        if (gPMRLPD_Channels[i].frequency == frequency && gPMRLPD_Channels[i].tx_allowed) {
            return true;
        }
    }
    return false;
}

bool PMRLPD_CanTransmit(uint8_t vfo, const VFO_Info_t *pInfo)
{
    vfo = PMRLPD_NormalizeVfo(vfo);

    return gPMRLPD_MemoryMode[vfo] &&
           gEeprom.ScreenChannel[vfo] == PMR_LPD_EEPROM_CHANNEL &&
           pInfo != NULL &&
           pInfo->CHANNEL_SAVE == PMR_LPD_EEPROM_CHANNEL &&
           PMRLPD_IsAllowedTxFrequency(pInfo->pTX->Frequency);
}
