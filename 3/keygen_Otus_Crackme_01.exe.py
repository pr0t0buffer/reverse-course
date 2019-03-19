"""
Keygen for Otus_Crackme_01.exe (fb42bfad815a9563b9f6fdd362b47f70)
"""

import string
from random import *
min_char = 8
max_char = 12

login = input("Input login: ")
login_result = 0xffffffff

for char in login:
	if(not char.isalnum()):
		exit("Login should contain only letters (A-Z/a-z) and digits (0-9)")
	login_result ^= ord(char)
	for i in range(8):
		login_result = -(login_result & 1) & 0xedb88320 ^ (login_result >> 1)
login_result = ~login_result

allchar = string.ascii_letters + string.digits

while(True):
	password = "".join(choice(allchar) for x in range(randint(min_char, max_char)))
	password_result = 0
	for index in range(0,len(password)):
		password_result += ord(password[index]) ^ 0x99
	if(hex(login_result & 0xff) == hex(password_result & 0xff)):
		print("For login %s password is: %s" % (login, password))
		break;




