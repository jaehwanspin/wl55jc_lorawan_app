#include "./at_command.h"
#include "../app.h"
// #include "../telemetry/uart_io.h"

#include "../util/custom_typedefs.h"

#include <inttypes.h>
#include <string.h>
#include <memory.h>

// from src/main.c
extern struct app_data app_data;

/**
 * @author Jin
 * @brief clears rx buffer
 * 
 * @param ctx 
 */
static void _at_command_reset_buffer(struct at_command_context* ctx)
{
    ctx->rx_buffer_size = 0;
    memset(ctx->rx_buffer, 0, sizeof(ctx->rx_buffer));
}

/**
 * @author Jin
 * @brief parses command line from context buffer
 * 
 * @param ctx AT command context
 */
static void _at_command_parse_cmd(struct at_command_context* ctx)
{
    if (ctx->rx_buffer[0] >= 'A' &&
         strstr(ctx->rx_buffer, ctx->cfg.main_cmd) == ctx->rx_buffer)
    {
        bool found = false;
        int argc = 0;
        char* argv[AT_COMMAND_ARGV_MAX_SIZE] = { 0, };
        char* rx_ptr = ctx->rx_buffer;

        argv[argc++] = strtok_r(rx_ptr, ctx->cfg.main_delim, &rx_ptr);

        while (argv[argc++] = strtok_r(rx_ptr, ctx->cfg.sub_delim, &rx_ptr));

        argc--;

        if (argc == 1)
            ctx->cfg.test_handler(argc, argv);
        else
            for (int i = 0; i < ctx->cmds_size; i++)
            {
                if (0 == strcmp(argv[1], ctx->cmds[i].command))
                {
                    found = true;
                    ctx->cmds[i].cb_func(argc, argv);
                    break;
                }
            }
    }
    // else
    // {
    //     char err_msg[64] = { 0, };


    //     uart_print()
    // }
    
    _at_command_reset_buffer(ctx);
}

/**
 * @author Jin
 * @brief adds buffer and prints what I wrote
 * 
 * @param ctx AT command context
 * @param single_buffer single buffer
 */
static void _at_command_add_char_into_buffer(struct at_command_context* ctx, char single_buffer)
{
    if (ctx->rx_buffer_size < AT_COMMAND_RX_BUFFER_MAX_SIZE)
    {
        ctx->rx_buffer[ctx->rx_buffer_size++] = single_buffer;
        ctx->rx_buffer[ctx->rx_buffer_size] = 0;
    }
}

/**
 * @author Jin
 * @brief constructor
 * 
 * @param ctx AT command context
 * @param cfg AT command configuration data
 * @return true success
 * @return false fail(AT command context is null, empty error message, empty prompt)
 */
bool at_command_init(struct at_command_context* ctx, struct at_command_cfg* cfg)
{
    bool res = false;
    if (cfg != nullptr && cfg->main_delim && cfg->sub_delim)
    {
        memcpy(&ctx->cfg, cfg, sizeof(struct at_command_cfg));
    }

    return res;
}

/**
 * @author Jin
 * @brief command addition
 * 
 * @param ctx AT command context
 * @param cmd command, command handler(callback)
 * @return true success
 * @return false fail(AT command context is null, empty command, context overflow)
 */
bool at_command_add_cmd(struct at_command_context* ctx, const char* cmd, at_command_cb_func_t cb_func)
{
    bool res = false;
    if (ctx != nullptr && ctx->cmds_size <= AT_COMMAND_COMMANDS_MAX_SIZE)
    {
        strcpy(ctx->cmds[ctx->cmds_size].command, cmd);
        ctx->cmds[ctx->cmds_size].cb_func = cb_func;
        ctx->cmds_size++;
    }

    return res;
}

/**
 * @brief command removal
 * 
 * @param ctx AT command context
 * @param cmd_name command name
 * @return true success
 * @return false fail(AT command context is null, command not found, empty commands)
 */
bool at_command_remove_cmd(struct at_command_context* ctx, const char* cmd_name)
{
    bool res = false;
    if (ctx != nullptr && ctx->cmds_size > 0)
    {
        int i = 0;
        while (i < ctx->cmds_size)
        {
            if (0 == strcmp(ctx->cmds[i].command, cmd_name))
                break;
            i++;
        }

        if (i == AT_COMMAND_COMMANDS_MAX_SIZE)
            goto remove_cmd_res;
    }

remove_cmd_res:
    return res;
}

/**
 * @brief update in infinite loop
 * 
 * @param ctx AT command context
 * @param rx_value single rx buffer
 */
void at_command_update(struct at_command_context* ctx, char rx_value)
{
    switch (rx_value)
    {
    case '\n':
    case '\r':
    {
        _at_command_parse_cmd(ctx);
        break;
    }
    default:
    {
        _at_command_add_char_into_buffer(ctx, rx_value);
        break;
    }
    }
}

/**
 * @author Jin
 * @brief destructor
 * 
 * @param ctx AT command context
 */
void at_command_deinit(struct at_command_context* ctx)
{
    if (ctx != nullptr)
        memset(ctx, 0, sizeof(struct at_command_context));
}
