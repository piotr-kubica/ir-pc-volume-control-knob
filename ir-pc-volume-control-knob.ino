/* ProMicro PIN <-> WIRE COLOR MAPPING

3 white oled SCK
2 violet oled SDA

7 btn bonze
8 btn green

A2 violet enc CLK
A1 gray enc DT
A0 white enc SW

*/
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#include <ClickEncoder.h>
#include <TimerOne.h>
#include <HID-Project.h>
#include <ClickButton.h>
#include <IRremote.h>

#define ENCODER_CLK A0
#define ENCODER_DT A1
#define ENCODER_SW A2
#undef  ENC_DOUBLECLICKTIME
#define ENC_DOUBLECLICKTIME  1500
#undef  ENC_HOLDTIME
#define ENC_HOLDTIME 2000 
#define STEPS_PER_NOTCH 4
ClickEncoder *rotEnc;
int16_t rotLast = 0;
int16_t rotValue = 0;
#define INVERT_KNOB_DIR 1

#define BTN_MODE 7
#define BTN_MUTE 8
ClickButton btn_mode(BTN_MODE, LOW);
ClickButton btn_mute(BTN_MUTE, LOW);

#define MODE_STEREO 0
#define MODE_PC 1
uint8_t mode = MODE_STEREO;
uint8_t mode_offset = 0;
uint8_t volume_step = 1;
String pc_header = String("PC       S: ");
String ste_header = String("STE      S: ");

IRsend irsend;
#define IR_KHZ 38
// no need to #define IR_SEND_PIN 5
// since it's default

#define OLED1306_SCK 3
#define OLED1306_SDA 2
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, OLED1306_SCK, OLED1306_SDA);

#define FIELDS 3
#if INVERT_KNOB_DIR
    #define FIELD_DIFF_NEXT FIELDS-1
    #define FIELD_DIFF_PREV 1
#else
    #define FIELD_DIFF_NEXT 1
    #define FIELD_DIFF_PREV FIELDS-1
#endif

int selected = 1;
int active_choice = false;
unsigned long millis_since_choice = 0;
unsigned long active_choice_timeout_ms = 2000;

// mycust font
#include "mycust_font.h"
// Font list: https://github.com/olikraus/u8g2/wiki/fntlistall
//  https://github.com/olikraus/u8g2/wiki/fntgrpiconic
const int GLYPHS[][3] = { 
    // PC
    { 8, 58, 'C'},  // prev
    {48, 58, 'A'}, // next
    {88, 58, 'B'}, // play/pause
    // STEREO
    {12, 54, 'A'}, // audio in
    {52, 54, 'T'}, // tuner
    {92, 54, 'Q'}, // on / off
};
const int BOX_POS[][4] = { 
    {6, 24, 36, 36},
    {46, 24, 36, 36},
    {86, 24, 36, 36},
};

