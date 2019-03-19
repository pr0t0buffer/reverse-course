"""
Keygen for CRACKME.EXE_ (66f573036f8b99863d75743eff84f15d)
"""

login = input("Input login: ")
sum = 0
for index in range(0,len(login)):
	if( not login[index].isalpha() or not login[index].isupper() ):
		exit("Login should contain letters only between A and Z!")
	sum += ord(login[index])

sum ^= 0x5678
password = sum ^ 0x1234
print ("Password for login %s is %d" % (login, password))

