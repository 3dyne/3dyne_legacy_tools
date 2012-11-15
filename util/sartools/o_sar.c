/* 
 * 3dyne Legacy Tools GPL Source Code
 * 
 * Copyright (C) 1996-2012 Matthias C. Berger & Simon Berger.
 * 
 * This file is part of the 3dyne Legacy Tools GPL Source Code ("3dyne Legacy
 * Tools Source Code").
 *   
 * 3dyne Legacy Tools Source Code is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * 3dyne Legacy Tools Source Code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * 3dyne Legacy Tools Source Code.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * In addition, the 3dyne Legacy Tools Source Code is also subject to certain
 * additional terms. You should have received a copy of these additional terms
 * immediately following the terms and conditions of the GNU General Public
 * License which accompanied the 3dyne Legacy Tools Source Code.
 * 
 * Contributors:
 *     Matthias C. Berger (mcb77@gmx.de) - initial API and implementation
 *     Simon Berger (simberger@gmail.com) - initial API and implementation
 */



// o_sar.cc

#include<stdio.h>
#include<stdlib.h>
//#include<iomanip.h>
//#include<fstream.h>
//#include<iostream.h>
//#include<string>
//#include<vector>
#include<string.h>
#include<getopt.h>
#include<time.h>
//#include"stypes.h"

#define HASH_IN ( 56 )
#define HASH_OUT ( sizeof(unsigned long) )
#define HEAD_ID_SIZE ( 4 )
#define HEAD_TYPE_SIZE ( 4 )
#define MAX_DIR_SIZE ( 4096 )
//#define MAX_DIR_SIZE (1024)

//#define O_SAR_XMESSAGES
//
//  LOCAL GLOBALS  
//

typedef struct {
	char id[HEAD_ID_SIZE];
	char type[HEAD_TYPE_SIZE];
	unsigned int dir_ofs;
	unsigned int file_num;
	unsigned int chksum;
} head;

typedef struct {
	char file_name[56];
	unsigned int file_pos;
	unsigned int file_size;
} FileData;

static const char VERSION[]="new";

static char pn[64];
static int copy_bb;
static unsigned int buf_size;
static unsigned int time1; 

static int head_start;
static int data_start;
//static int checksum_pos;
//vector<FileData> files;
static unsigned int file_num;
static unsigned int dir_ofs;
//static unsigned long dir_end;
//static unsigned long files_size;
//static char sar_name[64];
static FileData files[MAX_DIR_SIZE];
static unsigned int name_hashed[MAX_DIR_SIZE];
static head header;
static FILE * out_handle;
static FILE * in_handle;

static unsigned int hashKey(char* t_ptr);
void drawRotor();

void error (char *text,int severity) {
	if (severity == 0) { /* warning => no exit */
		printf("\nwarning: %s\n", text);
		return;
	}
	
	if (severity == 1) { /* error => exit */
		printf("\nerror: %s", text);
		printf("\nTry '-h' or '--help' for more information.\n");
		exit(-1);
	}
}

void timeStart(void) {
	time1 = clock();
}

void timeStop(void) {
	unsigned int tit_hs = 0;
	double tit_s;
	tit_hs = clock() - time1;
	tit_s = tit_hs/100.0;
	printf("time elapsed: %.2fs\n",tit_s);
}

void printHelpText() {
	printf("usage: %s -o outfile [-nbsthV]\n", pn);
	printf("-o --outfile     name of target archiv\n");
	printf("-n --nochecksum  write no checksum to archiv\n"); 
	printf("-b --byteread    use single bytes to copy file to archiv (very slow!)\n");
	printf("-s --bufsize     size of copy-buffer in bytes (default: 16384)\n");
	printf("-t --notime      don't show time it took\n");
	printf("-h --help        print this screen\n");
	printf("-V --version     print version\n");
	printf("\nmessage: -b ,-n and -t are out of order in this version, sorry!\n");
}


	
void newSar(char *file_name) {
	out_handle = fopen(file_name, "wb");
	head_start = ftell(out_handle);
#ifdef O_SAR_XMESSAGES
	printf("%i\n", head_start);
#endif

	header.dir_ofs = 0;
	header.file_num = 0;
	header.chksum = 0;
	fseek(out_handle, sizeof(header), SEEK_SET);
}

void sarInit(char * file_name) {
	//files = vector<FileData> (0);
	//file_cntr = 0;
	if (file_name != NULL) { 
		newSar(file_name);
	} else {
		error("no filename given\n",1);
	}
	
}

