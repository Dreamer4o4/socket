#ifndef _LOG_
#define _LOG_

#define LOG_FILE    "log.txt"
#define LOG_PORT    "4001"
#define LOG_BUFF_SIZE   300

/*
**  start a process to receive log info
*/
int log_start();

/*
**  write log info to log process
*/
int print_with_log(const char *formate, ...);




/**********  local function  **********/

static int log_init();

/*
**  try to open log txt
*/
static int log_test();

static int log_program(int sock);

/*
**  write info into log txt
*/
static int log_write(const char *formate);

/*
**  send info to log process
*/
static int write_to_log(char *s, int len);

#endif