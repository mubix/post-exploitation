/*
    Copyright 2012 Luigi Auriemma

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl-2.0.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "show_dump.h"

#ifdef WIN32
    #include <windows.h>
#else
    #define strnicmp    strncasecmp
#endif

typedef unsigned char   u8;



#define VER     "0.1"



int cstring(u8 *input, u8 *output, int maxchars, int *inlen);
u8 *base64_encode(u8 *data, int *size);
u8 *byte2hex(u8 *data, int *size);
int find(u8 *fname, u8 *buff, int buffsz, u8 *string, int stringsz, int adiacent, int case_sensitive);
int mymemicmp(u8 *a, u8 *b, int len);
u8 *fdload(u8 *fname, int *fsize);
void std_err(void);



int     g_sparse    = 0,
        g_sensitive = 0,
        verbose     = 0;
int     g_stringsz  = 0;
u8      *g_string   = NULL;



#include "scan.h"
int mygrep(u8 *fname);



int main(int argc, char *argv[]) {
    static  u8  filedir[4096];
    files_t *files;
    unsigned    found;
    int         i,
                input_total_files;

    fputs("\n"
        "mygrep " VER "\n"
        "by Luigi Auriemma\n"
        "e-mail: aluigi@autistici.org\n"
        "web:    aluigi.org\n"
        "\n", stderr);

    if(argc < 3) {
        printf("\n"
            "Usage: %s [options] <string> <file/folder> ...\n"
            "\n"
            "Options:\n"
            "-I     case sensitive only\n"
            "-a     allow the adiacent and sparse search too\n"
            "-v     verbose\n"
            "\n", argv[0]);
        exit(1);
    }

    for(i = 1; i < argc; i++) {
        if(((argv[i][0] != '-') && (argv[i][0] != '/')) || (strlen(argv[i]) != 2)) {
            break;
            //printf("\nError: wrong argument (%s)\n", argv[i]);
            //exit(1);
        }
        switch(argv[i][1]) {
            case 'I': g_sensitive   = 1;    break;
            case 'a': g_sparse      = 1;    break;
            case 'v': verbose       = 1;    break;
            default: {
                printf("\nError: wrong argument (%s)\n", argv[i]);
                exit(1);
            }
        }
    }

    g_string = argv[i++];
    if(i >= argc) {
        printf("\nError: invalid options or missing arguments\n");
        exit(1);
    }
    g_string = strdup(g_string);    // not necessary
    g_stringsz = cstring(g_string, g_string, -1, NULL);

    for(; i < argc; i++) {
        sprintf(filedir, "%.*s", sizeof(filedir) - 1, argv[i]);
        if(verbose) printf("- %s\n", filedir);

        if(recursive_dir(filedir, sizeof(filedir)) < 0) {
            // it's a file
            add_files(filedir, 0, NULL);
        }
    }
    files = add_files(NULL, 0, &input_total_files);

    found = 0;
    for(i = 0; i < input_total_files; i++) {
        found += mygrep(files[i].name);
    }

    fprintf(stderr, "\n- %u patterns found\n", found);

    for(i = 0; i < input_total_files; i++) {
        free(files[i].name);
    }
    free(files);
    return(0);
}



int mygrep(u8 *fname) {
    int     i,
            tmpsz,
            buffsz,
            string16sz,
            found   = 0;
    u8      *buff,
            *string16,
            *tmp;

    buff = fdload(fname, &buffsz);
    //if(!buff) std_err();
    if(!buff) { // useful to continue the scanning
        perror("\nError");
        return(0);
    }


    /*******************\
    |* provided string *|
    \*******************/

    found += find(fname, buff, buffsz, g_string, g_stringsz, 0, g_sensitive);


    /****************\
    |* utf16 string *|
    \****************/

    string16 = calloc(g_stringsz, sizeof(short));
    if(!string16) std_err();
    for(i = 0; i < g_stringsz; i++) {
        string16[i * 2]       = g_string[i];
        string16[(i * 2) + 1] = 0;
    }
    string16sz = (i * 2) - 1;   // big/little endian compatible
    found += find(fname, buff, buffsz, string16, string16sz, 0, g_sensitive);


    /**********\
    |* base64 *|
    \**********/

    tmpsz = g_stringsz;
    tmp = base64_encode(g_string, &tmpsz);
    found += find(fname, buff, buffsz, tmp, tmpsz, 0, g_sensitive);
    free(tmp);

    tmpsz = string16sz;
    tmp = base64_encode(string16, &tmpsz);
    found += find(fname, buff, buffsz, tmp, tmpsz, 0, g_sensitive);
    free(tmp);


    /*******\
    |* hex *|
    \*******/

    tmpsz = g_stringsz;
    tmp = byte2hex(g_string, &tmpsz);
    found += find(fname, buff, buffsz, tmp, tmpsz, 0, 0);   // case insensitive
    free(tmp);

    tmpsz = string16sz;
    tmp = byte2hex(string16, &tmpsz);
    found += find(fname, buff, buffsz, tmp, tmpsz, 0, 0);   // case insensitive
    free(tmp);


    if(g_sparse) {
        /************\
        |* adiacent *|
        \************/

        found += find(fname, buff, buffsz, g_string, g_stringsz, 10, g_sensitive);


        /**********\
        |* sparse *|
        \**********/

        found += find(fname, buff, buffsz, g_string, g_stringsz, -256, g_sensitive);
    }

    free(string16);
    free(buff);
    return(found);
}



