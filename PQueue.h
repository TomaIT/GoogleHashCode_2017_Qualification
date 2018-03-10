//
// Created by Nanni on 10/03/2018.
//

/*
Implementa un coda a priorità, su una struttura dati di tipo Array e logica di gestione a Heap.
Complessità delle varie operazione O(nlog(n)).
*/

#ifndef PQUEUE_H
#define PQUEUE_H

#include <stdlib.h>
#include <stdint.h>

/*La #define DEBUG rende compilabile i controlli delle varie funzioni, conviene
lasciarla fin quando il progetto non è terminato.*/

#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif //DEBUG

typedef struct PQueue_ *PQueue;

typedef struct item_ *Item;

struct item_{
    int idVideo;
    int idServer;
    double key;
    uint32_t index;
};


/*ItemGreater(a,b) definisce il criterio con il quale si determina la priorità maggiore*/
#define ItemGreater(a,b) ((a->key)>(b->key))
#define ItemLess(a,b) ((a->key)<(b->key))

Item item_init(int idServer,int idVideo,double key);

void item_free(Item item);

/*Inizializza la struttura dati adatta a contenere al più maxSize elementi (è presente malloc)*/
PQueue pqueue_init(uint32_t maxSize);

/*Libera la memoria occupata da PQueue in*/
void pqueue_free(PQueue in);

/*Ritorna 1 se la coda è vuota altrimenti 0*/
uint8_t pqueue_is_empty(PQueue in);

/*Ritorna 1 se la coda è piena altrimenti 0*/
uint8_t pqueue_is_full(PQueue in);

/*Ritorna il numero di elementi presenti nella coda*/
uint32_t pqueue_count(PQueue in);

/*Inserisce l'elemento nella coda*/
void pqueue_insert(PQueue in,Item item);

/*Ritorna l'elemento a priorità maggiore senza estrarlo dalla struttura dati*/
Item pqueue_get(PQueue in);

/*Ritorna ed estrae l'elemento a priorità maggiore dalla struttura dati*/
Item pqueue_extract(PQueue in);

/*Ridimensiona la struttura dati di PQueue in a contenere al più maxSize elementi, è necessario
 che la dimensione di PQueue in sia >= a maxSize (è presente malloc)*/
void pqueue_resize(PQueue in,uint32_t maxSize);

/*Sostituisce l'item nel posizione pos, mantenendo inalterate le proprietà di heap*/
void pqueue_change(PQueue in,Item item,double newKey);


#endif // PQUEUE_H

