/*
** A very simple http server.
**
** Just want to learn some socket programming and http protocol.
** use althttpd by D. Richard Hipp as my tutorial.
** https://sqlite.org/althttpd/file?name=althttpd.c&ci=tip
**
** TODO:
** use cmd argument to specify port
** GET request
** transfer other file
** use fork() to handle connections
*/

#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifndef DEFAULT_PORT
#define DEFAULT_PORT 80
#endif
#ifndef MAX_CONTENT_LENGTH
#define MAX_CONTENT_LENGTH 20000000
#endif

static char *zMethod = 0;
static char *zScript = 0;
static char *zRequestUri = 0;

typedef struct MimeTypeDef {
    const char *zSuffix;
    unsigned char size;
    const char *zMimetype;
} MimeTypeDef;

const MimeTypeDef *GetMimeType(const char *zName, int nName)
{
    const char *z;
    int i;
    int first, last;
    int len;
    char zSuffix[20];

    static const MimeTypeDef aMime[] = {
        { "ai", 2, "application/postscript" },
        { "aif", 3, "audio/x-aiff" },
        { "aifc", 4, "audio/x-aiff" },
        { "aiff", 4, "audio/x-aiff" },
        { "arj", 3, "application/x-arj-compressed" },
        { "asc", 3, "text/plain" },
        { "asf", 3, "video/x-ms-asf" },
        { "asx", 3, "video/x-ms-asx" },
        { "au", 2, "audio/ulaw" },
        { "avi", 3, "video/x-msvideo" },
        { "bat", 3, "application/x-msdos-program" },
        { "bcpio", 5, "application/x-bcpio" },
        { "bin", 3, "application/octet-stream" },
        { "c", 1, "text/plain" },
        { "cc", 2, "text/plain" },
        { "ccad", 4, "application/clariscad" },
        { "cdf", 3, "application/x-netcdf" },
        { "class", 5, "application/octet-stream" },
        { "cod", 3, "application/vnd.rim.cod" },
        { "com", 3, "application/x-msdos-program" },
        { "cpio", 4, "application/x-cpio" },
        { "cpt", 3, "application/mac-compactpro" },
        { "csh", 3, "application/x-csh" },
        { "css", 3, "text/css" },
        { "dcr", 3, "application/x-director" },
        { "deb", 3, "application/x-debian-package" },
        { "dir", 3, "application/x-director" },
        { "dl", 2, "video/dl" },
        { "dms", 3, "application/octet-stream" },
        { "doc", 3, "application/msword" },
        { "drw", 3, "application/drafting" },
        { "dvi", 3, "application/x-dvi" },
        { "dwg", 3, "application/acad" },
        { "dxf", 3, "application/dxf" },
        { "dxr", 3, "application/x-director" },
        { "eps", 3, "application/postscript" },
        { "etx", 3, "text/x-setext" },
        { "exe", 3, "application/octet-stream" },
        { "ez", 2, "application/andrew-inset" },
        { "f", 1, "text/plain" },
        { "f90", 3, "text/plain" },
        { "fli", 3, "video/fli" },
        { "flv", 3, "video/flv" },
        { "gif", 3, "image/gif" },
        { "gl", 2, "video/gl" },
        { "gtar", 4, "application/x-gtar" },
        { "gz", 2, "application/x-gzip" },
        { "hdf", 3, "application/x-hdf" },
        { "hh", 2, "text/plain" },
        { "hqx", 3, "application/mac-binhex40" },
        { "h", 1, "text/plain" },
        { "htm", 3, "text/html" },
        { "html", 4, "text/html" },
        { "ice", 3, "x-conference/x-cooltalk" },
        { "ief", 3, "image/ief" },
        { "iges", 4, "model/iges" },
        { "igs", 3, "model/iges" },
        { "ips", 3, "application/x-ipscript" },
        { "ipx", 3, "application/x-ipix" },
        { "jad", 3, "text/vnd.sun.j2me.app-descriptor" },
        { "jar", 3, "application/java-archive" },
        { "jpeg", 4, "image/jpeg" },
        { "jpe", 3, "image/jpeg" },
        { "jpg", 3, "image/jpeg" },
        { "js", 2, "application/x-javascript" },
        { "kar", 3, "audio/midi" },
        { "latex", 5, "application/x-latex" },
        { "lha", 3, "application/octet-stream" },
        { "lsp", 3, "application/x-lisp" },
        { "lzh", 3, "application/octet-stream" },
        { "m", 1, "text/plain" },
        { "m3u", 3, "audio/x-mpegurl" },
        { "man", 3, "application/x-troff-man" },
        { "me", 2, "application/x-troff-me" },
        { "mesh", 4, "model/mesh" },
        { "mid", 3, "audio/midi" },
        { "midi", 4, "audio/midi" },
        { "mif", 3, "application/x-mif" },
        { "mime", 4, "www/mime" },
        { "movie", 5, "video/x-sgi-movie" },
        { "mov", 3, "video/quicktime" },
        { "mp2", 3, "audio/mpeg" },
        { "mp2", 3, "video/mpeg" },
        { "mp3", 3, "audio/mpeg" },
        { "mpeg", 4, "video/mpeg" },
        { "mpe", 3, "video/mpeg" },
        { "mpga", 4, "audio/mpeg" },
        { "mpg", 3, "video/mpeg" },
        { "ms", 2, "application/x-troff-ms" },
        { "msh", 3, "model/mesh" },
        { "nc", 2, "application/x-netcdf" },
        { "oda", 3, "application/oda" },
        { "ogg", 3, "application/ogg" },
        { "ogm", 3, "application/ogg" },
        { "pbm", 3, "image/x-portable-bitmap" },
        { "pdb", 3, "chemical/x-pdb" },
        { "pdf", 3, "application/pdf" },
        { "pgm", 3, "image/x-portable-graymap" },
        { "pgn", 3, "application/x-chess-pgn" },
        { "pgp", 3, "application/pgp" },
        { "pl", 2, "application/x-perl" },
        { "pm", 2, "application/x-perl" },
        { "png", 3, "image/png" },
        { "pnm", 3, "image/x-portable-anymap" },
        { "pot", 3, "application/mspowerpoint" },
        { "ppm", 3, "image/x-portable-pixmap" },
        { "pps", 3, "application/mspowerpoint" },
        { "ppt", 3, "application/mspowerpoint" },
        { "ppz", 3, "application/mspowerpoint" },
        { "pre", 3, "application/x-freelance" },
        { "prt", 3, "application/pro_eng" },
        { "ps", 2, "application/postscript" },
        { "qt", 2, "video/quicktime" },
        { "ra", 2, "audio/x-realaudio" },
        { "ram", 3, "audio/x-pn-realaudio" },
        { "rar", 3, "application/x-rar-compressed" },
        { "ras", 3, "image/cmu-raster" },
        { "ras", 3, "image/x-cmu-raster" },
        { "rgb", 3, "image/x-rgb" },
        { "rm", 2, "audio/x-pn-realaudio" },
        { "roff", 4, "application/x-troff" },
        { "rpm", 3, "audio/x-pn-realaudio-plugin" },
        { "rtf", 3, "application/rtf" },
        { "rtf", 3, "text/rtf" },
        { "rtx", 3, "text/richtext" },
        { "scm", 3, "application/x-lotusscreencam" },
        { "set", 3, "application/set" },
        { "sgml", 4, "text/sgml" },
        { "sgm", 3, "text/sgml" },
        { "sh", 2, "application/x-sh" },
        { "shar", 4, "application/x-shar" },
        { "silo", 4, "model/mesh" },
        { "sit", 3, "application/x-stuffit" },
        { "skd", 3, "application/x-koan" },
        { "skm", 3, "application/x-koan" },
        { "skp", 3, "application/x-koan" },
        { "skt", 3, "application/x-koan" },
        { "smi", 3, "application/smil" },
        { "smil", 4, "application/smil" },
        { "snd", 3, "audio/basic" },
        { "sol", 3, "application/solids" },
        { "spl", 3, "application/x-futuresplash" },
        { "src", 3, "application/x-wais-source" },
        { "step", 4, "application/STEP" },
        { "stl", 3, "application/SLA" },
        { "stp", 3, "application/STEP" },
        { "sv4cpio", 7, "application/x-sv4cpio" },
        { "sv4crc", 6, "application/x-sv4crc" },
        { "svg", 3, "image/svg+xml" },
        { "swf", 3, "application/x-shockwave-flash" },
        { "t", 1, "application/x-troff" },
        { "tar", 3, "application/x-tar" },
        { "tcl", 3, "application/x-tcl" },
        { "tex", 3, "application/x-tex" },
        { "texi", 4, "application/x-texinfo" },
        { "texinfo", 7, "application/x-texinfo" },
        { "tgz", 3, "application/x-tar-gz" },
        { "tiff", 4, "image/tiff" },
        { "tif", 3, "image/tiff" },
        { "tr", 2, "application/x-troff" },
        { "tsi", 3, "audio/TSP-audio" },
        { "tsp", 3, "application/dsptype" },
        { "tsv", 3, "text/tab-separated-values" },
        { "txt", 3, "text/plain" },
        { "unv", 3, "application/i-deas" },
        { "ustar", 5, "application/x-ustar" },
        { "vcd", 3, "application/x-cdlink" },
        { "vda", 3, "application/vda" },
        { "viv", 3, "video/vnd.vivo" },
        { "vivo", 4, "video/vnd.vivo" },
        { "vrml", 4, "model/vrml" },
        { "vsix", 4, "application/vsix" },
        { "wasm", 4, "application/wasm" },
        { "wav", 3, "audio/x-wav" },
        { "wax", 3, "audio/x-ms-wax" },
        { "wiki", 4, "application/x-fossil-wiki" },
        { "wma", 3, "audio/x-ms-wma" },
        { "wmv", 3, "video/x-ms-wmv" },
        { "wmx", 3, "video/x-ms-wmx" },
        { "wrl", 3, "model/vrml" },
        { "wvx", 3, "video/x-ms-wvx" },
        { "xbm", 3, "image/x-xbitmap" },
        { "xlc", 3, "application/vnd.ms-excel" },
        { "xll", 3, "application/vnd.ms-excel" },
        { "xlm", 3, "application/vnd.ms-excel" },
        { "xls", 3, "application/vnd.ms-excel" },
        { "xlw", 3, "application/vnd.ms-excel" },
        { "xml", 3, "text/xml" },
        { "xpm", 3, "image/x-xpixmap" },
        { "xwd", 3, "image/x-xwindowdump" },
        { "xyz", 3, "chemical/x-pdb" },
        { "zip", 3, "application/zip" },
    };

    for (i = nName - 1; i > 0 && zName[i] != '.'; i--) { }
    z = &zName[i + 1];
    len = nName - i;
    if (len < (int)sizeof(zSuffix) - 1) {
        strcpy(zSuffix, z);
        for (i = 0; zSuffix[i]; i++)
            zSuffix[i] = tolower(zSuffix[i]);
        first = 0;
        last = sizeof(aMime) / sizeof(aMime[0]);
        while (first <= last) {
            int c;
            i = (first + last) / 2;
            c = strcmp(zSuffix, aMime[i].zSuffix);
            if (c == 0)
                return &aMime[i];
            if (c < 0) {
                last = i - 1;
            } else {
                first = i + 1;
            }
        }
    }
    return 0;
}

