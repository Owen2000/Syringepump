#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <stdint.h>
#include "TouchScreen.h"

#include <Wire.h>

// == TFT Touch Screen System ==
//TFT 3.5
#define YP A2
#define XM A3
#define YM 8
#define XP 9
/*
//TFT 2.4
#define YP A1
#define XM A2
#define YM 7
#define XP 6
*/
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define minpressure 250
#define maxpressure 1000
TouchScreen ts = TouchScreen(XP,YP,XM,YM,300);
int X,Y;

MCUFRIEND_kbv tft;

// ============ Sound (Indikator) ===============
int piezoPin = 31; //tone

#define doo   262
#define re    294
#define mi    330
#define fa    349
#define sol   395
#define la    440
#define si    494
#define dooo  523

// ============ warna LCD ===============
#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */
// ========== warna costum ===========
#define LightGreen      tft.color565(237,255,159)

// ======== Menyatakan keadaan =========
int currentdisplay = 0; //keadaan layar
int displaylama = 0; //untuk membantu menstabilkan layar apabila keadaan layar sama seperti sebelumnya
/*
 * 0  : display Menu 
 * 1  : display Bolus 
 * 2  : display Mode Loading Dose 
 * 3  : display Mode ml/h 
 * 4  : display Mode Ramp Up/Down
 * 5  : display Mode TIVA
 * 6  : display Mode weight
 * 7  : display Mode Sequence
 * 8  : display Input angka 
 * 9  : display Menampilkan Input
 * 10 : display Run Bolus
 * 11 : display Run ml/h
 * 12 : display Run Weight
 * 13 : display Menu Conc.
 * 14 : display Error Syringe tidak terdeteksi
 * 15 : display Error VTBI lebih besar dari ukuran syringe
 * 16 : display Error Jarka
 */
int dis = 0; //kondisi yang sedang terjadi
/*
 * 0    : nothing happen
 * 1    : keadaan Mode Bolus
 * 2    : keadaan Mode lainnya terpilih
 * 3    : keadaan Mode ml/h terpilih
 * 4    : keadaan Mode weight terpilih
 * 11   : [ml/h] keadaan Input VTBI
 * 12   : [ml/h] keadaan Input Rate
 * 13   : [ml/h] keadaan Input Time
 * 21   : [weight] keadaan Input VTBI
 * 22   : [weight] keadaan Input Rate
 * 23   : [weight] keadaan Conc. Terpilih
 * 231  : [weight, Conc.] keadaan Input Acti Agentia //massa obat
 * 232  : [weight, Conc.] keadaan Input Volume
 * 233  : [weight, Conc.] keadaan Input Conc.
 * 24   : [weight] keadaan Input Weight
 * 01   : Keadaan Run Bolus
 * 02   : keadaan Run ml/h
 * 03   : keadaan Run Weight
 * 101  : keadaan error bolus
 * 102  : keadaan error ml/h
 * 103  : keadaan error weight
 */
 
 // ===== variable angka =====
int angka       = 0; //input untuk menjalankan mode
int bolust      = 5; //sec
int volumeinj   = 10; //ml
int nilairate   = 0; //ml/h
int waktu       = 60; //detik
int konsentrasi = 0; //mg/ml
int beratbadan  = 40; //kg
int vob         = 0; //ml
int acti        = 0; //mg

// ====== variable untuk menjalankan syringe ======
#define pul 45
#define dir 43
#define rev 3200
float f,r,i;
unsigned long ml;
unsigned long sec;
unsigned long t;


float s,j,mls;
unsigned short lenth_val = 0;
unsigned char i2c_rx_buf[16];

// ====== flesibilitas syringe ========
int potmeterPin = A15;
int potmeterval = 0;
int ukuransyringe = 10;
int ukuransyringelama = 10;

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  Wire.begin();

  pinMode(dir,OUTPUT);
  pinMode(pul,OUTPUT);
  pinMode(piezoPin, OUTPUT); 

  tft.begin(0x9486);
  tft.setRotation(3);
  pembukaan();
  Menutetap();
  c_display(0);

}

void loop() {
  TSPoint p = waitTouch();
  Serial.println("");
  Serial.println("--------------------------");
  Y = p.y; X = p.x;
  Serial.print("X : "); Serial.print(X);
  Serial.print("\t Y : "); Serial.println(Y);

  // --- Sistem Kontrol Touch Screen ---
  if(dis == 0){ //pasti menu
    menutouch();
    pulltouch();
  }
  else if(dis !=0){ //banyak
    //bolus
    if(dis == 1){
      bolustouch();
      pulltouch();
      DetectButtons();
    }
    //mode lainnya
    else if(dis == 2){
      laintouch();
      pulltouch();
    }
    //ml/h
    else if(dis == 3){//
      mlhtouch();
      pulltouch();
      backmenu();
    }
    //weight
    else if(dis == 4){
      weighttouch();
      pulltouch();
      backmenu();
    }
    //conc. menu
    else if(dis == 23){
      conctouch();
      pulltouch();
      backweight();
    }
    //kondisi ml/h mode
    else if(dis == 11 | dis == 12 | dis == 13){
      backmlh();
      DetectButtons();
    }
    //kondisi weight mode
    else if(dis == 21 | dis == 22 | dis == 24){
      backweight();
      DetectButtons();
    }
    //kondisi conc. mode
    else if(dis == 231 | dis == 232 | dis == 233){
      backconc();
      DetectButtons();
    }
    else if(dis == 101 | dis == 102 | dis == 103){
      pulltouch();
      errortouch();
    }
  }
  
  if(currentdisplay != displaylama){
    displaylama = currentdisplay;
    c_display(currentdisplay);
  }
  else if(currentdisplay == 9){ //menampilkan kembali angka yang diinput
    c_display(currentdisplay);
  }

  if(dis == 101 | dis == 102 | dis == 103){
      warning();
  }
  
  Serial.println(" ");
  displaynilaiinput(); //menampilakan nilai dari setiap variable
  Serial.println("--------------------------");
  Serial.println(" ");
  delay(200);
}

TSPoint waitTouch() {
  TSPoint p;
  do {
    p = ts.getPoint(); 
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    DetectSyringe();
    // menstabilkan layar
    // apabila ukuran yang terdeteksi sama dengan ukuran sebelumnya, sistem baru akan mengubah ukuran pada display TFT
    if(ukuransyringe != ukuransyringelama){
      ukuransyringelama = ukuransyringe;
      tft_flexsyringe();
    }
  } while((p.z < minpressure )|| (p.z > maxpressure));
  return p;
}

