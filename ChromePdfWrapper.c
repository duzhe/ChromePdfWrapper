#define UNICODE
#include <windows.h>
#include <winuser.h>
#include <stdio.h>

#define CHROME_PATH "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"

#define MAX_PATHW 32767
static inline char ToHex(unsigned char x) 
{
    return x>9 ? x-10+'A' : x+'0';
}
/*
 * encoding a filename ento url
 * pszDest: out, encoded url;
 * cbDest : in,  buffer size of pszDest, in byte;
 * in     : in,  filename, in utf-8, '\0' terminated;  
 */
static const char* FilenameEncodeUrl(char * pszDest, size_t cbDest, const char * in)
{
    size_t w = 0;
    size_t n = strlen(in);
    
    w += snprintf(pszDest, cbDest, "file:///");
    for (size_t i=0; i<n; i++) {
        if (in[i] <= 0x7F) {
            if (w+1 > cbDest) return NULL;
            if (in[i] == '\\' || in[i] == '/') {
                pszDest[w++] = '/';
            } else if (isalnum(in[i]) || 
                in[i] == '*' ||
                in[i] == '-' ||
                in[i] == '.' ||
                in[i] == '_') {
                pszDest[w++] = in[i];
            } else if (in[i] == ':') {  // here a little nonstandard
                pszDest[w++] = in[i];
            } else {
                if (w+3 > cbDest) return NULL;
                pszDest[w++] = '%';
                pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
                pszDest[w++] = ToHex((unsigned char)in[i] % 16);
            }
        } else if (in[i] >= 0xC2 && in[i] <= 0xDF) {  //UTF8-2
            if (w+6 > cbDest) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
            if (++i >= n) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
        } else if (in[i] >= 0xE0 && in[i] <= 0xEF) {  //UTF8-3
            if (w+9 > cbDest) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
            if (++i >= n) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
            if (++i >= n) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
        } else if (in[i] >= 0xF0 && in[i] <= 0xF4) {  // UTF8-4
            if (w+12 > cbDest) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
            if (++i >= n) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
            if (++i >= n) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
            if (++i >= n) return NULL;
            pszDest[w++] = '%';
            pszDest[w++] = ToHex((unsigned char)in[i] >> 4);
            pszDest[w++] = ToHex((unsigned char)in[i] % 16);
        }
    }
    if (w+1 > cbDest) return NULL;
    pszDest[w++] = '\0'; 
    return pszDest;
}


int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    int nArgs;
    LPWSTR *szArglist;
    char asutf8[MAX_PATHW];
    char url[MAX_PATHW];
    char param[MAX_PATHW];

    (void)cmdline;
    (void)hInst;
    (void)hInstPrev;
    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (nArgs != 2) {
        MessageBoxA(NULL, "Accept exact one parameter (filename)", "Invalid run", MB_ICONERROR);
        return 0;
    }
    WideCharToMultiByte(CP_UTF8, WC_COMPOSITECHECK, szArglist[1], -1, asutf8, sizeof(asutf8), NULL, NULL);
    if (FilenameEncodeUrl(url, sizeof(url), asutf8) == NULL) {
        MessageBox(NULL, TEXT(""), TEXT("FilenameEncodeUrl failed"), MB_ICONERROR);
        return 0;
    }
    
    snprintf(param, sizeof(param)/sizeof(param[0]), "--app=%s", url);

    if (!ShellExecuteA(NULL, "open", CHROME_PATH, param, NULL, cmdshow)) {
        MessageBoxA(NULL, param, "ShellExecuteA failed", MB_ICONERROR);
    }
    return 0;
}