void readDir() {
	unsigned int i;
	int ret;
//string token;
	// file_cntr=0;
	char token[64];
	file_num = 0;
#ifndef O_SAR_XMESSAGES
	printf("reading directory and checking for definity .");
	fflush(stdout);
#endif
	
	for (i=0;;i++) {
		//cin >> token;
		ret = scanf("%s", token);
		//printf("%s\n", token);
		if ( feof(stdin) ) 
			break;
		memset(files[i].file_name, 0, 56);
		strcpy(files[i].file_name, token);
		name_hashed[i] = hashKey(files[i].file_name);
		file_num++;
		if(file_num >= MAX_DIR_SIZE) 
		{
			printf("abort\nyou want to add too much files to an archiv\n");
			printf("recompile 'o_sar' with more 'MAX_DIR_SIZE'\n");
			exit(0);
		}
/*
		for( ii = 0; ii < i; ii++)
		{
			if( name_hashed[i] == name_hashed[ii])
			{
				printf("funny warning:\nhash codes are not definite ( who cares ?)\n");
				printf("1: hash code: %x \nfile number: %i \nfile name: %s\n\n", name_hashed[i], i, files[i].file_name);
				printf("2: hash code: %x \nfile number: %i \nfile name: %s\n\n", name_hashed[ii], ii, files[ii].file_name);
				//exit(0);
			}
		}
		*/
#ifndef O_SAR_XMESSAGES
		drawRotor();
#endif

#ifdef O_SAR_XMESSAGES
		printf("add file to list: %s\nhash code: %x\n",files[i].file_name, name_hashed[i]);
#endif
		
		
	}
#ifndef O_SAR_XMESSAGES		
	printf("\b... o.k\n");
#endif
}
	
void addFiles( int percentage ) {
	unsigned char *buf;
	
	unsigned long read_real = 0;
	unsigned long i;
	float	percent, ffilenum;
	
	char file_name[56];
	ffilenum = file_num;
	buf = (unsigned char *) malloc(buf_size);	
	data_start = ftell(out_handle);
	//cerr << file_num << endl;
	for (i = 0;i < file_num;i++) {
		unsigned int file_size = 0;
		memset(file_name, 0, sizeof(file_name)); 
		strcpy(file_name, files[i].file_name);
		//sprintf(file_name, "./%s", files[i].file_name);
		//unsigned char buf;
		files[i].file_pos = ftell(out_handle);
		if( ! percentage )
		{	
			printf("adding file: %s ", file_name);
			fflush(stdout);
		}	
		if(( in_handle = fopen(file_name, "rb")) == NULL ) {
			error("addFiles: cannot open source-file\n",1);
		}

		for (;;) {
			read_real = fread(buf, 1, buf_size,in_handle);
			file_size += read_real;
		
			
			if(read_real < buf_size) {
				fwrite(buf, read_real, 1, out_handle);
				break;
			}
			fwrite(buf, buf_size, 1, out_handle);
		}
		fclose(in_handle);
		if( percentage )
		{
			percent = (( float ) i )/ffilenum;
			percent*=100.0;
			printf( "%d\n",( int ) percent );
		} else	
			printf("%i kbytes o.k\n", file_size/1024);
		files[i].file_size=file_size;
	}
}


void writeDir() {
	//cerr << ferror(out_handle) << endl;
	printf("writing directory information .."); 
	dir_ofs = ftell(out_handle);
	fwrite(files, 64 * file_num, 1, out_handle);
      
/* 
   out_file << endl;
   //unsigned long dir_start;
   dir_start = out_file.tellp();
	for (unsigned long i = 0;i<files.size();i++) {
		out_file << files[i].file_name << ' ';
		out_file << hex << files[i].file_pos << ' ';
		out_file << hex << files[i].file_size << ' ';
	}
	dir_end = out_file.tellp();
	file_num = files.size();
	out_file.seekp(head_start);
	out_file << setw(8) << hex << dir_start;
	out_file << ' ';
	out_file << setw(8) << hex << file_num;
	*/
	printf(". ready\n");
#ifdef O_SAR_XMESSAGES
	//cerr << "head start: " << head_start << endl;
	//cerr << "data start: " << data_start << endl;
	//cerr << "dir start: " << dir_start << endl;
#endif
}


