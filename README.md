System Monitoring Tool -- Concurrency & Signals

Solutions:
- Ensure modularity with separate functions for each system monitoring task, such as memory
utilization, connected users, and CPU utilization.
- Employed pipes for communication between different processes, enabling them to exchange
data.
- Used fork() to create child processes for each system monitoring task. Each child process is
responsible for executing a specific monitoring function.
- Data Sharing between parent process and child processes via pipes, which allows the parent
process to collect the results from all child processes concurrently.
- Ensured proper synchronization and coordination between the parent and child processes by
using waitpid() to wait for child processes to finish, handling signals, and managing resource
allocation.
- Error Handling: Incorporated error-checking mechanisms to handle potential failures gracefully.
- Ensured that the output format remained consistent across all system monitoring tasks.

  
Functions:
MAX_LENGTH is defined as 1024
MAX_USERS is defined as 100
FLAGS_LENGTH is defined as 5
GB_CONVERTER is defined as1024 * 1024 * 1024

get_system_usage(int samples, int tdelay):
This function retrieves system resource usage information, such as memory usage,
and prints it to the standard output.

get_memory_utilization(char buf[MAX_LENGTH], double *physical_used):
This function retrieves memory utilization information, including physical and
virtual memory usage, and stores it in the provided buffer `buf`.

print_memory_utilization(int pipes[2], int graphics_pipe[2], int graphics, double
*physical_used)
This function reads memory utilization information from a pipe and optionally from
a graphics pipe, then formats and prints the information to the standard output.

get_connected_user(char users[MAX_USERS][MAX_LENGTH], int *index)
This function retrieves information about currently connected users from the system's
user accounting database.

print_connected_user(int pipes[2], int rows)
This function reads information about connected users from a pipe and prints it to the
standard output. It expects the information to be formatted as an array of strings
containing details about each connected user.
get_cpu_cores()
This function reads and returns the number of CPU cores from the /proc/cpuinfo file.

get_cpu_utilization(long cpu_data[2])
This function reads CPU utilization information from the /proc/stat file.

print_cpu_utilization(int pipes[2], long data[2], int id, int graphics, int cursor)
This function reads CPU utilization data from a pipe and calculates CPU utilization
percentage.

get_system_info()
This function retrieves various system information such and prints this information to
the standard output in a formatted manner.

print_system_status(int flags[FLAGS_LENGTH], int samples, int tdelay)
This function prints various system status information, including memory utilization,
connected users, CPU utilization, and system information, to the standard output.


How To Run
- Compile with makefile
make
- Remove the generated object file and executable with clean
make clean
- Use command line flags to specify the type of process table to display, set thresholds, or save
tables in text or binary format.
./mySystemStatus —system —user —sequential —graphics —samples=10 —tdelay=1
