/* file: "picobit-vm.h" */

/*
 * Copyright 2004-2009 by Marc Feeley and Vincent St-Amour, All Rights Reserved.
 */

#ifndef PICOBIT_VM_H
#define PICOBIT_VM_H

#define DEBUG_not
#define DEBUG_GC_not
#define INFINITE_PRECISION_BIGNUMS

/*---------------------------------------------------------------------------*/

// types

typedef char int8;
typedef short int16;
typedef long int32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

typedef uint8 word;

typedef uint16 ram_addr;
typedef uint16 rom_addr;

// pointers are 13 bits
typedef uint16 obj;

/*---------------------------------------------------------------------------*/

// bignum definitions

#ifdef INFINITE_PRECISION_BIGNUMS

#define digit_width 16

typedef obj integer;
typedef uint16 digit; // TODO why these ? adds to the confusion
typedef uint32 two_digit;

#endif

/*---------------------------------------------------------------------------*/

// environment

#ifdef PICOBOARD2
#define ROBOT
#endif

#ifdef HI_TECH_C
#define ROBOT
#endif

#ifndef ROBOT
#define WORKSTATION
#endif


#ifdef HI_TECH_C

#include <pic18.h>

static volatile near uint8 FW_VALUE_UP       @ 0x33;
static volatile near uint8 FW_VALUE_HI       @ 0x33;
static volatile near uint8 FW_VALUE_LO       @ 0x33;

#define ACTIVITY_LED1_LAT LATB
#define ACTIVITY_LED1_BIT 5
#define ACTIVITY_LED2_LAT LATB
#define ACTIVITY_LED2_BIT 4
static volatile near bit ACTIVITY_LED1 @ ((unsigned)&ACTIVITY_LED1_LAT*8)+ACTIVITY_LED1_BIT;
static volatile near bit ACTIVITY_LED2 @ ((unsigned)&ACTIVITY_LED2_LAT*8)+ACTIVITY_LED2_BIT;

#endif


#ifdef WORKSTATION

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>

// for libpcap
#define MAX_PACKET_SIZE BUFSIZ
#define PROMISC 1
#define TO_MSEC 1
char errbuf[PCAP_ERRBUF_SIZE];
pcap_t *handle;
#define INTERFACE "eth0"
char buf [MAX_PACKET_SIZE]; // buffer for writing


#ifdef _WIN32

#include <sys/types.h>
#include <sys/timeb.h>
#include <conio.h>

#else

#include <sys/time.h>

#endif

#endif

/*---------------------------------------------------------------------------*/

// miscellaneous definitions
// TODO put at the end ?

// TODO these 2 are only used in negp, use them elsewhere ?
#define true  1
#define false 0

#define CODE_START 0x5000

/*---------------------------------------------------------------------------*/

// debugging

#ifdef DEBUG
#define IF_TRACE(x) x
#define IF_GC_TRACE(x) x
#else
#define IF_TRACE(x)
#define IF_GC_TRACE(x)
#endif

/*---------------------------------------------------------------------------*/

// error handling

#ifdef PICOBOARD2
#define ERROR(prim, msg) halt_with_error()
#define TYPE_ERROR(prim, type) halt_with_error()
#endif

#ifdef WORKSTATION
#define ERROR(prim, msg) error (prim, msg)
#define TYPE_ERROR(prim, type) type_error (prim, type)
void error (char *prim, char *msg);
void type_error (char *prim, char *type);
#endif

/*---------------------------------------------------------------------------*/

// address space layout
// TODO document each zone, also explain that since vector space is in ram, it uses the ram primitives

#define MAX_VEC_ENCODING 8191
#define MIN_VEC_ENCODING 4096
#define VEC_BYTES ((MAX_VEC_ENCODING - MIN_VEC_ENCODING + 1)*4)
// if the pic has less than 8k of memory, start vector space lower
// TODO the pic actually has 2k, so change these
// TODO we'd only actually need 1024 or so for ram and vectors, since we can't address more. this gives us a lot of rom space

#define MAX_RAM_ENCODING 4095
#define MIN_RAM_ENCODING 512
#define RAM_BYTES ((MAX_RAM_ENCODING - MIN_RAM_ENCODING + 1)*4)
// TODO watch out if we address more than what the PIC actually has

