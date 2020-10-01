/*
 * Copyright (C) Maxim Zhuchkov - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Maxim Zhuchkov <q3.max.2011@ya.ru>, May 2019
 */

#include "wmt.hpp"
#include <time.h>
#include "../lib/json.hpp"
#include <iostream>
#include <string>

using json = nlohmann::json;

#define _WMT_strsize(x) (sizeof(char)*strlen(x))

#ifndef __bswap_constant_32
#define __bswap_constant_32(x)                                        \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)        \
   | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))
#endif

const char *WMT_TerrainTypesStrings[] = {"sand", "sandy brush", "baked earth", "green mud", "red brush", "pink rock", "road", "water", "cliffface", "rubble", "sheetice", "slush", "max"};
unsigned short WMT_maptileoffset = 0x01ff;

#ifdef _WIN32
int vasprintf(char** strp, const char* fmt, va_list ap) {
	va_list ap2;
	va_copy(ap2, ap);
	char tmp[1];
	int size = vsnprintf(tmp, 1, fmt, ap2);
	if (size <= 0) return size;
	va_end(ap2);
	size += 1;
	*strp = (char*)malloc(size * sizeof(char));
	return vsnprintf(*strp, size, fmt, ap);
}
int asprintf(char **strp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vasprintf(strp, fmt, ap);
    va_end(ap);
    return r;
}

size_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;
    if (lineptr == NULL)
        return -1;
    if (stream == NULL)
        return -1;
    if (n == NULL)
        return -1;
    bufptr = *lineptr;
    size = *n;
    c = fgetc(stream);
    if (c == EOF)
        return -1;
    if (bufptr == NULL) {
        bufptr = (char*)malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = (char*)realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }
    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;
    return p - bufptr - 1;
}
#endif

bool WMT_equalstr(char* trg, const char* chk) {
	if(strlen(trg)!=strlen(chk))
		return false;
	for(unsigned int c=0; c<strlen(trg); c++)
		if(trg[c]!=chk[c])
			return false;
	return true;
}

int WMT_str_cut(char *str, int begin, int len) {
    int l = strlen(str);
    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);
    return len;
}

bool WMT_str_match(char* str, char* sub) {
	log_trace("String match between \"%s\" and \"%s\"", str, sub);
	int i, j=0, k;
	for(i=0; str[i]; i++) {
		if(str[i] == sub[j]) {
			for(k=i, j=0; str[k] && sub[j]; j++, k++)
				if(str[k]!=sub[j]) {
					log_trace("string constr fail in");
					log_trace("str1 %s", str);
					log_trace("     %*c^", k, ' ');
					log_trace("str2 %s", sub);
					log_trace("     %*c^", j, ' ');
					log_trace("on %d %d bc %d != %d", k, j, str[k], sub[j]);
					break;
				}
			if(!sub[j]) {
				return true;
			}
		}
	}
	if(strstr(str, sub)) {
		log_info("strstr worked!");
		return true;
	}
	return false;
}

void WMT_PrintObject(WZobject obj) {
	printf("WZobject dump:\n");
	printf("Name:           %s\n", obj.name);
	printf("Id:             %d\n", obj.id);
	printf("X:              %d\n", obj.x);
	printf("Y:              %d\n", obj.y);
	printf("Z:              %d\n", obj.z);
	printf("Direction:      %d\n", obj.direction);
	printf("Player:         %d\n", obj.player);
	if(obj.infire!=0) {
		printf("In fire:        %d\n", obj.infire);
		printf("Burn start:     %d\n", obj.burnStart);
		printf("Burn damage:    %d\n", obj.burnDamage);
	}
	if(obj.structPadding!=1)
		printf("Struct padding: %d\n", obj.structPadding);
	if(obj.buildPoints!=0)
		printf("Build points:   %d\n", obj.buildPoints);
	if(obj.body!=0)
		printf("Body:           %d\n", obj.body);
	if(obj.armour!=0)
		printf("Armour:         %d\n", obj.armour);
	if(obj.resistance!=0)
		printf("Resistance:     %d\n", obj.resistance);
	if(obj.subjectInc!=0)
		printf("Subject inc.:   %d\n", obj.subjectInc);
	if(obj.timeStarted!=0)
		printf("Time started:   %d\n", obj.timeStarted);
	if(obj.output!=0)
		printf("Output:         %d\n", obj.output);
	if(obj.capacity!=0)
		printf("Capacity:       %d\n", obj.capacity);
	if(obj.quantity!=0)
		printf("Quantity:       %d\n", obj.quantity);
	//printf("visibility [0]: %d\n", obj.visibility[0]);
	//printf("visibility [1]: %d\n", obj.visibility[1]);
	//printf("visibility [2]: %d\n", obj.visibility[2]);
	//printf("visibility [3]: %d\n", obj.visibility[3]);
	//printf("visibility [4]: %d\n", obj.visibility[4]);
	//printf("visibility [5]: %d\n", obj.visibility[5]);
	//printf("visibility [6]: %d\n", obj.visibility[6]);
	//printf("visibility [7]: %d\n", obj.visibility[7]);
}

void WMT_PrintObjectShort(WZobject obj) {
	printf("WZobject   %s\n", obj.name);
	printf("Id:        %d\n", obj.id);
	printf("X:         %d\n", obj.x);
	printf("Y:         %d\n", obj.y);
	printf("Z:         %d\n", obj.z);
	//printf("Direction: %d\n", obj.direction);
	printf("Player:    %d\n", obj.player);
}

void WMT_PrintFeatureShort(WZfeature feat) {
	printf("WZfeature: %s\n", feat.name);
	printf("Id:        %d\n", feat.id);
	printf("X:         %d\n", feat.x);
	printf("Y:         %d\n", feat.y);
	printf("Z:         %d\n", feat.z);
	printf("Player:    %d\n", feat.player);
}

int WMT_SearchFilename(char** arr, unsigned short sizearr, char* name, short urgent = 0) {
	//log_trace("Serching filename \"%s\" in array of filenames...", name);
	int foundindex=-1;
	for(int index=0; index<sizearr; index++)
		if(strstr(arr[index], name))
			foundindex=index;
	if(foundindex==-1) {
		switch(urgent) {
			case 2:
				log_fatal("Failed to find \"%s\".", name);
				exit(0);
			break;
			case 1:
				log_warn("Failed to find \"%s\".", name);
				return -1;
			break;
			default:
				log_trace("Failed to find \"%s\".", name);
				return -1;
			break;
		}
	} else {
		//log_trace("Serching filename \"%s\" in array of filenames... %d (%s)", name, foundindex, arr[foundindex]);
		return foundindex;
	}
}

int WMT_SearchFileExt(char** arr, unsigned short sizearr, char* ext, short urgent = 0) {
	// Don't return last/first found by ext. file. Just mark unfoun if found more than one.
	for(int i=0; i<sizearr; i++) {
		if(strncmp(arr[i]+strlen(arr[i])-strlen(ext), ext, strlen(ext)) == 0) {
			return i;
		}
	}
	switch(urgent) {
		case 2:
			log_fatal("Failed to find \"%s\" extension.", ext);
			exit(0);
		break;
		case 1:
			log_warn("Failed to find \"%s\" extension.", ext);
			return -1;
		break;
		default:
			log_trace("Failed to find file by \"%s\" extension.", ext);
			return -1;
		break;
	}
	return -1;
}

