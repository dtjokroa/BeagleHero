#ifndef _BUZZER_H
#define _BUZZER_H

#define BUZZER_PERIOD "/dev/bone/pwm/0/a/period"
#define BUZZER_DUTY_CYCLE "/dev/bone/pwm/0/a/duty_cycle"
#define BUZZER_ENABLE "/dev/bone/pwm/0/a/enable"
#define BUZZER_COMMAND "sudo config-pin p9_22 pwm"
#define NOTES 12
#define OCTAVES 9
#define FREQ_LEN 30

// Configures the pin on the BeagleBone to use PWM
void Buzzer_configurePin(char *command);

// Plays a note from the buzzer 
void Buzzer_playNote(char *note, int octave, double duration);
void Buzzer_playNoteAtIndex(int index);


// Start and stop the buzzer thread 
void Buzzer_start(void);
void Buzzer_stop(void);

#endif