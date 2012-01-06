#ifndef HEADER_35b6f286dbbc43bd8cf4efc47aa6952f
#define HEADER_35b6f286dbbc43bd8cf4efc47aa6952f


struct node_t {
    std::string val;
    int sibling;
    int child;
    int parent;
};

struct projected_t {
    int depth;
    unsigned int support;
    std::vector <std::pair <unsigned int, int> > locations;
};

typedef std::vector <node_t> nodes_t;


template <class T, class Iterator> 
void tokenize (const std::string &str, Iterator iterator) 
{
    std::istringstream is(str);
    std::copy (std::istream_iterator <T> (is), std::istream_iterator <T> (), iterator);
}

typedef void (*READER_FUNC)(const std::string& , nodes_t&);

void str2node (const std::string& str, nodes_t& node)
{
    try {
            unsigned int len = str.size();
            unsigned int size = 0;
            std::string buf = "";
            std::vector <std::string> tmp;

            for (unsigned int i = 0; i < len; i++) {
                if (str[i] == '(' || str[i] == ')') {
                if (! buf.empty()) {
                tmp.push_back (buf);
                buf = "";
                ++size;
                }
                if (str[i] == ')') tmp.push_back (")");
                } else if (str[i] == '\t' || str[i] == ' ') { 	  // do nothing
                } else {
                buf += str[i];
                }
            }

        if (! buf.empty()) throw 2;

        node.resize (size);
        std::vector <int> sibling (size);
        for (unsigned int i = 0; i < size; ++i) {
            node[i].parent = -1;
            node[i].child = -1;
            node[i].sibling = -1;
            sibling[i] = -1;
        }

        std::vector <int> sr;
        unsigned int id = 0;
        int top = 0;

        for (unsigned int i = 0; i < tmp.size(); ++i) {
            if (tmp[i] == ")") {
                top = sr.size()-1;
                if (top < 1) continue;
                unsigned int child  = sr[top];
                unsigned int parent = sr[top-1];
                node[child].parent = parent;
                if (node[parent].child == -1) node[parent].child = child;
                if (sibling[parent] != -1) node[sibling[parent]].sibling = child;
                sibling[parent] = child;
                sr.resize (top);
            } else {
                node[id].val = tmp[i];
                sr.push_back (id);
                id++;
            }
        }
        return;
    }
    catch (const int) {
        std::cerr << "Fatal: parse error << [" << str << "]\n";
        std::exit (-1);
    }
}



#endif
