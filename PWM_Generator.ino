//LİBRARY

#include <PWM.h> //
#include <LiquidCrystal_I2C_AvrI2C.h>
 
LiquidCrystal_I2C_AvrI2C lcd(0x27,20,4);
 

// 20X4 LCD TANIMLAMASI
// Bu kodu kullanırken ekranda yazı çıkmaz ise 0x27 yerine 0x3f yazınız !!  internetten bak bağlantıya A4=SCA A5=SCL FİLAN OLUYOR GENELDE

//////////////////////////////////

int Menu = 1; //Menü değişkeni
int Last_Menu = 1;

//////////////////////////////////

int Rotation; //Rotary enkoder değişkenleri
int Value;
int CLK = 11;
int DT = 8;

/////////////////////

unsigned long Old_Time= 0;
unsigned long New_Time;
unsigned long Switch_Time = 0;

//////////////////////
const int PWM_Pin_1 = 10; // pwm için çıkış pini
const int PWM_Pin_2 = 9; // pwm için çıkış pini

//////////////////////////////////

#define Rotary_Switch 6 //Menü seçme butonu
#define Laser_Control_Switch 5 //ateş  buton
#define Laser_Test_Mod_Control_Switch 4 //test  buton

//////////////////////////////////

int Menu_Choice = 0; //Enkoder ile seçili olan satır
byte Menu_Choice_Character[8] = { //Yuvarlak karakterin binary kodu
  0b00000,
  0b01110,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00000
};

//////////////////////////////////

boolean Value_Chance= 1; //Değer mi değişiliyor menü mü değişiliyor tutan değişken
boolean Last_Value_Chance= 0;
int Laser_Test_Mod_Control = 0;//1sn atış 1 sn durma için gerekli fonksiyonlar
int32_t Laser_Power; //Ortak güç değişkeni
int32_t Frequency; //Ortak Frequencydeğişkeni
int32_t Frequency_2; //Ortak Frequencydeğişkeni
int32_t PWM_Duty; //Ortak PWM_Duty değişkeni
int32_t PWM_Duty_2; //Ortak PWM_Duty değişkeni
int32_t PWM_Duty_255; //Ortak PWM_Duty değişkeni
int32_t PWM_Duty_Maks;
int32_t PWM_Duty_Pwm_Negetif;




void setup() {

 // Serial.begin(9600);
 //BAŞLANGIÇ EKRAN OLAYLARI

 
  lcd.begin();
  lcd.setCursor(4, 0);
  lcd.print("HUSEYIN SENTURK");
  delay (3000);
   
  // GEREKLİ DEĞERLERİ ATIYORUM İLK ATAMA
  
  Laser_Power= 0; //Güç, Frequency, PWM_Duty değişkenlerini 0'lıyorum
  Frequency= 10000;
  Frequency_2 = 10;
  PWM_Duty = 0;
  PWM_Duty_Pwm_Negetif=0;
  
  lcd.createChar(0, Menu_Choice_Character); //Yuvarlak karakteri oluşturuyorum ve numarası 0 diyorum
  
  //PİN AYARLARI
  pinMode (CLK, INPUT); //Enkoder ve buton bağlantılarını giriş olarak ayarlıyorum
  pinMode (DT, INPUT);
  pinMode(Rotary_Switch, INPUT_PULLUP);
  pinMode(Laser_Control_Switch, INPUT_PULLUP);
  pinMode(Laser_Test_Mod_Control_Switch, INPUT_PULLUP);
    
  //PWM SİNYALİ İÇİN GEREKLİ AYARLAMALAR
  InitTimersSafe(); //initialize all timers except for 0, to save time keeping functions
  pinMode(PWM_Pin_1, OUTPUT);
  pinMode(PWM_Pin_2, OUTPUT);
  
  Rotation = digitalRead(CLK); //Enkoderin başlangıçtaki değerini ölçüyorum
  Menu_ekrani_guncelle();
 //Serial.println("START");
}
//////////////////////////////////
void loop() {
  
  Encoder_Control(); //Enkoderi kontrol ediyorum
  Switch_Control(); //Butonları kontrol ediyorum
  PWM_Control();

/*  if (Frequency== Frequency_2)
  {
    Frequency= Frequency_2;
    Frequency_2 = Frequency_2 / 1000;
  }
  // Serial.println(Frequency);
  //Serial.println(Frequency_2);
   if (Frequency!= Frequency_2 )
    { Serial.print("Frequency:lop ");
     Serial.println(Frequency);
     Frequency_2 = Frequency;
    }
    if (PWM_Duty_255 != PWM_Duty_2)
    {
     PWM_Duty_2 = PWM_Duty_255;
     Serial.print("PWM_Duty:lop ");
     Serial.println(PWM_Duty);
     Serial.print("PWM_Duty:negatif ");
     Serial.println(PWM_Duty_Pwm_Negetif);

    }*/
}
//////////////////////////////////