// size of each array is 99
#define IR_CODE_SIZE 99
const uint16_t IR_AU_IN[]  PROGMEM = {3300,1700, 350,500, 350,450, 350,1350, 350,450, 350,1350, 350,450, 350,1300, 350,500, 350,500, 350,1300, 350,500, 350,450, 350,1300, 350,1350, 350,450, 350,500, 350,500, 350,450, 350,500, 350,450, 350,1350, 350,450, 350,1350, 350,450, 350,500, 350,1300, 350,500, 350,450, 350,500, 350,500, 350,450, 350,1300, 350,500, 350,500, 350,450, 350,500, 350,500, 350,450, 350,1300, 350,500, 350,500, 350,1300, 350,500, 350,450, 350,1300, 350,500, 350,500, 350,1300, 350};
const uint16_t IR_TUNER[]  PROGMEM = {3300,1700, 350,450, 400,450, 350,1300, 350,500, 350,1300, 400,450, 350,1300, 350,450, 400,450, 400,1250, 350,500, 400,450, 350,1300, 350,1300, 400,450, 350,450, 400,450, 400,450, 350,450, 400,450, 350,1300, 400,450, 350,1300, 350,500, 350,450, 400,1250, 400,1300, 350,450, 400,1300, 350,1300, 350,1300, 400,450, 350,450, 400,450, 400,450, 350,450, 400,450, 350,450, 400,1300, 350,450, 400,450, 350,1300, 400,1250, 400,450, 400,450, 350,1300, 350,1300, 400,450, 350};
const uint16_t IR_PWR[]    PROGMEM = {3250,1750, 300,500, 350,500, 300,1350, 350,500, 300,1350, 350,500, 300,1350, 300,550, 300,550, 300,1350, 300,550, 250,550, 300,1400, 250,1350, 350,500, 300,500, 350,550, 250,550, 300,500, 350,550, 250,1350, 350,500, 300,1350, 300,550, 300,1350, 300,550, 300,500, 350,500, 300,1350, 300,1350, 350,500, 300,550, 300,500, 350,500, 300,500, 350,500, 300,550, 300,500, 350,1350, 300,500, 300,1350, 350,500, 300,550, 300,500, 350,500, 300,1350, 300,550, 350,450, 350};
const uint16_t IR_MUTE[]   PROGMEM = {3300,1700, 400,450, 350,450, 400,1250, 400,450, 400,1250, 400,450, 400,1250, 400,450, 350,450, 400,1300, 350,450, 400,450, 400,1250, 400,1250, 400,450, 400,450, 350,450, 400,450, 350,450, 400,450, 400,1250, 400,450, 400,1250, 400,450, 350,1300, 400,450, 350,450, 400,450, 400,450, 350,1300, 350,1300, 400,450, 350,450, 400,450, 400,450, 350,450, 400,450, 350,450, 400,1300, 350,450, 400,1300, 350,450, 400,450, 350,450, 400,1300, 350,1300, 400,1250, 400,450, 350};
const uint16_t IR_VOL_UP[] PROGMEM = {3300,1700, 350,450, 400,450, 400,1250, 400,450, 350,1300, 400,450, 350,1300, 400,450, 350,450, 400,1300, 350,450, 400,450, 350,1300, 400,1250, 400,450, 350,450, 400,450, 400,450, 350,450, 400,450, 400,1250, 400,450, 350,1300, 400,450, 350,1300, 400,450, 350,450, 400,450, 350,450, 400,450, 400,1250, 400,450, 350,500, 350,450, 400,450, 350,450, 400,450, 400,450, 350,1300, 350,500, 350,1300, 350,450, 400,450, 400,450, 350,1300, 350,500, 350,1300, 350,450, 400};
const uint16_t IR_VOL_DN[] PROGMEM = {3250,1700, 350,500, 350,500, 350,1300, 350,500, 350,1300, 350,450, 350,1350, 350,450, 350,500, 350,1300, 350,500, 350,450, 350,1350, 350,1300, 350,500, 300,500, 350,500, 350,450, 350,500, 350,500, 350,1300, 350,500, 300,1350, 350,450, 350,1350, 350,450, 350,500, 350,500, 300,1350, 350,450, 350,1350, 350,450, 350,500, 350,500, 300,500, 350,500, 350,450, 350,500, 350,1300, 350,500, 350,1300, 350,500, 350,450, 350,500, 350,500, 350,450, 350,1350, 300,500, 350};

const uint16_t * const ir_codes[] = {IR_AU_IN, IR_TUNER, IR_PWR, IR_MUTE, IR_VOL_UP, IR_VOL_DN};

#define AU_IN 0
#define TUNER 1
#define PWR 2
#define MUTE 3
#define VOL_UP 4
#define VOL_DN 5

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void refresh_active_choice_timeout() {
    millis_since_choice = millis();
}

void clear_active_choice() {
    millis_since_choice = millis() + active_choice_timeout_ms;
}

void ir_send(int ir_id) {
    unsigned int code[99];
    for (byte i = 0; i < IR_CODE_SIZE; i++) {
        code[i] = pgm_read_word_near(ir_codes[ir_id] + i);
        Serial.print(String(code[i]) + " ");
    }
    Serial.println();
    irsend.sendRaw(code, sizeof(code) / sizeof(code[0]), IR_KHZ);
}

void vol_up() {
    if (mode == MODE_PC) {
        int step_copy = volume_step;
        while(step_copy--) {
            Consumer.write(MEDIA_VOLUME_UP);
        }
    } else { // STEREO
        int step_copy = volume_step;
        while(step_copy--) {
            ir_send(VOL_UP);
            delay(200);
        }
    }
}

void vol_dn() {
    if (mode == MODE_PC) {
        int step_copy = volume_step;
        while(step_copy--) {
            Consumer.write(MEDIA_VOLUME_DOWN);
        }
    } else { // STEREO
        int step_copy = volume_step;
        while(step_copy--) {
            ir_send(VOL_DN);
            delay(200);
        }
    }
}

void handleRotEncoderTurn() {
    int16_t encValue = rotEnc->getValue();
    rotValue += encValue;

    if (rotValue > rotLast) {
        if(!active_choice) {
            vol_dn();
        } else {
            selected = (selected + FIELD_DIFF_NEXT)%FIELDS;    
        }
    } else if(rotValue < rotLast) {
        if(!active_choice) {
            vol_up();
        } else {
            selected = (selected + FIELD_DIFF_PREV)%FIELDS;
        }
    }
    if (rotLast != rotValue) {
        refresh_active_choice_timeout();
    }
    rotLast = rotValue;
}

