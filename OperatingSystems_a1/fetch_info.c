#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define buffer_size 256
#define DAY 86400
#define HOUR 3600
#define MINUTE 60

void print_cpu_info();
void print_linux_version();
void print_memory_info();
void print_uptime();
void print_process_info(char*, char*);

int main(int argc, char *argv[]) {
    if (argc > 2) {
        printf("Wrong Usage: %s\n", argv[0]);
        return 1;
    }
    if(argc == 1){
        print_cpu_info();
        print_linux_version();
        print_memory_info();
        print_uptime();
        return 0;
    }
    else if (argc ==2){
        FILE *file;
        char path[20];
        snprintf(path, sizeof(path),"/proc/%s", argv[1]);
        file = fopen(path, "r");
        if (file == NULL) {
            char error_message[buffer_size];
            snprintf(error_message, sizeof(error_message), "Process number %s not found", argv[1]);
            perror(error_message);
            return 1;
        } 
        fclose(file);
        print_process_info(path, argv[1]);
    }
   
}

void print_cpu_info(){
    FILE *file;
    char line[buffer_size];

    // Open the file for reading
    file = fopen("/proc/cpuinfo", "r");
    if (file == NULL) {
        perror("Failed to open /proc/cpuinfo");
        return;
    }

    // Read each line of the file and printing model name and cpu cores
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "model name",10) == 0){
            char *model_name = strchr(line, ':');
            if (model_name){
                model_name++;
                while (*model_name == ' ') model_name++; 
            }
            printf("model name:   %s", model_name);
        }
        if (strncmp(line, "cpu cores",9) == 0){
            char *cpu_cores = strchr(line, ':');
            if (cpu_cores){
                cpu_cores++;
                while (*cpu_cores == ' ') cpu_cores++; 
            }
            printf("cpu cores:   %s", cpu_cores);
            break; // Break after printing cpu cores
        }
    }
    // Close the file
    fclose(file);
}
void print_linux_version(){
    FILE *file;
    char line[buffer_size];

    // Open the file for reading
    file = fopen("/proc/version", "r");
    if (file == NULL) {
        perror("Failed to open /proc/version");
        return;
    }

    // Read each line of the file and printing linux version
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("Linux version:%s", line+13);
        break; // Break after printing linux version
    }
    // Close the file
    fclose(file);
}
void print_memory_info(){
    FILE *file;
    char line[buffer_size];

    // Open the file for reading
    file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("Failed to open /proc/meminfo");
        return;
    }
    // Read first line of the file.
    fgets(line, sizeof(line), file);
    char *meminfo = strchr(line, ':');
    if (meminfo){
        meminfo++;
        while (*meminfo == ' ') meminfo++; // Skip spaces
        printf("MemTotal:   %s", meminfo);
    }
    // Close the file
    fclose(file);
}
void print_uptime(){
    FILE *file;
    char line[buffer_size];

    // Open the file for reading
    file = fopen("/proc/uptime", "r");
    if (file == NULL) {
        perror("Failed to open /proc/uptime");
        return;
    }
    // Read first line of the file.
    fgets(line, sizeof(line), file);
    // Extract uptime value
    char *uptime = strtok(line, " ");
    int time = atoi(uptime);// Convert to integer
   
    // Calculate days, hours, minutes, and seconds
    int days = time / DAY;
    int hours = (time - (days * DAY)) / HOUR;
    int minutes = (time - (days * DAY) - (hours * HOUR)) / MINUTE;
    int seconds = time - (days * DAY) - (hours * HOUR) - (minutes * MINUTE);
    printf("Uptime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);

    // Close the file
    fclose(file);
}
void print_process_info(char* path, char* process_number){
    
    FILE *file;
    FILE *file2;
    char line[buffer_size];
    char line2[buffer_size];
    char temp[buffer_size];
    int ctxt_switches = 0;

    //update path to include the process' status or cmdline
    char status[] = "/status";
    char cmdline[] = "/cmdline";

    snprintf(temp, sizeof(temp), "%s", path);
    

    // create the path for the required file
    strcat(temp, cmdline);

    file2 = fopen(temp, "r");
    if (file2 == NULL) {
        perror("Failed to open cmdline");
        return ;
    }
    int read = fread(line2, sizeof(char), sizeof(line2)-1, file2);
   
    fclose(file2);
    char cmd[buffer_size];
    
    if (read > 0) {
        snprintf(cmd, sizeof(cmd), "%s", line2);
    } 


    strcat(path, status);

    // Open the file for reading
    file = fopen(path, "r");
    if (file == NULL) {
        char error_message[buffer_size];
        snprintf(error_message, sizeof(error_message), "Failed to open %s", path);
        perror("error_message");
        return ;
    }
    
    printf("Process number:   %s\n", process_number);

    // Read each line of the file and printing process info
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "Name",4) == 0){
            printf("%s", line);
            printf("Filname (if any):   %s\n", cmd);
        }
        if (strncmp(line, "Threads",7) == 0){
            char *thrds = strchr(line, ':');
            if (thrds){
                thrds++;
                int i = atoi(thrds);
                printf("Threads:   %d\n", i);

            }
        }
        if (strncmp(line, "voluntary_ctxt_switches", 23) == 0){
            char* token = strtok(line, ":");
            token = strtok(NULL, ":");
            ctxt_switches += atoi(token);
        }
        if (strncmp(line, "nonvoluntary_ctxt_switches", 26) == 0){
           char* token = strtok(line, ":");
            token = strtok(NULL, ":");
            ctxt_switches += atoi(token);
        }
        // Break after printing process info
    }
    printf("Total Context switches:   %d\n", ctxt_switches);

    // Close the file
    fclose(file);
}