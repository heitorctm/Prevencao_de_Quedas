/*
  Código Elaborado por Heitor Teixeira e Pedro Henrique para o projeto:
  #########################################
  ######     PREVENÇÃO DE QUEDAS     ######
  #########################################
  Implantação do foward da RNA. Rede neural terá 4 entradas e um vetor de 12 posições na saída.
*/

// bluetooth apenas para conferencia sem fio dos valores!!!
// deletar header e mudar as impressões para o serial do arduino ide
//------------------------------------------------------------
//CONECTIVIDADE - Wifi - BT - Broker

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

//-----------------------------------

#include "weights1.h"
#include "weights2.h"
#include "weights3.h"
#include "BluetoothSerialBT.h"

BluetoothSerial SerialBT;
#include <HX711.h>

//----------- parte do modulo mp3-----------//
#include <JQ6500_Serial.h>// módulo mp3

JQ6500_Serial mp3(Serial1);// da pra utilizar outras seriais
//estou utilizando a GPIO1 e GPIO3, que sao as padroes do esp

void ligarAlarme() {
  mp3.setLoopMode(MP3_LOOP_ONE);
  mp3.playFileByIndexNumber(1);
}

void desligarAlarme() {
  mp3.pause();
  mp3.setLoopMode(MP3_LOOP_ONE_STOP);
}
//-----------------------------------------//

// DEFINIÇÕES DE PINOS
// A
#define pinDT_A  13
#define pinSCK_A  12
// B
#define pinDT_B  14
#define pinSCK_B  27
// C
#define pinDT_C  26
#define pinSCK_C  25
// D
#define pinDT_D  33
#define pinSCK_D  32

// instanciando objetos
HX711 scaleA;
HX711 scaleB;
HX711 scaleC;
HX711 scaleD;


// variáveis
float medidaA = 0;
float medidaB = 0;
float medidaC = 0;
float medidaD = 0;
float soma = 0;

int n_linha = 0;
bool tag = false;
String variavel_de_envio = "pause";
char aux;

float inputs[4];

float relu(float x) {
  if (x < 0) {
    return 0;
  }
  return x;
}

