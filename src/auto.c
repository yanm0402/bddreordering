#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_VARIABLES 1000
#define MAX_TERMS 10000

typedef struct {
    char name[10];
    int is_input;
    int is_output;
    int dependency_count[MAX_VARIABLES]; // Track dependency counts per output
    int total_dependency_count; // Total dependency count across all outputs
} Variable;

Variable inputs[MAX_VARIABLES];
Variable outputs[MAX_VARIABLES];
Variable sorted_inputs[MAX_VARIABLES];
int input_count = 0;
int output_count = 0;
int current_output_index = 0; // Used for sorting
int bdd_size = 1000000000;
int bdd_size1= 1000000000;
int original_order = 0;

int dependency_matrix[MAX_VARIABLES][MAX_VARIABLES][MAX_VARIABLES] = {{{0}}}; // Separate dependency matrix per output

void parse_pla_file(const char *filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        exit(1);
    }

    char line[256];
    int input_length = 0;
    int output_length = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '.' && strncmp(line, ".ilb", 4) == 0) {
            // Parse input labels
            char* token = strtok(line, " ");
            token = strtok(NULL, " \n");
            while (token != NULL) {
                strcpy(inputs[input_count].name, token);
                inputs[input_count].is_input = 1;
                inputs[input_count].total_dependency_count = 0;
                for (int i = 0; i < MAX_VARIABLES; i++) {
                    inputs[input_count].dependency_count[i] = 0;
                }
                input_count++;
                token = strtok(NULL, " \n");
            }
            input_length = input_count;
        } else if (line[0] == '.' && strncmp(line, ".ob", 3) == 0) {
            // Parse output labels
            char* token = strtok(line, " ");
            token = strtok(NULL, " \n");
            while (token != NULL) {
                strcpy(outputs[output_count].name, token);
                outputs[output_count].is_output = 1;
                output_count++;
                token = strtok(NULL, " \n");
            }
            output_length = output_count;
        } else if (line[0] != '.' && line[0] != '#') {
            // Parse minterm lines
            char* minterm = strtok(line, " ");
            char* output_values = strtok(NULL, " \n");

            if (minterm != NULL && output_values != NULL) {
                //printf("Minterm: %s, Output: %s\n", minterm, output_values);

                // Print which variables are involved
                for (int i = 0; i < input_length; i++) {
                    if (minterm[i] == '1') {
                        //printf("%s is present\n", inputs[i].name);
                    } else if (minterm[i] == '0') {
                       // printf("%s is negated\n", inputs[i].name);
                    } else if (minterm[i] == '-') {
                       // printf("%s is a don't care\n", inputs[i].name);
                    }
                }

                // Print the output values and update dependency count
                for (int i = 0; i < output_length; i++) {
                    if (output_values[i] == '1') {
                        //printf("%s = 1\n", outputs[i].name);
                        // Update dependency count for each input involved in this minterm
                        for (int j = 0; j < input_length; j++) {
                            if (minterm[j] == '1' || minterm[j] == '0') {
                                inputs[j].dependency_count[i]++;
                                inputs[j].total_dependency_count++;
                            }
                        }
                        // Update dependency matrix for the specific output
                        for (int j = 0; j < input_length; j++) {
                            if (minterm[j] == '1' || minterm[j] == '0') {  // Both '1' and '0' are relevant for dependency
                                for (int k = j + 1; k < input_length; k++) {
                                    if (minterm[k] == '1' || minterm[k] == '0') {
                                        dependency_matrix[i][j][k]++;
                                        dependency_matrix[i][k][j]++;
                                        // Increment the dependency count for both inputs involved
                                        inputs[j].dependency_count[i]++;
                                        inputs[k].dependency_count[i]++;
                                        inputs[j].total_dependency_count++;
                                        inputs[k].total_dependency_count++;
                                    }
                                }
                            }
                        }
                    } else if (output_values[i] == '0') {
                        //printf("%s = 0\n", outputs[i].name);
                    }
                }
            }
        }
    }

    fclose(file);
}

void print_dependency_matrix() {
    printf("\nDependency Matrix by Output:\n");
    for (int o = 0; o < output_count; o++) {
        printf("For output %s:\n", outputs[o].name);
        for (int i = 0; i < input_count; i++) {
            for (int j = 0; j < input_count; j++) {
                if (dependency_matrix[o][i][j]) {
                    printf("%s depends on %s (%d times)\n", inputs[i].name, inputs[j].name, dependency_matrix[o][i][j]);
                }
            }
        }
        printf("\n");
    }
}

int compare_dependency(const void* a, const void* b) {
    Variable* var_a = (Variable*)a;
    Variable* var_b = (Variable*)b;
    return (var_b->dependency_count[current_output_index] - var_a->dependency_count[current_output_index]); // Sort in descending order of dependency count for the specific output
}

int compare_total_dependency(const void* a, const void* b) {
    Variable* var_a = (Variable*)a;
    Variable* var_b = (Variable*)b;
    return (var_b->total_dependency_count - var_a->total_dependency_count); // Sort in descending order of total dependency count
}

