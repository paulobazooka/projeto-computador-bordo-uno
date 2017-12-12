
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
