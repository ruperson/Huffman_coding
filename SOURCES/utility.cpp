#include <iostream>
#include <climits>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <vector>
#include "compression.cpp"


inline bool validate(int argc, char **argv) {
    if (argc < 3 or argc > 4) { return false; }
    if (std::string(argv[1]) != "-c" && std::string(argv[1]) != "-d") { return false; }
    return true;
}


int main(int argc, char **argv) {
	/*
	if (argc == 1) {
		system("./compression-testing");
		return 0;
	}
	*/
    if (!validate(argc, argv)) {
        std::cout << "Usage: -c/-d <input-file> <*output-file>" << std::endl;
        return 0;
    }

  
    std::string source_name, destination_name;
	std::string type = std::string(argv[1]) == "-c" ? "compress" : "decompress";
    source_name = argv[2];
    destination_name = argc == 4 ? argv[3] : source_name + "." + type + "ed";

    std::ifstream source;
    std::ofstream destination;
    source.open(source_name, std::ios::binary);
    destination.open(destination_name, std::ios::binary);

    std::clock_t time = clock();
	if (type == "compress") {
		encode(source, destination);
	}
	else {
		try {
			decode(source, destination);
		}
		catch (std::logic_error & e) {
			std::cerr << "Decompression failed" + source_name << std::endl;
			std::cerr << e.what() << std::endl;
		}
		/*
		catch (...) {
			std::cerr << "Decompression failed" + source_name << std::endl;
		}
		*/
	}

    std::cerr << "Time: "
              << std::setprecision(3) << double(clock() - time) / CLOCKS_PER_SEC << " sec."
              << std::endl;
}

