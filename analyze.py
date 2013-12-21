#!/usr/bin/env python

import argparse
import json
import numpy as np
import matplotlib.pyplot as plt
import datetime
import serial
import io
import time
import pdb


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

    metaData = dict.fromkeys(DAYSOFWEEK,{'ts_list':[],'avg_tod':0})
    ind = np.arange(7)  # the x locations for the groups
    width = 0.35        # the width of the bars

    # read in the supplied timestamp data and make data for each day
    with open(json_file, 'r') as infile:
        ts_json = json.load(infile)


    for ts in ts_json:
    	day = ts_json[ts]['DOW'] - 1
    	t = datetime.time(ts_json[ts]['Hour'], ts_json[ts]['Minute'],0)
    	d = datetime.date(YEAR, ts_json[ts]['Month'], ts_json[ts]['DOW']) #I AM ABUSING DAY OF MONTH!
    	metaData[DAYSOFWEEK[day]]['ts_list'] = datetime.datetime.combine(d, t) #.append(datetime.datetime.combine(d, t))

    dayCumil=[]
    for day in metaData:
    	dayCumil.append(len(metaData[day]['ts_list']))


    # cumulative plot
    fig_c, ax_c = plt.subplots()
    rects_c = ax_c.bar(ind, dayCumil, width, color='r')

    ax_c.set_ylabel('Number of Activations')
    ax_c.set_title('Quantity of Activations per Day')
    ax_c.set_xticks(ind+width)
    ax_c.set_xticklabels( DAYSOFWEEK )

    autolabel(rects_c, ax_c)

    # median times plot
    fig_m, ax_m = plt.subplots()
    rects_m = ax_m.bar(ind, dayAvgs, width, color='b')

    ax_m.set_ylabel('Time of Activation (24 hour)')
    ax_m.set_title('Median Time of Activation versus Day')
    ax_m.set_xticks(ind+width)
    ax_m.set_xticklabels( DAYSOFWEEK )

    auto_label(rects_m, ax_m)

    plt.show()


def main():
    parser = argparse.ArgumentParser(description='a parser for EEPROM security monitors')
    parser.add_argument('--tty', type=str, help='the tty of your countduino')
    parser.add_argument('--fout', type=str, required=False, default='/var/tmp/time.json', help='the location of your graphs and EEPROM dump')
    args = parser.parse_args()

    dump_eeprom(args.tty, args.fout)
    # still working on the plot feature
    #make_plots(args.fout)



if __name__ == '__main__':
    main()


