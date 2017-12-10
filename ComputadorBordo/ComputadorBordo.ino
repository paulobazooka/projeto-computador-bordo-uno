// @autor Paulo Sérgio do Nascimento
// @inicio - Julho de 2017
// @Mini computador de Bordo

#include <LiquidCrystal_I2C.h> // Bliblioteca para LCD I2C
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <max6675.h>
#include "numeros_grandes.h"


#define DS1307_ADDRESS 0x68 // define o endereço do DS1307 como hex68

MAX6675 termopar(12,11,10);  // (CLK, CS, SO) instancia um objeto MAX6675 com os pinos de comunicação 10 11 12

SoftwareSerial bluetoothSerial(8, 9); // RX , TX

// *************** Declaração das variáveis globais *********************
byte zero = 0x00;  
byte menu = 0;
volatile byte estado = HIGH;
volatile bool alarme_on = false; // habilita os alarmes somente quando um dos alarmes for setado pelo celular
String dado; // string para enviar e receber dados seriais (via bluetooth tmb)

// Variáveis de Tensão
float leitura_tensao =  0;
float tensao_adjust  = 1;
float tensao_entrada = 21.213;
float alarme_tensao  =  0;
bool  alarme_bateria = false;

// Variáveis de Temperatura
float temperatura  = 0;
float temperatura2 = 0;
float temp = 0; 

// Variáveis de Temperatura do Motor
float temp_motor  = 0;
float alarme_temp = 500;
bool  alarme_motor = false;
byte  tempo_temp_motor = 0;

// Variáveis de contagem de tempo
volatile byte state = LOW;
byte TIMER_SEGUNDOSUNDOS = 0;
byte SEGUNDOS = 0;
byte TIMER_LEITURA_TEMPERATURA = 0;
byte TIMER_LEITURA_TENSAO = 0;
byte TIMER_ENVIAR_DADOS_BLUETOOTH = 0;
bool dez_SEGUNDOSundos = false;
bool backlight = true;
bool change = true;
bool carro_ligado = true;

//  Variveis globais de RPM
int rpm = 0;
int rpm_pulso = 1;
int rpm_tmp = 1000;  //tempo em mS
volatile byte revolution  = 0;
int rpm_milhar  = 0;
int rpm_centena = 0;
int rpm_dezena  = 0;
int rpm_unidade = 0;
unsigned long timeold = 0;

// Variáveis de data e hora
int SEGUNDOSundos = 0;
int minutos = 0;
int hora = 0; 
int diadasemana = 0; 
int diadomes = 0;
int mes = 0;
int ano = 0;

  
byte E[8] = {
  B00111,
  B01111,
  B01111,
  B01111,
  B01111,
  B01111,
  B01111,
  B00111
};
byte D[8] = {
  B11100,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11100
};

byte S[8] = {
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte I[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};  

byte C[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B01111,
  B00111
};

byte C2[8] = {
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};


byte C3[8] = {
  B00011,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B00111
};

byte C4[8] = {
  B11000,
  B11100,
  B00000,
  B00000,
  B00000,
  B00000,
  B11000,
  B11100
};

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);


// ********** Escopo das funções **********
void changeScreen();
void imprimeNumeroGrande(int number, byte pos);
void adjustBluetooth();
void selecionaDataeHora(byte SEGUNDOS, byte mi, byte horas, byte dia_semana, byte dia, byte mes, byte ano);
byte ConverteParaBCD(byte val);
byte ConverteparaDecimal(byte val);
void mostraRelogio();
void mostraRelogioGrande();  
void mostraRPM();
void desativaInterrupcoes();
void ativaInterrupcoes();
void serialEvent();
void verificaIgnicao();
// ********* Fim escopo das funções *******




