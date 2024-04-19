#include "stats.function.h"

/**
 * This function uses ANSI escape sequences to move the cursor to a specified row and column
 * position in the terminal window.
 */
void moveCursorTo(int row, int col)
{
    printf("\033[%d;%dH", row, col);
}

/**
 * This function uses ANSI escape sequences to clear the entire screen in the terminal window.
 */
void clearScreen()
{
    printf("\033[2J");
}

/**
 * This function retrieves system resource usage information, such as memory usage,
 * and prints it to the standard output. It utilizes the `getrusage` function to
 * obtain resource usage statistics for the calling process.
 *
 * @param samples Number of samples to be taken.
 * @param tdelay Time delay between each sample (in seconds).
 */
void get_system_usage(int samples, int tdelay)
{

    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == -1)
    {
        perror("failed to retrieve resourse usage");
        exit(EXIT_FAILURE);
    }

    long memory_usage = usage.ru_maxrss;
    printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
    printf(" Memory usage: %lu kilobytes\n", memory_usage);
    printf("---------------------------------------\n");
}

/**
 * This function retrieves memory utilization information, including physical and virtual memory usage,
 * and stores it in the provided buffer `buf`. It also calculates and returns the amount of physical
 * memory used through the pointer `physical_used`.
 *
 * @param buf The buffer to store the memory utilization information (should have a capacity of at least MAX_LENGTH).
 * @param physical_used Pointer to a double variable where the amount of physical memory used will be stored.
 */
void get_memory_utilization(char buf[MAX_LENGTH], double *physical_used)
{
    struct sysinfo info;
    if (sysinfo(&info) == -1)
    {
        perror("failed to retrieve memory usage");
        exit(EXIT_FAILURE);
    }
    double unit = (double)info.mem_unit;
    double physical = info.totalram * unit;
    *physical_used = physical - info.freeram * unit;
    double virtual = physical + info.totalswap * unit;
    double virtual_used = virtual - (info.freeram + info.freeswap) * unit;
    physical /= GB_CONVERTER;
    *physical_used /= GB_CONVERTER;
    virtual /= GB_CONVERTER;
    virtual_used /= GB_CONVERTER;
    sprintf(buf, "%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", *physical_used, physical, virtual_used, virtual);
}

/**
 * This function reads memory utilization information from a pipe and optionally from a graphics pipe,
 * then formats and prints the information to the standard output.
 * If graphics is enabled, it also visualizes the change in memory utilization compared
 * to the previous value using ASCII characters.
 *
 * @param pipes An array containing file descriptors for the memory pipe (pipes[0] for reading).
 * @param graphics_pipe An array containing file descriptors for the graphics pipe (graphics_pipe[0] for reading).
 * @param graphics Flag indicating whether to enable graphics visualization (1 for enabled, 0 for disabled).
 * @param physical_used Pointer to the amount of physical memory used (updated by reference).
 */
void print_memory_utilization(int pipes[2], int graphics_pipe[2], int graphics, double *physical_used)
{
    char buf[MAX_LENGTH];
    double new_used;
    close(pipes[1]);
    close(graphics_pipe[1]);
    ssize_t total_bytes_read = 0;
    while (total_bytes_read < sizeof(buf))
    {
        ssize_t bytes_read = read(pipes[0], buf + total_bytes_read, sizeof(buf) - total_bytes_read);
        if (bytes_read == -1)
        {
            perror("Error reading from memory pipe");
            exit(EXIT_FAILURE);
        }
        total_bytes_read += bytes_read;
    }
    buf[total_bytes_read] = '\0';
    close(pipes[0]);
    if (graphics == 1)
    {
        read(graphics_pipe[0], &new_used, sizeof(new_used));
        double util_diff = new_used - *physical_used;
        int diff_count = util_diff / 0.01;
        if (*physical_used == 0)
            util_diff = 0;

        strcat(buf, "\t|");
        if (util_diff == 0)
        {
            strcat(buf, "o");
        }
        else if (util_diff > 0)
        {
            for (int i = 0; i < diff_count; i++)
            {
                strcat(buf, "#");
            }
            strcat(buf, "*");
        }
        else
        {
            for (int i = 0; i < -diff_count; i++)
            {
                strcat(buf, ":");
            }
            strcat(buf, "@");
        }
        char line[MAX_LENGTH];
        sprintf(line, " %.2f (%.2f)", util_diff, new_used);
        strcat(buf, line);
    }
    close(graphics_pipe[0]);

    printf("%s\n", buf);
}

