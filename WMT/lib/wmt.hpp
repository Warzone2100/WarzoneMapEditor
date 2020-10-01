/*
 * Copyright (C) Maxim Zhuchkov - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Maxim Zhuchkov <q3.max.2011@ya.ru>, May 2019
 */

#ifndef WMT_H_INCLUDED
#define WMT_H_INCLUDED

#ifndef WMT_VERSION
#define WMT_VERSION "1.8.1"
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <fstream>
#include <iostream>
#include <errno.h>

#include "zip.hpp"
#ifndef _NOLOGBUILD
#include "log.hpp"
#endif

#define WMT_MAX_PATH_LEN 2048

#if defined(_NOLOGBUILD)
#undef log_trace
#undef log_debug
#undef log_info
#undef log_warn
#undef log_error
#undef log_fatal
#define log_trace(...) ;
#define log_debug(...) ;
#define log_info(...)  ;
#define log_warn(...)  ;
#define log_error(...) ;
#define log_fatal(...) ;
#endif

/*#ifndef typename(x)
#define typename(x) _Generic((x),                                                 \
        _Bool: "_Bool",                  unsigned char: "unsigned char",          \
         char: "char",                     signed char: "signed char",            \
    short int: "short int",         unsigned short int: "unsigned short int",     \
          int: "int",                     unsigned int: "unsigned int",           \
     long int: "long int",           unsigned long int: "unsigned long int",      \
long long int: "long long int", unsigned long long int: "unsigned long long int", \
        float: "float",                         double: "double",                 \
  long double: "long double",                   char *: "pointer to char",        \
       void *: "pointer to void",                int *: "pointer to int",         \
      default: "other")
#endif */

enum WMT_TerrainTypes { ttsand, ttsandybrush, ttbakedearth, ttgreenmud, ttredbrush, ttpinkrock, ttroad, ttwater, ttclifface, ttrubble, ttsheetice, ttslush, ttmax};
extern const char* WMT_TerrainTypesStrings[];
extern unsigned short WMT_maptileoffset;

bool WMT_equalstr(char* trg, const char* chk);
int WMT_str_cut(char *str, int begin, int len);
bool WMT_str_match(char* str, char* sub);
int WMT_SearchFilename(char** arr, unsigned short sizearr, char* name, short urgent);
void WMT_PrintInfoAboutMap(struct WZmap map);

const char* WMT_CheckMap(WZmap *map);
int WMT_WriteMap(WZmap *map);
void WMT_ReadMap(char* filename, struct WZmap *map);
void WMT_FreeMap(WZmap *map);

enum WZtileset { tileset_arizona, tileset_urban, tileset_rockies };

#define WZTILE_XFLIPMASK       0x8000
#define WZTILE_XFLIPSHIFT      15
#define WZTILE_YFLIPMASK       0x4000
#define WZTILE_YFLIPSHIFT      14
#define WZTILE_ROTMASK         0x3000
#define WZTILE_ROTSHIFT        12
#define WZTILE_TRIFLIP         0x0800
#define WZTILE_NUMMASK         0x01ff

//src/map.h:330
#define WZTTYPES_MAX 255

struct WZdroid {
	char name[128];
	uint32_t id;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t direction;
	uint32_t player;
	bool inFire;
	uint32_t burnStart;
	uint32_t burnDamage;
};

struct WZobject {
	char name[128];
	uint32_t id;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t direction;
	int rotation[3];
	uint32_t player;
	bool infire;
	uint32_t burnStart;
	uint32_t burnDamage;
	uint8_t structPadding;
	uint8_t structPadding1;
	uint8_t structPadding2;
	uint8_t structPadding3;
	int32_t buildPoints;
	uint32_t body;
	uint32_t armour;
	uint32_t resistance;
	uint32_t dummy;
	uint32_t subjectInc;
	uint32_t timeStarted;
	uint32_t output;
	uint32_t capacity;
	uint32_t quantity;

	uint32_t factoryInc;
	uint8_t loopsPerformed;
	uint8_t structPadding4;
	uint8_t structPadding5;
	uint8_t structPadding6;
	uint32_t powerAccrued;
	uint32_t dummy2;
	uint32_t droidTimeStarted;
	uint32_t timeToBuild;
	uint32_t timeStartHold;

	uint8_t visibility[8];

	char researchName[60];
	//int type;
	//char script[32];

	int modules = 0;
	bool oldformat = false;
};

struct WZfeature {
	char name[128];
	uint32_t id;
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t direction;
	uint32_t player;
	uint32_t inFire;
	uint32_t burnStart;
	uint32_t burnDamage;
};

struct WZlevel {
	char name[128];
	unsigned short players = 0;
	unsigned short type = 0;
	char dataset[128];
};

struct WZmap {
	char* path;
	char* mapname;

	bool valid = true;
	int errorcode = 0;

	char **filenames = NULL;
	zip_t *zip;
	int totalentries = -1;

	char createdon[128];
	char createddate[128];
	char createdauthor[128];
	char createdlicense[128];
	bool haveadditioninfo = false;
	unsigned int levelsfound = 0;
	WZlevel levels[64];
	unsigned short players = 0;

	unsigned int ttypver = -1;
	unsigned int ttypnum = -1;
	unsigned short ttyptt[WZTTYPES_MAX];
	void *ttpcontents;
	ssize_t ttpcontentslen;
	WZtileset tileset = tileset_arizona;

	unsigned int maparrsize = 0;
	unsigned int mapver = -1;
	unsigned int maptotalx = -1;
	unsigned int maptotaly = -1;
	unsigned short maptile[90000];
	unsigned short *mapheight;
	void *mapcontents;
	ssize_t mapcontentslen;

	uint32_t structVersion;
	uint32_t numStructures;
	WZobject *structs = NULL;

	uint32_t featureVersion;
	uint32_t featuresCount;
	WZfeature *features = NULL;

	uint32_t droidsVersion;
	uint32_t droidsCount;
	WZdroid *droids = NULL;

	uint32_t gamVersion;
	void* gamcontents;
	ssize_t gamcontentslen;
	uint32_t gameTime;
	uint32_t gameType;
	uint32_t scrollminx;
	uint32_t scrollminy;
	uint32_t scrollmaxx;
	uint32_t scrollmaxy;
	char gamLevelName[20];
	uint32_t gamPower[8];

	bool fields_clean = false;
};

inline bool WMT_TileGetTriFlip(unsigned short t) {
	return t & WZTILE_TRIFLIP;}
inline int WMT_TileGetTexture(unsigned short t) {
	return t & WZTILE_NUMMASK;}
inline WMT_TerrainTypes WMT_TileGetTerrainType(unsigned short tile, unsigned short types[WZTTYPES_MAX]) {
	return (WMT_TerrainTypes)(types[(short)(tile & WZTILE_NUMMASK)]);}
inline bool WMT_TileGetXFlip(unsigned short t) {
	return (t & WZTILE_XFLIPMASK) >> WZTILE_XFLIPSHIFT;}
inline bool WMT_TileGetYFlip(unsigned short t) {
	return (t & WZTILE_YFLIPMASK) >> WZTILE_YFLIPSHIFT;}
inline char WMT_TileGetRotation(unsigned short t) {
	return (t & WZTILE_ROTMASK) >> WZTILE_ROTSHIFT;}


#endif /* WMT_H_INCLUDED */
