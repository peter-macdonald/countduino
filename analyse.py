#!/usr/bin/env python

import json
import numpy as np
import matplotlib.pyplot as plt
import datetime

YEAR = 2013
DAYSOFWEEK = ('Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun')
metaData = dict.fromkeys(DAYSOFWEEK,{'ts_list':[],'avg_tod':0})
ind = np.arange(7)  # the x locations for the groups
width = 0.35        # the width of the bars

def auto_label(rects, axis):
    # attach some text labels on top of rect
    for rect in rects:
        height = rect.get_height()
        axis.text(rect.get_x()+rect.get_width()/2., 
        	1.05*height, '%d'%int(height),ha='center', va='bottom')

# read in the supplied timestamp data and make data for each day
ts_json = json.load(open('ts_json_num'))

import pdb
pdb.set_trace()

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