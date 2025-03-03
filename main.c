#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TAM_MEMORIA 65536 //64kb
#define TAM_PILHA 16
#define INICIO_PILHA 0x8200

uint8_t memoria_dados[TAM_MEMORIA];
uint8_t memoria_instr[TAM_MEMORIA];
uint8_t *pilha[TAM_PILHA];
uint16_t ultima_instr = 0;

typedef struct {
    uint8_t C;
    uint8_t Ov;
    uint8_t Z;
    uint8_t S;
} Flags;

typedef struct {
    uint16_t R[8];
    uint16_t PC;   // Program Counter
    uint16_t IR;   // Instruction Register
    uint16_t SP;   // Ponterio da Pilha
    Flags FLAGS; 
} SIMULADOR;


void load_memoria() {

    char file_name[256];
    
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

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (sscanf(linha, "%hx:0x%hx", &endereco, &conteudo) == 2) {
            memoria_instr[endereco] = conteudo & 0xFF;
            memoria_instr[endereco + 1] = conteudo >> 8;
            ultima_instr = endereco + 2;
        }
    }
    for (int i = 0; i < TAM_PILHA; i++) {
        pilha[i] = (uint16_t *)&memoria_dados[INICIO_PILHA - (TAM_PILHA + (i * 2))];
    }

    fclose(arquivo);
}

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
        if (i >= (INICIO_PILHA - TAM_PILHA) && i <= INICIO_PILHA) {
            continue;
        }
        if (memoria_dados[i] != 0 || memoria_dados[i + 1] != 0) {
            printf("%04X: 0x%02X%02X\n", i, memoria_dados[i + 1], memoria_dados[i]);
        }
    }
    printf("----------------------------------");

    printf("\nPilha: \n");
    for (int i = 0; i < TAM_PILHA; i += 2) {
        printf("%04X: 0x%04X\n", INICIO_PILHA - i, *((uint16_t *)&memoria_dados[INICIO_PILHA - i - 2]));
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
                            case 0b00: //JMP
                                imm = (simu.IR >> 2) & 0b111111111;
                                printf("imediato antes %d\n", imm);
                                printf("imediato depois %d\n", imm);
                                printf("chegou aqui %d\n", simu.PC);
                                simu.PC += imm;
                                printf("depois %d\n", simu.PC);
                                break;
                            case 0b01: //JEQ
                                imm = (simu.IR >> 2) & 0b111111111;
                                // printf("chegou aqui Z = %d S = %d \n", simu.FLAGS.Z, simu.FLAGS.S);
                                if (simu.FLAGS.Z == 1 && simu.FLAGS.S == 0){
                                    // printf("pasou aqui Z = %d S = %d \n", simu.FLAGS.Z, simu.FLAGS.S);
                                    simu.PC += imm;
                                }
                                break;
                            case 0b10: //JLT
                                imm = (simu.IR >> 2) & 0b111111111;
                                if (simu.FLAGS.Z == 0 && simu.FLAGS.S == 1){
                                    simu.PC += imm;
                                }
                                break;
                            case 0b11: //JGT
                                imm = (simu.IR >> 2) & 0b111111111;
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
                                if (simu.SP > INICIO_PILHA - TAM_PILHA) {
                                    rn = (simu.IR >> 2) & 0b111;
                                    memoria_dados[simu.SP] = (simu.R[rn] & 0xFF);
                                    memoria_dados[simu.SP + 1] = (simu.R[rn] >> 8) & 0xFF;
                                    simu.SP -= 2;
                                } else {
                                    printf("Erro: Pilha cheia!\n");
                                }
                                break;
                            
                            case 0b10: // POP
                                if (simu.SP >= INICIO_PILHA - TAM_PILHA && simu.SP < INICIO_PILHA) {
                                    rd = (simu.IR >> 8) & 0b111;
                                    simu.SP += 2;
                                    simu.R[rd] = memoria_dados[simu.SP] | (memoria_dados[simu.SP + 1] << 8);
                            
                                } else {
                                    printf("Erro: Pilha vazia!\n");
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
                                if (simu.R[rm] < simu.R[rn]) {
                                    simu.FLAGS.S = 1;
                                } else {
                                    simu.FLAGS.S = 0;
                                }
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
                if ((simu.IR >> 11) & 0b1) { // 11º bit ativo
                    imm = simu.IR & 0b11111111;
                    simu.R[rd] = imm;
                    // printf("%04X, %d\n",simu.R[rd], rd);
                } else {
                    rm = (simu.IR >> 5) & 0b111;
                    simu.R[rd] = simu.R[rm];
                    // printf("%04X, %d\n",simu.R[rd], rd);
                }
                break;

            case 0b0010: // STR
                rm = (simu.IR >> 5) & 0b111;

                if ((simu.IR >> 11) & 0b1) { // 11º bit ativo
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
                    memoria_dados[simu.R[rm] + 1] = (imm >> 8);
                } else {
                    // printf("\nSTR de registradores\n");
                    rn = (simu.IR >> 2) & 0b111;
                    memoria_dados[simu.R[rn]] = simu.R[rm] & 0xFF;
                    // printf("rn: %04X, Conteúdo: %04X, Memoria: %04X\n", simu.R[rn], simu.R[rm], memoria_dados[simu.R[rn]]);
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
                // printf("%04X, %d\n",simu.R[rd], rd);

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
            
                // printf("\n--- SUB ---\n");
                // printf("Registrador Rm (R%d): 0x%04X\n", rm, simu.R[rm]);
                // printf("Registrador Rn (R%d): 0x%04X\n", rn, simu.R[rn]);
            
                simu.R[rd] = simu.R[rm] - simu.R[rn];
            
                // printf("Resultado (R%d): 0x%04X\n", rd, simu.R[rd]);

                if (simu.R[rm] < simu.R[rn]) {
                    simu.FLAGS.C = 1;
                } else {
                    simu.FLAGS.C = 0;
                }
            
                if (simu.R[rd] == 0) {
                    simu.FLAGS.Z = 1;
                } else {
                    simu.FLAGS.Z = 0;
                }
            
                simu.FLAGS.S = (simu.R[rd] >> 15) & 0b1;
            
                if (((simu.R[rm] >> 15) != (simu.R[rn] >> 15)) && ((simu.R[rm] >> 15) != (simu.R[rd] >> 15))) {
                    simu.FLAGS.Ov = 1;
                } else {
                    simu.FLAGS.Ov = 0;
                }
            
                // printf("Flags após SUB: C=%d, Z=%d, S=%d, Ov=%d\n", simu.FLAGS.C, simu.FLAGS.Z, simu.FLAGS.S, simu.FLAGS.Ov);
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
                    if ((((simu.R[rn]^simu.R[rd]) >> 15) & 0b1) == 1) { // sinal diferente no resultado
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
                    simu.FLAGS.Ov = 1; 
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

int main(void) {

    SIMULADOR simu = {
        .R = {0},
        .PC = 0,
        .IR = 0,
        .SP = INICIO_PILHA,
        .FLAGS = { .C = 0, .Ov = 0, .Z = 0, .S = 0 }
    };

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