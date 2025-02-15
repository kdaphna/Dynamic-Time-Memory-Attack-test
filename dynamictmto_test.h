uint32_t apply_F(uint32_t p, uint32_t k){
    uint32_t c = encryption_oracle(p, k);
    /* printf("E=> %08X %08X %08X ::", p, k, c); */
    return c;
}

bool file_exists(const std::string& filename) {
	std::ifstream file(filename);
	return file.good();
}

//This function checking if a point is distingushed or not
int is_distinguished(uint32_t x){
    uint32_t last_digits = x & MASK;
    if(last_digits == 0){
        return 1;
    }
    return 0;
}

/*This stage is multithreaded. Suppose total number of tabes are t and we have p threads. So 
 *we can compute t/p tables in each thread. Inside each thread we compute t/p tables one by
 *one and save them in hard disk. 
 */

struct Args{
    uint32_t start;
    uint32_t end;
    uint32_t m;
};

void *dynamic_preprocessing(void *args1){//TODO: create two maps to save to two files
    struct timespec start, end;
    double time_meter;
    Args *args = (Args *)args1;
    for(uint32_t l = args->start; l<args->end; l++){ //for each table
        unordered_map<uint32_t, vector<uint32_t>> TL_short;
		unordered_map<uint32_t, vector<uint32_t>> TL_long;
        for(int j=0; j<M; j++){   //You want to cover M data in each table
            while(1){ //do until find a distinguished points for the jth starting point
				uint32_t length = 0;
                uint32_t x = rand();
                uint32_t temp = x;
                for(int i=0; i<(12288*(1UL << 12)); i++){
                    uint32_t k = temp ^ L_function[l]; //adding flavours
                    temp = encryption_oracle(args->m, k);  //Encryption Oracle
			
					length ++; //counting the length of the chain

                    if(is_distinguished(temp)){ //Check if this is a distinguished point
						if(length > 12288){ //If the length of the chain is longer than some number (need to be change accordanlly) 

                        	uint32_t y = temp;
                        	TL_long[y].push_back(x);
                        	break;  //Once you find a distinguished point, break and try for new data
						}
						else{
							uint32_t y = temp;
							TL_short[y].push_back(x);
							break;
						}

                    }
                }

                if(is_distinguished(temp)){
                    break; //break the while loop
                }
            }
        }
        string filename_short = "./data/T_" + to_string(l) + 'S' + ".tmp";
        saveMapToFile(TL_short, filename_short);
		string filename_long = "./data/T_" + to_string(l) + 'L' + ".tmp";
        saveMapToFile(TL_long, filename_long);
        TL_short.clear();
		TL_long.clear();
    }
    pthread_exit(NULL);
}

void dynamic_preprocessing_step(uint32_t msg){
    int THREAD = 120; //32;
    uint64_t size = T; //nr_of_tables
    pthread_t thread_ids[THREAD];
    Args thread_args[THREAD];

    uint64_t table_each_thread = size / THREAD;
    uint64_t table_last_thread = size % THREAD;

    /* printf("%d %f \n",table_each_thread, log(table_each_thread)/log(2)); */
    /* printf("%d %f \n",table_last_thread, log(table_last_thread)/log(2)); */

    for(int i=0; i < THREAD; i++){
        if( i == (THREAD-1) ){
            thread_args[i].start = i * table_each_thread;
            thread_args[i].end = (i+1) * table_each_thread + table_last_thread;
        }
        else{
            thread_args[i].start = i * table_each_thread;
            thread_args[i].end = (i+1) * table_each_thread;
        }
        thread_args[i].m = msg;

    }
    for(int i=0; i < THREAD; i++){
        pthread_create(thread_ids + i, NULL, dynamic_preprocessing, (void*) (thread_args + i));
    }
    for(int i=0; i < THREAD; i++){
        pthread_join(thread_ids[i], NULL);
    }
}


