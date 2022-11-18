//per installare board esp32
//https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

//librerie
//https://github.com/pschatzmann/arduino-audio-tools
//https://github.com/pschatzmann/arduino-libhelix
//https://github.com/greiman/SdFat
//https://github.com/pschatzmann/ESP32-A2DP

//IMPORTANTE: TOOLS->PARTITITION SCHEME -> HUGE APP

//esempio collegamenti
//https://github.com/pschatzmann/arduino-audio-tools/tree/main/examples/examples-player/player-sdfat-a2dp

//#include "esp32-hal-cpu.h"
#include "AudioTools.h"
#include "AudioLibs/AudioA2DP.h"
#include "AudioLibs/AudioSourceSDFAT.h"
#include "AudioCodecs/CodecMP3Helix.h"

#define PIN_FOTOCELLULA_NC 4

class FP
{
  public:
    FP(bool init_val=false)
    {
      out=false;
      in_= init_val;
      in_old = init_val;
    }
    bool update(bool in_val)
    {
      in_=in_val;
      if (in_ && !in_old)
      {
        out=true;
      }
      else
      {
        out=false;
      }
			  
      in_old = in_;
      return out;
    }

  public:
    bool out;
  private:
    bool in_old;
    bool in_;

};

const char *startFilePath="/";
const char* ext="mp3";
AudioSourceSDFAT source(startFilePath, ext);
A2DPStream out = A2DPStream::instance();  // A2DP input - A2DPStream is a singleton!
MP3DecoderHelix decoder;
AudioPlayer player(source, out, decoder);

uint32_t cpu_clock=0;

FP fronte_discesa_fotocellula(true);
FP fronte_salita_fotocellula(true);

void setup() {
  Serial.begin(115200);

  pinMode(PIN_FOTOCELLULA_NC, INPUT);

  //setCpuFrequencyMhz(80); //Set CPU clock to 80MHz fo example
  //default è 240MHz
  //cpu_clock =  getCpuFrequencyMhz(); //Get CPU clock

  AudioLogger::instance().begin(Serial, AudioLogger::Warning);
  //AudioLogger::instance().begin(Serial, AudioLogger::Debug);

  // setup output - We send the test signal via A2DP - so we conect to the "T0-BT09" Bluetooth Speaker
  auto cfg = out.defaultConfig(TX_MODE);
  cfg.name = "T0-BT09";
  //cfg.auto_reconnect = true;  // if this is use we just quickly connect to the last device ignoring cfg.name
  out.begin(cfg);

  // setup player
  player.setVolume(0.9);
  player.begin();
  player.stop();
  
}

void loop() {
  
  bool isAssente = digitalRead(PIN_FOTOCELLULA_NC);

  fronte_discesa_fotocellula.update(!isAssente);
  fronte_salita_fotocellula.update(isAssente);
  if (fronte_discesa_fotocellula.out)
  {
    //falling edge
    //rilevata presenza
    player.play();
    Serial.println("Rilevata Presenza");
  }
  else if (fronte_salita_fotocellula.out)
  {
    //raising edge
    //rilevata assenza
    player.stop();
    Serial.println("Rilevata Assenza");
  }

  

  player.copy();

}
