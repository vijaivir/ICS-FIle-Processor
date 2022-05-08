#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE_LEN 132
#define MAX_EVENTS 500

typedef struct event{
    char *dtstart;
    char *dtend;
    char *start_time;
    char *end_time;
    char *repeat_until;
    char *location;
    char *summary;
    char output[MAX_LINE_LEN];
} Event;


void extract(char *, int, int);
void sort_and_print(Event c[], int, int, int);
void print_date(char *, const char *, const int);
void print_line(char *);
void print_time_summary(int, int, char *, char *);
void increment_date(char *, const char *, int const);
void decrement_date(char *, const char *, int const);
int concatenate(int, int, int);

int main(int argc, char *argv[]){

    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i; 

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr, 
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }

    /* Starting calling your own code from this point. */
    int from = concatenate(from_y, from_m, from_d);
    int to = concatenate(to_y, to_m, to_d);
    extract(filename, from, to);
    exit(0);    
}


/*
 * Function: extract()
 * 
 * Purpose: Reads data from a file and stores it in an array called "words[][]".
 *          Iterates through words[][], and extracts useful information onto an
 *          array of Events called calendar[].
 * 
 * Parameters: char *filename - name of file
 *             int print_from - user specified start date for output
 *             int print_to - user specified end date for output
 */

void extract(char *filename, int print_from, int print_to){

    Event calendar[MAX_EVENTS];
    char words[(MAX_EVENTS*7)][MAX_LINE_LEN];
    char buffer[MAX_LINE_LEN];
    char *token, *st, *et, *rt;

    int num_words = 0;
    int size = 0;
    
    FILE *fptr = fopen(filename, "r");
    if(fptr == NULL){
        fprintf(stderr, "unable to open %s\n", filename);
    }
    
    while(fgets(buffer, MAX_LINE_LEN, fptr) != NULL){
        token = strtok(buffer, ":");
        while(token != NULL){
            strncpy(words[num_words], token, 80);
            num_words++;
            token = strtok(NULL, "\0");
        }
    }
    
    for(int i = 0; i < 2 * num_words; i++){
        if(strcmp(words[i], "DTSTART") == 0){
            st = strtok(words[i+1], "T");
            calendar[size].dtstart = st;
            strncpy(calendar[size].output, calendar[size].dtstart, MAX_LINE_LEN);
            st = strtok(NULL, "\n");
            calendar[size].start_time = st;            
        }
        if(strcmp(words[i], "DTEND") == 0){
            et = strtok(words[i+1], "T");
            calendar[size].dtend = et;
            et = strtok(NULL, "\n");
            calendar[size].end_time = et;
        }
        if(strcmp(words[i], "RRULE") == 0){
            rt = strtok(words[i+1], "N");
            rt = strtok(NULL, "=");
            rt = strtok(NULL, "T");
            calendar[size].repeat_until = rt;
        }
        if(strcmp(words[i], "LOCATION") == 0){
            calendar[size].location = words[i+1];
            calendar[size].location[strcspn(calendar[size].location, "\n")] = 0;
        }
        if(strcmp(words[i], "SUMMARY") == 0){
            calendar[size].summary = words[i+1];
            calendar[size].summary[strcspn(calendar[size].summary, "\n")] = 0;
        }
        if(strcmp(words[i], "END") == 0){
            if(strcmp(words[i+1], "VEVENT\n") == 0){
            size++;
            }
        }
    }
    sort_and_print(calendar, size, print_from, print_to);
    fclose(fptr);   
}


/* 
 * Function: sort_and_print()
 * 
 * Purpose: Iterates through an array of Events, adds any repeating events
 *          to the array, sorts the array chronologically by their start dates
 *          or start times, and prints the contents of the array
 *          within the user specified date range in a readable format.
 *
 * Parameters: Event c[] - array of structs of type Event
 *             int size - size of array
 *             int print_from - user specified start date
 *             int print_to - user specifed end date 
 */

