#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Definições básicas da memória e pilha
#define TAM_MEMORIA 65536 //64kb
#define TAM_PILHA 16
#define INICIO_PILHA 0x8200

// Definições do array da memória e pilha
uint8_t memoria_dados[TAM_MEMORIA]; // pode separar
uint8_t memoria_instr[TAM_MEMORIA]; // pode separar
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
    }

    FILE *arquivo = fopen(file_name, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo: %s\n", file_name);
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
            memoria_instr[endereco] = conteudo & 0xFF;
            memoria_instr[endereco + 1] = conteudo >> 8;
            ultima_instr = endereco + 2;
        }
    }
    fclose(arquivo);
}

// Função para apresentar o conteúdo da CPU
void apresentar_conteudo(SIMULADOR simu) {
    printf("----------------------------------");
    printf("\nRegistradores:\n\n");
    for (int i = 0; i <= 7; i++) {
        printf("R%d: 0x%04X\n", i, simu.R[i]);
    }
    printf("\nPC: 0x%04X\n", simu.PC);
    printf("SP: 0x%04X\n", simu.SP);
    printf("----------------------------------\n");

    printf("\nMemoria de Dados: \n");
    for (int i = 0; i < TAM_MEMORIA; i += 2) {
        if (memoria_dados[i] != 0 || memoria_dados[i + 1] != 0) {
            // printf("i: %d\n",i);
            printf("%04X: 0x%02X%02X\n", i, memoria_dados[i + 1], memoria_dados[i]);
        }
    }
    printf("----------------------------------");

    printf("\nPilha: \n");
    for (int i = 0; i < TAM_PILHA; i += 2) {
        printf("%04X: 0x%02X%02X\n", INICIO_PILHA + i, pilha[i], pilha[i + 1]);
    }
    printf("----------------------------------\n");
    printf("Flags: \nC = %d \nOv = %d \nZ = %d \nS = %d \n", simu.FLAGS.C, simu.FLAGS.Ov, simu.FLAGS.Z, simu.FLAGS.S);
}

