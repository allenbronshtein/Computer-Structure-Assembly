// _______________________ //
//         IMPORTS         //
// _______________________ //
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// _______________________ //
//         DATA            //
// _______________________ //
FILE *file_read;
FILE *file_write;
char *line;
int nnf = 0, case_count = 3, fcf = 1, min_case = 0, max_case = 0;
int case_Arr[1024];
// _______________________ //
//         CONSTS          //
// _______________________ //
#define BUFFER_SIZE 1024
#define READ_FILE_NAME "switch.c"
#define WRITE_FILE_NAME "switch.s"
#define ERR_MSG "switch.c file not found .\n"
#define SWITCH_STR  "switch("
#define ADD_STR "+="
#define SUB_STR "-="
#define MUL_STR "*="
#define DIV_STR "/="
#define LSHIFT_STR "<<="
#define RSHIFT_STR ">>="
#define ASSIGN_STR "="
#define DEFAULT_STR "default:"
#define BREAK_STR "break;"
#define CASE_STR "case"
// _______________________ //
//   ASSEMBLY STRa CONSTS   //
// _______________________ //
#define ASM_HEAD_STR "\t.section .text\n\t.globl   switch2\nswitch2:\n\tsubq $%d, %%rdx\n"
#define ASM_ADD_STR "\taddq %s, %s\n"
#define ASM_SUB_STR "\tsubq %s, %s\n"
#define ASM_MUL_STR "\timulq %s, %s\n"
#define ASM_DIV_STR "\tdivq %s, %s\n"
#define ASM_MOV_STR "\tmovq %s, %s\n"
#define ASM_SAL_STR "\tsalq %%cl, %s\n"
#define ASM_SHR_STR "\tsarq %%cl, %s\n"
#define ASM_SWITCH_STR ".section .rodata\n\t.align 8\n.L%d:\n"
#define ASM_CASE_STR ".L%d:\n"
#define ASM_BREAK_STR "\tret\n"

// _______________________ //
//         FUNCTIONS       //
// _______________________ //

void checkMalloc(const char *string) {
    if (string == NULL) {
        printf("Error Allocating Memory.\n");
        exit(1);
    }
}

char *assignRegister(const char *name) {
    if (strcmp(name, "p1") == 0) {
        return "(%rdi)";
    }
    if (strcmp(name, "p2") == 0) {
        return "(%rsi)";
    }
    if (strcmp(name, "result") == 0) {
        return "%rax";
    }
    char *constant = (char *) malloc(sizeof(long) + 1);
    strcpy(constant, "$");
    strcat(constant, name);
    return constant;
}

int isDig(char c) {
    if (c >= 48 && c <= 57) { return 1; }
    return 0;
}

char *stripRight() {
    char *string = (char *) malloc(BUFFER_SIZE);
    checkMalloc(string);
    int index_line = 0, index_string = 0;
    while (line[index_line] != '=') {
        index_line++;
    }
    index_line += 2;
    while (line[index_line] != ';') {
        string[index_string] = line[index_line];
        index_line++;
        index_string++;
    }
    string[index_string] = '\0';
    return string;
}

int p1isLeft(const char *token) {
    char *string = (char *) malloc(sizeof(token) + 5);
    checkMalloc(string);
    strcpy(string, "*p1 ");
    strcat(string, token);
    if (strstr(line, string)) {
        free(string);
        return 1;
    }
    free(string);
    return 0;
}

int p2isLeft(const char *token) {
    char *string = (char *) malloc(sizeof(token) + 5);
    checkMalloc(string);
    strcpy(string, "*p2 ");
    strcat(string, token);
    if (strstr(line, string)) {
        free(string);
        return 1;
    }
    free(string);
    return 0;
}

const char *checkLeft(const char *token) {
    const char *left;
    if (p1isLeft(token)) {
        left = "p1";
    } else if (p2isLeft(token)) {
        left = "p2";
    } else {
        left = "result";
    }
    return left;
}

int p1isRight(const char *token) {
    char *string = (char *) malloc(sizeof(token) + 6);
    checkMalloc(string);
    strcat(string, token);
    strcat(string, " *p1;");
    if (strstr(line, string)) {
        free(string);
        return 1;
    }
    free(string);
    return 0;
}

int p2isRight(const char *token) {
    char *string = (char *) malloc(sizeof(token) + 6);
    checkMalloc(string);
    strcat(string, token);
    strcat(string, " *p2;");
    if (strstr(line, string)) {
        free(string);
        return 1;
    }
    free(string);
    return 0;
}