bool WMT_ListFiles(WZmap *map) { //pass pointer to struct to make changes
	map->totalentries = zip_total_entries(map->zip);
	map->filenames = NULL;
	log_trace("Allocating %ld for %d names...", map->totalentries*sizeof(char*), map->totalentries);
	int tebcp = map->totalentries;
	map->filenames = (char**) calloc(tebcp, sizeof(char*));    //////////////////////////////
	for(int i=0; i<map->totalentries; i++)
		map->filenames[i] = (char*) malloc(1024 * sizeof(char));
	if(!map->filenames) {
		log_fatal("Filenames array allocation fail!");
		map->errorcode = -2;
		return false;
	}
	for(int i=0; i<map->totalentries; i++) {
		if(!map->filenames[i]) {
			log_fatal("Filename %d allocation fail!");
			map->errorcode = -2;
			return false;
		}
	}
	//log_trace("Listing all shit up (%d) ...", map->totalentries);
	for(int index=0; index<map->totalentries; index++) {
		//log_trace("Trying index %d", index);
		int ret = zip_entry_openbyindex(map->zip, index);
		if(ret>=0) {
			snprintf(map->filenames[index], 1024, "%s", (char*)zip_entry_name(map->zip));
			log_trace("%d.\t%s", index, map->filenames[index]);
			zip_entry_close(map->zip);
		}
		else {
			log_error("Error openig file %d! Status %d", index, ret);
		}
	}
	return true;
}

char* WMT_PrintTilesetName(WZtileset t) {
	switch(t) {
	case tileset_arizona:
	return (char*)"Arizona";
	break;
	case tileset_urban:
	return (char*)"Urban";
	break;
	case tileset_rockies:
	return (char*)"Rockies";
	break;
	default:
	return (char*)"Unknown";
	break;
	}
}

void WMT_PrintInfoAboutMap(struct WZmap map) {
	if(!map.valid) {
		printf("Not valid map!\n");
		return;
	}
	printf("Map: %s\n", map.mapname);
	printf("Tileset: %s\n", WMT_PrintTilesetName(map.tileset));
	printf("Size: %dx%d\n", map.maptotalx, map.maptotaly);
}

bool WMT_ReadFromFile(FILE *fp, size_t s, size_t v, void *var) {
	size_t r = fread(var, s, v, fp);
	//log_debug("Fread return %d need %d", r, v);
	if(r != v) {
		log_warn("Fread return count (%d) not (%d)!", r, v);
	}
	if(ferror(fp)) {
			log_fatal("Fread read only %ld instead of %lu. Cursor: %ld Readed: %u", r, s, ftell(fp), var);
			log_fatal("Fread error: %s", strerror(errno));
			return false;
	}
	return true;
}

bool WMT_ReadGAMFile(WZmap *map) {
	bool success = true;
	int indexgam = WMT_SearchFileExt(map->filenames, map->totalentries, (char*)".gam", 1);

	int namestart = 0;
	int flen = strlen(map->filenames[indexgam]);
	for(int i=0; i<flen; i++)
		if(map->filenames[indexgam][i] == '\\' ||
		   map->filenames[indexgam][i] == '/')
			namestart = i+1;
	map->mapname = (char*) malloc(WMT_MAX_PATH_LEN*sizeof(char));
	strncpy(map->mapname, map->filenames[indexgam]+namestart, flen-namestart-4);
	map->mapname[flen-namestart-4] = '\0';

	int openstatus = zip_entry_openbyindex(map->zip, indexgam);
	if(openstatus<0) {
		log_fatal("Opening file by index error! Status %d.", openstatus);
		map->errorcode = -4;
		success = false;
	} else {
		size_t readlen;
		ssize_t readed = zip_entry_read(map->zip, &map->gamcontents, &readlen);
		map->gamcontentslen = readed;
		if(readed == -1) {
			log_fatal("Error reading gam file!");
			map->errorcode = -6;
			success = false;
		} else {
			FILE* gamf = NULL;
#ifdef _WIN32
			FILE* tmpf = tmpfile();
			fwrite(map->gamcontents, readlen, 1, tmpf);
			rewind(tmpf);
			gamf = tmpf;
#else
			gamf = fmemopen(map->gamcontents, readlen, "r");
#endif
			if(gamf==NULL) {
				log_fatal("Error opening as file descriptor!");
				map->errorcode = -4;
				success = false;
			} else {
				char gamhead[5] = { '0', '0', '0', '0', '\0'};
				if(!WMT_ReadFromFile(gamf, sizeof(char), 4, &gamhead)) {
					log_error("Failed to read gam header!");
					fclose(gamf);
					zip_entry_close(map->zip);
					map->errorcode = -3;
					return false;
				}
				if(gamhead[0] != 'g' ||
				   gamhead[1] != 'a' ||
				   gamhead[2] != 'm' ||
				   gamhead[3] != 'e') {
					log_warn("GAM file header not \'game\'! (%d %d %d %d got)", gamhead[0], gamhead[1], gamhead[2], gamhead[3]);
					fclose(gamf);
					zip_entry_close(map->zip);
					map->errorcode = -3;
					return false;
				}
				if(!WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->gamVersion)) {
					log_error("Failed to read gam version!");
					fclose(gamf);
					zip_entry_close(map->zip);
					map->errorcode = -3;
					return false;
				}

				{union { uint32_t i; char c[4]; } e = { 0x01000000 };
				if(e.c[0]) {printf("We are dealing with some big endian shit!");} }

				bool needswap = false;
				if(map->gamVersion > 35) {
					//log_warn("GAM file is version %d and we need to read big endian!");
					needswap = true;
				}

				if(!WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->gameTime) ||
				   !WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->gameType) ||
				   !WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->scrollminx) ||
				   !WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->scrollminy) ||
				   !WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->scrollmaxx) ||
				   !WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->scrollmaxy) ||
				   !WMT_ReadFromFile(gamf, sizeof(char), 20, &map->gamLevelName)) {
					log_fatal("GAM data read seems to be incorrect!");
					fclose(gamf);
					zip_entry_close(map->zip);
					map->errorcode = -3;
					return false;
				}

				for(int energyc = 0; energyc < 8; energyc++) {
					if(map->gamVersion >= 10) {
						unsigned int dummy;
						if(!WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &map->gamPower[energyc]) ||
						   !WMT_ReadFromFile(gamf, sizeof(unsigned int), 1, &dummy))
						{
							log_fatal("GAM power data seems to be WRONG!");
							fclose(gamf);
							zip_entry_close(map->zip);
							map->errorcode = -3;
							return false;
						}
					} else {
						map->gamPower[energyc] = 0; // no default found
					}
				}

				if(needswap) { // see glibc/bits/byteswap.h
					__bswap_constant_32(map->gameTime);
					__bswap_constant_32(map->gameType);
					__bswap_constant_32(map->scrollminx);
					__bswap_constant_32(map->scrollminy);
					__bswap_constant_32(map->scrollmaxx);
					__bswap_constant_32(map->scrollmaxy);
					for(int swp=0; swp<8; swp++)
						__bswap_constant_32(map->gamPower[swp]);
				}

				fclose(gamf);
			}
		}
		zip_entry_close(map->zip);
	}
	return success;
}

