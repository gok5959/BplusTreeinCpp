#include <fstream>
#include <vector>
#include <iostream>
#include <cmath>
#include <map>
#include <queue>

using namespace std;

struct header {
    int block_size;
    int root_bid;
    int depth;

    header() {
        block_size = 0;
        root_bid = 0;
        depth = 0;
    }
};

struct index_entry {
    int key;
    int next_level_bid;

    index_entry() {
        key = 0;
        next_level_bid = 0;
    }

    index_entry(int key, int next_level_bid) {
        this->key = key;
        this->next_level_bid = next_level_bid;
    }
};

struct data_entry {
    int key;
    int value;

    data_entry() {
        key = 0;
        value = 0;
    }

    data_entry(int key, int value) {
        this->key = key;
        this->value = value;
    }
};

struct non_leaf_node {
    int bid;
    int next_level_bid;
    vector<index_entry> entry;

    non_leaf_node() {
        bid = 0;
        next_level_bid = 0;
    }
};

struct leaf_node {
    int bid;
    int next_bid;

    leaf_node() {
        bid = 0;
        next_bid = 0;
    }

    vector<data_entry> entry;
};

class Btree {
private:
    int cnt_block;
    const char *fileName;
    header h;
    fstream f;
    map<int, int> par;
public :
    Btree() {}

    Btree(const char *fileName, int blockSize) {
        this->fileName = fileName;
        this->h.block_size = blockSize;
        cnt_block = 0;
    }

    void create_file() {
        f.open(fileName, ios::out | ios::binary);
        f.write(reinterpret_cast<char *> (&h.block_size), sizeof(int));
        f.write(reinterpret_cast<char *> (&h.root_bid), sizeof(int));
        f.write(reinterpret_cast<char *> (&h.depth), sizeof(int));
        f.close();
    }
    void write_header() {
        f.open(fileName, ios::in | ios::out | ios::binary);
        f.seekp(0, ios::beg);
        f.write(reinterpret_cast<char *> (&h.block_size), sizeof(int));
        f.write(reinterpret_cast<char *> (&h.root_bid), sizeof(int));
        f.write(reinterpret_cast<char *> (&h.depth), sizeof(int));
        f.close();
    }
    void read_header() {
        f.open(fileName, ios::in | ios::binary);
        f.seekg(0, ios::beg);
        f.read(reinterpret_cast<char *> (&h.block_size), sizeof(int));
        f.read(reinterpret_cast<char *> (&h.root_bid), sizeof(int));
        f.read(reinterpret_cast<char *> (&h.depth), sizeof(int));
        cnt_block = entries_per_block(h.block_size);
        f.close();
    }
    leaf_node read_leaf_node(int bid) {
        f.open(fileName, ios::in | ios::binary);
        f.seekg(get_offset(bid));
        leaf_node tmp;
        int key, value, next_bid;
        for (int i = 0; i < cnt_block; i++) {
            f.read(reinterpret_cast<char *>(&(key)), sizeof(int));
            f.read(reinterpret_cast<char *>(&(value)), sizeof(int));
            if (key != 0 && value != 0) tmp.entry.push_back({key, value});
        }
        f.read(reinterpret_cast<char *>(&(next_bid)), sizeof(int));
        tmp.next_bid = next_bid;
        tmp.bid = bid;
        f.close();
        return tmp;
    }
    non_leaf_node read_non_leaf_node(int bid) {
        f.open(fileName, ios::in | ios::binary);
        f.seekg(get_offset(bid));
        non_leaf_node tmp;
        int key, next_bid, next_level_bid;
        f.read(reinterpret_cast<char *>(&next_level_bid), sizeof(int));
        for (int i = 0; i < cnt_block; i++) {
            f.read(reinterpret_cast<char *>(&key), sizeof(int));
            f.read(reinterpret_cast<char *>(&next_bid), sizeof(int));
            if (key && next_bid) tmp.entry.push_back({key, next_bid});
        }
        tmp.next_level_bid = next_level_bid;
        tmp.bid = bid;
        f.close();
        return tmp;
    }
    void write_non_leaf_node(int bid, non_leaf_node &cur) {
        f.open(fileName, ios::in | ios::out | ios::binary);
        f.seekp(get_offset(bid));
        f.write(reinterpret_cast<char *>(&cur.next_level_bid), sizeof(int));
        int tmp_memory = 0;
        for (int i = 0; i < cur.entry.size(); i++) {
            f.write(reinterpret_cast<char *>(&(cur.entry[i].key)), sizeof(int));
            f.write(reinterpret_cast<char *>(&(cur.entry[i].next_level_bid)), sizeof(int));
        }
        for (int i = 0; i < cnt_block - cur.entry.size(); i++) {
            f.write(reinterpret_cast<char *>(&tmp_memory), sizeof(int));
            f.write(reinterpret_cast<char *>(&tmp_memory), sizeof(int));
        }
        f.close();
    }
    void write_leaf_node(int bid, leaf_node &cur) {
        f.open(fileName, ios::in | ios::out | ios::binary);
        f.seekp(get_offset(bid));
        int tmp_memory = 0;
        int non_empty = cur.entry.size();
        for (int i = 0; i < non_empty; i++) {
            f.write(reinterpret_cast<char *>(&(cur.entry[i].key)), sizeof(int));
            f.write(reinterpret_cast<char *>(&(cur.entry[i].value)), sizeof(int));

        }
        for (int i = 0; i < cnt_block - non_empty; i++) {
            f.write(reinterpret_cast<char *>(&tmp_memory), sizeof(int));
            f.write(reinterpret_cast<char *>(&tmp_memory), sizeof(int));
        }
        f.write(reinterpret_cast<char *>(&(cur.next_bid)), sizeof(int));
        f.close();
    }