void sort_and_print(Event c[], int size, int print_from, int print_to){

    char output[MAX_LINE_LEN], cur_date[MAX_LINE_LEN], formatted_time[MAX_LINE_LEN];
    Event temp[1];
    int min = 0;
    int output_size = 0;
    int increment = 0; 
    char out[MAX_LINE_LEN], temporary[MAX_LINE_LEN];
    
    /* Expands repeating events and adds them to c[] */ 
    for(int i = 0; i < size; i++){
        if(c[i].repeat_until != NULL){
            strncpy(temporary, c[i].repeat_until, MAX_LINE_LEN);
            decrement_date(out, temporary, 7);
            strncpy(cur_date, c[i].dtstart, MAX_LINE_LEN);
            while(atoi(cur_date) <= atoi(out)){
                increment_date(output, cur_date, 7);
                strncpy(c[size].output, output, MAX_LINE_LEN);
                c[size].start_time = c[i].start_time;
                c[size].end_time = c[i].end_time;
                c[size].location = c[i].location;
                c[size].summary = c[i].summary;
                strncpy(cur_date, output, MAX_LINE_LEN);
                size++;               
            }
        }
    }

    /* Orders events in chronological order by start date using selection sort*/
    for(int k = 0; k < size - 1; k++){
        min = k;
        for(int j = k + 1; j < size; j++){
            if(atoi(c[j].output) < atoi(c[min].output)){
                min = j;
            }else if(atoi(c[j].output) == atoi(c[min].output)){
                if(atoi(c[j].start_time) < atoi(c[min].start_time)){
                    min = j;
                }
            }
        }
        temp[0] = c[k];
        c[k] = c[min];
        c[min] = temp[0];
    }
    
    /* Determines number of events to be printed within date range*/
    for(int i = 0; i < size; i++){
        if(atoi(c[i].output) >= print_from && atoi(c[i].output) <= print_to){
            output_size++;
        }
    } 
    
    /* Calls several print functions to output the events in c[] */
    for(int i = 0; i < size; i++){
        /* Check condition for valid date range */
        if(atoi(c[i].output) >= print_from && atoi(c[i].output) <= print_to){
        increment++;
            /* If c[] only contains a single Event */
            if(size == 1){
                print_date(formatted_time, c[i].output, MAX_LINE_LEN);
                print_line(formatted_time);
                print_time_summary(atoi(c[i].start_time), atoi(c[i].end_time), c[i].summary, c[i].location);
            }
            /* Else c[] contains multiple events */
            else{
                /* If next event is on the same date, but the previous event was on a different date */
                if((atoi(c[i+1].output) == atoi(c[i].output)) && (atoi(c[i-1].output) != atoi(c[i].output))){
                    print_date(formatted_time, c[i].output, MAX_LINE_LEN);
                    print_line(formatted_time);
                    print_time_summary(atoi(c[i].start_time), atoi(c[i].end_time), c[i].summary, c[i].location);
                }
                /* If the previous event was on the same date */
                else if(atoi(c[i-1].output) == atoi(c[i].output)){
                    print_time_summary(atoi(c[i].start_time), atoi(c[i].end_time), c[i].summary, c[i].location);
                    /* If not the last event to be printed, and next event is not on the same date, seperate output with line*/
                    if(increment != (output_size) && atoi(c[i+1].output) != atoi(c[i].output)){
                        printf("\n");
                    }
                }
                /* Else the event is on a different date */
                else{
                    print_date(formatted_time, c[i].output, MAX_LINE_LEN);
                    print_line(formatted_time);
                    print_time_summary(atoi(c[i].start_time), atoi(c[i].end_time), c[i].summary, c[i].location);
                    /* If not the last event to be printed, seperate output with a line */
                    if(increment != (output_size)){
                        printf("\n");
                    }
                }
            }
        }
    }
}


/*
 * Function: print_date()
 *
 * Purpose: Given a date-time, creates a more readable version of the
 *          calendar date by using some C-library routines. For example,
 *          if the string in "dt_time" corresponds to: 20190520T111500
 *          then the string stored at "formatted_time" is: May 20, 2019 (Mon).
 *
 * Parameters: char *formatted_time - address of a string to store the output
 *             cont char *dt_time - address of a string containing the the date
 *             const int len - length of the string at *dt_time 
 * 
 * Credit: Functioned copied from "timeplay.c" provided by Michael Zaztre
 */

void print_date(char *formatted_time, const char *dt_time, const int len){

    struct tm temp_time;
    time_t    full_time;
    char      temp[5];

    /*  
     * Ignore for now everything other than the year, month and date.
     * For conversion to work, months must be numbered from 0, and the 
     * year from 1900.
     */  
    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d", &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_time, len, "%B %d, %Y (%a)", localtime(&full_time));
    printf("%s\n", formatted_time);
}


/*
 * Function: print_line()
 * 
 * Purpose: Prints a line with "-" mathching the size of a specified date
 * 
 * Parameters: char *output - string containing the date
 */