bool WMT_ReadTTypesFile(WZmap *map) {
	bool success = true;
	int indexttypes = WMT_SearchFilename(map->filenames, map->totalentries, (char*)"ttypes.ttp", 2);
	int openstatus = zip_entry_openbyindex(map->zip, indexttypes);
	if(openstatus<0) {
		log_fatal("Opening file by index error! Status %d.", openstatus);
		map->errorcode = -4;
		success = false;
	} else {
		//size_t ttpfilesize = zip_entry_size(map->zip);
		size_t readlen;
		ssize_t readed = zip_entry_read(map->zip, &map->ttpcontents, &readlen);
		map->ttpcontentslen = readed;
		if(readed==-1) {
			log_fatal("Error reading ttypes file!");
			map->errorcode = -6;
			success = false;
		} else {
			FILE* ttpf = NULL;
#ifdef _WIN32
			FILE* tmpf = tmpfile();
			fwrite(map->ttpcontents, readlen, 1, tmpf);
			rewind(tmpf);
			ttpf = tmpf;
#else
			ttpf = fmemopen(map->ttpcontents, readlen, "r");
#endif
			if(ttpf==NULL) {
				log_fatal("Error opening as file descriptor!");
				success = false;
			} else {
				char ttphead[5] = { '0', '0', '0', '0', '\0'};
				if(!WMT_ReadFromFile(ttpf, sizeof(char), 4, &ttphead))
					log_error("Failed to read ttypes header!");
				if(ttphead[0] != 't' ||
				   ttphead[1] != 't' ||
				   ttphead[2] != 'y' ||
				   ttphead[3] != 'p')
					log_warn("Ttypes file header not \'ttyp\'!");
				if(!WMT_ReadFromFile(ttpf, sizeof(unsigned int), 1, &map->ttypver))
					log_error("Failed to read ttypes version!");
				if(!WMT_ReadFromFile(ttpf, sizeof(unsigned int), 1, &map->ttypnum))
					log_error("Failed to read number of terrain types!");
				if(!WMT_ReadFromFile(ttpf, sizeof(unsigned short), map->ttypnum, &map->ttyptt))
					log_error("Failed to read terrain types!");

				uint8_t TileSetProbe[3];
				TileSetProbe[0] = (uint8_t)map->ttyptt[0];
				TileSetProbe[1] = (uint8_t)map->ttyptt[1];
				TileSetProbe[2] = (uint8_t)map->ttyptt[2];

				if (TileSetProbe[0] == 1 && TileSetProbe[1] == 0 && TileSetProbe[2] == 2)
					map->tileset = tileset_arizona;
				else if (TileSetProbe[0] == 2 && TileSetProbe[1] == 2 && TileSetProbe[2] == 2)
					map->tileset = tileset_urban;
				else if (TileSetProbe[0] == 0 && TileSetProbe[1] == 0 && TileSetProbe[2] == 2)
					map->tileset = tileset_rockies;

				//printf("Tileset: %s\n", WMT_PrintTilesetName(map->tileset));

				fclose(ttpf);
				//printf("Results of readyng ttypes.ttp:\n");
				//printf("Header: \"%s\"\n", ttphead);
				//printf("Version: %d\n", map->ttypver);
				//printf("Types:  %d\n", map->ttypnum);
			}
			//FIXME there should be freeing!
		}
		zip_entry_close(map->zip);
	}
	return success;
}

bool WMT_ReadGameMapFile(WZmap *map) {
	bool success = true;
	int indexgamemap = WMT_SearchFilename(map->filenames, map->totalentries, (char*)"game.map", 1);
	int openstatus;
	if(indexgamemap==-1) {
		log_warn("Failed to search game.map file! Trying second way!");
		char mapnewpath[WMT_MAX_PATH_LEN];
		for(int i=0; i<WMT_MAX_PATH_LEN; i++)
			mapnewpath[i]='0';
		snprintf(mapnewpath, WMT_MAX_PATH_LEN, "multiplay/maps/%s/game.map", map->mapname);
		openstatus = zip_entry_open(map->zip, mapnewpath);
	} else {
		openstatus = zip_entry_openbyindex(map->zip, indexgamemap);
	}
	if(openstatus<0) {
		log_fatal("Failed to open game.map file!");
		map->errorcode = -4;
		success = false;
	} else {
		//size_t mapfilesize = zip_entry_size(map->zip);
		size_t readlen;
		ssize_t readed = zip_entry_read(map->zip, &map->mapcontents, &readlen);
		map->mapcontentslen = readed;
		if(readed==-1) {
			log_warn("Zip file reading error!");
		} else {
			FILE* mapf = NULL;
#ifdef _WIN32
			FILE* tmpf = tmpfile();
			fwrite(map->mapcontents, readlen, 1, tmpf);
			rewind(tmpf);
			mapf = tmpf;
#else
			mapf = fmemopen(map->mapcontents, readlen, "r");
#endif
			if(mapf==NULL) {
				log_fatal("Error opening file from memory!");
				success = false;
			} else {
				char maphead[5] = { '0', '0', '0', '0', '\0'};
				if(!WMT_ReadFromFile(mapf, sizeof(char), 4, &maphead))
					log_error("Failed to read ttypes header!");
				if(maphead[0] != 'm' ||
				   maphead[1] != 'a' ||
				   maphead[2] != 'p')
					log_warn("Map file header not \'map\'!");
				if(!WMT_ReadFromFile(mapf, sizeof(unsigned int), 1, &map->mapver))
					log_error("Failed to read map file version!");
				if(!WMT_ReadFromFile(mapf, sizeof(unsigned int), 1, &map->maptotalx))
					log_error("Failed to read map bounds (x)");
				if(!WMT_ReadFromFile(mapf, sizeof(unsigned int), 1, &map->maptotaly))
					log_error("Failed to read map bounds (y)");
				//printf("\nResults of reading game.map\n");
				//printf("Version: %d\n", map->mapver);
				//printf("Width:   %d\n", map->maptotaly);
				//printf("Height:  %d\n", map->maptotalx);

				int maparraysize = map->maptotaly*map->maptotalx;
				map->mapheight = (unsigned short*) calloc(maparraysize, sizeof(unsigned short));
				if(map->mapheight==NULL) {
					log_fatal("Height array allocation failed!");
				}
				unsigned short maptileinfo = 0;
				ssize_t mapreadret = -1;
				WMT_TerrainTypes maptileterrain = ttsand;
				for(int counter=0; counter<maparraysize; counter++)
				{
					mapreadret = fread(&map->maptile[counter], 2, 1, mapf);
					if(mapreadret != 1)
						log_error("Fread scanned %d elements instead of %d (tileinfo)", mapreadret, 1);
					mapreadret = fread(&map->mapheight[counter], 1, 1, mapf);
					if(mapreadret != 1)
						log_error("Fread scanned %d elements instead of %d (height)", mapreadret, 1);
				}
			}
		}
		//FIXME there should be freeing too!
		zip_entry_close(map->zip);
	}
	return success;
}