void switching(uint32_t table_number, uint32_t start_point, uint32_t end_point){
        //Delete a chain form the short table
        string filename = "./data/T_" + to_string(table_number) + 'S' + ".tmp";
        unordered_map<uint32_t, vector<uint32_t>> TL;
        TL = readMapFromFile(filename);
        TL.erase(1);
        saveMapToFile(TL, filename);
        TL.clear();
        //Add the new chain to the long table
        filename = "./data/T_" + to_string(table_number) + 'L' + ".tmp";
        TL[end_point].push_back(start_point);
        saveMapToFile(TL, filename);
        TL.clear();
}


uint32_t dynamic_online_step(uint32_t msg, uint32_t ctx){
    uint32_t length, k, temp, temp2, temp_long, success;
	uint32_t fkey = 0;
	uint32_t switch_number = 0;
    for(int l=0; l<T; l++){
        temp  = ctx;
        length = 0;
		success = 0;

        //Update until find a distinguished point
        while( (length < (12288*(1UL << D))) && (is_distinguished(temp) == 0) ){
            k = temp ^ L_function[l]; //adding flavours
            temp = encryption_oracle(msg, k);
            length++;
        }

        //If you found a distinguished point, search that in the table, Else go to next table
		
		//Search the short table
        if(is_distinguished(temp)){
			string filename = "./data/T_" + to_string(l) + 'S' + ".tmp";
            unordered_map<uint32_t, vector<uint32_t>> TL;
            TL = readMapFromFile(filename);
			temp_long = temp;

            //Find the data in table TL
            if (TL.find(temp) != TL.end()){
                temp = TL[temp][0];
                temp2 = temp;

                k = temp ^ L_function[l]; //adding flavours
                temp = encryption_oracle(msg, k);
                while((temp != ctx) && (is_distinguished(temp) == 0) ){
                    temp2 = temp;
                    k = temp ^ L_function[l]; //adding flavours
                    temp = encryption_oracle(msg, k);
                }
				k = temp2 ^ L_function[l]; //adding flavours
                temp = encryption_oracle(msg, k);
                if(temp == ctx){
                    fkey = (temp2 ^ L_function[l]);;
                    printf("fkey %08X in Table %d\n",fkey, l);
					success = 1;
                }
				
            }
			//If the search in the short table fails, the long table is searched
			if(success == 0){
				string filename = "./data/T_" + to_string(l) + 'L' + ".tmp";
				unordered_map<uint32_t, vector<uint32_t>> TL;
				TL = readMapFromFile(filename);
				temp = temp_long;

				//Find the data in table TL
				if (TL.find(temp) != TL.end()){
					temp = TL[temp][0];
					temp2 = temp;

					k = temp ^ L_function[l]; //adding flavours
					temp = encryption_oracle(msg, k);
					while((temp != ctx) && (is_distinguished(temp) == 0) ){
						temp2 = temp;
						k = temp ^ L_function[l]; //adding flavours
						temp = encryption_oracle(msg, k);
					}
					k = temp2 ^ L_function[l]; //adding flavours
					temp = encryption_oracle(msg, k);
					if(temp == ctx){
						fkey = (temp2 ^ L_function[l]);;
						printf("fkey %08X in Table %d\n",fkey, l);
						success = 1;
					}
					
				}
			}
			if(length > 12288){
				switching(l, ctx, temp);
				switch_number ++;
			}
        }
        else{
            printf("Failed; Not found distinguished Points \n");
        }
    }
	string filename_switcing = "./data/switching.txt";
	std::ofstream outFile(filename_switcing, std::ios::app);
	if (outFile.is_open()) {
        outFile << switch_number << " ";
	}
    outFile.close();
	return fkey;
}

