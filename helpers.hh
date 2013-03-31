/* General helper functions */
#ifndef HELPERS_HH
#define HELPERS_HH

/* Prints custom message (msg) and error message corresponding to errno to stderr */
void error(std::string msg);

/* Returns an UDP socket useful for server */
int customServerSocket(int family, const char port[]);

#endif
