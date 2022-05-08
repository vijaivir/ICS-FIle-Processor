import re
import datetime

class ICSout:

    def __init__(self, filename):
        self.filename = filename
        self.extract()
        self.expand()
        self.sort()

    def get_events_for_day(self, dt):
        """
        Purpose: Takes a datetime object dt, and prints any events
                 occuring on that day in a human readable format.
        """
        out = ''
        for e in range(len(self.events)):
            if dt == self.todt(self.events[e]['dtstart']):
                out += (self.date(self.events[e]) +
                    self.line(self.events[e]) +
                    self.summ(self.events[e]))
                while (e != len(self.events) - 1 and
                    dt == self.todt(self.events[e+1]['dtstart'])):
                    out += self.summ(self.events[e+1])
                    e += 1
                break
        return out.rstrip()

    def extract(self):
        """
        Purpose: Opens the file and stores its contents onto
                 a list of dictionaries using regular expressions.
        """
        self.events = []
        f = open(self.filename, "r")
        pattern = re.compile(r"(.+):(.+)")
        r_pattern = re.compile(r"L=(.+);")
        for line in f:
            line = line.rstrip()
            m = pattern.search(line)
            if m:
                if m.group(1) == "BEGIN" and m.group(2) == "VEVENT":
                    event = {'dtstart':'',
                            'dtend':'',
                            'rrule':'No',
                            'location':'',
                            'summary':''}
                if m.group(1) == "DTSTART":
                    event['dtstart'] = m.group(2)
                if m.group(1) == "DTEND":
                    event['dtend'] = m.group(2)
                if m.group(1) == "RRULE":
                    r = r_pattern.search(m.group(2))
                    event['rrule'] = r.group(1)
                if m.group(1) == "LOCATION":
                    event['location'] = m.group(2)
                if m.group(1) == "SUMMARY":
                    event['summary'] = m.group(2)
                if m.group(1) == "END" and m.group(2) == "VEVENT":
                    self.events.append(event)
        f.close()

    def expand(self):
        """
        Purpose: Expands any repeating events contained in the list
                 and appends them to the back of the list.
        """
        for e in self.events:
            if e['rrule'] != 'No':
                dt = self.todtm(e['dtstart'])
                ld = self.todtm(e['rrule']) - datetime.timedelta(7)
                while(dt < ld):
                    dt += datetime.timedelta(7)
                    event = {'dtstart':'',
                            'dtend':'',
                            'rrule':'No',
                            'location':'',
                            'summary':''}
                    event['dtstart'] = dt.strftime("%Y%m%dT%H%M")
                    event['dtend'] = e['dtend']
                    event['location'] = e['location']
                    event['summary'] = e['summary']
                    self.events.append(event)

    def sort(self):
        """
        Purpose: Sorts the list of events by their start dates and times.
        """
        self.events.sort(key = lambda i: self.todtm(i['dtstart']))

    def todtm(self, e):
        """
        Purpose: Takes a string e, and returns a datetime object including
                 the days hour and minute.
        """
        m = re.search(r"(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})", e)
        return datetime.datetime(int(m.group(1)), int(m.group(2)),
               int(m.group(3)), int(m.group(4)), int(m.group(5)))

    def todt(self, e):
        """
        Purpose: Takes a string e, and returns a datetime object without
                 specifying the time.
        """
        m = re.search(r"(\d{4})(\d{2})(\d{2})", e)
        return datetime.datetime(int(m.group(1)), int(m.group(2)), int(m.group(3)))

    def date(self, e):
        """
        Purpose: Takes a dictionary item e representing an event,
                 and returns a string of its date in a readable format.
        """
        d = self.todtm(e['dtstart'])
        return d.strftime("%B %d, %Y (%a)") + '\n'

    def line(self, e):
        """
        Purpose: Takes a dictionary item e representing an event, and returns
                 a dashed line matching its date's readable format's length.
        """
        l = self.date(e)
        dash = ''
        for x in range(len(l) - 1):
            dash += '-'
        return dash + '\n'

    def summ(self, i):
        """
        Purpose: Takes a dictionary item i representing an event, and returns
                 a string containing the time, summary, and location of the event.  
        """
        s = self.todtm(i['dtstart'])
        e = self.todtm(i['dtend'])
        start = int(s.strftime("%I"))
        end = int(e.strftime("%I"))
        if start < 10 and end < 10:
            return (" "+str(start)+s.strftime(":%M %p to ")+" "+str(end)+
                    e.strftime(":%M %p: ")+i['summary']+" {{"+i['location']+"}}")+'\n'
        elif start < 10 and end >= 10:
            return (" " +str(start)+s.strftime(":%M %p to ")+e.strftime("%I:%M %p: ")+
                    i['summary']+" {{"+i['location']+"}}")+'\n'
        elif start >= 10 and end < 10:
            return (s.strftime("%I:%M %p to ")+" "+str(end)+e.strftime(":%M %p: ")+
                    i['summary']+" {{"+i['location']+"}}")+'\n'
        elif start >= 10 and end >= 10:
            return (s.strftime("%I:%M %p to ")+e.strftime("%I:%M %p: ")+
                    i['summary']+" {{"+i['location']+"}}")+'\n'
