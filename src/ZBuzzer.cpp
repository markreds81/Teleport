#include "ZBuzzer.h"
#include "ZDebug.h"
#include "z/options.h"

const unsigned short ZBuzzer::melodyNotes[8] PROGMEM = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};

const unsigned char ZBuzzer::noteDurations[8] PROGMEM = {
    4, 8, 8, 4, 4, 4, 4, 4};

ZBuzzer::ZBuzzer(/* args */)
{
}

ZBuzzer::~ZBuzzer()
{
}

void ZBuzzer::tone(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t channel)
{
    if (ledcRead(channel))
    {
        DPRINTF("Tone channel %d is already in use\n", channel);
        return;
    }
    ledcAttachPin(pin, channel);
    ledcWriteTone(channel, frequency);
    if (duration)
    {
        delay(duration);
        noTone(pin, channel);
    }
}

void ZBuzzer::noTone(uint8_t pin, uint8_t channel)
{
    ledcDetachPin(pin);
    ledcWrite(channel, 0);
}

void ZBuzzer::playTune()
{
    for (int thisNote = 0; thisNote < 8; thisNote++)
    {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(PIN_BUZZER, melodyNotes[thisNote], noteDuration, BUZZER_CHANNEL);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(PIN_BUZZER, BUZZER_CHANNEL);
    }
    vTaskDelete(NULL);
}