    int get_offset(int bid) {
        return 12 + (bid - 1) * h.block_size;
    }
    int entries_per_block(int block_size) {
        return (block_size - 4) / 8;
    }
    void set_block_size(int block_size) {
        h.block_size = block_size;
    }

    non_leaf_node split_non_leaf_node(non_leaf_node &prev_node) {
        non_leaf_node next_node;

        next_node.bid = get_next_bid();

        int size = prev_node.entry.size();
        for (int i = cnt_block / 2; i < size; i++) {
            next_node.entry.push_back(prev_node.entry[i]);
        }
        for (int i = cnt_block / 2; i < size; i++) {
            prev_node.entry.pop_back();
        }

        index_entry tmp_entry = next_node.entry[0];
        next_node.next_level_bid = tmp_entry.next_level_bid;
        next_node.entry.erase(next_node.entry.begin());
        write_non_leaf_node(prev_node.bid, prev_node);
        write_non_leaf_node(next_node.bid, next_node);
        if (par.find(prev_node.bid) == par.end()) {
            non_leaf_node new_root;
            new_root.bid = get_next_bid();
            h.root_bid = new_root.bid;
            h.depth++;
            new_root.next_level_bid = prev_node.bid;
            insert_to_index_entry(new_root.entry, tmp_entry.key, next_node.bid);
            write_non_leaf_node(new_root.bid, new_root);
            write_header();
        } else {
            non_leaf_node parent = read_non_leaf_node(par[prev_node.bid]);
            insert_to_index_entry(parent.entry, tmp_entry.key, next_node.bid);
            if (parent.entry.size() <= cnt_block) {
                write_non_leaf_node(parent.bid, parent);
            } else {
                split_non_leaf_node(parent);
            }
        }
        return next_node;
    }
    void split_leaf_node(leaf_node &prev_node) {
        leaf_node next_node;
        next_node.bid = get_next_bid();
        next_node.next_bid = prev_node.next_bid;
        prev_node.next_bid = next_node.bid;

        int size = prev_node.entry.size();
        for (int i = cnt_block / 2; i < size; i++) {
            next_node.entry.push_back(prev_node.entry[i]);
        }
        for (int i = cnt_block / 2; i < size; i++) {
            prev_node.entry.pop_back();
        }

        write_leaf_node(prev_node.bid, prev_node);
        write_leaf_node(next_node.bid, next_node);

        // get parent
        if (par.find(prev_node.bid) == par.end()) { // 현재 노드가 root인 경우
            non_leaf_node new_root;
            new_root.bid = get_next_bid();
            h.root_bid = new_root.bid;
            h.depth++;
            new_root.next_level_bid = prev_node.bid;
            insert_to_index_entry(new_root.entry, next_node.entry[0].key, next_node.bid);
            write_non_leaf_node(new_root.bid, new_root);
            write_header();
        } else {
            non_leaf_node parent = read_non_leaf_node(par[prev_node.bid]);
            insert_to_index_entry(parent.entry, next_node.entry[0].key, next_node.bid);
            if (parent.entry.size() <= cnt_block) {
                write_non_leaf_node(parent.bid, parent);
            } else {
                split_non_leaf_node(parent);
            }
        }
    }

    void insert_to_data_entry(vector<data_entry> &entry, int key, int value) {
        auto ite = entry.begin();
        bool is_inserted = false;
        for (ite = entry.begin(); ite != entry.end(); ite++) {
            if (key < (*ite).key) {
                entry.insert(ite, {key, value});
                is_inserted = true;
                break;
            }
        }
        if (!is_inserted) entry.insert(ite, {key, value});
    }
    void insert_to_index_entry(vector<index_entry> &entry, int key, int value) {
        auto ite = entry.begin();
        bool is_inserted = false;
        for (ite = entry.begin(); ite != entry.end(); ite++) {
            if (key < (*ite).key) {
                entry.insert(ite, {key, value});
                is_inserted = true;
                break;
            }
        }
        if (!is_inserted) entry.insert(ite, {key, value});
    }