#define MIN_FIXNUM_ENCODING 3
#define MIN_FIXNUM -1
#define MAX_FIXNUM 255
#define MIN_ROM_ENCODING (MIN_FIXNUM_ENCODING+MAX_FIXNUM-MIN_FIXNUM+1)

#define OBJ_TO_RAM_ADDR(o,f) (((ram_addr)((uint16)(o) - MIN_RAM_ENCODING) << 2) + (f))
#define OBJ_TO_ROM_ADDR(o,f) (((rom_addr)((uint16)(o) - MIN_ROM_ENCODING) << 2) + (CODE_START + 4 + (f)))


#ifdef PICOBOARD2
#define ram_get(a) *(uint8*)(a+0x200)
#define ram_set(a,x) *(uint8*)(a+0x200) = (x)
#endif


#ifdef WORKSTATION
uint8 ram_mem[RAM_BYTES + VEC_BYTES];
#define ram_get(a) ram_mem[a]
#define ram_set(a,x) ram_mem[a] = (x)
#endif

#ifdef PICOBOARD2
uint8 rom_get (rom_addr a){
  return *(rom uint8*)a;
}
#endif


#ifdef WORKSTATION
#define ROM_BYTES 8192
// TODO the new pics have 32k, change this ? minus the vm size, firmware ?
uint8 rom_mem[ROM_BYTES];
uint8 rom_get (rom_addr a);
#endif

/*---------------------------------------------------------------------------*/

// memory access

#define RAM_GET_FIELD0_MACRO(o) ram_get (OBJ_TO_RAM_ADDR(o,0))
#define RAM_SET_FIELD0_MACRO(o,val) ram_set (OBJ_TO_RAM_ADDR(o,0), val)
#define ROM_GET_FIELD0_MACRO(o) rom_get (OBJ_TO_ROM_ADDR(o,0))

#define RAM_GET_GC_TAGS_MACRO(o) (RAM_GET_FIELD0_MACRO(o) & 0x60)
#define RAM_GET_GC_TAG0_MACRO(o) (RAM_GET_FIELD0_MACRO(o) & 0x20)
#define RAM_GET_GC_TAG1_MACRO(o) (RAM_GET_FIELD0_MACRO(o) & 0x40)
#define RAM_SET_GC_TAGS_MACRO(o,tags)                                      \
  (RAM_SET_FIELD0_MACRO(o,(RAM_GET_FIELD0_MACRO(o) & 0x9f) | (tags)))
#define RAM_SET_GC_TAG0_MACRO(o,tag)                                    \
  RAM_SET_FIELD0_MACRO(o,(RAM_GET_FIELD0_MACRO(o) & 0xdf) | (tag))
#define RAM_SET_GC_TAG1_MACRO(o,tag)                                    \
  RAM_SET_FIELD0_MACRO(o,(RAM_GET_FIELD0_MACRO(o) & 0xbf) | (tag))

#define RAM_GET_FIELD1_MACRO(o) ram_get (OBJ_TO_RAM_ADDR(o,1))
#define RAM_GET_FIELD2_MACRO(o) ram_get (OBJ_TO_RAM_ADDR(o,2))
#define RAM_GET_FIELD3_MACRO(o) ram_get (OBJ_TO_RAM_ADDR(o,3))
#define RAM_SET_FIELD1_MACRO(o,val) ram_set (OBJ_TO_RAM_ADDR(o,1), val)
#define RAM_SET_FIELD2_MACRO(o,val) ram_set (OBJ_TO_RAM_ADDR(o,2), val)
#define RAM_SET_FIELD3_MACRO(o,val) ram_set (OBJ_TO_RAM_ADDR(o,3), val)
#define ROM_GET_FIELD1_MACRO(o) rom_get (OBJ_TO_ROM_ADDR(o,1))
#define ROM_GET_FIELD2_MACRO(o) rom_get (OBJ_TO_ROM_ADDR(o,2))
#define ROM_GET_FIELD3_MACRO(o) rom_get (OBJ_TO_ROM_ADDR(o,3))

word ram_get_gc_tags (obj o);
word ram_get_gc_tag0 (obj o);
word ram_get_gc_tag1 (obj o);
void ram_set_gc_tags (obj o, word tags);
void ram_set_gc_tag0 (obj o, word tag);
void ram_set_gc_tag1 (obj o, word tag);
word ram_get_field0 (obj o);
word ram_get_field1 (obj o);
word ram_get_field2 (obj o);
word ram_get_field3 (obj o);
word ram_get_fieldn (obj o, word n);
void ram_set_field0 (obj o, word val);
void ram_set_field1 (obj o, word val);
void ram_set_field2 (obj o, word val);
void ram_set_field3 (obj o, word val);
void ram_set_fieldn (obj o, uint8 n, word val);
word rom_get_field0 (obj o);
word rom_get_field1 (obj o);
word rom_get_field2 (obj o);
word rom_get_field3 (obj o);