/**
 * This function retrieves information about currently connected users from the system's user accounting database.
 *
 * @param users A 2D character array to store information about connected users (each row represents a user).
 * @param index Pointer to an integer representing the number of connected users (updated by reference).
 */
void get_connected_user(char users[MAX_USERS][MAX_LENGTH], int *index)
{
    struct utmp *entry;
    setutent();
    while ((entry = getutent()) != NULL)
    {
        if (entry->ut_type == USER_PROCESS)
        {
            sprintf(users[*(index)], "%s\t%s (%s)", entry->ut_user, entry->ut_line, entry->ut_host);
            *(index) = *(index) + 1;
        }
    }
    endutent();
}

/**
 * This function reads information about connected users from a pipe and prints it to the standard output.
 * It expects the information to be formatted as an array of strings containing details about each connected user.
 *
 * @param pipes An array containing file descriptors for the pipe (pipes[0] for reading).
 * @param rows The number of rows to read from the pipe and print to the output.
 */
void print_connected_user(int pipes[2], int rows)
{

    close(pipes[1]);
    char new_users[MAX_USERS][MAX_LENGTH];
    ssize_t bytes_read = read(pipes[0], &new_users, rows * MAX_LENGTH);
    if (bytes_read == -1)
    {
        printf("Error reading users pipe");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++)
    {
        printf("%s\n", new_users[i]);
    }
    printf("---------------------------------------\n");
    close(pipes[0]);
}

/**
 * This function reads and returns the number of CPU cores from the /proc/cpuinfo file.
 *
 * @return The number of CPU cores, or -1 if the information couldn't be retrieved.
 */
int get_cpu_cores()
{
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo == NULL)
    {
        perror("failed to open proc file");
        exit(EXIT_FAILURE);
    }
    char line[MAX_LENGTH];
    while (fgets(line, sizeof(line), cpuinfo) != NULL)
    {
        if (strncmp(line, "cpu cores", 9) == 0)
        {
            char *arg = strchr(line, ':');
            if (arg != NULL)
            {
                arg++;
                fclose(cpuinfo);
                return atoi(arg);
            }
        }
    }
    fclose(cpuinfo);
    return -1;
}

/**
 * This function reads CPU utilization information from the /proc/stat file.
 * The total CPU time and idle time are stored in the cpu_data array.
 *
 * @param cpu_data Array to store CPU utilization data (total CPU time and idle time).
 */