static void sigHandler(int);

static char *GetFirstElement(char *zInput, char **zLeftOver)
{
    char *zResult = 0;
    if (zInput == 0) {
        if (zLeftOver) {
            *zLeftOver = 0;
        }
        return 0;
    }
    while (isspace(*(unsigned char *)zInput)) {
        zInput++;
    }
    zResult = zInput;
    while (*zInput && !isspace(*(unsigned char *)zInput)) {
        zInput++;
    }
    if (*zInput) {
        *zInput = 0;
        zInput++;
        while (isspace(*(unsigned char *)zInput)) {
            zInput++;
        }
    }
    if (zLeftOver) {
        *zLeftOver = zInput;
    }
    return zResult;
}

static int sendFile();

void ProcessOneRequest()
{
    char *z;
    int file_fd;
    char zLine[10000];
    int rc;
    char buffer[10000];
    struct stat statbuf;

    fgets(zLine, sizeof(zLine), stdin);
    zMethod = GetFirstElement(zLine, &z);

    if (strcmp(zMethod, "GET")) {
        printf("HTTP/1.1 501 Not Implemented");
        exit(3);
    }

    zScript = GetFirstElement(z, &z);

    if ((file_fd = open(strcmp(zScript, "/") ? zScript + 1 : "index.html", O_RDONLY)) == -1) {
        printf("HTTP/1.1 400 Not found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Not found</h1></body></html>\r\n");
        exit(3);
    } else {
        fputs("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", stdout);
    }
    stat(strcmp(zScript, "/") ? zScript + 1 : "index.html", &statbuf);
    sendfile(fileno(stdout), file_fd, 0, statbuf.st_size);
    close(file_fd);

    exit(0);
}

