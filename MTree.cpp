
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

template <class DT>
class MTree {
private:
    // Represents a node in the M-tree
    struct Node {
        vector<DT> values;       // Values stored in the node
        vector<Node*> children;  // Child pointers for internal nodes
        bool isLeaf;             // Flag indicating if the node is a leaf

        Node(bool leaf = true) : isLeaf(leaf) {}

        ~Node() {
            // Destructor to clean up dynamically allocated children
            for (auto child : children) delete child;
        }

        // Inserts a value into the node in sorted order
        bool insertValue(const DT& value) {
            auto it = lower_bound(values.begin(), values.end(), value);
            if (it != values.end() && *it == value) return false; // Duplicate value
            values.insert(it, value);
            return true;
        }

        // Removes a value from the node
        bool removeValue(const DT& value) {
            auto it = lower_bound(values.begin(), values.end(), value);
            if (it == values.end() || *it != value) return false; // Value not found
            values.erase(it);
            return true;
        }
    };

    Node* root;              // Root of the tree
    const int M;             // Maximum number of children per node
    set<DT> removedValues;   // Set of values that have been removed

    // Recursively removes a value from the tree starting from the given node
    void remove(Node*& node, const DT& value) {
        if (!node) {
            // Base case: value not found
            cout << "The value = " << value << " not found.\n";
            return;
        }

        auto it = lower_bound(node->values.begin(), node->values.end(), value);
        if (it != node->values.end() && *it == value) {
            // Value found in the current node
            if (node->isLeaf) {
                // Remove value from the leaf node
                node->removeValue(value);
                removedValues.insert(value); // Track removed values
                cout << "The value = " << value << " has been removed.\n";
                return;
            }

            // For internal nodes, replace the value with its successor
            Node* successor = node->children[it - node->values.begin() + 1];
            while (!successor->isLeaf) successor = successor->children.front();
            DT replacement = successor->values.front();
            node->values[it - node->values.begin()] = replacement;
            remove(node->children[it - node->values.begin() + 1], replacement);
        } else if (!node->isLeaf) {
            // Recurse into the appropriate child node
            int childIndex = distance(node->values.begin(), it);
            remove(node->children[childIndex], value);
        } else {
            // Value not found in the current leaf node
            cout << "The value = " << value << " not found.\n";
        }
    }

    // Recursively collects all values in the tree
    void collectValues(const Node* node, vector<DT>& result) const {
        if (!node) return;

        // Traverse children and collect values in sorted order
        for (size_t i = 0; i < node->children.size(); ++i) {
            collectValues(node->children[i], result);
            if (i < node->values.size()) result.push_back(node->values[i]);
        }

        // Add values from the leaf nodes
        if (node->isLeaf) {
            result.insert(result.end(), node->values.begin(), node->values.end());
        }
    }

public:
    // Constructor to initialize the tree with a given M value
    explicit MTree(const int m) : M(m), root(nullptr) {
        if (m < 2) {
            cout << "Error: M must be at least 2\n";
            throw runtime_error("Invalid M value");
        }
    }

    // Destructor to clean up the tree
    ~MTree() { delete root; }

    // Inserts a value into the tree
    void insert(const DT value) {
        // Reinserts a previously removed value
        if (removedValues.find(value) != removedValues.end()) {
            if (!root) root = new Node(true);

            Node* current = root;
            while (!current->isLeaf) {
                auto it = upper_bound(current->values.begin(), current->values.end(), value);
                int childIndex = distance(current->values.begin(), it);
                current = current->children[childIndex];
            }

            current->insertValue(value);
            removedValues.erase(value); // Remove from the removed set
            cout << "The value = " << value << " has been inserted.\n";
            return;
        }

        // Normal insertion logic
        if (!root) {
            root = new Node(true);
            root->values.push_back(value);
            return;
        }

        Node* current = root;
        while (!current->isLeaf) {
            auto it = upper_bound(current->values.begin(), current->values.end(), value);
            int childIndex = distance(current->values.begin(), it);
            current = current->children[childIndex];
        }

        if (!current->insertValue(value)) {
            cout << "The value = " << value << " already in the tree.\n";
        }
    }

    // Removes a value from the tree
    void remove(const DT value) {
        remove(root, value);
        rebuildTree();
    }

    // Checks if a value exists in the tree
    bool contains(const DT value) const {
        Node* current = root;
        while (current) {
            auto it = lower_bound(current->values.begin(), current->values.end(), value);
            if (it != current->values.end() && *it == value) {
                cout << "The element with value = " << value << " was found.\n";
                return true;
            }
            if (current->isLeaf) break;
            int childIndex = distance(current->values.begin(), it);
            current = current->children[childIndex];
        }
        cout << "The element with value = " << value << " not found.\n";
        return false;
    }

    // Returns all values in the tree in sorted order
    vector<DT> getAllValues() const {
        vector<DT> result;
        collectValues(root, result);
        return result;
    }

    // Rebuilds the tree from its current values
    void rebuildTree() {
        vector<DT> allValues = getAllValues();
        delete root;
        root = nullptr;
        for (const auto& value : allValues) {
            insert(value);
        }
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, MValue;
    cin >> n;

    vector<int> values(n);
    for (int i = 0; i < n; ++i) cin >> values[i];
    sort(values.begin(), values.end());

    cin >> MValue;
    MTree<int> tree(MValue);
    for (const auto& value : values) tree.insert(value);

    int numCommands;
    cin >> numCommands;

    char command;
    int value;
    while (numCommands--) {
        cin >> command;
        switch (command) {
            case 'I':
                cin >> value;
                tree.insert(value);
                break;
            case 'R':
                cin >> value;
                tree.remove(value);
                break;
            case 'F':
                cin >> value;
                tree.contains(value);
                break;
            case 'B':
                tree.rebuildTree();
                cout << "The tree has been rebuilt.\n";
                break;
            default:
                cout << "Invalid command.\n";
        }
    }

    const auto finalValues = tree.getAllValues();
    cout << "Final list: ";

    for (size_t i = 0; i < finalValues.size(); ++i) {
        cout << finalValues[i];
        cout << (((i + 1) % 20 == 0 || i == finalValues.size() - 1) ? '\n' : ' ');
    }

    return 0;
};