#ifndef HUFFMAN_COMPRESSION
#define HUFFMAN_COMPRESSION

#include <memory>   // shared_ptr
#include <cassert>
#include <algorithm> //std::sort
#include <queue>
#include <vector>
#include <array>
#include <istream>
#include <ostream>
//namespace {
const std::size_t CHUNK_SIZE = 32768;
class buffered_reader {
public:
	buffered_reader(std::istream &stream) : stream(stream) {
		fill_buffer();
	}

	void reset() {
		stream.clear();
		stream.seekg(0);
	}

	bool read_char(unsigned char & x) {
		if (pos < size) {
			x = buffer[pos++];
			return true;
		}
		else {
			fill_buffer();
			if (pos < size) {
				x = buffer[pos++];
				return true;
			}
			else {
				return false;
			}
		}
	}

private:
	void fill_buffer() {
		stream.read(buffer, CHUNK_SIZE);
		size = stream.gcount();
		pos = 0;
	}
	char buffer[CHUNK_SIZE];
	std::streamsize pos, size;
	std::istream &stream;
};
class buffered_writer {
public:
	buffered_writer(std::ostream &stream) : pos(0), stream(stream) {}


	~buffered_writer() {
		empty_buffer();
	}

	void write_char(unsigned char const & x) {
		if (pos < CHUNK_SIZE) {
			buffer[pos] = x;
			++pos;
		}
		else {
			empty_buffer();
			buffer[pos] = x;
			++pos;
		}
	}

private:
	void empty_buffer() {
		stream.write(buffer, pos);
		pos = 0;
	}
	char buffer[CHUNK_SIZE];
	std::size_t pos;
	std::ostream &stream;
};



std::vector<uint64_t> get_code(std::vector<unsigned char> const &len) {
	std::vector<std::pair<unsigned char, unsigned char> > length(256);
	{
		unsigned char i = 0;
		do { length[i] = { len[i], i }; } while (++i != 0);
	}
	std::sort(length.begin(), length.end());
	std::vector<uint64_t> code(256);
	uint64_t cur_code = 0;
	code[length[0].second] = 0;
	for (unsigned char i = 1; i != 0; ++i) {
		++cur_code;
		cur_code <<= (length[i].first - length[i - 1].first);
		code[length[i].second] = cur_code;
	}
	return code;
}

struct node {
	const static uint16_t nullpointer = -1;
	uint16_t left;
	uint16_t right;

	node() : left(nullpointer), right(nullpointer) {}

	node(int left, int right) : left(left), right(right) {}
};
void count_length(size_t pos, unsigned char depth, std::vector<node> const &tree, std::vector<unsigned char> &length) {
	if (tree[pos].left == node::nullpointer && tree[pos].right == node::nullpointer) {
		length[pos] = depth;
		return;
	}

	if (tree[pos].left != node::nullpointer) {
		count_length(tree[pos].left, depth + 1, tree, length);
	}
	if (tree[pos].right != node::nullpointer) {
		count_length(tree[pos].right, depth + 1, tree, length);
	}
}


void encode(std::istream &input, std::ostream &output) {
	buffered_reader in(input);
	buffered_writer out(output);
	uint64_t table[256] = {};
	unsigned char cur;
	while (in.read_char(cur)) {
		++table[cur];
	}
	in.reset();


	std::vector<node> tree(511);
	{
		std::priority_queue<std::pair<uint64_t, uint16_t>, std::vector<std::pair<uint64_t, uint16_t> >, std::greater<std::pair<uint64_t, uint16_t>>> freq; //второе - индекс в tree
		uint16_t cnt_nodes = 0;
		for (; cnt_nodes < 256; ++cnt_nodes) {
			freq.push({ table[cnt_nodes], cnt_nodes });
		}

		while (cnt_nodes < 511) {
			std::pair<uint64_t, uint16_t> node1 = freq.top();
			freq.pop();
			std::pair<uint64_t, uint16_t> node2 = freq.top();
			freq.pop();
			tree[cnt_nodes] = { node1.second, node2.second };
			freq.push({ node1.first + node2.first, cnt_nodes });
			++cnt_nodes;
		}
	}
	std::vector<unsigned char> code_lengths(256);
	count_length(510, 0, tree, code_lengths);
	
	std::vector<uint64_t> codes = get_code(code_lengths);

	unsigned char counter_bit = 0; // how many bits are used in last byte
	{
		unsigned char i = 0;
		do { //no whitespace!
			counter_bit = (counter_bit+((code_lengths[i]%8)*(table[i]%8))%8)%8;
		} while (++i != 0);
	}

	out.write_char(counter_bit);

	for (auto code : code_lengths) {
		out.write_char(code);
	}

	unsigned char acc = 0;
	int acc_size = 0;
	while (in.read_char(cur)) {
		for (unsigned char j = code_lengths[cur]; j-- > 0;) {
			if (acc_size == 8) {
				out.write_char(acc);
				acc_size = 0;
				acc = 0;
			}
			bool woof = codes[cur] & (1ULL << j);
			acc |= woof << (7 - acc_size);
			++acc_size;
		}
	}
	assert(acc_size % 8 == counter_bit);
	out.write_char(acc);
}


