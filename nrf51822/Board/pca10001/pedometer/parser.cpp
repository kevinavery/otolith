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

int total_lines = 0;
int line_num = 0;
void parse_file(string filename)
{
	string line, word;

	ifstream file(filename.c_str());
	if(file) 
	{
		while(getline(file, line, '\r')) {
			total_lines++;
		}
		std::cout << "total_lines " << total_lines << std::endl;
		file.clear();
		file.seekg(0, file.beg);
		acc_arr = new acc_data_t[total_lines];
		
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
	if(argc < 2)
		cout << "Provide file " << endl;

	parse_file(argv[1]);
	print_acc_data_array(acc_arr, 100);
	measurements measure;
	
	filter(acc_arr, SAMPLE_SIZE);
	get_max_min(&measure, acc_arr, SAMPLE_SIZE);
	count_steps(&measure, acc_arr, SAMPLE_SIZE);
	print_measure_data(&measure);
	return 0;
}