int nisRight() {
    nnf = 0;
    char *string = stripRight();
    int i = 0;
    for (; i < strlen(string); i++) {
        if (!isDig(string[i])) {
            nnf = 1;
            free(string);
            return 0;
        }
    }
    int number = atoi(string);
    free(string);
    return number;
}

const char *checkRight(const char *token) {
    const char *right;
    int number = nisRight();
    if (p1isRight(token)) {
        right = "p1";
    } else if (p2isRight(token)) {
        right = "p2";
    } else if (!nnf) {
        char num[sizeof(long)];
        sprintf(num, "%d", number);
        right = num;
        nnf = 0;
    } else {
        right = "result";
    }
    return right;
}

char *stripStart(const char *buffer) {
    char *string = (char *) malloc(BUFFER_SIZE);
    checkMalloc(string);
    int buffer_index = 0, string_index = 0;
    while (buffer[buffer_index] == ' ') {
        buffer_index++;
    }
    while (buffer[buffer_index] != '\0') {
        string[string_index] = buffer[buffer_index];
        buffer_index++;
        string_index++;
    }
    string[string_index] = '\0';
    return string;
}

int badLine() {
    return line[0] == ' ' || line[0] == '\n';
}

void openFiles() {
    file_read = fopen(READ_FILE_NAME, "r");
    if (!file_read) {
        printf(ERR_MSG);
        exit(1);
    }
    file_write = fopen(WRITE_FILE_NAME, "w");
}

void closeFiles() {
    fclose(file_read);
    fclose(file_write);
}

void doSwitch() {
    char buffer[BUFFER_SIZE];
    char num[BUFFER_SIZE];
    strcpy(buffer, "\tcmpq");
    sprintf(num, " $%d, ", max_case - min_case);
    strcat(buffer, num);
    strcat(buffer, "%rdx\n\tja .L1\n\tjmp *.L");
    sprintf(num, "%d", max_case + 1);
    strcat(buffer, num);
    strcat(buffer, "(,%rdx,8)\n");
    fprintf(file_write, "%s", buffer);
}

void doDefault() {
    fprintf(file_write, ASM_CASE_STR, 1);
}

void doBreak() {
    fprintf(file_write, ASM_BREAK_STR);
}

void doAdd() {
    char *left = assignRegister(checkLeft(ADD_STR));
    char *right = assignRegister(checkRight(ADD_STR));
    if (strcmp(left, "(%rdi)") == 0 && strcmp(right, "(%rsi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rsi)", "%rbx");
        fprintf(file_write, ASM_ADD_STR, "%rbx", "(%rdi)");
    }
    else if (strcmp(left, "(%rsi)") == 0 && strcmp(right, "(%rdi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rdi)", "%rbx");
        fprintf(file_write, ASM_ADD_STR, "%rbx", "(%rsi)");
    }else{
        fprintf(file_write, ASM_ADD_STR, right, left);
    }
}

void doSub() {
    char *left = assignRegister(checkLeft(SUB_STR));
    char *right = assignRegister(checkRight(SUB_STR));
    if (strcmp(left, "(%rdi)") == 0 && strcmp(right, "(%rsi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rsi)", "%rbx");
        fprintf(file_write, ASM_SUB_STR, "%rbx", "(%rdi)");
    }
    else if (strcmp(left, "(%rsi)") == 0 && strcmp(right, "(%rdi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rdi)", "%rbx");
        fprintf(file_write, ASM_SUB_STR, "%rbx", "(%rsi)");
    }else{
        fprintf(file_write, ASM_SUB_STR, right, left);
    }
}

void doMul() {
    char *left = assignRegister(checkLeft(MUL_STR));
    char *right = assignRegister(checkRight(MUL_STR));
    if (strcmp(left, "(%rdi)") == 0 && strcmp(right, "(%rsi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rsi)", "%rbx");
        fprintf(file_write, ASM_MUL_STR, "%rbx", "(%rdi)");
    }
    else if (strcmp(left, "(%rsi)") == 0 && strcmp(right, "(%rdi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rdi)", "%rbx");
        fprintf(file_write, ASM_MUL_STR, "%rbx", "(%rsi)");
    }else{
        fprintf(file_write, ASM_MUL_STR, right, left);
    }
}

void doDiv() {
    char *left = assignRegister(checkLeft(DIV_STR));
    char *right = assignRegister(checkRight(DIV_STR));
    if (strcmp(left, "(%rdi)") == 0 && strcmp(right, "(%rsi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rsi)", "%rbx");
        fprintf(file_write, ASM_DIV_STR, "%rbx", "(%rdi)");
    }
    else if (strcmp(left, "(%rsi)") == 0 && strcmp(right, "(%rdi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rdi)", "%rbx");
        fprintf(file_write, ASM_DIV_STR, "%rbx", "(%rsi)");
    }else{
        fprintf(file_write, ASM_DIV_STR, right, left);
    }
}

