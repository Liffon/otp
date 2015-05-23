GPP = g++ -g -Wall -Wno-unused-variable -Wno-unused-but-set-variable -pedantic -std=gnu++0x

hmac: *.cpp *.h
	$(GPP) hmactest.cpp

otp: *.cpp *.h
	$(GPP) otp.cpp -o otp

clean:
	rm -f ./otp
	rm -f ./a.out
