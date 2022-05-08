#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emalloc.h"
#include "ics.h"
#include "listy.h"

node_t *extract(char *);
void expand(node_t *, void *);
void print_events(node_t *, void *, int, int, int);
void print(node_t *);
void output(node_t *, void *, int, int);
void pdate(char *, const char *, const int);
void pline(char *);
void psumm(node_t *);
void freeall(node_t *);
void increment_date(char *, const char *, int const);
void decrement_date(char *, const char *, int const);
int within_range(int, int, node_t *);
int unique_event(node_t *);
int first_repeat(node_t *);
int mid_repeat(node_t *);
int concatenate(int, int, int);

int main(int argc, char *argv[]){

    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i;

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 7) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 5) == 0) {
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

    int from = concatenate(from_y, from_m, from_d);
    int to = concatenate(to_y, to_m, to_d);
    int op = 0;
    int inc = 0;

    node_t *head = extract(filename);
    r_apply(head, expand, NULL);
    apply(head, output, &op, from, to);
    p_apply(head, print_events, &inc, from, to, op);
    freeall(head);

    exit(0);
}


/* Function:   extract()
 * Parameters: char *filename - name of file
 * Purpose:    Reads data from a file using getline(), creates events
 *             from that data using strtok() and strncpy(), then adds the events
 *             onto a doubly-linked list.
 * Returns:    node_t *head - head of a list containing all events from the file
 */
node_t *extract(char *filename){

    char *token, *line = NULL;
    size_t size = 0;
    ssize_t read;
    event_t *event = NULL;
    node_t *calendar = NULL, *head = NULL;

    FILE *fptr = fopen(filename, "r");
    if(fptr == NULL){
        fprintf(stderr, "unable to open %s\n", filename);
        exit(1);
    }

    while((read = getline(&line, &size, fptr)) != -1){
        token = strtok(line, ":");
        while(token){
            if(strcmp(token, "DTSTART") == 0){
                event = emalloc(sizeof(event_t));
                strncpy(event->rrule, "", DT_LEN);
                token = strtok(NULL, "T");
                strncpy(event->dtstart, token, DT_LEN);
                token = strtok(NULL, "\n");
                strncpy(event->tmstart, token, TM_LEN);
            }
            if(strcmp(token, "DTEND") == 0){
                token = strtok(NULL, "T");
                strncpy(event->dtend, token, DT_LEN);
                token = strtok(NULL, "\n");
                strncpy(event->tmend, token, TM_LEN);
            }
            if(strcmp(token, "SUMMARY") == 0){
                token = strtok(NULL, "");
                strncpy(event->summary, token, MAX_LEN);
                event->summary[strcspn(event->summary, "\n")] = 0;
            }
            if(strcmp(token, "LOCATION") == 0){
                token = strtok(NULL, "");
                strncpy(event->location, token, MAX_LEN);
                event->location[strcspn(event->location, "\n")] = 0;

            }
            if(strcmp(token, "RRULE") == 0){
                token = strtok(NULL, "N");
                token = strtok(NULL, "=");
                token = strtok(NULL, "T");
                strncpy(event->rrule, token, DT_LEN);
            }
            if(strcmp(token, "END") == 0){
                token = strtok(NULL, "\n");
                if(strcmp(token, "VEVENT") == 0){
                    calendar = new_node(event);
                    head = insert(head, calendar);
                }
            }
            token = strtok(NULL, "\n");
        }
    }
    fclose(fptr);
    if(line) free(line);
    return head;
}


/* Function:   print_events()
 * Parameters: node_t *n - head of a list
 *             void *arg - address to a void
 *             int from - output start date
 *             int to - output end date
 *             int op - # of events to be output within date range
 * Purpose:    uses p_apply() to iterate through the linked list,
 *             printing events within range in a readable format. 
 */
void print_events(node_t *n, void *arg, int from, int to, int op){
    assert(n != NULL);
    
    node_t *e = n;
    int *i, final_event = 0;
    i = (int *)arg;
    
    if(within_range(from, to, e)){
        (*i)++; if(op == *i) final_event = 1;
        if(unique_event(e)){
            print(e);
            if(!final_event) printf("\n");
        }else if(first_repeat(e)){
            print(e);
        }else if(mid_repeat(e)){
            psumm(e);
        }else{
            psumm(e);
            if(!final_event) printf("\n");
        }
    }
}


/* Function:   within_range()
 * Parameters: node_t *e - node containing event information
 *             int from - output start date
 *             int to - output end date
 * Purpose:    Determines whether an event is within date range
 * Returns:    int - 0 or 1, false or true respectively
 */
int within_range(int from, int to, node_t *e){
    if(from <= atoi(e->val->dtstart) &&
     atoi(e->val->dtstart) <= to) return 1;
    return 0;
}


