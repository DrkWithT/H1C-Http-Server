/**
 * @file main.c
 * @author Derek Tan
 * @brief Implements driver code for the "H1C" server. Now this should use a producer-worker thread model.
 * @version 0.3.0
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <signal.h>
#include "server/core.h"

/* Constants and Helper Macros */

#define WWW_FILE_COUNT 2

static ServerDriver server;
static int server_wthrd_count = 0;
static const char *www_dir_files[WWW_FILE_COUNT] = {
    "./www/hello.html",
    "./www/index.css"
};

/* Handler Funcs. */

HandlerStatus handle_root(const HandlerContext *ctx, const BaseRequest *req, ResponseObj *res)
{
    const ResourceTable *restable_ref = &ctx->resources;
    const StaticResource *resrc_ref = restable_get(restable_ref, "./www/hello.html");

    if (!resrc_ref)
        return HANDLE_GENERAL_ERR;

    resinfo_set_mime_type(res, TXT_HTML);
    resinfo_set_content_length(res, statsrc_get_length(resrc_ref));
    resinfo_set_body_payload(res, resrc_ref->data);

    return HANDLE_OK;
}

HandlerStatus handle_index_css(const HandlerContext *ctx, const BaseRequest *req, ResponseObj *res)
{
    const ResourceTable *restable_ref = &ctx->resources;
    const StaticResource *resrc_ref = restable_get(restable_ref, "./www/index.css");

    if (!resrc_ref)
        return HANDLE_GENERAL_ERR;

    resinfo_set_mime_type(res, TXT_CSS);
    resinfo_set_content_length(res, statsrc_get_length(resrc_ref));
    resinfo_set_body_payload(res, resrc_ref->data);

    return HANDLE_OK;
}

void handle_signal_stops()
{
    // On SIGINT, etc, close server and cleanup its state.
    fprintf(stdout, "%s: recieved interrupt.\n", H1C_VERSION_STRING);
    server_core_cleanup(&server, server_wthrd_count);
}

int main(int argc, char *argv[])
{
    /// 1a. Setup server state.
    struct sigaction sa;
    bool ctx_ok = true;      // if resources in context loaded 
    bool handlers_ok = true; // if handlers loaded

    if (argc == 1)
    {
        // Use default host port if none is given in ARGV for user friendliness.
        server_core_init(&server, H1C_DEFAULT_HOSTNAME, H1C_DEFAULT_PORT, H1C_DEFAULT_BACKLOG);
    }
    else if (argc == 2 && atoi(argv[1]) > 1024)
    {
        // Use non-reserved port (1025+) to host server to prevent any extra socket errors.
        server_core_init(&server, H1C_DEFAULT_HOSTNAME, argv[1], H1C_DEFAULT_BACKLOG);
    }
    else
    {
        fprintf(stderr, "usage: %s <port?>", argv[0]);
        return 1;
    }

    /// 1b. Load resources to server.
    ctx_ok = server_core_setup_hdctx(&server, www_dir_files, WWW_FILE_COUNT);

    /// 1c. Load handlers to server.
    handlers_ok = server_core_put_handler(&server, "/home", GET, ANY_ANY, handle_root) && server_core_put_handler(&server, "/index.css", GET, ANY_ANY, handle_index_css);

    /// 1d. Put exit on interrupt handler for graceful cleanup.
    sa.sa_handler = handle_signal_stops;
    sigaction(SIGINT, &sa, NULL);

    /// 2. Run server... Automatically cleans up resources after service in server_run(...).
    if (ctx_ok && handlers_ok)
    {
        int server_wthrd_count = server_core_run(&server);
        server_core_join(&server, server_wthrd_count);
        fprintf(stdout, "%s: Launched server!\n", H1C_VERSION_STRING);
    }
    else
    {
        fprintf(stderr, "%s: Could not start, please check terminal output.\n", H1C_VERSION_STRING);
        server_core_cleanup(&server, server_wthrd_count);
    }

    return 0;
}
