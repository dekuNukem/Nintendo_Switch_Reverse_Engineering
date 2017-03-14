import sys

c2j_file = open("c2j.txt")
j2c_file = open("j2c.txt")

while 1:
	c2j_line = c2j_file.readline().replace('\r', '').replace('\n', '')
	j2c_line = j2c_file.readline().replace('\r', '').replace('\n', '')
	if c2j_line == '' or j2c_line == '':
		exit()

	print("C: " + c2j_line)
	print("J: " + j2c_line)
	print('\n')
