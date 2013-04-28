#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
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
		while(getline(file, line, '\n')) {
			total_lines++;
		}
		std::cout << "total_lines" << total_lines << std::endl;
		file.clear();
	

		acc_arr = new acc_data_t[total_lines];
		
	 
		while(getline(file, line, '\n')) {
			if(line_num > 0) {
				if(line[line.length() == '\r'])
					line.erase(line.length());
				int index = 0;
				while(getline(file, word, ',')) {
					istringstream buffer(word);
					int value;
					buffer >> value;   // value = 45
					switch (index)
					{
					case 0: acc_arr[line_num].x = value;
					    break;
					case 1: acc_arr[line_num].y = value;
					    break;
					case 2: acc_arr[line_num].z = value;
					    break;
					default: cout << "NOT INDEX" << std::endl;
					    break;
					}


					index++;	
				}
			}
			line_num++;
		
		}
	file.close();
	file.clear();
	}
	else
		cout << "Error: Could not find file "<< filename << endl;
	
}


int main (int argc, char* argv[]) {
	if(argc < 2)
		cout << "Provide file " << endl;

	parse_file(argv[1]);
	measurements measure;
	
	filter(acc_arr, SAMPLE_SIZE);
	get_max_min(&measure, acc_arr, SAMPLE_SIZE);
	count_steps(&measure, acc_arr, SAMPLE_SIZE);
	//print_measure_data(&measure);
	return 0;
}