void print_line(char *output){
    for(int i = 0; i < strlen(output); i++){
        printf("-");
    }
    printf("\n");
}


/*
 * Function: print_time_summary()
 * 
 * Purpose: Converts the 24 hour time of an event into 12 hour time and prints
 *          it along with the summary and location of the event. 
 * 
 * Parameters: int start - start time of event
 *             int end - end time of event
 *             char *summary - summary of what the event is
 *             char *location - location of the event
 */

void print_time_summary(int start, int end, char *summary, char *location){
    
    start /= 100;
    end /= 100;

    int start_min = start % 100;
    int end_min = end % 100;
    char *start_period;
    char *end_period;

    start /= 100;
    end /= 100;
    
    if(start == 0){
        start = 12;
        start_period = "AM";
    } else if(start == 12){
        start_period = "PM";
    } else if(start > 12){
        start -= 12;
        start_period = "PM";
    } else{
        start_period = "AM";
    }

    if(end == 0){
        end = 12;
        end_period = "AM";
    } else if(end == 12){
        end_period = "PM";
    } else if(end > 12){
        end -= 12;
        end_period = "PM";
    } else{
        end_period = "AM";
    }
    
    if(start < 10 && end < 10){
        printf(" %d:%02d %s to  %d:%02d %s: %s {{%s}}\n", start, start_min, start_period, end, end_min, end_period, summary, location);
    } else if(start < 10 && end >= 10){
        printf(" %d:%02d %s to %d:%02d %s: %s {{%s}}\n", start, start_min, start_period, end, end_min, end_period, summary, location);
    } else if(start >= 10 && end < 10){
        printf("%d:%02d %s to  %d:%02d %s: %s {{%s}}\n", start, start_min, start_period, end, end_min, end_period, summary, location);
    } else if(start >= 10 && end >= 10){
        printf("%d:%02d %s to %d:%02d %s: %s {{%s}}\n", start, start_min, start_period, end, end_min, end_period, summary, location);
    }
}


/*
 * Function: increment_date() 
 *
 * Purpose: Given a date-time, it adds the number of days in a way that
 *          results in the correct year, month, and day. For example,
 *          if the string in "before" corresponds to: 20190520T111500
 *          then the datetime string stored in "after", assuming that
 *          "num_days" is 100, will be: 20190828T111500 which is 100 days
 *          after May 20, 2019 (i.e., August 28, 2019).
 *
 * Parameters: char *after - address of string to store incremented date
 *             const char *before - date to be incremented
 *             int const num_days - number of days to increment
 *
 * Credit: Function copied from "timeplay.c" provided by Michael Zastre.
 */

void increment_date(char *after, const char *before, int const num_days){

    struct tm temp_time, *p_temp_time;
    time_t    full_time;
    char      temp[5];

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, MAX_LINE_LEN - 8); 
    after[MAX_LINE_LEN - 1] = '\0';
}


/*
 * Function: decrement_date()
 *
 * Purpose: Given a date-time, it subtracts the number of days in a way that
 *          results in the correct year, month, and day. For example,
 *          if the string in "before" corresponds to: 20190520T111500
 *          then the datetime string stored in "after", assuming that
 *          "num_days" is 100, will be: 20190409T111500 which is 100 days
 *          before May 20, 2019 (i.e., February 9, 2019).
 *
 * Parameters: char *after - address of string to store decremented date
 *             const char *before - date to be decremeted
 *             int const num_days - number of days to decrement
 *
 * Credit: Function modified from "timeplay.c" provided by Michael Zastre.
 */

void decrement_date(char *after, const char *before, int const num_days){

    struct tm temp_time, *p_temp_time;
    time_t    full_time;
    char      temp[5];

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday -= num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, MAX_LINE_LEN - 8); 
    after[MAX_LINE_LEN - 1] = '\0';
}


/* 
 * Function: concatenate() 
 * 
 * Purpose: Given a year, month, and day, concatenates them
 *          together to form a date. 
 * 
 * Parameters: int year, month, date - values to concatenate
 * 
 * Returns: int - concatenated date
 * 
 * Credit: Inspired by funnction provided on "GeeksForGeeks: How to concatenate
 *         two integer values into one?" by Ayush Singla
 */

int concatenate(int year, int month, int day){

    char y[10];
    char m[10];
    char d[10];
    sprintf(y, "%d", year);
    sprintf(m, "%02d", month);
    sprintf(d, "%02d", day);
    strcat(m, d);
    strcat(y, m);
    return atoi(y);
}

