#!/usr/bin/env python

import argparse
import json
import numpy as np
import datetime
import serial
import io
import time
import pdb
import matplotlib.pyplot as plt
import matplotlib.dates
from matplotlib.dates import DayLocator, HourLocator, DateFormatter, drange

YEAR = 2013
DAYSOFWEEK = ('Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun')


def dump_eeprom(tty, fout):
    # dump the EEPROM from tty into a file determined by fout
    ser = serial.Serial(tty, baudrate=115200,
                    bytesize=8, parity='N', stopbits=1,
                    xonxoff=0, rtscts=1, timeout=5)

    # buffer size is 1 byte, so directly passed to TextIOWrapper
    sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser, 1), encoding='ascii')
    #sio.flush()

    reading_json = False
    done_reading = False
    json_rows = []


    while(not done_reading):
        # handle serial exceptions
        try:
            readline = sio.readline()
        except Exception as e:
            print ("ERROR: encountered issue reading from serial tty")
            return

        # handle empty buffer?    
        if (len(readline) == 0 ):
            print ("read a line of size of 0")

        # handle end of JSON
        if (readline.find(r'ENDJSON') >= 0 ):
            print "...found end of json"
            reading_json = False
            done_reading = True

        # handle JSON meat
        if (reading_json):
            json_rows.append(readline)

        # handle beginning of JSON
        if (readline.find(r'STARTJSON') >= 0 ):
            print "found beginning of json..."
            # reset everything in buffer
            json_rows = []
            reading_json = True

    # convert json into actual object
    json_string = ''.join(json_rows)
    json_object = json.loads(json_string)

    # make the file
    with open(fout, 'w+') as outfile:
        json.dump(json_object, outfile)



def auto_label(rects, axis):
    # attach some text labels on top of rect
    for rect in rects:
        height = rect.get_height()
        axis.text(rect.get_x()+rect.get_width()/2., 
        	1.05*height, '%d'%int(height),ha='center', va='bottom')

def make_plots(json_file):

    metaData = dict.fromkeys(DAYSOFWEEK)
    ind = np.arange(7)  # the x locations for the groups
    width = 0.35        # the width of the bars

    #setup dict
    for day in metaData:
        metaData[day] = []

    # read in the supplied timestamp data and make data for each day
    with open(json_file, 'r') as infile:
        ts_json = json.load(infile)

    # iterate through input JSON
    for ts in ts_json:
        dow = ts_json[ts]['DOW'] - 1
    	t = datetime.time(ts_json[ts]['Hour'], ts_json[ts]['Minute'],0)
    	d = datetime.date(YEAR, ts_json[ts]['Month'], ts_json[ts]['DOW'])
        date = datetime.datetime.combine(d, t)
        #@TODO: upgrade to a logger
        print (date.strftime("Hit at: %H:%M:%S on %d/%m/%Y"))
    	metaData[DAYSOFWEEK[dow]].append(date)

    #TIMELINE PLOT
    trigger_times_list=[]
    for day in metaData:
        trigger_times_list.extend(metaData[day])

    trigger_quantities_list = len(trigger_times_list) * [1]

    fig, ax = plt.subplots()
    ax.plot_date(trigger_times_list,trigger_quantities_list, 'ro')
    ax.xaxis.set_major_locator( DayLocator() )
    ax.xaxis.set_minor_locator( HourLocator() )
    ax.xaxis.set_major_formatter( DateFormatter('%Y-%m-%d') )
    ax.fmt_xdata = DateFormatter('%Y-%m-%d %H:%M:%S')
    ax.xaxis.grid(True, which='minor') 
    ax.autoscale_view()
    plt.gcf().autofmt_xdate()

    plt.show()




def main():
    parser = argparse.ArgumentParser(description='a parser for EEPROM security monitors')
    parser.add_argument('--download', action='store_true', required=False)
    parser.add_argument('--tty', type=str, help='the tty of your countduino')
    parser.add_argument('--fout', type=str, required=False, default='/var/tmp/time.json', help='the location of your graphs and EEPROM dump')
    args = parser.parse_args()

    if(args.download and args.tty is None):
        print "must specify a \"tty [/dev/tty...]\""
        return

    if (args.download):
        dump_eeprom(args.tty, args.fout)
    
    make_plots(args.fout)



if __name__ == '__main__':
    main()


