GPP = g++ -g -Wall -Wno-unused-variable -Wno-unused-but-set-variable -pedantic -std=gnu++0x

otp: *.cpp *.h
	$(GPP) otp.cpp -o otp

clean:
	rm -f ./otp
	rm -f ./a.out
