/**
   ___ _                 _
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#include "fvm/bs_tree.h"
#include <algorithm>

namespace fvm {

                        /* ======= struct treeNode ======= */
inline treeNode::treeNode() : treeNode(TreeNodeType()) {}  // Default constructor

treeNode::treeNode(TreeNodeType type) {
    this->type = type;
    this->cnt = 1;
    this->next_brother = nullptr;
    this->link = -1;
    this->child_index = nullptr;  // OPTIMIZATION: Initialize to nullptr

    if (type == FILE_NODE || type == HEAD_NODE) {
        this->first_son = nullptr;
    } else if (type == DIR_NODE) {
        this->first_son = new treeNode(HEAD_NODE);
        // OPTIMIZATION: Create child index for DIR nodes
        this->child_index = new std::unordered_map<std::string, treeNode*>();
    }
}

// OPTIMIZATION: Destructor to clean up child_index
treeNode::~treeNode() {
    delete child_index;  // Safe to delete nullptr
    child_index = nullptr;
}


                        /* ======= class BSTree ======= */

BSTree::BSTree(fvm::interfaces::ILogger& logger, fvm::interfaces::INodeManager& node_manager)
    : logger_(logger), node_manager_(node_manager), path_cache_valid_(false) {
    path.clear();
}

// OPTIMIZATION: Ensure child index is built for a directory node
// Lazily builds the index from the sibling list on first access
void BSTree::ensure_child_index(treeNode* dir_node) {
    if (dir_node == nullptr || dir_node->type != DIR_NODE) {
        return;  // Only DIR nodes need an index
    }
    if (dir_node->child_index != nullptr) {
        return;  // Index already built
    }

    // Create the index
    dir_node->child_index = new std::unordered_map<std::string, treeNode*>();

    // Traverse siblings starting from first_son (which is HEAD_NODE)
    treeNode* current = dir_node->first_son;
    if (current != nullptr && current->next_brother != nullptr) {
        // Skip HEAD_NODE and index all actual children
        for (treeNode* child = current->next_brother; child != nullptr; child = child->next_brother) {
            if (child->link != (unsigned long long)-1) {
                std::string name = node_manager_.get_name(child->link);
                (*dir_node->child_index)[name] = child;
            }
        }
    }
}

