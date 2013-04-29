#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "step_counter.c"

measurements data;
int collected_data;
int total_steps = 0;
acc_data_t* acc_arr;
using namespace std;

int total_samples = 0;
int line_num = 0;
void parse_file(string filename)
{
	string line, word;

	ifstream file(filename.c_str());
	if(file) 
	{
		while(getline(file, line, '\r')) {
			total_samples++;
		}
		std::cout << "total_samples " << total_samples << std::endl;
		file.clear();
		file.seekg(0, file.beg);
		acc_arr = new acc_data_t[total_samples];
		
		while(getline(file, line, '\r')) {
			if(line_num > 0) {			
				if(line[line.length() == '\r'])
					line.erase(line.length());

				int index = 0;
				std::vector<std::string> strs;
				boost::algorithm::split(strs, line, boost::is_any_of(","));
				
				int tmp;
				istringstream buffer0(strs[0]);
				istringstream buffer1(strs[1]);
				istringstream buffer2(strs[2]);

				buffer0 >> tmp;
				acc_arr[line_num].x = tmp;
				buffer1 >> tmp;
				acc_arr[line_num].y = tmp;
				buffer2 >> tmp;
				acc_arr[line_num].z = tmp;
			}
			line_num++;
		
		}
	file.close();
	}
	else
		cout << "Error: Could not find file "<< filename << endl;
	
}


int main (int argc, char* argv[]) {
	int temp_steps;
	if(argc < 2)
		cout << "Provide file " << endl;

	parse_file(argv[1]);
	
	measurements measure;
	
    int i;
    std::cout << "Total Samples: " << total_steps << std::endl;
    for(i = 0; i< total_samples - SAMPLE_SIZE; i+=SAMPLE_SIZE) {
		filter((acc_arr + i), SAMPLE_SIZE);
		print_acc_data_array((acc_arr + i), SAMPLE_SIZE);
		get_max_min(&measure, (acc_arr + i), SAMPLE_SIZE);
		print_measure_data(&measure);
		temp_steps = count_steps1(&measure, (acc_arr + i), SAMPLE_SIZE);
		std::cout << "Temp Steps: " << temp_steps << std::endl;
		total_steps += temp_steps;
		
		
	}

	filter((acc_arr + total_samples - 1), (total_samples - i));
	print_acc_data_array((acc_arr + total_samples - 1), (total_samples - i));
	get_max_min(&measure, (acc_arr + total_samples - 1), (total_samples - i));
	print_measure_data(&measure);
	total_steps += count_steps(&measure, (acc_arr + total_samples - 1), (total_samples - i));
	std::cout << "Total Steps: " << total_steps << std::endl;
	return 0;
}