// --- Sistem Kontrol Tampilan ---
void c_display(int id_hal){
  switch(id_hal){
    //Menu
    case 0 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    delay(100);
    tft_print("SELECT MODE", 3, 5,10,White,DarkGreen); 
    HomeButton();
    menu();
    break;

    //Bolus
    case 1 : 
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    RunButton(); //tampilan run button
    delay(100);
    tft_print("BOLUS", 3, 5,10,White,DarkGreen);
    inputangka();
    break;

    //Mode Loading Dose
    case 2 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    delay(100);
    tft_print("LOAD DOSE", 3, 5,10,White,DarkGreen);
    tft_print("THIS IS Loading Dose MODE", 2,110,136,DarkGreen,LightGreen);
    break;

    //Mode ml/h
    case 3 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    RunButton(); //tampilan run button
    delay(100);
    tft_print("ML/H", 3, 5,10,White,DarkGreen);
    
    //menampilkan menu dari ml.h Mode beserta satuannya
    tftDrawRoundRect("VTBI", 3,118,66,164,109,5,DarkGreen,LightGreen);
    tft_print("ml", 2,251,153,DarkGreen,LightGreen);
    tftDrawRoundRect("RATE", 3,298,66,164,109,5,DarkGreen,LightGreen);
    tft_print("ml/h", 2,408,153,DarkGreen,LightGreen);
    tftDrawRoundRect("TIME",3,118,190,164,109,5,DarkGreen,LightGreen);
    tft_print("hour", 2,226,277,DarkGreen,LightGreen);    
    
    //menampilkan nilai input dari setiap variable
    nilaiinput(volumeinj,128,124);
    nilaiinput(nilairate,308,124);
    nilaiinput(waktu,128,248);
    break;

    //Mode Ramp Up/Down
    case 4 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    delay(100);
    tft_print("Ramp U/D", 3, 5,10,White,DarkGreen);
    tft_print("THIS IS Ramp Up/Down MODE", 2,110,136,DarkGreen,LightGreen);
    break;

    //Mode TIVA
    case 5 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    delay(100);
    tft_print("TIVA", 3, 5,10,White,DarkGreen);
    tft_print("THIS IS TIVA MODE", 2,110,136,DarkGreen,LightGreen);
    break;

    //Mode Weight
    case 6 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    RunButton();
    delay(100);
    tft_print("WEIGHT", 3, 5,10,White,DarkGreen);
    
    //menampilkan menu pada Weight Mode beserta dengan satuannya
    tftDrawRoundRect("VTBI", 3,118,66,164,109,5,DarkGreen,LightGreen);
    tft_print("ml", 2,251,153,DarkGreen,LightGreen);
    tftDrawRoundRect("RATE", 3,298,66,164,109,5,DarkGreen,LightGreen);
    tft_print("ml/h", 2,408,153,DarkGreen,LightGreen);
    tftDrawRoundRect("CONC.",3,118,190,164,109,5,DarkGreen,LightGreen);
    tft_print("mg/ml", 2,217,277,DarkGreen,LightGreen);
    tftDrawRoundRect("WEIGHT",3,298,190,164,109,5,DarkGreen,LightGreen);
    tft_print("kg", 2,427,277,DarkGreen,LightGreen);    
    
    //menampilkan nilai input dari setiap variable
    nilaiinput(volumeinj,128,124);
    nilaiinput(nilairate,308,124);
    nilaiinput(konsentrasi,128,248);
    nilaiinput(beratbadan,308,248);
    break;

    //Mode Sequence
    case 7 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    delay(100);
    tft_print("SEQUENCE", 3, 5,10,White,DarkGreen);
    tft_print("THIS IS Sequence MODE", 2,110,136,DarkGreen,LightGreen);
    break;

    //Input angka
    case 8 :
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    HomeButton();
    delay(100);
    tft_print("INPUT MODE", 3, 5,10,White,DarkGreen);
    delay(100);
    inputangka();
    break;

    //Menampilkan input detectbutton
    case 9 :
    tft.fillRect(142,61,301,53,LightGreen);
    tft.setCursor(152,71);
    tft.setTextColor(DarkGreen);
    tft.setTextSize(3);
    tft.print(angka);

    tft.setCursor(370,95);
    tft.setTextSize(2);
    
    //Bolus
    if(dis == 1){
      tft.print("hour");
    }
    //Time (ml/h)
    else if(dis == 13){
      tft.print("hour");
    }
    //VTBI (ml/h & weight)
    else if(dis == 11 | dis == 21){
      tft.print("ml"); 
    }
    //Rate (ml/h & weight)
    else if(dis == 12 | dis == 22){
      tft.print("ml/h");
    }
    //Acti Agentia (weight, conc.)
    else if(dis == 231){
      tft.print("mg");      
    }
    //Volume (weight, conc.)
    else if(dis ==232){
      tft.print("ml");      
    }
    //Conc.
    else if(dis == 233){
      tft.print("mg/ml");
    }
    //Weight
    else if(dis == 24){
      beratbadan = angka;
      tft.print("kg");
      Serial.print("Nilai Weight : ");
      Serial.println(beratbadan);
    }
    break;

    //Run Bolus
    case 10 :       
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    tft_print("RUN BOLUS", 3, 5,10,White,DarkGreen);    
    tft.fillRect(100,47,380,273,LightGreen);
    delay(100);
    
    //Menunjukkan time
    tft.fillCircle(225,184,85,DarkGreen);
    tft.drawCircle(225,184,100,DarkGreen);
    tft_print("TIME",4, 178,125,White,DarkGreen);
    tft.setCursor(209,171);
    tft.setTextColor(White);
    tft.print(bolust);
    tft_print("hour", 4,183,217,White, DarkGreen);

    //Menunjukkan Volume
    tft.fillCircle(396,116,50,DarkGreen);
    tft.drawCircle(396,116,60,DarkGreen);
    tft_print("VOLUME",2, 361,86,White,DarkGreen);
    tft.setCursor(387,111);
    tft.setTextColor(White);
    tft.print(volumeinj);
    tft_print("ml", 2,387,137,White, DarkGreen);
    
    //Menunjukkan Rate
    tft.fillCircle(396,251,50,DarkGreen);
    tft.drawCircle(396,251,60,DarkGreen);
    tft_print("RATE",2, 374,221,White,DarkGreen);
    tft.setCursor(387,246);
    tft.setTextColor(White);
    tft.print(nilairate);
    tft_print("ml/h", 2,374,272,White,DarkGreen);
    
    //tft_print("Bolus Process", 2,110,136,DarkGreen,LightGreen);
    break;

    //Run ml/h
    case 11 :       
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    tft_print("RUN ml/h", 3, 5,10,White,DarkGreen);    
    tft.fillRect(100,47,380,273,LightGreen);
    delay(100);
    
    //Menunjukkan time
    tft.fillCircle(225,184,85,DarkGreen);
    tft.drawCircle(225,184,100,DarkGreen);
    tft_print("TIME",4, 178,125,White,DarkGreen);
    tft.setCursor(204,171);
    tft.setTextColor(White);
    tft.print(waktu);
    tft_print("hour", 4,183,217,White, DarkGreen);

    //Menunjukkan Volume
    tft.fillCircle(396,116,50,DarkGreen);
    tft.drawCircle(396,116,60,DarkGreen);
    tft_print("VOLUME",2, 361,86,White,DarkGreen);
    tft.setCursor(387,111);
    tft.setTextColor(White);
    tft.print(volumeinj);
    tft_print("ml", 2,387,137,White, DarkGreen);
    
    //Menunjukkan Rate
    tft.fillCircle(396,251,50,DarkGreen);
    tft.drawCircle(396,251,60,DarkGreen);
    tft_print("RATE",2, 374,221,White,DarkGreen);
    tft.setCursor(387,246);
    tft.setTextColor(White);
    nilairate = 1;
    tft.print(nilairate);
    tft_print("ml/h", 2,374,272,White,DarkGreen);
    
    //tft_print("Injection Process", 2,110,136,DarkGreen,LightGreen);
    break;

    //Run Weight
    case 12 :       
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    tft_print("RUN Weight", 3, 5,10,White,DarkGreen);    
    tft.fillRect(100,47,380,273,LightGreen);
    delay(100);
    //Menunjukkan time
    tft.fillCircle(225,184,100,DarkGreen);
    tft.drawCircle(225,184,85,DarkGreen);
    tft_print("TIME",4, 194,125,White,DarkGreen);
    tft.setCursor(200,171);
    tft.setTextColor(White);
    tft.print(waktu);
    tft_print("hour", 4,183,217,White, DarkGreen);

    //Menunjukkan Volume
    tft.fillCircle(396,116,50,DarkGreen);
    tft.drawCircle(396,116,60,DarkGreen);
    tft_print("VOLUME",2, 361,86,White,DarkGreen);
    tft.setCursor(387,111);
    tft.setTextColor(White);
    tft.print(volumeinj);
    tft_print("ml", 2,387,137,White, DarkGreen);
    
    //Menunjukkan Rate
    tft.fillCircle(396,251,50,DarkGreen);
    tft.drawCircle(396,251,60,DarkGreen);
    tft_print("RATE",2, 374,221,White,DarkGreen);
    tft.setCursor(387,246);
    tft.setTextColor(White);
    tft.print(nilairate);
    tft_print("ml/h", 2,374,272,White,DarkGreen);
    
    //tft_print("Injection Process", 2,110,136,DarkGreen,LightGreen);
    break;

    //Menu Conc.
    case 13 :   
    tft.fillRect(0,0,240,46,DarkGreen);
    tft.fillRect(100,47,380,273,LightGreen);
    
    tft_print("CONC. MENU", 3, 5,10,White,DarkGreen);
    HomeButton();
    delay(100);
    //menampilkan menu dari Conc. beserta dengan satuannya
    tftDrawRoundRect("ACTI AGENTIA", 2,118,66,164,109,5,DarkGreen,LightGreen);
    tft_print("mg", 2,251,153,DarkGreen,LightGreen);
    tftDrawRoundRect("VOLUME", 3,298,66,164,109,5,DarkGreen,LightGreen);
    tft_print("ml", 2,428,153,DarkGreen,LightGreen);
    tftDrawRoundRect("CONC.",3,118,190,164,109,5,DarkGreen,LightGreen);
    tft_print("mg/ml", 2,217,277,DarkGreen,LightGreen);
    tftDrawRoundRect("CONC. UNIT",2,298,190,164,109,5,DarkGreen,LightGreen);
    //menampilkan satuan dari konsentrasi obat pada sistem
    tft_print("mg/kg/h", 3,308,248,Purple,LightGreen);    
    
    //menampilkan nilai input dari setiap variable
    nilaiinput(acti,128,124);
    nilaiinput(vob,308,124);
    nilaiinput(konsentrasi,128,248);
    break;

    //Error Syringe tidak terdeteksi
    case 14 :
    tft.fillRect(100,47,380,273,LightGreen);
    HomeButton();
    tft.fillRoundRect(116,84,348,201,10,Red);
    tft_print("ERROR", 4, 232,157,White,Red);
    tft_print("Tidak dapat menjalankan proses", 1,198,202,White,Red);
    tft_print("Syringe tidak terdeteksi", 1,232,228,White,Red);
    
    
    //Error Ukuran Syringe
    case 15 :
    tft.fillRect(100,47,380,273,LightGreen);
    HomeButton();
    tft.fillRoundRect(116,84,348,201,10,Red);
    tft_print("ERROR", 4, 232,157,White,Red);
    tft_print("Tidak dapat menjalankan proses", 1,198,202,White,Red);
    tft_print("VTBI lebih besar dari ukuran syringe", 1,185,228,White,Red);
    break;

    //Error Jarak
    case 16:
    tft.fillRect(100,47,380,273,LightGreen);
    HomeButton();
    tft.fillRoundRect(116,84,348,201,10,Red);
    tft_print("ERROR", 4, 232,157,White,Red);
    tft_print("STOP PROSES", 3,195,202,White,Red);
    tft_print("Jarak terlalu dekat", 1,232,245,White,Red);
  }
  
}

