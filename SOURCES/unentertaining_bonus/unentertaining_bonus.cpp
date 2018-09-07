//g++ unentertaining_bonus.cpp
#include <memory>
#include <random>
#include <functional>
#include <iostream>
#include <set>
#include <queue>
#include <algorithm>
#include <ostream>
#include <ctime>
#include <iomanip>
#include <set>

struct node {
	unsigned char data;
	uint64_t freq = 0;
	std::shared_ptr<node> lchild = nullptr;
	std::shared_ptr<node> rchild = nullptr;
	node(unsigned char data, uint64_t freq) : data(data), freq(freq) {}

	node(node smaller, node bigger)
		: freq(smaller.freq + bigger.freq)
		, lchild(std::make_shared<node>(smaller)), rchild(std::make_shared<node>(bigger)) {}

	bool is_leaf() const { return !lchild && !rchild; }

	bool operator<(node const & rhs) const { // TODO: C++17 way of sorting this shit
		return freq > rhs.freq;
	}
};
struct COMPARE {
	bool operator()(std::pair<size_t, size_t> const &a, std::pair<size_t, size_t> const &b) {
		return a.first > b.first;
	}
};
const std::size_t TIMES = 10000; 

int main() {
	std::random_device rd;    
 	std::mt19937_64 eng(rd()); 
  	std::uniform_int_distribution<uint64_t> distr;
    std::clock_t time;



	time = clock();

	for (size_t z = 0; z < TIMES; ++z) {
		std::priority_queue<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t> > , COMPARE> freq; //второе - индекс в tree
	
		for (size_t i = 0; i < 256; ++i) {
			freq.push({ distr(eng), i });
		}
	
		size_t cnt_nodes = 256;
		while (freq.size() > 1) {
			auto node1 = freq.top();
			freq.pop();
			auto node2 = freq.top();
			freq.pop();
			freq.push({ node1.first + node2.first, cnt_nodes });
			++cnt_nodes;
		}
	}

	std::cerr << "SORTING PAIRS priority_queue WITH COMPARE"
			  << std::endl 
			  << "time taken: "
              << std::setprecision(3) << double(clock() - time) / CLOCKS_PER_SEC << "s."
              << std::endl;

	time = clock();

	for (size_t z = 0; z < TIMES; ++z) {
		std::priority_queue<std::pair<size_t, size_t>> freq; 
	
		for (size_t i = 0; i < 256; ++i) {
			freq.push({ distr(eng), i });
		}
	
		size_t cnt_nodes = 256;
		while (freq.size() > 1) {
			auto node1 = freq.top();
			freq.pop();
			auto node2 = freq.top();
			freq.pop();
			freq.push({ node1.first + node2.first, cnt_nodes });
			++cnt_nodes;
		}
	}

	std::cerr << "SORTING PAIRS priority_queue WITHOUT COMPARE"
			  << std::endl 
			  << "time taken: "
              << std::setprecision(3) << double(clock() - time) / CLOCKS_PER_SEC << "s."
              << std::endl;

	time = clock();

	for (size_t z = 0; z < TIMES; ++z) {
		std::set<std::pair<size_t, size_t> > freq; //второе - индекс в tree

		for (size_t i = 0; i < 256; ++i) {
			freq.insert({ distr(eng), i });
		}

		size_t cnt_nodes = 256;
		while (freq.size() > 1) {
			auto node1 = *freq.begin();
			freq.erase(freq.begin());
			auto node2 = *freq.begin();
			freq.erase(freq.begin());
			freq.insert({ node1.first + node2.first, cnt_nodes });
			++cnt_nodes;
		}
	}

	std::cerr << "SORTING PAIRS WITH SET"
			  << std::endl 
			  << "time taken: "
              << std::setprecision(3) << double(clock() - time) / CLOCKS_PER_SEC << "s."
              << std::endl;

    time = clock();

	for (size_t z = 0; z < TIMES; ++z) {
		std::vector <node> nodes; 
		nodes.reserve(256);
		{
			unsigned char i = 0;
			do { nodes.emplace_back(i, distr(eng));  } while (++i != 0);
		}

    	std::priority_queue<node, std::vector<node>> queue(nodes.begin(), nodes.end());
    	while (queue.size() > 1) {
    	    auto smaller = queue.top();
    	    queue.pop();
    	    auto bigger = queue.top();
    	    queue.pop();
    	    node merged = node(smaller, bigger);
    	    queue.push(merged);
    	};
	}

	std::cerr << "priority_queue O(nlogn) algorithm ..."
			  << std::endl 
			  << "time taken: "
              << std::setprecision(3) << double(clock() - time) / CLOCKS_PER_SEC << "s."
              << std::endl;



    time = clock(); // starting sorting O(n^2) algorithm

  	for (size_t z = 0; z < TIMES; ++z) {
		std::vector <node> nodes; 
		nodes.reserve(256);
		{
			unsigned char i = 0;
			do { nodes.emplace_back(i, distr(eng));  } while (++i != 0);
		}
	
		std::sort(nodes.begin(), nodes.end());
		while (nodes.size() != 1) {
			node x = nodes.back();
			nodes.pop_back();
			node y = nodes.back();
			nodes.pop_back();
			nodes.push_back(node(x, y));
			std::sort(nodes.begin(), nodes.end());
		}
	}

	std::cerr << "sorting O(n^2) algorithm ..."
			  << std::endl 
			  << "time taken: "
              << std::setprecision(3) << double(clock() - time) / CLOCKS_PER_SEC << "s."
              << std::endl;

    time = clock();

	for (size_t z = 0; z < TIMES; ++z) {
		std::vector <node> nodes; 
		nodes.reserve(256);
		{
			unsigned char i = 0;
			do { nodes.emplace_back(i, distr(eng));  } while (++i != 0);
		}

    	std::set<node> freq(nodes.begin(), nodes.end()); 
	
    	while (freq.size() > 1) {
    	    auto smaller = *freq.begin();
    	    freq.erase(freq.begin());
    	    auto bigger = *freq.begin();
    	    freq.erase(freq.begin());
    	    node merged = node(smaller, bigger);
    	    freq.insert(merged);
    	}
    }

	std::cerr <<"set O(nlogn) algorithm ..."
			  << std::endl 
			  << "time taken: "
              << std::setprecision(3) << double(clock() - time) / CLOCKS_PER_SEC << "s."
              << std::endl;

    
}