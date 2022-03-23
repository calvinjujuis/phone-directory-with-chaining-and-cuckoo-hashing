#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>
using namespace std;

int find_first_bigger_prime(int start) {
// pre: start > 10
// post: returns smallest prime that is at least start
    if (start%2 == 0) { start++; }  // no need to test even numbers
    for (int i=start; i<=(2*start); i+=2) {
//By Bertrand's postulate, a prime has to exist in (start, 2*start)
        bool isPrime = true;
        for (int j=3; j*j <= i; j+=2) {
            if (i%j == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime==true) {
            return i;
        }
    }
    return -1;
}

int hash0(long key, int tableSize) {
    return (int) (key%tableSize);
}

int hash1(long key, int tableSize) {
    double phi = (sqrt(5)-1)/2;
    double val = key * phi;
    return (int) floor(tableSize*(val - floor(val)));
}

int flatten(string str, int bucketSize) {
    // using horner's rule
    int flattened = 0;
    int R = 255;
    for (int i = 0; i < str.length(); ++i) {
        flattened *= R;
        flattened += str[i];
        flattened = flattened % bucketSize;
    }
    return flattened;
}

string phoneNumberFormat(long phone) {
    string phoneStr = to_string(phone);
    int len = 10 - phoneStr.length();
    while (len > 0) { // pad zeros if needed
        phoneStr = '0' + phoneStr;
        --len;
    }
    string formatted = "";
    formatted = '(' + phoneStr.substr(0,3) + ')';
    formatted = formatted + phoneStr.substr(3,3) + '-' + phoneStr.substr(6,4);
    return formatted;
}

long trim(string phone) {
    string str = phone.substr(1, 3) + phone.substr(5, 3) + phone.substr(9, 4);
    long trimmed = stol(str);
    return trimmed;
}

struct Contact {
    long phone;
    string name;
};

struct LinkedList {
    Contact *node;
    LinkedList *next;
};

struct LinkedListHead {
    int size = 0;
    LinkedList *head = nullptr;
};

class NameTable {
    // chaining hashing
    int size;
    int item;
    vector<LinkedListHead *> buckets;
    void deleteBuckets() {
        for (int i = 0; i < size; ++i) {
            delete buckets[i];
        }
    }

  public:

    NameTable(int num) {
        size = num;
        item = 0;
        for (int i = 0; i < size; ++i) {
            LinkedListHead *newHead = new LinkedListHead();
            buckets.push_back(newHead); // initialize hash table
        }
    }

    void insert(Contact *c) {
        if (item + 1 > 2 * size) rehash();
        ++item;
        LinkedList *newNode = new LinkedList();
        newNode->node = c;
        newNode->next = nullptr;
        int idx = flatten(c->name, size);
        LinkedList **current = &buckets[idx]->head;
        if (buckets[idx]->size == 0) {
            *current = newNode;
            ++buckets[idx]->size;
            return;
        }
        ++buckets[idx]->size;
        if (strcmp(to_string(c->phone).c_str(), to_string((*current)->node->phone).c_str()) < 0) {
            newNode->next = *current;
            buckets[idx]->head = newNode;
            return;
        }
        // insert based on lexicographical order of phone numbers
        while ((*current)->next != nullptr) {
            if (strcmp(to_string(c->phone).c_str(), to_string((*current)->next->node->phone).c_str()) < 0) {
                // current name precedes next name
                newNode->next = (*current)->next;
                (*current)->next = newNode;
                return;
            }
            current = &(*current)->next;
        }
        // insert at the end of bucket
        (*current)->next = newNode;
        return;
    }

    void printAllPhone(string name) {
        int idx = flatten(name, size);
        LinkedList *current = buckets[idx]->head;
        bool Found = 0;
        if (buckets[idx]->size == 0) {
            cout << "not found" << endl;
            return;
        }
        while (current != nullptr) {
            if (current->node->name == name) {
                Found = 1;
                cout << phoneNumberFormat(current->node->phone) << " ";
            }
            current = current->next;
        }
        if (! Found) cout << "not found";
        cout << endl;
    }

    void printTable() {
        cout << size;
        for (int i = 0; i < size; ++i) {
            cout << " " << buckets[i]->size;
        }
        cout << endl;
    }

    void rehash() {
        int newSize = find_first_bigger_prime(2 * size + 1);
        vector<Contact *> contacts;
        // traverse the original table and insert all keys into the new table
        for (int i = 0; i < size; ++i) {
            LinkedList *current = buckets[i]->head;
            LinkedList *next = nullptr;
            for (int j = 0; j < buckets[i]->size; ++j) {
                contacts.push_back(current->node);
                next = current->next;
                delete current;
                current = next;
            }
            buckets[i]->size = 0;
        }
        for (int i = size; i < newSize; ++i) {
            LinkedListHead *newHead = new LinkedListHead();
            buckets.push_back(newHead); // initialize hash table
        }
        size = newSize;
        item = 0;
        for (int i = 0; i < contacts.size(); ++i) {
            insert(contacts[i]);
        }
    }

    void deleteNodes() {
        for (int i = 0; i < size; ++i) {
            LinkedList *current = buckets[i]->head;
            while (current != nullptr) {
                LinkedList * node = current;
                current = current->next;
                delete node;
            }
            delete buckets[i];
        }
    }
    
    void deleteKeys() {
        for (int i = 0; i < size; ++i) {
            LinkedList *current = nullptr;
            if (buckets[i]->size != 0) current = buckets[i]->head;
            while (current != nullptr) {
                delete current->node;
                LinkedList * node = current;
                current = current->next;
                delete node;
            }
            delete buckets[i];
        }
    }

};

class PhoneTable {
    // cuckoo hashing
    int size;
    int item;
    vector<Contact *> bucket1;
    vector<Contact *> bucket2;

  public:

    PhoneTable(int num) {
        size = num;
        item = 0;
        for (int i = 0; i < size; ++i) { // initialize buckets
            bucket1.push_back(nullptr);
            bucket2.push_back(nullptr);
        }
    }

    void insert(Contact *c) {
        if (item + 1 > 2 * size) rehash();
        int count = 0; // rehash count if iteration passes 2n
        int i = 0;
        while (count <= 2 * item) {
            ++count;
            if (i == 0) {
                if (bucket1[hash0(c->phone, size)] == nullptr) { // insert if empty
                    bucket1[hash0(c->phone, size)] = c;
                    ++item;
                    return;
                } else { // kick out old if not empty
                    swap(c, bucket1[hash0(c->phone, size)]);
                    i = 1 - i;
                }
            } else if (i == 1) {
                if (bucket2[hash1(c->phone, size)] == nullptr) { // insert if empty
                    bucket2[hash1(c->phone, size)] = c;
                    ++item;
                    return;
                } else { // kick out old if not empty
                    swap(c, bucket2[hash1(c->phone, size)]);
                    i = 1 - i;
                }
            }
            
        }
        rehash();
        insert(c);
    }

    void printName(long phone) {
        int idx1 = hash0(phone, size);
        int idx2 = hash1(phone, size);
        if (bucket1[idx1] != nullptr && bucket1[idx1]->phone == phone) {
            cout << bucket1[idx1]->name << endl;
        } else if (bucket2[idx2] != nullptr && bucket2[idx2]->phone == phone) {
            cout << bucket2[idx2]->name << endl;
        } else {
            cout << "not found" << endl;
        }
    }

    void printTable() {
        cout << size << " ";
        for (int i = 0; i < size; ++i) {
            if (bucket1[i] == nullptr) {
                cout << 0 << " ";
            } else {
                cout << 1 << " ";
            }
        }
        for (int i = 0; i < size; ++i) {
            if (bucket2[i] == nullptr) {
                cout << 0 << " ";
            } else {
                cout << 1 << " ";
            }
        }
        cout << endl;
    }

    void rehash() {
        PhoneTable newTable(find_first_bigger_prime(2 * size + 1));
        for (int i = 0; i < size; ++i) {
            if (bucket1[i] != nullptr) newTable.insert(bucket1[i]);
        }
        for (int i = 0; i < size; ++i) {
            if (bucket2[i] != nullptr) newTable.insert(bucket2[i]);
        }
        size = newTable.size;
        item = newTable.item;
        bucket1 = newTable.bucket1;
        bucket2 = newTable.bucket2;
    }
};

int main() {
    NameTable *nameTable = new NameTable(11);
    PhoneTable *phoneTable = new PhoneTable(11);

    while (true) {
        char cmd;
        cin >> cmd;
        switch (cmd)
        {
        case 'i':
        {
            string name, phone;
            cin >> name >> phone;
            Contact *newContact = new Contact;
            newContact->name = name;
            newContact->phone = trim(phone);
            nameTable->insert(newContact);
            phoneTable->insert(newContact);
            break;
        }
        case 'l':
        {
            string phone;
            cin >> phone;
            phoneTable->printName(trim(phone));
            break;
        }
        case 's':
        {
            string name;
            cin >> name;
            nameTable->printAllPhone(name);
            break;
        }
        case 'r':
            {
            nameTable->rehash();
            phoneTable->rehash();
            break;
            }
        case 'p':
        {
            int select;
            cin >> select;
            if (select == 0) {
                nameTable->printTable();
            } else if (select == 1) {
                phoneTable->printTable();
            }
            break;
        }
        case 'x':
            nameTable->deleteKeys();
            //phoneTable->deleteKeys();
            delete nameTable;
            delete phoneTable;
            return 0;
        default:
            cout << "Unrecognized Command. Exit." << endl;
            return 1;
        }
    }
}