// --- Settingan Tampilan ---
void menu (){
  tft_print("SELECT MODE",3, 5,10,White,DarkGreen);
  
  // == opsi mode ==
  tftDrawRoundRect("Loading Dose", 2,120,70,160,70,10,DarkGreen,LightGreen);
  tftDrawRoundRect("ml/h", 2,300,70,160,70,10,DarkGreen,LightGreen);
  tftDrawRoundRect("Ramp Up/Down", 2,120,155,160,70,10,DarkGreen,LightGreen);
  tftDrawRoundRect("TIVA", 2,300,155,160,70,10,DarkGreen,LightGreen);
  tftDrawRoundRect("Body Weight", 2,120,240,160,70,10,DarkGreen,LightGreen);
  tftDrawRoundRect("Sequence", 2,300,240,160,70,10,DarkGreen,LightGreen);
}

void pembukaan(){
  tft.fillScreen(Orange);
  //=== set gambar/posisi(kiri, atas, panjang, tebal, ....., warna) =====
  tft.drawRoundRect(137,106,207,108,60,White);
  //=== set lingkaran (titik tengah dari : kiri,atas,diameter,warna) ===
  tft.drawCircle(190,146,21,White);
  tft.drawCircle(290,146,21,White);
  tft.fillCircle(190,146,18,Red);
  tft.fillCircle(290,146,18,Red);
  //draw FastVLine (vertikal), FastHLine(horizontal), Line(bukan H & V)
  //=== set line (kiri, atas, panjang, warna)/untuk Line (kiri,atas,sudut,warna) ===
  tft.drawFastHLine(190,185,100,White);
  delay(1000);

  tft.drawFastHLine(190,146,21,White);
  tft.drawFastHLine(290,146,21,White);
  tft.drawCircle(190,146,21,Orange);
  tft.drawCircle(290,146,21,Orange);
  tft.fillCircle(190,146,18,Orange);
  tft.fillCircle(290,146,18,Orange);
  delay (200);

  tft.drawFastHLine(190,146,21,Orange);
  tft.drawFastHLine(290,146,21,Orange);
  tft.drawCircle(190,146,21,White);
  tft.drawCircle(290,146,21,White);
  tft.fillCircle(190,146,18,Red);
  tft.fillCircle(290,146,18,Red);
  delay (500);
  
  tft.drawFastHLine(190,146,21,White);
  tft.drawFastHLine(290,146,21,White);
  tft.drawCircle(190,146,21,Orange);
  tft.drawCircle(290,146,21,Orange);
  tft.fillCircle(190,146,18,Orange);
  tft.fillCircle(290,146,18,Orange);
  delay (200);

  tft.drawFastHLine(190,146,21,Orange);
  tft.drawFastHLine(290,146,21,Orange);
  tft.drawCircle(190,146,21,White);
  tft.drawCircle(290,146,21,White);
  tft.fillCircle(190,146,18,Red);
  tft.fillCircle(290,146,18,Red);
  delay (1000);
}


