#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_WORDS 1000
#define MAX_LEN 10000
#define MAX_FILES 5

typedef struct {
    char word[50];
    double value;
} WordTFIDF;

/* ---------------- Utility ---------------- */

void toLower(char *s) {
    for (int i = 0; s[i]; i++)
        s[i] = tolower(s[i]);
}

int isStopWord(char *w) {
    char *stop[] = {"the","is","and","a","to","of","in","on"};
    for (int i = 0; i < 8; i++)
        if (strcmp(w, stop[i]) == 0)
            return 1;
    return 0;
}

int findWord(WordTFIDF arr[], int size, char *w) {
    for (int i = 0; i < size; i++)
        if (strcmp(arr[i].word, w) == 0)
            return i;
    return -1;
}

/* ---------------- Reading & TF ---------------- */

int buildTF(char *text, WordTFIDF tf[]) {
    int size = 0;
    char *tok = strtok(text, " ,.-\n");

    while (tok) {
        toLower(tok);
        if (!isStopWord(tok)) {
            int idx = findWord(tf, size, tok);
            if (idx == -1) {
                strcpy(tf[size].word, tok);
                tf[size].value = 1;
                size++;
            } else {
                tf[idx].value++;
            }
        }
        tok = strtok(NULL, " ,.-\n");
    }
    return size;
}

void readFile(char *name, char *buf) {
    FILE *fp = fopen(name, "r");
    if (!fp) {
        printf("Cannot open %s\n", name);
        exit(1);
    }
    int len = fread(buf, 1, MAX_LEN - 1, fp);
    buf[len] = '\0';
    fclose(fp);
}

/* ---------------- IDF ---------------- */

double computeIDF(char *word, char files[][20], int fileCount) {
    int docs = 0;
    char buffer[MAX_LEN];

    for (int i = 0; i < fileCount; i++) {
        readFile(files[i], buffer);
        toLower(buffer);
        if (strstr(buffer, word))
            docs++;
    }

    if (docs == 0) return 0;
    return log((double)fileCount / docs);
}

void applyTFIDF(WordTFIDF tf[], int size, char files[][20], int fileCount) {
    for (int i = 0; i < size; i++)
        tf[i].value *= computeIDF(tf[i].word, files, fileCount);
}

/* ---------------- Similarity ---------------- */

double cosineSimilarity(WordTFIDF a[], int sa, WordTFIDF b[], int sb) {
    double dot = 0, magA = 0, magB = 0;

    for (int i = 0; i < sa; i++) {
        magA += a[i].value * a[i].value;
        int idx = findWord(b, sb, a[i].word);
        if (idx != -1)
            dot += a[i].value * b[idx].value;
    }

    for (int i = 0; i < sb; i++)
        magB += b[i].value * b[i].value;

    return dot / (sqrt(magA) * sqrt(magB));
}

/* ---------------- Main ---------------- */

int main() {
    char input[MAX_LEN];
    char buffer[MAX_LEN];

    char realFiles[][20] = {"real_1.txt", "real_2.txt"};
    char fakeFiles[][20] = {"fake_1.txt", "fake_2.txt"};

    WordTFIDF inputTF[MAX_WORDS];
    WordTFIDF realTF[MAX_WORDS];
    WordTFIDF fakeTF[MAX_WORDS];

    printf("Enter news text:\n");
    fgets(input, MAX_LEN, stdin);

    int inputSize = buildTF(input, inputTF);

    buffer[0] = '\0';
    for (int i = 0; i < 2; i++) {
        char temp[MAX_LEN];
        readFile(realFiles[i], temp);
        strcat(buffer, temp);
    }
    int realSize = buildTF(buffer, realTF);

    buffer[0] = '\0';
    for (int i = 0; i < 2; i++) {
        char temp[MAX_LEN];
        readFile(fakeFiles[i], temp);
        strcat(buffer, temp);
    }
    int fakeSize = buildTF(buffer, fakeTF);

    applyTFIDF(inputTF, inputSize, realFiles, 2);
    applyTFIDF(realTF, realSize, realFiles, 2);
    applyTFIDF(fakeTF, fakeSize, fakeFiles, 2);

    double realScore = cosineSimilarity(inputTF, inputSize, realTF, realSize);
    double fakeScore = cosineSimilarity(inputTF, inputSize, fakeTF, fakeSize);

    printf("\n==============================\n");
    printf("REAL score : %.4f\n", realScore);
    printf("FAKE score : %.4f\n", fakeScore);
    printf("==============================\n");

    if (realScore > fakeScore && realScore > 0.05)
        printf("FINAL CLASSIFICATION: REAL NEWS\n");
    else
        printf("FINAL CLASSIFICATION: FAKE NEWS\n");

}