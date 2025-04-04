#include<SDHCI.h>
#include<Audio.h>
#include<EEPROM.h>
#include<audio/utilities/playlist.h>

AudioClass *theAudio;
SDClass theSD;
File myFile;
//------------------------------------------
unsigned int eeprom_idx=0;
bool ErrEnd = false;
int index1 = 0;
bool isInit = false;
Track currentTrack;
Track musics[6] = {
  {"AUDIO/music_1.mp3", "周传雄 ", "恋人创世纪", 2, 8, 44100, AS_CODECTYPE_MP3},
  {"AUDIO/music_2.mp3", "周传雄", "我的心太乱", 2, 8, 44100, AS_CODECTYPE_MP3},
  {"AUDIO/music_3.MP3", "Niykee Heaton", "Bad Intentions", 2, 8, 44100, AS_CODECTYPE_MP3},
  {"AUDIO/music_4.mp3", "One Direction", "What Makes You Beautiful", 2, 8, 44100, AS_CODECTYPE_MP3},
  {"AUDIO/sound.mp3", "庞龙", "两只蝴蝶", 2, 8, 44100, AS_CODECTYPE_MP3},
  {"AUDIO/sound.wav", " ", " ", 2, 8,441000 , AS_CODECTYPE_WAV}
};
struct savedObject {
  int num = 0;
  int saved = 1;
  int volume;
  int random;
  int repeat;
  int autoplay;
} preset;
//------------------------------------------
static err_t initPlay() {
  Track t = currentTrack;
  err_t err = AUDIOLIB_ECODE_OK;
  static uint8_t   s_codec   = 0;
  static uint32_t  s_fs      = 0;
  static uint8_t   s_bitlen  = 0;
  static uint8_t   s_channel = 0;
  static AsClkMode s_clkmode = (AsClkMode) - 1;
  AsClkMode clkmode;
  if ((s_codec   != t.codec_type) ||
      (s_fs      != t.sampling_rate) ||
      (s_bitlen  != t.bit_length) ||
      (s_channel != t.channel_number)) {
    clkmode = (t.sampling_rate <= 48000) ? AS_CLKMODE_NORMAL : AS_CLKMODE_HIRES;
    if (s_clkmode != clkmode) {

     if (s_clkmode != (AsClkMode) - 1) {
        theAudio->setReadyMode();
      }

      s_clkmode = clkmode;

      theAudio->setRenderingClockMode(clkmode);


      theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
    }
    err = theAudio->initPlayer(AudioClass::Player0,
                               t.codec_type,
                               "/mnt/sd0/BIN",
                               AS_SAMPLINGRATE_AUTO,
                               AS_CHANNEL_STEREO);
    //if (err != AUDIOLIB_ECODE_OK) {
     // printf("Player0 initialize error\n");
     // return err;
    //}
    s_codec   = t.codec_type;
    s_fs      = t.sampling_rate;
    s_bitlen  = t.bit_length;
    s_channel = t.channel_number;
  }
  return err;
}

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
    ErrEnd = true;
  }
}

bool play() {
  err_t  err = initPlay();
  initPlay();
  Track * t = &currentTrack;

  if (err != AUDIOLIB_ECODE_OK) {
    printf("Player0 initialize error\n");
    return false;
  }
  String fullpath = t->title;

  myFile = theSD.open(fullpath);

  if (!myFile) {
    printf("File open error\n");
    return false;
  }

  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if ((err != AUDIOLIB_ECODE_OK) && (err != AUDIOLIB_ECODE_FILEEND)) {
    printf("File Read Error! =%d\n", err);
    return false;
  }

  printf("start\n");
  theAudio->startPlayer(AudioClass::Player0);

  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, index1 >> 2 & 0x001);
  digitalWrite(LED2, index1 >> 1 & 0x001);
  digitalWrite(LED3, index1 >> 0 & 0x001);
  return true;
}

void showAll(){
  Track t ;
  for(int i=0;i<6;i++){
          printf(i+1);
          t = musics[i];
          show(&t);
        }
}

void stop() {
  printf("stop\n");
  theAudio->stopPlayer(AudioClass::Player0, AS_STOPPLAYER_NORMAL);
  myFile.close();
  
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}

void next() {
  if (index1++ != 5)
    currentTrack = musics[index1];
  else
    currentTrack = musics[index1 = 0];
}

void pre() {
  if (index1-- != 0)
    currentTrack = musics[index1];
  else
    currentTrack = musics[index1 = 5];
}

static void show(Track *t)
{
  printf("%s | %s | %s | %d | %d | %d | %d \n", t->author, t->album, t->title,t->channel_number,t->bit_length,t->sampling_rate,t->codec_type);
}

