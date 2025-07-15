#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <mutex>
#include <string.h>
#include <thread>

using namespace std;

long long next_batch=0;
mutex mtx;

void prime_calculator(int thread, long long max, long long jump, long long* verif_primes, long long vp_size){ //vp stands for verif_primes
	//Declare variables
	long long current_batch=0;
	long long array_size = jump/16;
	unsigned char* primes = new unsigned char[array_size];
	unsigned char masks[] = {1,2,4,8,16,32,64,128};
	char f_byte, s_byte, t_byte;
	long long num, mult, offset;

	while(1){
		//Assign the new batch to this thread
		mtx.lock();
		current_batch = next_batch;
		next_batch+=jump;
		mtx.unlock();
		if(current_batch+1>=max){break;}
		
		//Compute primes
		for(long long i=0; i<array_size; i++){
			primes[i]=0;
		}
		offset = current_batch+3;
		//cout << offset << endl;

		for(long long i=1; (i<vp_size) && (verif_primes[i]*verif_primes[i]<=(offset+jump)); i++){
			num=verif_primes[i];
			(offset%num) ? mult = offset-(offset%num)+num : mult = offset;
			if(!(mult%2)){mult+=num;}
			if(verif_primes[i]==mult){mult*=3;}
			while(mult <= offset+jump){
				primes[(mult-offset)/16] |= masks[((mult-3)%16)/2];
				mult += 2*num;
			}
		}

		//Write the prime numbers to a file
		long long last_prime=offset-2; //=current_batch+1
		long diff = 0;
		string filename = ".database/primes_" + to_string(current_batch) + ".out";
		ofstream outfile(filename, ios_base::binary);
		for(int i=0; i<array_size; i++){
			for(int j=0; j<8; j++){
				if(!(masks[j] & primes[i])){
					diff = i*16+j*2+offset - last_prime;
					if(diff<256){ 
						outfile << (unsigned char)(diff%256);
					}else if(diff<32768){
						f_byte = (((diff)>>8)<<1)+1;
						s_byte = (unsigned char)(diff%256);
						outfile << f_byte << s_byte;
					}else{
						f_byte = (((diff)>>15)<<1)+1;
						s_byte = ((((diff)%32768)>>8)<<1)+1;
						t_byte = (unsigned char)(diff%256);
						outfile << f_byte << s_byte << t_byte;
					}
					last_prime += diff;
				}
			}
		}
		outfile.put('\0');
		outfile.close();
	}
}

