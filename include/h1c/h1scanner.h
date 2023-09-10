#ifndef H1SCANNER_H
#define H1SCANNER_H

#include <stdio.h>

#include "h1c/h1consts.h"
#include "h1c/reqinfo.h"
#include "basicio/buffers.h"
#include "basicio/sockets.h"

/** Enums */

/**
 * @brief Defines all possible state codes that the HttpScanner will use.
 */
typedef enum http_scanner_state_e
{
    START,       // idle before scanning
    EAT_METHOD,  // process HTTP method verb
    EAT_URL,     // process URL string
    EAT_SCHEMA,  // process HTTP schema
    EAT_HEADER,   // skip or process header
    EAT_BLOB,    // skip or process body
    STOP,        // finish the current request scan
    ERROR        // signals 400 Bad Request error to server!
} HttpScannerState;

/**
 * @brief Models a very crude finite state machine to read and parse HTTP/1.x requests.
 */
typedef struct h1scanner_t
{
    HttpScannerState state;      // operation current scanning
    ClientSocket *cli_sock_ref;  // reference to readable client stream
    bool buffers_ok;             // whether I/O buffers are allocated or not
    Buffer header_buf;
    Buffer body_buf;
} HttpScanner;

/** Helper Funcs. */

void h1scanner_init(HttpScanner *scanner, ClientSocket *cli_sock);
void h1scanner_dispose(HttpScanner *scanner);
void h1scanner_reset(HttpScanner *scanner);
bool h1scanner_is_ready(const HttpScanner *scanner);
HttpScannerState h1scanner_method(HttpScanner *scanner, BaseRequest *req_ref);
HttpScannerState h1scanner_url(HttpScanner *scanner, BaseRequest *req_ref);
HttpScannerState h1scanner_schema(HttpScanner *scanner, BaseRequest *req_ref);
HttpScannerState h1scanner_header(HttpScanner *scanner, BaseRequest *req_ref);
HttpScannerState h1scanner_eat_blob(HttpScanner *scanner, BaseRequest *req_ref);
bool h1scanner_read_reqinfo(HttpScanner *scanner, BaseRequest *req_ref);

#endif