unsigned long makeChecksum( void *t_ptr, unsigned long size ) {
	unsigned char buf, buf1;
	unsigned int checksum = 0;
	unsigned int i;
	buf = ((char *)t_ptr)[0];
	for (i = 1; i < size;i++) {
		buf1 = ((char *)t_ptr)[i];
		checksum+=buf^buf1;
		buf=buf1;
	}
	//printf("checksum %i\n", checksum);
	return checksum;	
}
	

void writeHead(void) 
{
	memcpy(&header.id, "CSAR", HEAD_ID_SIZE);
	memcpy(&header.type, "2new", HEAD_TYPE_SIZE);
	header.dir_ofs = dir_ofs;
	header.file_num = file_num;
	header.chksum = makeChecksum(files, 64 * file_num);
	fseek(out_handle, 0, SEEK_SET);
	fwrite(&header, sizeof(header), 1, out_handle);
	printf("closing archiv\n");
	printf("checksum: 0x%x\n", header.chksum);
	fclose(out_handle);
}


static unsigned int hashKey(char* t_ptr) {
	unsigned long hash_frag = 0;
	unsigned long hash_key = 0;
	unsigned int i;
	char tmp[HASH_IN];
	memcpy(tmp, t_ptr, HASH_IN);
	for(i=0; i<( HASH_IN / HASH_OUT ); i++) {
		memcpy( &hash_frag, t_ptr, HASH_OUT );
		//printf("%d \n", hash_frag);
		t_ptr += HASH_OUT;
		//hash_key = hash_key * hash_frag;		
		//hash_key += (hash_key << 8) ^ crctable[(char(hash_key >> 8)) ^ hash_frag];
		hash_key = hash_key ^ (hash_frag + i);
	}
	return hash_key ^ strlen(tmp);
}	

void drawRotor(void) 
{
	static int wm_ctr = 0;
	static char wm_anm[] = {
		'/', '-', '\\', '|'
	};
  
	//printf("%i\n", wm_ctr); 
	printf( "\b%c", wm_anm[wm_ctr]);
	fflush(stdout);
	wm_ctr++;
	if(wm_ctr > 3)
		wm_ctr = 0;
}

int main(int argc,char *argv[]) {
	int c_opt;
	//unsigned long checksum;
	
	char out_file[64];
	char write_file[64];
	int out_file_force = 1;
	int checksum_make = 1;
	int show_time = 1;
	int absolute = 0;
	int percentage = 0;
	//true = 1;
	//false = 0;
	
	struct option long_opts[] =
	{
		{"outfile",1,0,'i'},
		{"absolute",1,0,'a'},
		{"percentage",0,0,'p'},
		{"nochecksum",0,0,'n'},
		{"byteread",0,0,'b'},
		{"bufsize",0,0,'s'},
		{"notime",0,0,'t'},
		{"help",0,0,'h'},
		{"version",0,0,'v'},
		{0,0,0,0}
	};

	strcpy(pn, argv[0]);
	copy_bb = 1;
	buf_size = 16384;

	if (argc == 1) {
		error("too few arguments.",1);
	}
	
	
	while ((c_opt=getopt_long(argc,argv,"o:apnbs:thv",long_opts,(int *) 0)) !=EOF)
		switch(c_opt) {
		case 'o':
			memset(out_file, 0, sizeof(out_file));
			strcpy(out_file, optarg);
			out_file_force = 0;
			break;
		case 'a':
			absolute = 1;
			break;	
		
		case 'p':
			percentage = 1;
			break;			

		case 'n':
			checksum_make = 0;
			break;
			
		case 'b':
			copy_bb = 0;
			break;
			
		case 's':
			buf_size = atoi(optarg);
			printf("new buf-size is: %i\n", buf_size);
			break;
			
		case 't':
			show_time = 0;
			break;

		case 'h':
			printHelpText();
			exit(0);
			
		case 'v':
			printf("%s VERSION\n", pn);
			exit(0);
			
		case 0:
			error (" ",1);
			exit(-1);
			
		case 63:
			error (" ",1);
			exit(-1);
			
		}
	
	if (out_file_force) {
		error("you gave me no filename",1);
	}
	
	if (!copy_bb) {
		error("you should use the big buf\nit's much faster",0); 
	}
	
     
	timeStart();
	
	if( absolute )
	{
		sprintf( write_file, "%s", out_file );
	} else
	{
		sprintf(write_file, "../%s", out_file); 
	}
	printf("archiv: %s\n", write_file);
	sarInit(write_file);
	readDir();
	addFiles( percentage );
	writeDir();
	writeHead();
	
      
	timeStop();
	exit(0);
}