void handleRotEncoderClick(){
    ClickEncoder::Button btn = rotEnc->getButton();
    
    if (btn == ClickEncoder::Clicked) {
        Serial.println("Rot encoder Click");
        if(!active_choice) {
            active_choice = true;
            refresh_active_choice_timeout();
        } else {
            if(mode == MODE_PC){
                if (selected==0) {
                    Consumer.write(MEDIA_PREVIOUS);
                } else if (selected==1) {
                    Consumer.write(MEDIA_NEXT);
                } else if (selected==2) {
                    Consumer.write(MEDIA_PLAY_PAUSE);
                }
            } else {
                if (selected==0) {
                    ir_send(AU_IN);
                } else if (selected==1) {
                    ir_send(TUNER);
                } else if (selected==2) {
                    ir_send(PWR);
                }
            }
            clear_active_choice();
        }
    } else if(btn == ClickEncoder::DoubleClicked) {
        Serial.println("Rot encoder 2-Click");
        volume_step = volume_step%2 + 1;
    }
}

void timerIsr() {
  rotEnc->service();
}

void detect_buttons() {
    btn_mode.Update();
    if (btn_mode.changed) {
        Serial.println("btn_mode");
        mode = (mode+1) % 2;
        
        refresh_active_choice_timeout();
    }
    
    btn_mute.Update();
    if (btn_mute.changed) {
        // Serial.println("btn_mute");
        if (mode == MODE_PC) {
            // Serial.println("[PC] mute");
            Consumer.write(MEDIA_VOLUME_MUTE);
        } else {
            // Serial.println("[STEREO] mute");
            ir_send(MUTE);
        }
        refresh_active_choice_timeout();
    }
}

void display_oled() {
    u8g2.firstPage();  
    do {
      u8g2.setFontMode(1);  /* activate transparent font mode */
      u8g2.setDrawColor(1); /* color 1 for the box */
      u8g2.drawBox(0, 0, 128, 16);
      u8g2.setDrawColor(0);
      u8g2.setFont(u8g2_font_fub14_tr);
      
      if (mode == MODE_PC) {
        u8g2.drawStr(8, 16, (pc_header+volume_step).c_str());
        u8g2.setFont(mycust_font);
        mode_offset = 0;
      } else {
        u8g2.drawStr(8, 16, (ste_header+volume_step).c_str());
        u8g2.setFont(u8g2_font_fub25_tr);
        mode_offset = 3;
      }

      for(int i=0; i<FIELDS; i++) {
        u8g2.setDrawColor(1);
        if(i==selected) {
            if(active_choice) {
                u8g2.drawBox(BOX_POS[i][0], BOX_POS[i][1], BOX_POS[i][2], BOX_POS[i][3]);
                u8g2.setDrawColor(0);
            } else {
                u8g2.drawFrame(BOX_POS[i][0], BOX_POS[i][1], BOX_POS[i][2], BOX_POS[i][3]);
            }
            u8g2.drawGlyph(GLYPHS[i+mode_offset][0], GLYPHS[i+mode_offset][1], GLYPHS[i+mode_offset][2]);    
            
        } else {
            u8g2.drawGlyph(GLYPHS[i+mode_offset][0], GLYPHS[i+mode_offset][1], GLYPHS[i+mode_offset][2]);
        }
      }  
      
      detect_buttons();
    } while( u8g2.nextPage() );
}

void setup(void) {
    Serial.begin(9600);

    #if defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
    #endif
    
    u8g2.begin();
    
    // Initializes the rotary encoder with the mentioned pins
    rotEnc = new ClickEncoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW, STEPS_PER_NOTCH); 
    Timer1.initialize(500); // Initializes the timer, which the rotary encoder uses to detect rotation
    Timer1.attachInterrupt(timerIsr); 

    btn_mode.multiclickTime = 150; // default is 250
    btn_mute.multiclickTime = 150; // default is 250

    pinMode(BTN_MODE, INPUT_PULLUP);
    pinMode(BTN_MUTE, INPUT_PULLUP);
}

void loop(void) {
    handleRotEncoderTurn();
    handleRotEncoderClick();

    // detect_buttons() is inside display_oled to reduce lag
    if (millis() - millis_since_choice > active_choice_timeout_ms) {
        active_choice = false;
    }
    display_oled();
    delay(5);
}
