from random import randint

fo = open("data.txt", "wb")

for i in range(100000):
	fo.write(str(randint(-10000,10000)) + " ")

fo.close()
