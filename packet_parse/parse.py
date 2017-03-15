import sys

if len(sys.argv) != 2:
	print(__file__ + " filename")
	exit()

master_list = []
prev_ts = 0.10890256
cap_file = open(sys.argv[1])

for line in cap_file:
	line = line.replace('\r', '').replace('\n', '')
	line_split = line.split(',')
	timestamp = float(line_split[0])
	data = int(line_split[2], 16)
	if timestamp - prev_ts > 0.0005:
		master_list.append('|')
	master_list.append(data)
	prev_ts = timestamp

for item in master_list:
	if item == '|':
		print()
	else:
		print(hex(item), end=' ')