void RunButton(){
  //Run Button
  tft.fillRoundRect(10,60,80,78,5,White);
  tft.drawFastHLine(24,93,8,Black);
  tft.fillRoundRect(31,91,2,5,1,Black);
  tft.fillRect(34,90,9,9,Blue);
  tft.drawRoundRect(33,89,30,11,2,Black);
  tft.drawFastVLine(36,179,6,Black);
  tft.drawFastVLine(38,179,3,Black);
  tft.drawFastVLine(40,179,3,Black);
  tft.fillRect(41,90,3,9,Black);
  tft.fillRect(62,86,2,16,Black);
  tft.fillRect(71,88,2,13,Black);
  tft.drawRect(41,90,31,9,Black); 
  tft.setCursor(36,115); //tulisan
  tft.setTextSize(2);
  tft.setTextColor(DarkGreen);
  tft.print("RUN");
}

void HomeButton(){
  //Home Button
  tft.fillRoundRect(10,60,80,78,5,White);
  tft.drawRoundRect(38,89,25,16,5,DarkGreen);
  tft.fillTriangle(50,74,67,92,33,92,DarkGreen);
  tft.setCursor(27,115); //tulisan
  tft.setTextSize(2);
  tft.setTextColor(DarkGreen);
  tft.print("HOME");
}
void Menutetap(){
  tft.fillScreen(LightGreen);
  tft.fillRect(0,0,480,46,DarkGreen);
  tft.fillRect(0,46,100,275,DarkGreen);
  tftFillRoundRect("PULL", 2,391,12,81,26,5,DarkGreen,White);
  //Home Button
  HomeButton();
  //Bolus Button
  tft.fillRoundRect(10,148,80,78,5,White);
  tft.drawFastHLine(24,182,8,Black);
  tft.fillRoundRect(31,180,2,5,1,Black);
  tft.fillRect(34,178,25,9,Blue);
  tft.drawRoundRect(33,177,30,11,2,Black);
  tft.drawFastVLine(36,179,6,Black);
  tft.drawFastVLine(42,179,6,Black);
  tft.drawFastVLine(48,179,6,Black);
  tft.drawFastVLine(55,179,6,Black);
  tft.drawFastVLine(38,179,3,Black);
  tft.drawFastVLine(40,179,3,Black);
  tft.drawFastVLine(44,179,3,Black);
  tft.drawFastVLine(46,179,3,Black);
  tft.drawFastVLine(50,179,3,Black);
  tft.drawFastVLine(53,179,3,Black);
  tft.fillRect(59,178,3,9,Black);
  tft.fillRect(62,174,2,16,Black);
  tft.fillRect(78,176,2,13,Black);
  tft.drawRect(58,178,21,9,Black);  
  tft.setCursor(21,203); //tulisan
  tft.setTextSize(2);
  tft.setTextColor(DarkGreen);
  tft.print("BOLUS");

  //Back Button
  tft.fillRoundRect(10,234,80,78,5,White);
  tft.fillRoundRect(41,263,38,11,5,DarkGreen);
  
  tft.fillTriangle(21,269,47,257,47,281,DarkGreen);
  tft.setCursor(28,290); //tulisan
  tft.setTextSize(2);
  tft.setTextColor(DarkGreen);
  tft.print("BACK");
}

void inputangka(){
    tft.drawRect(135,60,315,55,DarkGreen);
    //1
    tft.fillRect(135,129,55,55, DarkGreen);
    tft_print("1",3,157,149,White, DarkGreen);
    //2
    tft.fillRect(201,129,55,55, DarkGreen);
    tft_print("2",3,222,149,White, DarkGreen);
    //3
    tft.fillRect(266,129,55,55, DarkGreen);
    tft_print("3",3,287,149,White, DarkGreen);

    //4
    tft.fillRect(135,191,55,55, DarkGreen);
    tft_print("4",3,157,211,White, DarkGreen);
    //5
    tft.fillRect(201,191,55,55, DarkGreen);
    tft_print("5",3,222,211,White, DarkGreen);
    //6
    tft.fillRect(266,191,55,55, DarkGreen);
    tft_print("6",3,287,211,White, DarkGreen);

    //7
    tft.fillRect(135,253,55,55, DarkGreen);
    tft_print("7",3,157,273,White, DarkGreen);
    //8
    tft.fillRect(201,253,55,55, DarkGreen);
    tft_print("8",3,222,273,White, DarkGreen);
    //9
    tft.fillRect(266,253,55,55, DarkGreen);
    tft_print("9",3,287,273,White, DarkGreen);

    //0
    tft.fillRect(332,253,118,55, DarkGreen);
    tft_print("0",3,384,273,White, DarkGreen);

    //Backspace
    tft.fillRect(332,129,118,55, DarkGreen);
    tft.drawFastHLine(350,155,88,White);
    tft.fillTriangle(344,156,363,145,363,167,White);
}

void displaynilaiinput(){
  Serial.print("dis Condition     : ");  Serial.println(dis);
  Serial.print("Display Condition : ");  Serial.println(currentdisplay);
  Serial.println(" ");
  Serial.println("NILAI :");
  Serial.print("- Bolus       : "); Serial.print(bolust)      ;Serial.println(" sec");
  Serial.print("- VTBI        : "); Serial.print(volumeinj)   ;Serial.println(" ml");
  Serial.print("- Rate        : "); Serial.print(nilairate)   ;Serial.println(" ml/h");
  Serial.print("- Time        : "); Serial.print(waktu)       ;Serial.println(" detik");
  Serial.print("- Konsentrasi : "); Serial.print(konsentrasi) ;Serial.println(" mg/ml");
  Serial.print("- Weight      : "); Serial.print(beratbadan)  ;Serial.println(" kg");
  Serial.print("- Volume Obat : "); Serial.print(vob)         ;Serial.println(" ml");
  Serial.print("- Acti Agentia: "); Serial.print(acti)        ;Serial.println(" mg");
  Serial.print("- Angka       : "); Serial.println(angka);
}

