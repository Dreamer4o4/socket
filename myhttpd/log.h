#ifndef _LOG_
#define _LOG_

#define LOG_FILE    "log.txt"
#define LOG_PORT    "4001"
#define LOG_BUFF_SIZE   300

int log_start();
int print_with_log(const char *formate, ...);
static int log_init();
static int log_test();
static int log_program(int sock);
static int log_write(const char *formate);
static int write_to_log(char *s, int len);

#endif