bool WMT_ReadAddonLev(WZmap *map) {
	int opened = 0;
	int index = 0;
	for(int i=0; i<map->totalentries; i++)
		if(strstr(map->filenames[i], ".xplayers.lev") != NULL)
			index = i;
	opened = zip_entry_openbyindex(map->zip, index);
	if(opened < 0) {
		for(int i=0; i<map->totalentries; i++)
			if(strstr(map->filenames[i], ".addon.lev") != NULL)
				index = i;
		opened = zip_entry_openbyindex(map->zip, index);
	}
	if(opened<0) {
		log_fatal("Failed to open addon.lev file!");
		map->errorcode = -4;
		return false;
	} else {
		void *addoncontents;
		size_t readlen;
		ssize_t readed = zip_entry_read(map->zip, &addoncontents, &readlen);
		//log_fatal("readlen %d readed %d", readlen, readed);
		if(readed==-1) {
			log_warn("Zip file reading error!");
			zip_entry_close(map->zip);
			return false;
		} else {
			FILE* addonf = NULL;
#ifdef _WIN32
			FILE* tmpf = tmpfile();
			fwrite(addoncontents, readlen, 1, tmpf);
			rewind(tmpf);
			addonf = tmpf;
#else
			addonf = fmemopen(addoncontents, readlen, "r");
#endif
			if(addonf==NULL) {
				log_fatal("Error opening file from memory!");
				return false;
			} else {
				char* tmpline = NULL;
				unsigned short linenumber = 0;
				ssize_t read;
				size_t len;
				unsigned int LevelNumber = 0;
				while((read = getline(&tmpline, &len, addonf)) != -1) {
					tmpline[strlen(tmpline)-1] = 0;
					if( (linenumber == 0 || linenumber == 1 || linenumber == 2 || linenumber == 3) &&
						tmpline[0] == '/' &&
						tmpline[1] == '/')
					{
						switch(linenumber) {
							case 0:
							strcpy(map->createdon, tmpline+3);
							break;
							case 1:
							strcpy(map->createddate, tmpline+3);
							break;
							case 2:
							strcpy(map->createdauthor, tmpline+3);
							break;
							case 3:
							strcpy(map->createdlicense, tmpline+3);
							break;
							default:
							log_info("Old map format!");
							break;
						}
					}
					else
					{
						log_trace("Parsing \"%s\"", tmpline);
						if(strstr(tmpline, "level") == tmpline) {
							log_trace("Level found!");
							LevelNumber++;
							int spaceoffset;
							for(spaceoffset=5; tmpline[spaceoffset] == ' '; spaceoffset++) {}
							strcpy(map->levels[LevelNumber-1].name, tmpline+spaceoffset);
							log_error("[%s]", map->levels[LevelNumber-1].name);
						}
						if(strstr(tmpline, "players") == tmpline) {
							log_trace("Players found!");
							map->levels[LevelNumber-1].players = atoi(tmpline+8);
						}
						if(strstr(tmpline, "type") == tmpline) {
							log_trace("Type found!");
							map->levels[LevelNumber-1].type = atoi(tmpline+5);
						}
						if(strstr(tmpline, "dataset") == tmpline) {
							log_trace("Level found!");
							strcpy(map->levels[LevelNumber-1].dataset, tmpline+7);
						}
					}
					linenumber++;
				}
				map->levelsfound = LevelNumber;
				int playercount=map->levels[0].players;
				for(unsigned int i=1; i<LevelNumber; i++)
					if(map->levels[i].players != playercount)
						log_warn("Multilevel map!");
				map->players = playercount;
				free(tmpline);
				map->haveadditioninfo = true;
				fclose(addonf);
			}
		}
		free(addoncontents);
		addoncontents = NULL;
		zip_entry_close(map->zip);
	}
	return true;
}

bool WMT_OldReadStructs(WZmap *map) {
	bool success = true;
	int indexstructs = WMT_SearchFilename(map->filenames, map->totalentries, (char*)"struct.bjo");
	if(indexstructs == -1) {
		log_fatal("Failed to find struct.bjo!");
		return false;
	}
	int openstatus = zip_entry_openbyindex(map->zip, indexstructs);
	if(openstatus<0) {
		log_fatal("Failed to open struct.bjo!");
		map->errorcode = -4;
		success = false;
	} else {
		void *structcontents;
		size_t readlen;
		ssize_t readed = zip_entry_read(map->zip, &structcontents, &readlen);
		if(readed == -1) {
			log_fatal("Failed to read struct.bjo!");
			success = false;
		} else {
			FILE* structf = NULL;
#ifdef _WIN32
			FILE* tmpf = tmpfile();
			fwrite(structcontents, readlen, 1, tmpf);
			rewind(tmpf);
			structf = tmpf;
#else
			structf = fmemopen(structcontents, readlen, "r");
#endif
			if(structf==NULL) {
				log_fatal("Error opening struct.bjo from memory!");
				success = false;
			} else {
				char structshead[5] = { '0', '0', '0', '0', '\0'};
				if(!WMT_ReadFromFile(structf, sizeof(char), 4, &structshead))
					log_error("Failed to read struct header!");
				if(structshead[0] != 's' ||
				   structshead[1] != 't' ||
				   structshead[2] != 'r' ||
				   structshead[3] != 'u')
					log_warn("Struct file header not \'stru\'! (%s)", structshead);
				if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structVersion))
					log_error("Failed to read struct file version!");
				if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->numStructures))
					log_error("Failed to read structure count!");

				//printf("Struct version: %d\n", map->structVersion);
				//printf("Structs count:  %d\n", map->numStructures);


				map->structs = (WZobject*)malloc(sizeof(WZobject)*map->numStructures);
				if(map->structs == NULL) {
					log_fatal("Failed to allocate memory for structures!");
					map->errorcode = -2;
					return false;
				}
				int nameLength = 60;
				if(map->structVersion <= 19)
					nameLength = 40;


				for(unsigned int structnum = 0; structnum<map->numStructures; structnum++) {
					if(!WMT_ReadFromFile(structf, sizeof(char), nameLength, &map->structs[structnum].name))
						log_error("Failed to read struct name!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].id))
						log_error("Failed to read struct id!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].x))
						log_error("Failed to read struct x!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].y))
						log_error("Failed to read struct y!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].z))
						log_error("Failed to read struct z!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].direction))
						log_error("Failed to read struct direction!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].player))
						log_error("Failed to read struct player!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].infire))
						log_error("Failed to read struct in fire!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].burnStart))
						log_error("Failed to read struct start burn!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].burnDamage))
						log_error("Failed to read struct burn damage!");
					if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].structPadding))
						log_error("Failed to read struct padding!");
					if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].structPadding1))
						log_error("Failed to read struct padding (1)!");
					if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].structPadding2))
						log_error("Failed to read struct padding (2)!");
					if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].structPadding3))
						log_error("Failed to read struct padding (3)!");
					if(!WMT_ReadFromFile(structf, sizeof(int32_t), 1, &map->structs[structnum].buildPoints))
						log_error("Failed to read struct build points!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].body))
						log_error("Failed to read struct body!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].armour))
						log_error("Failed to read struct armour!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].resistance))
						log_error("Failed to read struct resistance!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].dummy))
						log_error("Failed to read some dummy var!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].subjectInc))
						log_error("Failed to read struct subject inc!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].timeStarted))
						log_error("Failed to read struct time started!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].output))
						log_error("Failed to read struct output?!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].capacity))
						log_error("Failed to read struct capacity!");
					if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].quantity))
						log_error("Failed to read struct quantity!");
					if(map->structVersion >= 12) {
						if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].factoryInc))
							log_error("Failed to read struct factory inc!");
						if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].loopsPerformed))
							log_error("Failed to read struct loops!");
						if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].structPadding4))
							log_error("Failed to read struct padding (4)!");
						if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].structPadding5))
							log_error("Failed to read struct padding (5)!");
						if(!WMT_ReadFromFile(structf, sizeof(uint8_t), 1, &map->structs[structnum].structPadding6))
							log_error("Failed to read struct padding (6)!");
						if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].powerAccrued))
							log_error("Failed to read struct power!");
						if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].dummy2))
							log_error("Failed to read another dummy var!");
						if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].droidTimeStarted))
							log_error("Failed to read struct 'droid time started'!");
						if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].timeToBuild))
							log_error("Failed to read struct time to build!");
						if(!WMT_ReadFromFile(structf, sizeof(uint32_t), 1, &map->structs[structnum].timeStartHold))
							log_error("Failed to read struct time start hold!");
					}
					if(map->structVersion >= 14) {
						size_t freadret = fread(&map->structs[structnum].visibility, 8, 1, structf);
						if(freadret != 1)
							log_error("Error reading visibility! Code %d.", freadret);
					}
					if(map->structVersion >= 15) {
						size_t freadret = fread(&map->structs[structnum].researchName, sizeof(char), nameLength, structf);
						if(freadret != 1)
							log_error("Error reading research name! Code %d.", freadret);
					}
					if(map->structVersion >= 17) {
						int16_t dummy;
						WMT_ReadFromFile(structf, sizeof(int16_t), 1, &dummy);
						(void)dummy;
					}
					if(map->structVersion >= 15) {
						int16_t dummy;
						WMT_ReadFromFile(structf, sizeof(int16_t), 1, &dummy);
						(void)dummy;
					}
					if(map->structVersion >= 21) {
						uint32_t dummy;
						WMT_ReadFromFile(structf, sizeof(int16_t), 1, &dummy);
						(void)dummy;
					}
					//WMT_PrintObjectShort(map->structs[structnum]);
				}
				fclose(structf);
			}
			free(structcontents);
			structcontents = NULL;
		}
		zip_entry_close(map->zip);
	}
	return success;
}