// --- Setting Sistem Touch Screen ---
void menutouch(){
  //Home
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    currentdisplay = 0;
    dis = 0;
  }  
  //Bolus
  if((X >= 500) && (X < 690) && (Y > 160) && (Y < 290)){
    currentdisplay = 1;
    dis = 1;
    nilairate = 1;
  }
    //Loading Dose
    if((Y > 350) && (Y < 625)){
      //Loading Dose
      if((X >= 310) && (X < 480)){
        currentdisplay = 2;
        dis = 2;
      }
      //Ramp Up/Down
      else if((X >= 525) && (X < 695)){
        currentdisplay = 4;
        dis = 2;        
      }
      //Body Weight
      else if((X >= 735) && (X < 900)){
        currentdisplay = 6;
        dis = 4;
        
      }
    }
    else if((Y > 665) && (Y < 930)){
      //ml/h
      if((X >= 310) && (X < 480)){
        currentdisplay = 3;
        dis = 3;          
      }
      //TIVA
      else if((X >= 525) && (X < 695)){
        currentdisplay = 5;
        dis = 2;        
      }
      //Sequence
      else if((X >= 735) && (X < 900)){
        currentdisplay = 7;
        dis = 2;        
      }
    }
  }

void errortouch(){
  //Home
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    currentdisplay = 0;
    dis = 0;
  }  
  //Bolus
  if((X >= 500) && (X < 690) && (Y > 160) && (Y < 290)){
    currentdisplay = 1;
    dis = 1;
  }
  //Back
  else if((X >= 718) && (X < 900) && (Y > 160) && (Y < 290)){
    //error bolus
    if(dis == 101){
      currentdisplay = 1;
      dis = 1;
    }
    //error ml/h
    else if(dis == 102){
      currentdisplay = 3;
      dis = 3;
    }
    //error weight
    else if(dis == 103){
      currentdisplay = 6;
      dis = 4;
    }

  }

}

void laintouch(){
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    //Home mode
    currentdisplay = 0;
    dis = 0;
  }
  //Back
  else if((X >= 718) && (X < 900) && (Y > 160) && (Y < 290)){
    currentdisplay = 0;
    dis = 0;
  }
  //Bolus
  else if((X >= 500) && (X < 690) && (Y > 160) && (Y < 290)){
    currentdisplay = 1;
    dis = 1;
  } 
}

void pulltouch(){
  if((Y >= 815) && (Y <= 950) && (X >= 175) && (X <= 242) ){
    tftFillRoundRect("PULL", 2,391,12,81,26,5,White,LightGreen);
    runpull();
    tftFillRoundRect("PULL", 2,391,12,81,26,5,DarkGreen,White);
  }
}

void bolustouch(){
  //Run
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    if(ukuransyringe == 0){
      Serial.println("ERROR RUN, syringe tidak terdeteksi");
      currentdisplay = 14; 
      dis = 101;
    }
    else if(ukuransyringe != 0){
      if(ukuransyringe < volumeinj){
        angka = 0;
        Serial.println("ERROR, VTBI terlalu besar");
        currentdisplay = 15;
        dis = 101; 
      }
      //apabila ukuran syringe lebih kecil VTBI
      else if(ukuransyringe >= volumeinj){
      angka = 0;
       dis = 01;
      runbolus(bolust);
      }      
    }
  }
  //Back
  else if((X >= 718) && (X < 900) && (Y > 160) && (Y < 290)){
    angka = 0;
    currentdisplay = 0;
    dis = 0;
  }
  
}

void mlhtouch(){
  //Run ml/h mode
  //apabila ukuran syringe lebih besar dari VTBI
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    if(ukuransyringe == 0){
      Serial.println("ERROR RUN, syringe tidak terdeteksi");
      currentdisplay = 14; 
      dis = 102;
    }
    else if(ukuransyringe != 0){
      if(ukuransyringe < volumeinj){
        Serial.println("ERROR, VTBI terlalu besar");
        currentdisplay = 15;
        dis = 102; 
      }
      //apabila ukuran syringe lebih kecil VTBI
      else if(ukuransyringe >= volumeinj){
       dis = 02;
       runinject(volumeinj,waktu);
      }      
    }    
  }
  else if((X >= 315 ) && (X <=570)){
    //VTBI
    if((Y >= 350) && (Y <=630)){
      currentdisplay = 8;
      dis = 11;  
    }
    //RATE
    else if((Y >=664 ) && (Y <=934)){
      currentdisplay = 8;
      dis = 12;       
    }
  }

  //TIME
  else if((X >=615 ) && (X <=870) && (Y >= 350) && (Y <=630)){
    currentdisplay = 8;
    dis = 13;
  } 
}

void backmenu (){
  //Back
  if((X >= 718) && (X < 900) && (Y > 160) && (Y < 290)){
    currentdisplay = 0;
    dis = 0;
  }
  //Bolus
  else if((X >= 500) && (X < 690) && (Y > 160) && (Y < 290)){
    currentdisplay = 1;
    dis = 1;
  } 
}


void weighttouch(){  
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    if(ukuransyringe == 0){
      Serial.println("ERROR RUN, syringe tidak terdeteksi");
      currentdisplay = 14; 
      dis = 103;
    }
    else if(ukuransyringe != 0){
      if(ukuransyringe < volumeinj){
        Serial.println("ERROR, VTBI terlalu besar");
        currentdisplay = 15;
        dis = 103; 
      }
      //apabila ukuran syringe lebih kecil VTBI
      else if(ukuransyringe >= volumeinj){
       dis = 03;
       runinject(volumeinj,waktu);
      }      
    }
  }
  
  else if((X >= 315 ) && (X <= 570)){
    //VTBI
    if((Y >= 350) && (Y <= 630)){
      Serial.println("VTBI Selected");
      currentdisplay = 8;
      dis = 21;
    }
    //RATE
    else if((Y >=664 ) && (Y <= 934)){
      Serial.println("Rate Selected");
      currentdisplay = 8;
      dis = 22;     
    }
  }
  else if((X >=615 ) && (X <=870)){
    //CONC.
    if((Y >= 350) && (Y <=630)){
      Serial.println("Conc. Selected");
      currentdisplay = 13;
      dis = 23;
    }
    //WEIGHT
    else if((Y >=664 ) && (Y <=934)){
      Serial.println("Weight Selected");
      currentdisplay = 8;
      dis = 24;
    } 
  }
}

void conctouch(){
  if((X >= 315 ) && (X <= 570)){
    //Acti Agentia
    if((Y >= 350) && (Y <= 630)){
      Serial.println("Acti Agentia Selected");
      currentdisplay = 8;
      dis = 231;
      
    }
    //Volume
    else if((Y >=664 ) && (Y <= 934)){
      Serial.println("Volume Obat Selected");
      currentdisplay = 8;
      dis = 232;
    }    
  }
  //Conc.
  else if((X >=615 ) && (X <=870) && (Y >= 350) && (Y <=630)){
    Serial.println("Conc. Selected");
    currentdisplay = 8;
    dis = 233; 
  }
}