/* Function:   unique_event()
 * Parameters: node_t *e - node containing event information
 * Purpose:    Determines whether an event is unique, i.e. it does
 *             not have any additional events occuring on that day
 * Returns:    int - 0 or 1, false or true respectively
 */
int unique_event(node_t *e){
    if(e->prev == NULL && e->next == NULL) return 1;
    if(e->prev == NULL && e->next != NULL){
        if(atoi(e->next->val->dtstart) != atoi(e->val->dtstart)) return 1;
    }
    if(e->prev != NULL && e->next != NULL){
        if(atoi(e->next->val->dtstart) != atoi(e->val->dtstart) &&
         atoi(e->prev->val->dtstart) != atoi(e->val->dtstart)) return 1;
    }
    if(e->prev != NULL && e->next == NULL){
        if(atoi(e->prev->val->dtstart) != atoi(e->val->dtstart)) return 1;
    }
    return 0;
}


/* Function:   first_repeat()
 * Parameters: node_t *e - node containing event information
 * Purpose:    Determines whether an event is the first event on
 *             day with more than one events occuring on that day
 * Returns:    int - 0 or 1, false or true respectively
 */
int first_repeat(node_t *e){
    if(e->prev == NULL && e->next != NULL){
        if(atoi(e->next->val->dtstart) == atoi(e->val->dtstart)) return 1;
    }
    if(e->prev != NULL && e->next != NULL){
        if(atoi(e->next->val->dtstart) == atoi(e->val->dtstart) &&
         atoi(e->prev->val->dtstart) != atoi(e->val->dtstart)) return 1;
    }
    return 0;
}


/* Function:   mid_repeat()
 * Parameters: node_t *e - node containing event information
 * Purpose:    Determines whether an event is between two events
 *             occurring on the same day
 * Returns:    int - 0 or 1, false or true respectively
 */
int mid_repeat(node_t *e){
    if(e->prev != NULL && e->next != NULL){
        if(atoi(e->next->val->dtstart) == atoi(e->val->dtstart) &&
         atoi(e->prev->val->dtstart) == atoi(e->val->dtstart)) return 1;
    }
    return 0;
}


/* Function:   expand()
 * Parameters: node_t *n - head of a list
 *             void *arg - address to a void
 * Purpose:    uses r_apply() to iterate through the linked list,
 *             adding any repeating events to the list. 
 */
void expand(node_t *n, void *arg){
    assert(n != NULL);

    event_t *event = n->val;
    event_t *new_event = NULL;
    node_t *temp = NULL;
    char cur_date[DT_LEN], inc_date[DT_LEN], dec_date[DT_LEN];

    if(*event->rrule != '\0'){
        decrement_date(dec_date, event->rrule, 7);
        strncpy(cur_date, event->dtstart, DT_LEN);
        while(atoi(cur_date) <= atoi(dec_date)){
            increment_date(inc_date, cur_date, 7);
            new_event = emalloc(sizeof(event_t));
            strncpy(new_event->dtstart, inc_date, DT_LEN);
            strncpy(new_event->tmstart, event->tmstart, TM_LEN);
            strncpy(new_event->dtend, inc_date, DT_LEN);
            strncpy(new_event->tmend, event->tmend, TM_LEN);
            strncpy(new_event->rrule, "", DT_LEN);
            strncpy(new_event->summary, event->summary, MAX_LEN);
            strncpy(new_event->location, event->location, MAX_LEN);
            temp = new_node(new_event);
            n = insert(n, temp);
            strncpy(cur_date, inc_date, DT_LEN);
        }
    } 
}


/* Function:   output()
 * Parameters: node_t *n - head of a list
 *             void *arg - address to a void
 *             int from - output start date
 *             int to - output end date
 * Purpose:    uses apply() to iterate through the linked list,
 *             counting the number of events within range. 
 */
void output(node_t *n, void *arg, int from, int to){
    assert(n != NULL);
    node_t *e = n;
    int *op = (int *)arg;
    if(within_range(from, to, e)) (*op)++;
}


/* Function:   freeall()
 * Parameters: node_t *list - head of a list
 * Purpose:    frees all dynamically allocated memory in the list.
 * Credit:     Michael Zastre, C-dynamic memory lecture (slide 108)
 */
void freeall(node_t *list){
    node_t *next = NULL;
    for(; list != NULL; list = next){
        next = list->next;
        free(list->val);
        free(list);
    }
}


/* Function:   print()
 * Parameters: node_t *e - node containing the event to print
 * Purpose:    calls pdate(), pline(), and psumm() to print an event 
 */
