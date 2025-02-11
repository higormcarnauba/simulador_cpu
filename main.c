#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Definições básicas da memória e pilha
#define TAM_MEMORIA 32768
#define TAM_PILHA 16
#define INICIO_PILHA 0x8200

// Definições do array da memória e pilha
uint8_t memoria[TAM_MEMORIA];
uint8_t pilha[TAM_PILHA];
uint16_t ultima_instr = 0;

// Definição das flags
typedef struct {
    uint8_t C;
    uint8_t Ov;
    uint8_t Z;
    uint8_t S;
} Flags;

// Definição do simulador
typedef struct {
    uint16_t R[8]; // Registradores R0-R7
    uint16_t PC;   // Program Counter
    uint16_t IR;   // Instruction Register
    uint16_t SP;   // Stack Pointer
    Flags FLAGS;   // Flags
} SIMULADOR;

// Função para carregar a memória a partir de um arquivo
void load_memoria() {

    char file_name[256];

    // Solicita o caminho do arquivo
    printf("Digite o caminho do arquivo: ");
    if (scanf("%255s", file_name) != 1) {
        printf("Erro ao ler o caminho do arquivo.\n");
        return 1;
    }

    FILE *arquivo = fopen(file_name, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo: %s\n", file_name);
        return 1;
    }

    if (!arquivo) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    char linha[256];
    uint16_t endereco, conteudo;

    // Lê cada linha do arquivo
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (sscanf(linha, "%hx:0x%hx", &endereco, &conteudo) == 2) {
            memoria[endereco] = conteudo & 0xFF;
            memoria[endereco + 1] = conteudo >> 8;
        }
    }

    ultima_instr = endereco + 1;  // Atualiza o endereço da última instrução
    fclose(arquivo);
}

// Função para apresentar o conteúdo da CPU
void apresentar_conteudo(SIMULADOR simu) {
    printf("Registradores: \n");
    for (int i = 0; i <= 7; i++) {
        printf("R%d: 0x%04X\n", i, simu.R[i]);
    }
    printf("PC: 0x%04X\n", simu.PC);
    printf("SP: 0x%04X\n", simu.SP);

    printf("Memoria de Dados: \n");
    for (int i = 0; i < TAM_MEMORIA; i += 2) {
        if (memoria[i] != 0 || memoria[i + 1] != 0) {
            printf("%04X: 0x%02X%02X\n", i, memoria[i], memoria[i + 1]);
        }
    }

    printf("Pilha: \n");
    for (int i = 0; i < TAM_PILHA; i += 2) {
        printf("%04X: 0x%02X%02X\n", INICIO_PILHA + i, pilha[i], pilha[i + 1]);
    }

    printf("Flags: \nC=%d \nOv=%d \nZ=%d \nS=%d \n", simu.FLAGS.C, simu.FLAGS.Ov, simu.FLAGS.Z, simu.FLAGS.S);
}

SIMULADOR instrucao(SIMULADOR simu){
    while(1){
        simu.IR = memoria[simu.PC] | (memoria[simu.PC +1] << 8):
    }
} 

// Função principal
int main(void) {


    // Inicializa o simulador
    SIMULADOR simu = {0};
    simu.SP = INICIO_PILHA;

    // Carrega a memória e exibe o conteúdo
    load_memoria();
    apresentar_conteudo(simu);

    simu = instrucao(simu);

    return 0;
}