void setup()
{ 
  digitalWrite(4, HIGH);
  digitalWrite(7, HIGH);
  // Configuração do timer1 
  TCCR1A = 0;                        //confira timer para operação normal pinos OC1A e OC1B desconectados
  TCCR1B = 0;                        //limpa registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1
 
  TCNT1 = 0xC2F7;                    // incia timer com valor para que estouro ocorra em 1 SEGUNDOSundo
                                     // 65536-(16MHz/1024/1Hz) = 49911 = 0xC2F7
  
  TIMSK1 |= (1 << TOIE1);           // habilita a interrupção do TIMER1
  
  Serial.begin(9600);
  bluetoothSerial.begin(9600);
  
  Wire.begin();  
  lcd.begin (16,2);
  lcd.setBacklight(HIGH);
  
  lcd.createChar(0, E);
  lcd.createChar(1, D);
  lcd.createChar(2, S);
  lcd.createChar(3, I);
  lcd.createChar(4, C);
  lcd.createChar(5, C2);
  lcd.createChar(6, C3);
  lcd.createChar(7, C4);
  
  selecionaDataeHora(0, 0, 0, 0, 1, 1, 1);

  lcd.clear();
  
  // Configuração dos pinos 
  pinMode(2, INPUT_PULLUP);  // botão menu 
  pinMode(3, INPUT_PULLUP);  // RPM
  pinMode(4, OUTPUT);        // acionamento do auto Reset
  pinMode(6, INPUT);         // ignição
  pinMode(7, OUTPUT);        // liga/desliga bluetoot

  // Configuração das interrupções
  attachInterrupt(digitalPinToInterrupt(2),changeScreen,LOW);
  attachInterrupt(digitalPinToInterrupt(3),RPM,LOW);

  lcd.print("Carregando...");
  leSensoresTemperatura();
  leTensaoBateria();
  temp_motor = termopar.readCelsius();    
  delay(1500);
  lcd.clear();
}




void loop(){

 
   
   if(change==true) {
    lcd.clear();
    change = false;
   }

    if(menu==0) {
     
      if(TIMER_LEITURA_TEMPERATURA >=3){
      leSensoresTemperatura();   
      TIMER_LEITURA_TEMPERATURA = 0;  
      }
      carregaDataHora();     
      mostraRelogio();     
    }
    else
      if(menu==1) {       
        mostraTensaoTemperatura();        
      }
      else
        if(menu==2) {   
          carregaDataHora();    
          mostraRelogioGrande();        
        }
        else
          if(menu==3){
          
            if(TIMER_LEITURA_TEMPERATURA >=3){
               leSensoresTemperatura();   
               TIMER_LEITURA_TEMPERATURA = 0;  
            }         
            
            mostraTemperatura();         
          }
             else
                if(menu==4){
                  mostraRPM();
                }
                else
                  if(menu==5){
                    mostraValoresAlarmes();
                  }
                  else
                    if((menu==7) && (alarme_on == true)){
                      alarmeTemperaturaMotor();
                    }
                    else
                      if((menu==8) && (alarme_on == true)){
                        alarmeTensaoBateria();
                      }
          
    // se houver dados no serial bluetooth chama ajuste
    if(bluetoothSerial.available()) adjustBluetooth();

    // Verificar se o carro está ou não ligado.  
    verificaIgnicao();

    // Leitura da tensão da bateria   
    if(TIMER_LEITURA_TENSAO >= 1){
      leTensaoBateria();
      TIMER_LEITURA_TENSAO = 0;
    }   

    
  
    if(tempo_temp_motor >= 5){
      temp_motor = termopar.readCelsius();    
    //  delay(1000);
      monitoraTemperaturaMotor();       
      tempo_temp_motor = 0;
    }

    if((estado == LOW) && ((alarme_motor == false) || (alarme_bateria == false)) && (carro_ligado)) {
    lcd.setBacklight(HIGH);
    }

    if((alarme_motor == true) || (alarme_bateria == true)){       
        lcd.setBacklight(estado);
    }

    
    if(TIMER_ENVIAR_DADOS_BLUETOOTH >= 5){
  /*    dado = "!"; 
      dado.concat(leitura_tensao);
      dado.concat("@");
      dado.concat(temp);
      dado.concat("#");
      dado.concat(temp_motor);
      dado.concat("$");
      dado.concat(rpm);
      dado.concat("%");
      
      bluetoothSerial.print(dado); */
      TIMER_ENVIAR_DADOS_BLUETOOTH = 0;
    }
   
    monitoraTensaoBateria();
    
}