bool BSTree::check_path() {
    if (path.empty()) {
        logger_.log("Path is empty. This not normal.", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }
    for (auto &it : path) {
        if (it == nullptr) {
            logger_.log("Null pointer exists in path. This not normal.", fvm::interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }
    }
    return true;
}

bool BSTree::check_node(treeNode *p, int line) {
    if (p == nullptr) {
        logger_.log("The pointer is empty, please check whether the program is correct.", fvm::interfaces::LogLevel::FATAL, line);
        return false;
    }
    if (p->cnt <= 0) {
        logger_.log("The node counter is already less than or equal to 0, please check the program!", fvm::interfaces::LogLevel::FATAL, line);
        return false;
    }
    return true;
}

bool BSTree::is_son() {
    if (!check_path()) return false;
    return path.back()->type == 2;
}

bool BSTree::goto_tail() {
    if (!check_path()) return false;
    // OPTIMIZATION: Invalidate path cache since path will change
    invalidate_path_cache();
    while (path.back()->next_brother != nullptr) {
        path.push_back(path.back()->next_brother);
    }
    if (!check_path()) return false;
    return true;
}

bool BSTree::goto_head() {
    if (!check_path()) return false;
    // OPTIMIZATION: Invalidate path cache since path will change
    invalidate_path_cache();
    for (; !path.empty() && !is_son(); path.pop_back());
    if (!check_path()) return false;
    return true;
}

bool BSTree::name_exist(std::string name) {
    if (!goto_head()) return false;
    if (path.size() < 2) return false;

    // Get parent directory node (second-to-last in path)
    treeNode* parent_dir = path[path.size() - 2];

    // OPTIMIZATION: Use child index if available for O(1) lookup
    if (parent_dir->child_index != nullptr) {
        return parent_dir->child_index->find(name) != parent_dir->child_index->end();
    }

    // Fallback: traverse siblings (O(n))
    std::vector<std::string> dir_content;
    if (!list_directory_contents(dir_content)) return false;
    for (auto &nm : dir_content) {
        if (nm == name) return true;
    }
    return false;
}

bool BSTree::go_to(std::string name) {
    if (!goto_head()) return false;

    // Get parent directory node (second-to-last in path)
    // After goto_head(), path.back() is HEAD_NODE, so parent is at size()-2
    if (path.size() < 2) {
        logger_.log("Invalid path size for go_to", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }

    treeNode* parent_dir = path[path.size() - 2];

    // OPTIMIZATION: Use child index if available for O(1) lookup
    ensure_child_index(parent_dir);

    if (parent_dir->child_index != nullptr) {
        auto it = parent_dir->child_index->find(name);
        if (it != parent_dir->child_index->end()) {
            // OPTIMIZATION: Invalidate path cache since path will change
            invalidate_path_cache();
            path.push_back(it->second);
            return true;
        } else {
            logger_.log("no file or directory named " + name, fvm::interfaces::LogLevel::WARNING, __LINE__);
            return false;
        }
    }

    // Fallback: linear scan (O(n)) for compatibility during migration
    if (!name_exist(name)) {
        logger_.log("no file or directory named " + name, fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }
    // OPTIMIZATION: Invalidate path cache since path will change
    invalidate_path_cache();
    while (node_manager_.get_name(path.back()->link) != name) {
        if (path.back()->next_brother == nullptr) {
            return false;
        }
        path.push_back(path.back()->next_brother);
    }
    return true;
}

bool BSTree::goto_last_dir() {
    if (!goto_head()) return false;
    if (path.size() > 2) {
        // OPTIMIZATION: Invalidate path cache since path will change
        invalidate_path_cache();
        path.pop_back();
    }
    if (!check_path()) return false;
    return true;
}

bool BSTree::list_directory_contents(std::vector<std::string> &content) {
    if (!goto_head()) return false;
    if (!check_path()) return false;

    // Get parent directory node (second-to-last in path)
    if (path.size() < 2) return false;
    treeNode* parent_dir = path[path.size() - 2];

    // OPTIMIZATION: Use child index if available (O(1) iteration)
    ensure_child_index(parent_dir);

    if (parent_dir->child_index != nullptr) {
        content.clear();
        content.reserve(parent_dir->child_index->size());
        for (const auto& pair : *parent_dir->child_index) {
            content.push_back(pair.first);
        }
        return true;
    }

    // Fallback: traverse siblings without modifying path
    content.clear();
    treeNode* head_node = path.back();
    for (treeNode* node = head_node->next_brother; node != nullptr; node = node->next_brother) {
        content.push_back(node_manager_.get_name(node->link));
    }
    return true;
}

bool BSTree::get_current_path(std::vector<std::string> &p) {
    // OPTIMIZATION: Use cached path if valid (O(1) on cache hit)
    if (path_cache_valid_) {
        p = cached_path_;
        return true;
    }

    // OPTIMIZATION: Build path directly from path vector (O(d) instead of O(d²))
    // The path vector contains: root -> HEAD_NODE -> dir1 -> HEAD_NODE -> dir2 -> HEAD_NODE -> ...
    // We need to extract names from DIR nodes (every other node starting from root)
    p.clear();

    for (size_t i = 0; i < path.size(); i++) {
        treeNode* node = path[i];
        // Skip HEAD_NODE nodes (type == 2)
        if (node->type != HEAD_NODE) {
            p.push_back(node_manager_.get_name(node->link));
        }
    }

    // Update cache
    cached_path_ = p;
    path_cache_valid_ = true;

    return true;
}

} // namespace fvm
