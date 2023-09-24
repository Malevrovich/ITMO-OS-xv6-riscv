// Current process has no rights to read given process' registers
#define DUMP2_ENOACCESS -1
// Process with given PID doesn't exists
#define DUMP2_ENOPROC -2 
// Given register number is invalid 
#define DUMP2_EINVREG -3
// Attemptetion to write to return_value failed
#define DUMP2_ENOWRITE -4