void get_cpu_utilization(long cpu_data[2])
{
    FILE *stat_file = fopen("/proc/stat", "r");
    if (stat_file == NULL)
    {
        perror("failed to open proc file");
        exit(EXIT_FAILURE);
    }
    long user, nice, sys, iowait, idle, irq, softirq;
    if (fscanf(stat_file, "%*s %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &sys, &idle, &iowait, &irq, &softirq) != 7)
    {
        perror("failed to scan file info");
        exit(EXIT_FAILURE);
    }
    cpu_data[0] = user + nice + sys + idle + iowait + irq + softirq;
    cpu_data[1] = idle;

    fclose(stat_file);
}

/**
 * This function reads CPU utilization data from a pipe and calculates CPU utilization percentage.
 * It optionally visualizes the CPU utilization using ASCII characters if graphics is enabled.
 *
 * @param pipes An array containing file descriptors for the pipe (pipes[0] for reading).
 * @param data Array containing previous CPU utilization data (total CPU time and idle time).
 * @param id Identifier for the CPU.
 * @param graphics Flag indicating whether to enable graphics visualization (1 for enabled, 0 for disabled).
 * @param cursor Cursor position for printing graphics visualization.
 */
void print_cpu_utilization(int pipes[2], long data[2], int id, int graphics, int cursor)
{
    long cpu_data[2];
    double cpu_utilization = 0;
    long idle_diff = 0, time_diff = 0;
    read(pipes[0], &cpu_data, 2 * sizeof(long));
    if (id == 0)
    {
        data[0] = cpu_data[0];
        data[1] = cpu_data[1];
    }
    if (id > 0)
    {
        idle_diff = cpu_data[1] - data[1];
        time_diff = cpu_data[0] - data[0];
        cpu_utilization = (double)(time_diff - idle_diff) / time_diff * 100;
    }
    close(pipes[0]);
    printf("total cpu use = %.2f%%\n", cpu_utilization);
    if (graphics == 1 && id > 0 && cpu_utilization > 0)
    {
        moveCursorTo(cursor + id + 1, 1);
        int count = cpu_utilization / 0.03;
        printf("\t\t");
        for (int i = 0; i < count; i++)
        {
            printf("|");
        }
        printf("  %.2f\n", cpu_utilization);
    }
}

/**
 * This function retrieves various system information such and prints this information to
 * the standard output in a formatted manner.
 */
void get_system_info()
{
    struct utsname buf;
    if (uname(&buf) == -1)
    {
        perror("failed to retrieve system information");
        exit(EXIT_FAILURE);
    }

    FILE *uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file == NULL)
    {
        perror("failed to open file");
        exit(EXIT_FAILURE);
    }

    double uptime;
    if (fscanf(uptime_file, "%lf", &uptime) != 1)
    {
        perror("Error reading uptime file");

        fclose(uptime_file);
        exit(EXIT_FAILURE);
    }
    fclose(uptime_file);

    int to_seconds = (int)uptime;
    int days = to_seconds / (24 * 3600);
    to_seconds %= (24 * 3600);
    int hours = to_seconds / 3600;
    to_seconds %= 3600;
    int minutes = to_seconds / 60;
    int seconds = to_seconds % 60;

    printf("### System Information ### \n");
    printf("System Name = %s\n", buf.sysname);
    printf("Machine Name = %s\n", buf.nodename);
    printf("Version = %s\n", buf.version);
    printf("Release = %s\n", buf.release);
    printf("Architecture = %s\n", buf.machine);
    printf("System running since last reboot: %d days %d:%d:%d (%d:%d:%d)\n", days, hours, minutes, seconds, days * 24 + hours, minutes, seconds);
    printf("---------------------------------------\n");
}

void print_header(int base, int samples, int tdelay, int system, int user)
{
    get_system_usage(samples, tdelay);
    if (system == 1)
    {
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
        moveCursorTo(base + samples + 5, 1);
        printf("---------------------------------------\n");
    }
    if (user == 1)
        printf("### Sessions/users ###\n");
}

/**
 * This function prints various system status information, including memory utilization, connected users,
 * CPU utilization, and system information, to the standard output.
 *
 * @param flags An array containing flags to control the behavior of the function
 * @param samples Number of samples to be taken for system usage.
 * @param tdelay Time delay (in seconds) between each sample.
 */
void print_system_status(int flags[FLAGS_LENGTH], int samples, int tdelay)
{
    ssize_t bytes_write = 0;
    int saved_rows = 0;
    int base = 0;
    char buf[MAX_LENGTH];
    long cpu_data[2];
    double physical_used = 0;
    int memory_pipes[samples][2], users_pipes[samples][2], cpu_pipes[samples][2];
    int system = flags[0];
    int user = flags[1];
    int graphics = flags[2];
    int sequential = flags[3];
    int cores = get_cpu_cores();
    print_header(base, samples, tdelay, system, user);
    for (int i = 0; i < samples; i++)
    {
        int system_graphic_fd[2];
        if (sequential == 1 && i % 2 == 0)
        {
            clearScreen();
            base = 0;
        }
        else if (sequential == 1)
            base = samples + saved_rows + 13;

        if (pipe(memory_pipes[i]) == -1 || pipe(system_graphic_fd) == -1)
        {
            perror("failed to create pipe");

            exit(EXIT_FAILURE);
        }
        pid_t memory = fork();

        if (memory == -1)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (memory == 0)
        {
            if (system != 1)
            {
                close(memory_pipes[i][1]);
                close(memory_pipes[i][0]);
                exit(EXIT_SUCCESS);
            }

            get_memory_utilization(buf, &physical_used);
            bytes_write = write(memory_pipes[i][1], buf, sizeof(buf));
            if (bytes_write == -1)
            {
                perror("Error writing to memory pipe");
                exit(EXIT_FAILURE);
            }
            if (graphics == 1)
            {
                bytes_write = write(system_graphic_fd[1], &physical_used, sizeof(physical_used));
                if (bytes_write == -1)
                {
                    perror("Error writing to memory pipe");
                    exit(EXIT_FAILURE);
                }
            }
            close(system_graphic_fd[1]);
            exit(EXIT_SUCCESS);
        }

        else
        {
            double new_physical_used = 0;
            if (sequential == 1)
                print_header(base, samples, tdelay, system, user);
            moveCursorTo(base + i + 5, 1);
            if (system == 1)
                print_memory_utilization(memory_pipes[i], system_graphic_fd, graphics, &new_physical_used);
            int rows_fd[2];
            int rows = 0;
            if (pipe(users_pipes[i]) == -1 || pipe(rows_fd) == -1)
            {
                perror("failed to create pipe");
                exit(EXIT_FAILURE);
            }
            pid_t users = fork();
            if (users == -1)
            {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
            else if (users == 0)
            {
                if (user != 1)
                {
                    close(users_pipes[i][1]);
                    close(users_pipes[i][0]);
                    close(rows_fd[1]);
                    close(rows_fd[0]);
                    exit(EXIT_SUCCESS);
                }
                char all_users[MAX_USERS][MAX_LENGTH];
                get_connected_user(all_users, &rows);
                bytes_write = write(users_pipes[i][1], &all_users, rows * MAX_LENGTH);
                if (bytes_write == -1)
                {
                    perror("Error writing to users pipe");
                    exit(EXIT_FAILURE);
                }
                if (system == 1)
                    moveCursorTo(base + samples + 7, 1);
                else
                    moveCursorTo(base + 5, 1);
                print_connected_user(users_pipes[i], rows);
                write(rows_fd[1], &rows, sizeof(rows));
                close(rows_fd[1]);
                exit(EXIT_SUCCESS);
            }
            else
            {
                if (user == 1)
                {
                    read(rows_fd[0], &saved_rows, sizeof(int));
                    close(rows_fd[0]);
                }
                if (system == 1 && user == 1)
                {
                    moveCursorTo(base + samples + saved_rows + 8, 1);
                }
                else if (system == 1)
                {
                    moveCursorTo(base + samples + 6, 1);
                }

                else if (user == 1)
                {
                    moveCursorTo(base + saved_rows + 6, 1);
                }
                printf("Number of cores: %d", cores);
                if (pipe(cpu_pipes[i]) == -1)
                {
                    perror("failed to create pipe");
                    exit(EXIT_FAILURE);
                }
                pid_t cpu = fork();
                if (cpu == -1)
                {
                    perror("fork failed");
                    exit(EXIT_FAILURE);
                }
                else if (cpu == 0)
                {
                    long new_cpu_data[2];
                    get_cpu_utilization(new_cpu_data);
                    bytes_write = write(cpu_pipes[i][1], &new_cpu_data, 2 * sizeof(long));
                    if (bytes_write == -1)
                    {
                        perror("Error writing to cpu pipe");
                        exit(EXIT_FAILURE);
                    }
                    close(cpu_pipes[i][1]);
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    int cursor = 0;
                    if (system == 1 && user == 1)
                    {
                        cursor = base + samples + saved_rows + 9;
                    }
                    else if (user == 1)
                    {
                        cursor = base + saved_rows + 7;
                    }
                    else if (system == 1)
                    {
                        cursor = base + samples + 7;
                    }

                    moveCursorTo(cursor, 1);
                    print_cpu_utilization(cpu_pipes[i], cpu_data, i, graphics, cursor);
                    if (i == samples - 1)
                        printf("---------------------------------------\n");

                    sleep(tdelay);
                }
                waitpid(cpu, NULL, 0);
            }
            waitpid(users, NULL, 0);
        }
        waitpid(memory, NULL, 0);
    }
    get_system_info();
}