struct ArgsOnline{
    uint32_t start;
    uint32_t end;
    uint32_t m;
    uint32_t *successArr;  
};
void *hellman_online(void *args1){
	struct timespec start, end;
    double time_meter, time_meter_miss;
	long seconds, nanoseconds;
    double elapsed;
    ArgsOnline *args = (ArgsOnline *)args1;
	uint32_t msg = args->m;
	uint32_t *success_Arr = args -> successArr;
	uint32_t sucess_counter = args->start;
    for(uint32_t l = args->start; l<args->end; l++){ //l is counting the number of searches 
		uint32_t i, temp, temp2;
		uint32_t key = rand();
		uint32_t ctx = encryption_oracle(msg, key);
		uint32_t length, k, temp_long, success = 0;
		uint32_t fkey = 0;
		clock_gettime(CLOCK_MONOTONIC, &start);
		for(i=0; i<T; i++){//searching in all the T tables
			temp  = ctx;
			length = 0;
			success_Arr[sucess_counter] = 0;

			//Update until find a distinguished point
			while( (length < (12288*(1UL << D))) && (is_distinguished(temp) == 0) ){
				k = temp ^ L_function[i]; //adding flavours
				temp = encryption_oracle(msg, k);
				length++;
			}

			//If you found a distinguished point, search that in the table, Else go to next table
			
			//Search the short table
			if(is_distinguished(temp)){
				string filename = "./data/T_" + to_string(i) + 'S' + ".tmp";
				unordered_map<uint32_t, vector<uint32_t>> TL;
				TL = readMapFromFile(filename);
				temp_long = temp;

				//Find the data in table TL
				if (TL.find(temp) != TL.end()){
					temp = TL[temp][0];
					temp2 = temp;

					k = temp ^ L_function[i]; //adding flavours
					temp = encryption_oracle(msg, k);
					while((temp != ctx) && (is_distinguished(temp) == 0) ){
						temp2 = temp;
						k = temp ^ L_function[i]; //adding flavours
						temp = encryption_oracle(msg, k);
					}
					k = temp2 ^ L_function[i]; //adding flavours
					temp = encryption_oracle(msg, k);
					if(temp == ctx){
						fkey = (temp2 ^ L_function[i]);
						if(fkey = key){
							success_Arr[sucess_counter] = 1;
							success = 1;
						}
						break;
					}
					
				}else{
					string filename = "./data/T_" + to_string(i) + 'L' + ".tmp";
					unordered_map<uint32_t, vector<uint32_t>> TL;
					TL = readMapFromFile(filename);
					temp = temp_long;

					//Find the data in table TL
					if (TL.find(temp) != TL.end()){
						temp = TL[temp][0];
						temp2 = temp;

						k = temp ^ L_function[i]; //adding flavours
						temp = encryption_oracle(msg, k);
						while((temp != ctx) && (is_distinguished(temp) == 0) ){
							temp2 = temp;
							k = temp ^ L_function[i]; //adding flavours
							temp = encryption_oracle(msg, k);
						}
						k = temp2 ^ L_function[i]; //adding flavours
						temp = encryption_oracle(msg, k);
						if(temp == ctx){
							fkey = (temp2 ^ L_function[i]);;
							if(fkey = key){
								success_Arr[sucess_counter] = 1;
								success = 1;
							}
							break;
						}
						
					}
				}
			
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		seconds = end.tv_sec - start.tv_sec;
		nanoseconds = end.tv_nsec - start.tv_nsec;
		elapsed = seconds + nanoseconds*1e-9;
		if (success == 1){
			time_meter += elapsed;
		}else{
			time_meter_miss += elapsed;
		}
		sucess_counter++;

	}
	string filename = "./data/successTime" + to_string(args->start) + ".txt";
	std::ofstream outFile(filename, std::ios::app);
	if (outFile.is_open()) {
        outFile << time_meter << " ";
	}
    outFile.close();
	string filename_miss = "./data/failTime" + to_string(args->start) + ".txt";
	std::ofstream outFile_miss(filename_miss, std::ios::app);
	if (outFile_miss.is_open()) {
        outFile_miss << time_meter_miss << " ";
	}
    outFile_miss.close();
    pthread_exit(NULL);
}

void writeArrayToFile(const std::vector<uint32_t>& array, const std::string& filename) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        for (uint32_t num : array) {
            outFile << num << " ";
        }
        outFile.close();
        std::cout << "Array written to " << filename << std::endl;
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }
}


