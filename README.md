# ICS-File-Processor
Process ICS (iCalendar) files to output a readable summary of the events using C and Python. 

## Version 1 (C)
You are to write a C program that inputs lines of data from a calendar file provided, accepts options and arguments from the command line, and then outputs to the console events from the calendar file into a more readable form.

ICS files will have a format similar to the following `one.ics` file:

```
BEGIN:VCALENDAR
BEGIN:VEVENT 
DTSTART:20210214T180000 
DTEND:20210214T210000 
LOCATION:Burger King 
SUMMARY:Romantic dinner with Chris 
END:VEVENT
END:VCALENDAR
```
This particular file contains information for a fictional single event taking place on February 14, 2021. Let us suppose you type the following arguments into your program:

```
./icsout --start=2021/2/14 --end=2021/2/14 --file=one.ics
```
then the output to be produced is as follows:
```
February 14, 2021 (Sun) 
-----------------------
6:00 PM to 9:00 PM: Romantic dinner with Chris {{Burger King}}
```
Note that the arguments passed the program to the were two dates (year/month/yeadayr) plus a filename. The output reflects that the script printed out all events within the iCal file occurring during the specified range of dates (which, in this case, ends up being a single event).

Another possibility is for events to be repeating. For example, an event such as regular coffee appointments with a relative is represented as the single entry in the file shown below. (In order to simplify your implementation of icsout, we will use only iCal weekly frequencies for a single day in a week rather than more general repetition possibilities that can be expressed using the iCalendar standard.) This repetition is represented with use of the repeating-event rule, as seen in another provided test file name many.ics. Consider the contents of this file, followed by the use of icsout to produce a readable version of events within the file:

```
ABEGIN:VCALENDAR
VERSION:A
BEGIN:VEVENT
DTSTART:20210102T111500
DTEND:20210102T123000 
RRULE:FREQ=WEEKLY;WKST=MO;UNTIL=20210301T235959;BYDAY=SA 
LOCATION:The Bumptious Barista
SUMMARY:Coffee with Pat 
END:VEVENT 
END:VCALENDAR
```

```
./icsout --start=2021/2/1 --end=2021/3/1 --file=many.ics
```

```
February 06, 2021 (Sat)
-----------------------
11:15 AM to 12:30 PM: Coffee with Pat {{The Bumptious Barista}}

February 13, 2021 (Sat)
-----------------------
11:15 AM to 12:30 PM: Coffee with Pat {{The Bumptious Barista}}

February 20, 2021 (Sat)
-----------------------
11:15 AM to 12:30 PM: Coffee with Pat {{The Bumptious Barista}}

February 27, 2021 (Sat)
-----------------------
11:15 AM to 12:30 PM: Coffee with Pat {{The Bumptious Barista}}
```

## Version 2 (Python)
This version solves the same problem as Version 1, implemented in Python. The arguments used for the Python script are the same as used for version #1. That is, the range of dates to be used to generate the schedule is providing by “--start” and “--end” arguments. However, the executable will now be named “icsout2.py” (and not “icsout.py”).
```
./icsout2.py --start=2021/2/14 --end=2021/2/14 --file=one.ics
```
Additionally, this version contains four constraints for the implementation:
1. You are not to use **regular expressions**.
2. You must not write your own **classes**.
3. You must not use **global variables**.
4. You must make good use of **functional decomposition**.

## Version 3 (C)
This version is an implementation of the solution in C with the constraints that:
* ***only*** dynamic memory is used to store event info, and
* ***only*** linked-list routines are used (i.e. arrays of events are not permitted).
A call to icsout3 will use identical arguments to that from the previous assignment. For example:
```
./icsout3 --start=2021/2/14 --end=2021/2/14 --file=one.ics
```

## Version 4 (Python)
You are to complete the implementation of the class ICSout which will be contained in the file named icsout4.py. Unlike the three previous assignments, however, your own code will be called by a test-driver program named tester4.py.
The two methods of the class ICSout that must be implemented are:
* A constructor which takes a single parameter (i.e., the name of the ICS file)
* A method named get_events_for_day which takes a datetime object as its single parameter. If the day corresponding to the parameter has events (i.e., were in the ICS file whose named was given to the object’s constructor), then the method returns a string with that day’s events formatted as required in previous assignments (including heading for date plus line of dashes). If the day corresponding to the parameter has no events, then None is returned by the method.

A call to tester4.py will used identical arguments to that from the previous versions. For example:
```
./tester4.py --start=2021/01/24 --end=2021/4/15 --file=one.ics
```
A few additional constraints: 
* you are not permitted to use any ***global variables*** 
* You must use ***regular expressions***
* you must not use ***global variables***
* You must make good use of ***functional decomposition***

## Submission Score
100%