void DetectButtons(){
  if((Y > 377) && (Y < 475)){ //tombol 1 | 4 | 7
    //1
    if((X > 460) && (X < 593)){
      currentdisplay = 9;
      Serial.println ("tombol 1"); 
      if(angka == 0){
        angka = 1;
      }
      else{
        angka = (angka*10) + 1;
      }     
    }
    //4
    else if((X > 617) && (X < 748)){
      currentdisplay = 9;
      Serial.println ("tombol 4"); 
      if(angka == 0){
        angka = 4;
      }
      else{
        angka = (angka*10) + 4;
      }
    }
    //7
    else if((X > 771) && (X < 898)){
      currentdisplay = 9;
      Serial.println ("tombol 7"); 
      if(angka == 0){
        angka = 7;
      }
      else{
        angka = (angka*10) + 7;
      }      
    }
  }
  else if((Y > 495)&&(Y < 590)){ //tombol 2 | 5 | 8
    //2
    if((X > 460) && (X < 593)){
      currentdisplay = 9;
      Serial.println ("tombol 2"); 
      if(angka == 0){
        angka = 2;
      }
      else{
        angka = (angka*10) + 2;
      }
    }
    //5
    else if((X > 617) && (X < 748)){
      currentdisplay = 9;
      Serial.println ("tombol 5"); 
      if(angka == 0){
        angka = 5;
      }
      else{
        angka = (angka*10) + 5;
      }
    }
    //8
    else if((X > 771) && (X < 898)){
      currentdisplay = 9;
      Serial.println ("tombol 8"); 
      if(angka == 0){
        angka = 8;
      }
      else{
        angka = (angka*10) + 8;
      }
    }
  }
  else if((Y > 610 ) && (Y < 703 )){ //tombol 3 | 6 | 9
    //3
    if((X > 460) && (X < 593)){
      currentdisplay = 9;
      Serial.println ("tombol 3"); 
      if(angka == 0){
        angka = 3;
      }
      else{
        angka = (angka*10) + 3;
      }
    }
    //6
    else if((X > 617) && (X < 748)){
      currentdisplay = 9;
      Serial.println ("tombol 6"); 
      if(angka == 0){
        angka = 6;
      }
      else{
        angka = (angka*10) + 6;
      }
    }
    //9
    else if((X > 789) && (X < 899)){
      currentdisplay = 9;
      Serial.println ("tombol 9"); 
      if(angka == 0){
        angka = 9;
      }
      else{
        angka = (angka*10) + 9;
      }
    }
  }
  else if((Y > 725) && (Y < 920)){ //Backspace | run | 0
    //Clear
    if((X > 460) && (X < 593)){
      currentdisplay = 9;
      Serial.println("Backspace");
      angka  = 0;
    }
    /*
    //run
    else if((X > 617) && (X < 748)){
      currentdisplay = 9;
      Serial.println("Run");
      currentdisplay = 0;
      dis = 0;
    }
    */
    //0
    else if((X > 771) && (X < 898)){
      currentdisplay = 9;
      Serial.println ("tombol 0"); 
      if(angka == 0){
        angka = 0;
      }
      else{
        angka = (angka*10);
      }
    }
  }
  Serial.print("Nilai input : ");
  Serial.println(angka);

  kalkulasiauto();
}

void backconc(){
  //Home
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    angka = 0;
    currentdisplay = 0;
    dis = 0;
  }
  //Back
  else if((X >= 718) && (X < 900) && (Y > 160) && (Y < 290)){
    angka = 0;
    currentdisplay = 13;
    dis = 23;
  }
  //Bolus
  else if((X >= 500) && (X < 690) && (Y > 160) && (Y < 290)){
    angka = 0;
    currentdisplay = 1;
    dis = 1;
  } 
} 
void backmlh(){
  //Home
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    angka = 0;
    currentdisplay = 0;
    dis = 0;
  }
  //Back
  else if((X >= 718) && (X < 900) && (Y > 160) && (Y < 290)){
    angka = 0;
    currentdisplay = 3;
    dis = 3;
  }
  //Bolus
  else if((X >= 500) && (X < 690) && (Y > 160) && (Y < 290)){
    angka = 0;
    currentdisplay = 1;
    dis = 1;
  } 
} 

void backweight(){  //Home
  if((X >= 290) && (X <= 490) && (Y >= 160) && (Y <= 290)){
    angka = 0;
    currentdisplay = 0;
    dis = 0;
  }
  //Back
  else if((X >= 718) && (X < 900) && (Y > 160) && (Y < 290)){
    angka = 0;
    currentdisplay = 6;
    dis = 4;
  }
  //Bolus
  else if((X >= 500) && (X < 690) && (Y > 160) && (Y < 290)){
    angka = 0;
    currentdisplay = 1;
    dis = 1;
  } 
} 

void DetectSyringe (){
  /*
   * 5 ml   : 120 - 200
   * 10 ml  : 250 - 406
   * 20 ml  : 420 - 570
   * 30 ml  : 600 - 660
   * 40 ml  : 700 - 790
   * 50 ml  : 840 - 940
   */
  potmeterval = analogRead(potmeterPin); 
  //Serial.println(potmeterval);
  
  if (potmeterval < 120 | potmeterval > 940){
    //Serial.print("Syringe tidak terdeteksi");
    ukuransyringe = 0;
  }
  else if ((potmeterval >= 120) && (potmeterval < 190)){
    //Serial.print("Ukuran Syringe : 5 mL");
    ukuransyringe = 5;
  }
  else if ((potmeterval > 250) && (potmeterval < 406)){
    //Serial.print("Ukuran Syringe : 10 mL");
    ukuransyringe = 10;
  }
  else if ((potmeterval > 420) && (potmeterval <= 570)) { 
    //Serial.print("Ukuran Syringe : 20 mL");
    ukuransyringe = 20;
  }
  else if ((potmeterval > 600) && (potmeterval <= 660)) { 
    //Serial.print("Ukuran Syringe : 30 mL");
    ukuransyringe = 30;
  }
  else if ((potmeterval > 700) && (potmeterval <= 790)) { 
    //Serial.print("Ukuran Syringe : 40 mL");
    ukuransyringe = 40;
  }
  else if ((potmeterval > 840) && (potmeterval < 940)) { 
    //Serial.print("Ukuran Syringe : 50 mL");
    ukuransyringe = 50;
  }
  //Serial.println();
  delay(100);
}

void tft_flexsyringe(){
  tft.setTextSize(2);
  //Untuk menutup angka sebelumnya
  tft.setTextColor(DarkGreen,DarkGreen);
  tft.setCursor(250,25);
  tft.print("00");
  //Menampilkan ukuran syringe yang terdeteksi
  tft.setTextColor(White,DarkGreen);
  tft.setCursor(250,25);
  tft.print(ukuransyringe);
  tft_print("ml", 2, 280,25,White,DarkGreen);
  
  if(ukuransyringe != 0){
    tft_print("Syringe not Detected", 1,250,10,DarkGreen,DarkGreen);
  }
  else if (ukuransyringe == 0){
    tft_print("Syringe not Detected", 1,250,10,Red,DarkGreen);
  }
  Serial.println("\t \t \t \b TERTAMPILKAN");
}