// função para digitar no terminal
// removendo o caracter que representa o 'enter' tanto de pc como de mobile
void digitar_no_serial() {
  if (SerialBT.available()) {
    variavel_de_envio = "";
  }
  while (SerialBT.available()) {
    aux = SerialBT.read();
    if (aux != '\n' & aux != '^M') { // '^M' '/n' caracteres que representam o 'ENTER'.
      variavel_de_envio += aux;
    }
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("VORTEX");
  SerialBT.begin(115200);

  scaleA.begin(pinDT_A, pinSCK_A);
  delay(150);
  scaleA.set_scale(20481);
  delay(150);
  scaleA.tare();
  delay(300);
  
  scaleB.begin(pinDT_B, pinSCK_B); 
  delay(150);
  scaleB.set_scale(21406);
  delay(300);
  scaleB.tare();
  delay(150);
  
  scaleC.begin(pinDT_C, pinSCK_C);
  delay(300);
  //scaleC.set_scale(21035); 
  scaleC.set_scale(-21035); 
  delay(150);
  scaleC.tare();
 
  scaleD.begin(pinDT_D, pinSCK_D); 
  delay(150);
  scaleD.set_scale(25787);
  delay(150);
  scaleD.tare();

   //moudulo mp3
  Serial1.begin(9600,SERIAL_8N1, 16, 17);//rx, tx
  mp3.reset();//remove qualquer configuração anterior
  mp3.setVolume(15);//setar volume
  mp3.setLoopMode(MP3_LOOP_ONE_STOP);//toca uma vez e para
  
}

void loop() {

  // a variavel de start no código começa com 'pause',
  // ou seja, não está coletando dados até que qualquer outra coisa seja escrita no terminal.
  // caso digite 'reboot' vai zerar a variável que armazena o número de linhas, essa variável servia apenas para coleta de dados.
  // digitando 'zero' vai tarar a balança.
  // tanto reboot como zero, ao fim do seu ciclo, a variavel de start vai voltar a ser 'pause'
  // você vai precisará digitar qualquer outra coisa para voltar a coletar dados.
  
  digitar_no_serial();
  if (variavel_de_envio == "reboot") {
    variavel_de_envio = "pause";
    n_linha = 0;
    SerialBT.println("CONTAGEM REINICIADA");
  }
  
  if (variavel_de_envio == "zero") {
    
    // zerando as balanças(tara)
    scaleA.tare();
    delay(100);
    scaleB.tare();
    delay(100);
    scaleC.tare();
    delay(100);
    scaleD.tare();
    delay(100);
    SerialBT.println("BALANÇAS ZERADAS");
    variavel_de_envio = "pause";
  }
  
  if (variavel_de_envio != "pause") {
  inputs[0] = scaleA.get_units(2);
  inputs[1] = scaleB.get_units(2);
  inputs[2] = scaleC.get_units(2);
  inputs[3] = scaleD.get_units(2);
  soma = inputs[0] + inputs[1] + inputs[2] + inputs[3];

  // FOWARD
  float hidden1[1024];
  for (int i = 0; i < 1024; i++) {
    hidden1[i] = 0;
    for (int j = 0; j < 4; j++) {
      hidden1[i] += weights1[i][j] * inputs[j];
    }
    hidden1[i] += bias1[i];
    hidden1[i] = relu(hidden1[i]);
  }

  float hidden2[200];
  for (int i = 0; i < 200; i++) {
    hidden2[i] = 0;
    for (int j = 0; j < 1024; j++) {
      hidden2[i] += weights2[i][j] * hidden1[j];
    }
    hidden2[i] += bias2[i];
    hidden2[i] = relu(hidden2[i]);
  }

  float outputs[12];
  for (int i = 0; i < 12; i++) {
    outputs[i] = 0;
    for (int j = 0; j < 200; j++) {
      outputs[i] += weights3[i][j] * hidden2[j];
    }
    outputs[i] += bias3[i];
  }

  int indexMaior = 0;
  for (int i = 1; i < 12; i++) {
    if (outputs[i] > outputs[indexMaior]) {
      indexMaior = i;
    }
  }
  
  // classificação
  SerialBT.print(indexMaior);
  if(indexMaior == 0) { 
    SerialBT.print("/ fora da cama /");
        
  } 

  if(indexMaior == 1) {
    SerialBT.print("/ lado bc0 /");
    
  } 

  if(indexMaior == 2) {
    SerialBT.print("/ sentado em d /");
    
    
  } 

  if(indexMaior == 3) {
    SerialBT.print("/ lado ad0 /");

  } 

  if(indexMaior == 4) {
    SerialBT.print("/ lado bc1 /");

  } 

  if(indexMaior == 5) {
    SerialBT.print("/ deitado no meio /");

  } 

  if(indexMaior == 6) {
    SerialBT.print("/ diagonal bd /");
  } 

  if(indexMaior == 7) {
    SerialBT.print("/ lado ad1 /");

  } 

  if(indexMaior == 8) {
    SerialBT.print("/ sentado no meio /");

  } 

  if(indexMaior == 9) {
    SerialBT.print("/ sentado em c /");

  } 

  if(indexMaior == 10) {
    SerialBT.print("/ diagonal ac /");

  } 

  if(indexMaior == 11) {
    SerialBT.print("/ detectando queda / "); //sai
  
  } 

  // essas informações são apenas para eu checar no terminal se há alguma falha de medição.
  SerialBT.print(inputs[0]);
  SerialBT.print(" / ");
  SerialBT.print(inputs[1]);
  SerialBT.print(" / ");
  SerialBT.print(inputs[2]);
  SerialBT.print(" / ");
  SerialBT.print(inputs[3]);
  SerialBT.print(" / ");
  SerialBT.println(soma);
  
  } else {
    // coleta em stand by
  }

  
  delay(150);
}