bool WMT_ReadStructsJSON(WZmap *map) {
	size_t readlen = zip_entry_size(map->zip);
	char* content = (char*)malloc(readlen+1);
	ssize_t readed = zip_entry_noallocread(map->zip, (void*)content, readlen);
	if(readed == -1)
	{
		log_fatal("Error reading [%s] from archive.");
		zip_entry_close(map->zip);
		return false;
	}
	content[readlen] = '\0';
	auto structs = json::parse(content);
	map->numStructures = structs.size();
	map->structs = (WZobject*)malloc(map->numStructures*sizeof(WZobject));
	int scounter = 0;
	log_debug("Reading structs JSON...");
	bool noID = false;
	for(json::iterator stru = structs.begin(); stru != structs.end(); stru++)
	{
		auto st = stru.value();
		if(st.contains("id") &&
		   st["startpos"].is_number())
		{
			map->structs[scounter].id = st["id"].get<int>();
		}
		else if(sscanf(stru.key().c_str(), "structure_%d", &map->structs[scounter].id) != 1) {
			log_warn("Error getting structure id! [%s] [%d]", stru.key().c_str(), st.contains("id"));
			log_debug("structure dump: v[%s] k[%s]", stru.value().dump().c_str(), stru.key().c_str());
		}
		else
		{
			map->structs[scounter].id = scounter;
			if(!noID)
			{
				log_error("No id field in structure JSON! Counting objects...", stru.key().c_str(), st.contains("id"));
				log_debug("structure dump: v[%s] k[%s]", stru.value().dump().c_str(), stru.key().c_str());
				noID = true;
			}
		}

		if(st.contains("startpos") && st["startpos"].is_number())
		{
			map->structs[scounter].player = st["startpos"].get<int>();
		}
		else if(st.contains("player"))
		{
			if(st["player"].is_number())
			{
				map->structs[scounter].player = st["player"].get<int>();
			}
			else
			{
				if(st["startpos"].is_string() &&
				   strcmp(st["player"].get<std::string>().c_str(), "scavenger") == 0)
				{
					log_warn("No startpos or player in structure JSON. Interpreting as scavangers.");
					map->structs[scounter].player = -1;
				}
			}
		}
		else
		{
			map->structs[scounter].player = -1;
		}
		map->structs[scounter].x = st["position"][0].get<int>();
		map->structs[scounter].y = st["position"][1].get<int>();
		map->structs[scounter].z = st["position"][2].get<int>();
		map->structs[scounter].rotation[0] = st["rotation"][0].get<int>();
		map->structs[scounter].rotation[1] = st["rotation"][1].get<int>();
		map->structs[scounter].rotation[2] = st["rotation"][2].get<int>();
		strncpy(map->structs[scounter].name, st["name"].get<std::string>().c_str(), 128);
		scounter++;
	}
	free(content);
	zip_entry_close(map->zip);
	return true;
}

bool WMT_ReadStructs(WZmap *map) {
	if(map->levelsfound<=0)
	{
		log_warn("No levels found! Searching by filename.");
		return WMT_OldReadStructs(map);
	}
	char sfname[WMT_MAX_PATH_LEN];
	snprintf(sfname, WMT_MAX_PATH_LEN, "multiplay/maps/%s/struct.json", map->levels[0].name);
	int ecode = zip_entry_open(map->zip, sfname);
	if(ecode < 0)
	{
		return WMT_OldReadStructs(map);
	}
	else
	{
		return WMT_ReadStructsJSON(map);
	}
}

bool WMT_OldReadFeaturesFile(WZmap *map) {
	bool success = true;
	int indexfeat = WMT_SearchFilename(map->filenames, map->totalentries, (char*)"feat.bjo", 2);
	int openstatus = zip_entry_openbyindex(map->zip, indexfeat);
	if(openstatus<0) {
		log_fatal("Opening file by index error! Status %d.", openstatus);
		map->errorcode = -4;
		success = false;
	} else {
		//size_t featfilesize = zip_entry_size(map->zip);
		size_t readlen;
		void *featcontents;
		ssize_t readed = zip_entry_read(map->zip, &featcontents, &readlen);
		if(readed==-1) {
			log_fatal("Error reading features file!");
		} else {
			FILE* featf = NULL;
#ifdef _WIN32
			FILE* tmpf = tmpfile();
			fwrite(featcontents, readlen, 1, tmpf);
			rewind(tmpf);
			featf = tmpf;
#else
			featf = fmemopen(featcontents, readlen, "r");
#endif
			if(featf==NULL) {
				log_fatal("Error opening as file descriptor!");
				success = false;
			} else {
				char feathead[5] = { '0', '0', '0', '0', '\0'};
				if(!WMT_ReadFromFile(featf, sizeof(char), 4, &feathead))
					log_error("Failed to read features file header!");
				if(feathead[0] != 'f' ||
				   feathead[1] != 'e' ||
				   feathead[2] != 'a' ||
				   feathead[3] != 't')
					log_warn("Features file header not \'feat\'!");
				if(!WMT_ReadFromFile(featf, sizeof(unsigned int), 1, &map->featureVersion))
					log_error("Failed to read features version!");
				if(!WMT_ReadFromFile(featf, sizeof(unsigned int), 1, &map->featuresCount))
					log_error("Failed to read number of features!");

				map->features = (WZfeature*)malloc(map->featuresCount * sizeof(WZfeature));
				if(map->features == NULL) {
					log_error("Error allocating memory for features!");
					map->errorcode = -2;
					return false;
				}
				int nameLength = 60;
				if(map->featureVersion <= 19)
					nameLength = 40;

				for(uint32_t featnum = 0; featnum<map->featuresCount; featnum++) {
					WMT_ReadFromFile(featf, sizeof(char), nameLength, &map->features[featnum].name);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].id);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].x);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].y);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].z);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].direction);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].player);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].inFire);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].burnStart);
					WMT_ReadFromFile(featf, sizeof(uint32_t), 1, &map->features[featnum].burnDamage);
					//WMT_PrintFeatureShort(map->features[featnum]);
				}

				fclose(featf);
				//printf("Features version: %d\n", map->featureVersion);
				//printf("Features count:   %d\n", map->featuresCount);
			}
			free(featcontents);
			featcontents = NULL;
		}
		zip_entry_close(map->zip);
	}
	return success;
}

