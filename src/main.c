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

int main(int argc, char *argv[])
{
    /// 1. Setup server state.
    H1CServer server;

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

    /// 2. Run server.
    /// 3. Automatically cleanup resources after service in server_run(...).
    server_run(&server);

    return 0;
}