SIMULADOR decodificador(SIMULADOR simu){
    while(1){

        if (simu.PC >= ultima_instr) { // caso precise parar com a última instrução
            return simu;
        }
        // printf("PC: 0x%04X\n", simu.PC);
        simu.IR = (memoria_instr[simu.PC+1] << 8) | (memoria_instr[simu.PC] );
        // printf("IR: 0x%04X\n", simu.IR);
        simu.PC += 2;
        uint8_t opcode = (simu.IR >> 12) & 0b1111;
        // printf("opcode: 0b%04d\n", opcode);
        uint8_t last2b;
        uint8_t rd, rm, rn, imm;

        switch (opcode) {

            case 0b0000: 
                if (simu.IR == 0x0000){ // NOP
                    apresentar_conteudo(simu);
                    break;
                } else{
                    last2b = simu.IR & 0b11;
                    if ((simu.IR >> 11) & 0b1){ // 11º bit ativo
                        switch (last2b) {
                            imm = (simu.IR >> 2) & 0b111111111;  
                            case 0b00: //JMP
                                simu.PC += imm;
                                break;
                            case 0b01: //JEQ
                                if (simu.FLAGS.Z == 1 && simu.FLAGS.S == 0){
                                    simu.PC += imm;
                                }
                                break;
                            case 0b10: //JLT
                                if (simu.FLAGS.Z == 0 && simu.FLAGS.S == 1){
                                    simu.PC += imm;
                                }
                                break;
                            case 0b11: //JGT
                                if (simu.FLAGS.Z == 0 && simu.FLAGS.S == 0){
                                    simu.PC += imm;
                                }
                                break;
                            default:
                                break;
                        }
                    }else{
                        switch (last2b) {
                            case 0b01: // PSH
                                if (simu.SP >= INICIO_PILHA) { // Verifica se há espaço na pilha
                                    simu.SP -= 2; // Decrementa o ponteiro da pilha
                                    rn = (simu.IR >> 2) & 0b111; // Extrai o registrador Rn
                            
                                    printf("\n--- PSH ---\n");
                                    printf("Registrador Rn (R%d): 0x%04X\n", rn, simu.R[rn]);
                                    printf("SP antes: 0x%04X\n", simu.SP + 2); // SP antes de decrementar
                                    printf("SP depois: 0x%04X\n", simu.SP);
                            
                                    pilha[simu.SP - INICIO_PILHA] = (simu.R[rn] >> 8) & 0xFF; // Armazena o byte inferior
                                    pilha[simu.SP - INICIO_PILHA + 1] = simu.R[rn] & 0xFF; // Armazena o byte superior
                            
                                    printf("Pilha após PSH:\n");
                                    for (int i = 0; i < TAM_PILHA; i += 2) {
                                        printf("%04X: 0x%02X%02X\n", INICIO_PILHA + i, pilha[i], pilha[i + 1]);
                                    }
                                } else {
                                    printf("Pilha está cheia\n");
                                }
                                break;
                            
                            case 0b10: // POP
                                if (simu.SP < INICIO_PILHA + TAM_PILHA) { // Verifica se há elementos na pilha
                                    rd = (simu.IR >> 8) & 0b111; // Extrai o registrador Rd
                            
                                    printf("\n--- POP ---\n");
                                    printf("Registrador Rd (R%d) antes: 0x%04X\n", rd, simu.R[rd]);
                                    printf("SP antes: 0x%04X\n", simu.SP);
                            
                                    simu.R[rd] = pilha[simu.SP - INICIO_PILHA] | (pilha[simu.SP - INICIO_PILHA + 1] << 8); // Recupera o valor
                                    simu.SP += 2; // Incrementa o ponteiro da pilha
                            
                                    printf("Valor desempilhado: 0x%04X\n", simu.R[rd]);
                                    printf("SP depois: 0x%04X\n", simu.SP);
                            
                                    printf("Pilha após POP:\n");
                                    for (int i = 0; i < TAM_PILHA; i += 2) {
                                        printf("%04X: 0x%02X%02X\n", INICIO_PILHA + i, pilha[i], pilha[i + 1]);
                                    }
                                } else {
                                    printf("Pilha está vazia\n");
                                }
                                break;

                            case 0b11: // CMP
                                rm = (simu.IR >> 5) & 0b111;
                                rn = (simu.IR >> 2) & 0b111;
                                if (simu.R[rm] == simu.R[rn]) {
                                    simu.FLAGS.Z = 1;
                                } else {
                                    simu.FLAGS.Z = 0;
                                }
                                if ((simu.R[rm] - simu.R[rn]) & 0x8000) {
                                    simu.FLAGS.S = 1;
                                } else {
                                    simu.FLAGS.S = 0;
                                }
                                if (simu.R[rm] < simu.R[rn]) {
                                    simu.FLAGS.C = 1;
                                } else {
                                    simu.FLAGS.C = 0;
                                }
                                simu.FLAGS.Ov = (((simu.R[rm] & 0x8000) == (simu.R[rn] & 0x8000)) && ((simu.R[rm] & 0x8000) != (simu.R[rd] & 0x8000)));
               
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                }

            case 0b1111: // HALT
                if (simu.IR == 0xFFFF){
                    return simu;
                }else{
                    printf("Instrução de formato indefinido: 0x04X\n", simu.IR);
                    return simu;
                }

            case 0b0001: // MOV
                rd = (simu.IR >> 8) & 0b111;
                if ((simu.IR & 0b0000100000000000) == 0b0000100000000000) { // 11º bit ativo
                    imm = simu.IR & 0b11111111;
                    simu.R[rd] = imm;
                    printf("%04X, %d\n",simu.R[rd], rd);
                } else {
                    rm = (simu.IR >> 5) & 0b111;
                    simu.R[rd] = simu.R[rm];
                    printf("%04X, %d\n",simu.R[rd], rd);
                }
                break;

            case 0b0010: // STR
                rm = (simu.IR >> 5) & 0b111;

                printf("Entrou no STR\n");
                printf("simu.IR: %08X\n", simu.IR);
                int bit_11 = (simu.IR >> 11) & 0b1;
                printf("11º bit: %d\n", bit_11);

                if ((simu.IR & (1 << 11)) != 0) { // 11º bit ativo
                    // printf("\nSTR de imediato\n");
                    imm = ((simu.IR >> 8) & 0b111);
                    // printf("%d\n", imm);
                    imm = imm << 5; 
                    // printf("%d\n", imm);
                    imm = imm | (simu.IR & 0b11111);
                    // printf("%d\n", imm);

                    memoria_dados[simu.R[rm]] = imm & 0xFF;
                    // printf("imm: %d\n", imm);
                    // printf("Guardado em: %04X\n", simu.R[rm]);
                    // printf("Memoria: %04X\n", memoria_dados[simu.R[rm]]);
                    // memoria_dados[simu.R[rm] + 1] = (imm >> 8);
                } else {
                    printf("\nSTR de registradores\n");
                    rn = (simu.IR >> 2) & 0b111;
                    memoria_dados[simu.R[rn]] = simu.R[rm] & 0xFF;
                    printf("rn: %04X, Conteúdo: %04X, Memoria: %04X\n", simu.R[rn], simu.R[rm], memoria_dados[simu.R[rn]]);
                    memoria_dados[simu.R[rm] + 1] = simu.R[rn] >> 8;
                }
                break;

            case 0b0011: // LDR
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                simu.R[rd] = memoria_dados[simu.R[rm]] | (memoria_dados[simu.R[rm] + 1] << 8);
                break;

            case 0b0100: //ADD
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                rn = (simu.IR >> 2) & 0b111;

                simu.R[rd] = simu.R[rm] + simu.R[rn];
                printf("%04X, %d\n",simu.R[rd], rd);

                if (simu.R[rm] > (0xFFFF - simu.R[rn])){
                    simu.FLAGS.C = 1;
                }else{
                    simu.FLAGS.C = 0;
                }

                if ((simu.R[rd]) == 0){
                    simu.FLAGS.Z = 1;
                }else{
                    simu.FLAGS.Z = 0;
                }

                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;

                if (((simu.R[rm] >> 15) == (simu.R[rn] >> 15)) && ((simu.R[rm] >> 15) != (simu.R[rd] >> 15))) {   // Verifica se o resultado tem sinal diferente
                    simu.FLAGS.Ov = 1;
                } else {
                    simu.FLAGS.Ov = 0;
                }
        

                break;

            case 0b0101: // SUB
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                rn = (simu.IR >> 2) & 0b111;
            
                printf("\n--- SUB ---\n");
                printf("Registrador Rm (R%d): 0x%04X\n", rm, simu.R[rm]);
                printf("Registrador Rn (R%d): 0x%04X\n", rn, simu.R[rn]);
            
                simu.R[rd] = simu.R[rm] - simu.R[rn];
            
                printf("Resultado (R%d): 0x%04X\n", rd, simu.R[rd]);
            
                // Atualização das flags
                if (simu.R[rm] < simu.R[rn]) { // Verifica se houve empréstimo (borrow)
                    simu.FLAGS.C = 1; // Ativa a flag de carry
                } else {
                    simu.FLAGS.C = 0; // Desativa a flag de carry
                }
            
                if (simu.R[rd] == 0) { // Verifica se o resultado é zero
                    simu.FLAGS.Z = 1;
                } else {
                    simu.FLAGS.Z = 0;
                }
            
                simu.FLAGS.S = (simu.R[rd] >> 15) & 0b1; // Sinal do resultado
            
                // Verifica overflow (quando o sinal do resultado é diferente do esperado)
                if (((simu.R[rm] >> 15) != (simu.R[rn] >> 15)) && ((simu.R[rm] >> 15) != (simu.R[rd] >> 15))) {
                    simu.FLAGS.Ov = 1;
                } else {
                    simu.FLAGS.Ov = 0;
                }
            
                printf("Flags após SUB: C=%d, Z=%d, S=%d, Ov=%d\n", simu.FLAGS.C, simu.FLAGS.Z, simu.FLAGS.S, simu.FLAGS.Ov);
                break;
            

            case 0b0110: //MUL
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                rn = (simu.IR >> 2) & 0b111;

                simu.R[rd] = simu.R[rm] * simu.R[rn];

                if (simu.R[rm] > (0xFFFF - simu.R[rn])){
                    simu.FLAGS.C = 1;
                }else{
                    simu.FLAGS.C = 0;
                }

                if ((simu.R[rd]) == 0){
                    simu.FLAGS.Z = 1;
                }else{
                    simu.FLAGS.Z = 0;
                }

                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;

                if ((((simu.R[rm]^simu.R[rn]) >> 15) & 0b1) == 0) { //sinais iguais nos dois primeiros registradores
                    if ((((simu.R[rm]^simu.R[rd]) >> 15) & 0b1) == 0) { // sinal diferente no resultado
                        simu.FLAGS.Ov = 1;
                    }else{
                        simu.FLAGS.Ov = 0;
                    }
                }else{
                    simu.FLAGS.Ov = 0;
                }

                break;


            case 0b0111: //AND
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                rn = (simu.IR >> 2) & 0b111;

                simu.R[rd] = simu.R[rm] & simu.R[rn];

                if ((simu.R[rd]) == 0){
                    simu.FLAGS.Z = 1;
                }else{
                    simu.FLAGS.Z = 0;
                }

                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;

                break;
            
            case 0b1000: //ORR
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                rn = (simu.IR >> 2) & 0b111;

                simu.R[rd] = simu.R[rm] | simu.R[rn];

                if ((simu.R[rd]) == 0){
                    simu.FLAGS.Z = 1;
                }else{
                    simu.FLAGS.Z = 0;
                }

                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;

                break;

            case 0b1001: //NOT
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;

                simu.R[rd] = ~simu.R[rm];

                if ((simu.R[rd]) == 0){
                    simu.FLAGS.Z = 1;
                }else{
                    simu.FLAGS.Z = 0;
                }
                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;

                break;

                
            case 0b1010: //XOR
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                rn = (simu.IR >> 2) & 0b111;

                simu.R[rd] = simu.R[rm] ^ simu.R[rn];

                if ((simu.R[rd]) == 0){
                    simu.FLAGS.Z = 1;
                }else{
                    simu.FLAGS.Z = 0;
                }

                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;
            
                break;

                
            case 0b1011: //SHR
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                imm = simu.IR & 0b11111;
                simu.R[rd] = simu.R[rm] >> imm;

                simu.FLAGS.C = (simu.R[rm] >> (imm -1)) & 0b1;

                if (simu.R[rd] == 0){
                    simu.FLAGS.Z = 1;
                }else {
                    simu.FLAGS.Z = 0;
                }
                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;
                
                break;

                
            case 0b1100: //SHL
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                imm = simu.IR & 0b11111;
                simu.R[rd] = simu.R[rm] << imm;

                simu.FLAGS.C = (simu.R[rm] >> (16 -imm)) & 0b1;

                if (simu.R[rd] == 0){
                    simu.FLAGS.Z = 1;
                }else {
                    simu.FLAGS.Z = 0;
                }
                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;

                if ((simu.R[rm] & (1 << (16 - imm))) != 0) {
                    simu.FLAGS.Ov = 1;  // O bit mais significativo será deslocado para fora
                } else {
                    simu.FLAGS.Ov = 0;
                }

                break;
                
            case 0b1101: //ROR
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                simu.FLAGS.C = simu.R[rm] & 0b1;
                simu.R[rd] = (simu.R[rm] >> 1) | (simu.FLAGS.C << 15);


                if (simu.R[rd] == 0){
                    simu.FLAGS.Z = 1;
                }else {
                    simu.FLAGS.Z = 0;
                }
                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;


                break;
                
            case 0b1110: //ROL
                rd = (simu.IR >> 8) & 0b111;
                rm = (simu.IR >> 5) & 0b111;
                
                simu.FLAGS.C = (simu.R[rm] >> 15) & 0b1;

                simu.R[rd] = (simu.R[rm] << 1) | simu.FLAGS.C;

                if (simu.R[rd] == 0){
                    simu.FLAGS.Z = 1;
                }else {
                    simu.FLAGS.Z = 0;
                }
                simu.FLAGS.S = (simu.R[rd]>> 15) & 0b1;


                break;

            default:
                printf("Instrução de formato indefinido: 0x04X\n", simu.IR);
                return simu;
        }

    }
} 

// Função principal
int main(void) {


    // Inicializa o simulador
    SIMULADOR simu = {
        .R = {0},
        .PC = 0,
        .IR = 0,
        .SP = INICIO_PILHA+2,
        .FLAGS = { .C = 0, .Ov = 0, .Z = 0, .S = 0 }
    };

    // Carrega a memória e exibe o conteúdo
    load_memoria();
    if (ultima_instr == 0) {
        printf("Erro: Nenhuma instrução foi carregada na memória.\n");
        return 1;
    }

    simu = decodificador(simu);
    if (simu.PC == 0) {
        printf("Erro: Nenhuma instrução foi executada.\n");
        return 1;
    }

    apresentar_conteudo(simu);

    return 0;
}