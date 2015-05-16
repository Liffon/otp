otp: otp.cpp
	g++ otp.cpp -g -o otp -Wall -Wno-unused-variable -Wno-unused-but-set-variable -pedantic -std=gnu++0x

clean:
	rm -f ./otp