void Switch_Control()
{
  if (!digitalRead(Rotary_Switch)) //Seçme butonuna basıldıysa...

  {
    unsigned long Switch_Time = millis(); //Butona ne zaman basıldığını kayıt ediyorum

    while (!digitalRead(Rotary_Switch)); //Butonu bırakana kadar programı bekletiyorum
    
    {      
      if (millis() >= Switch_Time + 1000)
     
      {
        Menu = Menu / 10 ;
        Frequency= 10000;
        Frequency_2 = 10;
        Laser_Power= 0;
        PWM_Duty = 0;
        Switch_Time = millis();
      }

      else if (Menu >= 1000) //Menü 1000'den büyükse yani güç, Frequencyveya PWM_Duty ayarlama ekranına geldiyse

      {
        Menu = Menu / 10;
        //Değer değiş değişkenim 1 olduğu zaman enkoder ile güç, Frequencyveya PWM_Duty değerlerini değiştiriyorum
        //Değer değiş değişkenim 0 olduğu zaman enkoder ile menüde yukarı aşağı yapıyorum
        delay(100);
      }

      else

      {
        String string_Menu = String(Menu); //Burada string olarak bir toplama yapıyorum
        string_Menu += Menu_Choice; //Menü değişkenimin değeri 11'se ve seçenek değişkenimin değeri 3 se bunları yan yana getirerek 113 değerini oluşturuyorum
        Menu = string_Menu.toInt(); //Bunu da int olan menü değişkenimn içine atıyorum
        Menu_ekrani_guncelle(); //Menü ekranını güncelliyorum
      }
    }

    while (!digitalRead(Rotary_Switch));
    
    delay(100);
  }
}


//////////////////////////////////