// ************************* Funções de Interrupções ***************************** // 

ISR(TIMER1_OVF_vect) //Função de interrupção do TIMER1 
{
  TCNT1 = 0xC2F7; // Renicia TIMER com hexC2F7 = 11511
  
  TIMER_SEGUNDOSUNDOS++;    
  tempo_temp_motor++;
  TIMER_LEITURA_TEMPERATURA++;
  SEGUNDOS++;
  TIMER_LEITURA_TENSAO++;
  TIMER_ENVIAR_DADOS_BLUETOOTH++;

  estado = !estado;
  
  if(TIMER_ENVIAR_DADOS_BLUETOOTH >= 20) TIMER_ENVIAR_DADOS_BLUETOOTH = 0;
  if(TIMER_LEITURA_TENSAO > 20) TIMER_LEITURA_TENSAO = 0;
  if(TIMER_LEITURA_TEMPERATURA > 20) TIMER_LEITURA_TEMPERATURA = 0;
  if(SEGUNDOS > 10) SEGUNDOS  = 0;  
  if(TIMER_SEGUNDOSUNDOS > 10) TIMER_SEGUNDOSUNDOS  = 0;
  if(tempo_temp_motor > 20) tempo_temp_motor = 0;
}


   
void changeScreen(){  // Função que muda de tela de acordo com a interrupção do pino 2
 menu++;
 if(menu>5) menu=0;
 change = true;
 // delay(250);
}


void RPM(){ // Função que incrementa as revoluções
  revolution++;
  if(revolution > 9999) revolution=0;
}
// ************************** Fim das Interrupções ************************************




void desativaInterrupcoes(){
  detachInterrupt(0);
}

void ativaInterrupcoes(){
  attachInterrupt(digitalPinToInterrupt(2),changeScreen,FALLING);
  attachInterrupt(digitalPinToInterrupt(3),RPM,FALLING);
}


// Função que comunica-se com a placa bluetooth
void adjustBluetooth() {
  
  desativaInterrupcoes();
  
  if(bluetoothSerial.available() > 0){
     dado = bluetoothSerial.readString();
     if(dado.substring(0,5) == "ajust"){        
      
         lcd.clear();
         lcd.print("Ajuste Hora/Data");
        
         ano = stringToInt(dado.substring(5, 7));     
         mes = stringToInt(dado.substring(7, 9));
         diadomes = stringToInt(dado.substring(9, 11));     
         diadasemana = stringToInt(dado.substring(11, 12));
         diadasemana--;
         hora = stringToInt(dado.substring(12, 14));     
         minutos = stringToInt(dado.substring(14, 16));  
         SEGUNDOSundos = stringToInt(dado.substring(16, 18));     

         selecionaDataeHora(SEGUNDOSundos, minutos, hora, diadasemana, diadomes, mes, ano);
         
         delay(700);
         lcd.clear(); 
  }
  else
     if(dado.substring(0,3) == "rpm"){ 
       lcd.clear();
       lcd.setCursor(3,0);
       lcd.print("Ajuste RPM");   
       rpm_pulso = stringToInt(dado.substring(3));
       delay(700);
       lcd.clear();     
     }
     else
         if(dado.substring(0,6) == "tensao"){ 
           lcd.clear();
           lcd.setCursor(1,0);
           lcd.print("Ajuste TENSAO");          
           dado = dado.substring(6);
           tensao_adjust = dado.toFloat();       
           delay(700);
           lcd.clear();     
         }
         else 
           if(dado.substring(0,5) == "reset"){
            lcd.clear();
            lcd.print("Resetando...");
            delay(700); 
            lcd.clear();
            digitalWrite(4, LOW);          
           }
           else
             if(dado.substring(0,4) == "temp"){
                lcd.clear();
                lcd.print("Ajuste de Alarme");
                lcd.setCursor(0,1);
                lcd.print("temp do motor");
                dado = dado.substring(4);
                alarme_temp = dado.toFloat();
                alarme_on = true; // ativa o alarme
                delay(700); 
                lcd.clear();               
             }
             else
               if(dado.substring(0,5) == "alten"){
                  lcd.clear();
                  lcd.print("Ajuste de Alarme");
                  lcd.setCursor(0,1);
                  lcd.print("tensao bateria");                  
                  dado = dado.substring(5);
                  alarme_tensao = dado.toFloat();
                  alarme_on = true; // ativa o alarme
                  delay(700); 
                  lcd.clear();               
               }
    } 

     ativaInterrupcoes();
} // fim da função de comunicação com a placa bluetooth