    void insert(int key, int rid) {
        if (h.root_bid == 0) {
            leaf_node tmp;
            tmp.bid = 1;
            tmp.entry.push_back({key, rid});
            h.root_bid = 1;
            write_header();
            write_leaf_node(1, tmp);
        } else {
            par.clear();
            leaf_node tmp = find_leaf_node(key, h.depth, h.root_bid);
            insert_leaf_node(tmp, key, rid);
        }
    }
    void insert_leaf_node(leaf_node &tmp, int key, int value) {
        insert_to_data_entry(tmp.entry, key, value);
        if (tmp.entry.size() <= cnt_block) {
            write_leaf_node(tmp.bid, tmp);
        } else {
            split_leaf_node(tmp);
        }
    }

    leaf_node find_leaf_node(int key, int depth, int bid) {

        if (depth == 0) {
            leaf_node tmp = read_leaf_node(bid);
            return tmp;
        } else {
            non_leaf_node cur = read_non_leaf_node(bid);
            int next_bid = cur.next_level_bid;
            for (int i = 0; i < cur.entry.size(); i++) {
                if (key >= cur.entry[i].key) next_bid = cur.entry[i].next_level_bid;
            }
            par[next_bid] = bid;
            return find_leaf_node(key, depth - 1, next_bid);
        }
    }

    vector<vector<int>> print() {
        read_header();
        vector<int> level_root;
        vector<int> level_one;
        if (h.depth >= 1) {
            non_leaf_node root = read_non_leaf_node(h.root_bid);
            queue<pair<int, int>> next_bid_set;
            next_bid_set.push({root.next_level_bid, h.depth - 1});

            for (int i = 0; i < root.entry.size(); i++) {
                level_root.push_back(root.entry[i].key);
                next_bid_set.push({root.entry[i].next_level_bid, h.depth - 1});
            }
            while (!next_bid_set.empty()) {
                pair<int, int> set = next_bid_set.front();
                next_bid_set.pop();

                if (set.second == 0) {
                    leaf_node leaf = read_leaf_node(set.first);
                    for (int i = 0; i < leaf.entry.size(); i++) {
                        level_one.push_back(leaf.entry[i].key);
                    }
                } else {
                    non_leaf_node non_leaf = read_non_leaf_node(set.first);
                    for (int i = 0; i < non_leaf.entry.size(); i++) {
                        level_one.push_back(non_leaf.entry[i].key);
                    }
                }
            }
        } else if(h.depth == 0) {
            if (h.root_bid != 0) {
                leaf_node root = read_leaf_node(h.root_bid);
                for(int i = 0; i < root.entry.size(); i++){
                    level_root.push_back(root.entry[i].key);
                }
            }
        }
        vector<vector<int>> tmp;
        tmp.push_back(level_root);
        tmp.push_back(level_one);

        return tmp;
    }

    int search(int key) {
        read_header();
        leaf_node tmp = find_leaf_node(key, h.depth, h.root_bid);
        bool flag = false;
        int val = 0;
        while (tmp.next_bid != 0) {

            for (int i = 0; i < tmp.entry.size(); i++) {
                if (key == tmp.entry[i].key) {
                    val = tmp.entry[i].value;
                    flag = true;
                }
            }
            if (flag) break;
            tmp = read_leaf_node(tmp.next_bid);
        }
        return val;
    }

    vector<pair<int, int>> search(int startRange, int endRange) {
        vector<pair<int, int>> _tmp;
        read_header();
        leaf_node tmp = find_leaf_node(startRange, h.depth, h.root_bid);
        bool flag = false;
        while (tmp.next_bid != 0) {
            for (int i = 0; i < tmp.entry.size(); i++) {
                if (endRange < tmp.entry[i].key) {
                    flag = true;
                    break;
                } else if (startRange <= tmp.entry[i].key) {
                    _tmp.push_back({tmp.entry[i].key, tmp.entry[i].value});
                }
            }
            if (flag) break;
            tmp = read_leaf_node(tmp.next_bid);
        }
        return _tmp;
    }

    int get_next_bid() {
        f.open(fileName, ios::in | ios::binary);
        f.seekg(0, f.end);
        int tmp = f.tellg();
        int next_bid = (int) (tmp - 12) / h.block_size + 1;
        f.close();
        return next_bid;
    }

