#ifndef CONST_H
#define CONST_H

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define BAUD 57600
#define UBRR_VALUE ((F_CPU / 16 / BAUD) - 1)

#endif