#!/usr/bin/env python3

import sys
import argparse
import datetime

# The code below configures the argparse module for use with assignment #2.

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--start', type=str, help='start of date range')
    parser.add_argument('--end', type=str, help='end of data range')
    parser.add_argument('--file', type=str, help='file to be processed')

    args = parser.parse_args()

    if not args.start:
        print("Need --start=yyyy/mm/dd")

    if not args.end:
        print("Need --end=yyyy/mm/dd")

    if not args.file:
        print("Need --file=<ics filename>")
    
    # Create datetime objects for the start and end date
    start = start_range(args.start)
    end = end_range(args.end)
    extract(args.file, start, end)    


def extract(filename, start, end):
    """
    Purpose: Given a filename, opens the file and stores its contents onto
             a List of Dictionaries. 

    Parameters: filename - name of file
                start - start of date range to print events
                end - end of date range to print events
    """    
    calendar = []

    f = open(filename, "r")
    if f == None:
        print("Invalid Filename. Please try again.")
        sys.exit(1)

    # Traverse through the file line by line storing important information in a dictionary
    for line in f:
        if "BEGIN:VEVENT" in line:
            event = {'start':'', 'end':'', 'repeat':'', 'location':'', 'summary':''}
        if "DTSTART" in line:
            temp = line.split(":")
            event['start'] = temp[1].rstrip('\n')
        elif "DTEND" in line:
            temp = line.split(":")
            event['end'] = temp[1].rstrip('\n')
        elif "RRULE" in line:
            temp = line.split("UNTIL=")[1]
            temp_extra = temp.split(";")[0]
            event['repeat'] = temp_extra
        elif "LOCATION" in line:
            temp = line.split(":")
            event['location'] = temp[1].rstrip('\n')
        elif "SUMMARY" in line:
            temp = line.split(":")
            event['summary'] = temp[1].rstrip('\n')
        elif "END:VEVENT" in line:
            calendar.append(event)    

    f.close()
    
    expand_and_sort(calendar, start, end)


def expand_and_sort(c, start, end):
    """
    Purpose: If any repeating events are found in extract(), adds
             them to the back of the list until the repeat date.
             Then, sorts the list by the event start date and time. 

    Parameters: c - (calendar) list containing all events
                start - start date for output 
                end - end date for output 
    """

    # Traverse through the list
    for x in c:
        # If any repeating events found, append them to the list
        if (x['repeat'] != ''):
            dt = to_datetime(x['start'])
            last_dt = to_datetime(x['repeat'])
            while dt < last_dt - datetime.timedelta(7):
                dt += datetime.timedelta(7)
                event = {'start':'', 'end':'', 'location':'', 'summary':''}
                event['start'] = dt.strftime("%Y%m%dT%H%M")
                event['end'] = x['end']
                event['repeat'] = ''
                event['location'] = x['location']
                event['summary'] = x['summary']
                c.append(event)

    # Sort the list by start date using datetime to account for start time as well
    c.sort(key = lambda i: to_datetime(i['start']))    
    print_events(c, start, end)

            
def print_events(c, start, end):
    """
    Purpose: Given a list of events, prints them in a readable format
             within the user specified date range. 

    Parameters: c - (calendar) list containing all events
                start - start date for date range
                end - end date for date range
    """
    output = 0
    increment = 0

    # Output variable to track valid events in the date range
    for x in c:
        if to_datetime(x['start']) >= start and to_datetime(x['start']) <= end:
            output += 1

    # Dummy event appended to the list to prevent an IndexError        
    dummy_event = {'start':'20300101T000000', 'end':'20300101T000000', 'repeat':'', 'location':'', 'summary':''}
    c.append(dummy_event)

    # Traverse through the list using an int counter
    for i in range(0, len(c)):
        # If event is in the specified date range
        if to_datetime(c[i]['start']) >= start and to_datetime(c[i]['start']) <= end:
            increment += 1
            # If list contains one event
            if(len(c) == 1):
                print_date(c[i])
                print_line(c[i])
                print_summary(c[i])
            # Else list contains multiple events
            else:
                # If the next event occurs on the same day and the previous event occured on a different day
                if c[i]['start'].split("T")[0] == c[i+1]['start'].split("T")[0] and c[i]['start'].split("T")[0] != c[i-1]['start'].split("T")[0]:
                    print_date(c[i])
                    print_line(c[i])
                    print_summary(c[i])
                # Else if the previous event was on the same day
                elif c[i]['start'].split("T")[0] == c[i-1]['start'].split("T")[0]:
                    print_summary(c[i])
                    # If there are more events in the list and the next event is on a different day, print a line
                    if increment != output and c[i]['start'].split("T")[0] != c[i+1]['start'].split("T")[0]:
                        print()
                # Else the event occurs on a different day 
                else:
                    print_date(c[i])
                    print_line(c[i])
                    print_summary(c[i])
                    # If there are more events, print a line
                    if increment != output:
                        print()

        
def print_date(i):
    """
    Purpose: Prints the date of a specified event in a readable format
    
    Parameter: i - Dictionary containing event information
    """
    dt = to_datetime(i['start'])
    print(dt.strftime("%B %d, %Y (%a)"))


def print_line(i):
    """
    Purpose: Prints a dashed line corresponding to the length of a formatted date

    Paramter: i - Dictionary containing event information
    """
    dt = to_datetime(i['start'])
    line = dt.strftime("%B %d, %Y (%a)")
    for l in range(len(line)):
        print("-", end="")
    print()


def print_summary(i):
    """
    Purpose: Prints the time, location, and summary of an event

    Parameter: i - Dictionary containing event information
    """
    s = to_datetime(i['start'])
    e = to_datetime(i['end'])
    start = int(s.strftime("%I"))
    end = int(e.strftime("%I"))
    if start < 10 and end < 10:
        print(" " + str(start) + s.strftime(":%M %p to ") + " " + str(end) + e.strftime(":%M %p: ") + i['summary'] + " {{" + i['location'] + "}}")
    elif start < 10 and end >= 10: 
        print(" " + str(start) + s.strftime(":%M %p to ") + e.strftime("%I:%M %p: ") + i['summary'] + " {{" + i['location'] + "}}")
    elif start >= 10 and end < 10:
        print(s.strftime("%I:%M %p to ") + " " + str(end) + e.strftime(":%M %p: ") + i['summary'] + " {{" + i['location'] + "}}")
    elif start >= 10 and end >= 10:
        print(s.strftime("%I:%M %p to ") + e.strftime("%I:%M %p: ") + i['summary'] + " {{" + i['location'] + "}}")
    

def start_range(date):
    """
    Purpose: Given a start date, creates a datetime object
    
    Parameter: date - user specified start date
    """
    temp = date.split('/')
    y = int(temp[0])
    m = int(temp[1])
    d = int(temp[2])
    return datetime.datetime(y, m, d)


def end_range(date):
    """
    Purpose: Given an end date, creates a datetime object 

    Parameter: date - user specifed end date
    """
    temp = date.split('/')
    y = int(temp[0])
    m = int(temp[1])
    d = int(temp[2])
    return datetime.datetime(y, m, d, 23, 59)
    

def to_datetime(i):
    """
    Purpose: Given a string, creates a datetime object
    
    Parameter: i - Dictionary value containing date as a String
    """
    y = int(i[:4])
    m = int(i[4:6])
    d = int(i[6:8])
    h = int(i[9:11])
    mi = int(i[11:13]) 
    return datetime.datetime(y, m, d, h, mi)


if  __name__ == "__main__":
    main()
