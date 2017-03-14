for line in cap_file:
	line = line.replace('\r', '').replace('\n', '')
	line_split = line.split(',')
	timestamp = float(line_split[0])
	data = int(line_split[2], 16)
	print(line)
	print(line_split[2], end = ' ')
	print(data, end = ' ')
	master_list.append(data)
	if timestamp - prev_ts > 0.05:
		master_list.append('|')
	prev_ts = timestamp

# for item in master_list:
# 	if item == '|':
# 		print()
# 	else:
# 		print(hex(item), end=' ')