void shuffle_inputs_with_same_dependency_count(Variable* variables, int count) {
    srand(time(NULL));
    int start = 0;
    while (start < count) {
        int end = start;
        // Find the range of variables with the same dependency count
        while (end < count - 1 && variables[end].total_dependency_count == variables[end + 1].total_dependency_count) {
            end++;
        }
        // Shuffle variables within the range [start, end]
        for (int i = start; i < end; i++) {
            int j = start + rand() % (end - start + 1);
            Variable temp = variables[i];
            variables[i] = variables[j];
            variables[j] = temp;
        }
        start = end + 1;
    }
}

void print_dependency_counts() {
    printf("\nDependency Counts by Output:\n");
    for (int i = 0; i < output_count; i++) {
        printf("For output %s:\n", outputs[i].name);
        Variable sorted_inputs[MAX_VARIABLES];
        memcpy(sorted_inputs, inputs, input_count * sizeof(Variable)); // Copy only relevant part of inputs
        current_output_index = i;
        qsort(sorted_inputs, input_count, sizeof(Variable), compare_dependency);
        for (int j = 0; j < input_count; j++) {
            printf("%s has %d dependencies\n", sorted_inputs[j].name, sorted_inputs[j].dependency_count[current_output_index]);
        }
    }
}

void find_output_with_largest_dependency() {
    int max_dependency_count = -1;
    int max_dependency_index = -1;

    for (int i = 0; i < output_count; i++) {
        int total_dependency_count = 0;
        for (int j = 0; j < input_count; j++) {
            total_dependency_count += inputs[j].dependency_count[i];
        }
        if (total_dependency_count > max_dependency_count) {
            max_dependency_count = total_dependency_count;
            max_dependency_index = i;
        }
    }

    if (max_dependency_index != -1) {
        printf("\nOutput with the largest total dependency count: %s (Total Dependencies: %d)\n", outputs[max_dependency_index].name, max_dependency_count);
    }
}

void print_total_dependency_counts() {
    printf("\nTotal Dependency Counts for Inputs:\n");
    
    memcpy(sorted_inputs, inputs, input_count * sizeof(Variable)); // Copy only relevant part of inputs
    qsort(sorted_inputs, input_count, sizeof(Variable), compare_total_dependency);
    shuffle_inputs_with_same_dependency_count(sorted_inputs, input_count); // Shuffle inputs with the same dependency count
    for (int i = 0; i < input_count; i++) {
        printf("%s has a total of %d dependencies\n", sorted_inputs[i].name, sorted_inputs[i].total_dependency_count);
    }
}

void calculate_bdd_size(const char *filename) {
    // Run an external program to calculate the BDD size and read the size back
    char command[100];
    char command1[100];
    sprintf(command, "./blif2bdd -order input_order.ord %s > bdd_size.txt", filename);
    system(command);
    FILE *file = fopen("bdd_size.txt", "r");
    if (file == NULL) {
        perror("Unable to open BDD size file");
        exit(1);
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Search for the line containing "size:" and extract the value
        char *size_ptr = strstr(line, "size:");
        if (size_ptr != NULL) {
            if (sscanf(size_ptr, "size: %d", &bdd_size) == 1) {
                break;
            }
        }
    }
    sprintf(command1, "./blif2bdd %s > bdd_size_original.txt", filename);
    system(command1);
    FILE *file1 = fopen("bdd_size_original.txt", "r");
    if (file1 == NULL) {
        perror("Unable to open BDD size file");
        exit(1);
    }
    char line1[256];
    while (fgets(line1, sizeof(line1), file1)) {
        // Search for the line containing "size:" and extract the value
        char *size_ptr1 = strstr(line1, "size:");
        if (size_ptr1 != NULL) {
            if (sscanf(size_ptr1, "size: %d", &bdd_size1) == 1) {
                break;
            }
            else printf("default order cause segmentation fault"); 
        }
    }
    if (bdd_size1 < bdd_size) original_order = 1;
    
    fclose(file);
    fclose(file1);
}
void write_input_order_to_file() {
    FILE *file = fopen("input_order.ord", "w");
    if (file == NULL) {
        perror("Unable to open input order file");
        exit(1);
    }

    for (int i = 0; i < input_count; i++) {
        fprintf(file, "%s\n", sorted_inputs[i].name);} 
    
    fclose(file);
    printf("\nInput order written to input_order.ord\n");
}
int main(int argc, char *argv[]) {
    if (argc < 3){
    fprintf(stderr, "usage: %s <pla_file> <blif_file>\n", argv[0]);
    exit(1);
    }
    parse_pla_file(argv[1]);
    print_dependency_matrix();
    print_dependency_counts();
    find_output_with_largest_dependency();
    print_total_dependency_counts();
    write_input_order_to_file();
    calculate_bdd_size(argv[2]);
    
    if (bdd_size <= bdd_size1) {
        printf("\nBDD Size: %d\n", bdd_size);
    } else if (bdd_size > bdd_size1) {         //use default order if the size is smallest
        printf("\nBDD Size: %d, used default order\n", bdd_size1);
      
    }
    else {
        printf("\nFailed to read BDD size\n");
    }
    return 0;
}