class BinaryTrie {
public:
	BinaryTrie() : pos_trie(0), trie(1) {}
	BinaryTrie(std::vector<uint64_t> const & key, std::vector<unsigned char> const & len) : BinaryTrie() {
		trie.reserve(511);
		unsigned char i = 0;
		do {
			int pos = 0;
			for (unsigned char j = len[i]; j-- > 0;) {
				bool bit = key[i] & (1ULL << j);
				uint16_t to = bit ? trie[pos].right : trie[pos].left;
				if (to == trie_node::nullpointer) {
					to = trie.size();
					trie.push_back({});
					if (bit)
						trie[pos].right = to;
					else
						trie[pos].left = to;

				}
				pos = to;
			}
			trie[pos].term = true;
			trie[pos].c = i;
		} while (++i != 0);
		if (trie.size() != 511) throw std::invalid_argument("Trie construction error:\nTrie was built on incorrect input");
	}
	bool termimal() const {
		if (pos_trie < 0 || pos_trie >= trie.size()) throw std::out_of_range("Trie miscarriage:\nCalling terminal() on non-existing trie_node");
		return trie[pos_trie].term;
	}
	unsigned char character() const {
		assert(termimal());
		//You shouldn't call character() is if you are not sure it is terminal state
		return trie[pos_trie].c;
	}
	void make_step(bool bit) {
		if (pos_trie < 0 || pos_trie >= trie.size()) throw std::out_of_range("Trie miscarriage:\nCalling make_step() on non-existing trie_node");
		pos_trie = bit ? trie[pos_trie].right : trie[pos_trie].left;
	}
	void reset() {
		pos_trie = 0;
	}
private:
	struct trie_node : public node {
		bool term;
		char c;

		trie_node() : term(false) {}
	};
	uint16_t pos_trie;
	std::vector<trie_node> trie;
};


void decode(std::istream &input, std::ostream &output) {
	buffered_reader in(input);
	buffered_writer out(output);

	unsigned char counter_bit = 9;
	in.read_char(counter_bit);
	if (counter_bit > 8) throw std::invalid_argument("Decoder file is corrupted:\nThe number of bits used in last BYTE is more than 8");

	std::vector<unsigned char> code_lengths(256);
	for (auto & x : code_lengths) {
		if (!in.read_char(x)) throw std::invalid_argument("Decoder file is corrupted:\nExpected 256-code-length alphabet");
	}

	std::vector<uint64_t> codes = get_code(code_lengths);
	BinaryTrie trie(codes, code_lengths);	

	unsigned char current, next = '!'; 
	if (!in.read_char(current)) throw std::invalid_argument("Decoder file is corrupted:\nLast byte is sacred and must be presented by hell or high water");
	bool last_time = true, was_read = false;
	int CONDITION = -1;
	while ((was_read = in.read_char(next)) || last_time) {
		if (!was_read) {
			if (counter_bit == 0 && next != '!') counter_bit = 8; //zero is actually eight :)) unless it's an empty file
			last_time = false;
			CONDITION = 7 - counter_bit;
		}
		for (int j = 7; j > CONDITION; --j) {
			bool cur_bit = current & (1 << j);
			trie.make_step(cur_bit);
			if (trie.termimal()) {
				out.write_char(trie.character());
				trie.reset();
			}
		}
		current = next;
		next = '?';
	}
}

//}
#endif