static int http_server(int mnPort, int mxPort)
{
    int listener;
    int connection;
    int child;
    int opt = 1;
    struct sockaddr_in inaddr;
    socklen_t len;
    int iPort = mnPort;

    while (iPort < mxPort) {
        memset(&inaddr, 0, sizeof(inaddr));
        inaddr.sin_family = AF_INET;
        inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // only listening on localhost
        // inaddr.sin_addr.s_addr = htonl(INADDR_ANY); // listening all
        inaddr.sin_port = htons(iPort);

        if ((listener = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            iPort++;
            close(listener);
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(listener, (struct sockaddr *)&inaddr, sizeof(inaddr)) < 0) {
            iPort++;
            close(listener);
            continue;
        }
        break;
    }

    if (listen(listener, 20) < 0) {
    }
    printf("Listening on port %d\n", iPort);
    len = sizeof(inaddr);

    while (1) {
        connection = accept(listener, (struct sockaddr *)&inaddr, &len);

        if (connection >= 0) {
            child = fork();
            if (child != 0) {
                close(connection);
            } else {
                int fd;
                close(0);
                close(1);
                fd = dup(connection);
                fd = dup(connection);
                close(connection);
                close(listener);
                ProcessOneRequest();
            }
        }
    }
}

int main()
{
    int mnPort = 0;
    int mxPort = 0;

    signal(SIGINT, sigHandler);
    signal(SIGCHLD, SIG_IGN);

    /* TODO: parse cmd line arg */
    if (mnPort == 0) {
        mnPort = 8080;
        mxPort = 8100;
    }
    http_server(mnPort, mxPort);
    return 0;
}

void sigHandler(int sig)
{
    printf("\rExiting...\n");
    exit(0);
}
