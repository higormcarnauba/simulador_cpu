//inclusão de bibliotecas importantes ao longo do projeto
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//definições básicas da pilha ou memória-layssa mexeu nessa parte
#define TAM_MEMORIA 65536 //talvez aqui o tamanho seja seja referente ao exp 8
#define TAM_PILHA 16
#define INICIO_PILHA 0x8200

//definições do array da memoria e pilha-layssa mexeu nessa parte
uint8_t memoria[TAM_MEMORIA]; 
uint8_t pilha[TAM_PILHA];
