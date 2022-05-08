#ifndef _ICS_H_
#define _ICS_H_

#define DT_LEN       16
#define TM_LEN       8
#define MAX_LEN      80

typedef struct event_t{
    char dtstart[DT_LEN];
    char tmstart[TM_LEN];
    char dtend[DT_LEN];
    char tmend[TM_LEN];
    char summary[MAX_LEN];
    char location[MAX_LEN];
    char rrule[DT_LEN];
} event_t;

#endif
