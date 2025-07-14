#include <cstring>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char **argv){
	//Get execution parameters
	int arg_i = 1;
	long long max = 0;
	while(arg_i < argc){
		if(strcmp(argv[arg_i], "-m") == 0 || strcmp(argv[arg_i], "--max") == 0){
			max=stoll(argv[arg_i+1])-1;
		}
		arg_i+=2;
	}
	//Compute prime numbers
	unsigned char masks[] = {1,2,4,8,16,32,64,128};
	long array_size = max/16;
	max%16 ? array_size = (max/16)+1 : array_size = (max/16);
	max = 16*array_size+1; //Update the max to include the biggest number that will be in our array
	unsigned char* primes = new unsigned char[array_size];
	for(long i=0; i<array_size; i++){
		primes[i] = 0;
	}
	long long num, mult;
	num = 3;
	cout << "max: " << max << endl;
	while(num*num <= max){
		mult = 3*num;
		while(mult <= max){
			//cout << "num: " << num << " et mult: " << mult << endl;
			primes[(mult-3)/16] |= masks[((mult-3)%16)/2]; 
			mult += 2*num;
		}
		num+=2;
	}
	//Output the prime numbers
	ofstream outfile("primes.out");
	outfile << 2 << endl;
	for(long i=0; i<array_size; i++){
		for(int j=0; j<8; j++){
			if(!(masks[j] & primes[i])){
				outfile << i*16+j*2+3 << endl;
			}
		}
	}
	outfile.close();
	cout << "Finished!" << endl;
}