void print(node_t *e){
    char ft[DT_LEN];
    pdate(ft, e->val->dtstart, MAX_LEN);
    pline(ft);
    psumm(e);
}


/* Function:   pdate()
 * Parameters: char *formatted_time - address of a string to store the output
 *             cont char *dt_time - address of a string containing the the date
 *             const int len - length of the string at *dt_time
 * Purpose:    Given a date-time, creates a more readable version of the
 *             calendar date by using some C-library routines. For example,
 *             if the string in "dt_time" corresponds to: 20190520T111500
 *             then the string stored at "formatted_time" is: May 20, 2019 (Mon).
 * Credit:     Michael Zastre, timeplay.c
 */
void pdate(char *formatted_time, const char *dt_time, const int len){

    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_time, len, "%B %d, %Y (%a)", localtime(&full_time));
    printf("%s\n", formatted_time);
}


/* Function:   pline()
 * Parameters: char *output - string containing the date
 * Purpose:    Prints a line with "-" mathching the size of a specified date
 */
void pline(char *output){
    for(int i = 0; i < strlen(output); i++){
        printf("-");
    }
    printf("\n");
}


/* Function:   psumm()
 * Parameters: node_t *e - node containing event informatio
 * Purpose:    Converts the 24 hour time of an event into 12 hour time and prints
 *             it along with the summary and location of the event.
 */
void psumm(node_t *e){

    int start = atoi(e->val->tmstart)/100; 
    int end = atoi(e->val->tmend)/100;
    int start_min = start % 100;
    int end_min = end % 100;
    char *start_period;
    char *end_period;

    start /= 100;
    end /= 100;

    if(start == 0){ start = 12; start_period = "AM"; }
    else if(start == 12){ start_period = "PM"; }
    else if(start > 12){ start -= 12; start_period = "PM"; }
    else{ start_period = "AM"; }

    if(end == 0){ end = 12; end_period = "AM"; }
    else if(end == 12){ end_period = "PM"; }
    else if(end > 12){ end -= 12; end_period = "PM"; }
    else{ end_period = "AM"; }

    if(start < 10 && end < 10){
        printf(" %d:%02d %s to  %d:%02d %s: %s {{%s}}\n", 
            start, start_min, start_period, end, end_min, end_period,
            e->val->summary, e->val->location);
    } else if(start < 10 && end >= 10){
        printf(" %d:%02d %s to %d:%02d %s: %s {{%s}}\n",
            start, start_min, start_period, end, end_min, end_period,
            e->val->summary, e->val->location);
    } else if(start >= 10 && end < 10){
        printf("%d:%02d %s to  %d:%02d %s: %s {{%s}}\n",
            start, start_min, start_period, end, end_min, end_period,
            e->val->summary, e->val->location);
    } else if(start >= 10 && end >= 10){
        printf("%d:%02d %s to %d:%02d %s: %s {{%s}}\n",
            start, start_min, start_period, end, end_min, end_period,
            e->val->summary, e->val->location);
    }
}


/* Function:   increment_date()
 * Parameters: char *after - address of string to store incremented date
 *             const char *before - date to be incremented
 *             int const num_days - number of days to increment
 * Purpose:    Given a date-time, it adds the number of days in a way that
 *             results in the correct year, month, and day. For example,
 *             if the string in "before" corresponds to: 20190520T111500
 *             then the datetime string stored in "after", assuming that
 *             "num_days" is 100, will be: 20190828T111500 which is 100 days
 *             after May 20, 2019 (i.e., August 28, 2019).
 * Credit:     Michael Zastre, timeplay.c
 */
void increment_date(char *after, const char *before, int const num_days){

    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += num_days;
    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, DT_LEN - 8);
    after[DT_LEN - 1] = '\0';
}


/* Function:   decrement_date()
 * Parameters: char *after - address of string to store decremented date
 *             const char *before - date to be decremeted
 *             int const num_days - number of days to decrement
 * Purpose:    Given a date-time, it subtracts the number of days in a way that
 *             results in the correct year, month, and day. For example,
 *             if the string in "before" corresponds to: 20190520T111500
 *             then the datetime string stored in "after", assuming that
 *             "num_days" is 100, will be: 20190409T111500 which is 100 days
 *             before May 20, 2019 (i.e., February 9, 2019).
 * Credit:     Michael Zastre, timeplay.c
 */
void decrement_date(char *after, const char *before, int const num_days){

    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday -= num_days;
    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, DT_LEN - 8);
    after[DT_LEN - 1] = '\0';
}


/* Function:   concatenate()
 * Parameters: int year, month, date - values to concatenate
 * Purpose:    Given a year, month, and day, concatenates them
 *             together to form a date.
 * Returns:    int - concatenated date
 * Credit:     Ayush Singla, GeeksForGeeks
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