// ****************** Função para ligar/desligar o backlight do display ************************   
void verificaIgnicao(){    
    // Se a ignição estiver desligada, flag dez_SEGUNDOSundos em zero e o carro estiver ligado...
    if((digitalRead(6) == LOW) && (!dez_SEGUNDOSundos) && (carro_ligado)){
      Serial.println("Ignição = OFF");
      SEGUNDOS = 0;
      dez_SEGUNDOSundos = true;
    }
    // Se a flag dez_SEGUNDOSundos estiver setada e o tempo decorrido passar
    if((dez_SEGUNDOSundos) && (SEGUNDOS > 9)){
      dez_SEGUNDOSundos = false;
       if((digitalRead(6) == LOW)){ // Se passado o tempo a ignição continua desligada
        Serial.println("Carro Desligado");
        lcd.setBacklight(LOW);
        carro_ligado = false;
        digitalWrite(7,LOW);
       }
    }

    // Se a ignição estiver ligada, liga-se o backlight
    if((digitalRead(6) == HIGH) && (!carro_ligado)){
      Serial.println("Ignição = ON");
      Serial.println("Carro Ligado");
        lcd.setBacklight(HIGH);
        carro_ligado = true;
        digitalWrite(7,HIGH);
        saudacao();
    }
}
// ****************** Fim da função para ligar/desligar o backlight do display ************************

void carregaDataHora(){
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);
  SEGUNDOSundos = ConverteparaDecimal(Wire.read());
  minutos = ConverteparaDecimal(Wire.read());
  hora = ConverteparaDecimal(Wire.read() & 0b111111); 
  diadasemana = ConverteparaDecimal(Wire.read()); 
  diadomes = ConverteparaDecimal(Wire.read());
  mes = ConverteparaDecimal(Wire.read());
  ano = ConverteparaDecimal(Wire.read());
}


void saudacao(){
  desativaInterrupcoes();

  lcd.clear();
  
  if((hora >= 18) && (hora <= 23)){
    lcd.setCursor(3,0);
    lcd.print("Boa Noite!");    
  }else
   if((hora >= 0) && (hora < 5)){
    lcd.setCursor(3,0);
    lcd.print("Boa Noite!");    
  }else
  if((hora >= 5) && (hora < 12 )){
    lcd.setCursor(4,0);
    lcd.print("Bom Dia!");    
  }else
  if((hora >= 12) && (hora < 18)){
    lcd.setCursor(3,0);
    lcd.print("Boa Tarde!");
  }
    delay(1700);
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.print("Use sempre cinto");
    lcd.setCursor(2,1);
    lcd.print("de SEGUNDOSuranca!");
    delay(2300);
    lcd.clear();
    
    ativaInterrupcoes();
}

// ************************** Funções do Relógio ******************************
void selecionaDataeHora(byte SEGUNDOS, byte mi, byte horas, byte dia_semana, byte dia, byte mes, byte ano){   //Seta a data e a hora do DS1307
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //Stop no CI para que o mesmo possa receber os dados

  //As linhas abaixo escrevem no CI os valores de 
  //data e hora que foram colocados nas variaveis acima
  Wire.write(ConverteParaBCD(SEGUNDOS));
  Wire.write(ConverteParaBCD(mi));
  Wire.write(ConverteParaBCD(horas));
  Wire.write(ConverteParaBCD(dia_semana));
  Wire.write(ConverteParaBCD(dia));
  Wire.write(ConverteParaBCD(mes));
  Wire.write(ConverteParaBCD(ano));
  Wire.write(zero); //Start no CI
  Wire.endTransmission(); 
}