bool WMT_ReadFeaturesJSON(WZmap *map) {
	size_t readlen = zip_entry_size(map->zip);
	char* content = (char*)malloc(readlen+1);
	ssize_t readed = zip_entry_noallocread(map->zip, content, readlen);
	if(readed == -1) {
		log_fatal("Error reading [%s] from archive.");
		zip_entry_close(map->zip);
		return false;
	}
	content[readlen] = '\0';
	log_debug("Reading features JSON...");
	log_debug("%d %d", readlen, zip_entry_size(map->zip));
	auto features = json::parse(content);
	map->featuresCount = features.size();
	map->features = (WZfeature*)malloc(map->featuresCount*sizeof(WZfeature));
	int fcounter = 0;
	bool wrongID = false;
	for(auto feature : features)
	{
		if(feature.contains("id"))
		{
			map->features[fcounter].id = feature["id"].get<int>();
		}
		else
		{
			map->features[fcounter].id = fcounter;
			if(!wrongID)
			{
				log_warn("Features don't have \"id\" field. Counting objects instead. Further messages suppresed.");
				log_debug("feature dump: [%s]", feature.dump().c_str());
				wrongID = true;
			}
		}

		if(feature.contains("startpos"))
		{
			map->features[fcounter].player = feature["startpos"].get<int>();
		}

		if(feature.contains("name"))
		{
			strncpy(map->features[fcounter].name, feature["name"].get<std::string>().c_str(), 128);
		}
		else
		{
			log_fatal("features JSON does not contain \"name\" field!");
			log_debug("feature dump: [%s]", feature.dump().c_str());
			free(content);
			zip_entry_close(map->zip);
			return false;
		}
		map->features[fcounter].x = feature["position"][0].get<int>();
		map->features[fcounter].y = feature["position"][1].get<int>();
		map->features[fcounter].z = feature["position"][2].get<int>();
		fcounter++;
	}
	free(content);
	zip_entry_close(map->zip);
	return true;
}

bool WMT_ReadFeaturesFile(WZmap *map) {
	if(map->levelsfound<=0) {
		log_warn("No levels found! Searching by filename.");
		return WMT_OldReadFeaturesFile(map);
	}
	char ffname[WMT_MAX_PATH_LEN];
	snprintf(ffname, WMT_MAX_PATH_LEN, "multiplay/maps/%s/feature.json", map->levels[0].name);
	int ecode = zip_entry_open(map->zip, ffname);
	if(ecode < 0) {
		return WMT_OldReadFeaturesFile(map);
	} else {
		return WMT_ReadFeaturesJSON(map);
	}
}


bool WMT_OldReadDroidsFile(WZmap *map) {
	bool success = true;
	int indexdint = WMT_SearchFilename(map->filenames, map->totalentries, (char*)"dinit.bjo", 2);
	int openstatus = zip_entry_openbyindex(map->zip, indexdint);
	if(openstatus<0) {
		log_fatal("Opening file by index error! Status %d.", openstatus);
		map->errorcode = -4;
		success = false;
	} else {
		//size_t featfilesize = zip_entry_size(map->zip);
		size_t readlen;
		void *dintcontents;
		ssize_t readed = zip_entry_read(map->zip, &dintcontents, &readlen);
		if(readed==-1) {
			log_fatal("Error reading dinit file!");
		} else {
			FILE* dintf = NULL;
#ifdef _WIN32
			FILE* tmpf = tmpfile();
			fwrite(dintcontents, readlen, 1, tmpf);
			rewind(tmpf);
			dintf = tmpf;
#else
			dintf = fmemopen(dintcontents, readlen, "r");
#endif
			if(dintf==NULL) {
				log_fatal("Error opening as file descriptor!");
				success = false;
			} else {
				char dinthead[5] = { '0', '0', '0', '0', '\0'};
				if(!WMT_ReadFromFile(dintf, sizeof(char), 4, &dinthead))
					log_error("Failed to read droids file header!");
				if(dinthead[0] != 'd' ||
				   dinthead[1] != 'i' ||
				   dinthead[2] != 'n' ||
				   dinthead[3] != 't')
					log_warn("Features file header not \'dint\'!");
				if(!WMT_ReadFromFile(dintf, sizeof(unsigned int), 1, &map->droidsVersion))
					log_error("Failed to read droids version!");
				if(!WMT_ReadFromFile(dintf, sizeof(unsigned int), 1, &map->droidsCount))
					log_error("Failed to read number of droids!");

				map->droids = (WZdroid*)malloc(map->droidsCount * sizeof(WZdroid));
				if(map->droids == NULL) {
					log_error("Error allocating memory for droids!");
					map->errorcode = -2;
					return false;
				}
				int nameLength = 60;
				if(map->droidsVersion <= 19)
					nameLength = 40;

				for(uint32_t droidnum = 0; droidnum<map->droidsCount; droidnum++) {
					WMT_ReadFromFile(dintf, sizeof(char), nameLength, &map->droids[droidnum].name);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].id);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].x);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].y);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].z);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].direction);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].player);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].inFire);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].burnStart);
					WMT_ReadFromFile(dintf, sizeof(uint32_t), 1, &map->droids[droidnum].burnDamage);
				}

				fclose(dintf);
			}
			free(dintcontents);
			dintcontents = NULL;
		}
		zip_entry_close(map->zip);
	}
	return success;
}

bool WMT_ReadDroidsJSON(WZmap *map) {
	size_t readlen = zip_entry_size(map->zip);
	char *content = (char*)malloc(readlen+1);
	ssize_t readed = zip_entry_noallocread(map->zip, content, readlen);
	if(readed == -1) {
		log_fatal("Error reading [%s] from archive.");
		zip_entry_close(map->zip);
		return false;
	}
	content[readlen] = '\0';
	log_debug("Reading droids JSON...");
	auto droids = json::parse((char*)content);
	map->droidsCount = droids.size();
	map->droids = (WZdroid*)malloc(map->droidsCount*sizeof(WZdroid));
	int dcounter = 0;
	for(auto droid : droids) {
		if(droid.contains("id"))
		{
			if(droid["id"].is_number_integer())
			{
				map->droids[dcounter].id = droid["id"].get<int>();
			}
		}
		if(droid.contains("startpos"))
		{
			if(droid["startpos"].is_string())
			{
				if(droid["startpos"].get<std::string>() == "scavenger")
				{
					map->droids[dcounter].player = -1;
				}
			}
			else if(droid["startpos"].is_number_integer())
			{
				map->droids[dcounter].player = droid["startpos"].get<int>();
			}
			else
			{
				log_error("Not valid startpos field in droid JSON!");
				map->droids[dcounter].player = -2;
			}
		}
		else if(droid.contains("player"))
		{
			if(droid["player"].is_string())
			{
				if(droid["player"].get<std::string>() == "scavenger")
				{
					map->droids[dcounter].player = -1;
				}
			}
			else if(droid["player"].is_number_integer())
			{
				map->droids[dcounter].player = droid["startpos"].get<int>();
			}
			else
			{
				log_error("Not valid player field in droid JSON!");
				map->droids[dcounter].player = -2;
			}
		}
		if(droid.contains("position"))
		{
			if(droid["position"].is_array())
			{
				if(droid["position"].size() == 3)
				{
					for(int i=0; i<3; i++)
					{
						if(droid["position"][i].is_number_integer()) {
							map->droids[dcounter].x = droid["position"][i].get<int>();
						}
						else
						{
							log_warn("Droid %d position %d is not integer! [%s]", dcounter, i, droid.dump().c_str());
						}
					}
				}
				else
				{
					if(droid["position"].size() < 3)
					log_error("No valid position! [%s]", droid.dump().c_str());
				}
			}
		}
		strncpy(map->droids[dcounter].name, droid["template"].get<std::string>().c_str(), 128);
		dcounter++;
	}
	free(content);
	zip_entry_close(map->zip);
	return true;
}