void Encoder_Control()
{
  Value = digitalRead(CLK);
  if (Value != Rotation) 
  {
    if (digitalRead(DT) != Value)
    {  
      //Eğer enkoder + yönünde ilerlediyse...
      if (Value_Chance)//Değer değiş değişkenimin değeri 1 olmadığı zaman yani enkoder ile menüler arası hareket ettiğim zaman...
      {
        
        Menu_Choice ++; //Menu_Choice değişkenimi 1 arttırıyorum
        if (Menu == 1 || Menu == 11     )
        
        {
          //Bu menüler 3 satırlı, o yüzden max 3. satıra kadar inebilirim
          //Satırlara 0 da dahil olduğu için 0, 1, 2. satıra kadar inmesine izin veriyorum
          Menu_Choice = min(Menu_Choice, 2);
        }
        else if ( Menu == 100 || Menu == 101 || Menu == 102  || Menu == 103 || Menu == 110 || Menu == 111 || Menu == 112 || Menu == 120 || Menu == 121  )
        
        {
          //Bu menüler 4 satırlı, o yüzden max 3. satıra kadar inebilirim
          //Satırlara 0 da dahil olduğu için 0, 1, 2, 3. satıra kadar inmesine izin veriyorum
          Menu_Choice = min(Menu_Choice, 3);
        }
        
        else if (Menu == 12 || Menu == 10  )
        
        {
          //Bu menüler 4 satırlı, o yüzden max 3. satıra kadar inebilirim
          //Satırlara 0 da dahil olduğu için 0, 1, 2, 3. satıra kadar inmesine izin veriyorum
          Menu_Choice = min(Menu_Choice, 1);
        }


      }
      
      else //Değer değiş değişkenimin değeri 1 olduğu zaman yani enkoder ile güç, Frequency, PWM_Duty değişkenlerinin değerini değiştirdiğim zaman...
      
      {
        if (Menu == 1000) ////ANA MENU>ROFIN-LUXİNAR>SR-SCX-OEM>POWER
        
        {
          Laser_Power+= 1;
          Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
          PWM_Duty = map(Laser_Power, 0, 100, 0, 60);
          PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
          /* Serial.print("Laser_Power: ");
            Serial.println(Laser_Power);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.println("encoder_kontrol Laser_Power+");*/
        }


        else if (  Menu == 1010 )  ////ANA MENU>ROFIN-LUXİNAR>45-İX>POWER
        
        {
          Laser_Power+= 1;
          Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
          PWM_Duty = map(Laser_Power, 0, 100, 0, 55);
          PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
          /*  Serial.print("Laser_Power: ");
            Serial.println(Laser_Power);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.println("encoder_kontrol Laser_Power+");*/
        }

        else if (  Menu == 1100)  ////ANA MENU>COHERENT>E-SERİES>POWER
        
        {
          Laser_Power+= 1;
          Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
          PWM_Duty = map(Laser_Power, 0, 100, 0, 70);
          PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
          /* Serial.print("Laser_Power: ");
            Serial.println(Laser_Power);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.println("encoder_kontrol Laser_Power+");*/
        }
        
        else if (   Menu == 1110 )  ////ANA MENU>COHERENT>J2-J3>POWER
        
        {
          Laser_Power+= 1;
          Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
          PWM_Duty = map(Laser_Power, 0, 100, 0, 60);
          PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
          /*Serial.print("Laser_Power: ");
            Serial.println(Laser_Power);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.println("encoder_kontrol Laser_Power+");*/
        }

        else if (   Menu == 1120 )  ////ANA MENU>COHERENT>J5-J6>POWER
        
        {
          Laser_Power+= 1;
          Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
          PWM_Duty = map(Laser_Power, 0, 100, 0, 40);
          PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
          /*   Serial.print("Laser_Power: ");
             Serial.println(Laser_Power);
             Serial.print("PWM_Duty: ");
             Serial.println(PWM_Duty);
             Serial.print("PWM_Duty: ");
             Serial.println(PWM_Duty);
             Serial.println("encoder_kontrol Laser_Power+");*/
        }
        
        else if ( Menu == 1202)  ////ANA MENU>MANUEL LASER>POWER>POWER
        
        {
          Laser_Power+= 1;
          Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz

          PWM_Duty = max(0, PWM_Duty); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olama
          PWM_Duty = min(PWM_Duty_Maks, PWM_Duty);

          PWM_Duty = map(Laser_Power, 0, 100, 0, PWM_Duty_Maks);
          PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
          /*   Serial.print("PWM_Duty: ");
             Serial.println(PWM_Duty);
             Serial.print("Laser_Power: ");
             Serial.println(Laser_Power);
             Serial.print("PWM_DUTY_MAKS: ");
             Serial.println(PWM_Duty_Maks);*/
        }

        else if (Menu == 1001 || Menu == 1011 || Menu == 1021 || Menu == 1031 || Menu == 1101 || Menu == 1111 || Menu == 1121  || Menu == 1201  || Menu == 1210)
        
        {
          //Bu menüler Frequencyseçeneğine tıklandığı zaman gelinen menüler o yüzden burada enkoder arttığı zaman Frequencydeğerini 0.5 arttırıyorum
          Frequency+= 1000 ;
          Frequency= max(0, Frequency); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          /*  Serial.print("Frequency: ");
            Serial.println(Frequency);
            Serial.println("encoder_kontrol Frequency+");*/

          if (Frequency!= Frequency_2 )
          
          {
            // Serial.print("Frequency:lop ");
            //Serial.println(Frequency);
            Frequency_2 = Frequency;
          }
          if (Frequency== Frequency_2)
          
          {
            Frequency= Frequency_2;
            Frequency_2 = Frequency_2 / 1000;
          }
        }

        else if ( Menu == 1200) // MANUEL>POWER LASER>MAKS PWM_DUTY DEĞER AYARLAMA
        
        {
          PWM_Duty_Maks += 1;
          PWM_Duty_Maks = max(0, PWM_Duty_Maks); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          PWM_Duty_Maks = min(100, PWM_Duty_Maks); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
        }
        
        else if ( Menu == 1211) // MANUEL>POWER LASER>FREQUENCYDEĞER AYARLAMA
        
        {
          PWM_Duty += 1;
          PWM_Duty = max(0, PWM_Duty); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
          PWM_Duty = min(100, PWM_Duty); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
          PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
        }
      }
    }
    
    else //eğer enkoder - yönünde ilerlediyse...
    
    { 
      if (Value_Chance) // Değer değiş değişkenimin değeri 1 olmadığı zaman yani enkoder ile menüler arası hareket ettiğim zaman...
      
      {
        Menu_Choice--; //Seçenek değişkenimin değerini 1 azaltıyorum
        Menu_Choice = max(0, Menu_Choice); //Seçenek değişkenimin değeri min 0 olacak şekilde ayarlıyorum
      }
      
      if (Menu == 1000 ) //ANA MENU>ROFIN-LUXİNAR>SR-SCX-OEM>POWER
      
      {
        Laser_Power-= 1;
        Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
        PWM_Duty = map(Laser_Power, 0, 100, 0, 60);
        PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
        /*   Serial.print("Laser_Power: ");
           Serial.println(Laser_Power);
           Serial.print("PWM_Duty: ");
           Serial.println(PWM_Duty);
           Serial.print("PWM_Duty: ");
           Serial.println(PWM_Duty);
           Serial.println("encoder_kontrol Laser_Power+");*/
      }
      
      else if (  Menu == 1010 )  //ANA MENU>ROFIN-LUXİNAR>45-İX>POWER
      
      {
        Laser_Power-= 1;
        Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
        PWM_Duty = map(Laser_Power, 0, 100, 0, 55);
        PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
        /*    Serial.print("Laser_Power: ");
            Serial.println(Laser_Power);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.println("encoder_kontrol Laser_Power+");*/
      }
      
      else if (   Menu == 1100 )  ////ANA MENU>COHERENT>E-SERİES>POWER
      
      {
        Laser_Power-= 1;
        Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
        PWM_Duty = map(Laser_Power, 0, 100, 0, 70);
        PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
        /*    Serial.print("Laser_Power: ");
            Serial.println(Laser_Power);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.print("PWM_Duty: ");
            Serial.println(PWM_Duty);
            Serial.println("encoder_kontrol Laser_Power+");*/
      }
      
      else if (   Menu == 1110 )  ////ANA MENU>COHERENT>J2-J3>POWER
      
      {
        Laser_Power-= 1;
        Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
        PWM_Duty = map(Laser_Power, 0, 100, 0, 60);
        PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
        /*   Serial.print("Laser_Power: ");
           Serial.println(Laser_Power);
           Serial.print("PWM_Duty: ");
           Serial.println(PWM_Duty);
           Serial.print("PWM_Duty: ");
           Serial.println(PWM_Duty);
           Serial.println("encoder_kontrol Laser_Power+");*/
      }

      else if (   Menu == 1120 )  ////ANA MENU>COHERENT>J5-J6>POWER
      
      {
        Laser_Power-= 1;
        Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
        PWM_Duty = map(Laser_Power, 0, 100, 0, 40);
        PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
        /*     Serial.print("Laser_Power: ");
             Serial.println(Laser_Power);
             Serial.print("PWM_Duty: ");
             Serial.println(PWM_Duty);
             Serial.print("PWM_Duty: ");
             Serial.println(PWM_Duty);
             Serial.println("encoder_kontrol Laser_Power+");*/
      }

      else if ( Menu == 1202)  ////ANA MENU>MANUEL LASER>POWER>POWER
      
      {
        Laser_Power-= 1;
        Laser_Power= max(0, Laser_Power); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        Laser_Power= min(100, Laser_Power); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz

        PWM_Duty = max(0, PWM_Duty); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olama
        PWM_Duty = min(PWM_Duty_Maks, PWM_Duty);

        PWM_Duty = map(Laser_Power, 0, 100, 0, PWM_Duty_Maks);
        PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);
        /*     Serial.print("PWM_Duty: ");
             Serial.println(PWM_Duty);
             Serial.print("Laser_Power: ");
             Serial.println(Laser_Power);*/

      }

      else if ( Menu == 1200)// MANUEL>POWER LASER>MAKS PWM_DUTY DEĞER AYARLAMA
      
      {
        PWM_Duty_Maks -= 1;
        PWM_Duty_Maks = max(0, PWM_Duty_Maks); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        PWM_Duty_Maks = min(100, PWM_Duty_Maks); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz

      }

      else if ( Menu == 1211)// MANUEL>FREQUENCY>PWM_DUTY DEĞER AYARLAMA
      
      {
        PWM_Duty -= 1;
        PWM_Duty = max(0, PWM_Duty); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz
        PWM_Duty = min(100, PWM_Duty); //Küçük olan sayı güç değerine atanır yani güç değişkeni 100'den büyük olamaz
        PWM_Duty_255 = map(PWM_Duty, 0, 100, 0, 255);

      }
      
      else if (Menu == 1001 || Menu == 1011 || Menu == 1021 || Menu == 1031 || Menu == 1101 || Menu == 1111 || Menu == 1121 || Menu == 1210 || Menu == 1201)
      
      {
        //Bu menüler Frequencyseçeneğine tıklandığı zaman gelinen menüler o yüzden burada enkoder arttığı zaman Frequencydeğerini 0.5 arttırıyorum
        Frequency-= 1000 ;
        Frequency= max(0, Frequency); //Büyük olan sayı güç değerine atanır yani güç değişkeni 0'dan az olamaz

        /*     Serial.print("Frequency: ");
             Serial.println(Frequency);
             Serial.println("encoder_kontrol Frequency+");*/
        if (Frequency!= Frequency_2 )
        
        { // Serial.print("Frequency:lop ");
          //Serial.println(Frequency);
          Frequency_2 = Frequency;
        }

        if (Frequency== Frequency_2)
        
        {
          Frequency= Frequency_2;
          Frequency_2 = Frequency_2 / 1000;
        }
      }
    }

    Rotation = Value;
    Menu_ekrani_guncelle();
  }
}


