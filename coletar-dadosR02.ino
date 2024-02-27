/*
  Código Elaborado por Heitor Teixeira para o projeto:

  #########################################
  ######     PREVENÇÃO DE QUEDAS     ######
  #########################################

  Coleta de dados para alimentar rede neural
*/

// INCLUSÃO DE BIBLIOTECAS

#include <HX711.h>
// DEFINIÇÕES DE PINOS
// A----------------------------------------------------------------------------------------------
#define pinDT_A  13
#define pinSCK_A  12
// B----------------------------------------------------------------------------------------------
#define pinDT_B  14
#define pinSCK_B  27
// C----------------------------------------------------------------------------------------------
#define pinDT_C  26
#define pinSCK_C  25
// D----------------------------------------------------------------------------------------------
#define pinDT_D  33
#define pinSCK_D  32

// INSTANCIANDO OBJETOS
HX711 scaleA;
HX711 scaleB;
HX711 scaleC;
HX711 scaleD;
// DECLARAÇÃO DE VARIÁVEIS
float medidaA = 0;
float medidaB = 0;
float medidaC = 0;
float medidaD = 0;
float soma = 0;
int n_linha = 0;
bool tag = false;
String variavel_de_envio = "pause";
char aux;

// FUNÇOES BLUETOOTH
//---------------------------------------------------------------------------------------------------------------------------
void digitar_no_serial() {
  if (Serial.available()) {
    variavel_de_envio = "";
  }
  while (Serial.available()) {
    aux = Serial.read();
    if (aux != '\n' & aux != '^M') { // '^M' '/n' caracteres que representam o 'ENTER'.
      variavel_de_envio += aux;
    }
  }
}

void setup() {
  Serial.begin(115200);
  scaleA.begin(pinDT_A, pinSCK_A); // CONFIGURANDO OS PINOS DA BALANÇA
  delay(150);
  scaleA.set_scale(20481);
  delay(150);
  scaleA.tare(); // ZERANDO A BALANÇA PARA DESCONSIDERAR A MASSA DA ESTRUTURA
  delay(300);
  // CONFIG BALANÇA B----------------------------------------------------------------------------------------------
  scaleB.begin(pinDT_B, pinSCK_B); 
  delay(150);
  scaleB.set_scale(21406);
  delay(300);
  scaleB.tare();
  delay(150);
  // CONFIG BALANÇA C----------------------------------------------------------------------------------------------
  scaleC.begin(pinDT_C, pinSCK_C);
  delay(300);
  scaleC.set_scale(-21035); 
  delay(150);
  scaleC.tare();
  // CONFIG BALANÇA D----------------------------------------------------------------------------------------------
  scaleD.begin(pinDT_D, pinSCK_D); 
  delay(150);
  scaleD.set_scale(25787);
  delay(150);
  scaleD.tare(); 
}

//As medidas das balanças vão aparecer no monitor serialBT separadas por '/', tratar arquivo no python ou excel
//ao digitar pause, irá parar de mandar as medidas no monitor serialBT, qualquer outro comando irá voltar.
void loop() {
  digitar_no_serial();
  if (variavel_de_envio == "reboot") {
    variavel_de_envio = "pause";
    n_linha = 0;
    Serial.println("CONTAGEM REINICIADA");
  }
  if (variavel_de_envio == "zero") {
    
    // ZERANDO OS VALORES DA BALANÇA
    scaleA.tare();
    delay(100);
    scaleB.tare();
    delay(100);
    scaleC.tare();
    delay(100);
    scaleD.tare();
    delay(100);
    Serial.println("BALANÇAS ZERADAS");
    variavel_de_envio = "pause";
  }
  if (variavel_de_envio != "pause") {
    Serial.print(n_linha);
    medidaA = scaleA.get_units();
    Serial.print("/");
    Serial.print(medidaA, 3);
    Serial.print("/");
    medidaB = scaleB.get_units();
    Serial.print(medidaB, 3);
    Serial.print("/");
    medidaC = scaleC.get_units();
    Serial.print(medidaC, 3);
    Serial.print("/");
    medidaD = scaleD.get_units();
    Serial.print(medidaD, 3);
    Serial.print("/");
    Serial.print(medidaA + medidaB + medidaC + medidaD); //SOMA DAS 4 BALANÇAS
    Serial.print("/");
    Serial.println(variavel_de_envio);
    n_linha++;
  } else {
    // COLETA DE DADOS EM STAND BY
  }
  //scaleA.power_down(); // DESLIGANDO O SENSOR
  delay(100);
  //scaleA.power_up(); // LIGANDO O SENSOR
}
