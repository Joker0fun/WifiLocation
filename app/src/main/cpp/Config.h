#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <fstream>
//#include <string>
#include "typesdef.h"

using namespace std;

class Config {
public:
	void Parse(const char *path, SectionMap &sections) {
		FILE *fp = fopen(path, "r");
		if (fp == NULL) {
			return;
		}

		ifstream fs(path);

		ifstream::streamoff begin = 0;
		ifstream::streamoff end = 0;
		size_t line_count = 1;
		size_t character_count = 0;
		ConfigMap *current_section = nullptr;

		string key;
		char current_content = 'n'; // n: none c: comment s: section k: key v: value
		bool quotation = false;
		char *buf = nullptr;
		ifstream::streamoff len;

		while (!fs.eof()) {
			int c = fs.get();
			++character_count;
			if (quotation && c != '"')
				continue;

			switch (c) {
			case '#': // comments, skip whole line.
				while (!fs.eof() && fs.peek() != '\n') {
					fs.get();
				}
				break;
			case '\n': // next line, end of everything.
				character_count = 0;
				++line_count;

				if (current_content == 's' || current_content == 'k') {
					return;
				}
				else if (current_content == 'v') {
					end = fs.tellg();
					ifstream::streamoff length = end - begin;
					fs.seekg(begin, ios::beg);

					char *value = new char[length];
					fs.read(value, length);
					value[length - 1] = '\0';
					(*current_section)[key] = value;

					delete[] value;
				}
				begin = 0;
				end = 0;
				current_content = 'n';
				break;
			case '[': // section begin.
				if (current_content != 'n') {
					Error(line_count, character_count, path);
					return;
				}
				begin = fs.tellg();
				current_content = 's';
				break;
			case ']':
				if (current_content != 's') {
					Error(line_count, character_count, path);
					return;
				}
				end = fs.tellg();
				fs.seekg(begin, ios::beg);
				len = end - begin;
				buf = new char[end - begin + 1];
				fs.read(buf, len);
				buf[len - 1] = 0;
				sections[buf] = ConfigMap();
				current_section = &sections[buf];
				delete[] buf;
				current_content = 'n';
				break;
			case '=':
				if (current_content == 'k') {
					end = fs.tellg();
					ifstream::streamoff length = end - begin;
					fs.seekg(begin, ios::beg);

					buf = new char[length];
					fs.read(buf, length);
					buf[length - 1] = '\0';

					for (ifstream::streamoff i = length - 2; i >= 0; --i) { // trim right.
						if (buf[i] == ' ')
							buf[i] = '\0';
						else
							break;
					}
					key = buf;
					delete[] buf;

					current_content = 'v';
					begin = fs.tellg();
				}
				else if (!quotation) {
					Error(line_count, character_count, path);
					return;
				}
				break;
			case '"':
				quotation = !quotation;
				break;
			case ' ':
			case '\t':
				break;
			default:
				if (current_content == 'k') {
					if (current_section == nullptr) {
						return;
					}
				}
				if (current_content == 'n') {
					current_content = 'k';
					begin = fs.tellg() - (ifstream::streamoff) 1;
				}
				break;
			}
		}
		if (current_content == 'v') {
			fs.clear();
			fs.seekg(0, ios::end);
			end = fs.tellg();
			fs.seekg(begin, ios::beg);
			ifstream::streamoff length = end - begin + 1;
			char *value = new char[length];
			fs.read(value, length);
			value[length - 1] = '\0';
			(*current_section)[key] = value;

			delete[] value;
		}
	}

	void Error(size_t line, size_t character, const char *path) {
		cout << "Config Parse Error." << endl;
	}
};

#endif