///////////////////////////


void PWM_Control()

{ 
  New_Time = millis();
  SetPinFrequencySafe(PWM_Pin_1, Frequency);
  SetPinFrequencySafe(PWM_Pin_2, Frequency);
  PWM_Duty_Pwm_Negetif=255-PWM_Duty_255;
  
  if (  !digitalRead(Laser_Control_Switch) == 1 || Menu == 1002 || Menu == 1012 || Menu == 1102 || Menu == 1112 || Menu == 1122 || Menu == 1203 || Menu == 1212)//LASER OF/ONN
  
  {

    pwmWrite(PWM_Pin_1, PWM_Duty_255);// PWM ATIŞ BAŞLADI
    pwmWrite(PWM_Pin_2, PWM_Duty_Pwm_Negetif);
   // pwmWrite(LED_BUILTIN, PWM_Duty_Pwm_Negetif);
  }

  else if ( !digitalRead(Laser_Test_Mod_Control_Switch) == 1  || Menu == 1003 || Menu == 1013 || Menu == 1103 || Menu == 1113 || Menu == 1123 || Menu == 1213) //LASER TEST MOD OFF/ON
  
  {
    if (New_Time - Old_Time> 1000) //PWM ATIŞ İÇİN
    
    {
      if (Laser_Test_Mod_Control == 1)
      
      { pwmWrite(PWM_Pin_1, PWM_Duty_255);
        pwmWrite(PWM_Pin_2, PWM_Duty_Pwm_Negetif);
      //  pwmWrite(LED_BUILTIN, PWM_Duty_Pwm_Negetif);
        Laser_Test_Mod_Control = 0;
      }
      else
      {
        pwmWrite(PWM_Pin_1, LOW);
        pwmWrite(PWM_Pin_2, LOW);
  //    pwmWrite(LED_BUILTIN, LOW);
        Laser_Test_Mod_Control = 1;
      }
      /* Eski zaman değeri yeni zaman değeri ile güncelleniyor */
      Old_Time= New_Time;
    }
    /*pwmWrite(PWM_Pin_1, PWM_Duty_255);
      delay(1000);
      pwmWrite(PWM_Pin_1, LOW);
      delay(1000);*/
  }
 
  else
  
  {
    pwmWrite(PWM_Pin_1, LOW);
    pwmWrite(PWM_Pin_2, LOW);
  }
}