// função auxiliar
byte ConverteParaBCD(byte val){ //Converte o número de decimal para BCD
  return ( (val/10*16) + (val%10) );
}

// função auxiliar
byte ConverteparaDecimal(byte val)  { //Converte de BCD para decimal
  return ( (val/16*10) + (val%16) );
}


// Exibe data/hora padrão
void mostraRelogio()
{
  desativaInterrupcoes();
  
  lcd.setCursor(0,0);
  switch(diadasemana){
      case 0: lcd.print("Domingo");
      break;
      case 1: lcd.print("SEGUNDOSunda");
      break;
      case 2: lcd.print("Terca  ");
      break;
      case 3: lcd.print("Quarta ");
      break;
      case 4: lcd.print("Quinta ");
      break;
      case 5: lcd.print("Sexta  ");
      break;
      case 6: lcd.print("Sabado ");
    }
  
  lcd.setCursor(8,0);
   
  if(diadomes<10) lcd.print("0");
     lcd.print(diadomes);
     lcd.print("/");
  if(mes<10) lcd.print("0");
     lcd.print(mes);
     lcd.print("/");
  if(ano<10) lcd.print("0");   
     lcd.print(ano);

    int temperatura = temp*10;
    int dezena;
    int unidade;
    int decimal;

    dezena = temperatura/100;
    unidade = temperatura%100;

    decimal = unidade%10;
    unidade = unidade/10;
     
    lcd.setCursor(0,1);
    lcd.print(dezena);
    lcd.print(unidade);
    lcd.print(".");
    lcd.print(decimal);
    
    lcd.setCursor(4,1); 
    lcd.print((char)223);
    lcd.setCursor(5,1); 
    lcd.print("C");
    
    lcd.setCursor(8,1);
  if(hora<10) lcd.print("0");
    lcd.print(hora);
    lcd.print(":");
  if(minutos<10) lcd.print("0");
    lcd.print(minutos);
    lcd.print(":");
  if(SEGUNDOSundos<10) lcd.print("0"); 
    lcd.print(SEGUNDOSundos);

    ativaInterrupcoes();
}


// Função para exibir o relógio com números grandes
void mostraRelogioGrande(){
  
  int dezena = hora/10;
  int unidade = hora%10;

  imprimeNumeroGrande(dezena, 0);
  imprimeNumeroGrande(unidade, 3);
  lcd.setCursor(6,0);
  lcd.print((char) 165);
  lcd.setCursor(6,1);
  lcd.print((char) 165);

  dezena = minutos/10;
  unidade = minutos%10;

  imprimeNumeroGrande(dezena, 7);
  imprimeNumeroGrande(unidade, 10);

  lcd.setCursor(13,1);
  lcd.print(":");
  if(SEGUNDOSundos<10) lcd.print("0"); 
  lcd.print(SEGUNDOSundos);
}

int potencia(int base, int expoente){

  if(expoente == 0)
    return 1;
  else   
    return base * potencia(base, expoente - 1);
}

int stringToInt(String minhaString) {
    int i, x;
    int tam = minhaString.length() - 1;
    int numero = 0;

  for(i = tam; i >= 0; i--) {
    x = minhaString.charAt(i) - 48;
    numero += x * pow(10, tam - i);
  }
  return numero;
}


// ******************************* Fim das Funções do Relógio ****************************************
void mostraTemperatura(){
  
  desativaInterrupcoes();
  
  int dezena;
  int unidade;
  int decimal;

  dezena = (int)(temp*10)/100;
  unidade = (int)(temp*10)%100;

  decimal = unidade%10;
  unidade = unidade/10;
    
  lcd.setCursor(7,1);
  lcd.print((char)B00101110);
   
  imprimeNumeroGrande(dezena,1);
  imprimeNumeroGrande(unidade,4);
  imprimeNumeroGrande(decimal,8);
  imprimeCaracterGraus(11);  

  ativaInterrupcoes();
}