void doAssign() {
    char *left = assignRegister(checkLeft(ASSIGN_STR));
    char *right = assignRegister(checkRight(ASSIGN_STR));
    if (strcmp(left, "(%rdi)") == 0 && strcmp(right, "(%rsi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rsi)", "%rbx");
        fprintf(file_write, ASM_MOV_STR, "%rbx", "(%rdi)");
    }
    else if (strcmp(left, "(%rsi)") == 0 && strcmp(right, "(%rdi)") == 0){
        fprintf(file_write, ASM_MOV_STR, "(%rdi)", "%rbx");
        fprintf(file_write, ASM_MOV_STR, "%rbx", "(%rsi)");
    }else{
        fprintf(file_write, ASM_MOV_STR, right, left);
    }
}

void doLShift() {
    char *left = assignRegister(checkLeft(LSHIFT_STR));
    char *right = assignRegister(checkRight(LSHIFT_STR));
    fprintf(file_write, ASM_MOV_STR, right, "%rcx");
    fprintf(file_write, ASM_SAL_STR, left);
}

void doRShift() {
    char *left = assignRegister(checkLeft(RSHIFT_STR));
    char *right = assignRegister(checkRight(RSHIFT_STR));
    fprintf(file_write, ASM_MOV_STR, right, "%rcx");
    fprintf(file_write, ASM_SHR_STR, left);
}

int getCaseNum() {
    char buffer[BUFFER_SIZE];
    int buff_index = 0;
    int i = 0;
    for (; i < strlen(line) - 2; i++) {
        if (isDig(line[i])) {
            buffer[buff_index] = line[i];
            buff_index++;
        }
    }
    int num = atoi(buffer);
    return num;
}

void getMinMaxCase() {
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file_read)) {
        line = stripStart(buffer);
        if (strstr(line, CASE_STR)) {
            int caseNum = getCaseNum();
            if (fcf) {
                fcf = 0;
                min_case = caseNum;
                max_case = caseNum;
            } else {
                if (caseNum < min_case) {
                    min_case = caseNum;
                }
                if (caseNum > max_case) {
                    max_case = caseNum;
                }
            }
        }
    }
    int len = max_case - min_case;
    int i = 0;
    for (; i <= len; i++) {
        case_Arr[i] = 0;
    }
    rewind(file_read);
    while (fgets(buffer, BUFFER_SIZE, file_read)) {
        line = stripStart(buffer);
        if (strstr(line, CASE_STR)) {
            int caseNum = getCaseNum();
            case_Arr[caseNum - min_case] = 1;
        }
    }
    rewind(file_read);
}

void doCase() {
    int num = getCaseNum();
    fprintf(file_write, ASM_CASE_STR, num - min_case);
}

// _______________________ //
//           MAIN          //
// _______________________ //
int main() {
    openFiles();
    getMinMaxCase();
    fprintf(file_write, ASM_HEAD_STR, min_case);
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file_read)) {
        line = stripStart(buffer);
        if (badLine()) {
            continue;
        } else if (strstr(line, SWITCH_STR)) {
            doSwitch();
        } else if (strstr(line, ADD_STR)) {
            doAdd();
        } else if (strstr(line, SUB_STR)) {
            doSub();
        } else if (strstr(line, MUL_STR)) {
            doMul();
        } else if (strstr(line, DIV_STR)) {
            doDiv();
        } else if (strstr(line, LSHIFT_STR)) {
            doLShift();
        } else if (strstr(line, RSHIFT_STR)) {
            doRShift();
        } else if (strstr(line, ASSIGN_STR)) {
            doAssign();
        } else if (strstr(line, CASE_STR)) {
            doCase();
        } else if (strstr(line, DEFAULT_STR)) {
            doDefault();
        } else if (strstr(line, BREAK_STR)) {
            doBreak();
        }
        free(line);
    }
    fprintf(file_write, "%s", "\tret\n\n");
    fprintf(file_write, ASM_SWITCH_STR, max_case + 1);
    int i = 0;
    for (; i <= max_case - min_case; i++) {
        if (case_Arr[i] == 1) { fprintf(file_write, "\t.quad\t.L%d\n", i); }
        else { fprintf(file_write, "\t.quad\t.L1\n"); }
    }
    closeFiles();
    return 0;
}
