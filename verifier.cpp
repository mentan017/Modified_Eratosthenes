#include <iostream>
#include <fstream>

using namespace std;

int main(){
	int found = 0;
	long long prime = 3;
	char byte[1] = {0};
	long long diff;
	ifstream infile("primes.out", ios_base::binary);
	infile.read(byte, 1);
	infile.read(byte, 1);
	infile.read(byte, 1);
	while(byte[0] != 0){
		diff = 0;
		while((((unsigned char)byte[0])%2) == 1){
			diff += ((unsigned char)byte[0])>>1;
			diff = diff<<7;
			infile.read(byte, 1);
		}
		diff = diff<<1;
		prime += diff + (unsigned char)byte[0];
		infile.read(byte, 1);
	}
	infile.close();
	cout << prime << endl;
}