bool WMT_ReadDroidsFile(WZmap *map) {
	if(map->levelsfound<=0) {
		log_warn("No levels found! Searching by filename.");
		return WMT_OldReadDroidsFile(map);
	}
	char dfname[WMT_MAX_PATH_LEN];
	snprintf(dfname, WMT_MAX_PATH_LEN, "multiplay/maps/%s/droid.json", map->levels[0].name);
	log_error("%s", dfname);
	int ecode = zip_entry_open(map->zip, dfname);
	if(ecode < 0) {
		return WMT_OldReadDroidsFile(map);
	} else {
		return WMT_ReadDroidsJSON(map);
	}
}

void WMT_ReadMap(char* filename, WZmap *map) {
	//struct WZmap map = (struct WZmap)malloc(sizeof(struct WZmap));
	map->path=filename;
	map->zip = zip_open(map->path, 0, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 1, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 2, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 3, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 4, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 5, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 6, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 7, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 8, 'r');
	if(map->zip == NULL)
		map->zip = zip_open(map->path, 9, 'r');
	if(map->zip == NULL) {
		log_fatal("Error opening/reading warzone map file! (bad zip file)");
		map->valid=false;
		map->errorcode = -1;
		return;
	}
	if(zip_is64(map->zip) == -1) {
		log_error("WARNING! Got 64 bit zip file. This read can fail with no reason.");
		log_error(" Consult https://github.com/kuba--/zip/issues/109 for more info.");
	}
	log_debug("Listing files...");
	if(!WMT_ListFiles(map)) {
		log_fatal("Error listing map files!");
		map->valid=false;
		return;
	}
	log_debug("Reading addon...");
	if(!WMT_ReadAddonLev(map)) {
		log_fatal("Error reading addon file!");
		map->valid=false;
		return;
	}
	log_debug("Reading GAM...");
	if(!WMT_ReadGAMFile(map)) {
		log_fatal("Error reading gam file!");
		map->valid=false;
		return;
	}
	log_debug("Reading TTP...");
	if(!WMT_ReadTTypesFile(map)) {
		log_fatal("Error reading ttypes file!");
		map->valid=false;
		return;
	}
	log_debug("Reading MAP...");
	if(!WMT_ReadGameMapFile(map)) {
		log_fatal("Error reading map file!");
		map->valid=false;
		return;
	}
	log_debug("Reading droids...");
	if(!WMT_ReadDroidsFile(map)) {
		log_fatal("Error reading droid file!");
		map->valid=false;
		return;
	}
	log_debug("Reading structs...");
	if(!WMT_ReadStructs(map)) {
		log_fatal("Error reading struct file!");
		map->valid=false;
		return;
	}
	log_debug("Reading features...");
	if(!WMT_ReadFeaturesFile(map)) {
		log_fatal("Error reading features file!");
		map->valid=false;
		return;
	}
	log_info("Map reading done!");
	return;
}

void WMT_FreeMap(WZmap *map) {
	if(map->fields_clean) {
		log_warn("Prevented double clean!\n");
		return;
	}

	free(map->mapname);
	map->mapname = NULL;

	free(map->mapheight);
	map->mapheight = NULL;

	free(map->structs);
	map->structs = NULL;

	free(map->features);
	map->features = NULL;

	free(map->gamcontents);
	map->gamcontents = NULL;

	for(int i=0; i<map->totalentries; i++) {
		free(map->filenames[i]);
		map->filenames[i] = NULL;
	}
	free(map->filenames);
	map->filenames = NULL;

	zip_entry_close(map->zip);
	zip_close(map->zip);
	map->fields_clean=true;
}

const char* WMT_CheckMap(WZmap *map) {
	log_info("Checking map...");
	if(!map->valid) {
		log_error("Map not valid!");
		return "Not valid map structure passed.";
	}
	if(map->players < 2 || map->players > 11) {
		log_error("Wrong players count! (%d)", map->players);
		return "Wronf player count (must be >2 & <11)";
	}
	log_info("Check completed! Nothing wrong found!");
	return NULL;
}

/*

+  /multiplay/maps/<Level>/droid.json
+  /multiplay/maps/<Level>/feature.json
+  /multiplay/maps/<Level>/struct.json
+  /multiplay/maps/<Level>/ttypes.ttp
+  /multiplay/maps/<Level>/game.map
?  /multiplay/maps/<Level>.json
-  /multiplay/maps/<Level>.gam
+  <Level>.xplayer.lev

*/

