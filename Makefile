GPP = g++ -g -Wall -Wno-unused-variable -Wno-unused-but-set-variable -pedantic -std=gnu++0x

otp: *.cpp *.h
	$(GPP) otp.cpp -o otp

sha: *.cpp *.h
	$(GPP) sha-256.cpp

clean:
	rm -f ./otp
	rm -f ./a.out