// --- Sound Effect ---
void warning(){
  tone(piezoPin, si,500); 
  delay(500);
  tone(piezoPin, 0, 500);
  delay(200);
  
  tone(piezoPin, si,500); 
  delay(500);
  tone(piezoPin, 0, 500);
  delay(200);
  
  tone(piezoPin, si,500); 
  delay(500);
  tone(piezoPin, 0, 500);
  delay(200);  
}

void dangerous(){
  
}

// ----------------------- Format Penulisan ----------------------
void tft_print(String isi, int ukuran, int x, int y, int warna,int bg){
  tft.setCursor(x,y);
  tft.setTextColor(warna,bg);
  tft.setTextSize(ukuran);
  tft.print(isi);
}

//tampilan untuk hasil input angka yang diberikan
void nilaiinput(int isi,int x, int y){
  tft.setCursor(x,y);
  tft.setTextColor(Purple);
  tft.setTextSize(4);
  tft.print(isi);  
}

void tftDrawRoundRect(String isi,int ukuran, int x, int y,int lebar,int tinggi,int sudut, int warna,int bg ){
  tft.drawRoundRect(x,y,lebar,tinggi,sudut,warna);
  int a = x + 10;
  int b = y + 10;
  tft.setTextSize(ukuran);
  tft.setTextColor(warna, bg);
  tft.setCursor(a, b);
  tft.print(isi);  
}

//khusus tombol pull
void tftFillRoundRect(String isi,int ukuran, int x, int y,int lebar,int tinggi,int sudut, int warna,int bg ){
  tft.fillRoundRect(x,y,lebar,tinggi,sudut,bg);
  int a = x + 18;
  int b = y + 6;
  tft.setTextSize(ukuran);
  tft.setTextColor(warna, bg);
  tft.setCursor(a, b);
  tft.print(isi);  
}

// ------------------- Sistem Run ----------------
void kalkulasiauto(){
  //Bolus
  if(dis == 1){
    bolust = angka;
    volumeinj = bolust*1; //0.5 rate tetap khusus mode bolus
    Serial.print("Nilai Bolus : ");
    Serial.println(bolust); 
  }
  //VTBI (ml/h & weight)
  if(dis == 11 | dis == 21){
    volumeinj = angka;
    Serial.print("Nilai VTBI : ");
    Serial.println(volumeinj);
    if(dis == 11){
      if(waktu == 0 && nilairate != 0){
        waktu = volumeinj/nilairate;
      }
      else if(waktu !=0 && nilairate == 0){
        nilairate = volumeinj/waktu;
      }
      //kalau keadaan tidak 0, maka time tidak diubah
      //kesimpulan mode ml/h lebih mengutamakan waktu dari pada time
      else if (waktu !=0 && nilairate !=0){
        nilairate = volumeinj/waktu;
      }      
    }
    else if(dis == 22){
      if(konsentrasi != 0){
        nilairate = beratbadan/konsentrasi;
        delay(50);
        waktu = volumeinj/nilairate;
      }
    }

  }
  //Rate (ml/h & weight)
  else if(dis == 12 | dis == 22){
    nilairate = angka;
    Serial.print("Nilai Rate : ");
    Serial.println(nilairate);
    if(dis == 12){
      waktu = volumeinj/nilairate;
    }
    else if(dis == 22){
      beratbadan = 0;
      konsentrasi = 0;
      vob = 0;
      acti = 0;
      waktu = volumeinj/nilairate;
    }
  }
  //Time (ml/h)
  else if(dis == 13){
    waktu = angka;
    Serial.print("Nilai Time : ");
    Serial.println(waktu);
    nilairate = volumeinj/waktu;
  }
  
  // ==== CONC MENU ===
  else if(dis == 231 | dis == 232){
    if (dis == 231){
      acti = angka;
      Serial.print("Nilai Konsentrasi : ");
      Serial.println(acti);
    }
    else if(dis == 232){
      vob = angka;
      Serial.print("Volume Obat : ");
      Serial.println(vob);
    }
    konsentrasi = acti/vob;
    delay(50);
    nilairate = beratbadan/konsentrasi;
    delay(50);
    waktu = volumeinj/nilairate;
  }
  
  //Conc.
  else if(dis == 233){
    konsentrasi = angka;
    Serial.print("Nilai Konsentrasi : ");
    Serial.println(konsentrasi);
    vob = 0;
    acti = 0;
    nilairate = beratbadan/konsentrasi;
    delay(50);
    waktu = volumeinj/nilairate;
  }
  
  //Weight
  else if(dis == 24){
    beratbadan = angka;
    Serial.print("Nilai Weight : ");
    Serial.println(beratbadan);
    if(nilairate != 0){
      nilairate = beratbadan/konsentrasi;
      delay(50);
      waktu = nilairate/volumeinj;
    }
  }
}

void runpull (){
  digitalWrite(dir,LOW);
  digitalWrite(pul,HIGH);

  f = 10*1000; //ml to ul                  
  r = (f/(4*2*166.8)); //rotasi                       
  t = (10*1000/(rev*r))/2*1000; //delayMicrosecond
  Serial.println("pull proses");
  digitalWrite(dir,LOW);
  for(i=0; i<rev*r; i++){
    digitalWrite(pul, HIGH);
    delayMicroseconds(t);
    digitalWrite(pul, LOW);
    delayMicroseconds(t);
  }  
}

//sistem menjalankan bolus
void runbolus (unsigned long sec){
  //rumus blm untuk time integrasi ke volume
  /*Bolus
   * input time
   * rate 1
   * rumus volume = time*rate
   * contoh : input time 10sec dan rate 1ml/s
   * maka volume obat yang keluar dari syringe adalah 10 ml
   */
   /* bolus
    * ml = 1*bolust
    * f = ml * 1000; 
    * r =(f/(4*2*166.8));  
    * t = (bolust*1000/(rev*r))/2*1000;
    */
    
  ml = 1*bolust;
  f = ml*1000; //1 ml disuntikan
  r = (f/(4*2*166.8)); //rotasi
  t = (bolust*1000/(rev*r))/2*1000;
  
  currentdisplay = 10;
  c_display(10);
  Serial.print("proses bolus dengan waktu : ");
  Serial.println(bolust);

  digitalWrite(dir,HIGH);
  for(i=0; i<rev*r; i++){
    digitalWrite(pul,HIGH);
    delayMicroseconds(t);
    digitalWrite(pul, LOW);
    delayMicroseconds(t);
  }
  /*
  Serial.print("JARAK (ldr) : ");
  int ldr = ReadDistance();
  ldr = ReadDistance();
  Serial.println(ldr);

  digitalWrite(dir,HIGH);
  for(j = 0; j <= ml; j+= 0.5){
    int ldr = ReadDistance();
    ldr = ReadDistance();
    Serial.print("JARAK (ldr) : ");
    Serial.println(ldr);
    if(ldr > 10){      
      for(i = 0; i < s; i++){
         digitalWrite(pul,HIGH);
         delayMicroseconds(t);
         digitalWrite(pul,LOW);
         delayMicroseconds(t);
      }
    }
    
    else if(ldr <= 3){
      j = ml;
      Serial.print("-------- ERROR --------->>");
      Serial.println(ldr);
      currentdisplay = 16;
      c_display(currentdisplay);
      dis = 101;
      warning();
      break;
    }
  }
  */
  Serial.println("Proses Bolus Selesai");

  currentdisplay = 1;
  dis = 1;
  
  angka = 0;
}