//////////////////////////////////

void Menu_ekrani_guncelle()
{
  lcd.clear();

  if (Menu >= 1000 )
  {
    Value_Chance= 0;
  }
  else
  {
    Value_Chance= 1;
  }
  if (Menu != Last_Menu)
  {
    //  Serial.print("Menu: ");
    //  Serial.println(Menu);
    Last_Menu = Menu;
  }
  if (Value_Chance!= Last_Value_Chance)
  {
    //  Serial.print("Value_Chance: ");
    //  Serial.println(Value_Chance);
    Last_Value_Chance= Value_Chance;
  }
  switch (Menu) //MENULERİN İÇLERİNİN NASIL OLDUĞU YAZIYOR
  {
    case 1://Ana Menü
      {
        lcd.print("ROFIN-LUXINAR LASER");
        lcd.setCursor(0, 1);
        lcd.print("COHERENT LASER");
        lcd.setCursor(0, 2);
        lcd.print("MANUEL LASER");
        break;
      }
    case 10://Ana Menü>ROFİN-LUXINAR LASER
      {
        lcd.print("SR-SCX-OEM SERIES");
        lcd.setCursor(0, 1);
        lcd.print("45 IX SERIES");
        lcd.setCursor(0, 2);
        break;
      }
    case 11://Ana Menü>Coherent Lazer
      {
        lcd.print("E-SERIES");
        lcd.setCursor(0, 1);
        lcd.print("J2-J3 SERIES");
        lcd.setCursor(0, 2);
        lcd.print("J5-J6 SERIES");
        break;
      }
    case 12://Ana Menü>MANUEL
      {
        lcd.print("LASER POWER");
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES");
        lcd.setCursor(0, 2);
        break;
      }

    case 120://Ana Menü>LASER POWER

      {


        lcd.print("MAKS PWM_DUTY (%) ");
        lcd.print(PWM_Duty_Maks);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 3);
        lcd.print("LASER OFF");
        break;
      }

    case 1200://Ana Menü>Güç Hesabı

      {


        lcd.print("MAKS PWM_DUTY (%) ");
        lcd.print(PWM_Duty_Maks);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 3);
        lcd.print("LASER OFF");
        break;
      }
    case 1201://Ana Menü>Güç Hesabı

      {

        lcd.print("MAKS PWM_DUTY (%) ");
        lcd.print(PWM_Duty_Maks);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 3);
        lcd.print("LASER OFF");
        break;
      }
    case 1202://Ana Menü>Güç Hesabı

      {

        lcd.print("MAKS PWM_DUTY (%) ");
        lcd.print(PWM_Duty_Maks);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 3);
        lcd.print("LASER OFF");
        break;
      }
    case 1203: //Ana Menü>Güç Hesabı

      {


        lcd.print("MAKS PWM_DUTY (%) ");
        lcd.print(PWM_Duty_Maks);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 3);
        lcd.print("LASER ON");
        break;
      }
    case 121://Ana Menü>FrequencyPWM_Duty

      {


        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 0);
        lcd.print("kHz ");
        lcd.setCursor(0, 1);
        lcd.print("LASER PWM_DUTY (%) ");
        lcd.print(PWM_Duty);
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }

    case 1210://Ana Menü>FrequencyPWM_Duty

      {


        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 0);
        lcd.print("kHz ");
        lcd.setCursor(0, 1);
        lcd.print("LASER PWM_DUTY (%) ");
        lcd.print(PWM_Duty);
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }

    case 1211://Ana Menü>FrequencyPWM_Duty

      {


        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 0);
        lcd.print("kHz ");
        lcd.setCursor(0, 1);
        lcd.print("LASER PWM_DUTY (%) ");
        lcd.print(PWM_Duty);
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1212://Ana Menü>FrequencyPWM_Duty

      {


        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 0);
        lcd.print("kHz ");
        lcd.setCursor(0, 1);
        lcd.print("LASER PWM_DUTY (%) ");
        lcd.print(PWM_Duty);
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }

    case 1213://Ana Menü>FrequencyPWM_Duty

      {


        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 0);
        lcd.print("kHz ");
        lcd.setCursor(0, 1);
        lcd.print("LASER PWM_DUTY (%) ");
        lcd.print(PWM_Duty);
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }
    case 100://Ana Menü>Rafine Lazer>SCX 35
      {

        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }



    case 101://Ana Menü>Rafine Lazer>SCX 65
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }


    case 102://Ana Menü>Rafine Lazer>OEM 45
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }






    case 103://Ana Menü>Rafine Lazer>OEM 65
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 110://Ana Menü>Coherent Lazer>J3
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;

      }
    case 111 ://Ana Menü>Coherent Lazer>J3
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;

      }
    case 112://Ana Menü>Coherent Lazer>>J3
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;

      }
    case 1000:   // ana menü SCX35 GÜÇ DEĞERİ
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1001: //SCX35 FREQUENCYDEĞERİ
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1002: //SCX35 PWM_DUTY DEĞERİ
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1003://Ana Menü>Rafine Lazer>SCX 35
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }
    case 1010:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1011:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1012:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }


    case 1013://Ana Menü>Rafine Lazer>SCX 65
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }
    case 1020:
      {

        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1021:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1022:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }

    case 1023:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }
    case 1030:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1031:
      {

        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1032:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }

    case 1033:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }
    case 1100:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1101:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1102:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }

    case 1103:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }
    case 1110:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1111:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1112:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1113:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }

    case 1120:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1121:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1122:
      {


        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER ON");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD OFF");
        break;
      }
    case 1123:
      {



        lcd.print("LASER POWER (%) ");
        lcd.print(Laser_Power);
        lcd.setCursor(0, 1);
        lcd.print("FREQUENCIES ");
        lcd.print(Frequency_2);
        lcd.setCursor(16, 1);
        lcd.print("kHz ");
        lcd.setCursor(0, 2);
        lcd.print("LASER OFF");
        lcd.setCursor(0, 3);
        lcd.print("LASER TEST MOD ON");
        break;
      }

    default:
      lcd.clear();
      lcd.print("Islem Bitti");
      delay(1000);
      Menu = 1; //Menüyü 1 yapıyorum
      Laser_Power= 0; //Güç, Frequency, PWM_Duty değişkenlerini 0'lıyorum
      Frequency= 10000;
      PWM_Duty = 0;
      Menu_ekrani_guncelle(); //Menü ekranını güncellediğim fonksiyonu çağırıyorum

      break;
  }
  lcd.setCursor(19, Menu_Choice);
  lcd.write((byte)0);
}
