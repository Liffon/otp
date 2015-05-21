GPP = g++ -g -Wall -Wno-unused-variable -Wno-unused-but-set-variable -pedantic -std=gnu++0x

otp: otp.cpp
	$(GPP) otp.cpp -o otp

sha: sha-256.cpp
	$(GPP) sha-256.cpp

clean:
	rm -f ./otp
	rm -f ./a.out