    void cur_tree_bfs() {
        read_header();
        queue<pair<int, int>> q;
        int depth = h.depth;
        non_leaf_node cur;
        leaf_node cur2;
        q.push({h.root_bid, h.depth});
        while (!q.empty()) {
            pair<int, int> pii;
            pii = q.front();
            cout << pii.first << ' ';
            q.pop();
            if (pii.second != 0) {
                cur = read_non_leaf_node(pii.first);
                cout << "cur node is non_leaf_node ";
                if (cur.next_level_bid != 0) {
                    q.push({cur.next_level_bid, pii.second - 1});
                }
                for (int i = 0; i < cur.entry.size(); i++) {
                    cout << cur.entry[i].key << ' ';
                    if (cur.entry[i].next_level_bid != 0) {
                        q.push({cur.entry[i].next_level_bid, pii.second - 1});
                    }
                }
                cout << "this node has a " << cur.entry.size() << ' ' << "entries" << '\n';
            } else {
                cur2 = read_leaf_node(pii.first);
                cout << "cur node is leaf" << ' ';
                for (int i = 0; i < cur2.entry.size(); i++) {
                    cout << cur2.entry[i].key << ' ';
                }
                cout << "this node has a " << cur2.entry.size() << ' ' << "entries" << '\n';
            }
        }
    }
    void all_the_leaf() {
        leaf_node tmp = find_leaf_node(8, h.depth, h.root_bid);
        while (tmp.next_bid != 0) {
            for (int i = 0; i < tmp.entry.size(); i++) {
                cout << tmp.entry[i].key << ' ';
            }
            cout << '\n';
            tmp = read_leaf_node(tmp.next_bid);
        }
    }
};

int main(int argc, char *argv[]) {
    char command = argv[1][0];
    const char *fileName = argv[2];
    char *file_input;
    char *file_output;

    Btree *myBtree = new Btree(fileName, 0);
    fstream f;
    fstream f2;
    switch (command) {
        case 'c' :
            myBtree->set_block_size(atoi(argv[3]));
            myBtree->create_file();
            break;
        case 'i' :
            file_input = argv[3];
            f.open(file_input, ios::in);
            while (!f.eof()) {
                string data;
                getline(f, data);
                int con = data.find('|');
                int key = 0;
                int value = 0;
                key = stoi(data.substr(0, con));
                value = stoi(data.substr(con + 1, data.size() - con - 1));
                myBtree->read_header();
                myBtree->insert(key, value);
            }
            f.close();
            break;
        case 's' :
            file_input = argv[3];
            file_output = argv[4];

            f.open(file_input, ios::in);
            f2.open(file_output, ios::out);
            while (!f.eof()) {
                string data;
                getline(f, data);
                int tmp = stoi(data);
                int tmp2 = myBtree->search(tmp);
                data = to_string(tmp) + '|' + to_string(tmp2) + '\n';
                char *c = strcpy(new char[data.length()], data.c_str());
                f2.write(c, strlen(c));
            }
            f.close();
            f2.close();
            break;
        case 'r' :
            file_input = argv[3];
            file_output = argv[4];

            f.open(file_input, ios::in);
            f2.open(file_output, ios::out);
            while (!f.eof()) {
                string data;
                getline(f, data);
                int delim = data.find('-');
                int tmp = stoi(data.substr(0, delim));
                int _tmp = stoi(data.substr(delim + 1, data.size() - delim - 1));
                vector<pair<int, int>> rs = myBtree->search(tmp, _tmp);
                string v = "";
                for (int i = 0; i < rs.size(); i++) {
                    v += to_string(rs[i].first) + '|' + to_string(rs[i].second) + ' ';
                }
                v += '\n';
                char *c = strcpy(new char[v.length()], v.c_str());
                f2.write(c, strlen(c));
            }
            f.close();
            f2.close();
            break;
        case 'p' : {
            f.open(argv[3], ios::out);
            vector<vector<int>> tmp = myBtree->print();
            string t;
            t = "<0>\n\n";
            char *c = strcpy(new char[t.length()], t.c_str());
            f.write(c, strlen(c));
            t = "";
            for(int i = 0; i < tmp[0].size(); i++){
                t += to_string(tmp[0][i]) + ',';
            }
            t.pop_back();
            c = strcpy(new char[t.length()], t.c_str());
            f.write(c, strlen(c));
            t = "\n\n<1>\n\n";
            c = strcpy(new char[t.length()], t.c_str());
            f.write(c, strlen(c));

            t="";
            for(int i = 0; i < tmp[1].size(); i++){
                t += to_string(tmp[1][i]) + ',';
            }
            t.pop_back();
            c = strcpy(new char[t.length()], t.c_str());
            f.write(c, strlen(c));
            f.close();
        }
            break;
        default:
            break;
    }
    delete myBtree;
}
