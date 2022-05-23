#ifndef _COMMAND_CODES_H_
#define _COMMAND_CODES_H_

#define COMM_CODE_MAX_CHAR 256

/*
 * The following is a list of identifiers used for commands.
 * Each command is identified by 2 "command codes",
 * a numerical code starting at 0, and a text code.
 *
 * The numerical code is used in binary communication,
 * which is fast but not human-readable.
 * The text code is used in text-based communication,
 * which is much slower, but is human-readable.
 *
 * The macro below is pre-processed using some C++ macro magic.
 * 1. The macro takes advantage of enum-style auto-numbering.
 *    Thus the order of these is commands is important, as the
 *    order effectively defines each command's numerical code.
 * 2. The numerical codes are "#define" macro words that have
 *    "CC_" prepended to them. For example, the numerical code
 *    "SHAKE_REQ" below should be used as "CC_SHAKE_REQ".
 */
#define COMMANDCODES \
        etype(NONE=0                      , "none"       ),\
        etype(SHAKE_REQ                   , "shkreq"     ),\
        etype(SHAKE_RSP                   , "shkrsp"     ),\
        etype(ECHO                        , "echo"       ),\
        etype(DEVICE_LOG_LEVEL            , "log_level"  ),\
        /* */\
	etype(WRITE_LED                   , "wled"       ),\
	etype(WRITE_BNC                   , "wbnc"       ),\
	etype(WRITE_BNC_MODE              , "wbncmode"   ),\
        etype(CLEAR_STIM                  , "clear_stim" ),\
        etype(LOG                         , "log"        ),\
        etype(STIM                        , "*stim"      ),\
        etype(WAIT                        , "*wait"      ),\
        etype(TRIG                        , "*trig"      ),\
        etype(START_STIM                  , "start_stim" ),\
        etype(STOP_STIM                   , "stop_stim"  ),\
        etype(PAUSE_STIM                  , "pause_stim" ),\
        etype(START_STAIR                 , "start_stair"),\
        etype(STOP_STAIR                  , "stop_stair" ),\
        etype(SET_VAR                     , "set_var"    ),\
        etype(GET_VAR                     , "get_var"    ),\
        etype(DELETE_VAR                  , "delete_var" ),\
        etype(DELETE_VARS                 , "delete_vars" ),\
        /* ADD NEW COMMAND CODES ABOVE HERE! */\
        etype(MAX_SIZE                    , "maxsize"    )
/*
 * C++ is very picky about how multi-row defines are processed.:
 * - Each row MUST end in "\" followed by a newline/carriage-return.
 * - single-line comments canNOT be used (i.e. "//")
 * - only block comments CAN be used.
 */


#define COMM_CODE_T uint8_t

#define etype(x,y) CC_##x
typedef enum { COMMANDCODES } CommandCodes;
#undef etype

const char *enum2str (CommandCodes f);
const char *enum2str (COMM_CODE_T f);
CommandCodes str2enum (const char *f);


#endif // _COMMAND_CODES_H_