obj ram_get_car (obj o);
obj rom_get_car (obj o);
obj ram_get_cdr (obj o);
obj rom_get_cdr (obj o);
void ram_set_car (obj o, obj val);
void ram_set_cdr (obj o, obj val);

obj ram_get_entry (obj o);
obj rom_get_entry (obj o);

obj get_global (uint8 i);
void set_global (uint8 i, obj o);

/*---------------------------------------------------------------------------*/

/*
  OBJECT ENCODING:

  #f           0
  #t           1
  ()           2
  fixnum n     MIN_FIXNUM -> 3 ... MAX_FIXNUM -> 3 + (MAX_FIXNUM-MIN_FIXNUM)
  rom object   4 + (MAX_FIXNUM-MIN_FIXNUM) ... MIN_RAM_ENCODING-1
  ram object   MIN_RAM_ENCODING ... MAX_RAM_ENCODING
  u8vector     MIN_VEC_ENCODING ... 8191

  layout of memory allocated objects:

  Gs represent mark bits used by the gc

  ifdef INFINITE_PRECISION_BIGNUMS
  bignum n     0GG***** **next** hhhhhhhh llllllll  (16 bit digit)
  TODO what to do with the gc tags for the bignums ? will this work ?
  
  ifndef INFINITE_PRECISION_BIGNUMS
  bignum n     00000000 uuuuuuuu hhhhhhhh llllllll  (24 bit signed integer)

  pair         1GGaaaaa aaaaaaaa 000ddddd dddddddd
  a is car
  d is cdr
  gives an address space of 2^13 * 4 = 32k divided between simple objects,
  rom, ram and vectors

  symbol       1GG00000 00000000 00100000 00000000

  string       1GG***** *chars** 01000000 00000000

  u8vector     1GGxxxxx xxxxxxxx 011yyyyy yyyyyyyy
  x is length of the vector, in bytes (stored raw, not encoded as an object)
  y is pointer to the elements themselves (stored in vector space)

  closure      01Gaaaaa aaaaaaaa aaaxxxxx xxxxxxxx
  0x5ff<a<0x4000 is entry
  x is pointer to environment
  the reason why the environment is on the cdr (and the entry is split on 3
  bytes) is that, when looking for a variable, a closure is considered to be a
  pair. The compiler adds an extra offset to any variable in the closure's
  environment, so the car of the closure (which doesn't really exist) is never
  checked, but the cdr is followed to find the other bindings
  
  continuation 1GGxxxxx xxxxxxxx 100yyyyy yyyyyyyy
  x is parent continuation
  y is pointer to the second half, which is a closure (contains env and entry)
  
  An environment is a list of objects built out of pairs.  On entry to
  a procedure the environment is the list of parameters to which is
  added the environment of the closure being called.

  The first byte at the entry point of a procedure gives the arity of
  the procedure:

  n = 0 to 127    -> procedure has n parameters (no rest parameter)
  n = -128 to -1  -> procedure has -n parameters, the last is
  a rest parameter
*/

#define OBJ_FALSE 0
#define OBJ_TRUE  1
#define encode_bool(x) ((obj)(x))

#define OBJ_NULL  2

// fixnum definitions in picobit-vm.h , address space layout section

#define ENCODE_FIXNUM(n) ((obj)(n) + (MIN_FIXNUM_ENCODING - MIN_FIXNUM))
#define DECODE_FIXNUM(o) ((int32)(o) - (MIN_FIXNUM_ENCODING - MIN_FIXNUM))

#define IN_VEC(o) ((o) >= MIN_VEC_ENCODING)
#define IN_RAM(o) (!IN_VEC(o) && ((o) >= MIN_RAM_ENCODING))
#define IN_ROM(o) (!IN_VEC(o) && !IN_RAM(o) && ((o) >= MIN_ROM_ENCODING))