int WMT_WriteMap(WZmap *map) { // TODO: add more sanity checks
	log_info("Writing map...");
	const char* CheckError = WMT_CheckMap(map);
	if(CheckError != NULL) {
		log_error("Map check failed! Please fix errors! (%s)", CheckError);
		return -1;
	}

	char filename[256];
	for(int i=0; i<256; i++)
		filename[i]=0;
	snprintf(filename, 256, "%s.wz", map->mapname);
	log_info("Filename \"%s\"", filename);

	struct zip_t *zip = zip_open(filename, 0, 'w');

	char xplayersfilename[256];
	for(int i=0; i<256; i++)
		xplayersfilename[i]=0;
	snprintf(xplayersfilename, 256, "%s.xplayers.lev", map->mapname);

	zip_entry_open(zip, xplayersfilename);
    char buf[16384];
	for(int i=0; i<16384; i++)
		buf[i]=0;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	snprintf(buf, 16384, "// Made with WMT\n// Date: %d/%d/%d %d:%d:%d\n// Author: %s\n// License: %s\n\n",
						 tm.tm_year + 1900,
						 tm.tm_mon + 1,
						 tm.tm_mday,
						 tm.tm_hour,
						 tm.tm_min,
						 tm.tm_sec,
						 map->createdauthor,
						 map->createdlicense);
    zip_entry_write(zip, buf, strlen(buf));
	for(unsigned int i=0; i<map->levelsfound; i++) {
		for(int i=0; i<16384; i++)
			buf[i]=0;
		snprintf(buf, 16384, "level   %s\nplayers %d\ntype    %d\ndataset %s\ngame    \"multiplay/maps/%s.gam\"\ndata    \"wrf/multi/skirmish2.wrf\"\ndata    \"wrf/multi/fog1.wrf\"\n\n",
							 map->levels[i].name,
							 map->levels[i].players,
							 map->levels[i].type,
							 map->levels[i].dataset,
							 map->mapname);
		zip_entry_write(zip, buf, strlen(buf));
	}
	zip_entry_close(zip);


	char mapfilename[256];
	for(int i=0; i<256; i++)
		mapfilename[i]=0;
	snprintf(mapfilename, 256, "multiplay/maps/%s/game.map", map->mapname);
	zip_entry_open(zip, mapfilename);
	//FIXME: Need to reparce and redone from raw values!!!
	zip_entry_write(zip, map->mapcontents, map->mapcontentslen);
	zip_entry_close(zip);

	char ttpfilename[256] = {0};
	snprintf(ttpfilename, 256, "multiplay/maps/%s/ttypes.ttp", map->mapname);
	zip_entry_open(zip, ttpfilename);
	//FIXME: Need to reparce and redone from raw values!!!
	zip_entry_write(zip, map->ttpcontents, map->ttpcontentslen);
	zip_entry_close(zip);

	char gamfilename[256] = {0};
	snprintf(gamfilename, 256, "multiplay/maps/%s.gam", map->mapname);
	zip_entry_open(zip, gamfilename);
	//FIXME: Need to reparce and redone from raw values!!!
	zip_entry_write(zip, map->gamcontents, map->gamcontentslen);
	zip_entry_close(zip);


	char mapjsonfilename[256] = {'b'};
	snprintf(mapjsonfilename, 256, "multiplay/maps/%s.json", map->mapname);
	zip_entry_open(zip, mapjsonfilename);
	char mapjsonheader[2048] = {'b'};
	snprintf(mapjsonheader, 2048, "{\n    \"map\": {\n        \"file\": \"%s\",\n        \"id\": \"map\",\n        \"maxPlayers\": %d,\n        \"name\": \"%s\"\n    },\n", map->mapname, map->players, map->mapname);
	zip_entry_write(zip, mapjsonheader, strlen(mapjsonheader)*sizeof(char));
	for(int plcounter = 0; plcounter<map->players; plcounter++) {
		char mapjsonplayerchunk[1024] = {'b'};
		snprintf(mapjsonplayerchunk, 1024, "    \"player_%d\": {\n        \"id\": \"player_%d\",\n        \"team\": %d\n    }%c\n", plcounter, plcounter, (plcounter%2==0 ? plcounter : plcounter), ( !(plcounter+1<map->players) ? ' ' : ','));
		zip_entry_write(zip, mapjsonplayerchunk, strlen(mapjsonplayerchunk)*sizeof(char));
	}
	zip_entry_write(zip, "}\n", strlen("}\n")*sizeof(char));
	zip_entry_close(zip);


	for(unsigned int i=0; i<map->numStructures; i++) {
		if(strcmp(map->structs[i].name, "A0FacMod1") == 0 ||
		   strcmp(map->structs[i].name, "A0ResearchModule1") == 0 ||
		   strcmp(map->structs[i].name, "A0PowMod1") == 0) {
			for(unsigned int j=0; j<map->numStructures; j++) {
				if(map->structs[i].x == map->structs[j].x && map->structs[i].y == map->structs[j].y && map->structs[i].z == map->structs[j].z) {
					map->structs[j].modules++;
					log_debug("Added module to %s on %d %d %d now modules %d", map->structs[j].name,
																			   map->structs[j].x,
																			   map->structs[j].y,
																			   map->structs[j].z,
																			   map->structs[j].modules);
					break;
				}
			}
			map->structs[i].oldformat = true;
		}
	}

	char structjsonfilename[256] = {'b'};
	snprintf(structjsonfilename, 256, "multiplay/maps/%s/struct.json", map->mapname);
	zip_entry_open(zip, structjsonfilename);
	zip_entry_write(zip, "{\n", _WMT_strsize("{\n"));
	for(unsigned int i=0; i<map->numStructures; i++) {
		//if(map->structs[i].oldformat == true)
		//	continue;
		char chunk[4096] = {'b'};
		char modulesstr[32] = {'b'};
		if(strcmp(map->structs[i].name, "A0LightFactory") == 0 ||
		   strcmp(map->structs[i].name, "A0VTOLFactory1") == 0 ||
		   strcmp(map->structs[i].name, "A0ResearchFacility") == 0 ||
		   strcmp(map->structs[i].name, "A0PowerGenerator") == 0) {
			snprintf(modulesstr, 32, "        \"modules\": %d,\n", map->structs[i].modules);
		} else {
			modulesstr[0] = '\0';
		}
		snprintf(chunk, 4096, "    \"structure_%d\": {\n%s        \"name\": \"%s\",\n        \"position\": [\n            %d,\n            %d,\n            %d\n        ],\n        \"rotation\": [\n            %d,\n            %d,\n            %d\n        ],\n        \"startpos\": %d\n    }%c\n",
							  map->structs[i].id,
							  modulesstr,
							  map->structs[i].name,
							  map->structs[i].x,
							  map->structs[i].y,
							  map->structs[i].z,
							  0,
							  0,
							  0,
							  map->structs[i].player,
							  i+1<map->numStructures ? ',' : ' ');
		//log_fatal("%c", i+1<map->numStructures ? ',' : ' ');
		zip_entry_write(zip, chunk, _WMT_strsize(chunk));
	}
	zip_entry_write(zip, "}\n", _WMT_strsize("}\n"));
	zip_entry_close(zip);

	char droidjsonfilename[256] = {'b'};
	snprintf(droidjsonfilename, 256, "multiplay/maps/%s/droid.json", map->mapname);
	zip_entry_open(zip, droidjsonfilename);
	zip_entry_write(zip, "{\n", _WMT_strsize("{\n"));
	for(unsigned int i=0; i<map->droidsCount; i++) {
		char chunk[4096] = {'b'};
		snprintf(chunk, 4096, "    \"droid_%d\": {\n        \"id\": %d,\n        \"position\": [\n            %d,\n            %d,\n            %d\n        ],\n        \"rotation\": [\n            %d,\n            %d,\n            %d\n        ],\n        \"startpos\": %d,\n        \"template\": \"%s\"\n    }%c\n",
							  map->droids[i].id,
							  map->droids[i].id,
							  map->droids[i].x,
							  map->droids[i].y,
							  map->droids[i].z,
							  0,
							  0,
							  0,
							  map->droids[i].player,
							  map->droids[i].name,
							  i+1<map->droidsCount ? ',' : ' ');
		zip_entry_write(zip, chunk, _WMT_strsize(chunk));
	}
	zip_entry_write(zip, "}\n", _WMT_strsize("}\n"));
	zip_entry_close(zip);

	char featurejsonfilename[256] = {'b'};
	snprintf(featurejsonfilename, 256, "multiplay/maps/%s/feature.json", map->mapname);
	zip_entry_open(zip, featurejsonfilename);
	zip_entry_write(zip, "{\n", _WMT_strsize("{\n"));
	for(unsigned int i=0; i<map->featuresCount; i++) {
		char chunk[4096] = {'b'};
		snprintf(chunk, 4096, "    \"feature_%d\": {\n        \"name\": \"%s\",\n        \"position\": [\n            %d,\n            %d,\n            %d\n        ],\n        \"rotation\": [\n            %d,\n            %d,\n            %d\n        ]\n    }%c\n",
							  map->features[i].id,
							  map->features[i].name,
							  map->features[i].x,
							  map->features[i].y,
							  map->features[i].z,
							  0,
							  0,
							  0,
							  i+1<map->featuresCount ? ',' : ' ');
		zip_entry_write(zip, chunk, _WMT_strsize(chunk));
	}
	zip_entry_write(zip, "}\n", _WMT_strsize("}\n"));
	zip_entry_close(zip);


	zip_close(zip);
	return 0;
}
