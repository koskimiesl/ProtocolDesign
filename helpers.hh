/* General helper functions */

#ifndef PD_IOTPS_HELPERS_HH
#define PD_IOTPS_HELPERS_HH

// Print custom message (msg) and error message corresponding to errno to stderr
void error(std::string msg);

// Return command line option as int
// first and last are input iterators to the initial and final positions in a sequence
int getCmdOptAsInt(char** first, char** last, const std::string& option);

#endif