int cstring(u8 *input, u8 *output, int maxchars, int *inlen) {
    int     n,
            len;
    u8      *p,
            *o;

    if(!input || !output) {
        if(inlen) *inlen = 0;
        return(0);
    }

    p = input;
    o = output;
    while(*p) {
        if(maxchars >= 0) {
            if((o - output) >= maxchars) break;
        }
        if(*p == '\\') {
            p++;
            switch(*p) {
                case 0:  return(-1); break;
                //case '0':  n = '\0'; break;
                case 'a':  n = '\a'; break;
                case 'b':  n = '\b'; break;
                case 'e':  n = '\e'; break;
                case 'f':  n = '\f'; break;
                case 'n':  n = '\n'; break;
                case 'r':  n = '\r'; break;
                case 't':  n = '\t'; break;
                case 'v':  n = '\v'; break;
                case '\"': n = '\"'; break;
                case '\'': n = '\''; break;
                case '\\': n = '\\'; break;
                case '?':  n = '\?'; break;
                case '.':  n = '.';  break;
                case ' ':  n = ' ';  break;
                case 'x': {
                    //n = readbase(p + 1, 16, &len);
                    //if(len <= 0) return(-1);
                    if(sscanf(p + 1, "%02x%n", &n, &len) != 1) return(-1);
                    if(len > 2) len = 2;
                    p += len;
                    } break;
                default: {
                    //n = readbase(p, 8, &len);
                    //if(len <= 0) return(-1);
                    if(sscanf(p, "%3o%n", &n, &len) != 1) return(-1);
                    if(len > 3) len = 3;
                    p += (len - 1); // work-around for the subsequent p++;
                    } break;
            }
            *o++ = n;
        } else {
            *o++ = *p;
        }
        p++;
    }
    *o = 0;
    len = o - output;
    if(inlen) *inlen = p - input;
    return(len);
}



u8 *byte2hex(u8 *data, int *size) {
    int     i,
            len;
    u8      *ret    = NULL;

    if(!data) data = "";
    if(size) len = *size;
    else     len = strlen(data);

    ret = malloc((len * 2) + 1);
    if(!ret) std_err();
    for(i = 0; i < len; i++) {
        sprintf(ret + (i * 2), "%02x", data[i]);
    }
    if(size) *size = strlen(ret);
    return(ret);
}



u8 *base64_encode(u8 *data, int *size) {
    int     len,
            a,
            b,
            c;
    u8      *buff,
            *p;
    static const u8 base[64] = {
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
        'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
        'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
        'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
    };

    if(!size || (*size < 0)) {      // use size -1 for auto text size!
        len = strlen(data);
    } else {
        len = *size;
    }
    buff = malloc(((len / 3) << 2) + 6);
    if(!buff) return(NULL);

    p = buff;
    do {
        a = data[0];
        b = data[1];
        c = data[2];
        *p++ = base[(a >> 2) & 63];
        *p++ = base[(((a &  3) << 4) | ((b >> 4) & 15)) & 63];
        *p++ = base[(((b & 15) << 2) | ((c >> 6) &  3)) & 63];
        *p++ = base[c & 63];
        data += 3;
        len  -= 3;
    } while(len > 0);
    for(*p = 0; len < 0; len++) *(p + len) = '=';

    if(size) *size = p - buff;
    return(buff);
}



int find(u8 *fname, u8 *buff, int buffsz, u8 *string, int stringsz, int adiacent, int case_sensitive) {
    int     i,
            x,
            res,
            found   = 0;
    u8      *p,
            *l,
            *s;

    if(verbose) {
        fputc('\n', stdout);
        show_dump(string, stringsz, stdout);
        fputc('\n', stdout);
    }

    l = buff + (buffsz - stringsz);

    if(adiacent) {
        for(p = buff; p <= l; p++) {
            if(p[0] != string[0]) continue;
            s = p;
            for(i = 0; i < stringsz; i++) {
                if(adiacent < 0) {
                    if((s + adiacent) < buff) break;
                    if((s - adiacent) >= l) break;
                    for(x = 0; x < -adiacent; x++) {
                        if((s[x]) == string[i]) {
                            s += x;
                            break;
                        }
                        if((s[-x]) == string[i]) break;
                    }
                    if(x >= -adiacent) break;
                } else {
                    for(x = 0; x < adiacent; x++) {
                        if((*s++) == string[i]) break;
                    }
                    if(x >= adiacent) break;
                }
            }
            if(i >= stringsz) {
                printf("  %08x %s\n", p - buff, fname);
                found++;
                if(adiacent < 0) p = s;
            }
        }
    } else {
        for(p = buff; p <= l; p++) {
            if(case_sensitive) {
                res = memcmp(p, string, stringsz);
            } else {
                res = mymemicmp(p, string, stringsz);
            }
            if(!res) {
                printf("  %08x %s\n", p - buff, fname);
                found++;
            }
        }
    }
    return(found);
}



int mymemicmp(u8 *a, u8 *b, int len) {
    int     i;

    for(i = 0; i < len; i++) {
        if(tolower(a[i]) != tolower(b[i])) return(-1);
    }
    return(0);
}



u8 *fdload(u8 *fname, int *fsize) {
    struct stat xstat;
    FILE    *fd;
    int     size;
    u8      *buff;

    if(!fname) return(NULL);
    if(verbose) fprintf(stdout, "\n- %s\n", fname);
    fd = fopen(fname, "rb");
    if(!fd) return(NULL);
    fstat(fileno(fd), &xstat);
    size = xstat.st_size;
    if(size == -1) size = -2;   // lame, impossible case
    buff = malloc(size + 1);
    if(buff) {
        fread(buff, 1, size, fd);
        buff[size] = 0;
    } else {
        size = 0;
    }
    fclose(fd);
    if(fsize) *fsize = size;
    if(verbose) fprintf(stdout, "- %u bytes loaded\n", size);
    return(buff);
}



void std_err(void) {
    perror("\nError");
    exit(1);
}

