/**
 * @file main.c
 * @author Derek Tan
 * @brief Implements driver code.
 * @version 0.0.1
 * @date 2023-08-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "server/h1c.h"

/* Constants and Helper Macros */

#define WWW_FILE_COUNT 2

static const char *www_dir_files[WWW_FILE_COUNT] = {
    "./www/index.html",
    "./www/index.css"
};

/* Handler Funcs. */

HandlerStatus handle_root(HandlerContext *ctx, BaseRequest *req, ResponseObj *res)
{
    const ResourceTable *restable_ref = &ctx->resources;
    const StaticResource *resrc_ref = restable_get(restable_ref, "./www/index.html");

    if (!resrc_ref)
        return HANDLE_GENERAL_ERR;

    resinfo_set_mime_type(res, TXT_HTML);
    resinfo_set_content_length(res, statsrc_get_length(resrc_ref));
    resinfo_set_body_payload(res, resrc_ref->data);

    return HANDLE_OK;
}

int main(int argc, char *argv[])
{
    /// 1a. Setup server state.
    H1CServer server;
    bool ctx_ok = true;      // if resources in context loaded 
    bool handlers_ok = true; // if handlers loaded

    if (argc == 1)
    {
        // Use default host port if none is given in ARGV for user friendliness.
        server_init(&server, H1C_HOSTNAME, H1C_DEFAULT_PORT, 4);
    }
    else if (argc == 2 && atoi(argv[1]) > 1024)
    {
        // Use non-reserved port (1025+) to host server to prevent any extra socket errors.
        server_init(&server, H1C_HOSTNAME, argv[1], 4);
    }
    else
    {
        perror("usage: h1cserver <port?>");
        return 1;
    }

    /// 1b. Load resources to server.
    ctx_ok = server_setup_ctx(&server, www_dir_files, WWW_FILE_COUNT);

    /// 1c. Load handlers to server.
    handlers_ok = server_add_handler(&server, "/", GET, ANY_ANY, handle_root);

    /// 2. Run server... Automatically cleans up resources after service in server_run(...).
    server_run(&server);

    return 0;
}