// Função de RPM
void mostraRPM(){
  lcd.setCursor(13,1); lcd.print("RPM");
   
  imprimeNumeroGrande(rpm_milhar,0);
  imprimeNumeroGrande(rpm_centena,3);  
  imprimeNumeroGrande(rpm_dezena,6);
  imprimeNumeroGrande(rpm_unidade,9); 
 
  
  delay(rpm_tmp);
     desativaInterrupcoes(); // Desativa as interrupções para poder realizar os calculos de forma correta  
     rpm = (int)((revolution*(1000/rpm_tmp)*60)*(1/rpm_pulso)); 
 //    rpm = ((10*1000/rpm_pulso)/(millis() - timeold)*revolution);
     
 //    timeold = millis();
     
     separaAlgarismos(rpm, &rpm_milhar, &rpm_centena, &rpm_dezena, &rpm_unidade);
     revolution = 0;
     rpm = 0;    
       
     ativaInterrupcoes(); // ativa novamente as interrupções     

  
}



void mostraValoresAlarmes(){
  lcd.setCursor(0,0);
  lcd.print("Alarm > ");
  lcd.print(alarme_temp);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Alarm < ");
  lcd.print(alarme_tensao);
  lcd.print("V");
  
}



void imprimeNumeroGrande(int number, byte pos){
      
      switch (number) {

      case 0:
      lcd.setCursor(pos, 0);
      lcd.print((char) 0);      
      lcd.setCursor(pos, 1);
      lcd.print((char) 0);      
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1); 
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);          
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 2);      
      lcd.setCursor(pos + 1, 1);
      lcd.print((char) 3);
      break;

      case 1:
      lcd.setCursor(pos,0);
      lcd.print(" ");
      lcd.setCursor(pos,1);
      lcd.print(" ");
      lcd.setCursor(pos+1,0);
      lcd.print(" ");
      lcd.setCursor(pos+1,1);
      lcd.print(" ");
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1);
      lcd.setCursor(pos + 2, 1);      
      lcd.print((char) 1);
      break;

      case 2:
      lcd.setCursor(pos, 0);
      lcd.print((char) 6);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 5);
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1);    
      lcd.setCursor(pos, 1);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 1);
      lcd.print((char) 3);
      lcd.setCursor(pos+2, 1);
      lcd.print((char) 3);    
      break;

      case 3:
      lcd.setCursor(pos, 0);
      lcd.print((char) 6);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 5);
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1); 
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);      
      lcd.setCursor(pos + 1, 1);
      lcd.print((char) 3);
      lcd.setCursor(pos, 1);
      lcd.print((char) 4);    
      break;
      
      case 4:
      lcd.setCursor(pos,1);
      lcd.print(" ");
      lcd.setCursor(pos+1,1);
      lcd.print(" ");
      lcd.setCursor(pos, 0);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 3);
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1);     
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);
      break;

      case 5:
      lcd.setCursor(pos, 0);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 5);        
      lcd.setCursor(pos + 1, 1);
      lcd.print((char) 3);
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);
      lcd.setCursor(pos+2, 0);
      lcd.print((char) 7);
      lcd.setCursor(pos, 1);
      lcd.print((char)4);
      break;

      case 6:           
      lcd.setCursor(pos, 0);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 5);
      lcd.setCursor(pos+2, 0);
      lcd.print((char) 7);
      lcd.setCursor(pos, 1);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 1);
      lcd.print((char) 3);
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);
      break;
      
      case 7:
      lcd.setCursor(pos,1);
      lcd.print(" ");
      lcd.setCursor(pos+1,1);
      lcd.print(" ");
      lcd.setCursor(pos, 0);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 2);
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1);    
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);    
      break;
      
      case 8:
      lcd.setCursor(pos, 0);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 5);
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1);    
      lcd.setCursor(pos, 1);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 1);
      lcd.print((char) 3);
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);
      break;

      case 9:
      lcd.setCursor(pos,1);
      lcd.print(" ");
      lcd.setCursor(pos+1,1);
      lcd.print(" ");
      lcd.setCursor(pos, 0);
      lcd.print((char) 0);
      lcd.setCursor(pos + 1, 0);
      lcd.print((char) 5);
      lcd.setCursor(pos + 2, 0);
      lcd.print((char) 1);     
      lcd.setCursor(pos + 2, 1);
      lcd.print((char) 1);
      break;

      default: 
      break;        
     }
}