int main(int argc, char **argv){
	//Get execution parameters
	int arg_i = 1;
	long long max = 0;
	long long jump = 0;
	int threads = 1;
	while(arg_i < argc){
		if(strcmp(argv[arg_i], "-m") == 0 || strcmp(argv[arg_i], "--max") == 0){
			max=stoll(argv[arg_i+1]);
		}else if(strcmp(argv[arg_i], "-j") == 0 || strcmp(argv[arg_i], "--jump") == 0){
			jump=stoll(argv[arg_i+1]);
		}else if(strcmp(argv[arg_i], "-t") == 0 || strcmp(argv[arg_i], "--threads") == 0){
			threads=stoul(argv[arg_i+1]);
		}
		arg_i+=2;
	}
	if(jump == 0){
		cout << "Please enter a \"jump\" parameter" << endl;
	}else if(max == 0){
		cout << "Please enter a \"max\" parameter" << endl;
	}else if(jump%16){
		cout << "The jump parameter has to be a multiple of 16" << endl;
	}else if(max%jump){
		cout << "The max parameter has to be a multiple of the jump parameter" << endl;
	}else{
		//Compute the actual maximum number that will be checked
		long long total_bytes;
		max--;
		max%16 ? total_bytes = (max/16)+1 : total_bytes = (max/16);
		max = 16*total_bytes+1;
		
		//Compute all the prime numbers to the sqrt of max
		//These prime numbers will be used to compute the rest of the prime numbers
		long long array_size, num, mult, primes_amount;
		long long max_sqrt = floor(sqrt(max));
		max_sqrt%16 ? array_size = (max_sqrt/16)+1 : array_size = (max_sqrt/16);
		max_sqrt = 16*array_size+1;
		
		unsigned char masks[] = {1,2,4,8,16,32,64,128};
		unsigned char* primes = new unsigned char[array_size];
		for(long i=0; i<array_size; i++){
			primes[i]=0;
		}

		num=3;
		while(num*num <= max_sqrt){
			mult = 3*num;
			while(mult <= max_sqrt){
				primes[(mult-3)/16] |= masks[((mult-3)%16)/2];
				mult += 2*num;
			}
			num+=2;
		}
		
		//"Extract" the prime numbers calculated and store them in an array
		primes_amount = 1;
		for(long long i=0; i<array_size; i++){
			for(int j=0; j<8; j++){
				if(!(masks[j] & primes[i])){
					primes_amount++;
				}
			}
		}
		long long* outprimes = new long long[primes_amount];
		long long index = 1;
		outprimes[0]=2;
		for(long long i=0; i<array_size; i++){
			for(int j=0; j<8; j++){
				if(!(masks[j] & primes[i])){
					outprimes[index] = i*16+j*2+3;
					index++;
				}
			}
		}
		
		//Launch the threads
		thread* Threads[threads];
		for(int i=0; i<threads; i++){
			Threads[i] = new thread(prime_calculator, i, max, jump, outprimes, primes_amount);
		}
		for(int i=0; i<threads; i++){
			Threads[i] -> join();
		}

		//Merge all files
		long long diff;
		long long last_prime = 3;
		long batch_num = jump;
		char byte[1] = {2};
		unsigned char f_byte, s_byte, t_byte;
		string filename;

		ofstream outfile("primes.out", ios_base::binary);
		ifstream infile(".database/primes_0.out", ios_base::binary);
		//We merge the first file separetly because the difference between 2 and 3 is 1, which is the only execption of the encoding algorithm used here (where a odd number signifies that two bytes or more encode the difference between two primes)
		outfile << byte[0];
		infile.read(byte, 1);
		outfile << (unsigned char)(byte[0] - 1);
		infile.read(byte, 1);
		while(byte[0]){
			outfile << (unsigned char)byte[0];
			diff = 0;
			while(((unsigned char)byte[0])%2 == 1){
				diff += ((unsigned char)byte[0])>>1;
				diff = diff<<7;
				infile.read(byte, 1);
				outfile << (unsigned char)byte[0];
			}
			diff = diff << 1;
			last_prime += diff + (unsigned char)byte[0];
			infile.read(byte, 1);
		}
		infile.close();
		filesystem::remove(".database/primes_0.out");
		//Merge the rest of the file
		while(batch_num+1 < max){
			filename = ".database/primes_" + to_string(batch_num) + ".out";
			ifstream infile(filename, ios_base::binary);
			diff = 0;
			infile.read(byte, 1);
			while(((unsigned char)byte[0])%2 == 1){
				diff += ((unsigned char)byte[0])>>1;
				diff = diff<<7;
				infile.read(byte, 1);
			}
			diff = diff<<1;
			diff += (unsigned char)byte[0] + batch_num - last_prime+1;
			if(diff<256){
				outfile<<(unsigned char)(diff%256);
			}else if(diff<32768){
				f_byte = (((diff)>>8)<<1)+1;
				s_byte = (unsigned char)(diff%256);
				outfile << f_byte << s_byte;
			}else{
				f_byte = (((diff)>>15)<<1)+1;
				s_byte = ((((diff)%32768)>>8)<<1)+1;
				t_byte = (unsigned char)(diff%256);
				outfile << f_byte << s_byte << t_byte;
			}
			last_prime += diff;
			infile.read(byte, 1);
			while(byte[0] != 0){
				outfile << (unsigned char)byte[0];
				diff = 0;
				while(((unsigned char)byte[0])%2 == 1){
					diff += ((unsigned char)byte[0])>>1;
					diff = diff<<7;
					infile.read(byte, 1);
					outfile << (unsigned char)byte[0];
				}
				diff = diff << 1;
				last_prime += diff + (unsigned char)byte[0];
				infile.read(byte, 1);
			}
			infile.close();
			batch_num+=jump;
			filesystem::remove(filename);
		}
		outfile.put('\0');
		outfile.close();
		cout << "Finished!" << endl;
	}
	return 0;
}
