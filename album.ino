#include <Adafruit_GFX.h> 
#include <Adafruit_ILI9341.h> 
#include <SDHCI.h>
#include <stdlib.h>

#define TFT_CS -1  
#define TFT_RST 8  
#define TFT_DC  9  
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);
File thePic;
SDClass theSD;

String album[10]={"PIC/qwe.bmp","PIC/asd.bmp","PIC/zxc.bmp","PIC/rty.bmp","PIC/fgh.bmp","PIC/vbn.bmp","PIC/uio.bmp","PIC/jkl.bmp","PIC/nm.bmp","PIC/qwer.bmp"};
int currentPosition=0;
int point=0;
 short color =0;
 int  x=0;
 int  y=0;
  unsigned int  Size=0;
 int sizeOfb=0;
 int width=0;
 int hight=0;
 int blue=0,green=0,red=0;

void getInfo(){
  //Read the Size if pic 
  
  point=0;
  color =0;
  x=0;
  y=0;
  Size=0;
  sizeOfb=0;
  width=0;
  hight=0;
  thePic.seek(2);
  for(int i=0;i<4;i++){
    Size += thePic.read()*pow(256,i);
  }
  printf("Size:%d\n",Size);
  //Read the beginning of data of pic
  thePic.seek(10);
  
  for(int i=0;i<4;i++){
    sizeOfb += thePic.read()*pow(256,i);
  }
printf("sizoOfb:%d\n",sizeOfb);
  //Read the width of the pic 

  thePic.seek(18);
  for(int i=0;i<4;i++){
    width += thePic.read()*pow(256,i);
  }
  x=width;
printf("weith:%d\n",width);
  //Read the hight of the pic 
  
  for(int i=0;i<4;i++){
    hight += thePic.read()*pow(256,i);
  }
  y=hight;
  printf("height:%d\n",hight);
}

void pic(){
  int ends=0;
  int begins=0;
  
  
  //move the pointer to the head of the metadata of the pic
  thePic.seek(14+sizeOfb);
  
  begins=thePic.position();
  ends=Size;
  printf("begins:%d\n",begins);
  printf("ends:%d\n",ends);
  for(int i=begins;i<ends;i+=3){
    for(int j=0;j<3;j++){
      if(j == 0)
       blue = thePic.read()>>3;
      else if(j == 1)
       green = (thePic.read()>>2)<<5;
      else 
       red = (thePic.read()>>3)<<11;
    }
    color = red | blue | green;
    if(x == 0){
      x=width;
      y--;
    }
    tft.fillRect(x--,y,1,1,color);
  }
  
}

void selfDrawing(){
      tft.fillScreen(0x07e00);
  for(int i=0;i<240;i++)
  tft.drawLine(i,i,i+1,i+1,250*i);
  for(int i=0;i<240;i++){
    tft.drawLine(i,0,240,i,200*i);
  }
  for(int i=0;i<240;i++){
    tft.drawLine(0,i,i,240,200*i);
  }
}
void displayPic(int index){
  thePic = theSD.open(album[index]);
  getInfo();
  pic();
  thePic.close();
  printf("\n");
};

void next(){
  if(currentPosition == 10){
    selfDrawing();
  }
  else 
  displayPic(++currentPosition);
}

void pre(){
  if(currentPosition == 0){
    printf("This is the first Picture!\n");
    return;
  }
  else 
  displayPic(--currentPosition);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  tft.begin(40000000);  
  tft.setRotation(3); 
  tft.fillScreen(0x000f);
  if(!theSD.begin()){
    printf("please insert SD card\n");
  }
  thePic = theSD.open("PIC/qwe.bmp",FILE_READ);
  if(!thePic)
  {
    printf("Something is wrong with the file!\n");
  }
  getInfo();
  pic();
  thePic.close();
}
char operation;
char clear1;
void loop() {
  
    if(Serial.available()){
        operation = Serial.read();
      }
    clear1=Serial.read();
    if(clear1 == '\n'){
      printf("capture \\n");
    }
    switch(operation){
      case 'N':
      next();
      break;
      case 'P':
      pre();
      break;
      case 'A':{
      
      displayPic(++currentPosition);
      printf("\nsleep\n");
      sleep(5);
      if(currentPosition==10)
      currentPosition=-1;
      }
      break;
      case 'S':
      break;
      default:
      
      break;
    }
  

}
