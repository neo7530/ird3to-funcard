#ifndef IRDETO_CORE
#define IRDETO_CORE

//#include <time.h>
#include<avr/io.h>
//#include <fstream>
//#include <cstring>

//using namespace std;

//extern void encrypt(uint8_t *klo,uint8_t *keys, int runden, int start, int pposition);

extern void decrypt(uint8_t *klo,uint8_t *keys, int runden, int start, int pposition);
extern void sign(uint8_t *signaturnew, uint8_t *wert, uint8_t *zplainkey, int start);
extern void camcrypt(uint8_t *camkey, uint8_t *key);
#endif