// bignum first byte : 00Gxxxxx
#define BIGNUM_FIELD0 0
#define RAM_BIGNUM(o) ((ram_get_field0 (o) & 0xc0) == BIGNUM_FIELD0)
#define ROM_BIGNUM(o) ((rom_get_field0 (o) & 0xc0) == BIGNUM_FIELD0)

// composite first byte : 1GGxxxxx
#define COMPOSITE_FIELD0 0x80
#define RAM_COMPOSITE(o) ((ram_get_field0 (o) & 0x80) == COMPOSITE_FIELD0)
#define ROM_COMPOSITE(o) ((rom_get_field0 (o) & 0x80) == COMPOSITE_FIELD0)

// pair third byte : 000xxxxx
#define PAIR_FIELD2 0
#define RAM_PAIR(o) (RAM_COMPOSITE (o) && ((ram_get_field2 (o) & 0xe0) == PAIR_FIELD2))
#define ROM_PAIR(o) (ROM_COMPOSITE (o) && ((rom_get_field2 (o) & 0xe0) == PAIR_FIELD2))

// symbol third byte : 001xxxxx
#define SYMBOL_FIELD2 0x20
#define RAM_SYMBOL(o) (RAM_COMPOSITE (o) && ((ram_get_field2 (o) & 0xe0) == SYMBOL_FIELD2))
#define ROM_SYMBOL(o) (ROM_COMPOSITE (o) && ((rom_get_field2 (o) & 0xe0) == SYMBOL_FIELD2))

// string third byte : 010xxxxx
#define STRING_FIELD2 0x40
#define RAM_STRING(o) (RAM_COMPOSITE (o) && ((ram_get_field2 (o) & 0xe0) == STRING_FIELD2))
#define ROM_STRING(o) (ROM_COMPOSITE (o) && ((rom_get_field2 (o) & 0xe0) == STRING_FIELD2))

// vector third byte : 011xxxxx
#define VECTOR_FIELD2 0x60
#define RAM_VECTOR(o) (RAM_COMPOSITE (o) && ((ram_get_field2 (o) & 0xe0) == VECTOR_FIELD2))
#define ROM_VECTOR(o) (ROM_COMPOSITE (o) && ((rom_get_field2 (o) & 0xe0) == VECTOR_FIELD2))

// continuation third byte : 100xxxxx
#define CONTINUATION_FIELD2 0x80
#define RAM_CONTINUATION(o) (RAM_COMPOSITE (o) && ((ram_get_field2 (o) & 0xe0) == CONTINUATION_FIELD2))
#define ROM_CONTINUATION(o) (ROM_COMPOSITE (o) && ((rom_get_field2 (o) & 0xe0) == CONTINUATION_FIELD2))

// closure first byte : 01Gxxxxx
#define CLOSURE_FIELD0 0x40
#define RAM_CLOSURE(o) ((ram_get_field0 (o) & 0xc0) == CLOSURE_FIELD0)
#define ROM_CLOSURE(o) ((rom_get_field0 (o) & 0xc0) == CLOSURE_FIELD0)

/*---------------------------------------------------------------------------*/

// garbage collector

// TODO explain what each tag means, with 1-2 mark bits
#define GC_TAG_0_LEFT   (1<<5)
#define GC_TAG_1_LEFT   (2<<5)
#define GC_TAG_UNMARKED (0<<5)

/* Number of object fields of objects in ram */
#define HAS_2_OBJECT_FIELDS(visit) (RAM_PAIR(visit) || RAM_CONTINUATION(visit))
#ifdef INFINITE_PRECISION_BIGNUMS
#define HAS_1_OBJECT_FIELD(visit)  (RAM_COMPOSITE(visit) \
				    || RAM_CLOSURE(visit) || RAM_BIGNUM(visit))
#else
#define HAS_1_OBJECT_FIELD(visit)  (RAM_COMPOSITE(visit) || RAM_CLOSURE(visit))
#endif
// all composites except pairs and continuations have 1 object field

#define NIL OBJ_FALSE

obj free_list; /* list of unused cells */
obj free_list_vec; /* list of unused cells in vector space */

obj arg1; /* root set */
obj arg2;
obj arg3;
obj arg4;
obj arg5;
obj cont;
obj env;

uint8 na; /* interpreter variables */
rom_addr pc;
uint8 glovars;
rom_addr entry;
uint8 bytecode;
uint8 bytecode_hi4;
uint8 bytecode_lo4;
int32 a1;
int32 a2;
int32 a3;

/*---------------------------------------------------------------------------*/

#endif