static void menu()
{
  printf("=== MENU (input key ?) ==============\n");
  printf("p: play  s: stop  +/-: volume up/down\n");
  printf("l: list  n: next  b: back\n");
  printf("1-6: play the music with given index1\n");
  printf("m,h,?: menu\n");
  printf("i: show information of the currnt music\n");
  printf("=====================================\n");
}

void beep(){
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP,0, 0);
  
  theAudio->setBeep(1, -50, 250);
  sleep(1);
  theAudio->setBeep(0, 0, 0);
  theAudio->setReadyMode();
}
void setup() {

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  Serial.begin(115200);

  if (!preset.saved) {
    /* If no preset data, come here */
    preset.saved = 1;
    preset.volume = -160; /* default */
    preset.random = 0;
    preset.repeat = 0;
    preset.autoplay = 0;
    EEPROM.put(eeprom_idx, preset);
  }

  theAudio = AudioClass::getInstance();
  
  theAudio->begin(audio_attention_cb);
  puts("initialization Audio Library");
 
  /*theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
  
  theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);


  myFile = theSD.open("music_1.mp3");
  
  theAudio->writeFrames(AudioClass::Player0, myFile);
  
  
   
  theAudio->startPlayer(AudioClass::Player0);*/
  theAudio->setVolume(preset.volume);
  if (!theSD.begin()) {
    printf("No SD Card!");
  }

  currentTrack = musics[0];
  menu();
}

void loop() {
  
  static enum State {
    Stopped,
    Ready,
    Active
  } s_state = preset.autoplay ? Ready : Stopped;
  err_t err = AUDIOLIB_ECODE_OK;

  if (ErrEnd) {
    printf("Error End\n");
    goto stop_player;
  }

  char operate ;
  if (Serial.available() > 0) {
    operate = Serial.read();
    Serial.read();
    switch (operate) {
      case 'p': // play
      printf("play\n");
        if (s_state == Stopped) {
          s_state = Ready;
          show(&currentTrack);
        }
        break;
      case 'n': // next
      printf("next\n");
        if (s_state == Active) {
          stop();
        }
        next();
        s_state = Ready;
        show(&currentTrack);
        break;
      case 'b': // pre
      printf("next\n");
        if (s_state == Active) {
          stop();
        }
        pre();
        s_state = Ready;
        show(&currentTrack);
        break;
      case 's': // stop
      printf("stop\n");
        if (s_state == Active) {
          stop();
        }
        s_state = Stopped;
        break;
      case 'i': // show
        
        if(s_state == Active)
          show(&currentTrack);
        else 
          printf("infomation\n");
          printf("No music is playing!");
        break;
      case 'l': // list
        printf("list\n");
        showAll();
        
        break;
      case '1':
      printf("1\n");
      if(s_state == Active){
        stop();
      }
        currentTrack = musics['1'-'0'-1];
        s_state = Ready;
        index1 = 0;
        break;
      case '2':
      printf("2\n");
      if(s_state == Active){
        stop();
      }
        currentTrack = musics['2'-'0'-1];
        s_state = Ready;
        index1 = 1; 
        break;
      case '3':
      printf("3\n");
      if(s_state == Active){
        stop();
      }
        currentTrack = musics['3'-'0'-1];
        s_state = Ready;
        index1 = 2;
        break;
      case '4':
      printf("4\n");
      if(s_state == Active){
        stop();
      }
        currentTrack = musics['4'-'0'-1];
        s_state = Ready;
        index1 = 3;
        break;
      case '5':
      printf("5\n");
      if(s_state == Active){
        stop();
      }
        currentTrack = musics['5'-'0'-1];
        s_state = Ready;
        index1 = 4;
        break;
      case '6':
      printf("6\n");
      if(s_state == Active){
        stop();
      }
        currentTrack = musics['6'-'0'-1];
        s_state = Ready;
        index1 = 5;
        break;
      case 'h':
      case 'm':
      case '?':
      menu();
      break;
      default:
      beep();
  }
}
switch (s_state) {
case Stopped:
  break;

case Ready:
if(play())
    s_state = Active;
  else 
  goto stop_player;
  break;

case Active:
  /* Send new frames to be decoded until end of file */

  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if (err == AUDIOLIB_ECODE_FILEEND) {
    /* Stop player after playback until end of file */

    theAudio->stopPlayer(AudioClass::Player0, AS_STOPPLAYER_ESEND);
    myFile.close();
    next();
    s_state = Ready;

  } else if (err != AUDIOLIB_ECODE_OK) {
    printf("Main player error code: %d\n", err);
    goto stop_player;
  }
  break;

default:
  break;
}
//theAudio->writeFrames(AudioClass::Player0, myFile);
usleep(40000);

return;

stop_player:
theAudio->stopPlayer(AudioClass::Player0);
myFile.close();
theAudio->setReadyMode();
theAudio->end();
printf("Exit player\n");
exit(1);
}