// Função para imprimir o simbolo de graus Celsius com letra grande
void imprimeCaracterGraus(byte posicao){
  
  lcd.setCursor(posicao,0);
  lcd.print((char)223);
  lcd.setCursor(posicao+1, 0);
  lcd.print((char) 0);
  lcd.setCursor(posicao+1, 1);
  lcd.print((char) 0);        
  lcd.setCursor(posicao+2, 0);
  lcd.print((char) 2);
  lcd.setCursor(posicao+3, 0);
  lcd.print((char) 2);
  lcd.setCursor(posicao+2, 1);
  lcd.print((char) 3);
  lcd.setCursor(posicao+3, 1);
  lcd.print((char) 3);           
}


// Função para separar algarismos
void separaAlgarismos(int numero, int *milhar, int *centena, int *dezena, int *unidade){
     
     (*milhar)  = numero/1000;
     (*centena) = (int)numero%1000;
     (*dezena)  = (int)(*centena)%100;
     (*unidade) = (int)(*dezena)%10;
     
     (*centena) = (int)(*centena)/100;
     (*dezena)  = (int)(*dezena)/10;
}


// Leitura dos sensores de temperatura
void leSensoresTemperatura(){
    
    desativaInterrupcoes();
 
    for(byte i=0; i<15; i++){
       temperatura = temperatura + (analogRead(A1) * 0.48875855); // 0.48875855 = ((5volts/1023resolução)*100)
       delay(5);
    }
    for(byte i=0; i<15; i++){
       temperatura2 = temperatura2 + (analogRead(A2) * 0.48875855); // 0.48875855 = ((5volts/1023resolução)*100)
       delay(5);
    }
   
    temperatura  = temperatura/15;
    temperatura2 = temperatura2/15;   
    temp = ((temperatura + temperatura2)/2);
        
    ativaInterrupcoes();
}





// função imprimir tensão da bateria e temperatura do motor
void mostraTensaoTemperatura(){
  
  lcd.setCursor(0,0);
  lcd.print("Bateria: ");
  lcd.print(leitura_tensao);
  lcd.print("V");  
  lcd.setCursor(0,1);
  lcd.print("Motor: ");
  lcd.print(temp_motor);
  lcd.print((char)223);
  lcd.print("C");
}


void leTensaoBateria(){  
  desativaInterrupcoes();  
  
  for(int i=0; i<15; i++) {
    leitura_tensao = (float)leitura_tensao + analogRead(A0);
    delay(5);
  }
   leitura_tensao = (float)(leitura_tensao/15);
   leitura_tensao = (float)(((leitura_tensao * tensao_entrada)/1024) * tensao_adjust);    
  
  ativaInterrupcoes();
}




void monitoraTensaoBateria(){

  desativaInterrupcoes();    
  if(leitura_tensao < alarme_tensao){
       if(menu!=8) menu=8;
       if(alarme_bateria == false) alarme_bateria = true;
   }
   else
     if(alarme_bateria == true) {
       alarme_bateria = false;
       menu = 1;
     }
   ativaInterrupcoes();
}


void monitoraTemperaturaMotor(){ 
 
  desativaInterrupcoes();  
  if(temp_motor > alarme_temp){
       if(menu!=7) menu=7;
       if(alarme_motor == false) alarme_motor = true; 
  }
  else
    if(alarme_motor == true) {
      alarme_motor = false;
      menu = 0;
    }
   ativaInterrupcoes();
}


void alarmeTemperaturaMotor(){
  
   lcd.setCursor(0,0);
   lcd.print("*** ATENCAO ***");
   lcd.setCursor(0,1);
   lcd.print("Motor: ");
   lcd.print(temp_motor);
   lcd.print((char)223);
   lcd.print("C          ");
}

void alarmeTensaoBateria(){
    
   lcd.setCursor(0,0);
   lcd.print("*** ATENCAO ***");
   lcd.setCursor(0,1);
   lcd.print("Tensao: ");
   lcd.print(leitura_tensao);
   lcd.print("V           ");
}