void hellman_online_step(uint32_t msg, uint32_t number_of_runs, uint32_t run_num){
    int THREAD = 120; //32;
    uint64_t size = number_of_runs; //nr_of_runs we want to check to recive the success rate/cover
	size_t size_of_array = number_of_runs;
    pthread_t thread_ids[THREAD];
    ArgsOnline thread_args[THREAD];

    uint64_t runs_each_thread = size / THREAD;
    uint64_t runs_last_thread = size % THREAD;

    /* printf("%d %f \n",table_each_thread, log(table_each_thread)/log(2)); */
    /* printf("%d %f \n",table_last_thread, log(table_last_thread)/log(2)); */

    uint32_t *successArr = (uint32_t *)malloc(size*sizeof(uint32_t)); 
    memset(successArr, 0, size*sizeof(uint32_t));

    for(int i=0; i < THREAD; i++){
        if( i == (THREAD-1) ){
            thread_args[i].start = i * runs_each_thread;
            thread_args[i].end = (i+1) * runs_each_thread + runs_last_thread;
        }
        else{
            thread_args[i].start = i * runs_each_thread;
            thread_args[i].end = (i+1) * runs_each_thread;
        }
        thread_args[i].m = msg;
        thread_args[i].successArr = successArr;

    }
    for(int i=0; i < THREAD; i++){
        pthread_create(thread_ids + i, NULL, hellman_online, (void*) (thread_args + i));
    }
    for(int i=0; i < THREAD; i++){
        pthread_join(thread_ids[i], NULL);
    }
	std::vector<uint32_t> successVec(successArr, successArr + size_of_array);
    free(successArr);

	string filename = "./data/successArr" + to_string(run_num) + ".txt";
	writeArrayToFile(successVec, filename);
}




void online_step(uint32_t msg, uint32_t ctx, uint32_t number_of_runs, uint32_t success_rate){
	uint32_t ciphertx, key, tempkey = 0;
	ciphertx = ctx;
	for(uint32_t i = 0; i < number_of_runs; i++){
		for(uint32_t j = 0; j < 100; j++){
			tempkey = dynamic_online_step(msg, ciphertx);
			key = rand();
			ciphertx = encryption_oracle(msg, key);
			if(tempkey == key){
				printf("This is gooood!");
			}
		}
		hellman_online_step(msg, success_rate, i);
		printf("Number of run %d \n", i);
	}
	
	/*
	uint32_t found_key_hellman = hellman_online_step(msg, ctx);
	uint32_t found_key_dynamic = dynamic_online_step(msg, ctx);
    uint32_t number_of_success = 0;
	uint32_t number_of_success_hellman = 0;
    uint32_t rate = 0;
	uint32_t hellman_rate = 0;
	uint32_t rate_number = success_rate;
	vector<uint32_t> dynamic_success_array;
	vector<uint32_t> hellman_success_array;
	for(uint32_t i = 0; i < number_of_runs; i++){
		if(key == found_key_dynamic){
			number_of_success ++;
		}
		if(rate == rate_number){
			dynamic_success_array.push_back(number_of_success);
			number_of_success = 0;
			rate = 0;
		}
		printf("found    key 0x%08X \n",found_key_dynamic);
		printf("original key 0x%08X \n",key);
		
		
		if(key == found_key_hellman){
			number_of_success_hellman ++;
		}
		if(hellman_rate == rate_number){
			hellman_success_array.push_back(number_of_success_hellman);
			number_of_success_hellman = 0;
			hellman_rate = 0;
		}
		printf("found    key 0x%08X \n",found_key_hellman);
		printf("original key 0x%08X \n",key);
		
		key = rand();
		ctx = encryption_oracle(msg, key);
		found_key_hellman = hellman_online_step(msg, ctx);
		found_key_dynamic = dynamic_online_step(msg, ctx);
		
		hellman_rate ++;
		rate ++;
    }
	ofstream myfile;
	myfile.open ("dynamic_value.txt");
    myfile << "The number of dynamic success: \n";
    for(uint32_t success : dynamic_success_array) {
        myfile << success << " ";
    }
   myfile << "\n";
	
	myfile.close();
	myfile.open("value.txt");
	myfile << "The number of hellman success: \n";
    for(uint32_t success_hellman : hellman_success_array) {
        myfile << success_hellman << " ";
    }
   myfile << "\n";
	myfile.close();*/
}