void runinject(int ml, unsigned long sec){
  f = ml*1000; //1 ml disuntikan
  r = (f/(4*2*166.8)); //rotasi
  t = (sec*1000/(rev*r))/2*1000;

  s = rev*r/ml;
  
  if(dis == 02){
    currentdisplay = 11;
    c_display(currentdisplay);
  }
  else if(03){
    currentdisplay = 12;
    c_display(currentdisplay);
  }
  
  Serial.print("Mulai Proses Injeksi dengan waktu : ");
  Serial.println(sec);
  Serial.print("JARAK (ldr) : ");
  int ldr = ReadDistance();
  ldr = ReadDistance();
  Serial.println(ldr);
  
  digitalWrite(dir,HIGH);
  for(j = 0.5; j <= ml; j+= 0.5){
    int ldr = ReadDistance();
    ldr = ReadDistance();
    Serial.print("JARAK (ldr) : ");
    Serial.println(ldr);
    if(ldr > 3){      
      for(i = 0; i < rev/2; i++){
         digitalWrite(pul,HIGH);
         delayMicroseconds(t);
         digitalWrite(pul,LOW);
         delayMicroseconds(t);
      }
    }
    
    else if(ldr <= 10){
      j = ml;
      Serial.print("-------- ERROR --------->>");
      Serial.println(ldr);
      currentdisplay = 16;
      c_display(currentdisplay);
      
      if(dis = 02){
        dis = 102;
      }
      else if(03){
        dis = 103;
      }
    }
  }
  Serial.print("Injeksi Selesai");
  
  //ml/h back to menu
  if(dis == 02){
    currentdisplay = 3;
    c_display(3);
    dis = 3;
  }
  //weight back to menu
  else if(dis == 03){
    currentdisplay = 6;
    c_display(6);
    dis = 4;
  }
}

void runmlh(int ml, unsigned long sec){
    /*ml/h Mode
   * input VTBI (else time & rate)
   * rumus time = VTBI/rate
   * rumus rate = VTBI/time
   * contoh 1 : input volume 5 ml & input rate 5 ml/sec
   * sistem akan mengkalkulasi automatis time, maka time injeksi adalah 1 sec
   * contoh 2 : input volume 5 ml & input time 3 sec
   * sistem akan mengkalkulasi automatis rate, maka nilai rate adalah 1.667 ml/sec
   */
   /*ml/h Mode
    * [inp time] //kalkulasi rate
    * f = volumeinj*1000;
    * r =(f/(4*2*166.8));
    * t = (waktu*1000/(rev*r))/2*1000;
    * 
    * [inp rate] //kalkulasi time
    * sec = volumeinj/nilairate
    * f = volumeinj*1000;
    * r =(f/(4*2*166.8));
    * t = (sec*1000/(rev*r))/2*1000;
    */
}

void runweight(int ml, unsigned long sec){
    /*
   * Weight Mode
   * input VTBI (else [rate], [weight & conc.], [weight, volume & acti])
   * rumus conc. = acti / volume
   * rumus time = VTBI/Rate
   * rumus rate = weight/conc.
   * contoh 1 : input volume 5 ml & input rate 5 ml/sec
   * sistem akan mengkalkulasi automatis time, maka time injeksi adalah 1 sec
   * contoh 2 : input volume 5 ml, weight 40 kg & konsentrasi obat 5 mg/ml
   * sistem akan mengkalkulasi automatis time & rate, maka time injeksi adalah 0.625 sec dengan rate 8 ml/s
   * contoh 3 : input volume 5 ml, weight 40 kg, acti agentia 30 mg & volume obat 10 ml
   * sistem akan mengkalkulasi automatis time, rate dan konsentrasi, maka time injeksi adalah 0.375 sec dengan rate 13.33 ml/s dan konsentrasi obat 3 mg/ml
   */
   /*
    * Weight Mode
    * [inp rate] //kalkulasi time
    * sec = volumeinj/nilairate
    * f = volumeinj*1000;
    * r =(f/(4*2*166.8));
    * t = (sec*1000/(rev*r))/2*1000;
    * 
    * [inp weight & conc.] kalkulasi rate & time
    * nilairate = beratbadan/konsentrasi
    * sec = volumeinj/nilairate
    * f = volumeinj*1000;
    * r =(f/(4*2*166.8));
    * t = (sec*1000/(rev*r))/2*1000;
    * 
    * [inp weight, volumeobat & acti agentia] //kalkulasi rate & time
    * konsentrasi = acti/vob
    * nilairate = beratbadan/konsentrasi
    * sec = volumeinj/nilairate
    * f = volumeinj*1000;
    * r =(f/(4*2*166.8));
    * t = (sec*1000/(rev*r))/2*1000;
    */
}

// --- Tambahan saat run ---
void SensorRead(unsigned char addr,unsigned char* datbuf,unsigned char cnt) {
  unsigned short result=0;
  // step 1: instruct sensor to read echoes
  Wire.beginTransmission(82); // transmit to device #82 (0x52)
  // the address specified in the datasheet is 164 (0xa4)
  // but i2c adressing uses the high 7 bits so it's 82
  Wire.write(byte(addr));      // sets distance data address (addr)
  Wire.endTransmission();      // stop transmitting
  // step 2: wait for readings to happen
  delay(1);                   // datasheet suggests at least 30uS
  // step 3: request reading from sensor
  Wire.requestFrom(82, cnt);    // request cnt bytes from slave device #82 (0x52)
  // step 5: receive reading from sensor
  if (cnt <= Wire.available()) { // if two bytes were received
    *datbuf++ = Wire.read();  // receive high byte (overwrites previous reading)
    *datbuf++ = Wire.read(); // receive low byte as lower 8 bits
  }
}

int ReadDistance(){
    SensorRead(0x00,i2c_rx_buf,2);
    lenth_val=i2c_rx_buf[0];
    lenth_val=lenth_val<<8;
    lenth_val|=i2c_rx_buf[1];
    delay(300); 
    return lenth_val;
} 
