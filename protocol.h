#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <inttypes.h>

typedef struct block {

    int instruction;
    int size;
    int encode_type;

} __attribute__((packed))block;


typedef struct encode0 {

    /* all have topics */
    char topic[60];
    unsigned int type;
    uint32_t INT;

    
    /* client UDP info */
    char ip_address[16];
    char port[6];

} __attribute__((packed))encode0;

typedef struct encode1 {

    /* all have topics */
    char topic[60];
    unsigned int type;
    float SHORT_REAL;
    
    
    /* client UDP info */
    char ip_address[16];
    char port[6];


} __attribute__((packed))encode1;

typedef struct encode2 {

    /* all have topics */
    char topic[60];
    unsigned int type;
    float FLOAT;

    /* client UDP info */
    char ip_address[16];
    char port[6];


} __attribute__((packed))encode2;

typedef struct encode3 {

    /* all have topics */
    char topic[60];
    unsigned int type;

    /* client UDP info */
    char ip_address[16];
    char port[6];

    /* modify this */
    char payload[1600];


} __attribute__((packed))encode3;


#endif