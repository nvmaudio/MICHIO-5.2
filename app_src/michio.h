#ifndef __MICHIO_H__
#define __MICHIO_H__

void MUTE(void);
void UN_MUTE(void);
void MUTE_ON_OFF();

void APP_MUTE_CMD(bool Mute);
void REMIND_CMD(bool Mute);



bool get_APP_MUTE(void);



void VR_LED_CMD(uint8_t VR_INDEX);

void Michio(void);
#endif
